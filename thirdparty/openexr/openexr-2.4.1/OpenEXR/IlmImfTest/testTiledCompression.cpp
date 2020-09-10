///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004-2012, Industrial Light & Magic, a division of Lucas
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

#include "compareB44.h"
#include "compareDwa.h"

#include <ImfOutputFile.h>
#include <ImfInputFile.h>
#include <ImfTiledOutputFile.h>
#include <ImfTiledInputFile.h>
#include <ImfChannelList.h>
#include <ImfArray.h>
#include <half.h>
#include <ImathRandom.h>
#include <ImfTileDescriptionAttribute.h>
#include "compareFloat.h"

#include <stdio.h>
#include <assert.h>
#include <algorithm>

using namespace OPENEXR_IMF_NAMESPACE;
using namespace std;
using namespace IMATH_NAMESPACE;


namespace {

void
fillPixels1 (Array2D<unsigned int> &pi,
	     Array2D<half> &ph,
	     Array2D<float> &pf,
	     int width,
	     int height)
{
    cout << "only zeroes" << endl;

    for (int y = 0; y < height; ++y)
	for (int x = 0; x < width; ++x)
	{
	    pi[y][x] = 0;
	    ph[y][x] = 0;
	    pf[y][x] = 0;
	}
}


void
fillPixels2 (Array2D<unsigned int> &pi,
	     Array2D<half> &ph,
	     Array2D<float> &pf,
	     int width,
	     int height)
{
    cout << "pattern 1" << endl;

    for (int y = 0; y < height; ++y)
	for (int x = 0; x < width; ++x)
	{
	    pi[y][x] = (x + y) & 1;
	    ph[y][x] = pi[y][x];
	    pf[y][x] = pi[y][x];
	}
}


void
fillPixels3 (Array2D<unsigned int> &pi,
	     Array2D<half> &ph,
	     Array2D<float> &pf,
	     int width,
	     int height)
{
    cout << "pattern 2" << endl;

    for (int y = 0; y < height; ++y)
	for (int x = 0; x < width; ++x)
	{
	    pi[y][x] = x % 100 + 100 * (y % 100);
	    ph[y][x] = sin (double (x)) + sin (y * 0.5);
	    pf[y][x] = sin (double (y)) + sin (x * 0.5);
	}
}


void
fillPixels4 (Array2D<unsigned int> &pi,
	     Array2D<half> &ph,
	     Array2D<float> &pf,
	     int width,
	     int height)
{
    cout << "random bits" << endl;

    //
    // Use of a union to extract the bit pattern from a float, as is
    // done below, works only if int and float have the same size.
    //

    assert (sizeof (int) == sizeof (float));

    Rand48 rand;

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            pi[y][x] = rand.nexti();

            do
            {
                ph[y][x].setBits (rand.nexti());
            }
            while (ph[y][x].isNan());

            union {int i; float f;} u;

            do
            {
                u.i = rand.nexti();
                pf[y][x] = u.f;
            }
            while (pf[y][x] != pf[y][x]);
        }
}


