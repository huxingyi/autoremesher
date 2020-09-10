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

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <algorithm>

#include "tmpDir.h"

#include "testMultiTiledPartThreading.h"

#include <ImfPartType.h>
#include <ImfMultiPartInputFile.h>
#include <ImfMultiPartOutputFile.h>
#include <ImfOutputFile.h>
#include <ImfTiledOutputFile.h>
#include <ImfGenericOutputFile.h>
#include <ImfArray.h>
#include <ImfChannelList.h>
#include <ImfOutputPart.h>
#include <ImfInputPart.h>
#include <ImfTiledOutputPart.h>
#include <ImfTiledInputPart.h>
#include <IlmThreadPool.h>
#include <IlmThreadMutex.h>


namespace
{

namespace IMF = OPENEXR_IMF_NAMESPACE;
using namespace IMF;
using namespace std;
using namespace IMATH_NAMESPACE;
using namespace ILMTHREAD_NAMESPACE;

const int height = 263;
const int width = 197;

vector<Header> headers;
int pixelTypes[2];
int levelMode;
int tileSize;

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

class WritingTask: public Task
{
    public:
        WritingTask (TaskGroup *group, TiledOutputPart& part,
                     int lx, int ly, int startY, int numXTiles):
            Task(group),
            part(part),
            lx(lx),
            ly(ly),
            startY(startY),
            numXTiles(numXTiles)
        {}

        void execute()
        {
            part.writeTiles(0, numXTiles - 1, startY, startY, lx, ly);
        }

    private:
        TiledOutputPart& part;
        int lx, ly;
        int startY;
        int numXTiles;
};

class ReadingTask: public Task
{
    public:
        ReadingTask (TaskGroup *group, TiledInputPart& part,
                     int lx, int ly, int startY, int numXTiles):
            Task(group),
            part(part),
            lx(lx),
            ly(ly),
            startY(startY),
            numXTiles(numXTiles)
        {}

        void execute()
        {
            part.readTiles(0, numXTiles - 1, startY, startY, lx, ly);
        }

