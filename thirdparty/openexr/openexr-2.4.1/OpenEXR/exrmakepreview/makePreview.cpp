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


//----------------------------------------------------------------------------
//
//	Add a preview image to an OpenEXR file.
//
//----------------------------------------------------------------------------


#include "makePreview.h"

#include <ImfInputFile.h>
#include <ImfOutputFile.h>
#include <ImfTiledOutputFile.h>
#include <ImfRgbaFile.h>
#include <ImfPreviewImage.h>
#include <ImfArray.h>
#include <ImathMath.h>
#include <ImathFun.h>
#include <math.h>
#include <iostream>
#include <algorithm>

#include <OpenEXRConfig.h>
using namespace OPENEXR_IMF_NAMESPACE;
using namespace IMATH_NAMESPACE;
using namespace std;


namespace {

float
knee (float x, float f)
{
    return log (x * f + 1) / f;
}


unsigned char
gamma (half h, float m)
{
    //
    // Conversion from half to unsigned char pixel data,
    // with gamma correction.  The conversion is the same
    // as in the exrdisplay program's ImageView class,
    // except with defog, kneeLow, and kneeHigh fixed
    // at 0.0, 0.0, and 5.0 respectively.
    //

    float x = max (0.f, h * m);

    if (x > 1)
	x = 1 + knee (x - 1, 0.184874f);

    return (unsigned char) (IMATH_NAMESPACE::clamp (Math<float>::pow (x, 0.4545f) * 84.66f, 
				   0.f,
				   255.f));
}


void
generatePreview (const char inFileName[],
		 float exposure,
		 int previewWidth,
		 int &previewHeight,
		 Array2D <PreviewRgba> &previewPixels)
{
    //
    // Read the input file
    //

    RgbaInputFile in (inFileName);

    Box2i dw = in.dataWindow();
    float a = in.pixelAspectRatio();
    int w = dw.max.x - dw.min.x + 1;
    int h = dw.max.y - dw.min.y + 1;

    Array2D <Rgba> pixels (h, w);
    in.setFrameBuffer (ComputeBasePointer (&pixels[0][0], dw), 1, w);
    in.readPixels (dw.min.y, dw.max.y);

    //
    // Make a preview image
    //

    previewHeight = max (int (h / (w * a) * previewWidth + .5f), 1);
    previewPixels.resizeErase (previewHeight, previewWidth);

    double fx = (previewWidth  > 1)? (double (w - 1) / (previewWidth  - 1)): 1;
    double fy = (previewHeight > 1)? (double (h - 1) / (previewHeight - 1)): 1;
    float m  = Math<float>::pow (2.f, IMATH_NAMESPACE::clamp (exposure + 2.47393f, -20.f, 20.f));

    for (int y = 0; y < previewHeight; ++y)
    {
	for (int x = 0; x < previewWidth; ++x)
	{
	    PreviewRgba &preview = previewPixels[y][x];
	    const Rgba &pixel = pixels[int (y * fy + .5f)][int (x * fx + .5f)];

	    preview.r = gamma (pixel.r, m);
	    preview.g = gamma (pixel.g, m);
	    preview.b = gamma (pixel.b, m);
	    preview.a = int (IMATH_NAMESPACE::clamp (pixel.a * 255.f, 0.f, 255.f) + .5f);
	}
    }
}

} // namespace


void
makePreview (const char inFileName[],
	     const char outFileName[],
	     int previewWidth,
	     float exposure,
	     bool verbose)
{
    if (verbose)
	cout << "generating preview image" << endl;

    Array2D <PreviewRgba> previewPixels;
    int previewHeight;

    generatePreview (inFileName,
		     exposure,
		     previewWidth,
		     previewHeight,
		     previewPixels);

    InputFile in (inFileName);
    Header header = in.header();

    header.setPreviewImage
	(PreviewImage (previewWidth, previewHeight, &previewPixels[0][0]));

    if (verbose)
	cout << "copying " << inFileName << " to " << outFileName << endl;

    if (header.hasTileDescription())
    {
	TiledOutputFile out (outFileName, header);
	out.copyPixels (in);
    }
    else
    {
	OutputFile out (outFileName, header);
	out.copyPixels (in);
    }

    if (verbose)
	cout << "done." << endl;
}
