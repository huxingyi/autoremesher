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

#include "testDeepTiledBasic.h"
#include "random.h"

#include <assert.h>
#include <string.h>

#include <ImfDeepTiledInputFile.h>
#include <ImfDeepTiledOutputFile.h>
#include <ImfChannelList.h>
#include <ImfArray.h>
#include <ImfPartType.h>
#include <IlmThreadPool.h>

#include <stdio.h>
#include <stdlib.h>
#include <vector>

namespace IMF = OPENEXR_IMF_NAMESPACE;
using namespace IMF;
using namespace IMATH_NAMESPACE;
using namespace ILMTHREAD_NAMESPACE;
using namespace std;

namespace
{

const int width = 273;
const int height = 169;
const int minX = 10;
const int minY = 11;
const Box2i dataWindow(V2i(minX, minY), V2i(minX + width - 1, minY + height - 1));
const Box2i displayWindow(V2i(0, 0), V2i(minX + width * 2, minY + height * 2));

vector<int> channelTypes;
Array2D< Array2D<unsigned int> > sampleCountWhole;
Header header;

void generateRandomFile (int channelCount,
                         Compression compression,
                         bool bulkWrite,
                         bool relativeCoords,
                         const std::string & filename)
{
    if (relativeCoords)
        assert(bulkWrite == false);

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

    header.setType(DEEPTILE);
    header.setTileDescription(
        TileDescription(random_int(width) + 1, random_int(height) + 1, RIPMAP_LEVELS));


    //
    // Set up the output file
    //
    remove (filename.c_str());
    DeepTiledOutputFile file(filename.c_str(), header, 8);

    DeepFrameBuffer frameBuffer;

    Array<Array2D< void* > > data(channelCount);
    for (int i = 0; i < channelCount; i++)
        data[i].resizeErase(height, width);

    Array2D<unsigned int> sampleCount;
    sampleCount.resizeErase(height, width);

    cout << " tileSizeX " << file.tileXSize()
         << " tileSizeY " << file.tileYSize() << " ";

    sampleCountWhole.resizeErase(file.numYLevels(), file.numXLevels());
    for (int i = 0; i < sampleCountWhole.height(); i++)
        for (int j = 0; j < sampleCountWhole.width(); j++)
            sampleCountWhole[i][j].resizeErase(height, width);

    int memOffset;
    if (relativeCoords)
        memOffset = 0;
    else
        memOffset = dataWindow.min.x + dataWindow.min.y * width;

    frameBuffer.insertSampleCountSlice (Slice (IMF::UINT,
                                        (char *) (&sampleCount[0][0] - memOffset),
                                        sizeof (unsigned int) * 1,
                                        sizeof (unsigned int) * width,
                                        1, 1,
                                        0,
                                        relativeCoords,
                                        relativeCoords));

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

        frameBuffer.insert (str,
                            DeepSlice (type,
                            (char *) (&data[i][0][0] - memOffset),
                            pointerSize * 1,
                            pointerSize * width,
                            sampleSize,
                            1, 1,
                            0,
                            relativeCoords,
                            relativeCoords));
    }

    file.setFrameBuffer(frameBuffer);

    cout << "writing " << flush;

    if (bulkWrite)
        cout << "bulk " << flush;
    else
    {
        if (relativeCoords == false)
            cout << "per-tile " << flush;
        else
            cout << "per-tile with relative coordinates " << flush;
    }

