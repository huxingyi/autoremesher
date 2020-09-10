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
#include <ImathRandom.h>
#include <ImfTiledInputFile.h>
#include <ImfChannelList.h>
#include <ImfArray.h>
#include <ImfThreading.h>
#include <IlmThread.h>
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
writeCopyReadONE (const char fileName[],
		  int width,
		  int height,
		  LineOrder lorder,
		  LevelRoundingMode rmode,
		  int xSize, 
		  int ySize,
		  Compression comp,
		  bool triggerBuffering,
		  bool triggerSeeks)
{
    cout << "LineOrder " << lorder << ", buffer " << triggerBuffering <<
            ", seek " << triggerSeeks << ", levelMode 0, " <<
	    "roundingMode " << rmode << ", "
	    "compression " << comp << endl;
    
    Header hdr ((Box2i (V2i (0, 0),			// display window
		        V2i (width - 1, height -1))),
		(Box2i (V2i (0, 0),		// data window
		        V2i (width - 1, height - 1))));

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
                          (char *) &ph1[0][0],
                          sizeof (ph1[0][0]),
                          sizeof (ph1[0][0]) * width));

        cout << " writing" << flush;

        remove (fileName);
        TiledOutputFile out (fileName, hdr);
        out.setFrameBuffer (fb);

        int i;
        
        Rand32 rand1 = Rand32();
        std::vector<int> tileYs = std::vector<int>(out.numYTiles());
        std::vector<int> tileXs = std::vector<int>(out.numXTiles());
        for (i = 0; i < out.numYTiles(); i++)
        {
            if (lorder == DECREASING_Y)
                tileYs[out.numYTiles()-1-i] = i;    
            else
                tileYs[i] = i;
        }

        for (i = 0; i < out.numXTiles(); i++)
        {
            tileXs[i] = i;
        }
        
        if (triggerBuffering)
        {
            // shuffle the tile orders
            for (i = 0; i < out.numYTiles(); i++)
                std::swap(tileYs[i], tileYs[int(rand1.nextf(i,out.numYTiles()-1) + 0.5)]);

            for (i = 0; i < out.numXTiles(); i++)
                std::swap(tileXs[i], tileXs[int(rand1.nextf(i,out.numXTiles()-1) + 0.5)]);
        }

        for (int tileY = 0; tileY < out.numYTiles(); tileY++)
            for (int tileX = 0; tileX < out.numXTiles(); tileX++)
                out.writeTile (tileXs[tileX], tileYs[tileY]);
    }

    {
        cout << " reading" << flush;

        TiledInputFile in (fileName);

        const Box2i &dw = in.header().dataWindow();
        int w = dw.max.x - dw.min.x + 1;
        int h = dw.max.y - dw.min.y + 1;
        int dwx = dw.min.x;
        int dwy = dw.min.y;

        Array2D<half> ph2 (h, w);

        FrameBuffer fb;

        fb.insert ("H",
                   Slice (HALF,
                          (char *) &ph2[-dwy][-dwx],
                          sizeof (ph2[0][0]),
                          sizeof (ph2[0][0]) * w));

        in.setFrameBuffer (fb);
        
        int startTileY, endTileY;
        int dy;

        if ((lorder == DECREASING_Y && !triggerSeeks) ||
            (lorder == INCREASING_Y && triggerSeeks) ||
            (lorder == RANDOM_Y && triggerSeeks))
        {
            startTileY = in.numYTiles() - 1;
            endTileY = -1;

            dy = -1;
        }        
        else
        {
            startTileY = 0;
            endTileY = in.numYTiles();

            dy = 1;
        }
    
        for (int tileY = startTileY; tileY != endTileY; tileY += dy)
            for (int tileX = 0; tileX < in.numXTiles(); ++tileX)
                in.readTile (tileX, tileY);

        cout << " comparing" << flush;

        assert (in.header().displayWindow() == hdr.displayWindow());
        assert (in.header().dataWindow() == hdr.dataWindow());
        assert (in.header().pixelAspectRatio() == hdr.pixelAspectRatio());
        assert (in.header().screenWindowCenter() == hdr.screenWindowCenter());
        assert (in.header().screenWindowWidth() == hdr.screenWindowWidth());
        assert (in.header().lineOrder() == hdr.lineOrder());
        assert (in.header().compression() == hdr.compression());
        assert (in.header().channels() == hdr.channels());

        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
            assert (ph1[y][x] == ph2[y][x]);
    }

    remove (fileName);
    cout << endl;
}



