//////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2012, Industrial Light & Magic, a division of Lucasfilm
// Entertainment Company Ltd.  Portions contributed and copyright held by
// others as indicated.  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above
//       copyright notice, this list of conditions and the following
//       disclaimer.
//
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided with
//       the distribution.
//
//     * Neither the name of Industrial Light & Magic nor the names of
//       any other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//
//	Load an OpenEXR image into a pixel array.
//
//----------------------------------------------------------------------------

#include "loadImage.h"

#include <ImfRgbaFile.h>
#include <ImfTiledRgbaFile.h>
#include <ImfInputFile.h>
#include <ImfTiledInputFile.h>
#include <ImfPreviewImage.h>
#include <ImfChannelList.h>
#include <Iex.h>

#include <ImfPartType.h>
#include <ImfMultiPartInputFile.h>
#include <ImfInputPart.h>
#include <ImfTiledInputPart.h>
#include <ImfDeepScanLineInputPart.h>
#include <ImfDeepFrameBuffer.h>
#include <ImfCompositeDeepScanLine.h>
#include <ImfDeepCompositing.h>
#include <ImfDeepTiledInputPart.h>

#include <vector>

using namespace IMF;
using namespace IMATH;
using namespace std;

namespace {

void
loadImage (const char fileName[],
           const char layer[],
           int partnum,
           Header &header,
           Array<Rgba> &pixels)
{
    MultiPartInputFile inmaster (fileName);
    InputPart in (inmaster, partnum);
    header = in.header();

    ChannelList ch = header.channels();
    if(ch.findChannel("Y"))
    {
        //
        // Not handling YCA image right now
        //
        cout << "Cannot handle YCA image now!" << endl;

        //no data for YCA image
        pixels.resizeErase (1);
        header.dataWindow() = Box2i (V2i (0, 0), V2i (0, 0));
    }
    else
    {
        Box2i &dataWindow = header.dataWindow();
        int dw = dataWindow.max.x - dataWindow.min.x + 1;
        int dh = dataWindow.max.y - dataWindow.min.y + 1;
        int dx = dataWindow.min.x;
        int dy = dataWindow.min.y;

        pixels.resizeErase (dw * dh);
        memset (pixels, 0, (dw * dh) * (sizeof(Rgba)));

        size_t xs = 1 * sizeof (Rgba);
        size_t ys = dw * sizeof (Rgba);

        FrameBuffer fb;
        Rgba *base = pixels - dx - dy * dw;

        fb.insert ("R",
                   Slice (HALF,
                          (char *) &base[0].r,
                          xs, ys,
                          1, 1,     // xSampling, ySampling
                          0.0));    // fillValue

        fb.insert ("G",
                   Slice (HALF,
                          (char *) &base[0].g,
                          xs, ys,
                          1, 1,     // xSampling, ySampling
                          0.0));    // fillValue

        fb.insert ("B",
                   Slice (HALF,
                          (char *) &base[0].b,
                          xs, ys,
                          1, 1,     // xSampling, ySampling
                          0.0));    // fillValue

        fb.insert ("A",
                   Slice (HALF,
                          (char *) &base[0].a,
                          xs, ys,
                          1, 1,             // xSampling, ySampling
                          1.0));    // fillValue
        in.setFrameBuffer (fb);


        try
        {
            in.readPixels (dataWindow.min.y, dataWindow.max.y);
        }
        catch (const exception &e)
        {
            //
            // If some of the pixels in the file cannot be read,
            // print an error message, and return a partial image
            // to the caller.
            //

            cerr << e.what() << endl;
        }
    }
}

void
loadTiledImage (const char fileName[],
                const char layer[],
                int lx,
                int ly,
                int partnum,
                Header &header,
                Array<Rgba> &pixels)
{
    MultiPartInputFile inmaster (fileName);
    TiledInputPart in (inmaster, partnum);
    header = in.header();

    if (!in.isValidLevel (lx, ly))
    {
        //
        //for part doesn't have valid level
        //
        pixels.resizeErase (1);
        header.dataWindow() = Box2i (V2i (0, 0), V2i (0, 0));

        cout<<"Level (" << lx << ", " << ly << ") does "
	      "not exist in part "<< partnum << " of file "
	      << fileName << "."<<endl;
    }
    else
    {
        header.dataWindow() = in.dataWindowForLevel (lx, ly);
        header.displayWindow() = header.dataWindow();

        ChannelList ch = header.channels();
        if(ch.findChannel("Y"))
        {
            //
            // Not handling YCA image right now
            //
            cout << "Cannot handle YCA image now!" << endl;

            //no data for YCA image
            pixels.resizeErase (1);
            header.dataWindow() = Box2i (V2i (0, 0), V2i (0, 0));
        }
        else
        {
            Box2i &dataWindow = header.dataWindow();
            int dw = dataWindow.max.x - dataWindow.min.x + 1;
            int dh = dataWindow.max.y - dataWindow.min.y + 1;
            int dx = dataWindow.min.x;
            int dy = dataWindow.min.y;

            pixels.resizeErase (dw * dh);
            memset (pixels, 0, (dw * dh) * (sizeof(Rgba)));

            size_t xs = 1 * sizeof (Rgba);
            size_t ys = dw * sizeof (Rgba);
            FrameBuffer fb;
            Rgba *base = pixels - dx - dy * dw;

            fb.insert ("R",
                       Slice (HALF,
                              (char *) &base[0].r,
                              xs, ys,
                              1, 1,         // xSampling, ySampling
                              0.0));        // fillValue

            fb.insert ("G",
                       Slice (HALF,
                              (char *) &base[0].g,
                              xs, ys,
                              1, 1,         // xSampling, ySampling
                              0.0));        // fillValue

            fb.insert ("B",
                       Slice (HALF,
                              (char *) &base[0].b,
                              xs, ys,
                              1, 1,         // xSampling, ySampling
                              0.0));        // fillValue

            fb.insert ("A",
                       Slice (HALF,
                              (char *) &base[0].a,
                              xs, ys,
                              1, 1,         // xSampling, ySampling
                              1.0));        // fillValue
            in.setFrameBuffer (fb);

            try
            {
            int tx = in.numXTiles (lx);
            int ty = in.numYTiles (ly);

            //
            // For maximum speed, try to read the tiles in
            // the same order as they are stored in the file.
            //

            if (in.header().lineOrder() == INCREASING_Y)
            {
                for (int y = 0; y < ty; ++y)
                    for (int x = 0; x < tx; ++x)
                        in.readTile (x, y, lx, ly);
            }
            else
            {
                for (int y = ty - 1; y >= 0; --y)
                    for (int x = 0; x < tx; ++x)
                        in.readTile (x, y, lx, ly);
            }
            }
            catch (const exception &e)
            {
                //
                // If some of the tiles in the file cannot be read,
                // print an error message, and return a partial image
                // to the caller.
                //

                cerr << e.what() << endl;
            }
        }

    }
}


void
loadPreviewImage (const char fileName[],
                  int partnum,
                  Header &header,
                  Array<Rgba> &pixels)
{
    MultiPartInputFile inmaster (fileName);
    InputPart in (inmaster, partnum);
    header = in.header();

    if (!in.header().hasPreviewImage())
    {
        //
        // If no preview, make a 100*100 display window
        //
        header.dataWindow() = Box2i (V2i (0, 0), V2i (0, 0));
        header.displayWindow() = Box2i (V2i (0, 0), V2i (99, 99));
        pixels.resizeErase (1);

        cout << "Part " << partnum << " contains no preview image."<< endl;
    }
    else{
        const PreviewImage &preview = in.header().previewImage();
        int w = preview.width();
        int h = preview.height();

        header.displayWindow() = Box2i (V2i (0, 0), V2i (w-1, h-1));
        header.dataWindow() = header.displayWindow();
        header.pixelAspectRatio() = 1;

        pixels.resizeErase (w * h);

        //
        // Convert the 8-bit gamma-2.2 preview pixels
        // into linear 16-bit floating-point pixels.
        //

        for (int i = 0; i < w * h; ++i)
        {
            Rgba &p = pixels[i];
            const PreviewRgba &q = preview.pixels()[i];

            p.r = 2.f * pow (q.r / 255.f, 2.2f);
            p.g = 2.f * pow (q.g / 255.f, 2.2f);
            p.b = 2.f * pow (q.b / 255.f, 2.2f);
            p.a = q.a / 255.f;
        }
    }
}

void
loadImageChannel (const char fileName[],
                  const char channelName[],
                  int partnum,
                  Header &header,
                  Array<Rgba> &pixels)
{
    MultiPartInputFile inmaster (fileName);
    InputPart in (inmaster, partnum);

    header = in.header();

    if (const Channel *ch = in.header().channels().findChannel (channelName))
    {
        Box2i &dataWindow = header.dataWindow();
        int dw = dataWindow.max.x - dataWindow.min.x + 1;
        int dh = dataWindow.max.y - dataWindow.min.y + 1;
        int dx = dataWindow.min.x;
        int dy = dataWindow.min.y;

        pixels.resizeErase (dw * dh);

        for (int i = 0; i < dw * dh; ++i)
        {
            pixels[i].r = half::qNan();
            pixels[i].g = half::qNan();
            pixels[i].b = half::qNan();
        }
        FrameBuffer fb;

        fb.insert (channelName,
                   Slice (HALF,
                          (char *) &pixels[-dx - dy * dw].g,
                          sizeof (Rgba) * ch->xSampling,
                          sizeof (Rgba) * ch->ySampling * dw,
                          ch->xSampling,
                          ch->ySampling));

        in.setFrameBuffer (fb);

        try
        {
            in.readPixels (dataWindow.min.y, dataWindow.max.y);
        }
        catch (const exception &e)
        {
            //
            // If some of the pixels in the file cannot be read,
            // print an error message, and return a partial image
            // to the caller.
            //

            cerr << e.what() << endl;
        }

        for (int i = 0; i < dw * dh; ++i)
        {
            pixels[i].r = pixels[i].g;
            pixels[i].b = pixels[i].g;
        }
    }
    else
    {
        cerr << "Image file \"" << fileName << "\" has no "
        "channel named \"" << channelName << "\"." << endl;

        //
        //no data for this channel
        //
        pixels.resizeErase (1);
        header.dataWindow() = Box2i (V2i (0, 0), V2i (0, 0));
    }
}

void
loadTiledImageChannel (const char fileName[],
                       const char channelName[],
                       int lx,
                       int ly,
                       int partnum,
                       Header &header,
                       Array<Rgba> &pixels)
{
    MultiPartInputFile inmaster (fileName);
    TiledInputPart in (inmaster, partnum);

    if (!in.isValidLevel (lx, ly))
    {
        THROW (IEX_NAMESPACE::InputExc, "Level (" << lx << ", " << ly << ") does "
               "not exist in file " << fileName << ".");
    }

    header = in.header();

    if (const Channel *ch = in.header().channels().findChannel (channelName))
    {
        header.dataWindow() = in.dataWindowForLevel (lx, ly);
        header.displayWindow() = header.dataWindow();

        Box2i &dataWindow = header.dataWindow();
        int dw = dataWindow.max.x - dataWindow.min.x + 1;
        int dh = dataWindow.max.y - dataWindow.min.y + 1;
        int dx = dataWindow.min.x;
        int dy = dataWindow.min.y;

        pixels.resizeErase (dw * dh);

        for (int i = 0; i < dw * dh; ++i)
        {
            pixels[i].r = half::qNan();
            pixels[i].g = half::qNan();
            pixels[i].b = half::qNan();
        }

        FrameBuffer fb;

        fb.insert (channelName,
                   Slice (HALF,
                          (char *) &pixels[-dx - dy * dw].g,
                          sizeof (Rgba) * ch->xSampling,
                          sizeof (Rgba) * ch->ySampling * dw,
                          ch->xSampling,
                          ch->ySampling));

        in.setFrameBuffer (fb);

        try
        {
            int tx = in.numXTiles (lx);
            int ty = in.numYTiles (ly);

            //
            // For maximum speed, try to read the tiles in
            // the same order as they are stored in the file.
            //

            if (in.header().lineOrder() == INCREASING_Y)
            {
                for (int y = 0; y < ty; ++y)
                    for (int x = 0; x < tx; ++x)
                        in.readTile (x, y, lx, ly);
            }
            else
            {
                for (int y = ty - 1; y >= 0; --y)
                    for (int x = 0; x < tx; ++x)
                        in.readTile (x, y, lx, ly);
            }
        }
        catch (const exception &e)
        {
            //
            // If some of the tiles in the file cannot be read,
            // print an error message, and return a partial image
            // to the caller.
            //

            cerr << e.what() << endl;
        }

        for (int i = 0; i < dw * dh; ++i)
        {
            pixels[i].r = pixels[i].g;
            pixels[i].b = pixels[i].g;
        }
    }
    else
    {
        cerr << "Image file \"" << fileName << "\" part " << partnum << " "
        "has no channel named \"" << channelName << "\"." << endl;

        //
        //no data for this channel
        //
        pixels.resizeErase (1);
        header.dataWindow() = Box2i (V2i (0, 0), V2i (0, 0));
    }
}

void
loadDeepScanlineImage (MultiPartInputFile &inmaster,
                       int partnum,
                       int &zsize,
                       Header &header,
                       Array<Rgba> &pixels,
                       Array<float*> &zbuff,
                       Array<unsigned int> &sampleCount,
                       bool deepComp)
{
    DeepScanLineInputPart in (inmaster, partnum);
    header = in.header();

    Box2i &dataWindow = header.dataWindow();
    int dw = dataWindow.max.x - dataWindow.min.x + 1;
    int dh = dataWindow.max.y - dataWindow.min.y + 1;
    int dx = dataWindow.min.x;
    int dy = dataWindow.min.y;

    // display black right now
    pixels.resizeErase (dw * dh);
    memset (pixels, 0, (dw * dh) * (sizeof(Rgba)));

    Array< half* > dataR;
    Array< half* > dataG;
    Array< half* > dataB;

    Array< float* > zback;
    Array< half* > alpha;

    zsize = dw * dh;
    zbuff.resizeErase (zsize);
    zback.resizeErase (zsize);
    alpha.resizeErase (dw * dh);

    dataR.resizeErase (dw * dh);
    dataG.resizeErase (dw * dh);
    dataB.resizeErase (dw * dh);
    sampleCount.resizeErase (dw * dh);

    int rgbflag = 0;
    int deepCompflag = 0;

    if (header.channels().findChannel ("R"))
    {
        rgbflag = 1;
    }
    else if (header.channels().findChannel ("B"))
    {
        rgbflag = 1;
    }
    else if (header.channels().findChannel ("G"))
    {
        rgbflag = 1;
    }

    if (header.channels().findChannel ("Z") &&
        header.channels().findChannel ("A") &&
        deepComp)
    {
        deepCompflag = 1;
    }

    DeepFrameBuffer fb;

    fb.insertSampleCountSlice (Slice (UINT,
                                      (char *) (&sampleCount[0]
                                                - dx- dy * dw),
                                      sizeof (unsigned int) * 1,
                                      sizeof (unsigned int) * dw));

    fb.insert ("Z",
               DeepSlice (FLOAT,
                          (char *) (&zbuff[0] - dx- dy * dw),
                          sizeof (float *) * 1,    // xStride for pointer array
                          sizeof (float *) * dw,   // yStride for pointer array
                          sizeof (float) * 1));    // stride for z data sample
    fb.insert ("ZBack",
               DeepSlice (FLOAT,
                          (char *) (&zback[0] - dx- dy * dw),
                          sizeof (float *) * 1,    // xStride for pointer array
                          sizeof (float *) * dw,   // yStride for pointer array
                          sizeof (float) * 1));    // stride for z data sample

    if (rgbflag)
    {
        fb.insert ("R",
                   DeepSlice (HALF,
                              (char *) (&dataR[0] - dx- dy * dw),
                              sizeof (half *) * 1,
                              sizeof (half *) * dw,
                              sizeof (half) * 1));

        fb.insert ("G",
                   DeepSlice (HALF,
                              (char *) (&dataG[0] - dx- dy * dw),
                              sizeof (half *) * 1,
                              sizeof (half *) * dw,
                              sizeof (half) * 1));

        fb.insert ("B",
                   DeepSlice (HALF,
                              (char *) (&dataB[0] - dx- dy * dw),
                              sizeof (half *) * 1,
                              sizeof (half *) * dw,
                              sizeof (half) * 1));
    }

    fb.insert ("A",
               DeepSlice (HALF,
                          (char *) (&alpha[0] - dx- dy * dw),
                          sizeof (half *) * 1,    // xStride for pointer array
                          sizeof (half *) * dw,   // yStride for pointer array
                          sizeof (half) * 1,      // stride for z data sample
                          1, 1,                   // xSampling, ySampling
                          1.0));                  // fillValue

    in.setFrameBuffer (fb);

    in.readPixelSampleCounts (dataWindow.min.y, dataWindow.max.y);

    for (int i = 0; i < dh * dw; i++)
    {
        zbuff[i] = new float[sampleCount[i]];
        zback[i] = new float[sampleCount[i]];
        alpha[i] = new half[sampleCount[i]];
        if(rgbflag)
        {
            dataR[i] = new half[sampleCount[i]];
            dataG[i] = new half[sampleCount[i]];
            dataB[i] = new half[sampleCount[i]];
        }
    }

    in.readPixels (dataWindow.min.y, dataWindow.max.y);

    if (deepCompflag)
    {
        //
        //try deep compositing
        //
        CompositeDeepScanLine comp;
        comp.addSource (&in);

        FrameBuffer fbuffer;
        Rgba *base = pixels - dx - dy * dw;
        size_t xs = 1 * sizeof (Rgba);
        size_t ys = dw * sizeof (Rgba);

        fbuffer.insert ("R",
                   Slice (HALF,
                          (char *) &base[0].r,
                          xs, ys,
                          1, 1,     // xSampling, ySampling
                          0.0));    // fillValue

        fbuffer.insert ("G",
                   Slice (HALF,
                          (char *) &base[0].g,
                          xs, ys,
                          1, 1,     // xSampling, ySampling
                          0.0));    // fillValue

        fbuffer.insert ("B",
                   Slice (HALF,
                          (char *) &base[0].b,
                          xs, ys,
                          1, 1,     // xSampling, ySampling
                          0.0));    // fillValue

        fbuffer.insert ("A",
                   Slice (HALF,
                          (char *) &base[0].a,
                          xs, ys,
                          1, 1,             // xSampling, ySampling
                          1.0));    // fillValue
        comp.setFrameBuffer (fbuffer);
        comp.readPixels (dataWindow.min.y, dataWindow.max.y);
    }
    else
    {
        for (int i = 0; i < dh * dw; i++)
        {
            if (sampleCount[i] > 0)
            {
                if (rgbflag)
                {
                    pixels[i].r = dataR[i][0];
                    pixels[i].g = dataG[i][0];
                    pixels[i].b = dataB[i][0];
                }
                else
                {
                    pixels[i].r = zbuff[i][0];
                    pixels[i].g = alpha[i][0];
                    pixels[i].b = zback[i][0];
                }
            }
        }
    }

}


void
loadDeepTileImage (MultiPartInputFile &inmaster,
                   int partnum,
                   int &zsize,
                   Header &header,
                   Array<Rgba> &pixels,
                   Array<float*> &zbuff,
                   Array<unsigned int> &sampleCount,
                   bool deepComp)
{
    DeepTiledInputPart in (inmaster, partnum);
    header = in.header();

    Box2i &dataWindow = header.dataWindow();
    int dw = dataWindow.max.x - dataWindow.min.x + 1;
    int dh = dataWindow.max.y - dataWindow.min.y + 1;
    int dx = dataWindow.min.x;
    int dy = dataWindow.min.y;

    // display black right now
    pixels.resizeErase (dw * dh);
    memset(pixels, 0, (dw * dh) * (sizeof(Rgba)));

    Array< half* > dataR;
    Array< half* > dataG;
    Array< half* > dataB;

    Array< float* > zback;
    Array< half* > alpha;

    zsize = dw * dh;
    zbuff.resizeErase (zsize);
    zback.resizeErase (zsize);
    alpha.resizeErase (dw * dh);

    dataR.resizeErase (dw * dh);
    dataG.resizeErase (dw * dh);
    dataB.resizeErase (dw * dh);
    sampleCount.resizeErase (dw * dh);

    int rgbflag = 0;
    int deepCompflag = 0;

    if (header.channels().findChannel ("R"))
    {
        rgbflag = 1;
    }
    else if (header.channels().findChannel ("B"))
    {
        rgbflag = 1;
    }
    else if (header.channels().findChannel ("G"))
    {
        rgbflag = 1;
    }

    if (header.channels().findChannel ("Z") &&
        header.channels().findChannel ("A") &&
        deepComp)
    {
        deepCompflag = 1;
    }

    DeepFrameBuffer fb;

    fb.insertSampleCountSlice (Slice (UINT,
                                      (char *) (&sampleCount[0]
                                                - dx- dy * dw),
                                      sizeof (unsigned int) * 1,
                                      sizeof (unsigned int) * dw));

    fb.insert ("Z",
               DeepSlice (FLOAT,
                          (char *) (&zbuff[0] - dx- dy * dw),
                          sizeof (float *) * 1,    // xStride for pointer array
                          sizeof (float *) * dw,   // yStride for pointer array
                          sizeof (float) * 1));    // stride for z data sample
    fb.insert ("ZBack",
               DeepSlice (FLOAT,
                          (char *) (&zback[0] - dx- dy * dw),
                          sizeof (float *) * 1,    // xStride for pointer array
                          sizeof (float *) * dw,   // yStride for pointer array
                          sizeof (float) * 1));    // stride for z data sample

    if (rgbflag)
    {
        fb.insert ("R",
                   DeepSlice (HALF,
                              (char *) (&dataR[0] - dx- dy * dw),
                              sizeof (half *) * 1,
                              sizeof (half *) * dw,
                              sizeof (half) * 1));

        fb.insert ("G",
                   DeepSlice (HALF,
                              (char *) (&dataG[0] - dx- dy * dw),
                              sizeof (half *) * 1,
                              sizeof (half *) * dw,
                              sizeof (half) * 1));

        fb.insert ("B",
                   DeepSlice (HALF,
                              (char *) (&dataB[0] - dx- dy * dw),
                              sizeof (half *) * 1,
                              sizeof (half *) * dw,
                              sizeof (half) * 1));


    }

    fb.insert ("A",
               DeepSlice (HALF,
                          (char *) (&alpha[0] - dx- dy * dw),
                          sizeof (half *) * 1,    // xStride for pointer array
                          sizeof (half *) * dw,   // yStride for pointer array
                          sizeof (half) * 1,      // stride for z data sample
                          1, 1,                   // xSampling, ySampling
                          1.0));                  // fillValue

    in.setFrameBuffer (fb);

    int numXTiles = in.numXTiles(0);
    int numYTiles = in.numYTiles(0);

    in.readPixelSampleCounts (0, numXTiles - 1, 0, numYTiles - 1);

    for (int i = 0; i < dh * dw; i++)
    {
        zbuff[i] = new float[sampleCount[i]];
        zback[i] = new float[sampleCount[i]];
        alpha[i] = new half[sampleCount[i]];
        if (rgbflag)
        {
            dataR[i] = new half[sampleCount[i]];
            dataG[i] = new half[sampleCount[i]];
            dataB[i] = new half[sampleCount[i]];
        }
    }

    in.readTiles (0, numXTiles - 1, 0, numYTiles - 1);

    if (deepCompflag)
    {
        // Loop over all the pixels and comp manually
        // @ToDo implent deep compositing for the DeepTile case
        for (int i=0; i<zsize; ++i)
        {
            float a     = alpha[i][0];
            pixels[i].r = dataR[i][0];
            pixels[i].g = dataG[i][0];
            pixels[i].b = dataB[i][0];

            for(unsigned int s=1; s<sampleCount[i]; s++)
            {
                if(a>=1.f)
                    break;

                pixels[i].r += (1.f - a) * dataR[i][s];
                pixels[i].g += (1.f - a) * dataG[i][s];
                pixels[i].b += (1.f - a) * dataB[i][s];
                a           += (1.f - a) * alpha[i][s];
            }
        }
    }
    else
    {
        for (int i = 0; i < dh * dw; i++)
        {
            if (sampleCount[i] > 0)
            {
                if (rgbflag)
                {
                    pixels[i].r = dataR[i][0];
                    pixels[i].g = dataG[i][0];
                    pixels[i].b = dataB[i][0];
                }
                else
                {
                    pixels[i].r = zbuff[i][0];
                    pixels[i].g = alpha[i][0];
                    pixels[i].b = zback[i][0];
                }
            }
        }
    }

}

} // namespace


