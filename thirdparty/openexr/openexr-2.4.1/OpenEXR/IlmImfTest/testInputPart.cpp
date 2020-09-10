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

#include "tmpDir.h"
#include "testInputPart.h"
#include "random.h"

#include <IlmThreadPool.h>
#include <ImfMultiPartInputFile.h>
#include <ImfMultiPartOutputFile.h>
#include <ImfArray.h>
#include <ImfChannelList.h>
#include <ImfOutputPart.h>
#include <ImfInputPart.h>
#include <ImfTiledOutputPart.h>
#include <ImfPartType.h>
#include <ImfMisc.h>

namespace IMF = OPENEXR_IMF_NAMESPACE;
using namespace IMF;
using namespace std;
using namespace IMATH_NAMESPACE;
using namespace ILMTHREAD_NAMESPACE;

namespace
{
 
const int height = 267;
const int width = 193;

vector<Header> headers;
vector<int> pixelTypes;
vector<int> partTypes;
vector<int> levelModes;

template <class T>
void fillPixels (Array2D<T> &ph, int width, int height)
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
void fillPixels (Array2D<unsigned int>& sampleCount, Array2D<T*> &ph, int width, int height)
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

#if 0
void allocatePixels(int type, Array2D<unsigned int>& sampleCount,
                    Array2D<unsigned int*>& uintData, Array2D<float*>& floatData,
                    Array2D<half*>& halfData, int x1, int x2, int y1, int y2)
{
    for (int y = y1; y <= y2; y++)
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

void allocatePixels(int type, Array2D<unsigned int>& sampleCount,
                    Array2D<unsigned int*>& uintData, Array2D<float*>& floatData,
                    Array2D<half*>& halfData, int width, int height)
{
    allocatePixels(type, sampleCount, uintData, floatData, halfData, 0, width - 1, 0, height - 1);
}

void releasePixels(int type, Array2D<unsigned int*>& uintData, Array2D<float*>& floatData,
                   Array2D<half*>& halfData, int x1, int x2, int y1, int y2)
{
    for (int y = y1; y <= y2; y++)
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

void releasePixels(int type, Array2D<unsigned int*>& uintData, Array2D<float*>& floatData,
                   Array2D<half*>& halfData, int width, int height)
{
    releasePixels(type, uintData, floatData, halfData, 0, width - 1, 0, height - 1);
}
#endif

template <class T>
bool checkPixels (Array2D<T> &ph, int lx, int rx, int ly, int ry, int width)
{
    for (int y = ly; y <= ry; ++y)
        for (int x = lx; x <= rx; ++x)
            if (ph[y][x] != static_cast<T>(((y * width + x) % 2049)))
            {
                cout << "value at " << x << ", " << y << ": " << ph[y][x]
                     << ", should be " << (y * width + x) % 2049 << endl << flush;
                return false;
            }
    return true;
}

template <class T>
bool checkPixels (Array2D<T> &ph, int width, int height)
{
    return checkPixels<T> (ph, 0, width - 1, 0, height - 1, width);
}

template <class T>
bool checkPixels (Array2D<unsigned int>& sampleCount, Array2D<T*> &ph,
                  int lx, int rx, int ly, int ry, int width)
{
    for (int y = ly; y <= ry; ++y)
        for (int x = lx; x <= rx; ++x)
        {
            for (unsigned int i = 0; i < sampleCount[y][x]; i++)
            {
                if (ph[y][x][i] != (y * width + x) % 2049)
                {
                    cout << "value at " << x << ", " << y << ", sample " << i << ": " << ph[y][x][i]
                         << ", should be " << (y * width + x) % 2049 << endl << flush;
                    return false;
                }
            }
        }
    return true;
}

template <class T>
bool checkPixels (Array2D<unsigned int>& sampleCount, Array2D<T*> &ph, int width, int height)
{
    return checkPixels<T> (sampleCount, ph, 0, width - 1, 0, height - 1, width);
}

#if 0
bool checkSampleCount(Array2D<unsigned int>& sampleCount, int x1, int x2, int y1, int y2, int width)
{
    for (int i = y1; i <= y2; i++)
        for (int j = x1; j <= x2; j++)
        {
            if (sampleCount[i][j] != static_cast<unsigned int>(((i * width) + j) % 10 + 1))
            {
                cout << "sample count at " << j << ", " << i << ": " << sampleCount[i][j]
                     << ", should be " << (i * width + j) % 10 + 1 << endl << flush;
                return false;
            }
        }
    return true;
}

bool checkSampleCount(Array2D<unsigned int>& sampleCount, int width, int height)
{
    return checkSampleCount(sampleCount, 0, width - 1, 0, height - 1, width);
}
#endif

void generateRandomHeaders(int partCount, vector<Header>& headers)
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
        int partType = random_int(2);
        
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
        }

        int tileX;
        int tileY;
        int levelMode;
        if (partType == 1)
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
        

        if (partType == 0)
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

        headers.push_back(header);
    }
}

