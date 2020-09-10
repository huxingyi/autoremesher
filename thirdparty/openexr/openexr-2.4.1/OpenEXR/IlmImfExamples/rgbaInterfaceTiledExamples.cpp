///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004, Industrial Light & Magic, a division of Lucas
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



//-----------------------------------------------------------------------------
//
//	Code examples that show how class TiledRgbaInputFile and
//	class TiledRgbaOutputFile can be used to read and write
//	OpenEXR image files with 16-bit floating-point red,
//	green, blue and alpha channels.
//
//-----------------------------------------------------------------------------

#include <ImfTiledRgbaFile.h>
#include <ImfStringAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfArray.h>

#include "drawImage.h"

#include <iostream>

#include "namespaceAlias.h"
using namespace IMF;
using namespace std;
using namespace IMATH_NAMESPACE;


void
writeTiledRgbaONE1 (const char fileName[],
                    const Rgba *pixels,
                    int width, int height,
                    int tileWidth, int tileHeight)
{
    //
    // Write a tiled image with one level using an image-sized framebuffer.
    //

    TiledRgbaOutputFile out (fileName,
                             width, height,		// image size
                             tileWidth, tileHeight,	// tile size
                             ONE_LEVEL,			// level mode
			     ROUND_DOWN,		// rounding mode
                             WRITE_RGBA);		// channels in file

    out.setFrameBuffer (pixels, 1, width);
    out.writeTiles (0, out.numXTiles() - 1, 0, out.numYTiles() - 1);
}


void
writeTiledRgbaONE2 (const char fileName[],
                    int width, int height,
                    int tileWidth, int tileHeight)
{
    //
    // Write a tiled image with one level using a tile-sized framebuffer.
    //

    TiledRgbaOutputFile out (fileName,
                             width, height,		// image size
                             tileWidth, tileHeight,	// tile size
                             ONE_LEVEL,			// level mode
			     ROUND_DOWN,		// rounding mode
                             WRITE_RGBA);		// channels in file

    Array2D<Rgba> pixels (tileHeight, tileWidth);

    for (int tileY = 0; tileY < out.numYTiles (); ++tileY)
    {
        for (int tileX = 0; tileX < out.numXTiles (); ++tileX)
        {
            Box2i range = out.dataWindowForTile (tileX, tileY);

            drawImage3 (pixels,
			width, height,
                        range.min.x, range.max.x + 1,
                        range.min.y, range.max.y + 1,
                        0, 0);

            out.setFrameBuffer (&pixels[-range.min.y][-range.min.x],
                                1,		// xStride
                                tileWidth);	// yStride

            out.writeTile (tileX, tileY);
        }
    }
}


void
writeTiledRgbaMIP1 (const char fileName[],
                    int width, int height,
                    int tileWidth, int tileHeight)
{
    //
    // Write a tiled image with mipmap levels using an image-sized framebuffer.
    //

    TiledRgbaOutputFile out (fileName,
                             width, height,		// image size
                             tileWidth, tileHeight,	// tile size
                             MIPMAP_LEVELS,		// level mode
			     ROUND_DOWN,		// rounding mode
                             WRITE_RGBA);		// channels in file

    Array2D<Rgba> pixels (height, width);
    out.setFrameBuffer (&pixels[0][0], 1, width);

    for (int level = 0; level < out.numLevels (); ++level)
    {
        drawImage4 (pixels,
		    out.levelWidth (level), out.levelHeight (level),
                    0, out.levelWidth (level),
		    0, out.levelHeight (level),
                    level, level);

	out.writeTiles (0, out.numXTiles (level) - 1,
			0, out.numYTiles (level) - 1,
			level);
    }
}


void
writeTiledRgbaMIP2 (const char fileName[],
                    int width, int height,
                    int tileWidth, int tileHeight)
{
    //
    // Write a tiled image with mipmap levels using a tile-sized framebuffer.
    //

    TiledRgbaOutputFile out (fileName,
                             width, height,		// image size
                             tileWidth, tileHeight,	// tile size
                             MIPMAP_LEVELS,		// level mode
			     ROUND_DOWN,		// rounding mode
                             WRITE_RGBA);		// channels in file

    Array2D<Rgba> pixels (tileHeight, tileWidth);

    for (int level = 0; level < out.numLevels (); ++level)
    {
        for (int tileY = 0; tileY < out.numYTiles (level); ++tileY)
        {
            for (int tileX = 0; tileX < out.numXTiles (level); ++tileX)
            {
                Box2i range = out.dataWindowForTile (tileX, tileY, level);

                drawImage4 (pixels,
			    out.levelWidth (level), out.levelHeight (level),
                            range.min.x, range.max.x + 1,
                            range.min.y, range.max.y + 1,
                            level, level);

                out.setFrameBuffer (&pixels[-range.min.y][-range.min.x],
                                    1,		// xStride
                                    tileWidth);	// yStride

                out.writeTile (tileX, tileY, level);
            }
        }
    }
}