void
loadImage (const char fileName[],
           const char channel[],
           const char layer[],
           bool preview,
           int lx,
           int ly,
           int partnum,
           int &zsize,
           Header &header,
           Array<Rgba> &pixels,
           Array<float*>  &zbuff,
           Array<unsigned int> &sampleCount,
           bool deepComp)
{
    zsize = 0;

    MultiPartInputFile inmaster (fileName);
    Header h = inmaster.header(partnum);
    std::string  type = h.type();

    if (type == DEEPTILE)
    {
        loadDeepTileImage(inmaster,
                          partnum,
                          zsize,
                          header,
                          pixels,
                          zbuff,
                          sampleCount,
                          deepComp);
    }
    else if(type == DEEPSCANLINE)
    {
        loadDeepScanlineImage(inmaster,
                              partnum,
                              zsize,
                              header,
                              pixels,
                              zbuff,
                              sampleCount,
                              deepComp);
    }


    else if (preview)
    {
        loadPreviewImage (fileName, partnum, header, pixels);
    }
    else if (lx >= 0 || ly >= 0)
    {
        if (channel)
        {
            loadTiledImageChannel (fileName,
                                   channel,
                                   lx, ly,
                                   partnum,
                                   header,
                                   pixels);
        }
        else
        {
            loadTiledImage (fileName,
                            layer,
                            lx, ly,
                            partnum,
                            header,
                            pixels);
        }
    }
    else
    {
        if (channel)
        {

            loadImageChannel (fileName,
                              channel,
                              partnum,
                              header,
                              pixels);
        }
        else
        {
            loadImage (fileName,
                       layer,
                       partnum,
                       header,
                       pixels);
        }
    }
}
