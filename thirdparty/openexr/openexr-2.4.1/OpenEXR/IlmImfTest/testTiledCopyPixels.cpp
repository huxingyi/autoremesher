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
#include <ImfInputFile.h>
#include <ImfTiledInputFile.h>
#include <ImfChannelList.h>
#include <ImfArray.h>
#include <half.h>

#include <vector>
#include <stdio.h>
#include <assert.h>


using namespace OPENEXR_IMF_NAMESPACE;
using namespace std;
using namespace IMATH_NAMESPACE;


namespace {

void
fillPixels (Array2D<half> &ph, int width, int height)
{
    for (int y = 0; y < height; ++y)
	for (int x = 0; x < width; ++x)
	    ph[y][x] = sin (double (x)) + sin (y * 0.5);
}


void
writeCopyReadONE (const char fileName1[],
		  const char fileName2[],
		  int width,
		  int height,
		  int xSize, 
		  int ySize,
		  int xOffset,
		  int yOffset,
		  Compression comp,
		  LevelRoundingMode rmode)
{
    cout << "levelMode 0, " << "compression " << comp <<
	    ", roundingMode " << rmode <<
            ", xOffset " << xOffset << ", yOffset " << yOffset << endl;
    
    Header hdr ((Box2i (V2i (0, 0),			// display window
		        V2i (width - 1, height -1))),
		(Box2i (V2i (xOffset, yOffset),		// data window
		        V2i (xOffset + width - 1, yOffset + height - 1))));

    hdr.compression() = comp;
    hdr.lineOrder() = INCREASING_Y;
    hdr.channels().insert ("H", Channel (HALF, 1, 1));
    
    hdr.setTileDescription(TileDescription(xSize, ySize, ONE_LEVEL, rmode));

    Array2D<half> ph1 (height, width);
    fillPixels (ph1, width, height);
    
    {
        FrameBuffer fb; 

        fb.insert ("H",
                   Slice (HALF,
                          (char *) &ph1[-yOffset][-xOffset],
                          sizeof (ph1[0][0]),
                          sizeof (ph1[0][0]) * width));

        cout << " writing" << flush;

        remove (fileName1);
        TiledOutputFile out (fileName1, hdr);
        out.setFrameBuffer (fb);
        out.writeTiles (0, out.numXTiles() - 1, 0, out.numYTiles() - 1);
    }

    {
        cout << " copying" << flush;

        remove (fileName2);
        InputFile in (fileName1);
        TiledOutputFile out (fileName2, in.header());
        out.copyPixels (in);
    }

    {
        cout << " reading" << flush;

        TiledInputFile in1 (fileName1);
        TiledInputFile in2 (fileName2);

        const Box2i &dw = in2.header().dataWindow();
        int w = dw.max.x - dw.min.x + 1;
        int h = dw.max.y - dw.min.y + 1;
        int dx = dw.min.x;
        int dy = dw.min.y;

        Array2D<half> ph1 (h, w);
        Array2D<half> ph2 (h, w);

        FrameBuffer fb1;
        FrameBuffer fb2;

        fb1.insert ("H",
                    Slice (HALF,
                           (char *) &ph1[-dy][-dx],
                           sizeof (ph1[0][0]),
                           sizeof (ph1[0][0]) * w));

        fb2.insert ("H",
                    Slice (HALF,
                           (char *) &ph2[-dy][-dx],
                           sizeof (ph2[0][0]),
                           sizeof (ph2[0][0]) * w));

        in1.setFrameBuffer (fb1);
        in1.readTiles (0, in1.numXTiles() - 1, 0, in1.numYTiles() - 1);
        in2.setFrameBuffer (fb2);
        in2.readTiles (0, in2.numXTiles() - 1, 0, in2.numYTiles() - 1);

        cout << " comparing" << flush;

        assert (in2.header().displayWindow() == hdr.displayWindow());
        assert (in2.header().dataWindow() == hdr.dataWindow());
        assert (in2.header().pixelAspectRatio() == hdr.pixelAspectRatio());
        assert (in2.header().screenWindowCenter() == hdr.screenWindowCenter());
        assert (in2.header().screenWindowWidth() == hdr.screenWindowWidth());
        assert (in2.header().lineOrder() == hdr.lineOrder());
        assert (in2.header().compression() == hdr.compression());
        assert (in2.header().channels() == hdr.channels());

        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
		assert (ph1[y][x] == ph2[y][x]);
    }

    remove (fileName1);
    remove (fileName2);
    cout << endl;
}



void
writeCopyReadMIP (const char fileName1[],
		  const char fileName2[],
		  int width,
		  int height,
		  int xSize, 
		  int ySize,
		  int xOffset,
		  int yOffset,
		  Compression comp,
		  LevelRoundingMode rmode)
{
    cout << "levelMode 1, " << "compression " << comp <<
	    ", roundingMode " << rmode <<
            ", xOffset " << xOffset << ", yOffset " << yOffset << endl;
    
    Header hdr ((Box2i (V2i (0, 0),			// display window
                        V2i (width - 1, height -1))),
                (Box2i (V2i (xOffset, yOffset),		// data window
                        V2i (xOffset + width - 1, yOffset + height - 1))));

    hdr.compression() = comp;
    hdr.lineOrder() = INCREASING_Y;
    hdr.channels().insert ("H", Channel (HALF, 1, 1));
    
    hdr.setTileDescription(TileDescription(xSize, ySize, MIPMAP_LEVELS, rmode));
    
    Array < Array2D<half> > levels;

    {
        cout << " writing" << flush;

        remove (fileName1);
        TiledOutputFile out (fileName1, hdr);
        
        int numLevels = out.numLevels();
	levels.resizeErase (numLevels);

        for (int level = 0; level < out.numLevels(); ++level)
        {
            int levelWidth  = out.levelWidth(level);
            int levelHeight = out.levelHeight(level);
            levels[level].resizeErase(levelHeight, levelWidth);
            fillPixels (levels[level], levelWidth, levelHeight);
            
            FrameBuffer fb; 

            fb.insert ("H",
                       Slice (HALF,
                              (char *) &levels[level][-yOffset][-xOffset],
                              sizeof (levels[level][0][0]),
                              sizeof (levels[level][0][0]) * levelWidth));
        
            out.setFrameBuffer (fb);
            out.writeTiles (0, out.numXTiles(level) - 1,
                            0, out.numYTiles(level) - 1, level);
        }        
    }

    {
        cout << " copying" << flush;

        remove (fileName2);
        InputFile in (fileName1);
        TiledOutputFile out (fileName2, in.header());
        out.copyPixels (in);
    }

    {
        cout << " reading" << flush;

        TiledInputFile in1 (fileName1);
        TiledInputFile in2 (fileName2);

        const Box2i &dw = in2.header().dataWindow();
        int dx = dw.min.x;
        int dy = dw.min.y;

        int numLevels = in2.numLevels();
        Array < Array2D<half> > levels1 (numLevels);
        Array < Array2D<half> > levels2 (numLevels);

        for (int level = 0; level < numLevels; ++level)
        {
            int levelWidth = in2.levelWidth(level);
            int levelHeight = in2.levelHeight(level);
            levels1[level].resizeErase(levelHeight, levelWidth);
            levels2[level].resizeErase(levelHeight, levelWidth);

            FrameBuffer fb1; 
            FrameBuffer fb2; 

            fb1.insert ("H",
                        Slice (HALF,
                               (char *) &levels1[level][-dy][-dx],
                               sizeof (levels1[level][0][0]),
                               sizeof (levels1[level][0][0]) * levelWidth));

            fb2.insert ("H",
                        Slice (HALF,
                               (char *) &levels2[level][-dy][-dx],
                               sizeof (levels2[level][0][0]),
                               sizeof (levels2[level][0][0]) * levelWidth));

            in1.setFrameBuffer (fb1);
            in2.setFrameBuffer (fb2);

            in1.readTiles (0, in1.numXTiles(level) - 1,
                           0, in1.numYTiles(level) - 1, level);

            in2.readTiles (0, in2.numXTiles(level) - 1,
                           0, in2.numYTiles(level) - 1, level);
        }

        cout << " comparing" << flush;

        assert (in2.header().displayWindow() == hdr.displayWindow());
        assert (in2.header().dataWindow() == hdr.dataWindow());
        assert (in2.header().pixelAspectRatio() == hdr.pixelAspectRatio());
        assert (in2.header().screenWindowCenter() == hdr.screenWindowCenter());
        assert (in2.header().screenWindowWidth() == hdr.screenWindowWidth());
        assert (in2.header().lineOrder() == hdr.lineOrder());
        assert (in2.header().compression() == hdr.compression());
        assert (in2.header().channels() == hdr.channels());

        for (int l = 0; l < numLevels; ++l)
            for (int y = 0; y < in1.levelHeight(l); ++y)
                for (int x = 0; x < in1.levelWidth(l); ++x)                    
		{
                    assert ((levels2[l])[y][x] == (levels1[l])[y][x]);

		    if (comp != B44_COMPRESSION && comp != B44A_COMPRESSION)
			assert ((levels2[l])[y][x] == (levels[l])[y][x]);
		}
    }

    remove (fileName1);
    remove (fileName2);
    cout << endl;
}


void
writeCopyReadRIP (const char fileName1[],
		  const char fileName2[],
		  int width,
		  int height,
		  int xSize, 
		  int ySize,
		  int xOffset,
		  int yOffset,
		  Compression comp,
		  LevelRoundingMode rmode)
{
    cout << "levelMode 2, " << "compression " << comp <<
	    ", roundingMode " << rmode <<
            ", xOffset " << xOffset << ", yOffset " << yOffset << endl;
    
    Header hdr ((Box2i (V2i (0, 0),			// display window
                        V2i (width - 1, height -1))),
                (Box2i (V2i (xOffset, yOffset),		// data window
                        V2i (xOffset + width - 1, yOffset + height - 1))));

    hdr.compression() = comp;
    hdr.lineOrder() = INCREASING_Y;
    hdr.channels().insert ("H", Channel (HALF, 1, 1));
    
    hdr.setTileDescription(TileDescription(xSize, ySize, RIPMAP_LEVELS, rmode));
    
    Array2D < Array2D<half> > levels;

    {
        cout << " writing" << flush;

        remove (fileName1);
        TiledOutputFile out (fileName1, hdr);
        
	levels.resizeErase (out.numYLevels(), out.numXLevels());

        for (int ylevel = 0; ylevel < out.numYLevels(); ++ylevel)
        {            
            for (int xlevel = 0; xlevel < out.numXLevels(); ++xlevel)
            {
                int levelWidth = out.levelWidth(xlevel);
                int levelHeight = out.levelHeight(ylevel);
                levels[ylevel][xlevel].resizeErase(levelHeight, levelWidth);          
                fillPixels (levels[ylevel][xlevel], levelWidth, levelHeight);
                
                FrameBuffer fb;
                fb.insert ("H",
		   Slice (HALF,
			  (char *) &levels[ylevel][xlevel][-yOffset][-xOffset],
			  sizeof (levels[ylevel][xlevel][0][0]),
			  sizeof (levels[ylevel][xlevel][0][0]) * levelWidth));

                out.setFrameBuffer (fb);

                out.writeTiles (0, out.numXTiles(xlevel) - 1,
                                0, out.numYTiles(ylevel) - 1, xlevel, ylevel);
            }
        }        
    }

    {
        cout << " copying" << flush;

        remove (fileName2);
        InputFile in (fileName1);
        TiledOutputFile out (fileName2, in.header());
        out.copyPixels (in);
    }

    {
        cout << " reading" << flush;

        TiledInputFile in1 (fileName1);
        TiledInputFile in2 (fileName2);

        const Box2i &dw = in2.header().dataWindow();
        int dx = dw.min.x;
        int dy = dw.min.y;

        int numXLevels = in2.numXLevels();
        int numYLevels = in2.numYLevels();
        Array2D < Array2D<half> > levels1 (numYLevels, numXLevels);
        Array2D < Array2D<half> > levels2 (numYLevels, numXLevels);

        for (int ylevel = 0; ylevel < numYLevels; ++ylevel)
        {
            for (int xlevel = 0; xlevel < numXLevels; ++xlevel)
            {
                int levelWidth  = in2.levelWidth(xlevel);
                int levelHeight = in2.levelHeight(ylevel);
                levels1[ylevel][xlevel].resizeErase(levelHeight, levelWidth);
                levels2[ylevel][xlevel].resizeErase(levelHeight, levelWidth);

                FrameBuffer fb1;
                FrameBuffer fb2;

                fb1.insert ("H",
		   Slice (HALF,
			  (char *) &levels1[ylevel][xlevel][-dy][-dx],
			  sizeof (levels1[ylevel][xlevel][0][0]),
			  sizeof (levels1[ylevel][xlevel][0][0]) * levelWidth));

                fb2.insert ("H",
		   Slice (HALF,
			  (char *) &levels2[ylevel][xlevel][-dy][-dx],
			  sizeof (levels2[ylevel][xlevel][0][0]),
			  sizeof (levels2[ylevel][xlevel][0][0]) * levelWidth));

                in1.setFrameBuffer (fb1);
                in2.setFrameBuffer (fb2);

                in1.readTiles (0, in1.numXTiles(xlevel) - 1,
                               0, in1.numYTiles(ylevel) - 1,
                               xlevel, ylevel);

                in2.readTiles (0, in2.numXTiles(xlevel) - 1,
                               0, in2.numYTiles(ylevel) - 1,
                               xlevel, ylevel);
            }
        }

        cout << " comparing" << flush;

        assert (in2.header().displayWindow() == hdr.displayWindow());
        assert (in2.header().dataWindow() == hdr.dataWindow());
        assert (in2.header().pixelAspectRatio() == hdr.pixelAspectRatio());
        assert (in2.header().screenWindowCenter() == hdr.screenWindowCenter());
        assert (in2.header().screenWindowWidth() == hdr.screenWindowWidth());
        assert (in2.header().lineOrder() == hdr.lineOrder());
        assert (in2.header().compression() == hdr.compression());
        assert (in2.header().channels() == hdr.channels());

        for (int ly = 0; ly < numYLevels; ++ly)
            for (int lx = 0; lx < numXLevels; ++lx)
                for (int y = 0; y < in1.levelHeight(ly); ++y)
                    for (int x = 0; x < in1.levelWidth(lx); ++x)
		    {
                        assert ((levels2[ly][lx])[y][x] ==
                                (levels1[ly][lx])[y][x]);

			if (comp != B44_COMPRESSION && comp != B44A_COMPRESSION)
			    assert ((levels2[ly][lx])[y][x] ==
				    (levels[ly][lx])[y][x]);

		    }
    }

    remove (fileName1);
    remove (fileName2);
    cout << endl;
}


void
writeCopyRead (const std::string &tempDir, int w, int h, int xs, int ys, int dx, int dy)
{
    std::string filename1 = tempDir + "imf_test_copy1.exr";
    std::string filename2 = tempDir + "imf_test_copy2.exr";

    for (int comp = 0; comp < NUM_COMPRESSION_METHODS; ++comp)
    {
	for (int rmode = 0; rmode < NUM_ROUNDINGMODES; ++rmode)
	{
	    writeCopyReadONE (filename1.c_str(), filename2.c_str(), w, h, xs, ys, dx, dy,
			      Compression (comp), LevelRoundingMode (rmode));

	    writeCopyReadMIP (filename1.c_str(), filename2.c_str(), w, h, xs, ys, dx, dy,
			      Compression (comp), LevelRoundingMode (rmode));
			      
	    writeCopyReadRIP (filename1.c_str(), filename2.c_str(), w, h, xs, ys, dx, dy,
			      Compression (comp), LevelRoundingMode (rmode));
	}
    }
}

} // namespace


void
testTiledCopyPixels (const std::string &tempDir)
{
    try
    {
        cout << "Testing fast pixel copying for tiled files" << endl;

        const int W = 171;
        const int H = 259;
        const int DX = 17;
        const int DY = 29;
        const int YS = 55;

        writeCopyRead (tempDir, W, H, DX, YS, 0,  0);
        writeCopyRead (tempDir, W, H, DX, YS, 0,  DY);
        writeCopyRead (tempDir, W, H, DX, YS, DX, 0);
        writeCopyRead (tempDir, W, H, DX, YS, DX, DY);

        cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
        cerr << "ERROR -- caught exception: " << e.what() << endl;
        assert (false);
    }
}