void
writeRead (const Array2D<unsigned int> &pi1,
           const Array2D<half> &ph1,
           const Array2D<float> &pf1,
           const char fileName[],
           LineOrder lorder,
           int width,
           int height,
           int xSize,
           int ySize,
           int xOffset,
           int yOffset,
           Compression comp)
{
    //
    // Write the pixel data in pi1, ph1 and ph2 to an
    // image file using the specified compression type.
    // Read the pixel data back
    // from the file and verify that the data did not
    // change.
    //

    cout << "compression " << comp << ":" << flush;


    Header hdr ((Box2i (V2i (0, 0),                     // display window
                        V2i (width - 1, height -1))),
                (Box2i (V2i (xOffset, yOffset),         // data window
                        V2i (xOffset + width - 1, yOffset + height - 1))));

    hdr.compression() = comp;
    hdr.lineOrder() = lorder;

    hdr.channels().insert ("I", Channel (UINT));
    hdr.channels().insert ("H", Channel (HALF));
    hdr.channels().insert ("F", Channel (FLOAT));
    
    hdr.setTileDescription(TileDescription(xSize, ySize, ONE_LEVEL));
    
    {
        FrameBuffer fb; 

        fb.insert ("I",                                       // name
                   Slice (UINT,                               // type
                          (char *) &pi1[-yOffset][-xOffset],  // base
                          sizeof (pi1[0][0]),                 // xStride
                          sizeof (pi1[0][0]) * width)         // yStride
                  );
                  
        fb.insert ("H",                                       // name
                   Slice (HALF,                               // type
                          (char *) &ph1[-yOffset][-xOffset],  // base
                          sizeof (ph1[0][0]),                 // xStride
                          sizeof (ph1[0][0]) * width)         // yStride
                  );
                  
        fb.insert ("F",                                       // name
                   Slice (FLOAT,                              // type
                          (char *) &pf1[-yOffset][-xOffset],  // base
                          sizeof (pf1[0][0]),                 // xStride
                          sizeof (pf1[0][0]) * width)         // yStride
                  );

        cout << " writing" << flush;

        remove (fileName);
        TiledOutputFile out (fileName, hdr);
        out.setFrameBuffer (fb);
        out.writeTiles (0, out.numXTiles() - 1, 0, out.numYTiles() - 1);
    }
    
    {
        cout << ", reading (whole file)" << flush;

        TiledInputFile in (fileName);

        const Box2i &dw = in.header().dataWindow();
        int w = dw.max.x - dw.min.x + 1;
        int h = dw.max.y - dw.min.y + 1;
        int dwx = dw.min.x;
        int dwy = dw.min.y;

        Array2D<unsigned int> pi2 (h, w);
        Array2D<half>         ph2 (h, w);
        Array2D<float>        pf2 (h, w);

        FrameBuffer fb;

        fb.insert ("I",                             // name
                   Slice (UINT,                     // type
                          (char *) &pi2[-dwy][-dwx],// base
                          sizeof (pi2[0][0]),       // xStride
                          sizeof (pi2[0][0]) * w)   // yStride
                  );

        fb.insert ("H",                             // name
                   Slice (HALF,                     // type
                          (char *) &ph2[-dwy][-dwx],// base
                          sizeof (ph2[0][0]),       // xStride
                          sizeof (ph2[0][0]) * w)   // yStride
                  );

        fb.insert ("F",                             // name
                   Slice (FLOAT,                    // type
                          (char *) &pf2[-dwy][-dwx],// base
                          sizeof (pf2[0][0]),       // xStride
                          sizeof (pf2[0][0]) * w)   // yStride
                  );

        in.setFrameBuffer (fb);
        in.readTiles (0, in.numXTiles() - 1, 0, in.numYTiles() - 1);
        

        cout << ", comparing (whole file)" << flush;

        assert (in.header().displayWindow() == hdr.displayWindow());
        assert (in.header().dataWindow() == hdr.dataWindow());
        assert (in.header().pixelAspectRatio() == hdr.pixelAspectRatio());
        assert (in.header().screenWindowCenter() == hdr.screenWindowCenter());
        assert (in.header().screenWindowWidth() == hdr.screenWindowWidth());
        assert (in.header().lineOrder() == hdr.lineOrder());
        assert (in.header().compression() == hdr.compression());

        ChannelList::ConstIterator hi = hdr.channels().begin();
        ChannelList::ConstIterator ii = in.header().channels().begin();

        while (hi != hdr.channels().end())
        {
            assert (!strcmp (hi.name(), ii.name()));
            assert (hi.channel().type == ii.channel().type);
            assert (hi.channel().xSampling == ii.channel().xSampling);
            assert (hi.channel().ySampling == ii.channel().ySampling);

            ++hi;
            ++ii;
        }

        assert (ii == in.header().channels().end());

	if (comp == B44_COMPRESSION ||
            comp == B44A_COMPRESSION ||
            comp == DWAA_COMPRESSION ||
            comp == DWAB_COMPRESSION)
	{
	    for (int y = 0; y < h; y += ySize)
	    {
		for (int x = 0; x < w; x += xSize)
		{
		    int nx = min (w - x, xSize);
		    int ny = min (h - y, ySize);

		    Array2D<half> ph3 (ny, nx);
		    Array2D<half> ph4 (ny, nx);

		    for (int y1 = 0; y1 < ny; ++y1)
		    {
			for (int x1 = 0; x1 < nx; ++x1)
			{
			    ph3[y1][x1] = ph1[y + y1][x + x1];
			    ph4[y1][x1] = ph2[y + y1][x + x1];
			}
		    }

                    if (comp == B44_COMPRESSION ||
                        comp == B44A_COMPRESSION)
                    {
                        compareB44 (nx, ny, ph3, ph4);
                    }
                    else if (comp == DWAA_COMPRESSION ||
                             comp == DWAB_COMPRESSION)
                    {
                        //XXX compareDwa (nx, ny, ph3, ph4);
                    }
		}
	    }
	}

	for (int y = 0; y < h; ++y)
	{
	    for (int x = 0; x < w; ++x)
	    {
		assert (pi1[y][x] == pi2[y][x]);

		if (comp != B44_COMPRESSION &&
                    comp != B44A_COMPRESSION &&
                    comp != DWAA_COMPRESSION &&
                    comp != DWAB_COMPRESSION)
                {
		    assert (ph1[y][x] == ph2[y][x]);
                }

		assert (equivalent (pf1[y][x], pf2[y][x], comp));
	    }
	}
    }
    
    {
        cout << ", reading and comparing (tile-by-tile)" << flush;

        TiledInputFile in (fileName);

        Array2D<unsigned int> pi2 (ySize, xSize);
        Array2D<half>         ph2 (ySize, xSize);
        Array2D<float>        pf2 (ySize, xSize);

        FrameBuffer fb;

        fb.insert ("I",                                 // name
                   Slice (UINT,                         // type
                          (char *) &pi2[0][0],          // base
                          sizeof (pi2[0][0]),           // xStride
                          sizeof (pi2[0][0]) * xSize,   // yStride
                          1,                            // xSampling
	                  1,                            // ySampling
	                  0.0,                          // fillValue
                          true,                         // reuse tiles 
                          true)
                  );

        fb.insert ("H",                                 // name
                   Slice (HALF,                         // type
                          (char *) &ph2[0][0],          // base
                          sizeof (ph2[0][0]),           // xStride
                          sizeof (ph2[0][0]) * xSize,   // yStride
                          1,                            // xSampling
	                  1,                            // ySampling
	                  0.0,                          // fillValue
                          true,                         // reuse tiles 
                          true)
                  );

        fb.insert ("F",                                 // name
                   Slice (FLOAT,                        // type
                          (char *) &pf2[0][0],          // base
                          sizeof (pf2[0][0]),           // xStride
                          sizeof (pf2[0][0]) * xSize,   // yStride
                          1,                            // xSampling
	                  1,                            // ySampling
	                  0.0,                          // fillValue
                          true,                         // reuse tiles 
                          true)
                  );

        in.setFrameBuffer (fb);
        
        assert (in.header().displayWindow() == hdr.displayWindow());
        assert (in.header().dataWindow() == hdr.dataWindow());
        assert (in.header().pixelAspectRatio() == hdr.pixelAspectRatio());
        assert (in.header().screenWindowCenter() == hdr.screenWindowCenter());
        assert (in.header().screenWindowWidth() == hdr.screenWindowWidth());
        assert (in.header().lineOrder() == hdr.lineOrder());
        assert (in.header().compression() == hdr.compression());

        ChannelList::ConstIterator hi = hdr.channels().begin();
        ChannelList::ConstIterator ii = in.header().channels().begin();

        while (hi != hdr.channels().end())
        {
            assert (!strcmp (hi.name(), ii.name()));
            assert (hi.channel().type == ii.channel().type);
            assert (hi.channel().xSampling == ii.channel().xSampling);
            assert (hi.channel().ySampling == ii.channel().ySampling);

            ++hi;
            ++ii;
        }

        assert (ii == in.header().channels().end());
        
        for (int tileY = 0; tileY < in.numYTiles(); tileY++)
        {
            for (int tileX = 0; tileX < in.numXTiles(); ++tileX)
            {
                in.readTile (tileX, tileY);

                IMATH_NAMESPACE::Box2i win = in.dataWindowForTile (tileX, tileY);
                int oX = win.min.x - xOffset;
                int oY = win.min.y - yOffset;

                for (int y = 0, y2 = win.min.y; y < ySize && y2 <= win.max.y;
                     ++y, y2++)
                {
                    for (int x = 0, x2 = win.min.x;
                         x < xSize && x2 <= win.max.x; ++x, x2++)
                    {
                        assert (pi1[oY + y][oX + x] == pi2[y][x]);

			if (comp != B44_COMPRESSION &&
                            comp != B44A_COMPRESSION &&
                            comp != DWAA_COMPRESSION &&
                            comp != DWAB_COMPRESSION)
			{
			    assert (ph1[oY + y][oX + x] == ph2[y][x]);
			}

                        assert (equivalent (pf1[oY + y][oX + x],
                                            pf2[y][x], comp));
                    }
                }
            }
        }
    }

    remove (fileName);
    cout << endl;
}


