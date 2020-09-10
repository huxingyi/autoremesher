///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2011, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Industrial Light & Magic nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

#ifdef NDEBUG
#    undef NDEBUG
#endif

#include "testDeepScanLineBasic.h"
#include "random.h"

#include "ImfDeepScanLineInputFile.h"
#include "ImfDeepScanLineOutputFile.h"
#include "ImfDeepFrameBuffer.h"
#include "ImfPartType.h"
#include "ImfChannelList.h"
#include "ImfArray.h"
#include "IlmThreadPool.h"

#include "tmpDir.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <vector>

namespace IMF = OPENEXR_IMF_NAMESPACE;
using namespace IMF;
using namespace std;
using namespace IMATH_NAMESPACE;
using namespace ILMTHREAD_NAMESPACE;

namespace
{

const int width = 8193;
const int height = 1;
const int minX = 0;
const int minY = 0;
const long numGib = 1; // number of GiB to allocate for huge test
const Box2i dataWindow(V2i(minX, minY), V2i(minX + width - 1, minY + height - 1));
const Box2i displayWindow(V2i(0, 0), V2i(minX + width * 2, minY + height * 2));

vector<int> channelTypes;
Array2D<unsigned int> sampleCount;
vector<unsigned char> storage; // actual pixel storage for entire image (effectively)

Header header;

void
generateRandomFile (int channelCount,
                    Compression compression,
                    bool random_channel_data,
                    const std::string & fn)
{
    cout << "generating ... " << flush;
    header = Header(displayWindow, dataWindow,
                    1,
                    IMATH_NAMESPACE::V2f (0, 0),
                    1,
                    INCREASING_Y,
                    compression);

    cout << "compression " << compression << " " << flush;

    //
    // Add channels.
    //

    channelTypes.clear();

    for (int i = 0; i < channelCount; i++)
    {
        int type = random_int(3);
        stringstream ss;
        ss << i;
        string str = ss.str();
        if (type == 0)
            header.channels().insert(str, Channel(IMF::UINT));
        if (type == 1)
            header.channels().insert(str, Channel(IMF::HALF));
        if (type == 2)
            header.channels().insert(str, Channel(IMF::FLOAT));
        channelTypes.push_back(type);
    }

    header.setType(DEEPSCANLINE);

    Array<Array2D< void* > > data(channelCount);
    for (int i = 0; i < channelCount; i++)
        data[i].resizeErase(height, width);

    sampleCount.resizeErase(height, width);

    remove (fn.c_str());
    DeepScanLineOutputFile file (fn.c_str(), header, 8);

    DeepFrameBuffer frameBuffer;

    frameBuffer.insertSampleCountSlice (Slice (IMF::UINT,                    // type // 7
                                        (char *) (&sampleCount[0][0]
                                                  - dataWindow.min.x
                                                  - dataWindow.min.y * width),               // base // 8
                                        sizeof (unsigned int) * 1,          // xStride// 9
                                        sizeof (unsigned int) * width));    // yStride// 10

    
    // count total size of all pixels
    Int64 bytes_per_sample = 0;
    for (int i = 0; i < channelCount; i++)
    {
        PixelType type = NUM_PIXELTYPES;
        if (channelTypes[i] == 0)
            type = IMF::UINT;
        if (channelTypes[i] == 1)
            type = IMF::HALF;
        if (channelTypes[i] == 2)
            type = IMF::FLOAT;

        stringstream ss;
        ss << i;
        string str = ss.str();

        int sampleSize = 0;
        if (channelTypes[i] == 0) sampleSize = sizeof (unsigned int);
        if (channelTypes[i] == 1) sampleSize = sizeof (half);
        if (channelTypes[i] == 2) sampleSize = sizeof (float);

        int pointerSize = sizeof(char *);

         bytes_per_sample+=sampleSize;
        
        frameBuffer.insert (str,                            // name // 6
                            DeepSlice (type,                    // type // 7
                            (char *) (&data[i][0][0]
                                      - dataWindow.min.x
                                      - dataWindow.min.y * width),               // base // 8
                            pointerSize * 1,          // xStride// 9
                            pointerSize * width,      // yStride// 10
                            sampleSize));             // sampleStride
    }

    file.setFrameBuffer(frameBuffer);

    cout << "writing file " << endl;

    Int64 total_number_of_samples = 0;
    
    // compute ideal number of samples per pixel assuming we want abotut 5GiB of data
    // int samples_per_pixel = int(5l*1024l*1024l*1024l/Int64(width*height)) / bytes_per_sample;

    // compute ideal number of samples per pixel assuming we want abotut 15GiB of data
    int samples_per_pixel = int(numGib*1024l*1024l*1024l/Int64(width*height)) / bytes_per_sample;
    
    cout << "  generating approx. " << samples_per_pixel << " samples per pixel\n";
    
    for (int i = 0; i < height; i++)
    {
            for (int j = 0; j < width; j++)
            {
                sampleCount[i][j] = random_int(4000) + (samples_per_pixel-2000);
                total_number_of_samples += sampleCount[i][j];
            }
    }
    
    cout << "  total number of samples: " << total_number_of_samples << std::endl;
    cout << "  storage required: " << total_number_of_samples*bytes_per_sample << " bytes (" <<
    ((total_number_of_samples*bytes_per_sample)>>30) << "GiB)" <<  std::endl;
    
    
    //
    // storage layout scheme:
    // [Pixel1: [Channel1: [Sample1 Sample2 Sample...] ] [Channel2: [Sample1 Sample2...] ] [Channnel...] ]
    // [Pixel2: [Channel1: [Sample1 Sample2 Sample...] ] [Channel2: [Sample1 Sample2...] ] [Channnel...] ]
    // [Pixel...]
    //
    storage.resize(total_number_of_samples*bytes_per_sample);
    

    
    Int64 write_pointer=0;
    
    for (int i = 0; i < height; i++)
    {
        //
        // Fill in data at the last minute.
        //
        
        
        for (int j = 0; j < width; j++)
        {
                for (int k = 0; k < channelCount; k++)
                {
                    data[k][i][j]=&storage[write_pointer];
                    if (channelTypes[k] == 0)
                        write_pointer+=sizeof(int)*sampleCount[i][j];
                    if (channelTypes[k] == 1)
                        write_pointer+=sizeof(half)*sampleCount[i][j];
                    if (channelTypes[k] == 2)
                        write_pointer+=sizeof(float)*sampleCount[i][j];
                    
                    if(random_channel_data)
                    {
                        for (unsigned int l = 0; l < sampleCount[i][j]; l++)
                        {
                            if (channelTypes[k] == 0)
                                ((unsigned int*)data[k][i][j])[l] = random_int();
                            if (channelTypes[k] == 1)
                                ((half*)data[k][i][j])[l] = random_float();
                            if (channelTypes[k] == 2)
                                ((float*)data[k][i][j])[l] = random_float();
                        }
                    }
                    else
                    {
                      for (unsigned int l = 0; l < sampleCount[i][j]; l++)
                      {
                          if (channelTypes[k] == 0)
                              ((unsigned int*)data[k][i][j])[l] = (i * width + j) % 2049;
                          if (channelTypes[k] == 1)
                              ((half*)data[k][i][j])[l] = (i * width + j) % 2049;
                          if (channelTypes[k] == 2)
                              ((float*)data[k][i][j])[l] = (i * width + j) % 2049;
                      }
                   }
                }
        
        }
    }

    cout << " data prepared, writing ...";

    file.writePixels(height);
    cout << " data written\n";
    
}

void
readFile (int channelCount, bool bulkRead, const std::string & fn)
{
    cout << "reading \n" << flush;

    DeepScanLineInputFile file (fn.c_str(), 8);

    const Header& fileHeader = file.header();
    assert (fileHeader.displayWindow() == header.displayWindow());
    assert (fileHeader.dataWindow() == header.dataWindow());
    assert (fileHeader.pixelAspectRatio() == header.pixelAspectRatio());
    assert (fileHeader.screenWindowCenter() == header.screenWindowCenter());
    assert (fileHeader.screenWindowWidth() == header.screenWindowWidth());
    assert (fileHeader.lineOrder() == header.lineOrder());
    assert (fileHeader.compression() == header.compression());
    assert (fileHeader.channels() == header.channels());
    assert (fileHeader.type() == header.type());

    Array2D<unsigned int> localSampleCount;
    localSampleCount.resizeErase(height, width);
    Array<Array2D< void* > > data(channelCount);
    for (int i = 0; i < channelCount; i++)
        data[i].resizeErase(height, width);
    
    DeepFrameBuffer frameBuffer;

    frameBuffer.insertSampleCountSlice (Slice (IMF::UINT,                    // type // 7
                                        (char *) (&localSampleCount[0][0]
                                                  - dataWindow.min.x
                                                  - dataWindow.min.y * width),               // base // 8)
                                        sizeof (unsigned int) * 1,          // xStride// 9
                                        sizeof (unsigned int) * width));    // yStride// 10

    Int64 bytes_per_sample=0;
    
    for (int i = 0; i < channelCount; i++)
    {
        PixelType type = NUM_PIXELTYPES;
        if (channelTypes[i] == 0)
            type = IMF::UINT;
        if (channelTypes[i] == 1)
            type = IMF::HALF;
        if (channelTypes[i] == 2)
            type = IMF::FLOAT;

        stringstream ss;
        ss << i;
        string str = ss.str();

        int sampleSize = 0;
        if (channelTypes[i] == 0) sampleSize = sizeof (unsigned int);
        if (channelTypes[i] == 1) sampleSize = sizeof (half);
        if (channelTypes[i] == 2) sampleSize = sizeof (float);

        int pointerSize = sizeof (char *);

        bytes_per_sample+=sampleSize;
        
        frameBuffer.insert (str,                            // name // 6
                            DeepSlice (type,                    // type // 7
                            (char *) (&data[i][0][0]
                                      - dataWindow.min.x
                                      - dataWindow.min.y * width),               // base // 8)
                            pointerSize * 1,          // xStride// 9
                            pointerSize * width,      // yStride// 10
                            sampleSize));             // sampleStride
    }

    file.setFrameBuffer(frameBuffer);

    file.readPixelSampleCounts(dataWindow.min.y, dataWindow.max.y);
    Int64 total_pixel_count = 0;
    
    for (int i = 0; i < dataWindow.max.y - dataWindow.min.y + 1; i++)
    {
         for (int j = 0; j < width; j++)
         {
              assert(localSampleCount[i][j] == sampleCount[i][j]);
              total_pixel_count += localSampleCount[i][j];
         }     
    }
    
    
    vector<char> localstorage(total_pixel_count*bytes_per_sample);
    
    Int64 write_pointer=0;
    
    for (int i = 0; i < height; i++)
    {
        //
        // Fill in data at the last minute.
        //
        
        
        for (int j = 0; j < width; j++)
        {
            for (int k = 0; k < channelCount; k++)
            {
                data[k][i][j]=&localstorage[write_pointer];
                if (channelTypes[k] == 0)
                    write_pointer+=sizeof(int)*sampleCount[i][j];
                if (channelTypes[k] == 1)
                    write_pointer+=sizeof(half)*sampleCount[i][j];
                if (channelTypes[k] == 2)
                    write_pointer+=sizeof(float)*sampleCount[i][j];
            }
        }
    }

    cout << "reading image data ... " << flush;
    
    file.readPixels(dataWindow.min.y, dataWindow.max.y);

    cout << " image read \n" << flush;
    
}

void
readWriteTest (int channelCount,
               int testTimes,
               bool random_channel_data,
               const std::string & fn)
{
    cout << "Testing files with " << channelCount << " channels " << testTimes << " times."
         << endl << flush;
    for (int i = 0; i < testTimes; i++)
    {
        int compressionIndex = i % 3;
        Compression compression;
        switch (compressionIndex)
        {
            case 0:
                compression = NO_COMPRESSION;
                break;
            case 1:
                compression = RLE_COMPRESSION;
                break;
            case 2:
                compression = ZIPS_COMPRESSION;
                break;
        }

        generateRandomFile (channelCount, compression, random_channel_data, fn);
        readFile (channelCount, false, fn);
        remove (fn.c_str());
     
    }
}

}; // namespace

void testDeepScanLineHuge (const std::string & tempDir)
{
    try
    {
        cout << "\n\nTesting the DeepScanLineInput/OutputFile for huge scanlines:\n" << endl;

        random_reseed(1);
        std::string fn = tempDir + "imf_test_deep_scanline_huge.exr";

        readWriteTest (10, 5 , false, fn);
        readWriteTest (10, 5 , true,  fn);
        cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
        cerr << "ERROR -- caught exception: " << e.what() << endl;
        assert (false);
    }
}
