///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002, Industrial Light & Magic, a division of Lucas
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
//	Code examples that show how class TiledInputFile and
//	class TiledOutputFile can be used to read and write
//	OpenEXR image files with an arbitrary set of channels.
//
//-----------------------------------------------------------------------------
#include <ImfTiledOutputFile.h>
#include <ImfTiledInputFile.h>
#include <ImfChannelList.h>
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
writeTiled1 (const char fileName[],
             Array2D<GZ> &pixels,
             int width, int height,
             int tileWidth, int tileHeight)
{
    Header header (width, height);
    header.channels().insert ("G", Channel (IMF::HALF));
    header.channels().insert ("Z", Channel (IMF::FLOAT));

    header.setTileDescription
	(TileDescription (tileWidth, tileHeight, ONE_LEVEL));
    
    TiledOutputFile out (fileName, header);

    FrameBuffer frameBuffer;

    frameBuffer.insert ("G",					 // name
                        Slice (IMF::HALF,			 // type
			       (char *) &pixels[0][0].g,	 // base
				sizeof (pixels[0][0]) * 1,	 // xStride
				sizeof (pixels[0][0]) * width)); // yStride

    frameBuffer.insert ("Z",					 // name
                        Slice (IMF::FLOAT,			 // type
			       (char *) &pixels[0][0].z,	 // base
				sizeof (pixels[0][0]) * 1,	 // xStride
				sizeof (pixels[0][0]) * width)); // yStride

    out.setFrameBuffer (frameBuffer);
    out.writeTiles (0, out.numXTiles() - 1, 0, out.numYTiles() - 1);
}


void
readTiled1 (const char fileName[],
            Array2D<GZ> &pixels,
            int &width, int &height)
{
    TiledInputFile in (fileName);

    Box2i dw = in.header().dataWindow();
    width  = dw.max.x - dw.min.x + 1;
    height = dw.max.y - dw.min.y + 1;
    int dx = dw.min.x;
    int dy = dw.min.y;

    pixels.resizeErase (height, width);

    FrameBuffer frameBuffer;

    frameBuffer.insert ("G",					 // name
                        Slice (IMF::HALF,			 // type
			       (char *) &pixels[-dy][-dx].g,	 // base
				sizeof (pixels[0][0]) * 1,	 // xStride
				sizeof (pixels[0][0]) * width)); // yStride

    frameBuffer.insert ("Z",					 // name
                        Slice (IMF::FLOAT,			 // type
			       (char *) &pixels[-dy][-dx].z,	 // base
				sizeof (pixels[0][0]) * 1,	 // xStride
				sizeof (pixels[0][0]) * width)); // yStride

    in.setFrameBuffer (frameBuffer);
    in.readTiles (0, in.numXTiles() - 1, 0, in.numYTiles() - 1);
}


void
generalInterfaceTiledExamples ()
{
    cout << "\nGZ (green, depth) tiled files\n" << endl;
    cout << "drawing image" << endl;

    int w = 800;
    int h = 600;

    Array2D<GZ>  pixels (h, w);
    drawImage6 (pixels, w, h);

    cout << "writing file" << endl;

    writeTiled1 ("tiledgz1.exr", pixels, w, h, 64, 64);

    cout << "reading file" << endl;

    readTiled1 ("tiledgz1.exr", pixels, w, h);
}