    for (int ly = 0; ly < file.numYLevels(); ly++)
        for (int lx = 0; lx < file.numXLevels(); lx++)
        {
            Box2i dataWindowL = file.dataWindowForLevel(lx, ly);

            if (bulkWrite)
            {
                //
                // Bulk write (without relative coordinates).
                //

                for (int j = 0; j < file.numYTiles(ly); j++)
                {
                    for (int i = 0; i < file.numXTiles(lx); i++)
                    {
                        Box2i box = file.dataWindowForTile(i, j, lx, ly);
                        for (int y = box.min.y; y <= box.max.y; y++)
                            for (int x = box.min.x; x <= box.max.x; x++)
                            {
                                int dwy = y - dataWindowL.min.y;
                                int dwx = x - dataWindowL.min.x;
                                sampleCount[dwy][dwx] = random_int(10) + 1;
                                sampleCountWhole[ly][lx][dwy][dwx] = sampleCount[dwy][dwx];
                                for (int k = 0; k < channelCount; k++)
                                {
                                    if (channelTypes[k] == 0)
                                        data[k][dwy][dwx] = new unsigned int[sampleCount[dwy][dwx]];
                                    if (channelTypes[k] == 1)
                                        data[k][dwy][dwx] = new half[sampleCount[dwy][dwx]];
                                    if (channelTypes[k] == 2)
                                        data[k][dwy][dwx] = new float[sampleCount[dwy][dwx]];
                                    for (unsigned int l = 0; l < sampleCount[dwy][dwx]; l++)
                                    {
                                        if (channelTypes[k] == 0)
                                            ((unsigned int*)data[k][dwy][dwx])[l] = (dwy * width + dwx) % 2049;
                                        if (channelTypes[k] == 1)
                                            ((half*)data[k][dwy][dwx])[l] = (dwy* width + dwx) % 2049;
                                        if (channelTypes[k] == 2)
                                            ((float*)data[k][dwy][dwx])[l] = (dwy * width + dwx) % 2049;
                                    }
                                }
                            }
                    }
                }

                file.writeTiles(0, file.numXTiles(lx) - 1, 0, file.numYTiles(ly) - 1, lx, ly);
            }
            else if (bulkWrite == false)
            {
                if (relativeCoords == false)
                {
                    //
                    // Per-tile write without relative coordinates.
                    //

                    for (int j = 0; j < file.numYTiles(ly); j++)
                    {
                        for (int i = 0; i < file.numXTiles(lx); i++)
                        {
                            Box2i box = file.dataWindowForTile(i, j, lx, ly);
                            for (int y = box.min.y; y <= box.max.y; y++)
                                for (int x = box.min.x; x <= box.max.x; x++)
                                {
                                    int dwy = y - dataWindowL.min.y;
                                    int dwx = x - dataWindowL.min.x;
                                    sampleCount[dwy][dwx] = random_int(10) + 1;
                                    sampleCountWhole[ly][lx][dwy][dwx] = sampleCount[dwy][dwx];
                                    for (int k = 0; k < channelCount; k++)
                                    {
                                        if (channelTypes[k] == 0)
                                            data[k][dwy][dwx] = new unsigned int[sampleCount[dwy][dwx]];
                                        if (channelTypes[k] == 1)
                                            data[k][dwy][dwx] = new half[sampleCount[dwy][dwx]];
                                        if (channelTypes[k] == 2)
                                            data[k][dwy][dwx] = new float[sampleCount[dwy][dwx]];
                                        for (unsigned int l = 0; l < sampleCount[dwy][dwx]; l++)
                                        {
                                            if (channelTypes[k] == 0)
                                                ((unsigned int*)data[k][dwy][dwx])[l] = (dwy * width + dwx) % 2049;
                                            if (channelTypes[k] == 1)
                                                ((half*)data[k][dwy][dwx])[l] = (dwy* width + dwx) % 2049;
                                            if (channelTypes[k] == 2)
                                                ((float*)data[k][dwy][dwx])[l] = (dwy * width + dwx) % 2049;
                                        }
                                    }
                                }
                            file.writeTile(i, j, lx, ly);
                        }
                    }
                }
                else if (relativeCoords)
                {
                    //
                    // Per-tile write with relative coordinates.
                    //

                    for (int j = 0; j < file.numYTiles(ly); j++)
                    {
                        for (int i = 0; i < file.numXTiles(lx); i++)
                        {
                            Box2i box = file.dataWindowForTile(i, j, lx, ly);
                            for (int y = box.min.y; y <= box.max.y; y++)
                                for (int x = box.min.x; x <= box.max.x; x++)
                                {
                                    int dwy = y - dataWindowL.min.y;
                                    int dwx = x - dataWindowL.min.x;
                                    int ty = y - box.min.y;
                                    int tx = x - box.min.x;
                                    sampleCount[ty][tx] = random_int(10) + 1;
                                    sampleCountWhole[ly][lx][dwy][dwx] = sampleCount[ty][tx];
                                    for (int k = 0; k < channelCount; k++)
                                    {
                                        if (channelTypes[k] == 0)
                                            data[k][ty][tx] = new unsigned int[sampleCount[ty][tx]];
                                        if (channelTypes[k] == 1)
                                            data[k][ty][tx] = new half[sampleCount[ty][tx]];
                                        if (channelTypes[k] == 2)
                                            data[k][ty][tx] = new float[sampleCount[ty][tx]];
                                        for (unsigned int l = 0; l < sampleCount[ty][tx]; l++)
                                        {
                                            if (channelTypes[k] == 0)
                                                ((unsigned int*)data[k][ty][tx])[l] =
                                                        (dwy * width + dwx) % 2049;
                                            if (channelTypes[k] == 1)
                                                ((half*)data[k][ty][tx])[l] =
                                                        (dwy * width + dwx) % 2049;
                                            if (channelTypes[k] == 2)
                                                ((float*)data[k][ty][tx])[l] =
                                                        (dwy * width + dwx) % 2049;
                                        }
                                    }
                                }
                            file.writeTile(i, j, lx, ly);

                            for (int y = box.min.y; y <= box.max.y; y++)
                                for (int x = box.min.x; x <= box.max.x; x++)
                                    for (int k = 0; k < channelCount; k++)
                                    {
                                        int ty = y - box.min.y;
                                        int tx = x - box.min.x;
                                        if (channelTypes[k] == 0)
                                            delete[] (unsigned int*) data[k][ty][tx];
                                        if (channelTypes[k] == 1)
                                            delete[] (half*) data[k][ty][tx];
                                        if (channelTypes[k] == 2)
                                            delete[] (float*) data[k][ty][tx];
                                    }
                        }
                    }
                }
            }

            if (relativeCoords == false)
            {
                for (int i = 0; i < file.levelHeight(ly); i++)
                    for (int j = 0; j < file.levelWidth(lx); j++)
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
        }
}

void checkValue (void* sampleRawData,
                 int sampleCount,
                 int channelType,
                 int dwx,
                 int dwy)
{
    for (int l = 0; l < sampleCount; l++)
    {
        if (channelType == 0)
        {
            unsigned int* value = (unsigned int*)(sampleRawData);
            if (value[l] != static_cast<unsigned int>((dwy * width + dwx) % 2049))
                cout << dwx << ", " << dwy << " error, should be "
                     << (dwy * width + dwx) % 2049 << ", is " << value[l]
                     << endl << flush;
            assert (value[l] == static_cast<unsigned int>((dwy * width + dwx) % 2049));
        }
        if (channelType == 1)
        {
            half* value = (half*)(sampleRawData);
            if (value[l] != (dwy * width + dwx) % 2049)
                cout << dwx << ", " << dwy << " error, should be "
                     << (dwy * width + dwx) % 2049 << ", is " << value[l]
                     << endl << flush;
            assert (value[l] == (dwy * width + dwx) % 2049);
        }
        if (channelType == 2)
        {
            float* value = (float*)(sampleRawData);
            if (value[l] != (dwy * width + dwx) % 2049)
                cout << dwx << ", " << dwy << " error, should be "
                     << (dwy * width + dwx) % 2049 << ", is " << value[l]
                     << endl << flush;
            assert (value[l] == (dwy * width + dwx) % 2049);
        }
    }
}

void readFile (int channelCount,
               bool bulkRead,
               bool relativeCoords,
               bool randomChannels,
               const std::string & filename)
{
    if (relativeCoords)
        assert(bulkRead == false);

    cout << "reading " << flush;

    DeepTiledInputFile file (filename.c_str(), 4);

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
    assert (fileHeader.tileDescription() == header.tileDescription());

    Array2D<unsigned int> localSampleCount;
    localSampleCount.resizeErase(height, width);
    
    // also test filling channels. Generate up to 2 extra channels
    int fillChannels=random_int(3);
    
    Array<Array2D< void* > > data(channelCount+fillChannels);
    for (int i = 0; i < channelCount+fillChannels; i++)
        data[i].resizeErase(height, width);

    DeepFrameBuffer frameBuffer;

    int memOffset;
    if (relativeCoords)
        memOffset = 0;
    else
        memOffset = dataWindow.min.x + dataWindow.min.y * width;
    frameBuffer.insertSampleCountSlice (Slice (IMF::UINT,
                                        (char *) (&localSampleCount[0][0] - memOffset),
                                        sizeof (unsigned int) * 1,
                                        sizeof (unsigned int) * width,
                                        1, 1,
                                        0,
                                        relativeCoords,
                                        relativeCoords));

    
    vector<int> read_channel(channelCount);
    int channels_added=0;
    for (int i = 0; i < channelCount; i++)
    {
         if(randomChannels)
        {
	     read_channel[i] = random_int(2);
	     
        }
        if(!randomChannels || read_channel[i]==1)
        {
            PixelType type = IMF::NUM_PIXELTYPES;
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

            frameBuffer.insert (str,
                                DeepSlice (type,
                                (char *) (&data[i][0][0] - memOffset),
                                pointerSize * 1,
                                pointerSize * width,
                                sampleSize,
                                1, 1,
                                0,
                                relativeCoords,
                                relativeCoords));
                channels_added++;
            }
    }
     if(channels_added==0)
    {
      cout << "skipping " <<flush;
      return;
    }
    for(int i = 0 ; i < fillChannels ; ++i )
    { 
        PixelType type  = IMF::FLOAT;
        int sampleSize = sizeof(float);            
        int pointerSize = sizeof (char *);
        stringstream ss;
        // generate channel names that aren't in file but (might) interleave with existing file
        ss << i << "fill";
        string str = ss.str();
        frameBuffer.insert (str,
                            DeepSlice (type,
                            (char *) (&data[i+channelCount][0][0] - memOffset),
                            pointerSize * 1,
                            pointerSize * width,
                            sampleSize,
                            1, 1,
                            0,
                            relativeCoords,
                            relativeCoords));
    }

    file.setFrameBuffer(frameBuffer);

    if (bulkRead)
        cout << "bulk " << flush;
    else
    {
        if (relativeCoords == false)
            cout << "per-tile " << flush;
        else
            cout << "per-tile with relative coordinates " << flush;
    }

    for (int ly = 0; ly < file.numYLevels(); ly++)
        for (int lx = 0; lx < file.numXLevels(); lx++)
        {
            Box2i dataWindowL = file.dataWindowForLevel(lx, ly);

            if (bulkRead)
            {
                //
                // Testing bulk read (without relative coordinates).
                //

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
                                assert(localSampleCount[dwy][dwx] == sampleCountWhole[ly][lx][dwy][dwx]);

                                for (size_t k = 0; k < channelTypes.size(); k++)
                                {
                                    if (channelTypes[k] == 0)
                                        data[k][dwy][dwx] = new unsigned int[localSampleCount[dwy][dwx]];
                                    if (channelTypes[k] == 1)
                                        data[k][dwy][dwx] = new half[localSampleCount[dwy][dwx]];
                                    if (channelTypes[k] == 2)
                                        data[k][dwy][dwx] = new float[localSampleCount[dwy][dwx]];
                                }
                                
                                for( int f = 0 ; f < fillChannels ; ++f )
                                {
                                     data[f + channelTypes.size()][dwy][dwx] = new float[localSampleCount[dwy][dwx]];
                                }
                                
                            }
                            
                    }
                }

