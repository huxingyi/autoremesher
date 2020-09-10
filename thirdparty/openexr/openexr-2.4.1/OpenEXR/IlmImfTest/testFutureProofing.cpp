///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2011, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
//
// Portions (c) 2013, Weta Digital Ltd
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
//         Weta Digital nor any other ontributors may be used to endorse 
//         or promote products derived from this software without specific prior written permission.
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

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "bswap_32.h"

#include "tmpDir.h"
#include "testFutureProofing.h"
#include "testMultiPartFileMixingBasic.h"
#include "random.h"

#include <IlmThreadPool.h>
#include <ImfMultiPartInputFile.h>
#include <ImfMultiPartOutputFile.h>
#include <ImfArray.h>
#include <ImfChannelList.h>
#include <ImfOutputPart.h>
#include <ImfInputPart.h>
#include <ImfTiledOutputPart.h>
#include <ImfTiledInputPart.h>
#include <ImfDeepTiledOutputPart.h>
#include <ImfDeepScanLineOutputPart.h>
#include <ImfDeepTiledInputPart.h>
#include <ImfDeepScanLineInputPart.h>
#include <ImfPartType.h>
#include <ImfMisc.h>

#include <ImfNamespace.h>
#include <ImathNamespace.h>
#include <IlmThreadNamespace.h>
#include <ImfSystemSpecific.h>

namespace IMF = OPENEXR_IMF_NAMESPACE;
using namespace IMF;
using namespace std;
using namespace IMATH_NAMESPACE;
using namespace ILMTHREAD_NAMESPACE;

namespace
{
 
const int      height = 16;
const int      width = 16;
std::string    filename;
vector<Header> headers;
vector<int>    pixelTypes;
vector<int>    partTypes;
vector<int>    levelModes;

template <class T>
void
fillPixels (Array2D<T> &ph, int width, int height)
{
    ph.resizeErase(height, width);
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            //
            // We do this because half cannot store number bigger than 2048 exactly.
            //
            ph[y][x] = (y * width + x) % 2049;
        }
}

template <class T>
void
fillPixels (Array2D<unsigned int>& sampleCount, Array2D<T*> &ph, int width, int height)
{
    ph.resizeErase(height, width);
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            ph[y][x] = new T[sampleCount[y][x]];
            for (unsigned int i = 0; i < sampleCount[y][x]; i++)
            {
                //
                // We do this because half cannot store number bigger than 2048 exactly.
                //
                ph[y][x][i] = (y * width + x) % 2049;
            }
        }
}

void
allocatePixels (int type,
                Array2D<unsigned int>& sampleCount,
                Array2D<unsigned int*>& uintData,
                Array2D<float*>& floatData,
                Array2D<half*>& halfData,
                int x1, int x2, int y1, int y2)
{
    for (int y = y1; y <= y2; y++)
    {
        for (int x = x1; x <= x2; x++)
        {
            if (type == 0)
                uintData[y][x] = new unsigned int[sampleCount[y][x]];
            if (type == 1)
                floatData[y][x] = new float[sampleCount[y][x]];
            if (type == 2)
                halfData[y][x] = new half[sampleCount[y][x]];
        }
    }
}

void
allocatePixels (int type,
                Array2D<unsigned int>& sampleCount,
                Array2D<unsigned int*>& uintData,
                Array2D<float*>& floatData,
                Array2D<half*>& halfData,
                int width, int height)
{
    allocatePixels(type, sampleCount, uintData, floatData, halfData, 0, width - 1, 0, height - 1);
}

void
releasePixels (int type,
               Array2D<unsigned int*>& uintData,
               Array2D<float*>& floatData,
               Array2D<half*>& halfData,
               int x1, int x2, int y1, int y2)
{
    for (int y = y1; y <= y2; y++)
    {
        for (int x = x1; x <= x2; x++)
        {
            if (type == 0)
                delete[] uintData[y][x];
            if (type == 1)
                delete[] floatData[y][x];
            if (type == 2)
                delete[] halfData[y][x];
        }
    }
}

void
releasePixels (int type,
               Array2D<unsigned int*>& uintData,
               Array2D<float*>& floatData,
               Array2D<half*>& halfData,
               int width, int height)
{
    releasePixels(type, uintData, floatData, halfData, 0, width - 1, 0, height - 1);
}

template <class T>
bool
checkPixels (Array2D<T> &ph, int lx, int rx, int ly, int ry, int width)
{
    for (int y = ly; y <= ry; ++y)
    {
        for (int x = lx; x <= rx; ++x)
        {
            if (ph[y][x] != static_cast<T>(((y * width + x) % 2049)))
            {
                cout << "value at " << x << ", " << y << ": " << ph[y][x]
                     << ", should be " << (y * width + x) % 2049 << endl << flush;
                return false;
            }
        }
    }

    return true;
}

template <class T>
bool
checkPixels (Array2D<T> &ph, int width, int height)
{
    return checkPixels<T> (ph, 0, width - 1, 0, height - 1, width);
}

