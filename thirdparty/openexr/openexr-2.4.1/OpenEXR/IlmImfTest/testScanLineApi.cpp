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

#include <ImfTiledOutputFile.h>
#include <ImfTiledInputFile.h>
#include <ImfInputFile.h>
#include <ImfTiledRgbaFile.h>
#include <ImfRgbaFile.h>
#include <ImfArray.h>
#include <ImfChannelList.h>
#include <ImfThreading.h>
#include "IlmThread.h"
#include "ImathRandom.h"
#include <string>
#include <stdio.h>
#include <assert.h>
#include <vector>
#include <math.h>
#include <ImfTileDescriptionAttribute.h>


namespace IMF = OPENEXR_IMF_NAMESPACE;
using namespace IMF;
using namespace std;
using namespace IMATH_NAMESPACE;

namespace {


void
fillPixels (Array2D<unsigned int> &pi,
            Array2D<half> &ph,
            Array2D<float> &pf,
            int width,
            int height)
{
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            pi[y][x] = x % 100 + 100 * (y % 100);
            ph[y][x] = sin (double (x)) + sin (y * 0.5);
            pf[y][x] = sin (double (y)) + sin (x * 0.5);
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
           Compression comp,
           LevelMode mode,
	   LevelRoundingMode rmode)
{
    //
    // Write the pixel data in pi1, ph1 and ph2 to a tiled
    // image file using the specified parameters.
    // Read the pixel data back from the file using the scanline
    // interface one scanline at a time, and verify that the data did
    // not change.
    // For MIPMAP and RIPMAP_LEVELS, the lower levels of the images
    // are filled in cropped versions of the level(0,0) image,
    // i.e. no filtering is done.
    //

    cout << "levelMode " << mode <<
	    ", roundingMode " << rmode <<
            ", line order " << lorder <<
            ",\ntileSize " << xSize << "x" << ySize <<
            ", xOffset " << xOffset <<
            ", yOffset "<< yOffset << endl;

    Header hdr ((Box2i (V2i (0, 0),                     // display window
                        V2i (width - 1, height -1))),
                (Box2i (V2i (xOffset, yOffset),         // data window
                        V2i (xOffset + width - 1, yOffset + height - 1))));
    hdr.lineOrder() = lorder;
    hdr.compression() = comp;

    hdr.channels().insert ("I", Channel (IMF::UINT));
    hdr.channels().insert ("H", Channel (IMF::HALF));
    hdr.channels().insert ("F", Channel (IMF::FLOAT));
    
    hdr.setTileDescription(TileDescription(xSize, ySize, mode, rmode));
    {
        FrameBuffer fb; 

        fb.insert ("I",                                       // name
                   Slice (IMF::UINT,                          // type
                          (char *) &pi1[-yOffset][-xOffset],  // base
                          sizeof (pi1[0][0]),                 // xStride
                          sizeof (pi1[0][0]) * width)         // yStride
                  );
                  
        fb.insert ("H",                                       // name
                   Slice (IMF::HALF,                          // type
                          (char *) &ph1[-yOffset][-xOffset],  // base
                          sizeof (ph1[0][0]),                 // xStride
                          sizeof (ph1[0][0]) * width)         // yStride
                  );
                  
        fb.insert ("F",                                       // name
                   Slice (IMF::FLOAT,                              // type
                          (char *) &pf1[-yOffset][-xOffset],  // base
                          sizeof (pf1[0][0]),                 // xStride
                          sizeof (pf1[0][0]) * width)         // yStride
                  );

        cout << " writing" << flush;

        remove (fileName);
        TiledOutputFile out (fileName, hdr);
        out.setFrameBuffer (fb);
        
        int startTileY = -1;
	int endTileY = -1;
        int dy;

        switch (mode)
        {
          case ONE_LEVEL:
          {
            if (lorder == DECREASING_Y)
            {
                startTileY = out.numYTiles() - 1;
                endTileY = -1;

                dy = -1;
            }        
            else
            {
                startTileY = 0;
                endTileY = out.numYTiles();

                dy = 1;
            }

            for (int tileY = startTileY; tileY != endTileY; tileY += dy)
                for (int tileX = 0; tileX < out.numXTiles(); ++tileX)         
                    out.writeTile (tileX, tileY);
          }
          break;

          case MIPMAP_LEVELS:
          {
            if (lorder == DECREASING_Y)
            {
                endTileY = -1;
                dy = -1;
            }        
            else
            {
                startTileY = 0;
                dy = 1;
            }

            for (int level = 0; level < out.numLevels(); ++level)
            {
                if (lorder == DECREASING_Y)
                    startTileY = out.numYTiles(level) - 1;
                else
                    endTileY = out.numYTiles(level);

                for (int tileY = startTileY; tileY != endTileY; tileY += dy)
                    for (int tileX = 0; tileX < out.numXTiles(level); ++tileX)
                        out.writeTile (tileX, tileY, level);
            }
          }
          break;
          
          case RIPMAP_LEVELS:
          {
            for (int ylevel = 0; ylevel < out.numYLevels(); ++ylevel)
            {               
                if (lorder == DECREASING_Y)
                {
                    startTileY = out.numYTiles(ylevel) - 1;
                    endTileY = -1;

                    dy = -1;
                }        
                else
                {
                    startTileY = 0;
                    endTileY = out.numYTiles(ylevel);

                    dy = 1;
                }

                for (int xlevel = 0; xlevel < out.numXLevels(); ++xlevel)
                {
                    for (int tileY = startTileY; tileY != endTileY;
                         tileY += dy)
                        for (int tileX = 0; tileX < out.numXTiles (xlevel);
                             ++tileX)
                            out.writeTile (tileX, tileY, xlevel, ylevel);
                }
            }
          }
          break;
			case NUM_LEVELMODES:
			default:
				std::cerr << "Invalid tile mode " << int(mode) << std::endl;
				break;
        }
    }

    {
        cout << " reading INCREASING_Y" << flush;

        InputFile in (fileName);

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
                   Slice (IMF::UINT,                // type
                          (char *) &pi2[-dwy][-dwx],// base
                          sizeof (pi2[0][0]),       // xStride
                          sizeof (pi2[0][0]) * w)   // yStride
                  );

        fb.insert ("H",                             // name
                   Slice (IMF::HALF,                // type
                          (char *) &ph2[-dwy][-dwx],// base
                          sizeof (ph2[0][0]),       // xStride
                          sizeof (ph2[0][0]) * w)   // yStride
                  );

        fb.insert ("F",                             // name
                   Slice (IMF::FLOAT,               // type
                          (char *) &pf2[-dwy][-dwx],// base
                          sizeof (pf2[0][0]),       // xStride
                          sizeof (pf2[0][0]) * w)   // yStride
                  );

        in.setFrameBuffer (fb);
        for (int y = dw.min.y; y <= dw.max.y; ++y)
            in.readPixels (y);

        cout << " comparing" << flush;

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

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                assert (pi1[y][x] == pi2[y][x]);
                assert (ph1[y][x] == ph2[y][x]);
                assert (pf1[y][x] == pf2[y][x]);
            }
        }    
    }

    {
        cout << endl << "         reading DECREASING_Y" << flush;

        InputFile in (fileName);

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
                   Slice (IMF::UINT,                // type
                          (char *) &pi2[-dwy][-dwx],// base
                          sizeof (pi2[0][0]),       // xStride
                          sizeof (pi2[0][0]) * w)   // yStride
                  );

        fb.insert ("H",                             // name
                   Slice (IMF::HALF,                // type
                          (char *) &ph2[-dwy][-dwx],// base
                          sizeof (ph2[0][0]),       // xStride
                          sizeof (ph2[0][0]) * w)   // yStride
                  );

        fb.insert ("F",                             // name
                   Slice (IMF::FLOAT,               // type
                          (char *) &pf2[-dwy][-dwx],// base
                          sizeof (pf2[0][0]),       // xStride
                          sizeof (pf2[0][0]) * w)   // yStride
                  );

        in.setFrameBuffer (fb);
        for (int y = dw.max.y; y >= dw.min.y; --y)
            in.readPixels (y);

        cout << " comparing" << flush;

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

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                assert (pi1[y][x] == pi2[y][x]);
                assert (ph1[y][x] == ph2[y][x]);
                assert (pf1[y][x] == pf2[y][x]);
            }
        }
    }

    {
        cout << endl << "         reading INCREASING_Y "
		        "(new frame buffer on every line)" << flush;

        InputFile in (fileName);

        const Box2i &dw = in.header().dataWindow();
        int w = dw.max.x - dw.min.x + 1;
        int h = dw.max.y - dw.min.y + 1;
        int dwx = dw.min.x;
        int dwy = dw.min.y;

        Array2D<unsigned int> pi2 (h, w);
        Array2D<half>         ph2 (h, w);
        Array2D<float>        pf2 (h, w);

        for (int y = dw.min.y; y <= dw.max.y; ++y)
	{
	    FrameBuffer fb;

	    fb.insert ("I",					// name
		       Slice (IMF::UINT,			// type
			      (char *) &pi2[y - dwy][-dwx],	// base
			      sizeof (pi2[0][0]),		// xStride
			      0)				// yStride
		      );

	    fb.insert ("H",					// name
		       Slice (IMF::HALF,			// type
			      (char *) &ph2[y - dwy][-dwx],	// base
			      sizeof (ph2[0][0]),		// xStride
			      0)				// yStride
		      );

	    fb.insert ("F",                     	        // name
		       Slice (IMF::FLOAT,			// type
			      (char *) &pf2[y - dwy][-dwx],	// base
			      sizeof (pf2[0][0]),		// xStride
			      0)				// yStride
		      );

	    in.setFrameBuffer (fb);
            in.readPixels (y);
	}

        cout << " comparing" << flush;

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

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                assert (pi1[y][x] == pi2[y][x]);
                assert (ph1[y][x] == ph2[y][x]);
                assert (pf1[y][x] == pf2[y][x]);
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
           int W,
           int H,
           LineOrder lorder,
           Compression comp,
	   LevelRoundingMode rmode,
           int dx, int dy,
           int xSize, int ySize)
{
    std::string filename = tempDir + "imf_test_scanline_api.exr";

    writeRead (pi, ph, pf, filename.c_str(), lorder, W, H,
               xSize, ySize, dx, dy, comp, ONE_LEVEL, rmode);
    writeRead (pi, ph, pf, filename.c_str(), lorder, W, H,
               xSize, ySize, dx, dy, comp, MIPMAP_LEVELS, rmode);
    writeRead (pi, ph, pf, filename.c_str(), lorder, W, H,
               xSize, ySize, dx, dy, comp, RIPMAP_LEVELS, rmode);
}

} // namespace