void
writeRead (const std::string &tempDir,
           const Array2D<unsigned int> &pi,
           const Array2D<half> &ph,
           const Array2D<float> &pf,
           int w,
           int h,
           int xs,
           int ys,
           int dx,
           int dy)
{
    std::string filename = tempDir + "imf_test_comp.exr";

    for (int comp = 0; comp < NUM_COMPRESSION_METHODS; ++comp)
    {
        writeRead (pi, ph, pf,
                   filename.c_str(),
                   LineOrder (0),
                   w, h,
                   xs, ys,
                   dx, dy,
                   Compression (comp));
    }
}

} // namespace


void
testTiledCompression (const std::string &tempDir)
{
    try
    {
        cout << "Testing pixel data types and data "
		"window offsets for tiled files" << endl;

        const int W = 171;
        const int H = 59;
        const int DX[] = {-17, 0, 23};
        const int DY[] = {-29, 0, 13};
        const int XS = 15;
        const int YS = 15;

        Array2D<unsigned int> pi (H, W);
        Array2D<half> ph (H, W);
        Array2D<float> pf (H, W);

        //
        // If the following assertion fails, new pixel types have
        // been added to the Imf library; testing code for the new
        // pixel types should be added to this file.
        //

        assert (NUM_PIXELTYPES == 3);
        
        for (int i = 0; i < 3; ++i)
        {
            cout << endl <<
		    "xOffset = " << DX[i] << ", "
		    "yOffset = " << DY[i] << endl;

            fillPixels1 (pi, ph, pf, W, H);
            writeRead (tempDir, pi, ph, pf, W, H, XS, YS, DX[i], DY[i]);

            fillPixels2 (pi, ph, pf, W, H);
            writeRead (tempDir, pi, ph, pf, W, H, XS, YS, DX[i], DY[i]);

            fillPixels3 (pi, ph, pf, W, H);
            writeRead (tempDir, pi, ph, pf, W, H, XS, YS, DX[i], DY[i]);

            fillPixels4 (pi, ph, pf, W, H);
            writeRead (tempDir, pi, ph, pf, W, H, XS, YS, DX[i], DY[i]);
        }

        cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
        cerr << "ERROR -- caught exception: " << e.what() << endl;
        assert (false);
    }
}