template <class T>
bool
checkPixels (Array2D<unsigned int>& sampleCount,
             Array2D<T*> &ph,
             int lx, int rx, int ly, int ry, int width)
{
    for (int y = ly; y <= ry; ++y)
    {
        for (int x = lx; x <= rx; ++x)
        {
            for (unsigned int i = 0; i < sampleCount[y][x]; i++)
            {
                if (ph[y][x][i] != static_cast<T>(((y * width + x) % 2049)))
                {
                    cout << "value at " << x << ", " << y << ", sample " << i << ": " << ph[y][x][i]
                         << ", should be " << (y * width + x) % 2049 << endl << flush;
                    return false;
                }
            }
        }
    }

    return true;
}

template <class T>
bool
checkPixels (Array2D<unsigned int>& sampleCount,
             Array2D<T*> &ph,
             int width, int height)
{
    return checkPixels<T> (sampleCount, ph, 0, width - 1, 0, height - 1, width);
}

bool
checkSampleCount (Array2D<unsigned int>& sampleCount,
                  int x1, int x2, int y1, int y2, int width)
{
    for (int i = y1; i <= y2; i++)
    {
        for (int j = x1; j <= x2; j++)
        {
            if (sampleCount[i][j] != static_cast<unsigned int>(((i * width) + j) % 10 + 1))
            {
                cout << "sample count at " << j << ", " << i << ": " << sampleCount[i][j]
                     << ", should be " << (i * width + j) % 10 + 1 << endl << flush;
                return false;
            }
        }
    }
    return true;
}

#if 0
bool
checkSampleCount (Array2D<unsigned int>& sampleCount, int width, int height)
{
    return checkSampleCount(sampleCount, 0, width - 1, 0, height - 1, width);
}
#endif

void
generateRandomHeaders (int partCount, vector<Header>& headers)
{
    cout << "Generating headers and data" << endl << flush;

    headers.clear();
    for (int i = 0; i < partCount; i++)
    {
        Header header (width, 
                       height,
                       1.f, 
                       IMATH_NAMESPACE::V2f (0, 0), 
                       1.f, 
                       INCREASING_Y, 
                       ZIPS_COMPRESSION);
                   
        int pixelType = random_int(3);
        int partType = random_int(4);
        
        pixelTypes[i] = pixelType;
        partTypes[i] = partType;

        stringstream ss;
        ss << i;
        header.setName(ss.str());

        switch (pixelType)
        {
            case 0:
                header.channels().insert("UINT", Channel(IMF::UINT));
                break;
            case 1:
                header.channels().insert("FLOAT", Channel(IMF::FLOAT));
                break;
            case 2:
                header.channels().insert("HALF", Channel(IMF::HALF));
                break;
        }

        switch (partType)
        {
            case 0:
                header.setType(SCANLINEIMAGE);
                break;
            case 1:
                header.setType(TILEDIMAGE);
                break;
            case 2:
                header.setType(DEEPSCANLINE);
                break;
            case 3:
                header.setType(DEEPTILE);
                break;
        }

        int tileX;
        int tileY;
        int levelMode;
        if (partType == 1 || partType == 3)
        {
            tileX = random_int(width) + 1;
            tileY = random_int(height) + 1;
            levelMode = random_int(3);
            levelModes[i] = levelMode;
            LevelMode lm  = NUM_LEVELMODES;
            switch (levelMode)
            {
                case 0:
                    lm = ONE_LEVEL;
                    break;
                case 1:
                    lm = MIPMAP_LEVELS;
                    break;
                case 2:
                    lm = RIPMAP_LEVELS;
                    break;
            }
            header.setTileDescription(TileDescription(tileX, tileY, lm));
        }

 
        int order = random_int(NUM_LINEORDERS);
        if(partType==0 || partType ==2)
        {
            // can't write random scanlines
            order = random_int(NUM_LINEORDERS-1);
        }
        LineOrder l = NUM_LINEORDERS;
        switch(order)
        {
             case 0 : 
                 l = INCREASING_Y;
                 break;
             case 1 :
                  l = DECREASING_Y;
                 break;
             case 2 : 
                  l = RANDOM_Y;
                  break;
        }
        
        header.lineOrder()=l;
        

        if (partType == 0 || partType == 2)
        {
            cout << "pixelType = " << pixelType << " partType = " << partType
                 << " line order =" << header.lineOrder() << endl << flush;
        }
        else
        {
            cout << "pixelType = " << pixelType << " partType = " << partType
                 << " tile order =" << header.lineOrder()
                 << " levelMode = " << levelModes[i] << endl << flush;
        }
        // future types MUST have a chunkCount attribute - ommitting causes the library
        // to raise an exception (can't compute chunkOffsetTable) and prevents us from reading
        // the rest of the image
        header.setChunkCount(getChunkOffsetTableSize(header,true));
        headers.push_back(header);
    }
}

