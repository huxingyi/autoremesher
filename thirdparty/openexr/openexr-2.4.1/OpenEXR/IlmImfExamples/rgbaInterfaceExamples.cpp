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
//	Code examples that show how class RgbaInputFile and
//	class RgbaOutputFile can be used to read and write
//	OpenEXR image files with 16-bit floating-point red,
//	green, blue and alpha channels.
//
//-----------------------------------------------------------------------------

#include <ImfRgbaFile.h>
#include <ImfStringAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfArray.h>

#include "drawImage.h"

#include <iostream>
#include <algorithm>

#include "namespaceAlias.h"
using namespace IMF;
using namespace std;
using namespace IMATH_NAMESPACE;


void
writeRgba1 (const char fileName[],
	    const Rgba *pixels,
	    int width,
	    int height)
{
    //
    // Write an RGBA image using class RgbaOutputFile.
    //
    //	- open the file
    //	- describe the memory layout of the pixels
    //	- store the pixels in the file
    //


    RgbaOutputFile file (fileName, width, height, WRITE_RGBA);
    file.setFrameBuffer (pixels, 1, width);
    file.writePixels (height);
}


void
writeRgba2 (const char fileName[],
	    const Rgba *pixels,
	    int width,
	    int height,
	    const Box2i &dataWindow)
{
    //
    // Write an RGBA image using class RgbaOutputFile.
    // Don't store the whole image in the file, but
    // crop it according to the given data window.
    //
    //	- open the file
    //	- describe the memory layout of the pixels
    //	- store the pixels in the file
    //

    Box2i displayWindow (V2i (0, 0), V2i (width - 1, height - 1));
    RgbaOutputFile file (fileName, displayWindow, dataWindow, WRITE_RGBA);
    file.setFrameBuffer (pixels, 1, width);
    file.writePixels (dataWindow.max.y - dataWindow.min.y + 1);
}


void
writeRgba3 (const char fileName[],
	    const Rgba *pixels,
	    int width,
	    int height,
	    const char comments[],
	    const M44f &cameraTransform)
{
    //
    // Write an RGBA image using class RgbaOutputFile.
    // Store two extra attributes in the image header:
    // a string and a 4x4 transformation matrix.
    //
    //	- open the file
    //	- describe the memory layout of the pixels
    //	- store the pixels in the file
    //

    Header header (width, height);
    header.insert ("comments", StringAttribute (comments));
    header.insert ("cameraTransform", M44fAttribute (cameraTransform));

    RgbaOutputFile file (fileName, header, WRITE_RGBA);
    file.setFrameBuffer (pixels, 1, width);
    file.writePixels (height);
}


void
readRgba1 (const char fileName[],
	   Array2D<Rgba> &pixels,
	   int &width,
	   int &height)
{
    //
    // Read an RGBA image using class RgbaInputFile:
    //
    //	- open the file
    //	- allocate memory for the pixels
    //	- describe the memory layout of the pixels
    //	- read the pixels from the file
    //

    RgbaInputFile file (fileName);
    Box2i dw = file.dataWindow();

    width  = dw.max.x - dw.min.x + 1;
    height = dw.max.y - dw.min.y + 1;
    pixels.resizeErase (height, width);

    file.setFrameBuffer (&pixels[0][0] - dw.min.x - dw.min.y * width, 1, width);
    file.readPixels (dw.min.y, dw.max.y);
}


void
readRgba2 (const char fileName[])
{
    //
    // Read an RGBA image using class RgbaInputFile.
    // Read the pixels, 10 scan lines at a time, and
    // store the pixel data in a buffer that is just
    // large enough to hold 10 scan lines worth of data.
    //
    //	- open the file
    //	- allocate memory for the pixels
    //	- for each block of 10 scan lines,
    //	  describe the memory layout of the pixels,
    //	  read the pixels from the file,
    //	  process the pixels and discard them
    //

    RgbaInputFile file (fileName);
    Box2i dw = file.dataWindow();

    int width  = dw.max.x - dw.min.x + 1;

    Array2D<Rgba> pixels (10, width);

    while (dw.min.y <= dw.max.y)
    {
	file.setFrameBuffer (&pixels[0][0] - dw.min.x - dw.min.y * width,
			     1, width);

	file.readPixels (dw.min.y, min (dw.min.y + 9, dw.max.y));
	// processPixels (pixels)
	
	dw.min.y += 10;
    }
}


void
readHeader (const char fileName[])
{
    //
    // Read an image's header from a file, and if the header
    // contains comments and camera transformation attributes,
    // print the values of those attributes.
    //
    //	- open the file
    //	- get the file header
    //	- look for the attributes
    //

    RgbaInputFile file (fileName);

    const StringAttribute *comments =
	file.header().findTypedAttribute <StringAttribute> ("comments");

    const M44fAttribute *cameraTransform = 
	file.header().findTypedAttribute <M44fAttribute> ("cameraTransform");

    if (comments)
	cout << "comments\n   " << comments->value() << endl;

    if (cameraTransform)
	cout << "cameraTransform\n" << cameraTransform->value() << flush;
}


void
rgbaInterfaceExamples ()
{
    cout << "\nRGBA images\n" << endl;
    cout << "drawing image" << endl;

    int w = 800;
    int h = 600;

    Array2D<Rgba> p (h, w);
    drawImage1 (p, w, h);

    cout << "writing entire image" << endl;

    writeRgba1 ("rgba1.exr", &p[0][0], w, h);

    cout << "writing cropped image" << endl;

    writeRgba2 ("rgba2.exr", &p[0][0], w, h,
	        Box2i (V2i (w/6, h/6), V2i (w/2, h/2)));

    cout << "writing image with extra header attributes" << endl;

    writeRgba3 ("rgba3.exr", &p[0][0], w, h,
	        "may contain peanuts", M44f());

    cout << "reading rgba file" << endl;

    readRgba1 ("rgba2.exr", p, w, h);

    cout << "reading rgba file into 10-scanline buffer" << endl;

    readRgba2 ("rgba2.exr");

    cout << "reading extra file header attributes" << endl;

    readHeader ("rgba3.exr");
}
