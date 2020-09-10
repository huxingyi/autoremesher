///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006, Industrial Light & Magic, a division of Lucas
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
//	Apply CTL transforms
//
//-----------------------------------------------------------------------------

#include "applyCtl.h"

#if HAVE_CTL_INTERPRETER

    #include <ImfCtlApplyTransforms.h>
    #include <CtlSimdInterpreter.h>
    #include <ImfStandardAttributes.h>
    #include <ImfHeader.h>
    #include <ImfFrameBuffer.h>
    #include <stdlib.h>
    #include <cassert>
    #include <cstdio>
    #include <iostream>
    #include<stdio.h>

    using namespace std;
    using namespace Ctl;
    using namespace IMATH;

#else

    #include <ImfStandardAttributes.h>
    #include <ImfHeader.h>
    #include <stdlib.h>
    #include <cassert>
    #include <cstdio>
    #include <iostream>
    #include <stdio.h>

    using namespace std;
    using namespace IMATH;

#endif


using namespace IMF;

#define WARNING(message) (cerr << "Warning: " << message << endl)


float
displayVideoGamma ()
{
    //
    // Get the display's video gamma from an environment variable.
    // If this fails, use a default value (1/2.2).
    //

    const char gammaEnv[] = "EXR_DISPLAY_VIDEO_GAMMA";
    float g = 2.2f;

    if (const char *gamma = getenv (gammaEnv))
    {
	float tmp;
	int n = sscanf (gamma, " %f", &tmp);

	if (n != 1)
	    WARNING ("Cannot parse environment variable " << gammaEnv << "; "
		     "using default value (" << g << ").");
	else if (tmp < 1.f)
	    WARNING ("Display video gamma, specified in environment "
		     "variable " << gammaEnv << " is out of range; "
		     "using default value (" << g << ").");
	else
	    g = tmp;
    }
    else
    {
	    WARNING ("Environment variable " << gammaEnv << " is not set; "
		     "using default value (" << g << ").");
    }

    return 1.f / g;
}


namespace {

Chromaticities
displayChromaticities ()
{
    //
    // Get the chromaticities of the display's primaries and
    // white point from an environment variable.  If this fails,
    // assume chromaticities according to Rec. ITU-R BT.709.
    //

    static const char chromaticitiesEnv[] = "CTL_DISPLAY_CHROMATICITIES";
    Chromaticities c;  // default-initialized according to Rec. 709

    if (const char *chromaticities = getenv (chromaticitiesEnv))
    {
	Chromaticities tmp;

	int n = sscanf (chromaticities,
			" red %f %f green %f %f blue %f %f white %f %f",
			&tmp.red.x, &tmp.red.y,
			&tmp.green.x, &tmp.green.y,
			&tmp.blue.x, &tmp.blue.y,
			&tmp.white.x, &tmp.white.y);

	if (n == 8)
	    c = tmp;
	else
	    WARNING ("Cannot parse environment variable " <<
		     chromaticitiesEnv << "; using default value "
		     "(chromaticities according to Rec. ITU-R BT.709).");
    }
    else
    {
	WARNING ("Environment variable " << chromaticitiesEnv << " is "
		 "not set; using default value (chromaticities according "
		 "to Rec. ITU-R BT.709).");
    }

    return c;
}

} // namespace

#if HAVE_CTL_INTERPRETER