void
setOutputFrameBuffer (FrameBuffer& frameBuffer,
                      int pixelType,
                      Array2D<unsigned int>& uData,
                      Array2D<float>& fData,
                      Array2D<half>& hData,
                      int width)
{
    switch (pixelType)
    {
        case 0:
            frameBuffer.insert ("UINT",
                                Slice (IMF::UINT,
                                (char *) (&uData[0][0]),
                                sizeof (uData[0][0]) * 1,
                                sizeof (uData[0][0]) * width));
            break;
        case 1:
            frameBuffer.insert ("FLOAT",
                                Slice (IMF::FLOAT,
                                (char *) (&fData[0][0]),
                                sizeof (fData[0][0]) * 1,
                                sizeof (fData[0][0]) * width));
            break;
        case 2:
            frameBuffer.insert ("HALF",
                                Slice (IMF::HALF,
                                (char *) (&hData[0][0]),
                                sizeof (hData[0][0]) * 1,
                                sizeof (hData[0][0]) * width));
            break;
    }
}

void
setOutputDeepFrameBuffer (DeepFrameBuffer& frameBuffer,
                          int pixelType,
                          Array2D<unsigned int*>& uData,
                          Array2D<float*>& fData,
                          Array2D<half*>& hData,
                          int width)
{
    switch (pixelType)
    {
        case 0:
            frameBuffer.insert ("UINT",
                                DeepSlice (IMF::UINT,
                                (char *) (&uData[0][0]),
                                sizeof (uData[0][0]) * 1,
                                sizeof (uData[0][0]) * width,
                                sizeof (unsigned int)));
            break;
        case 1:
            frameBuffer.insert ("FLOAT",
                                DeepSlice (IMF::FLOAT,
                                (char *) (&fData[0][0]),
                                sizeof (fData[0][0]) * 1,
                                sizeof (fData[0][0]) * width,
                                sizeof (float)));
            break;
        case 2:
            frameBuffer.insert ("HALF",
                                DeepSlice (IMF::HALF,
                                (char *) (&hData[0][0]),
                                sizeof (hData[0][0]) * 1,
                                sizeof (hData[0][0]) * width,
                                sizeof (half)));
            break;
    }
}

void
setInputFrameBuffer (FrameBuffer& frameBuffer,
                     int pixelType,
                     Array2D<unsigned int>& uData,
                     Array2D<float>& fData,
                     Array2D<half>& hData,
                     int width, int height)
{
    switch (pixelType)
    {
        case 0:
            uData.resizeErase(height, width);
            frameBuffer.insert ("UINT",
                                Slice (IMF::UINT,
                                (char *) (&uData[0][0]),
                                sizeof (uData[0][0]) * 1,
                                sizeof (uData[0][0]) * width,
                                1, 1,
                                0));
            break;
        case 1:
            fData.resizeErase(height, width);
            frameBuffer.insert ("FLOAT",
                                Slice (IMF::FLOAT,
                                (char *) (&fData[0][0]),
                                sizeof (fData[0][0]) * 1,
                                sizeof (fData[0][0]) * width,
                                1, 1,
                                0));
            break;
        case 2:
            hData.resizeErase(height, width);
            frameBuffer.insert ("HALF",
                                Slice (IMF::HALF,
                                (char *) (&hData[0][0]),
                                sizeof (hData[0][0]) * 1,
                                sizeof (hData[0][0]) * width,
                                1, 1,
                                0));
            break;
    }
}

void
setInputDeepFrameBuffer (DeepFrameBuffer& frameBuffer,
                         int pixelType,
                         Array2D<unsigned int*>& uData,
                         Array2D<float*>& fData,
                         Array2D<half*>& hData,
                         int width, int height)
{
    switch (pixelType)
    {
        case 0:
            uData.resizeErase(height, width);
            frameBuffer.insert ("UINT",
                                DeepSlice (IMF::UINT,
                                (char *) (&uData[0][0]),
                                sizeof (uData[0][0]) * 1,
                                sizeof (uData[0][0]) * width,
                                sizeof (unsigned int)));
            break;
        case 1:
            fData.resizeErase(height, width);
            frameBuffer.insert ("FLOAT",
                                DeepSlice (IMF::FLOAT,
                                (char *) (&fData[0][0]),
                                sizeof (fData[0][0]) * 1,
                                sizeof (fData[0][0]) * width,
                                sizeof (float)));
            break;
        case 2:
            hData.resizeErase(height, width);
            frameBuffer.insert ("HALF",
                                DeepSlice (IMF::HALF,
                                (char *) (&hData[0][0]),
                                sizeof (hData[0][0]) * 1,
                                sizeof (hData[0][0]) * width,
                                sizeof (half)));
            break;
    }
}