    private:
        TiledInputPart& part;
        int lx, ly;
        int startY;
        int numXTiles;
};

void
generateFiles (const std::string & fn)
{
    //
    // Generating headers.
    //

    cout << "Generating headers " << flush;
    headers.clear();
    for (int i = 0; i < 2; i++)
    {
        Header header(width, height);
        int pixelType = pixelTypes[i];

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

        header.setType(TILEDIMAGE);

        int tileX = tileSize;
        int tileY = tileSize;
        LevelMode lm = NUM_LEVELMODES;
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

        headers.push_back(header);
    }

    //
    // Preparing.
    //
    remove (fn.c_str());
    MultiPartOutputFile file(fn.c_str(), &headers[0],headers.size());
    vector<TiledOutputPart> parts;
    Array2D<half> halfData[2];
    Array2D<float> floatData[2];
    Array2D<unsigned int> uintData[2];
    for (int i = 0; i < 2; i++)
    {
        TiledOutputPart part(file, i);
        parts.push_back(part);
    }

    //
    // Writing files.
    //
    cout << "Writing files " << flush;

    //
    // Two parts are the same, and we pick parts[0].
    //
    TiledOutputPart& part = parts[0];

    int numXLevels = part.numXLevels();
    int numYLevels = part.numYLevels();

    for (int xLevel = 0; xLevel < numXLevels; xLevel++)
        for (int yLevel = 0; yLevel < numYLevels; yLevel++)
        {
            if (!part.isValidLevel(xLevel, yLevel))
                continue;

            int w = part.levelWidth(xLevel);
            int h = part.levelHeight(yLevel);

            FrameBuffer frameBuffers[2];

            for (int i = 0; i < 2; i++)
            {
                FrameBuffer& frameBuffer = frameBuffers[i];

                switch (pixelTypes[i])
                {
                    case 0:
                        fillPixels<unsigned int>(uintData[i], w, h);
                        break;
                    case 1:
                        fillPixels<float>(floatData[i], w, h);
                        break;
                    case 2:
                        fillPixels<half>(halfData[i], w, h);
                        break;
                }
                setOutputFrameBuffer(frameBuffer, pixelTypes[i],
                                     uintData[i],
                                     floatData[i],
                                     halfData[i],
                                     w);
                parts[i].setFrameBuffer(frameBuffer);
            }

            TaskGroup taskGroup;
            ThreadPool* threadPool = new ThreadPool(2);
            int numXTiles = part.numXTiles(xLevel);
            int numYTiles = part.numYTiles(yLevel);
            for (int i = 0; i < numYTiles; i++)
            {
                threadPool->addTask(
                                (new WritingTask (&taskGroup, parts[0],
                                                  xLevel, yLevel, i, numXTiles)));
                threadPool->addTask(
                                (new WritingTask (&taskGroup, parts[1],
                                                  xLevel, yLevel, i, numXTiles)));
            }
            delete threadPool;
        }
}

void
readFiles (const std::string & fn)
{

    cout << "Checking headers " << flush;
    MultiPartInputFile file(fn.c_str());
    assert (file.parts() == 2);
    for (size_t i = 0; i < 2; i++)
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

    //
    // Preparing.
    //

    Array2D<unsigned int> uData[2];
    Array2D<float> fData[2];
    Array2D<half> hData[2];
    vector<TiledInputPart> parts;
    for (int i = 0; i < 2; i++)
    {
        TiledInputPart part(file, i);
        parts.push_back(part);
    }

    //
    // Reading files.
    //

    cout << "Reading and comparing files " << flush;
    TiledInputPart& part = parts[0];

    int numXLevels = part.numXLevels();
    int numYLevels = part.numYLevels();

    for (int xLevel = 0; xLevel < numXLevels; xLevel++)
        for (int yLevel = 0; yLevel < numYLevels; yLevel++)
        {
            if (!part.isValidLevel(xLevel, yLevel))
                continue;

            int w = part.levelWidth(xLevel);
            int h = part.levelHeight(yLevel);

            FrameBuffer frameBuffers[2];

            for (int i = 0; i < 2; i++)
            {
                FrameBuffer& frameBuffer = frameBuffers[i];

                setInputFrameBuffer(frameBuffer, pixelTypes[i],
                                     uData[i],
                                     fData[i],
                                     hData[i],
                                     w, h);
                parts[i].setFrameBuffer(frameBuffer);
            }

            TaskGroup taskGroup;
            ThreadPool* threadPool = new ThreadPool(2);
            int numXTiles = part.numXTiles(xLevel);
            int numYTiles = part.numYTiles(yLevel);
            for (int i = 0; i < numYTiles; i++)
            {
                threadPool->addTask(
                                (new ReadingTask (&taskGroup, parts[0],
                                                  xLevel, yLevel, i, numXTiles)));
                threadPool->addTask(
                                (new ReadingTask (&taskGroup, parts[1],
                                                  xLevel, yLevel, i, numXTiles)));
            }
            delete threadPool;

            for (int i = 0; i < 2; i++)
            {
                switch (pixelTypes[i])
                {
                    case 0:
                        assert(checkPixels<unsigned int>(uData[i], w, h));
                        break;
                    case 1:
                        assert(checkPixels<float>(fData[i], w, h));
                        break;
                    case 2:
                        assert(checkPixels<half>(hData[i], w, h));
                        break;
                }
            }
        }
}

void
testWriteRead (const std::string & tempDir)
{
    std::string fn = tempDir + "imf_test_multi_tiled_part_threading.exr";
    string typeNames[2];
    string levelModeName;
    for (int i = 0; i < 2; i++)
    {
        switch (pixelTypes[i])
        {
            case 0:
                typeNames[i] = "unsigned int";
                break;
            case 1:
                typeNames[i] = "float";
                break;
            case 2:
                typeNames[i] = "half";
                break;
        }

        switch (levelMode)
        {
            case 0:
                levelModeName = "ONE_LEVEL";
                break;
            case 1:
                levelModeName = "MIPMAP";
                break;
            case 2:
                levelModeName = "RIPMAP";
                break;
        }
    }
    cout << "part 1: type " << typeNames[0]
         << " tiled part, "
         << "part 2: type " << typeNames[1]
         << " tiled part, "
         << "level mode " << levelModeName
         << " tile size " << tileSize << "x" << tileSize
         << endl << flush;

    generateFiles (fn);
    readFiles (fn);

    remove (fn.c_str());

    cout << endl << flush;
}

} // namespace

void testMultiTiledPartThreading (const std::string & tempDir)
{
    try
    {
        cout << "Testing the two threads reading/writing on two-tiled-part file" << endl;

        int numThreads = ThreadPool::globalThreadPool().numThreads();
        ThreadPool::globalThreadPool().setNumThreads(2);

        for (int pt1 = 0; pt1 < 3; pt1++)
            for (int pt2 = 0; pt2 < 3; pt2++)
                for (int lm = 0; lm < 3; lm++)
                    for (int size = 1; size < min(width, height); size += 50)
                    {
                        pixelTypes[0] = pt1;
                        pixelTypes[1] = pt2;
                        levelMode = lm;
                        tileSize = size;
                        testWriteRead (tempDir);
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