void setOutputFrameBuffer(FrameBuffer& frameBuffer, int pixelType,
                          Array2D<unsigned int>& uData, Array2D<float>& fData,
                          Array2D<half>& hData, int width)
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

void setInputFrameBuffer(FrameBuffer& frameBuffer, int pixelType,
                         Array2D<unsigned int>& uData, Array2D<float>& fData,
                         Array2D<half>& hData, int width, int height)
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
generateRandomFile (int partCount, const std::string & fn)
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

    remove(fn.c_str());
    MultiPartOutputFile file(fn.c_str(), &headers[0],headers.size());

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
        }
    }
}

void
readWholeFiles (const std::string & fn)
{
    Array2D<unsigned int> uData;
    Array2D<float> fData;
    Array2D<half> hData;

    Array2D<unsigned int*> deepUData;
    Array2D<float*> deepFData;
    Array2D<half*> deepHData;

    Array2D<unsigned int> sampleCount;

    MultiPartInputFile file(fn.c_str());
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
        assert (header.type() == headers[i].type());
    }

    cout << "Reading whole files " << flush;

    //
    // Shuffle part numbers.
    //
    vector<int> shuffledPartNumber;
    int nHeaders = static_cast<int>(headers.size());
    for (int i = 0; i < nHeaders; i++)
        shuffledPartNumber.push_back(i);
    for (int i = 0; i < nHeaders; i++)
    {
        int a = random_int(nHeaders);
        int b = random_int(nHeaders);
        swap (shuffledPartNumber[a], shuffledPartNumber[b]);
    }

    //
    // Start reading whole files.
    //
    int i;
    int partNumber;
    try
    {
        for (i = 0; i < nHeaders; i++)
        {
            partNumber = shuffledPartNumber[i];
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
        }
    }
    catch (...)
    {
        cout << "Error while reading part " << partNumber << endl << flush;
        throw;
    }
}

void
readFirstPart (const std::string & fn)
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
    //int levelMode = levelModes[0];

    int l1, l2;
    l1 = random_int(height);
    l2 = random_int(height);
    if (l1 > l2) swap(l1, l2);

    InputFile part(fn.c_str());

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
}

void
readPartialFiles (int randomReadCount, const std::string & fn)
{
    Array2D<unsigned int> uData;
    Array2D<float> fData;
    Array2D<half> hData;

    Array2D<unsigned int*> deepUData;
    Array2D<float*> deepFData;
    Array2D<half*> deepHData;

    Array2D<unsigned int> sampleCount;

    cout << "Reading partial files " << flush;
    MultiPartInputFile file(fn.c_str());

    for (int i = 0; i < randomReadCount; i++)
    {
        int partNumber = random_int(file.parts());
        //int partType = partTypes[partNumber];
        int pixelType = pixelTypes[partNumber];
        //int levelMode = levelModes[partNumber];

        int l1, l2;
        l1 = random_int(height);
        l2 = random_int(height);
        
        if (l1 > l2) swap(l1, l2);

        InputPart part(file, partNumber);

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
    }
}



void
testWriteRead (int partNumber,
               int runCount,
               int randomReadCount,
               const std::string & tempDir)
{
    cout << "Testing file with " << partNumber << " part(s)." << endl << flush;

    const std::string fn = tempDir + "imf_test_input_part.exr";

    for (int i = 0; i < runCount; i++)
    {
        generateRandomFile (partNumber, fn);
        readWholeFiles (fn);
        readFirstPart (fn);
        readPartialFiles (randomReadCount, fn);

        remove (fn.c_str());

        cout << endl << flush;
    }
}

} // namespace


void testInputPart (const std::string & tempDir)
{
    try
    {
        cout << "Testing reading multipart tiles and scanlines with InputPart" << endl;

        random_reseed(1);

        int numThreads = ThreadPool::globalThreadPool().numThreads();
        ThreadPool::globalThreadPool().setNumThreads(4);

        testWriteRead ( 1, 1,  5, tempDir);
        testWriteRead ( 2, 2,  1, tempDir);
        testWriteRead ( 8, 4,  2, tempDir);
        testWriteRead (50, 3, 11, tempDir);

        ThreadPool::globalThreadPool().setNumThreads(numThreads);

        cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
        cerr << "ERROR -- caught exception: " << e.what() << endl;
        assert (false);
    }
}