void
generateRandomFile (int partCount)
{
    //
    // Init data.
    //
    Array2D<half> halfData;
    Array2D<float> floatData;
    Array2D<unsigned int> uintData;

    Array2D<unsigned int> sampleCount;
    Array2D<half*> deepHalfData;
    Array2D<float*> deepFloatData;
    Array2D<unsigned int*> deepUintData;

    vector<GenericOutputFile*> outputfiles;

    pixelTypes.resize(partCount);
    partTypes.resize(partCount);
    levelModes.resize(partCount);

    //
    // Generate headers and data.
    //
    generateRandomHeaders(partCount, headers);

    remove(filename.c_str());
    MultiPartOutputFile file(filename.c_str(), &headers[0],headers.size());

    //
    // Writing files.
    //
    cout << "Writing files " << flush;

    //
    // Pre-generating frameBuffers.
    //
    for (int i = 0; i < partCount; i++)
    {
        switch (partTypes[i])
        {
            case 0:
            {
                OutputPart part(file, i);

                FrameBuffer frameBuffer;

                fillPixels <unsigned int> (uintData, width, height);
                fillPixels <float> (floatData, width, height);
                fillPixels <half> (halfData, width, height);

                setOutputFrameBuffer(frameBuffer, pixelTypes[i], uintData, floatData, halfData, width);

                part.setFrameBuffer(frameBuffer);

                part.writePixels(height);

                break;
            }
            case 1:
            {
                TiledOutputPart part(file, i);

                int numXLevels = part.numXLevels();
                int numYLevels = part.numYLevels();

                for (int xLevel = 0; xLevel < numXLevels; xLevel++)
                    for (int yLevel = 0; yLevel < numYLevels; yLevel++)
                    {
                        if (!part.isValidLevel(xLevel, yLevel))
                            continue;

                        int w = part.levelWidth(xLevel);
                        int h = part.levelHeight(yLevel);

                        FrameBuffer frameBuffer;

                        fillPixels <unsigned int> (uintData, w, h);
                        fillPixels <float> (floatData, w, h);
                        fillPixels <half> (halfData, w, h);
                        setOutputFrameBuffer(frameBuffer, pixelTypes[i],
                                             uintData, floatData, halfData,
                                             w);

                        part.setFrameBuffer(frameBuffer);

                        part.writeTiles(0, part.numXTiles(xLevel) - 1,
                                        0, part.numYTiles(yLevel) - 1,
                                        xLevel, yLevel);
                    }

                break;
            }
            case 2:
            {
                DeepScanLineOutputPart part(file, i);

                DeepFrameBuffer frameBuffer;

                sampleCount.resizeErase(height, width);
                for (int j = 0; j < height; j++)
                    for (int k = 0; k < width; k++)
                        sampleCount[j][k] = (j * width + k) % 10 + 1;

                frameBuffer.insertSampleCountSlice (Slice (IMF::UINT,
                                                    (char *) (&sampleCount[0][0]),
                                                    sizeof (unsigned int) * 1,
                                                    sizeof (unsigned int) * width));

                if (pixelTypes[i] == 0)
                    fillPixels <unsigned int> (sampleCount, deepUintData, width, height);
                if (pixelTypes[i] == 1)
                    fillPixels <float> (sampleCount, deepFloatData, width, height);
                if (pixelTypes[i] == 2)
                    fillPixels <half> (sampleCount, deepHalfData, width, height);
                setOutputDeepFrameBuffer(frameBuffer, pixelTypes[i],
                                         deepUintData, deepFloatData, deepHalfData,
                                         width);

                part.setFrameBuffer(frameBuffer);

                part.writePixels(height);

                releasePixels(pixelTypes[i], deepUintData, deepFloatData, deepHalfData, width, height);

                break;
            }
            case 3:
            {
                DeepTiledOutputPart part(file, i);

                int numXLevels = part.numXLevels();
                int numYLevels = part.numYLevels();

                for (int xLevel = 0; xLevel < numXLevels; xLevel++)
                    for (int yLevel = 0; yLevel < numYLevels; yLevel++)
                    {
                        if (!part.isValidLevel(xLevel, yLevel))
                            continue;

                        int w = part.levelWidth(xLevel);
                        int h = part.levelHeight(yLevel);

                        DeepFrameBuffer frameBuffer;

                        sampleCount.resizeErase(h, w);
                        for (int j = 0; j < h; j++)
                            for (int k = 0; k < w; k++)
                                sampleCount[j][k] = (j * w + k) % 10 + 1;

                        frameBuffer.insertSampleCountSlice (Slice (IMF::UINT,
                                                            (char *) (&sampleCount[0][0]),
                                                            sizeof (unsigned int) * 1,
                                                            sizeof (unsigned int) * w));

                        if (pixelTypes[i] == 0)
                            fillPixels <unsigned int> (sampleCount, deepUintData, w, h);
                        if (pixelTypes[i] == 1)
                            fillPixels <float> (sampleCount, deepFloatData, w, h);
                        if (pixelTypes[i] == 2)
                            fillPixels <half> (sampleCount, deepHalfData, w, h);
                        setOutputDeepFrameBuffer(frameBuffer, pixelTypes[i],
                                                 deepUintData, deepFloatData, deepHalfData,
                                                 w);

                        part.setFrameBuffer(frameBuffer);

                        part.writeTiles(0, part.numXTiles(xLevel) - 1,
                                        0, part.numYTiles(yLevel) - 1,
                                        xLevel, yLevel);

                        releasePixels(pixelTypes[i], deepUintData, deepFloatData, deepHalfData, w, h);
                    }

                break;
            }
        }
    }
}

