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


#ifndef INCLUDED_SCALE_IMAGE_H
#define INCLUDED_SCALE_IMAGE_H

//----------------------------------------------------------------------------
//
//	Image scaling and filtering functions.
//
//----------------------------------------------------------------------------
#include "namespaceAlias.h"

#include <ImfRgba.h>
#include <ImfArray.h>



//
// Scale an image horizontally or vertically, by a factor of
// approximately f (f is adjusted slightly so that the corners
// of the display window and the data window fall on integer
// pixel locations).
//
// f		scale factor; must be >= 1.0
//
// w, h		width and height of the display window
//
// dw, dh	width and height of the data window
//
// dx, dy	offset of the data window's upper left
// 		corner from the display window's upper
// 		left corner
//
// pixels	the image's pixel array
//

void	scaleX (float f,
		int &w, int &h,
		int &dw, int &dh,
		int &dx, int &dy,
		IMF::Array<IMF::Rgba> &pixels);

void	scaleY (float f, 
		int &w, int &h, 
		int &dw, int &dh,
		int &dx, int &dy,
		IMF::Array<IMF::Rgba> &pixels);

//
// Normalize the pixel values in an image so that the smallest
// value becomes 0.0 and the largest value becomes 1.0.
//

void	normalizePixels (int dw, int dh, IMF::Array<IMF::Rgba> &pixels);


//
// Swap the left and right half of and image; then swap the
// top and bottom half, so that the four corners of the image
// end up in the center.
//

void	swapPixels (int dw, int dh, IMF::Array<IMF::Rgba> &pixels);


#endif
