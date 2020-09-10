///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2012, Industrial Light & Magic, a division of Lucas
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


//----------------------------------------------------------------------------
//
//	Produce a tiled version of an OpenEXR image.
//
//----------------------------------------------------------------------------

#include "makeTiled.h"
#include "Image.h"

#include "ImfTiledInputPart.h"
#include "ImfTiledOutputPart.h"
#include "ImfInputPart.h"
#include "ImfOutputPart.h"
#include "ImfDeepScanLineInputPart.h"
#include "ImfDeepScanLineOutputPart.h"
#include "ImfDeepTiledInputPart.h"
#include "ImfDeepTiledOutputPart.h"
#include "ImfChannelList.h"
#include "ImfChannelList.h"
#include "ImfFrameBuffer.h"
#include "ImfStandardAttributes.h"
#include "ImathFun.h"
#include "Iex.h"
#include "ImfMisc.h"

#include <map>
#include <algorithm>
#include <iostream>
#include <vector>

#include "namespaceAlias.h"
using namespace IMF;
using namespace IMATH_NAMESPACE;
using namespace std;


namespace {

string
extToString (Extrapolation ext)
{
    string str;

    switch (ext)
    {
        case BLACK:
            str = "black";
            break;

        case CLAMP:
            str = "clamp";
            break;

        case PERIODIC:
            str = "periodic";
            break;

        case MIRROR:
            str = "mirror";
            break;
    }

    return str;
}


int
mirror (int x, int w)
{
    int d = divp (x, w);
    int m = modp (x, w);
    return (d & 1)? w - 1 - m: m;
}


template <class T>
double
sampleX (const TypedImageChannel<T> &channel,
         int w,
         double x,
         int y,
         Extrapolation ext)
{
    //
    // Sample an image channel at location (x, y), where
    // x is a floating point number, and y is an integer.
    //

    int xs = IMATH_NAMESPACE::floor (x);
    int xt = xs + 1;
    double s = xt - x;
    double t = 1 - s;
    double vs=0.0;
    double vt=0.0;

    switch (ext)
    {
        case BLACK:

            vs = (xs >= 0 && xs < w)? double (channel (xs, y)): 0.0;
            vt = (xt >= 0 && xt < w)? double (channel (xt, y)): 0.0;
            break;

        case CLAMP:

            xs = IMATH_NAMESPACE::clamp (xs, 0, w - 1);
            xt = IMATH_NAMESPACE::clamp (xt, 0, w - 1);
            vs = channel (xs, y);
            vt = channel (xt, y);
            break;

        case PERIODIC:

            xs = modp (xs, w);
            xt = modp (xt, w);
            vs = channel (xs, y);
            vt = channel (xt, y);
            break;

        case MIRROR:

            xs = mirror (xs, w);
            xt = mirror (xt, w);
            vs = channel (xs, y);
            vt = channel (xt, y);
            break;
    }

    return s * vs + t * vt;
}


template <class T>
double
sampleY (const TypedImageChannel<T> &channel,
         int h,
         int x,
         double y,
         Extrapolation ext)
{
    //
    // Sample an image channel at location (x, y), where
    // x is an integer, and y is a floating point number.
    //

    int ys = IMATH_NAMESPACE::floor (y);
    int yt = ys + 1;
    double s = yt - y;
    double t = 1 - s;
    double vs=0.0;
    double vt=0.0;

    switch (ext)
    {
        case BLACK:

            vs = (ys >= 0 && ys < h)? double (channel (x, ys)): 0.0;
            vt = (yt >= 0 && yt < h)? double (channel (x, yt)): 0.0;
            break;

        case CLAMP:

            ys = IMATH_NAMESPACE::clamp (ys, 0, h - 1);
            yt = IMATH_NAMESPACE::clamp (yt, 0, h - 1);
            vs = channel (x, ys);
            vt = channel (x, yt);
            break;

        case PERIODIC:

            ys = modp (ys, h);
            yt = modp (yt, h);
            vs = channel (x, ys);
            vt = channel (x, yt);
            break;

        case MIRROR:

            ys = mirror (ys, h);
            yt = mirror (yt, h);
            vs = channel (x, ys);
            vt = channel (x, yt);
            break;
    }

    return s * vs + t * vt;
}


template <class T>
T
filterX (const TypedImageChannel<T> &channel,
         int w,
         double x,
         int y,
         Extrapolation ext)
{
    //
    // Horizontal four-tap filter, centered on pixel (x + 0.5, y)
    //

    return T (0.125 * sampleX (channel, w, x - 1, y, ext) +
              0.375 * sampleX (channel, w, x,     y, ext) +
              0.375 * sampleX (channel, w, x + 1, y, ext) +
              0.125 * sampleX (channel, w, x + 2, y, ext));
}


template <class T>
T
filterY (const TypedImageChannel<T> &channel,
         int h,
         int x,
         double y,
         Extrapolation ext)
{
    //
    // Vertical four-tap filter, centered on pixel (x, y + 0.5)
    //

    return T (0.125 * sampleY (channel, h, x, y - 1, ext) +
              0.375 * sampleY (channel, h, x, y,     ext) +
              0.375 * sampleY (channel, h, x, y + 1, ext) +
              0.125 * sampleY (channel, h, x, y + 2, ext));
}


template <class T>
void
reduceX (const TypedImageChannel<T> &channel0,
         TypedImageChannel<T> &channel1,
         bool filter,
         Extrapolation &ext,
         bool odd)
{
    //
    // Shrink an image channel, channel0, horizontally
    // by a factor of 2, and store the result in channel1.
    //

    int w0 = channel0.image().width();
    int w1 = channel1.image().width();
    int h1 = channel1.image().height();

    if (filter)
    {
        //
        // Low-pass filter and resample.
        // For pixels (0, y) and (w1 - 1, y) in channel 1,
        // the low-pass filter in channel0 is centered on
        // pixels (0.5, y) and (w0 - 1.5, y) respectively.
        //

        double f = (w1 > 1)? double (w0 - 2) / (w1 - 1): 1;

        for (int y = 0; y < h1; ++y)
            for (int x = 0; x < w1; ++x)
                channel1 (x, y) = filterX (channel0, w0, x * f, y, ext);
    }
    else
    {
        //
        // Resample, skipping every other pixel, without
        // low-pass filtering.  In order to keep the image
        // from sliding to the right if the channel is
        // resampled repeatedly, we skip the rightmost
        // pixel of every row on even passes, and the
        // leftmost pixel on odd passes.
        //

        int offset = odd? ((w0 - 1) - 2 * (w1 - 1)): 0;

        for (int y = 0; y < h1; ++y)
            for (int x = 0; x < w1; ++x)
                channel1 (x, y) = channel0 (2 * x + offset, y);
    }
}


template <class T>
void
reduceY (const TypedImageChannel<T> &channel0,
         TypedImageChannel<T> &channel1,
         bool filter,
         Extrapolation ext,
         bool odd)
{
    //
    // Shrink an image channel, channel0, vertically
    // by a factor of 2, and store the result in channel1.
    //

    int w1 = channel1.image().width();
    int h0 = channel0.image().height();
    int h1 = channel1.image().height();

    if (filter)
    {
        //
        // Low-pass filter and resample.
        // For pixels (x, 0) and (x, h1 - 1) in channel 1,
        // the low-pass filter in channel0 is centered on
        // pixels (x, 0.5) and (x, h0 - 1.5) respectively.
        //

        double f = (h1 > 1)? double (h0 - 2) / (h1 - 1): 1;

        for (int y = 0; y < h1; ++y)
            for (int x = 0; x < w1; ++x)
                channel1 (x, y) = filterY (channel0, h0, x, y * f, ext);
    }
    else
    {
        //
        // Resample, skipping every other pixel, without
        // low-pass filtering.  In order to keep the image
        // from sliding towards the top if the channel is
        // resampled repeatedly, we skip the top pixel of
        // every column on even passes, and the bottom pixel
        // on odd passes.
        //

        int offset = odd? ((h0 - 1) - 2 * (h1 - 1)): 0;

        for (int y = 0; y < h1; ++y)
            for (int x = 0; x < w1; ++x)
                channel1 (x, y) = channel0 (x, 2 * y + offset);
    }
}


void
reduceX (const ChannelList &channels,
         const set<string> &doNotFilter,
         Extrapolation ext,
         bool odd,
         const Image &image0,
         Image &image1)
{
    //
    // Shrink image image0 horizontally by a factor of 2,
    // and store the result in image image1.
    //

    for (ChannelList::ConstIterator i = channels.begin();
         i != channels.end();
         ++i)
    {
        const char *name = i.name();
        const Channel &channel = i.channel();
        bool filter = (doNotFilter.find (name) == doNotFilter.end());

        switch (channel.type)
        {
            case IMF::HALF:

                reduceX (image0.typedChannel<half> (name),
                         image1.typedChannel<half> (name),
                         filter, ext, odd);
                break;

            case IMF::FLOAT:

                reduceX (image0.typedChannel<float> (name),
                         image1.typedChannel<float> (name),
                         filter, ext, odd);
                break;

            case IMF::UINT:

                reduceX (image0.typedChannel<unsigned int> (name),
                         image1.typedChannel<unsigned int> (name),
                         filter, ext, odd);
                break;
            default : 
                break;
           
        }
    }
}


void
reduceY (const ChannelList &channels,
         const set<string> &doNotFilter,
         Extrapolation ext,
         bool odd,
         const Image &image0,
         Image &image1)
{
    //
    // Shrink image image0 vertically by a factor of 2,
    // and store the result in image image1.
    //

    for (ChannelList::ConstIterator i = channels.begin();
         i != channels.end();
         ++i)
    {
        const char *name = i.name();
        const Channel &channel = i.channel();
        bool filter = (doNotFilter.find (name) == doNotFilter.end());

        switch (channel.type)
        {
            case IMF::HALF:

                reduceY (image0.typedChannel<half> (name),
                         image1.typedChannel<half> (name),
                         filter, ext, odd);
                break;

            case IMF::FLOAT:

                reduceY (image0.typedChannel<float> (name),
                         image1.typedChannel<float> (name),
                         filter, ext, odd);
                break;

            case IMF::UINT:

                reduceY (image0.typedChannel<unsigned int> (name),
                         image1.typedChannel<unsigned int> (name),
                         filter, ext, odd);
                break;
            default : 
                break;
                
        }
    }
}


void
storeLevel (TiledOutputPart &out,
            const ChannelList &channels,
            int lx,
            int ly,
            const Image &image)
{
    //
    // Store the pixels for level (lx, ly) in output file out.
    //

    FrameBuffer fb;

    for (ChannelList::ConstIterator i = channels.begin();
         i != channels.end();
         ++i)
    {
        const char *name = i.name();
        fb.insert (name, image.channel(name).slice());
    }

    out.setFrameBuffer (fb);

    for (int y = 0; y < out.numYTiles (ly); ++y)
        for (int x = 0; x < out.numXTiles (lx); ++x)
            out.writeTile (x, y, lx, ly);
}

} // namespace