void
readWholeFiles (int modification)
{
    Array2D<unsigned int> uData;
    Array2D<float> fData;
    Array2D<half> hData;

    Array2D<unsigned int*> deepUData;
    Array2D<float*> deepFData;
    Array2D<half*> deepHData;

    Array2D<unsigned int> sampleCount;

    MultiPartInputFile file(filename.c_str());
    for (int i = 0; i < file.parts(); i++)
    {
        const Header& header = file.header(i);
        assert (header.displayWindow() == headers[i].displayWindow());
        assert (header.dataWindow() == headers[i].dataWindow());
        assert (header.pixelAspectRatio() == headers[i].pixelAspectRatio());
        assert (header.screenWindowCenter() == headers[i].screenWindowCenter());
        assert (header.screenWindowWidth() == headers[i].screenWindowWidth());
        assert (header.lineOrder() == headers[i].lineOrder());
        assert (header.compression() == headers[i].compression());
        assert (header.channels() == headers[i].channels());
        assert (header.name() == headers[i].name());
        if(modification==1 && i==0)
        {
            assert (header.type() != headers[i].type());
        }else{
            assert (header.type() == headers[i].type());
        }
    }

    cout << "Reading whole files " << flush;

    
    
    //
    // Shuffle part numbers.
    //
    vector<int> shuffledPartNumber;
    for (int i = modification>0 ? 1 : 0; i < static_cast<int>(headers.size()); i++)
        shuffledPartNumber.push_back(i);
    for (size_t i = 0; i < shuffledPartNumber.size(); i++)
    {
        size_t a = random_int(shuffledPartNumber.size());
        size_t b = random_int(shuffledPartNumber.size());
        swap (shuffledPartNumber[a], shuffledPartNumber[b]);
    }


   

    //
    // Start reading whole files.
    //
    int partNumber;
    try
    {
        for (size_t i = 0; i < shuffledPartNumber.size(); i++)
        {
            partNumber = shuffledPartNumber[i];
            switch (partTypes[partNumber])
            {
                case 0:
                {
                    FrameBuffer frameBuffer;
                    setInputFrameBuffer(frameBuffer, pixelTypes[partNumber],
                                        uData, fData, hData, width, height);

                    InputPart part(file, partNumber);
                    part.setFrameBuffer(frameBuffer);
                    part.readPixels(0, height - 1);
                    switch (pixelTypes[partNumber])
                    {
                        case 0:
                            assert(checkPixels<unsigned int>(uData, width, height));
                            break;
                        case 1:
                            assert(checkPixels<float>(fData, width, height));
                            break;
                        case 2:
                            assert(checkPixels<half>(hData, width, height));
                            break;
                    }
                    break;
                }
                case 1:
                {
                    TiledInputPart part(file, partNumber);
                    int numXLevels = part.numXLevels();
                    int numYLevels = part.numYLevels();
                    for (int xLevel = 0; xLevel < numXLevels; xLevel++)
                        for (int yLevel = 0; yLevel < numYLevels; yLevel++)
                        {
                            if (!part.isValidLevel(xLevel, yLevel))
                                continue;

                            int w = part.levelWidth(xLevel);
                            int h = part.levelHeight(yLevel);

                            FrameBuffer frameBuffer;
                            setInputFrameBuffer(frameBuffer, pixelTypes[partNumber],
                                                uData, fData, hData, w, h);

                            part.setFrameBuffer(frameBuffer);
                            int numXTiles = part.numXTiles(xLevel);
                            int numYTiles = part.numYTiles(yLevel);
                            part.readTiles(0, numXTiles - 1, 0, numYTiles - 1, xLevel, yLevel);
                            switch (pixelTypes[partNumber])
                            {
                                case 0:
                                    assert(checkPixels<unsigned int>(uData, w, h));
                                    break;
                                case 1:
                                    assert(checkPixels<float>(fData, w, h));
                                    break;
                                case 2:
                                    assert(checkPixels<half>(hData, w, h));
                                    break;
                            }
                        }
                    break;
                }
                case 2:
                {
                    DeepScanLineInputPart part(file, partNumber);

                    DeepFrameBuffer frameBuffer;

                    sampleCount.resizeErase(height, width);
                    frameBuffer.insertSampleCountSlice (Slice (IMF::UINT,
                                                        (char *) (&sampleCount[0][0]),
                                                        sizeof (unsigned int) * 1,
                                                        sizeof (unsigned int) * width));

                    setInputDeepFrameBuffer(frameBuffer, pixelTypes[partNumber],
                                            deepUData, deepFData, deepHData, width, height);

                    part.setFrameBuffer(frameBuffer);

                    part.readPixelSampleCounts(0, height - 1);

                    allocatePixels(pixelTypes[partNumber], sampleCount,
                                   deepUData, deepFData, deepHData, width, height);

                    part.readPixels(0, height - 1);
                    switch (pixelTypes[partNumber])
                    {
                        case 0:
                            assert(checkPixels<unsigned int>(sampleCount, deepUData, width, height));
                            break;
                        case 1:
                            assert(checkPixels<float>(sampleCount, deepFData, width, height));
                            break;
                        case 2:
                            assert(checkPixels<half>(sampleCount, deepHData, width, height));
                            break;
                    }

                    releasePixels(pixelTypes[partNumber],
                                  deepUData, deepFData, deepHData, width, height);

                    break;
                }
                case 3:
                {
                    DeepTiledInputPart part(file, partNumber);
                    int numXLevels = part.numXLevels();
                    int numYLevels = part.numYLevels();
                    for (int xLevel = 0; xLevel < numXLevels; xLevel++)
                        for (int yLevel = 0; yLevel < numYLevels; yLevel++)
                        {
                            if (!part.isValidLevel(xLevel, yLevel))
                                continue;

                            int w = part.levelWidth(xLevel);
                            int h = part.levelHeight(yLevel);

                            DeepFrameBuffer frameBuffer;

                            sampleCount.resizeErase(h, w);
                            frameBuffer.insertSampleCountSlice (Slice (IMF::UINT,
                                                                (char *) (&sampleCount[0][0]),
                                                                sizeof (unsigned int) * 1,
                                                                sizeof (unsigned int) * w));

                            setInputDeepFrameBuffer(frameBuffer, pixelTypes[partNumber],
                                                    deepUData, deepFData, deepHData, w, h);

                            part.setFrameBuffer(frameBuffer);

                            int numXTiles = part.numXTiles(xLevel);
                            int numYTiles = part.numYTiles(yLevel);

                            part.readPixelSampleCounts(0, numXTiles - 1, 0, numYTiles - 1,
                                                       xLevel, yLevel);

                            allocatePixels(pixelTypes[partNumber], sampleCount,
                                           deepUData, deepFData, deepHData, w, h);

                            part.readTiles(0, numXTiles - 1, 0, numYTiles - 1, xLevel, yLevel);
                            switch (pixelTypes[partNumber])
                            {
                                case 0:
                                    assert(checkPixels<unsigned int>(sampleCount, deepUData, w, h));
                                    break;
                                case 1:
                                    assert(checkPixels<float>(sampleCount, deepFData, w, h));
                                    break;
                                case 2:
                                    assert(checkPixels<half>(sampleCount, deepHData, w, h));
                                    break;
                            }

                            releasePixels(pixelTypes[partNumber],
                                          deepUData, deepFData, deepHData, w, h);
                        }

                    break;
                }
            }
            cerr << "part " << partNumber << " ok ";
            
        }
    }
    catch (...)
    {
        cout << "Error while reading part " << partNumber << endl << flush;
        throw;
    }
}