void
writeTiledRgbaRIP1 (const char fileName[],
                    int width, int height,
                    int tileWidth, int tileHeight)
{
    //
    // Write a tiled image with ripmap levels using an image-sized framebuffer.
    //

    TiledRgbaOutputFile out (fileName,
                             width, height,		// image size
                             tileWidth, tileHeight,	// tile size
                             RIPMAP_LEVELS,		// level mode
			     ROUND_DOWN,		// rounding mode
                             WRITE_RGBA);		// channels in file

    Array2D<Rgba> pixels (height, width);
    out.setFrameBuffer (&pixels[0][0], 1, width);

    for (int yLevel = 0; yLevel < out.numYLevels (); ++yLevel)
    {
        for (int xLevel = 0; xLevel < out.numXLevels (); ++xLevel)
        {
            drawImage5 (pixels,
			out.levelWidth (xLevel), out.levelHeight (yLevel),
                        0, out.levelWidth (xLevel),
			0, out.levelHeight (yLevel),
			xLevel, yLevel);

	    out.writeTiles (0, out.numXTiles (xLevel) - 1,
			    0, out.numYTiles (yLevel) - 1,
			    xLevel,
			    yLevel);
        }
    }
}


void
writeTiledRgbaRIP2 (const char fileName[],
                    int width, int height,
                    int tileWidth, int tileHeight)
{
    //
    // Write a tiled image with ripmap levels using a tile-sized framebuffer.
    //

    TiledRgbaOutputFile out (fileName,
                             width, height,		// image size
                             tileWidth, tileHeight,	// tile size
                             RIPMAP_LEVELS,		// level mode
			     ROUND_DOWN,		// rounding mode
                             WRITE_RGBA);		// channels in file

    Array2D<Rgba> pixels (tileHeight, tileWidth);

    for (int yLevel = 0; yLevel < out.numYLevels(); ++yLevel)
    {
        for (int xLevel = 0; xLevel < out.numXLevels(); ++xLevel)
        {
            for (int tileY = 0; tileY < out.numYTiles (yLevel); ++tileY)
            {
                for (int tileX = 0; tileX < out.numXTiles (xLevel); ++tileX)
                {
                    Box2i range = out.dataWindowForTile (tileX, tileY,
							 xLevel, yLevel);

                    drawImage5 (pixels ,
				out.levelWidth(xLevel), out.levelHeight(yLevel),
                                range.min.x, range.max.x + 1,
                                range.min.y, range.max.y + 1,
                                xLevel, yLevel);

                    out.setFrameBuffer (&pixels[-range.min.y][-range.min.x],
                                        1,		// xStride
                                        tileWidth);	// yStride

                    out.writeTile (tileX, tileY, xLevel, yLevel);
                }
            }
        }
    }
}


void
readTiledRgba1 (const char fileName[],
                Array2D<Rgba> &pixels,
                int &width,
                int &height)
{
    TiledRgbaInputFile in (fileName);
    Box2i dw = in.dataWindow();

    width  = dw.max.x - dw.min.x + 1;
    height = dw.max.y - dw.min.y + 1;
    int dx = dw.min.x;
    int dy = dw.min.y;

    pixels.resizeErase (height, width);

    in.setFrameBuffer (&pixels[-dy][-dx], 1, width);
    in.readTiles (0, in.numXTiles() - 1, 0, in.numYTiles() - 1);
}


void
rgbaInterfaceTiledExamples ()
{
    cout << "\nRGBA tiled images\n" << endl;

    const int tw = 100;
    const int th = 75;
    int w = 600;
    int h = 400;

    cout << "writing tiled image with image-size framebuffer" << endl;

    Array2D<Rgba> pixels (h, w);
    drawImage3 (pixels, w, h, 0, w, 0, h, 0);
    writeTiledRgbaONE1 ("tiledrgba1.exr", &pixels[0][0], w, h, tw, th);

    cout << "writing tiled image with tile-size framebuffer" << endl;

    writeTiledRgbaONE2 ("tiledrgba2.exr", w, h, tw, th);

    cout << "writing tiled mipmap image with image-size framebuffer" << endl;

    writeTiledRgbaMIP1 ("tiledrgba3.exr", 512, 512, tw, th);

    cout << "writing tiled mipmap image with tile-size framebuffer" << endl;

    writeTiledRgbaMIP2 ("tiledrgba4.exr", 512, 512, tw, th);

    cout << "writing tiled ripmap image with image-size framebuffer" << endl;

    writeTiledRgbaRIP1 ("tiledrgba5.exr", 256, 256, tw, th);

    cout << "writing tiled ripmap image with tile-size framebuffer" << endl;

    writeTiledRgbaRIP2 ("tiledrgba6.exr", 256, 256, tw, th);

    cout << "reading tiled rgba file" << endl;

    readTiledRgba1 ("tiledrgba1.exr", pixels, w, h);
}