void
makeTiled (const char inFileName[],
           const char outFileName[],
           int partnum,
           LevelMode mode,
           LevelRoundingMode roundingMode,
           Compression compression,
           int tileSizeX,
           int tileSizeY,
           const set<string> &doNotFilter,
           Extrapolation extX,
           Extrapolation extY,
           bool verbose)
{
    Image image0;
    Image image1;
    Image image2;
    Header header;
    FrameBuffer fb;
    vector<Header> headers;

    //
    // Load the input image
    //

    MultiPartInputFile input (inFileName);
    int parts = input.parts();

    for (int p = 0 ; p < parts; p++)
    {
        if (verbose)
            cout << "reading file " << inFileName << endl;

        if(p == partnum)
        {
            InputPart in (input, p);
            header = in.header();
            if (hasEnvmap (header) && mode != ONE_LEVEL)
            {
                //
                // Proper low-pass filtering and subsampling
                // of environment maps is not implemented in
                // this program.
                //

                throw IEX_NAMESPACE::NoImplExc ("This program cannot generate "
                                      "multiresolution environment maps.  "
                                      "Use exrenvmap instead.");
            }

            image0.resize (header.dataWindow());

            for (ChannelList::ConstIterator i = header.channels().begin();
                            i != header.channels().end();
                            ++i)
            {
                const char *name = i.name();
                const Channel &channel = i.channel();

                if (channel.xSampling != 1 || channel.ySampling != 1)
                {
                    throw IEX_NAMESPACE::InputExc ("Sub-sampled image channels are "
                                         "not supported in tiled files.");
                }

                image0.addChannel (name, channel.type);
                image1.addChannel (name, channel.type);
                image2.addChannel (name, channel.type);
                fb.insert (name, image0.channel(name).slice());
            }

            in.setFrameBuffer (fb);
            in.readPixels (header.dataWindow().min.y, header.dataWindow().max.y);


            //
            // Generate the header for the output file by modifying
            // the input file's header
            //

            header.setTileDescription (TileDescription (tileSizeX, tileSizeY,
                                                        mode, roundingMode));

            header.compression() = compression;
            header.lineOrder() = INCREASING_Y;

            if (mode != ONE_LEVEL)
                addWrapmodes (header, extToString (extX) + "," + extToString (extY));

            //
            // set tileDescription, type, and chunckcount for multipart
            //
            header.setType(TILEDIMAGE);
            int chunkcount = getChunkOffsetTableSize(header, true);
            header.setChunkCount(chunkcount);

            headers.push_back(header);
        }
        else
        {
            Header h = input.header(p);
            headers.push_back(h);
        }

    }

    //
    // Store the highest-resolution level of the image in the output file
    //

    MultiPartOutputFile output (outFileName, &headers[0], headers.size());

    for(int p = 0 ; p < parts; p++)
    {
        if (p == partnum)
        {
            try
            {
                TiledOutputPart out (output, partnum);
            //    TiledOutputFile out (outFileName, header);


                out.setFrameBuffer (fb);

                if (verbose)
                    cout << "writing file " << outFileName << "\n"
                            "level (0, 0)" << endl;

                for (int y = 0; y < out.numYTiles (0); ++y)
                    for (int x = 0; x < out.numXTiles (0); ++x)
                        out.writeTile (x, y, 0);

                //
                // If necessary, generate the lower-resolution mipmap
                // or ripmap levels, and store them in the output file.
                //

                if (mode == MIPMAP_LEVELS)
                {
                    for (int l = 1; l < out.numLevels(); ++l)
                    {
                        image1.resize (out.dataWindowForLevel (l, l - 1));

                        reduceX (header.channels(),
                                 doNotFilter,
                                 extX,
                                 l & 1,
                                 image0,
                                 image1);

                        image0.resize (out.dataWindowForLevel (l, l));

                        reduceY (header.channels(),
                                 doNotFilter,
                                 extY,
                                 l & 1,
                                 image1,
                                 image0);

                        if (verbose)
                            cout << "level (" << l << ", " << l << ")" << endl;

                        storeLevel (out, header.channels(), l, l, image0);
                    }
                }

                if (mode == RIPMAP_LEVELS)
                {
                    Image *iptr0 = &image0;
                    Image *iptr1 = &image1;
                    Image *iptr2 = &image2;

                    for (int ly = 0; ly < out.numYLevels(); ++ly)
                    {
                        if (ly < out.numYLevels() - 1)
                        {
                            iptr2->resize (out.dataWindowForLevel (0, ly + 1));

                            reduceY (header.channels(),
                                     doNotFilter,
                                     extY,
                                     ly & 1,
                                     *iptr0,
                                     *iptr2);
                        }

                        for (int lx = 0; lx < out.numXLevels(); ++lx)
                        {
                            if (lx != 0 || ly != 0)
                            {
                                if (verbose)
                                    cout << "level (" << lx << ", " << ly << ")" << endl;

                                storeLevel (out, header.channels(), lx, ly, *iptr0);
                            }

                            if (lx < out.numXLevels() - 1)
                            {
                                iptr1->resize (out.dataWindowForLevel (lx + 1, ly));

                                reduceX (header.channels(),
                                         doNotFilter,
                                         extX,
                                         lx & 1,
                                         *iptr0,
                                         *iptr1);

                                swap (iptr0, iptr1);
                            }
                        }

                        swap (iptr2, iptr0);
                    }
                }
            }
            catch (const exception &e)
            {
                cerr << e.what() << endl;
            }
        }
        else
        {
            Header header = headers[p];
            std::string type = header.type();
            if (type == TILEDIMAGE)
            {
                TiledInputPart in (input, p);
                TiledOutputPart out (output, p);
                out.copyPixels (in);
            }
            else if (type == SCANLINEIMAGE)
            {
                using std::max;  InputPart in (input, p);
                OutputPart out (output, p);
                out.copyPixels (in);
            }
            else if (type == DEEPSCANLINE)
            {
                DeepScanLineInputPart in (input,p);
                DeepScanLineOutputPart out (output,p);
                out.copyPixels (in);
            }
            else if (type == DEEPTILE)
            {
                DeepTiledInputPart in (input,p);
                DeepTiledOutputPart out (output,p);
                out.copyPixels (in);
            }
        }
    }

    if (verbose)
	cout << "done." << endl;
}