void
readFirstPart()
{
    Array2D<unsigned int> uData;
    Array2D<float> fData;
    Array2D<half> hData;
    
    Array2D<unsigned int*> deepUData;
    Array2D<float*> deepFData;
    Array2D<half*> deepHData;
    
    Array2D<unsigned int> sampleCount;
    
    cout << "Reading first part " << flush;
    int pixelType = pixelTypes[0];
    int partType = partTypes[0];
    int levelMode = levelModes[0];
    switch (partType)
    {
        case 0:
        {
        int l1, l2;
        l1 = random_int(height);
        l2 = random_int(height);
        if (l1 > l2) swap(l1, l2);

        InputFile part(filename.c_str());

        FrameBuffer frameBuffer;
        setInputFrameBuffer(frameBuffer, pixelType,
                            uData, fData, hData, width, height);

        part.setFrameBuffer(frameBuffer);
        part.readPixels(l1, l2);

        switch (pixelType)
        {
        case 0:
            assert(checkPixels<unsigned int>(uData, 0, width - 1, l1, l2, width));
            break;
        case 1:
            assert(checkPixels<float>(fData, 0, width - 1, l1, l2, width));
            break;
        case 2:
            assert(checkPixels<half>(hData, 0, width - 1, l1, l2, width));
            break;
        }

        break;
    }
    case 1:
    {
        int tx1, tx2, ty1, ty2;
        int lx, ly;

        TiledInputFile part(filename.c_str());

        int numXLevels = part.numXLevels();
        int numYLevels = part.numYLevels();

        lx = random_int(numXLevels);
        ly = random_int(numYLevels);
        if (levelMode == 1) ly = lx;

        int w = part.levelWidth(lx);
        int h = part.levelHeight(ly);

        int numXTiles = part.numXTiles(lx);
        int numYTiles = part.numYTiles(ly);
        tx1 = random_int(numXTiles);
        tx2 = random_int(numXTiles);
        ty1 = random_int(numYTiles);
        ty2 = random_int(numYTiles);
        if (tx1 > tx2) swap(tx1, tx2);
        if (ty1 > ty2) swap(ty1, ty2);

        FrameBuffer frameBuffer;
        setInputFrameBuffer(frameBuffer, pixelType,
                            uData, fData, hData, w, h);

        part.setFrameBuffer(frameBuffer);
        part.readTiles(tx1, tx2, ty1, ty2, lx, ly);

        Box2i b1 = part.dataWindowForTile(tx1, ty1, lx, ly);
        Box2i b2 = part.dataWindowForTile(tx2, ty2, lx, ly);

        switch (pixelType)
        {
        case 0:
            assert(checkPixels<unsigned int>(uData, b1.min.x, b2.max.x, b1.min.y, b2.max.y,
                                             w));
            break;
        case 1:
            assert(checkPixels<float>(fData, b1.min.x, b2.max.x, b1.min.y, b2.max.y,
                                      w));
            break;
        case 2:
            assert(checkPixels<half>(hData, b1.min.x, b2.max.x, b1.min.y, b2.max.y,
                                     w));
            break;
        }

        break;
    }
    case 2:
    {
        DeepScanLineInputFile part(filename.c_str());

        DeepFrameBuffer frameBuffer;

        sampleCount.resizeErase(height, width);
        frameBuffer.insertSampleCountSlice (Slice (IMF::UINT,
                                            (char *) (&sampleCount[0][0]),
                                            sizeof (unsigned int) * 1,
                                            sizeof (unsigned int) * width));

        setInputDeepFrameBuffer(frameBuffer, pixelType,
                                deepUData, deepFData, deepHData, width, height);

        part.setFrameBuffer(frameBuffer);

        int l1, l2;
        l1 = random_int(height);
        l2 = random_int(height);
        if (l1 > l2) swap(l1, l2);

        part.readPixelSampleCounts(l1, l2);
        assert(checkSampleCount(sampleCount, 0, width - 1, l1, l2, width));

        allocatePixels(pixelType, sampleCount,
                       deepUData, deepFData, deepHData, 0, width - 1, l1, l2);

        part.readPixels(l1, l2);

        switch (pixelType)
        {
        case 0:
            assert(checkPixels<unsigned int>(sampleCount, deepUData, 0, width - 1, l1, l2, width));
            break;
        case 1:
            assert(checkPixels<float>(sampleCount, deepFData, 0, width - 1, l1, l2, width));
            break;
        case 2:
            assert(checkPixels<half>(sampleCount, deepHData, 0, width - 1, l1, l2, width));
            break;
        }

        releasePixels(pixelType, deepUData, deepFData, deepHData, 0, width - 1, l1, l2);

        break;
    }
    case 3:
    {
        DeepTiledInputFile part(filename.c_str());
        int numXLevels = part.numXLevels();
        int numYLevels = part.numYLevels();

        int tx1, tx2, ty1, ty2;
        int lx, ly;
        lx = random_int(numXLevels);
        ly = random_int(numYLevels);
        if (levelMode == 1) ly = lx;

        int w = part.levelWidth(lx);
        int h = part.levelHeight(ly);

        int numXTiles = part.numXTiles(lx);
        int numYTiles = part.numYTiles(ly);
        tx1 = random_int(numXTiles);
        tx2 = random_int(numXTiles);
        ty1 = random_int(numYTiles);
        ty2 = random_int(numYTiles);
        if (tx1 > tx2) swap(tx1, tx2);
        if (ty1 > ty2) swap(ty1, ty2);

        DeepFrameBuffer frameBuffer;

        sampleCount.resizeErase(h, w);
        frameBuffer.insertSampleCountSlice (Slice (IMF::UINT,
                                            (char *) (&sampleCount[0][0]),
                                            sizeof (unsigned int) * 1,
                                            sizeof (unsigned int) * w));

        setInputDeepFrameBuffer(frameBuffer, pixelType,
                                deepUData, deepFData, deepHData, w, h);

        part.setFrameBuffer(frameBuffer);

        part.readPixelSampleCounts(tx1, tx2, ty1, ty2, lx, ly);

        Box2i b1 = part.dataWindowForTile(tx1, ty1, lx, ly);
        Box2i b2 = part.dataWindowForTile(tx2, ty2, lx, ly);
        assert(checkSampleCount(sampleCount, b1.min.x, b2.max.x, b1.min.y, b2.max.y, w));

        allocatePixels(pixelType, sampleCount,
                       deepUData, deepFData, deepHData,
                       b1.min.x, b2.max.x, b1.min.y, b2.max.y);

        part.readTiles(tx1, tx2, ty1, ty2, lx, ly);

        switch (pixelType)
        {
        case 0:
            assert(checkPixels<unsigned int>(sampleCount, deepUData,
                                             b1.min.x, b2.max.x, b1.min.y, b2.max.y, w));
            break;
        case 1:
            assert(checkPixels<float>(sampleCount, deepFData,
                                      b1.min.x, b2.max.x, b1.min.y, b2.max.y, w));
            break;
        case 2:
            assert(checkPixels<half>(sampleCount, deepHData,
                                     b1.min.x, b2.max.x, b1.min.y, b2.max.y, w));
            break;
        }

        releasePixels(pixelType, deepUData, deepFData, deepHData,
                      b1.min.x, b2.max.x, b1.min.y, b2.max.y);

        break;
    }
    }
}