void
writeCopyReadMIP (const char fileName[],
		  int width,
		  int height,
		  LineOrder lorder,
		  LevelRoundingMode rmode,
		  int xSize, 
		  int ySize,
		  Compression comp,
		  bool triggerBuffering,
		  bool triggerSeeks)
{
    cout << "LineOrder " << lorder << ", buffer " << triggerBuffering <<
            ", seek " << triggerSeeks << ", levelMode 1, " <<
	    "roundingMode " << rmode << ", "
	    "compression " << comp << endl;
    
    Header hdr ((Box2i (V2i (0, 0),			// display window
                        V2i (width - 1, height -1))),
                (Box2i (V2i (0, 0),		// data window
                        V2i (width - 1, height - 1))));

    hdr.compression() = comp;
    hdr.lineOrder() = INCREASING_Y;
    hdr.channels().insert ("H", Channel (HALF, 1, 1));
    
    hdr.setTileDescription(TileDescription(xSize, ySize, MIPMAP_LEVELS, rmode));
    
    Array < Array2D<half> > levels;

    {
        cout << " writing" << flush;

        remove (fileName);
        TiledOutputFile out (fileName, hdr);
        
        int numLevels = out.numLevels();
	levels.resizeErase (numLevels);

        int i;
        
        Rand32 rand1 = Rand32();
        std::vector<int> shuffled_levels = std::vector<int>(numLevels);
        
        for (i = 0; i < numLevels; i++)
            shuffled_levels[i] = i;

        if (triggerBuffering)
            // shuffle the level order
            for (i = 0; i < numLevels; i++)
                std::swap(shuffled_levels[i], shuffled_levels[int(rand1.nextf(i,numLevels-1) + 0.5)]);

        for (int level = 0; level < numLevels; ++level)
        {
            const int slevel = shuffled_levels[level];
            
            int levelWidth  = out.levelWidth(slevel);
            int levelHeight = out.levelHeight(slevel);
            levels[slevel].resizeErase(levelHeight, levelWidth);
            fillPixels (levels[slevel], levelWidth, levelHeight);

            FrameBuffer fb; 

            fb.insert ("H",
                       Slice (HALF,
                              (char *) &levels[slevel][0][0],
                              sizeof (levels[slevel][0][0]),
                              sizeof (levels[slevel][0][0]) * levelWidth));
        
            out.setFrameBuffer (fb);

            std::vector<int> tileYs = std::vector<int>(out.numYTiles(slevel));
            std::vector<int> tileXs = std::vector<int>(out.numXTiles(slevel));
            for (i = 0; i < out.numYTiles(slevel); i++)
            {
                if (lorder == DECREASING_Y)
                    tileYs[out.numYTiles(slevel)-1-i] = i;    
                else
                    tileYs[i] = i;
            }

            for (i = 0; i < out.numXTiles(slevel); i++)
                tileXs[i] = i;
            
            if (triggerBuffering)
            {
                // shuffle the tile orders
                for (i = 0; i < out.numYTiles(slevel); i++)
                    std::swap(tileYs[i], tileYs[int(rand1.nextf(i,out.numYTiles(slevel)-1) + 0.5)]);

                for (i = 0; i < out.numXTiles(slevel); i++)
                    std::swap(tileXs[i], tileXs[int(rand1.nextf(i,out.numXTiles(slevel)-1) + 0.5)]);
            }

            for (int tileY = 0; tileY < out.numYTiles(slevel); ++tileY)
                for (int tileX = 0; tileX < out.numXTiles(slevel); ++tileX)
                    out.writeTile (tileXs[tileX], tileYs[tileY], slevel);
        }
    }

    {
        cout << " reading" << flush;

        TiledInputFile in (fileName);

        const Box2i &dw = in.header().dataWindow();
        int dwx = dw.min.x;
        int dwy = dw.min.y;

        int numLevels = in.numLevels();
        Array < Array2D<half> > levels2 (numLevels);

        int startTileY, endTileY;
        int dy;
        
        for (int level = 0; level < in.numLevels(); ++level)
        {
            int levelWidth = in.levelWidth(level);
            int levelHeight = in.levelHeight(level);
            levels2[level].resizeErase(levelHeight, levelWidth);

            FrameBuffer fb; 

            fb.insert ("H",
                       Slice (HALF,
                              (char *) &levels2[level][-dwy][-dwx],
                              sizeof (levels2[level][0][0]),
                              sizeof (levels2[level][0][0]) * levelWidth));

            in.setFrameBuffer (fb);
            
            if ((lorder == DECREASING_Y && !triggerSeeks) ||
                (lorder == INCREASING_Y && triggerSeeks) ||
                (lorder == RANDOM_Y && triggerSeeks))
            {
                startTileY = in.numYTiles(level) - 1;
                endTileY = -1;

                dy = -1;
            }        
            else
            {
                startTileY = 0;
                endTileY = in.numYTiles(level);

                dy = 1;
            }
            
            for (int tileY = startTileY; tileY != endTileY; tileY += dy)
                for (int tileX = 0; tileX < in.numXTiles (level); ++tileX)
                    in.readTile (tileX, tileY, level);
        }

        cout << " comparing" << flush;

        assert (in.header().displayWindow() == hdr.displayWindow());
        assert (in.header().dataWindow() == hdr.dataWindow());
        assert (in.header().pixelAspectRatio() == hdr.pixelAspectRatio());
        assert (in.header().screenWindowCenter() == hdr.screenWindowCenter());
        assert (in.header().screenWindowWidth() == hdr.screenWindowWidth());
        assert (in.header().lineOrder() == hdr.lineOrder());
        assert (in.header().compression() == hdr.compression());
        assert (in.header().channels() == hdr.channels());

        for (int l = 0; l < numLevels; ++l)
            for (int y = 0; y < in.levelHeight(l); ++y)
                for (int x = 0; x < in.levelWidth(l); ++x)                    
                    assert ((levels2[l])[y][x] == (levels[l])[y][x]);
    }

    remove (fileName);
    cout << endl;
}


