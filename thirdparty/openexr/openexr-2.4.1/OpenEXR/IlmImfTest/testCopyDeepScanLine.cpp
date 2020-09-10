///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2011, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
//
// Portions (c) 2012, Weta Digital Ltd
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

#include "testCopyDeepScanLine.h"
#include "random.h"

#include <assert.h>
#include <string.h>

#include <ImfDeepScanLineInputFile.h>
#include <ImfDeepScanLineOutputFile.h>
#include <ImfPartType.h>
#include <ImfChannelList.h>
#include <ImfDeepFrameBuffer.h>
#include <ImfArray.h>
#include <IlmThreadPool.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include "tmpDir.h"

namespace IMF = OPENEXR_IMF_NAMESPACE;
using namespace IMF;
using namespace std;
using namespace IMATH_NAMESPACE;
using namespace ILMTHREAD_NAMESPACE;

namespace
{

const int width = 538;
const int height = 234;
const int minX = 42;
const int minY = 51;
const Box2i dataWindow(V2i(minX, minY), V2i(minX + width - 1, minY + height - 1));
const Box2i displayWindow(V2i(0, 0), V2i(minX + width * 2, minY + height * 2));

vector<int> channelTypes;
Array2D<unsigned int> sampleCount;
Header header;

void generateRandomFile (const std::string & source_filename,
                         int channelCount,
                         Compression compression)
{
    cout << "generating " << flush;
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

    remove (source_filename.c_str());
    DeepScanLineOutputFile file(source_filename.c_str(), header, 8);

    DeepFrameBuffer frameBuffer;

    frameBuffer.insertSampleCountSlice (Slice (IMF::UINT,                    // type // 7
                                        (char *) (&sampleCount[0][0]
                                                  - dataWindow.min.x
                                                  - dataWindow.min.y * width),               // base // 8
                                        sizeof (unsigned int) * 1,          // xStride// 9
                                        sizeof (unsigned int) * width));    // yStride// 10

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

    cout << "writing " << flush;
    for (int i = 0; i < height; i++)
    {
            //
            // Fill in data at the last minute.
            //

            for (int j = 0; j < width; j++)
            {
                sampleCount[i][j] = random_int(10) + 1;
                for (int k = 0; k < channelCount; k++)
                {
                    if (channelTypes[k] == 0)
                        data[k][i][j] = new unsigned int[sampleCount[i][j]];
                    if (channelTypes[k] == 1)
                        data[k][i][j] = new half[sampleCount[i][j]];
                    if (channelTypes[k] == 2)
                        data[k][i][j] = new float[sampleCount[i][j]];
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

        file.writePixels(height);
  

    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            for (int k = 0; k < channelCount; k++)
            {
                if (channelTypes[k] == 0)
                    delete[] (unsigned int*) data[k][i][j];
                if (channelTypes[k] == 1)
                    delete[] (half*) data[k][i][j];
                if (channelTypes[k] == 2)
                    delete[] (float*) data[k][i][j];
            }
}

void copyFile (const std::string & source_filename,
               const std::string & copy_filename)
{
    cout << "copying " ;
    cout.flush();
    {
       DeepScanLineInputFile in_file(source_filename.c_str(), 8);
       remove (copy_filename.c_str());
       DeepScanLineOutputFile out_file (copy_filename.c_str(),
                                        in_file.header(),
                                        8);
       out_file.copyPixels(in_file);
    }
    // remove the source file here to prevent accidentally reading it
  //  remove (source_filename);
    
}

void readFile (const std::string & copy_filename, int channelCount)
{
    cout << "reading " ;
    cout.flush();
    DeepScanLineInputFile file(copy_filename.c_str(), 8);

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

            frameBuffer.insert (str,                            // name // 6
                                DeepSlice (type,                // type // 7
                                (char *) (&data[i][0][0]
                                          - dataWindow.min.x
                                          - dataWindow.min.y * width),  // base // 8)
                                pointerSize * 1,          // xStride// 9
                                pointerSize * width,      // yStride// 10
                                sampleSize));             // sampleStride
	    
        
    }
    
    
    file.setFrameBuffer(frameBuffer);

        file.readPixelSampleCounts(dataWindow.min.y, dataWindow.max.y);
        for (int i = 0; i < dataWindow.max.y - dataWindow.min.y + 1; i++)
        {
            for (int j = 0; j < width; j++)
                assert(localSampleCount[i][j] == sampleCount[i][j]);

            for (int j = 0; j < width; j++)
            {
                for (int k = 0; k < channelCount; k++)
                {
                         if (channelTypes[k] == 0)
                              data[k][i][j] = new unsigned int[localSampleCount[i][j]];
                         if (channelTypes[k] == 1)
                              data[k][i][j] = new half[localSampleCount[i][j]];
                         if (channelTypes[k] == 2)
                             data[k][i][j] = new float[localSampleCount[i][j]];
                }
            }
        }
        
        file.readPixels(dataWindow.min.y, dataWindow.max.y);
    
    
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            for (int k = 0; k < channelCount; k++)
            {
                    for (unsigned int l = 0; l < sampleCount[i][j]; l++)
                    {
                        if (channelTypes[k] == 0)
                        {
                            unsigned int* value = (unsigned int*)(data[k][i][j]);
                            if (value[l] != static_cast<unsigned int>(i * width + j) % 2049)
                                cout << j << ", " << i << " error, should be "
                                     << (i * width + j) % 2049 << ", is " << value[l]
                                     << endl << flush;
                            assert (value[l] == static_cast<unsigned int>(i * width + j) % 2049);
                        }
                        if (channelTypes[k] == 1)
                        {
                            half* value = (half*)(data[k][i][j]);
                            if (value[l] != static_cast<unsigned int>(i * width + j) % 2049)
                                cout << j << ", " << i << " error, should be "
                                     << (i * width + j) % 2049 << ", is " << value[l]
                                     << endl << flush;
                            assert (((half*)(data[k][i][j]))[l] == (i * width + j) % 2049);
                        }
                        if (channelTypes[k] == 2)
                        {
                            float* value = (float*)(data[k][i][j]);
                            if (value[l] != (i * width + j) % 2049)
                                cout << j << ", " << i << " error, should be "
                                     << (i * width + j) % 2049 << ", is " << value[l]
                                     << endl << flush;
                            assert (((float*)(data[k][i][j]))[l] == (i * width + j) % 2049);
                        }
                    }
            }

    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            for (int k = 0; k < channelCount; k++)
            {
                    if (channelTypes[k] == 0)
                        delete[] (unsigned int*) data[k][i][j];
                    if (channelTypes[k] == 1)
                        delete[] (half*) data[k][i][j];
                    if (channelTypes[k] == 2)
                        delete[] (float*) data[k][i][j];
            }
}

void readCopyWriteTest (const std::string & tempDir,
                        int channelCount,
                        int testTimes)
{
    cout << "Testing files with " << channelCount << " channels " << testTimes << " times."
         << endl << flush;

    std::string source_filename = tempDir + "imf_test_copy_deep_scanline_source.exr";
    std::string copy_filename   = tempDir + "imf_test_copy_deep_scanline_copy.exr";

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

        generateRandomFile (source_filename, channelCount, compression);
        copyFile (source_filename, copy_filename);
        readFile (copy_filename, channelCount );
        remove (source_filename.c_str());
        remove (copy_filename.c_str());
        cout << endl << flush;

    }
}


}; // namespace

void testCopyDeepScanLine (const std::string &tempDir)
{
    try
    {
        cout << "\n\nTesting raw data copy in DeepScanLineInput/OutputFile:\n" << endl;

        random_reseed(1);

        int numThreads = ThreadPool::globalThreadPool().numThreads();
        ThreadPool::globalThreadPool().setNumThreads(4);

        
        readCopyWriteTest (tempDir,  1, 100);
        readCopyWriteTest (tempDir,  3,  50);
        readCopyWriteTest (tempDir, 10,  10);

        ThreadPool::globalThreadPool().setNumThreads(numThreads);

        cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
        cerr << "ERROR -- caught exception: " << e.what() << endl;
        assert (false);
    }
}