void
modifyType (bool modify_version)
{
    FILE * f = fopen(filename.c_str(),"r+b");
    
    cout << " simulating new part type ";
    cout.flush();
    
    for(int i=0;i<4;i++)
    {
        fgetc(f); // magic number
    }
    fpos_t verflag_pos;
    fgetpos (f, &verflag_pos);
    for(int i=0;i<4;i++)
    {
        fgetc(f); // version
    }
    
    // skip over each header
    for(size_t i=0;i<headers.size();i++)
    {
        // read each attribute in header i
        while(1)
        {
            char a;
            int length=0;
            
            std::string attrib_name;
            //name
            do{
                a=fgetc(f);
                if( a!='\0' )  attrib_name+=a;
                length++;
            }while(a!='\0');
            
            
            // check for end-of-header byte
            if(length==1) break;
            
            //type of attribute
            do
            {
                a=fgetc(f);
                
            }while(a!='\0');
            
            
            //length of attribute
            size_t nr = fread (&length,4,1,f);
            if ( nr != 1 )
                throw IEX_NAMESPACE::IoExc (
                    "unable to read length of attribute");
            if (!GLOBAL_SYSTEM_LITTLE_ENDIAN)
            {
        	length = bswap_32(length);
            }

            if(!modify_version && attrib_name=="type")
            {
                // modify the type of part 1 to be 'X<whatevever>'
                fpos_t position;
                fgetpos (f, &position);
                fsetpos (f, &position);
                char x='X';
                fwrite(&x,1,1,f);
                
                // need to set the 'not just an image' byte for single part regular image files
                // and clear the tiled bit
                if(headers.size()==1 && (headers[0].type()==SCANLINEIMAGE || headers[0].type()==TILEDIMAGE))
                {
                    cerr << " flipping header ";
                    
                    fsetpos(f,&verflag_pos);
                    char x=2;
                    char y=8;
                    fwrite(&x,1,1,f);
                    fwrite(&y,1,1,f);
                }
                fclose(f);
                
                cerr << " modified ";
                return;
            }
            
            if(modify_version && attrib_name=="version")
            {
                fpos_t position;
                fgetpos (f, &position);
                fsetpos (f, &position);
                char x='X';
                fwrite(&x,1,1,f);
                fclose(f);
                cerr << " modified ";
                return;
                
            }
            
            //value of attribute
            for(int i=0;i<length;i++) 
            {
                fgetc(f);
            }
        }
    }
    
    
}