void
writeCopyReadRIP (const char fileName[],
		  int width,
		  int height,
		  LineOrder lorder,
		  LevelRoundingMode rmode,
		  int xSize, 
		  int ySize,
		  Compression comp,
		  bool triggerBuffering,
		  bool triggerSeeks)
{
    cout << "LineOrder " << lorder << ", buffer " << triggerBuffering <<
            ", seek " << triggerSeeks << ", levelMode 2, " <<
	    "roundingMode " << rmode << ", "
	    "compression " << comp << endl;
    
    Header hdr ((Box2i (V2i (0, 0),			// display window
                        V2i (width - 1, height -1))),
                (Box2i (V2i (0, 0),		// data window
                        V2i (width - 1, height - 1))));

    hdr.compression() = comp;
    hdr.lineOrder() = INCREASING_Y;
    hdr.channels().insert ("H", Channel (HALF, 1, 1));
    
    hdr.setTileDescription(TileDescription(xSize, ySize, RIPMAP_LEVELS, rmode));
    
    Array2D < Array2D<half> > levels;

    {
        cout << " writing" << flush;

        remove (fileName);
        TiledOutputFile out (fileName, hdr);
        
	levels.resizeErase (out.numYLevels(), out.numXLevels());
                                     
        int i;
        
        Rand32 rand1 = Rand32();
        std::vector<int> shuffled_xLevels = std::vector<int>(out.numXLevels());
        std::vector<int> shuffled_yLevels = std::vector<int>(out.numYLevels());
        
        for (i = 0; i < out.numXLevels(); i++)
            shuffled_xLevels[i] = i;
        
        for (i = 0; i < out.numYLevels(); i++)
            shuffled_yLevels[i] = i;

        if (triggerBuffering)
        {
            // shuffle the level orders
            for (i = 0; i < out.numXLevels(); i++)
                std::swap(shuffled_xLevels[i], shuffled_xLevels[int(rand1.nextf(i,out.numXLevels()-1) + 0.5)]);
                
            for (i = 0; i < out.numYLevels(); i++)
                std::swap(shuffled_yLevels[i], shuffled_yLevels[int(rand1.nextf(i,out.numYLevels()-1) + 0.5)]);
        }

        for (int ylevel = 0; ylevel < out.numYLevels(); ++ylevel)
        {
            const int sylevel = shuffled_yLevels[ylevel];
            
            std::vector<int> tileYs = std::vector<int>(out.numYTiles(sylevel));
            for (i = 0; i < out.numYTiles(sylevel); i++)
            {
                if (lorder == DECREASING_Y)
                    tileYs[out.numYTiles(sylevel)-1-i] = i;    
                else
                    tileYs[i] = i;
            }
            
            if (triggerBuffering)
                // shuffle the tile orders
                for (i = 0; i < out.numYTiles(sylevel); i++)
                    std::swap(tileYs[i], tileYs[int(rand1.nextf(i,out.numYTiles(sylevel)-1) + 0.5)]);
            
            for (int xlevel = 0; xlevel < out.numXLevels(); ++xlevel)
            {
                const int sxlevel = shuffled_xLevels[xlevel];
                
                int levelWidth = out.levelWidth(sxlevel);
                int levelHeight = out.levelHeight(sylevel);
                levels[sylevel][sxlevel].resizeErase(levelHeight, levelWidth);          
                fillPixels (levels[sylevel][sxlevel], levelWidth, levelHeight);
                
                FrameBuffer fb;
                fb.insert ("H",
                           Slice (HALF,
                                  (char *) &levels[sylevel][sxlevel][0][0],
                                  sizeof (levels[sylevel][sxlevel][0][0]),
                                  sizeof (levels[sylevel][sxlevel][0][0]) * levelWidth));

                out.setFrameBuffer (fb);
    
                std::vector<int> tileXs = std::vector<int>(out.numXTiles(sxlevel));
                for (i = 0; i < out.numXTiles(sxlevel); i++)
                    tileXs[i] = i;

                if (triggerBuffering)
                    // shuffle the tile orders
                    for (i = 0; i < out.numXTiles(sxlevel); i++)
                        std::swap(tileXs[i], tileXs[int(rand1.nextf(i,out.numXTiles(sxlevel)-1) + 0.5)]);
                
                for (int tileY = 0; tileY < out.numYTiles(sylevel); ++tileY)
                    for (int tileX = 0; tileX < out.numXTiles(sxlevel); ++tileX)
                        out.writeTile(tileXs[tileX], tileYs[tileY], sxlevel, sylevel);
            }
        }
    }

    {
        cout << " reading" << flush;

        TiledInputFile in (fileName);

        const Box2i &dw = in.header().dataWindow();
        int dwx = dw.min.x;
        int dwy = dw.min.y;

        int numXLevels = in.numXLevels();
        int numYLevels = in.numYLevels();
	Array2D < Array2D<half> > levels2 (numYLevels, numXLevels);

        int startTileY, endTileY;
        int dy;
        
        for (int ylevel = 0; ylevel < in.numYLevels(); ++ylevel)
        {
            if ((lorder == DECREASING_Y && !triggerSeeks) ||
                (lorder == INCREASING_Y && triggerSeeks) ||
                (lorder == RANDOM_Y && triggerSeeks))
            {
                startTileY = in.numYTiles(ylevel) - 1;
                endTileY = -1;

                dy = -1;
            }        
            else
            {
                startTileY = 0;
                endTileY = in.numYTiles(ylevel);

                dy = 1;
            }
            
            for (int xlevel = 0; xlevel < in.numXLevels(); ++xlevel)
            {
                int levelWidth  = in.levelWidth(xlevel);
                int levelHeight = in.levelHeight(ylevel);
                levels2[ylevel][xlevel].resizeErase(levelHeight, levelWidth);

                FrameBuffer fb;
                fb.insert ("H",
                           Slice (HALF,
                                  (char *) &levels2[ylevel][xlevel][-dwy][-dwx],
                                  sizeof (levels2[ylevel][xlevel][0][0]),
                                  sizeof (levels2[ylevel][xlevel][0][0]) * levelWidth));

                in.setFrameBuffer (fb);
                
                for (int tileY = startTileY; tileY != endTileY; tileY += dy)
                    for (int tileX = 0; tileX < in.numXTiles (xlevel); ++tileX)
                        in.readTile (tileX, tileY, xlevel, ylevel);
            }
        }

        cout << " comparing" << flush;

        assert (in.header().displayWindow() == hdr.displayWindow());
        assert (in.header().dataWindow() == hdr.dataWindow());
        assert (in.header().pixelAspectRatio() == hdr.pixelAspectRatio());
        assert (in.header().screenWindowCenter() == hdr.screenWindowCenter());
        assert (in.header().screenWindowWidth() == hdr.screenWindowWidth());
        assert (in.header().lineOrder() == hdr.lineOrder());
        assert (in.header().compression() == hdr.compression());
        assert (in.header().channels() == hdr.channels());

        for (int ly = 0; ly < numYLevels; ++ly)
            for (int lx = 0; lx < numXLevels; ++lx)
                for (int y = 0; y < in.levelHeight(ly); ++y)
                    for (int x = 0; x < in.levelWidth(lx); ++x)
                        assert ((levels2[ly][lx])[y][x] ==
                                (levels[ly][lx])[y][x]);
    }

    remove (fileName);
    cout << endl;
}