                file.readTiles(0, file.numXTiles(lx) - 1, 0, file.numYTiles(ly) - 1, lx, ly);
            }
            else if (bulkRead == false)
            {
                if (relativeCoords == false)
                {
                    //
                    // Testing per-tile read without relative coordinates.
                    //

                    for (int i = 0; i < file.numYTiles(ly); i++)
                    {
                        for (int j = 0; j < file.numXTiles(lx); j++)
                        {
                            file.readPixelSampleCount(j, i, lx, ly);

                            Box2i box = file.dataWindowForTile(j, i, lx, ly);
                            for (int y = box.min.y; y <= box.max.y; y++)
                                for (int x = box.min.x; x <= box.max.x; x++)
                                {
                                    int dwy = y - dataWindowL.min.y;
                                    int dwx = x - dataWindowL.min.x;
                                    assert(localSampleCount[dwy][dwx] == sampleCountWhole[ly][lx][dwy][dwx]);

                                    for (size_t k = 0; k < channelTypes.size(); k++)
                                    {
                                        if (channelTypes[k] == 0)
                                            data[k][dwy][dwx] = new unsigned int[localSampleCount[dwy][dwx]];
                                        if (channelTypes[k] == 1)
                                            data[k][dwy][dwx] = new half[localSampleCount[dwy][dwx]];
                                        if (channelTypes[k] == 2)
                                            data[k][dwy][dwx] = new float[localSampleCount[dwy][dwx]];
                                    }
                                    for( int f = 0 ; f < fillChannels ; ++f )
                                    {
                                       data[f + channelTypes.size()][dwy][dwx] = new float[localSampleCount[dwy][dwx]];
                                    }
                                }

                            file.readTile(j, i, lx, ly);
                        }
                    }
                }
                else if (relativeCoords)
                {
                    //
                    // Testing per-tile read with relative coordinates.
                    //

                    for (int i = 0; i < file.numYTiles(ly); i++)
                    {
                        for (int j = 0; j < file.numXTiles(lx); j++)
                        {
                            file.readPixelSampleCount(j, i, lx, ly);

                            Box2i box = file.dataWindowForTile(j, i, lx, ly);
                            for (int y = box.min.y; y <= box.max.y; y++)
                                for (int x = box.min.x; x <= box.max.x; x++)
                                {
                                    int dwy = y - dataWindowL.min.y;
                                    int dwx = x - dataWindowL.min.x;
                                    int ty = y - box.min.y;
                                    int tx = x - box.min.x;
                                    assert(localSampleCount[ty][tx] == sampleCountWhole[ly][lx][dwy][dwx]);

                                    for (size_t k = 0; k < channelTypes.size(); k++)
                                    {
                                          if( !randomChannels || read_channel[k]==1)
                                          {
                                                if (channelTypes[k] == 0)
                                                    data[k][ty][tx] = new unsigned int[localSampleCount[ty][tx]];
                                                if (channelTypes[k] == 1)
                                                    data[k][ty][tx] = new half[localSampleCount[ty][tx]];
                                                if (channelTypes[k] == 2)
                                                    data[k][ty][tx] = new float[localSampleCount[ty][tx]];
                                          }
                                            
                                         
                                    }
                                    for( int f = 0 ; f < fillChannels ; ++f )
                                    {
                                        data[f + channelTypes.size()][ty][tx] = new float[localSampleCount[ty][tx]];
                                    }
                                }

                            file.readTile(j, i, lx, ly);

                            for (int y = box.min.y; y <= box.max.y; y++)
                                for (int x = box.min.x; x <= box.max.x; x++)
                                {
                                    int dwy = y - dataWindowL.min.y;
                                    int dwx = x - dataWindowL.min.x;
                                    int ty = y - box.min.y;
                                    int tx = x - box.min.x;

                                    for (size_t k = 0; k < channelTypes.size(); k++)
                                    {
                                         if( !randomChannels || read_channel[k]==1)
                                         {
                                                checkValue(data[k][ty][tx],
                                                        localSampleCount[ty][tx],
                                                        channelTypes[k],
                                                        dwx, dwy);
                                                if (channelTypes[k] == 0)
                                                    delete[] (unsigned int*) data[k][ty][tx];
                                                if (channelTypes[k] == 1)
                                                    delete[] (half*) data[k][ty][tx];
                                                if (channelTypes[k] == 2)
                                                    delete[] (float*) data[k][ty][tx];
                                         }
                                    }
                                    for( int f = 0 ; f < fillChannels ; ++f )
                                    {
                                         delete[] (float*) data[f+channelTypes.size()][ty][tx];
                                    }
                                }
                        }
                    }
                }
            }

            if (relativeCoords == false)
            {
                for (int i = 0; i < file.levelHeight(ly); i++)
                    for (int j = 0; j < file.levelWidth(lx); j++)
                        for (int k = 0; k < channelCount; k++)
                        {
                            if( !randomChannels || read_channel[k]==1)
                            {
                                for (unsigned int l = 0; l < localSampleCount[i][j]; l++)
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
                                        if (value[l] != (i * width + j) % 2049)
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
                        }

                for (int i = 0; i < file.levelHeight(ly); i++)
                    for (int j = 0; j < file.levelWidth(lx); j++)
                    {
                        for (int k = 0; k < channelCount; k++)
                        {
                                if (channelTypes[k] == 0)
                                    delete[] (unsigned int*) data[k][i][j];
                                if (channelTypes[k] == 1)
                                    delete[] (half*) data[k][i][j];
                                if (channelTypes[k] == 2)
                                    delete[] (float*) data[k][i][j];
                        }
                        for( int f = 0 ; f < fillChannels ; ++f )
                        {
                            delete[] (float*) data[f+channelTypes.size()][i][j];
                        }
                    }
            }
        }
}

