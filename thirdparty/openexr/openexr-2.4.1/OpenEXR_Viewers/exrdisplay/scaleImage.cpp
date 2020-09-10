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
//	Image scaling and filtering functions.
//
//----------------------------------------------------------------------------

#include "scaleImage.h"

#include <ImathLimits.h>
#include <ImathFun.h>
#include <algorithm>
#include <math.h>


using namespace IMF;
using namespace IMATH;
using std::min;
using std::max;


int
scaleInt (float f, int i)
{
    return int (f * i + 0.5);
}


void
scaleX (float f, 
	int &w, int &h,
	int &dw, int &dh,
	int &dx, int &dy,
	Array<Rgba> &pixels)
{
    int dw1 = scaleInt (f, dw);

    if (dw1 <= dw)
    {
	//
	// Do nothing if the width of the data window
	// woudn't increase by at least one pixel.
	//

	return;
    }

    w = scaleInt (f, w);
    dx = scaleInt (f, dx);

    //
    // Copy the pixels to a temporary array
    //

    Array<Rgba> tmp (dw * dh);
    
    for (int i = 0; i < dw * dh; ++i)
	tmp[i] = pixels[i];

    //
    // Resize the original pixel array,
    // and copy the pixels back into the
    // resized array.
    //

    pixels.resizeErase (dw1 * dh);

    f = float (dw - 1) / float (dw1 - 1);

    for (int x = 0; x < dw1; ++x)
    {
	float x1 = x * f;
	int xs = int (x1);
	int xt = min (xs + 1, dw - 1);
	float t = x1 - xs;
	float s = 1 - t;

	for (int y = 0; y < dh; ++y)
	{
	    const Rgba &ps = tmp [y * dw + xs];
	    const Rgba &pt = tmp [y * dw + xt];
	    Rgba &p = pixels[y * dw1 + x];

	    p.r = ps.r * s + pt.r * t;
	    p.g = ps.g * s + pt.g * t;
	    p.b = ps.b * s + pt.b * t;
	    p.a = ps.a * s + pt.a * t;
	}
    }

    dw = dw1;
}


void
scaleY (float f, 
	int &w, int &h, 
	int &dw, int &dh,
	int &dx, int &dy,
	Array<Rgba> &pixels)
{
    int dh1 = scaleInt (f, dh);

    if (dh1 <= dh)
    {
	//
	// Do nothing if the height of the data window
	// woudn't increase by at least one pixel.
	//

	return;
    }

    h = scaleInt (f, h);
    dy = scaleInt (f, dy);

    //
    // Copy the pixels to a temporary array
    //

    Array<Rgba> tmp (dw * dh);
    
    for (int i = 0; i < dw * dh; ++i)
	tmp[i] = pixels[i];

    //
    // Resize the original pixel array,
    // and copy the pixels back into the
    // resized array.
    //

    pixels.resizeErase (dw * dh1);

    f = float (dh - 1) / float (dh1 - 1);

    for (int y = 0; y < dh1; ++y)
    {
	float y1 = y * f;
	int ys = int (y1);
	int yt = min (ys + 1, dh - 1);
	float t = y1 - ys;
	float s = 1 - t;

	for (int x = 0; x < dw; ++x)
	{
	    const Rgba &ps = tmp [ys * dw + x];
	    const Rgba &pt = tmp [yt * dw + x];
	    Rgba &p = pixels[y * dw + x];

	    p.r = ps.r * s + pt.r * t;
	    p.g = ps.g * s + pt.g * t;
	    p.b = ps.b * s + pt.b * t;
	    p.a = ps.a * s + pt.a * t;
	}
    }

    dh = dh1;
}


void
normalizePixels (int dw, int dh, Array<Rgba> &pixels)
{
    float pMax = -IMATH::limits<float>::max ();
    float pMin =  IMATH::limits<float>::max ();

    for (int i = 0; i < dw * dh; ++i)
    {
	const Rgba &p = pixels[i];

	if (p.r.isFinite())
	{
	    pMax = max (float (p.r), pMax);
	    pMin = min (float (p.r), pMin);
	}

	if (p.g.isFinite())
	{
	    pMax = max (float (p.g), pMax);
	    pMin = min (float (p.g), pMin);
	}

	if (p.b.isFinite())
	{
	    pMax = max (float (p.b), pMax);
	    pMin = min (float (p.b), pMin);
	}
    }

    if (pMax <= pMin)
	pMax = pMin + 1;

    for (int i = 0; i < dw * dh; ++i)
    {
	Rgba &p = pixels[i];

	if (p.r.isFinite())
	    p.r = (p.r - pMin) / (pMax - pMin);

	if (p.g.isFinite())
	    p.g = (p.g - pMin) / (pMax - pMin);

	if (p.b.isFinite())
	    p.b = (p.b - pMin) / (pMax - pMin);
    }
}


void
swapPixels (int dw, int dh, Array<Rgba> &pixels)
{
    Array<Rgba> tmp (max (dw, dh));

    int dw2 = dw / 2;
    int dh2 = dh / 2;

    //
    // Swap top and bottom half
    //

    for (int x = 0; x < dw; ++x)
    {
	for (int y = 0; y < dh; ++y)
	    tmp[(y + dh2) % dh] = pixels[dw * y + x];

	for (int y = 0; y < dh; ++y)
	    pixels[dw * y + x] = tmp[y];
    }

    //
    // Swap left and right half
    //

    for (int y = 0; y < dh; ++y)
    {
	for (int x = 0; x < dw; ++x)
	    tmp[(x + dw2) % dw] = pixels[dw * y + x];

	for (int x = 0; x < dw; ++x)
	    pixels[dw * y + x] = tmp[x];
    }
}
