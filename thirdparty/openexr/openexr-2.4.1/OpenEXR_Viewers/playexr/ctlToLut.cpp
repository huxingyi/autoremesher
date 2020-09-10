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
//	Run a set of CTL transforms to generate a color lookup table.
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include "ctlToLut.h"

#if HAVE_CTL_INTERPRETER

    #include <ImfCtlApplyTransforms.h>
    #include <CtlSimdInterpreter.h>
    #include <ImfStandardAttributes.h>
    #include <ImfHeader.h>
    #include <stdlib.h>
    #include <ImfFrameBuffer.h>
    #include <cassert>
    #include <iostream>

    using namespace std;
    using namespace Ctl;
    
    using namespace OPENEXR_IMF_NAMESPACE;
    using namespace IMATH_NAMESPACE;

#else

    #include <ImfStandardAttributes.h>
    #include <ImfHeader.h>
    #include <stdlib.h>
    #include <cassert>
    #include <iostream>

    using namespace std;
    
    using namespace OPENEXR_IMF_NAMESPACE;
    using namespace IMATH_NAMESPACE;

#endif


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

} // namespace


void
ctlToLut (vector<string> transformNames,
	  Header inHeader,
	  size_t lutSize,
	  const half pixelValues[/*lutSize*/],
	  half lut[/*lutSize*/])
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

    Header envHeader;
    Header outHeader;

    if (!hasChromaticities (inHeader))
	addChromaticities (inHeader, Chromaticities());

    if (!hasAdoptedNeutral (inHeader))
	addAdoptedNeutral (inHeader, chromaticities(inHeader).white);

    initializeEnvHeader (envHeader);

    //
    // Set up input and output FrameBuffer objects for the CTL transforms.
    //

    assert (lutSize % 4 == 0);

    FrameBuffer inFb;

    inFb.insert ("R",
		 Slice (HALF,				// type
		        (char *)pixelValues,		// base
			4 * sizeof (half),		// xStride
			0));				// yStride

    inFb.insert ("G",
		 Slice (HALF,				// type
			(char *)(pixelValues + 1),	// base
			4 * sizeof (half),		// xStride
			0));				// yStride

    inFb.insert ("B",
		 Slice (HALF,				// type
			(char *)(pixelValues + 2),	// base
			4 * sizeof (half),		// xStride
			0));				// yStride

    FrameBuffer outFb;

    outFb.insert ("R_display",
		  Slice (HALF,				// type
			 (char *)lut,			// base
			 4 * sizeof (half),		// xStride
			 0));				// yStride

    outFb.insert ("G_display",
		  Slice (HALF,				// type
			 (char *)(lut + 1),		// base
			 4 * sizeof (half),		// xStride
			 0));				// yStride

    outFb.insert ("B_display",
		  Slice (HALF,				// type
			 (char *)(lut + 2),		// base
			 4 * sizeof (half),		// xStride
			 0));				// yStride

    //
    // Run the CTL transforms.
    //

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

    ImfCtl::applyTransforms (interpreter,
			     transformNames,
			     Box2i (V2i (0, 0), V2i (lutSize / 4 - 1, 0)),
			     envHeader,
			     inHeader,
			     inFb,
			     outHeader,
			     outFb);
}


#else

#include <ImfStandardAttributes.h>
#include <ImfHeader.h>
#include <cassert>
#include <iostream>


using namespace std;

using namespace OPENEXR_IMF_NAMESPACE;
using namespace IMATH_NAMESPACE;


#define WARNING(message) (cerr << "Warning: " << message << endl)


void
ctlToLut (vector<string> transformNames,
	  Header inHeader,
	  size_t lutSize,
	  const half pixelValues[/*lutSize*/],
	  half lut[/*lutSize*/])
{
    //
    // This program has been compiled without CTL support.
    //
    // Our fallback solution is to build a lookup table that
    // performs a coordinate transform from the primaries and
    // white point of the input files to the primaries and
    // white point of the display.
    //

    //
    // Get the input file chromaticities
    //

    Chromaticities fileChroma;

    if (hasChromaticities (inHeader))
	fileChroma = chromaticities (inHeader);

    //
    // Get the display chromaticities
    //

    static const char chromaticitiesEnv[] = "CTL_DISPLAY_CHROMATICITIES";
    Chromaticities displayChroma;

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
	    displayChroma = tmp;
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

    //
    // Do the coordinate transform
    //

    M44f M = RGBtoXYZ (fileChroma, 1) * XYZtoRGB (displayChroma, 1);

    assert (lutSize % 4 == 0);

    for (size_t i = 0; i < lutSize; i += 4)
    {
	V3f rgb (pixelValues[i], pixelValues[i + 1], pixelValues[i + 2]);
	rgb = rgb * M;

	lut[i + 0] = rgb[0];
	lut[i + 1] = rgb[1];
	lut[i + 2] = rgb[2];
	lut[i + 3] = 0;
    }
}

#endif