void
testScanLineApi (const std::string &tempDir)
{
    try
    {
        cout << "Testing the scanline API for tiled files" << endl;

        const int W = 48;
        const int H = 81;
        const int DX = -17;
        const int DY = -29;
        
        Array2D<unsigned int> pi (H, W);
        Array2D<half> ph (H, W);
        Array2D<float> pf (H, W);
        fillPixels (pi, ph, pf, W, H);

	int maxThreads = ILMTHREAD_NAMESPACE::supportsThreads()? 3: 0;

	for (int n = 0; n <= maxThreads; ++n)
	{
	    if (ILMTHREAD_NAMESPACE::supportsThreads())
	    {
		setGlobalThreadCount (n);
		cout << "\nnumber of threads: " << globalThreadCount() << endl;
	    }

	    for (int lorder = 0; lorder < NUM_LINEORDERS; ++lorder)
	    {
		for (int rmode = 0; rmode < NUM_ROUNDINGMODES; ++rmode)
		{
		    writeRead (tempDir, pi, ph, pf,  W, H, 
			       LineOrder (lorder),
			       ZIP_COMPRESSION,
			       LevelRoundingMode (rmode),
			       0, 0, 1, 1);

		    writeRead (tempDir, pi, ph, pf, W, H, 
			       LineOrder (lorder),
			       ZIP_COMPRESSION,
			       LevelRoundingMode (rmode),
			       DX, DY, 1, 1);
		    
		    writeRead (tempDir, pi, ph, pf, W, H,
			       LineOrder (lorder),
			       ZIP_COMPRESSION,
			       LevelRoundingMode (rmode),
			       0, 0, 24, 26);

		    writeRead (tempDir, pi, ph, pf, W, H,
			       LineOrder (lorder),
			       ZIP_COMPRESSION,
			       LevelRoundingMode (rmode),
			       DX, DY, 24, 26);
		    
		    writeRead (tempDir, pi, ph, pf, W, H,
			       LineOrder (lorder),
			       ZIP_COMPRESSION,
			       LevelRoundingMode (rmode),
			       0, 0, 48, 81);

		    writeRead (tempDir, pi, ph, pf, W, H,
			       LineOrder (lorder),
			       ZIP_COMPRESSION,
			       LevelRoundingMode (rmode),
			       DX, DY, 48, 81);
			       
		    writeRead (tempDir, pi, ph, pf, W, H,
			       LineOrder (lorder),
			       ZIP_COMPRESSION,
			       LevelRoundingMode (rmode),
			       0, 0, 128, 96);

		    writeRead (tempDir, pi, ph, pf, W, H,
			       LineOrder (lorder),
			       ZIP_COMPRESSION,
			       LevelRoundingMode (rmode),
			       DX, DY, 128, 96);
		}
	    }
	}

        cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
        cerr << "ERROR -- caught exception: " << e.what() << endl;
        assert (false);
    }
}