namespace {

void
initializeEnvHeader (Header &envHeader)
{
    //
    // Initialize the "environment header" for the CTL
    // transforms by adding displayChromaticities,
    // displayWhiteLuminance and displaySurroundLuminance
    // attributes.
    //

    Chromaticities c = displayChromaticities();
    envHeader.insert ("displayChromaticities", ChromaticitiesAttribute (c));

    //
    // Get the display's white luminance from an environment variable.
    // If this fails, assume 120 candelas per square meter.
    // (Screen aim luminance according to SMPTE RP 166.)
    //

    static const char whiteLuminanceEnv[] = "CTL_DISPLAY_WHITE_LUMINANCE";
    static const float whiteLuminanceDefault = 120.0;
    float wl = whiteLuminanceDefault;

    if (const char *whiteLuminance = getenv (whiteLuminanceEnv))
    {
	int n = sscanf (whiteLuminance, " %f", &wl);

	if (n != 1)
	    WARNING ("Cannot parse environment variable " <<
		     whiteLuminanceEnv << "; using default value "
		     "(" << wl << " candelas per square meter).");
    }
    else
    {
	    WARNING ("Environment variable " << whiteLuminanceEnv << "  is "
		     "is not set; using default value (" << wl << " candelas "
		     "per square meter).");
    }

    envHeader.insert ("displayWhiteLuminance", FloatAttribute (wl));

    //
    // Get the display's surround luminance from an environment variable.
    // If this fails, assume 10% of the display's white luminance.
    // (Recommended setup according to SMPTE RP 166.)
    //

    static const char surroundLuminanceEnv[] = "CTL_DISPLAY_SURROUND_LUMINANCE";
    float sl = wl * 0.1f;

    if (const char *surroundLuminance = getenv (surroundLuminanceEnv))
    {
	int n = sscanf (surroundLuminance, " %f", &sl);

	if (n != 1)
	    WARNING ("Cannot parse environment variable " <<
		     surroundLuminanceEnv << "; using default value "
		     "(" << sl << " candelas per square meter).");
    }
    else
    {
	    WARNING ("Environment variable " << surroundLuminanceEnv << "  is "
		     "is not set; using default value (" << sl << " candelas "
		     "per square meter).");
    }

    envHeader.insert ("displaySurroundLuminance", FloatAttribute (sl));
}


string
displayTransformName ()
{
    //
    // Get the name of the display transform from an environment
    // variable.  If this fails, use a default name.
    //

    static const char displayTransformEnv[] = "CTL_DISPLAY_TRANSFORM";
    static const char displayTransformDefault[] = "transform_display_video";
    const char *displayTransform = getenv (displayTransformEnv);

    if (!displayTransform)
    {
	displayTransform = displayTransformDefault;

	WARNING ("Environment variable " << displayTransformEnv << " "
		 "is not set; using default value "
		 "(\"" << displayTransform << "\").");
    }

    return displayTransform;
}


void
initializeInFrameBuffer
    (int w,
     int h,
     const Array<Rgba> &pixels,
     FrameBuffer &fb)
{
    fb.insert ("R", Slice (HALF,			// type
			   (char *)(&pixels[0].r),	// base
			   sizeof (pixels[0]),		// xStride
			   sizeof (pixels[0]) * w));	// yStride

    fb.insert ("G", Slice (HALF,			// type
			   (char *)(&pixels[0].g),	// base
			   sizeof (pixels[0]),		// xStride
			   sizeof (pixels[0]) * w));	// yStride

    fb.insert ("B", Slice (HALF,			// type
			   (char *)(&pixels[0].b),	// base
			   sizeof (pixels[0]),		// xStride
			   sizeof (pixels[0]) * w));	// yStride
}


void
initializeOutFrameBuffer
    (int w,
     int h,
     const Array<Rgba> &pixels,
     FrameBuffer &fb)
{
    fb.insert ("R_display", Slice (HALF,			// type
				   (char *)(&pixels[0].r),	// base
				   sizeof (pixels[0]),		// xStride
				   sizeof (pixels[0]) * w));	// yStride

    fb.insert ("G_display", Slice (HALF,			// type
				   (char *)(&pixels[0].g),	// base
				   sizeof (pixels[0]),		// xStride
				   sizeof (pixels[0]) * w));	// yStride

    fb.insert ("B_display", Slice (HALF,			// type
				   (char *)(&pixels[0].b),	// base
				   sizeof (pixels[0]),		// xStride
				   sizeof (pixels[0]) * w));	// yStride
}

} // namespace


void
applyCtl (vector<string> transformNames,
	  Header inHeader,
	  const Array<Rgba> &inPixels,
	  int w,
	  int h,
	  Array<Rgba> &outPixels)
{
    //
    // If we do not have an explicit set of transform names
    // then find suitable look modification, rendering and
    // display transforms.
    //

    if (transformNames.empty())
    {
	if (hasLookModTransform (inHeader))
	    transformNames.push_back (lookModTransform (inHeader));

	if (hasRenderingTransform (inHeader))
	    transformNames.push_back (renderingTransform (inHeader));
	else
	    transformNames.push_back ("transform_RRT");

	transformNames.push_back (displayTransformName());
    }

    //
    // Initialize an input and an environment header:
    // Make sure that the headers contain information about the primaries
    // and the white point of the image files an the display, and about
    // the display's white luminance and surround luminance.
    //

    if (!hasChromaticities (inHeader))
	addChromaticities (inHeader, Chromaticities());

    if (!hasAdoptedNeutral (inHeader))
	addAdoptedNeutral (inHeader, chromaticities(inHeader).white);

    Header envHeader;
    initializeEnvHeader (envHeader);

    //
    // Set up input and output FrameBuffer objects for the transforms.
    //

    FrameBuffer inFb;
    initializeInFrameBuffer (w, h, inPixels, inFb);

    FrameBuffer outFb;
    initializeOutFrameBuffer (w, h, outPixels, outFb);

    //
    // Run the CTL transforms
    //

    Box2i transformWindow (V2i (0, 0), V2i (w - 1, h - 1));

    SimdInterpreter interpreter;

    #ifdef CTL_MODULE_BASE_PATH

	//
	// The configuration scripts has defined a default
	// location for CTL modules.  Include this location
	// in the CTL module search path.
	//

	vector<string> paths = interpreter.modulePaths();
	paths.push_back (CTL_MODULE_BASE_PATH);
	interpreter.setModulePaths (paths);

    #endif

    Header outHeader;

    ImfCtl::applyTransforms (interpreter,
			     transformNames,
			     transformWindow,
			     envHeader,
			     inHeader,
			     inFb,
			     outHeader,
			     outFb);
}

#endif


void
adjustChromaticities (const Header &header,
		      const Array<Rgba> &inPixels,
		      int w, int h,
		      Array<Rgba> &outPixels)
{
    Chromaticities fileChroma;  // default-initialized according to Rec. 709

    if (hasChromaticities (header))
	fileChroma = chromaticities (header);

    Chromaticities displayChroma = displayChromaticities();

    if (fileChroma.red   == displayChroma.red &&
	fileChroma.green == displayChroma.green &&
	fileChroma.blue  == displayChroma.blue &&
	fileChroma.white == displayChroma.white)
    {
	return;
    }

    M44f M = RGBtoXYZ (fileChroma, 1) * XYZtoRGB (displayChroma, 1);

    size_t numPixels = w * h;

    for (size_t i = 0; i < numPixels; ++i)
    {
	const Rgba &in = inPixels[i];
	Rgba &out = outPixels[i];

	V3f rgb = V3f (in.r, in.g, in.b) * M;

	out.r = rgb[0];
	out.g = rgb[1];
	out.b = rgb[2];
    }
}
