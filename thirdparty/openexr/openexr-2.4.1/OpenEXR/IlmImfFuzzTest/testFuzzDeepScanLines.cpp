///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2013, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC and Weta Digital Ltd
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

#include "fuzzFile.h"

#include <ImfDeepScanLineOutputFile.h>
#include <ImfDeepScanLineInputFile.h>
#include <ImfDeepFrameBuffer.h>
#include <ImfPartType.h>
#include <ImfArray.h>
#include <ImfThreading.h>
#include <IlmThread.h>
#include <Iex.h>
#include <iostream>
#include <cassert>
#include <stdio.h>
#include <vector>

#include "tmpDir.h"


// Handle the case when the custom namespace is not exposed
#include <ImfNamespace.h>
#include <ImfChannelList.h>
#include <ImfMultiPartOutputFile.h>
#include <ImfDeepScanLineOutputPart.h>
#include <ImfMultiPartInputFile.h>
#include <ImfDeepScanLineInputPart.h>

namespace IMF = OPENEXR_IMF_NAMESPACE;
using namespace IMF;
using namespace std;
using namespace IMATH_NAMESPACE;



namespace
{
    
const int width = 90;
const int height = 80;
const int minX = 10;
const int minY = 11;
const Box2i dataWindow(V2i(minX, minY), V2i(minX + width - 1, minY + height - 1));
const Box2i displayWindow(V2i(0, 0), V2i(minX + width * 2, minY + height * 2));

Array2D<unsigned int> sampleCount;

void generateRandomFile(const char filename[], int channelCount,int parts , Compression compression)
{
    cout << "generating file with " << parts << " parts and compression " << compression << flush;
    vector<Header> headers(parts);
    
    headers[0] = Header(displayWindow, dataWindow,
                    1,
                    IMATH_NAMESPACE::V2f (0, 0),
                    1,
                    INCREASING_Y,
                    compression);
                        
                    
                        
    for (int i = 0; i < channelCount; i++)
    {
        stringstream ss;
        ss << i;
        string str = ss.str();
        headers[0].channels().insert(str, Channel(IMF::FLOAT));
    }
                        
     headers[0].setType(DEEPSCANLINE);
            
     headers[0].setName("bob");
     
     for(int p=1;p<parts;p++)
     {
         headers[p]=headers[0];
         ostringstream s;
         s << p;
         headers[p].setName(s.str());
     }
     
     
     Array<Array2D< void* > > data(channelCount);
     for (int i = 0; i < channelCount; i++)
         data[i].resizeErase(height, width);
     
     sampleCount.resizeErase(height, width);
                        
     remove (filename);
     

     MultiPartOutputFile file(filename,&headers[0],parts);

     DeepFrameBuffer frameBuffer;
         
     frameBuffer.insertSampleCountSlice (Slice (IMF::UINT,        // type // 7
                                                (char *) (&sampleCount[0][0]
                                                - dataWindow.min.x
                                                - dataWindow.min.y * width),        // base // 8
                                                sizeof (unsigned int) * 1,          // xStride// 9
                                                sizeof (unsigned int) * width));    // yStride// 10
     
     for (int i = 0; i < channelCount; i++)
     {
         PixelType type = IMF::FLOAT;
         stringstream ss;
         ss << i;
         string str = ss.str();
         
         int sampleSize = sizeof (float);
                            
         int pointerSize = sizeof(char *);
         
         frameBuffer.insert (str,                                    // name // 6
                             DeepSlice (type,                        // type // 7
                                        (char *) (&data[i][0][0]
                                        - dataWindow.min.x
                                        - dataWindow.min.y * width), // base // 8
                                        pointerSize * 1,             // xStride// 9
                                        pointerSize * width,         // yStride// 10
                                        sampleSize));                // sampleStride
     }

    for(int p=0;p<parts;p++)
    {

        DeepScanLineOutputPart pt(file,p);
        pt.setFrameBuffer(frameBuffer);
                        
        cout << "writing " << p << flush;
        for (int i = 0; i < height; i++)
        {
            //
            // Fill in data at the last minute.
            //
            
            for (int j = 0; j < width; j++)
            {
                sampleCount[i][j] = rand() % 4 + 1;
                for (int k = 0; k < channelCount; k++)
                {
                    data[k][i][j] = new float[sampleCount[i][j]];
                    for (unsigned int l = 0; l < sampleCount[i][j]; l++)
                    {
                        ((float*)data[k][i][j])[l] = (i * width + j) % 2049;
                    }
                }
            }
        }
        
        pt.writePixels(height);
    }
}
    
void readFile(const char filename[])
{
    //single part interface to read file
    try{
        
        DeepScanLineInputFile file(filename, 8);
        
        
        const Header& fileHeader = file.header();
        
        int channelCount=0;
        for(ChannelList::ConstIterator i=fileHeader.channels().begin();i!=fileHeader.channels().end();++i,++channelCount);
        
        Array2D<unsigned int> localSampleCount;
        localSampleCount.resizeErase(height, width);
        Array<Array2D< void* > > data(channelCount);
        
        
        for (int i = 0; i < channelCount; i++)
            data[i].resizeErase(height, width);
        
        DeepFrameBuffer frameBuffer;
        
        frameBuffer.insertSampleCountSlice (Slice (IMF::UINT,        // type // 7
                                                   (char *) (&localSampleCount[0][0]
                                                   - dataWindow.min.x
                                                   - dataWindow.min.y * width),        // base // 8)
                                                   sizeof (unsigned int) * 1,          // xStride// 9
                                                   sizeof (unsigned int) * width));    // yStride// 10
        
        vector<int> read_channel(channelCount);
        
        
        for (int i = 0; i < channelCount; i++)
        {
            PixelType type = IMF::FLOAT;
            
            stringstream ss;
            ss << i;
            string str = ss.str();
            
            int sampleSize = sizeof (float);
            
            int pointerSize = sizeof (char *);
            
            frameBuffer.insert (str,                    
                                DeepSlice (type,        
                                           (char *) (&data[i][0][0]
                                           - dataWindow.min.x
                                           - dataWindow.min.y * width), // base // 8)
                                           pointerSize * 1,             // xStride// 9
                                           pointerSize * width,         // yStride// 10
                                           sampleSize));                // sampleStride
        }
        
        
        
        file.setFrameBuffer(frameBuffer);
        file.readPixelSampleCounts(dataWindow.min.y, dataWindow.max.y);
        for (int i = 0; i < dataWindow.max.y - dataWindow.min.y + 1; i++)
        {
            for (int j = 0; j < width; j++)
            {
                for (int k = 0; k < channelCount; k++)
                {
                    data[k][i][j] = new float[localSampleCount[i][j]];
                }
            }
            
        }
        
        try{
            file.readPixels(dataWindow.min.y, dataWindow.max.y);
        }catch(...)
        {
            // if readPixels excepts we must clean up
            assert (true);
        }
        
        for (int i = 0; i < height; i++)
            for (int j = 0; j < width; j++)
                for (int k = 0; k < channelCount; k++)
                {
                    delete[] (float*) data[k][i][j];
                }
                
    }catch(std::exception & e)
    {
        /* ... yeah, that's likely to happen a lot ... */
        assert (true);
    }
    
    
    try{
        
        MultiPartInputFile file(filename, 8);
    
    
        for(int p=0;p<file.parts();p++)
        {
            DeepScanLineInputPart inpart(file,p);
            const Header& fileHeader = inpart.header();
            
            int channelCount=0;
            for(ChannelList::ConstIterator i=fileHeader.channels().begin();i!=fileHeader.channels().end();++i,++channelCount);
            
            Array2D<unsigned int> localSampleCount;
            localSampleCount.resizeErase(height, width);
            Array<Array2D< void* > > data(channelCount);
            
            
            for (int i = 0; i < channelCount; i++)
                data[i].resizeErase(height, width);
            
            DeepFrameBuffer frameBuffer;
            
            frameBuffer.insertSampleCountSlice (Slice (IMF::UINT,        // type // 7
                                                       (char *) (&localSampleCount[0][0]
                                                       - dataWindow.min.x
                                                       - dataWindow.min.y * width),        // base // 8)
                                                       sizeof (unsigned int) * 1,          // xStride// 9
                                                       sizeof (unsigned int) * width));    // yStride// 10
            
            vector<int> read_channel(channelCount);
            
        
            for (int i = 0; i < channelCount; i++)
            {
                PixelType type = IMF::FLOAT;
                
                stringstream ss;
                ss << i;
                string str = ss.str();
                
                int sampleSize = sizeof (float);
                
                int pointerSize = sizeof (char *);
                
                frameBuffer.insert (str,                    
                                    DeepSlice (type,        
                                               (char *) (&data[i][0][0]
                                               - dataWindow.min.x
                                               - dataWindow.min.y * width), // base // 8)
                                               pointerSize * 1,             // xStride// 9
                                               pointerSize * width,         // yStride// 10
                                               sampleSize));                // sampleStride
            }
            
            inpart.setFrameBuffer(frameBuffer);
            inpart.readPixelSampleCounts(dataWindow.min.y, dataWindow.max.y);
            for (int i = 0; i < dataWindow.max.y - dataWindow.min.y + 1; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    for (int k = 0; k < channelCount; k++)
                    {
                        data[k][i][j] = new float[localSampleCount[i][j]];
                    }
                }
            }
            try{
                inpart.readPixels(dataWindow.min.y, dataWindow.max.y);
            }catch(...)
            {
                assert (true);
            }
    
            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    for (int k = 0; k < channelCount; k++)
                    {
                        delete[] (float*) data[k][i][j];
                    }
                }
            }
        }
    }catch(...)
    {
        // nothing
        assert (true);
    }
}