void
testWriteRead (int partNumber)
{
    cout << "Testing file with " << partNumber << " part(s)." << endl << flush;

    for(int i=0;i<40;i++)
    {
        generateRandomFile(partNumber);

        try
        {
            readFirstPart();
        }
        catch(std::exception & e)
        {
            cerr << " part reading failed with " << e.what() << " but should have succeeded\n";
            assert(false);
        }
        
        
        readWholeFiles(0);
        
        
        bool caught = false;

        // for deep images, check that "version 2" files don't load
        if(headers[0].type()==DEEPSCANLINE || headers[0].type()==DEEPTILE)
        {
            modifyType(true);
            try
            {
                caught = false;
                readFirstPart();
                cerr << " part reading succeeded but should have failed\n";
                assert(false);
            }
            catch(std::exception & e)
            {
                cout << "recieved exception (" << e.what() << ") as expected\n";
                caught = true;
                // that's what we thought would happen
            }
            assert (caught);
            readWholeFiles(2);
        }
        
        modifyType(false);
        
        try
        {
            caught = false;
            readFirstPart();
            cerr << " part reading succeeded but should have failed\n";
            assert(false);
        }
        catch(std::exception & e)
        {
            cout << "recieved exception (" << e.what() << ") as expected\n";
            caught = true;
            // that's what we thought would happen
        }
        assert (caught);
        
        // this should always succeed: it doesn't try to read the strange new type in part 0
        readWholeFiles(1);
        
        remove (filename.c_str());
        cout << endl << flush;
    }
    
}

} // namespace



void
testFutureProofing (const std::string & tempDir)
{
    filename = tempDir + "imf_test_future_proofing.exr";

    try
    {
        cout << "Testing reading future-files" << endl;

        random_reseed(1);

        int numThreads = ThreadPool::globalThreadPool().numThreads();
        ThreadPool::globalThreadPool().setNumThreads(4);

        testWriteRead( 1 );
        testWriteRead( 10 );

        ThreadPool::globalThreadPool().setNumThreads(numThreads);

        cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
        cerr << "ERROR -- caught exception: " << e.what() << endl;
        assert (false);
    }
}
