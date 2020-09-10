#ifndef INCLUDED_APPLY_CTL_H
#define INCLUDED_APPLY_CTL_H

//////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005, Industrial Light & Magic, a division of Lucasfilm
// Entertainment Company Ltd.  Portions contributed and copyright held by
// others as indicated.  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above
//       copyright notice, this list of conditions and the following
//       disclaimer.
//
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided with
//       the distribution.
//
//     * Neither the name of Industrial Light & Magic nor the names of
//       any other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//
//	Apply CTL transforms
//
//----------------------------------------------------------------------------
#include "namespaceAlias.h"

#include <ImfRgba.h>
#include <ImfArray.h>
#include <ImfHeader.h>


#include <vector>
#include <cstdlib>

//
// Apply a series of CTL transforms to the raw pixel data from an image file
// in order to generate pixel data that can be displayed on the screen.
//
//	transformNames	A list of the names of the CTL transforms that will
//			be applied to raw pixels.  If this list is empty,
//			applyCtl() looks for a rendering transform and a
//			display transform:
//
//			If inHeader contains a string attribute called
//			"renderingTransform" then the value of this attribute
//			is the name of the rendering transform.
//			If inHeader contains no such attribute, then the
//			name of the rendering transform is "transform_RRT".
//
//			If the environment variable CTL_DISPLAY_TRANSFORM
//			is set, the value of the environment variable is
//			the name of the display transform.
//			If the environment variable is not set, then the name
//			of the display transform is "transform_display_video".
//
//	inHeader	The header of the image file.
//
//	inPixels	Raw pixels from the image file.
//
//	w, h		Width and height of the inPixels and outPixels
//			arrays.  (The caller must set the size of the
//			arrays to w*h, before calling applyCtl().)
//
//	outPixels	Output -- pixels for display on the screen.
//			The data in outPixels represent relative
//			luminance values; an R, G or B value of 1.0
//			represents the maximum red, green or blue
//			luminance that the display can achieve.
//			

void	applyCtl (std::vector<std::string> transformNames,
		  IMF::Header inHeader,
		  const IMF::Array<IMF::Rgba> &inPixels,
		  int w,
		  int h,
		  IMF::Array<IMF::Rgba> &outPixels);


//
// If the chromaticities of the RGB pixel loaded from a file
// are not the same as the chromaticities of the display,
// then transform the pixels from the RGB coordinate system
// of the file to the RGB coordinate system of the display.
//

void	adjustChromaticities (const IMF::Header &header,
			      const IMF::Array<IMF::Rgba> &inPixels,
			      int w,
			      int h,
			      IMF::Array<IMF::Rgba> &outPixels);


//
// Get the gamma factor that must be used in order to convert luminance
// values output by applyCtl() into display frame buffer values.
//

float	displayVideoGamma ();


#endif
