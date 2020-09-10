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

#include <ImfDeepTiledOutputFile.h>
#include <ImfDeepTiledInputFile.h>
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
#include <sstream>

#include "tmpDir.h"


// Handle the case when the custom namespace is not exposed
#include <ImfNamespace.h>
#include <ImfChannelList.h>
#include <ImfMultiPartInputFile.h>
#include <ImfMultiPartOutputFile.h>
#include <ImfDeepTiledOutputPart.h>
#include <ImfDeepTiledInputPart.h>

namespace IMF = OPENEXR_IMF_NAMESPACE;
using namespace IMF;
using namespace std;
using namespace IMATH_NAMESPACE;


namespace
{
    
const int width = 127;
const int height = 46;
const int minX = 10;
const int minY = 11;
const Box2i dataWindow(V2i(minX, minY), V2i(minX + width - 1, minY + height - 1));
const Box2i displayWindow(V2i(0, 0), V2i(minX + width * 2, minY + height * 2));
Array2D< Array2D<unsigned int> > sampleCountWhole;
Header header;
    
void generateRandomFile(const char filename[], int channelCount, int parts , Compression compression)                            
{

    vector<Header> headers(parts);
    
    cout << "generating " << flush;
    headers[0] = Header(displayWindow, dataWindow,
                    1,
                    IMATH_NAMESPACE::V2f (0, 0),
                    1,
                    INCREASING_Y,
                    compression
                    ); 
    cout << "compression " << compression << " " << flush;
                    
    for (int i = 0; i < channelCount; i++)
    {
        ostringstream ss;
        ss << i;
        string str = ss.str();
        headers[0].channels().insert(str, Channel(IMF::FLOAT));
    }
       
    headers[0].setType(DEEPTILE);
    headers[0].setTileDescription( TileDescription(rand() % width + 1, rand() % height + 1, RIPMAP_LEVELS));
       
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
    {
        data[i].resizeErase(height, width);
    }
    
    Array2D<unsigned int> sampleCount;
    sampleCount.resizeErase(height, width);
       
    remove (filename);
    
    MultiPartOutputFile file(filename, &headers[0], parts);
    
    DeepTiledOutputPart part(file,0);
    
    cout << "tileSizeX " << part.tileXSize() << " tileSizeY " << part.tileYSize() << " ";
    
    sampleCountWhole.resizeErase(part.numYLevels(), part.numXLevels());
    for (int i = 0; i < sampleCountWhole.height(); i++)
    {
        for (int j = 0; j < sampleCountWhole.width(); j++)
        {
            sampleCountWhole[i][j].resizeErase(height, width);
        }
    }
    
    DeepFrameBuffer frameBuffer;
    
    int memOffset = dataWindow.min.x + dataWindow.min.y * width;
                                                
    frameBuffer.insertSampleCountSlice (Slice (IMF::UINT,
                                               (char *) (&sampleCount[0][0] - memOffset),
                                               sizeof (unsigned int) * 1,
                                               sizeof (unsigned int) * width) );
    for (int i = 0; i < channelCount; i++)
    {
        stringstream ss;
        ss << i;
        string str = ss.str();
        
        int sampleSize = sizeof (float);
        
        int pointerSize = sizeof (char *);
        
        frameBuffer.insert (str,
                            DeepSlice (IMF::FLOAT,
                                       (char *) (&data[i][0][0] - memOffset),
                                       pointerSize * 1,
                                       pointerSize * width,
                                       sampleSize));
    }
       
    for(int part=0;part<parts;part++)
    {
        DeepTiledOutputPart p(file,part);
        p.setFrameBuffer(frameBuffer);
                                                                                           
        cout << "writing " << flush;
                                                                                           
        for (int ly = 0; ly < p.numYLevels(); ly++)
        {
            for (int lx = 0; lx < p.numXLevels(); lx++)
            {
                Box2i dataWindowL = p.dataWindowForLevel(lx, ly);
                
                for (int j = 0; j < p.numYTiles(ly); j++)
                {
                    for (int i = 0; i < p.numXTiles(lx); i++)
                    {
                        Box2i box = p.dataWindowForTile(i, j, lx, ly);
                        for (int y = box.min.y; y <= box.max.y; y++)
                        {
                            for (int x = box.min.x; x <= box.max.x; x++)
                            {
                                int dwy = y - dataWindowL.min.y;
                                int dwx = x - dataWindowL.min.x;
                                sampleCount[dwy][dwx] = rand() % 5 + 1;
                                sampleCountWhole[ly][lx][dwy][dwx] = sampleCount[dwy][dwx];
                                for (int k = 0; k < channelCount; k++)
                                {
                                    data[k][dwy][dwx] = new float[sampleCount[dwy][dwx]];
                                    for (unsigned int l = 0; l < sampleCount[dwy][dwx]; l++)
                                    {
                                        ((float*)data[k][dwy][dwx])[l] = (dwy * width + dwx) % 2049;
                                    }
                                }
                            }
                        }
                    }
                }
                
                p.writeTiles(0, p.numXTiles(lx) - 1, 0, p.numYTiles(ly) - 1, lx, ly);
                    
                
                for(int k=0;k<data.size();k++)
                {
                    for(int y=0;y<data[k].height();y++)
                    {
                        for(int x=0;x<data[k].width();x++)
                        {
                            delete reinterpret_cast<char *>( data[k][y][x] );
                            data[k][y][x]=0;
                        }
                    }
                }
                
                
                
            }// next level
        }//next level
    }//next part
}
                            
void readFile(const char filename[])
{
                       
    try
    {
        DeepTiledInputFile file(filename, 8);
    
        const Header& fileHeader = file.header();
        
        Array2D<unsigned int> localSampleCount;
        
        Box2i dataWindow = fileHeader.dataWindow();
        
        int height = dataWindow.size().y+1;
        int width = dataWindow.size().x+1;
        
        
        localSampleCount.resizeErase(height, width);
        
        int channelCount=0;
        for(ChannelList::ConstIterator i=fileHeader.channels().begin();i!=fileHeader.channels().end();++i, channelCount++);
        
        Array<Array2D< void* > > data(channelCount);
        
        for (int i = 0; i < channelCount; i++)
        {
            data[i].resizeErase(height, width);
        }
        
        DeepFrameBuffer frameBuffer;
        
        int memOffset = dataWindow.min.x + dataWindow.min.y * width;
        frameBuffer.insertSampleCountSlice (Slice (IMF::UINT,
                                                   (char *) (&localSampleCount[0][0] - memOffset),
                                                   sizeof (unsigned int) * 1,
                                                   sizeof (unsigned int) * width)
                                                   );
                                                   
         for (int i = 0; i < channelCount; i++)
         {                              
             stringstream ss;
             ss << i;
             string str = ss.str();
             
             int sampleSize  = sizeof (float);
             
             int pointerSize = sizeof (char *);
             
             frameBuffer.insert (str,
                                 DeepSlice (IMF::FLOAT,
                                            (char *) (&data[i][0][0] - memOffset),
                                            pointerSize * 1,
                                            pointerSize * width,
                                            sampleSize) );
         }
         
         file.setFrameBuffer(frameBuffer);
         for (int ly = 0; ly < file.numYLevels(); ly++)
         {
             for (int lx = 0; lx < file.numXLevels(); lx++)
             {
                 Box2i dataWindowL = file.dataWindowForLevel(lx, ly);
                 
                 
                 file.readPixelSampleCounts(0, file.numXTiles(lx) - 1, 0, file.numYTiles(ly) - 1, lx, ly);
                 
                 for (int i = 0; i < file.numYTiles(ly); i++)
                 {
                     for (int j = 0; j < file.numXTiles(lx); j++)
                     {
                         Box2i box = file.dataWindowForTile(j, i, lx, ly);
                         for (int y = box.min.y; y <= box.max.y; y++)
                             for (int x = box.min.x; x <= box.max.x; x++)
                             {
                                 int dwy = y - dataWindowL.min.y;
                                 int dwx = x - dataWindowL.min.x;
                                 
                                 for (int k = 0; k < channelCount; k++)
                                 {
                                     data[k][dwy][dwx] = new float[localSampleCount[dwy][dwx]];
                                 }
                             }
                     }
                 }
                 
                 try{
                     
                     file.readTiles(0, file.numXTiles(lx) - 1, 0, file.numYTiles(ly) - 1, lx, ly);
                 }catch(...)
                 {
                     // catch exceptions thrown by readTiles, clean up anyway
                     assert (true);
                 }
                 for (int i = 0; i < file.levelHeight(ly); i++)
                 {
                     for (int j = 0; j < file.levelWidth(lx); j++)
                     {
                         for (int k = 0; k < channelCount; k++)
                         {
                             delete[] (float*) data[k][i][j];
                         }
                     }
                 }
             }
         }
         
    }catch(std::exception & e)
    {
        /* expect to get exceptions*/
        assert (true);
    }
    
    
    // test multipart inputfile interface
    
    try
    {
        MultiPartInputFile file(filename, 8);
        
        for(int p=0;p<file.parts();p++)
        {
            DeepTiledInputPart part(file,p);
            const Header& fileHeader = part.header();
        
            Array2D<unsigned int> localSampleCount;
        
            Box2i dataWindow = fileHeader.dataWindow();
            
            int height = dataWindow.size().y+1;
            int width = dataWindow.size().x+1;
        
        
            localSampleCount.resizeErase(height, width);
            
            int channelCount=0;
            for(ChannelList::ConstIterator i=fileHeader.channels().begin();i!=fileHeader.channels().end();++i, channelCount++);
            
            Array<Array2D< void* > > data(channelCount);
            
            for (int i = 0; i < channelCount; i++)
            {
                data[i].resizeErase(height, width);
            }
            
            DeepFrameBuffer frameBuffer;
        
            int memOffset = dataWindow.min.x + dataWindow.min.y * width;
            frameBuffer.insertSampleCountSlice (Slice (IMF::UINT,
                                                       (char *) (&localSampleCount[0][0] - memOffset),
                                                       sizeof (unsigned int) * 1,
                                                       sizeof (unsigned int) * width)
                                                       );
                                                       
            for (int i = 0; i < channelCount; i++)
            {                              
                stringstream ss;
                ss << i;
                string str = ss.str();
                
                int sampleSize  = sizeof (float);
                
                int pointerSize = sizeof (char *);
                
                frameBuffer.insert (str,
                                    DeepSlice (IMF::FLOAT,
                                               (char *) (&data[i][0][0] - memOffset),
                                               pointerSize * 1,
                                               pointerSize * width,
                                               sampleSize) );
            }
            
            part.setFrameBuffer(frameBuffer);
             
            for (int ly = 0; ly < part.numYLevels(); ly++)
            {
                for (int lx = 0; lx < part.numXLevels(); lx++)
                {
                    Box2i dataWindowL = part.dataWindowForLevel(lx, ly);
                    
                    
                    part.readPixelSampleCounts(0, part.numXTiles(lx) - 1, 0, part.numYTiles(ly) - 1, lx, ly);
                    
                    for (int i = 0; i <  part.numYTiles(ly); i++)
                    {
                        for (int j = 0; j < part.numXTiles(lx); j++)
                        {
                            Box2i box = part.dataWindowForTile(j, i, lx, ly);
                            for (int y = box.min.y; y <= box.max.y; y++)
                                for (int x = box.min.x; x <= box.max.x; x++)
                                {
                                    int dwy = y - dataWindowL.min.y;
                                    int dwx = x - dataWindowL.min.x;
                                    
                                    for (int k = 0; k < channelCount; k++)
                                    {
                                        data[k][dwy][dwx] = new float[localSampleCount[dwy][dwx]];
                                    }
                                }
                        }
                    }
                    
                    try{
                        part.readTiles(0, part.numXTiles(lx) - 1, 0, part.numYTiles(ly) - 1, lx, ly);
                    }catch(...)
                    {
                        assert (true);
                    }
                    
                    for (int i = 0; i < part.levelHeight(ly); i++)
                    {
                        for (int j = 0; j < part.levelWidth(lx); j++)
                        {
                            for (int k = 0; k < channelCount; k++)
                            {
                                delete[] (float*) data[k][i][j];
                            }
                        }
                    }
                }
            }
        }        
    }catch(std::exception & e)
    {
        /* expect to get exceptions*/
        assert (true);
    }
        
}


void
fuzzDeepTiles (int numThreads, Rand48 &random)
{
    
    
 
    
    if (ILMTHREAD_NAMESPACE::supportsThreads())
    {
	setGlobalThreadCount (numThreads);
	cout << "\nnumber of threads: " << globalThreadCount() << endl;
    }

    Header::setMaxImageSize (10000, 10000);

    const char *goodFile = IMF_TMP_DIR "imf_test_deep_tile_file_fuzz_good.exr";
    const char *brokenFile = IMF_TMP_DIR "imf_test_deel_tile_file_fuzz_broken.exr";

    
    // read file if it already exists: allows re-testing reading of broken file
    readFile(brokenFile);
    
    
    for(int parts=1;parts<3;parts++)
    {
    
        for(int comp_method=0;comp_method<2;comp_method++)
        {
            generateRandomFile(goodFile,8,parts , comp_method==0 ? NO_COMPRESSION : ZIPS_COMPRESSION);
            fuzzFile (goodFile, brokenFile, readFile, 5000, 3000, random);
        }
    }

    remove (goodFile);
    remove (brokenFile);
}

} // namespace


void
testFuzzDeepTiles ()
{
    
    
    try
    {
	cout << "Testing deep tile-based files "
		"with randomly inserted errors" << endl;

	Rand48 random (1);

	fuzzDeepTiles (0, random);

	if (ILMTHREAD_NAMESPACE::supportsThreads())
	    fuzzDeepTiles (2, random);

	cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
	cerr << "ERROR -- caught exception: " << e.what() << endl;
	assert (false);
    }
}