void
writeCopyRead (const std::string &tempDir, int w, int h, int xs, int ys)
{
    std::string filename = tempDir + "imf_test_copy.exr";

    for (int comp = 0; comp < NUM_COMPRESSION_METHODS; ++comp)
    {
	if (comp == B44_COMPRESSION ||
            comp == B44A_COMPRESSION)
        {
	    continue;
        }

        for (int lorder = 0; lorder < RANDOM_Y; ++lorder)
        {
	    for (int rmode = 0; rmode < NUM_ROUNDINGMODES; ++rmode)
	    {
		for (int tb = 0; tb <= 1; ++tb)
		{
		    for (int ts = 0; ts <= 1; ++ts)
		    {
			writeCopyReadONE (filename.c_str(), w, h,
					  (LineOrder)lorder,
					  (LevelRoundingMode) rmode,
					  xs, ys,
					  Compression (comp),
					  (bool)tb, (bool)ts);

			writeCopyReadMIP (filename.c_str(), w, h,
					  (LineOrder)lorder,
					  (LevelRoundingMode) rmode,
					  xs, ys,
					  Compression (comp),
					  (bool)tb, (bool)ts);

			writeCopyReadRIP (filename.c_str(), w, h,
					  (LineOrder)lorder,
					  (LevelRoundingMode) rmode,
					  xs, ys,
					  Compression (comp),
					  (bool)tb, (bool)ts);
		    }
		}
	    }
        }
    }
}

} // namespace


void
testTiledLineOrder (const std::string &tempDir)
{
    try
    {
        cout << "Testing line orders for tiled files and "
		"buffered/unbuffered tile writes" << endl;

        const int W = 171;
        const int H = 259;
        const int XS = 55;
        const int YS = 55;

	int maxThreads = ILMTHREAD_NAMESPACE::supportsThreads()? 3: 0;

	for (int n = 0; n <= maxThreads; ++n)
	{
	    if (ILMTHREAD_NAMESPACE::supportsThreads())
	    {
		setGlobalThreadCount (n);
		cout << "\nnumber of threads: " << globalThreadCount() << endl;
	    }

	    writeCopyRead (tempDir, W, H, XS, YS);
	}

        cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
        cerr << "ERROR -- caught exception: " << e.what() << endl;
        assert (false);
    }
}