void
fuzzDeepScanLines (int numThreads, Rand48 &random)
{
    if (ILMTHREAD_NAMESPACE::supportsThreads())
    {
	setGlobalThreadCount (numThreads);
	cout << "\nnumber of threads: " << globalThreadCount() << endl;
    }

    Header::setMaxImageSize (10000, 10000);

    const char *goodFile = IMF_TMP_DIR "imf_test_deep_scanline_file_fuzz_good.exr";
    const char *brokenFile = IMF_TMP_DIR "imf_test_deep_scanline_file_fuzz_broken.exr";

    // read file if it already exists: allows re-testing reading of broken file
    readFile(brokenFile);
    
    for(int parts=1 ; parts < 3 ; parts++)
    {
        for(int comp_method=0;comp_method<2;comp_method++)
        {
            generateRandomFile(goodFile,8,parts,comp_method==0 ? NO_COMPRESSION : ZIPS_COMPRESSION);
            fuzzFile (goodFile, brokenFile, readFile, 5000, 3000, random);
        }
    }

    remove (goodFile);
    remove (brokenFile);
}

} // namespace


void
testFuzzDeepScanLines ()
{
    try
    {
	cout << "Testing deep scanline-based files "
		"with randomly inserted errors" << endl;

	Rand48 random (1);

	fuzzDeepScanLines (0, random);

	if (ILMTHREAD_NAMESPACE::supportsThreads())
	    fuzzDeepScanLines (2, random);

	cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
	cerr << "ERROR -- caught exception: " << e.what() << endl;
	assert (false);
    }
}