void readWriteTestWithAbsoluateCoordinates (int channelCount,
                                            int testTimes,
                                            const std::string & tempDir)
{
    cout << "Testing files with " << channelCount
         << " channels, using absolute coordinates "
         << testTimes << " times."
         << endl << flush;

    std::string fn = tempDir + "imf_test_deep_tiled_basic.exr";

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

        generateRandomFile (channelCount, compression, false, false, fn);
        readFile (channelCount, false, false, false , fn);
        readFile (channelCount, false, false, true , fn);
        remove (fn.c_str());
        cout << endl << flush;

        generateRandomFile (channelCount, compression, true, false, fn);
        readFile (channelCount, true, false, false, fn);
        readFile (channelCount, true, false, true, fn);
        
        remove (fn.c_str());
        cout << endl << flush;

        generateRandomFile (channelCount, compression, false, true, fn);
        readFile (channelCount, false, true, false , fn);
        readFile (channelCount, false, true, true , fn);

        remove (fn.c_str());
        cout << endl << flush;
    }
}

} // namespace

void testDeepTiledBasic (const std::string & tempDir)
{
    try
    {
        cout << "Testing the DeepTiledInput/OutputFile for basic use" << endl;

        random_reseed(1);

        int numThreads = ThreadPool::globalThreadPool().numThreads();
        ThreadPool::globalThreadPool().setNumThreads(2);

        for(int pass = 0 ; pass < 4 ; pass++)
        {
           readWriteTestWithAbsoluateCoordinates ( 1, 2, tempDir);
           readWriteTestWithAbsoluateCoordinates ( 3, 2, tempDir);
           readWriteTestWithAbsoluateCoordinates (10, 2, tempDir);
        }
        ThreadPool::globalThreadPool().setNumThreads(numThreads);

        cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
        cerr << "ERROR -- caught exception: " << e.what() << endl;
        assert (false);
    }
}
