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

//----------------------------------------------------------------------------
//
//	Play an OpenEXR image sequence.
//
//	This is the display thread of the playExr program.
//	It does the following:
//
//	* Reads the first frame in the image sequence to find out how
//	  big the images are and what channels they contain.
//
//	* Allocates a ring buffer for receiving images from the file
//	  file reading thread.
//
//	* Creates an OpenGL window for displaying the images.
//
//	* Launches a file reading thread, which reads the frames in
//	  the image sequence and passes them to the display thread.
//
//	* Enters an infinite loop: get the next frame from the
//	  ring buffer, display the frame.
//
//----------------------------------------------------------------------------

#include "playExr.h"
#include "ctlToLut.h"
#include "fileNameForFrame.h"
#include "FileReadingThread.h"
#include "osDependent.h"
#include "ImageBuffers.h"
#include "Timer.h"
#include <ImfThreading.h>
#include <ImfInputFile.h>
#include <ImfChannelList.h>
#include <ImfStandardAttributes.h>
#include <ImfRgbaYca.h>
#include <ImfArray.h>

#include <half.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>
#include <cmath>


using namespace OPENEXR_IMF_NAMESPACE;
using namespace IMATH_NAMESPACE;
using namespace std;
using namespace ILMTHREAD_NAMESPACE;

namespace {

//
// Static variables.
//

ImageBuffers ib;		// Ring buffer; transports frames from
				// the file reading thread to the display
				// thread.

int i = 0;			// Index of current frame buffer

int frameNumber = 0;		// Frame number of current frame

int firstFrameNumber = 0;	// Frame number of first and last frame
int lastFrameNumber = 0;	// in the sequence

Timer timer;			// Timing control

GLuint texId[3];		// For display, a frame's three image channels
				// are converted to OpenGL textures, eigher
				// one three-channel texture or three one-
				// channel textures.  texId contains the
				// names by which OpenGL refers to those
				// textures.

int glWindowWidth = 0;		// Preferred width and height of the on-screen
int glWindowHeight = 0;		// window where the images will be displayed

Box2i drawRect;			// The size and location of the images' data
				// window relative to the on-screen window.

V3f yWeights (1, 1, 1);		// Weights for converting the pixels in
				// luminance chroma channels to RGB,
				// computed from the chromaticities of
				// the frames' primaries and white point

float exposure = 0;		// Current exposure setting.  All pixels
				// are multiplied by pow(2,exposure) before
				// they appear on the screen

bool enableCtl = true;		// If enableCtl is true, CTL transforms
				// are applied to the pixels after exposure
				// has been applied.  If enableCtl is false,
				// no CTL transforms are applied.

const size_t lutN = 64;		// The 3D color lookup table that is used
				// to approximate the CTL transforms has
				// lutN by lutN by lutN entries.

bool hwTexInterpolation = true;	// Flag that controls whether the Cg shader
				// that performs 3D color table lookups
				// relies on hardware-interpolated texture
				// lookups or if the shader itself interpolates
				// between texture samples

bool showTextOverlay = true;	// Flag that controls whether the actual
				// frame rate and the current exposure
				// setting are displayed

bool fullScreenMode = false;	// Flag that controls whether the images
				// are displayed in full-screen mode or not


//
// Initialization of the ring buffer, ib.
// We allocate space for the pixels of ib.numBuffers() frames,
// and we initialize Imf::FrameBuffer objects that allow the
// the file reading thread to fill the pixel buffers.
// 

char *
addSlice
    (FrameBuffer &fb,
     const Box2i &dw,
     const char name[],
     int xSampling,
     int ySampling)
{
    int w = dw.max.x - dw.min.x + 1;
    int h = dw.max.y - dw.min.y + 1;
    size_t pixelSize = sizeof (half);
    size_t lineSize = pixelSize * (w / xSampling);
    size_t numLines = h / ySampling;

    char *pixels = new char [lineSize * numLines];

    fb.insert (name,
	       Slice (HALF,
		      pixels - (dw.min.y / ySampling) * lineSize -
			       (dw.min.x / xSampling) * pixelSize,
		      pixelSize,
		      lineSize,
		      xSampling,
		      ySampling));

    return pixels;
}
     

void
initializeImageBuffers
    (ImageBuffers &ib,
     Header &header,
     const char fileNameTemplate[])
{
    InputFile
	in (fileNameForFrame (fileNameTemplate, firstFrameNumber).c_str());

    header = in.header();

    const ChannelList &ch = in.header().channels();
    const Box2i &dw = in.header().dataWindow();

    ib.dataWindow = dw;

    if (ch.findChannel ("Y") || ch.findChannel ("RY") || ch.findChannel ("BY"))
    {
	//
	// Luminance/chroma mode
	//
	// The image channels go into three separate pixel buffers.
	// The buffer for the luminance channel has the same width and
	// height as the frame.  The buffers for the two chroma channels
	// have half the width and half the height of the frame.
	//

	ib.rgbMode = false;

	for (int i = 0; i < ib.numBuffers(); ++i)
	{
	    FrameBuffer &fb = ib.frameBuffer (i);
	    ib.pixels (i, 0) = addSlice (fb, dw, "Y", 1, 1);
	    ib.pixels (i, 1) = addSlice (fb, dw, "RY", 2, 2);
	    ib.pixels (i, 2) = addSlice (fb, dw, "BY", 2, 2);
	}

	Chromaticities chroma;

	if (hasChromaticities (in.header()))
	    chroma = chromaticities (in.header());

	yWeights = RgbaYca::computeYw (chroma);
    }
    else
    {
	//
	// RGB mode
	//
	// The pixel buffers for the tree image channels (RGB)
	// are padded with a fourth dummy channel (A) and interleaved
	// (RGBARGBARGBA...).  All three buffers have the same width
	// and height as the frame.
	//

	ib.rgbMode = true;

	for (int i = 0; i < ib.numBuffers(); ++i)
	{
	    FrameBuffer &fb = ib.frameBuffer (i);

	    int w = dw.max.x - dw.min.x + 1;
	    int h = dw.max.y - dw.min.y + 1;
	    size_t pixelSize = sizeof (half) * 4;
	    size_t lineSize = pixelSize * w;

	    char *pixels = new char [lineSize * h];

	    ib.pixels (i, 0) = pixels;
	    ib.pixels (i, 1) = pixels + sizeof (half);
	    ib.pixels (i, 2) = pixels + sizeof (half) * 2;

	    fb.insert ("R",
		       Slice (HALF,
			      ib.pixels (i, 0) - dw.min.y * lineSize -
					         dw.min.x * pixelSize,
			      pixelSize,
			      lineSize,
			      1, 1));

	    fb.insert ("G",
		       Slice (HALF,
			      ib.pixels (i, 1) - dw.min.y * lineSize -
					         dw.min.x * pixelSize,
			      pixelSize,
			      lineSize,
			      1, 1));

	    fb.insert ("B",
		       Slice (HALF,
			      ib.pixels (i, 2) - dw.min.y * lineSize -
					         dw.min.x * pixelSize,
			      pixelSize,
			      lineSize,
			      1, 1));
	}
    }
}


//
// Compute the size of the window on the screen where the image
// sequence will be displayed, and the size and location of the
// images within that window.
//

void
computeWindowSizes
    (Box2i &dataWindow,
     Box2i &displayWindow,
     float pixelAspectRatio,
     float xyScale)
{
    //
    // Beginning with the data and display window of the first frame
    // in the image sequence, translate both windows so that the upper
    // left corner of the display window is at coordinates (0,0) in
    // OpenEXR's pixel space (with y going from top to bottom).
    //

    dataWindow.min -= displayWindow.min;
    dataWindow.max -= displayWindow.min;
    displayWindow.max -= displayWindow.min;
    displayWindow.min = V2i (0, 0);

    //
    // If the pixel aspect is not 1.0, stretch the display
    // and data window so that the pixels become square.
    //

    if (pixelAspectRatio < 1.0)
    {
	dataWindow.min.y =
	    int (floor (dataWindow.min.y / pixelAspectRatio + 0.5));

	dataWindow.max.y =
	    int (floor (dataWindow.max.y / pixelAspectRatio + 0.5));

	displayWindow.max.y =
	    int (floor (displayWindow.max.y / pixelAspectRatio + 0.5));
    }

    if (pixelAspectRatio > 1.0)
    {
	dataWindow.min.x =
	    int (floor (dataWindow.min.x * pixelAspectRatio + 0.5));

	dataWindow.max.x =
	    int (floor (dataWindow.max.x * pixelAspectRatio + 0.5));

	displayWindow.max.x =
	    int (floor (displayWindow.max.x * pixelAspectRatio + 0.5));
    }

    //
    // The size of the OpenGL window on the screen is equal to the
    // size of the (possibly stretched) display window.
    //

    glWindowWidth  = displayWindow.max.x + 1;
    glWindowHeight = displayWindow.max.y + 1;

    //
    // The size and location of the image within the OpenGL window
    // is determined by the (possibly stretched) data window.
    // The data window must be transformed from OpenEXR pixel space
    // to OpenGL coordinates (with y going from bottom to top).
    //

    drawRect.min.x = dataWindow.min.x;
    drawRect.min.y = displayWindow.max.y - dataWindow.max.y;

    drawRect.max.x = dataWindow.max.x + 1;
    drawRect.max.y = displayWindow.max.y - dataWindow.min.y + 1;

    //
    // The user may have requested that the images be
    // displayed smaller or larger than their original size.
    //

    glWindowWidth  = int (floor (glWindowWidth  * xyScale + 0.5));
    glWindowHeight = int (floor (glWindowHeight * xyScale + 0.5));

    drawRect.min.x = int (floor (drawRect.min.x * xyScale + 0.5));
    drawRect.min.y = int (floor (drawRect.min.y * xyScale + 0.5));
    drawRect.max.x = int (floor (drawRect.max.x * xyScale + 0.5));
    drawRect.max.y = int (floor (drawRect.max.y * xyScale + 0.5));
}


//
// Cg shaders.  For each frame, the drawFrame() function, below, draws
// a big rectangle that fills the entire OpenGL window.  The current
// frame is stored in one or three textures; a Cg shader projects the
// textures onto the rectangle, making the frame appear in the window.
//

CGcontext cgContext;
CGprogram cgProgram;
CGprofile cgProfile;


void
handleCgErrors ()
{
    cerr << cgGetErrorString (cgGetError()) << endl;
    cerr << cgGetLastListing (cgContext) << endl;
    exit (1);
}


//
// Shader for luminance/chroma images:
// R, G and B are computed from the full-resolution Y (luminance)
// channel and the half-resolution RY and BY (chroma) channels.
//

const char shaderLuminanceChromaSource[] =
"                                                                          \n"
"    struct Out                                                            \n"
"    {                                                                     \n"
"        half3 pixel: COLOR;                                               \n"
"    };                                                                    \n"
"                                                                          \n"
"    Out                                                                   \n"
"    main (float2 tc: TEXCOORD0,                                           \n"
"          uniform sampler2D yImage: TEXUNIT0,                             \n"
"          uniform sampler2D ryImage: TEXUNIT1,                            \n"
"          uniform sampler2D byImage: TEXUNIT2,                            \n"
"          uniform sampler3D lut: TEXUNIT3,                                \n"
"          uniform float3 yw,                                              \n"
"          uniform float expMult,                                          \n"
"          uniform float videoGamma,                                       \n"
"          uniform float lutMin,                                           \n"
"          uniform float lutMax,                                           \n"
"          uniform float lutM,                                             \n"
"          uniform float lutT,                                             \n"
"          uniform float lutF,                                             \n"
"          uniform float enableLut)                                        \n"
"    {                                                                     \n"
"        //                                                                \n"
"        // Sample luminance and chroma, convert to RGB.                   \n"
"        //                                                                \n"
"                                                                          \n"
"        half Y  =  tex2D (yImage, tc).r;                                  \n"
"        half RY =  tex2D (ryImage, tc).r;                                 \n"
"        half BY =  tex2D (byImage, tc).r;                                 \n"
"                                                                          \n"
"        float r = (RY + 1) * Y;                                           \n"
"        float b = (BY + 1) * Y;                                           \n"
"        float g = (Y - r * yw.x - b * yw.z) / yw.y;                       \n"
"                                                                          \n"
"        //                                                                \n"
"        // Apply exposure                                                 \n"
"        //                                                                \n"
"                                                                          \n"
"        half3 color = half3 (r, g, b) * expMult;                          \n"
"                                                                          \n"
"        //                                                                \n"
"        // Apply 3D color lookup table (in log space).                    \n"
"        //                                                                \n"
"                                                                          \n"
"        if (enableLut)                                                    \n"
"	 {                                                                 \n"
"            if (lutF)                                                     \n"
"            {                                                             \n"
"                //                                                        \n"
"                // Texture hardware does not support                      \n"
"                // interpolation between texture samples.                 \n"
"                //                                                        \n"
"                                                                          \n"
"                half3 i = lutF * half3                                    \n"
"                    (lutT + lutM * log (clamp (color, lutMin, lutMax)));  \n"
"                                                                          \n"
"                half3 fi = floor (i);                                     \n"
"                half3 fj = fi + 1;                                        \n"
"                half3 s = i - fi;                                         \n"
"                                                                          \n"
"                fi = fi / lutF;                                           \n"
"                fj = fj / lutF;                                           \n"
"                                                                          \n"
"                half3 c0 = tex3D (lut, half3 (fi.x, fi.y, fi.z)).rgb;     \n"
"                half3 c1 = tex3D (lut, half3 (fj.x, fi.y, fi.z)).rgb;     \n"
"                half3 c2 = tex3D (lut, half3 (fi.x, fj.y, fi.z)).rgb;     \n"
"                half3 c3 = tex3D (lut, half3 (fj.x, fj.y, fi.z)).rgb;     \n"
"                half3 c4 = tex3D (lut, half3 (fi.x, fi.y, fi.z)).rgb;     \n"
"                half3 c5 = tex3D (lut, half3 (fj.x, fi.y, fj.z)).rgb;     \n"
"                half3 c6 = tex3D (lut, half3 (fi.x, fj.y, fj.z)).rgb;     \n"
"                half3 c7 = tex3D (lut, half3 (fj.x, fj.y, fj.z)).rgb;     \n"
"                                                                          \n"
"                color = ((c0 * (1-s.x) + c1 * s.x) * (1-s.y) +            \n"
"                         (c2 * (1-s.x) + c3 * s.x) *  s.y) * (1-s.z) +    \n"
"                        ((c4 * (1-s.x) + c5 * s.x) * (1-s.y) +            \n"
"                         (c6 * (1-s.x) + c7 * s.x) *  s.y) * s.z;         \n"
"                                                                          \n"
"                color = exp (color);                                      \n"
"            }                                                             \n"
"            else                                                          \n"
"            {                                                             \n"
"                //                                                        \n"
"                // Texture hardware supports trilinear                    \n"
"                // interpolation between texture samples.                 \n"
"                //                                                        \n"
"                                                                          \n"
"                color = lutT + lutM * log (clamp (color, lutMin, lutMax));\n"
"                color = exp (tex3D (lut, color).rgb);                     \n"
"            }                                                             \n"
"        }                                                                 \n"
"                                                                          \n"
"        //                                                                \n"
"        // Apply video gamma correction.                                  \n"
"        //                                                                \n"
"                                                                          \n"
"        Out output;                                                       \n"
"        output.pixel = pow (color, videoGamma);                           \n"
"        return output;                                                    \n"
"    }                                                                     \n"
"                                                                          \n";


void
initShaderLuminanceChroma
    (float lutMin,
     float lutMax,
     float lutM,
     float lutT)
{
    cgSetErrorCallback (handleCgErrors);

    cgContext = cgCreateContext();
    cgProfile = cgGLGetLatestProfile (CG_GL_FRAGMENT);
    cgGLSetOptimalOptions (cgProfile);

    cgProgram =
	cgCreateProgram (cgContext, CG_SOURCE, shaderLuminanceChromaSource,
		         cgProfile, "main", 0);

    cgGLLoadProgram (cgProgram);
    cgGLBindProgram (cgProgram);
    
    cgGLEnableProfile (cgProfile);

    CGparameter ywParam = cgGetNamedParameter (cgProgram, "yw");
    cgSetParameter3f (ywParam, yWeights.x, yWeights.y, yWeights.z);

    CGparameter emParam = cgGetNamedParameter (cgProgram, "expMult");
    cgSetParameter1f (emParam, pow (2.0f, exposure));

    CGparameter vgParam = cgGetNamedParameter (cgProgram, "videoGamma");
    cgSetParameter1f (vgParam, displayVideoGamma());

    CGparameter lutMinParam = cgGetNamedParameter (cgProgram, "lutMin");
    cgSetParameter1f (lutMinParam, lutMin);

    CGparameter lutMaxParam = cgGetNamedParameter (cgProgram, "lutMax");
    cgSetParameter1f (lutMaxParam, lutMax);

    CGparameter lutMParam = cgGetNamedParameter (cgProgram, "lutM");
    cgSetParameter1f (lutMParam, lutM);

    CGparameter lutTParam = cgGetNamedParameter (cgProgram, "lutT");
    cgSetParameter1f (lutTParam, lutT);

    CGparameter enableLutParam = cgGetNamedParameter (cgProgram, "enableLut");
    cgSetParameter1f (enableLutParam, enableCtl? 1.0: 0.0);

    CGparameter lutFParam = cgGetNamedParameter (cgProgram, "lutF");
    cgSetParameter1f (lutFParam, hwTexInterpolation? 0: lutN - 1);
}


//
// Shader for RGB images
//

const char shaderRgbSource[] =
"                                                                          \n"
"    struct Out                                                            \n"
"    {                                                                     \n"
"        half3 pixel: COLOR;                                               \n"
"    };                                                                    \n"
"                                                                          \n"
"    Out                                                                   \n"
"    main (float2 tc: TEXCOORD0,                                           \n"
"          uniform sampler2D rgbImage: TEXUNIT0,                           \n"
"          uniform sampler3D lut: TEXUNIT3,                                \n"
"          uniform float expMult,                                          \n"
"          uniform float videoGamma,                                       \n"
"          uniform float lutMin,                                           \n"
"          uniform float lutMax,                                           \n"
"          uniform float lutM,                                             \n"
"          uniform float lutT,                                             \n"
"          uniform float lutF,                                             \n"
"          uniform float enableLut)                                        \n"
"    {                                                                     \n"
"        //                                                                \n"
"        // Sample RGB image, apply exposure.                              \n"
"        //                                                                \n"
"                                                                          \n"
"        half3 color = tex2D (rgbImage, tc).rgb * expMult;                 \n"
"                                                                          \n"
"        //                                                                \n"
"        // Apply 3D color lookup table (in log space).                    \n"
"        //                                                                \n"
"                                                                          \n"
"        if (enableLut)                                                    \n"
"	 {                                                                 \n"
"            if (lutF)                                                     \n"
"            {                                                             \n"
"                //                                                        \n"
"                // Texture hardware does not support                      \n"
"                // interpolation between texture samples.                 \n"
"                //                                                        \n"
"                                                                          \n"
"                half3 i = lutF * half3                                    \n"
"                    (lutT + lutM * log (clamp (color, lutMin, lutMax)));  \n"
"                                                                          \n"
"                half3 fi = floor (i);                                     \n"
"                half3 fj = fi + 1;                                        \n"
"                half3 s = i - fi;                                         \n"
"                                                                          \n"
"                fi = fi / lutF;                                           \n"
"                fj = fj / lutF;                                           \n"
"                                                                          \n"
"                half3 c0 = tex3D (lut, half3 (fi.x, fi.y, fi.z)).rgb;     \n"
"                half3 c1 = tex3D (lut, half3 (fj.x, fi.y, fi.z)).rgb;     \n"
"                half3 c2 = tex3D (lut, half3 (fi.x, fj.y, fi.z)).rgb;     \n"
"                half3 c3 = tex3D (lut, half3 (fj.x, fj.y, fi.z)).rgb;     \n"
"                half3 c4 = tex3D (lut, half3 (fi.x, fi.y, fi.z)).rgb;     \n"
"                half3 c5 = tex3D (lut, half3 (fj.x, fi.y, fj.z)).rgb;     \n"
"                half3 c6 = tex3D (lut, half3 (fi.x, fj.y, fj.z)).rgb;     \n"
"                half3 c7 = tex3D (lut, half3 (fj.x, fj.y, fj.z)).rgb;     \n"
"                                                                          \n"
"                color = ((c0 * (1-s.x) + c1 * s.x) * (1-s.y) +            \n"
"                         (c2 * (1-s.x) + c3 * s.x) *  s.y) * (1-s.z) +    \n"
"                        ((c4 * (1-s.x) + c5 * s.x) * (1-s.y) +            \n"
"                         (c6 * (1-s.x) + c7 * s.x) *  s.y) * s.z;         \n"
"                                                                          \n"
"                color = exp (color);                                      \n"
"            }                                                             \n"
"            else                                                          \n"
"            {                                                             \n"
"                //                                                        \n"
"                // Texture hardware supports trilinear                    \n"
"                // interpolation between texture samples.                 \n"
"                //                                                        \n"
"                                                                          \n"
"                color = lutT + lutM * log (clamp (color, lutMin, lutMax));\n"
"                color = exp (tex3D (lut, color).rgb);                     \n"
"            }                                                             \n"
"        }                                                                 \n"
"                                                                          \n"
"        //                                                                \n"
"        // Apply video gamma correction.                                  \n"
"        //                                                                \n"
"                                                                          \n"
"        Out output;                                                       \n"
"        output.pixel = pow (color, videoGamma);                           \n"
"        return output;                                                    \n"
"    }                                                                     \n"
"                                                                          \n";


void
initShaderRgb
    (float lutMin,
     float lutMax,
     float lutM,
     float lutT)
{
    cgSetErrorCallback (handleCgErrors);

    cgContext = cgCreateContext();
    cgProfile = cgGLGetLatestProfile (CG_GL_FRAGMENT);
    cgGLSetOptimalOptions (cgProfile);

    cgProgram =
	cgCreateProgram (cgContext, CG_SOURCE, shaderRgbSource,
		         cgProfile, "main", 0);

    cgGLLoadProgram (cgProgram);
    cgGLBindProgram (cgProgram);
    cgGLEnableProfile (cgProfile);

    CGparameter emParam = cgGetNamedParameter (cgProgram, "expMult");
    cgSetParameter1f (emParam, pow (2.0f, exposure));

    CGparameter vgParam = cgGetNamedParameter (cgProgram, "videoGamma");
    cgSetParameter1f (vgParam, displayVideoGamma());

    CGparameter lutMinParam = cgGetNamedParameter (cgProgram, "lutMin");
    cgSetParameter1f (lutMinParam, lutMin);

    CGparameter lutMaxParam = cgGetNamedParameter (cgProgram, "lutMax");
    cgSetParameter1f (lutMaxParam, lutMax);

    CGparameter lutMParam = cgGetNamedParameter (cgProgram, "lutM");
    cgSetParameter1f (lutMParam, lutM);

    CGparameter lutTParam = cgGetNamedParameter (cgProgram, "lutT");
    cgSetParameter1f (lutTParam, lutT);

    CGparameter enableLutParam = cgGetNamedParameter (cgProgram, "enableLut");
    cgSetParameter1f (enableLutParam, enableCtl? 1.0: 0.0);

    CGparameter lutFParam = cgGetNamedParameter (cgProgram, "lutF");
    cgSetParameter1f (lutFParam, hwTexInterpolation? 0: lutN - 1);
}


//
// GL drawing code
//

void
checkGlErrors (const char where[])
{
    GLenum error = glGetError();

    if (error != GL_NO_ERROR)
    {
	cerr << where << ": " << gluErrorString (error) << endl;
	exit (1);
    }
}


void
handleReshape (int w, int h)
{
    int xOffset = (w - glWindowWidth) / 2;
    int yOffset = (h - glWindowHeight) / 2;

    glViewport (xOffset, yOffset, glWindowWidth, glWindowHeight);
    glScissor (xOffset, yOffset, glWindowWidth, glWindowHeight);

    checkGlErrors ("handleReshape");
}


void
initTexturesLuminanceChroma ()
{
    const Box2i &dw = ib.dataWindow;
    int w = dw.max.x - dw.min.x + 1;
    int h = dw.max.y - dw.min.y + 1;

    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    glGenTextures (3, texId);

    glActiveTexture (GL_TEXTURE0);
    glBindTexture (GL_TEXTURE_2D, texId[0]);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D (GL_TEXTURE_2D,
		  0,			// level
		  GL_LUMINANCE16F_ARB,	// internalFormat
		  w, h, 
		  0,			// border
		  GL_LUMINANCE,		// format
		  GL_HALF_FLOAT_ARB,	// type
		  0);			// pixels

    glActiveTexture (GL_TEXTURE1);
    glBindTexture (GL_TEXTURE_2D, texId[1]);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D (GL_TEXTURE_2D,
		  0,			// level
		  GL_LUMINANCE16F_ARB,	// internalFormat
		  w / 2, h / 2, 
		  0,			// border
		  GL_LUMINANCE,		// format
		  GL_HALF_FLOAT_ARB,	// type
		  0);			// pixels

    glActiveTexture (GL_TEXTURE2);
    glBindTexture (GL_TEXTURE_2D, texId[2]);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D (GL_TEXTURE_2D,
		  0,			// level
		  GL_LUMINANCE16F_ARB,	// internalFormat
		  w / 2, h / 2, 
		  0,			// border
		  GL_LUMINANCE,		// format
		  GL_HALF_FLOAT_ARB,	// type
		  0);			// pixels

    checkGlErrors ("initTexturesLuminanceChroma");
}


void
initTexturesRgb ()
{
    const Box2i &dw = ib.dataWindow;
    int w = dw.max.x - dw.min.x + 1;
    int h = dw.max.y - dw.min.y + 1;

    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    glGenTextures (1, texId);

    glActiveTexture (GL_TEXTURE0);
    glBindTexture (GL_TEXTURE_2D, texId[0]);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D (GL_TEXTURE_2D,
		  0,			// level
		  GL_RGBA16F_ARB,	// internalFormat
		  w, h, 
		  0,			// border
		  GL_RGBA,		// format
		  GL_HALF_FLOAT_ARB,	// type
		  0);			// pixels

    checkGlErrors ("initTexturesRgb");
}


void
init3DLut
    (const vector<string> &transformNames, 	// names of CTL transforms
     const Header &header,			// header of first frame
     float &lutMin,
     float &lutMax,
     float &lutM,
     float &lutT)
{
    //
    // We build a 3D color lookup table by running a set of color
    // samples through a series of CTL transforms.
    //
    // The 3D lookup table covers a range from lutMin to lutMax or
    // NUM_STOPS f-stops above and below 0.18 or MIDDLE_GRAY.  The
    // size of the table is lutN by lutN by lutN samples.
    //
    // In order make the distribution of the samples in the table
    // approximately perceptually uniform, the Cg shaders that use
    // the table perform lookups in "log space":
    // In a Cg shader, the lookup table is represented as a 3D texture.
    // In order to apply the table to a pixel value, the Cg shader takes
    // the logarithm of the pixel value and scales and offsets the result
    // so that lutMin and lutMax map to 0 and 1 respectively.  The scaled
    // value is used to perform a texture lookup and the shader computes
    // e raised to the power of the result of the texture lookup.
    //

    //
    // Compute lutMin, lutMax, and scale and offset
    // values, lutM and lutT, so that
    //
    //	lutM * lutMin + lutT == 0
    //	lutM * lutMax + lutT == 1
    //

    static const int NUM_STOPS = 7;
    static const float MIDDLE_GRAY = 0.18;

    lutMin = MIDDLE_GRAY / (1 << NUM_STOPS);
    lutMax = MIDDLE_GRAY * (1 << NUM_STOPS);

    float logLutMin = log (lutMin);
    float logLutMax = log (lutMax);

    lutM = 1 / (logLutMax - logLutMin);
    lutT = -lutM * logLutMin;

    size_t LUT_SIZE = lutN * lutN * lutN * 4;

    //
    // Build a 3D array of RGB input pixel values.
    // such that R, G and B are between lutMin and lutMax.
    //

    Array<half> pixelValues (LUT_SIZE);

    for (size_t ib = 0; ib < lutN; ++ib)
    {
	float b = ib / (lutN - 1.0);
	half B = exp ((b - lutT) / lutM);

	for (size_t ig = 0; ig < lutN; ++ig)
	{
	    float g = ig / (lutN - 1.0);
	    half G = exp ((g - lutT) / lutM);

	    for (size_t ir = 0; ir < lutN; ++ir)
	    {
		float r = ir / (lutN - 1.0);
		half R = exp ((r - lutT) / lutM);

		size_t i = (ib * lutN * lutN + ig * lutN + ir) * 4;
		pixelValues[i + 0] = R;
		pixelValues[i + 1] = G;
		pixelValues[i + 2] = B;
	    }
	}
    }

    //
    // Initialize an array output pixel values to zero.
    //

    Array<half> lut (LUT_SIZE);

    for (size_t i = 0; i < LUT_SIZE; ++i)
	lut[i] = 0;

    //
    // Generate output pixel values by applying CTL transforms
    // to the pixel values.  (If the CTL transforms fail to
    // write to the output values, zero-initialization, above,
    // causes the displayed image to be black.)
    //

    ctlToLut (transformNames, header, LUT_SIZE, pixelValues, lut);

    //
    // Take the logarithm of the output values that were
    // produced by the CTL transforms.
    //

    for (size_t i = 0; i < LUT_SIZE; ++i)
    {
	if (lut[i] >= HALF_MIN && lut[i] <= HALF_MAX)
	{
	    //
	    // lut[i] is finite and positive.
	    //

	    lut[i] = log (lut[i]);
	}
	else
	{
	    //
	    // lut[i] is zero, negative or not finite;
	    // log (lut[i]) is undefined.
	    //

	    lut[i] = log (HALF_MIN);
	}
    }

    //
    // Convert the output values into a 3D texture.
    //

    GLuint lutId;

    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    glGenTextures (1, &lutId);

    glActiveTexture (GL_TEXTURE3);
    glBindTexture (GL_TEXTURE_3D, lutId);
    glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage3D (GL_TEXTURE_3D,
		  0,			// level
		  GL_RGBA16F_ARB,	// internalFormat
		  lutN, lutN, lutN,	// width, height, depth
		  0,			// border
		  GL_RGBA,		// format
		  GL_HALF_FLOAT_ARB,	// type
		  (char *) &lut[0]);

    checkGlErrors ("init3DLut");
}


void
drawString (GLfloat x, GLfloat y, const char str[])
{
    //
    // Draw a text string.
    //

    glPushMatrix();
    glTranslatef (x, y, 0);
    glScalef (0.10, 0.15, 0);

    while (*str)
	glutStrokeCharacter (GLUT_STROKE_MONO_ROMAN, *str++);

    glPopMatrix();
}


void
drawStringWithBorder (GLfloat x, GLfloat y, const char str[])
{
    //
    // Draw text string where each character is surrounded
    // by a one pixel wide black border.
    //

    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_BLEND);
    glEnable (GL_LINE_SMOOTH);
    glColor4f (0, 0, 0, 1);
    glLineWidth (2);
    drawString (x - 1, y - 1, str);
    drawString (x + 1, y - 1, str);
    drawString (x - 1, y + 1, str);
    drawString (x + 1, y + 1, str);
    glLineWidth (2);
    glColor4f (0.8, 0.8, 0.8, 1);
    drawString (x, y, str);
    glDisable (GL_LINE_SMOOTH);
    glDisable (GL_BLEND);
}


void
drawFrame ()
{
    //
    // Draw the current frame
    //

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho (0, glWindowWidth, 0, glWindowHeight, -1, 1);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();

    glDisable (GL_SCISSOR_TEST);
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glClear (GL_COLOR_BUFFER_BIT);

    glEnable (GL_SCISSOR_TEST);
    glClearColor (0.3, 0.3, 0.3, 1.0);
    glClear (GL_COLOR_BUFFER_BIT);

    //
    // Convert the pixels of the current frame into OpenGL textures.
    //

    const Box2i &dw = ib.dataWindow;
    int w = dw.max.x - dw.min.x + 1;
    int h = dw.max.y - dw.min.y + 1;

    if (ib.rgbMode)
    {
	glActiveTexture (GL_TEXTURE0);
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, texId[0]);

	glTexSubImage2D (GL_TEXTURE_2D,
			 0,				// level
			 0, 0,				// xoffset, yoffset
			 w, h,
			 GL_RGBA,			// format
			 GL_HALF_FLOAT_ARB,		// type
			 (GLvoid *) ib.pixels (i, 0));

    }
    else
    {
	glActiveTexture (GL_TEXTURE0);
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, texId[0]);

	glTexSubImage2D (GL_TEXTURE_2D,
			 0,				// level
			 0, 0,				// xoffset, yoffset
			 w, h,
			 GL_LUMINANCE,			// format
			 GL_HALF_FLOAT_ARB,		// type
			 (GLvoid *) ib.pixels (i, 0));

	glActiveTexture (GL_TEXTURE1);
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, texId[1]);

	glTexSubImage2D (GL_TEXTURE_2D,
			 0,				// level
			 0, 0,				// xoffset, yoffset
			 w / 2, h / 2,
			 GL_LUMINANCE,			// format
			 GL_HALF_FLOAT_ARB,		// type
			 (GLvoid *) ib.pixels (i, 1));

	glActiveTexture (GL_TEXTURE2);
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, texId[2]);

	glTexSubImage2D (GL_TEXTURE_2D,
			 0,				// level
			 0, 0,				// xoffset, yoffset
			 w / 2, h / 2,
			 GL_LUMINANCE,			// format
			 GL_HALF_FLOAT_ARB,		// type
			 (GLvoid *) ib.pixels (i, 2));
    }

    //
    // Enable Cg shading and draw a rectangle that fills
    // the entire data window.  The textures will be mapped
    // onto this rectangle.
    //

    glActiveTexture (GL_TEXTURE3);
    glEnable (GL_TEXTURE_3D);

    cgGLEnableProfile (cgProfile);
		     
    glBegin (GL_POLYGON);
    glTexCoord2f (0, 1);
    glVertex2i (drawRect.min.x, drawRect.min.y);
    glTexCoord2f (1, 1);
    glVertex2i (drawRect.max.x, drawRect.min.y);
    glTexCoord2f (1, 0);
    glVertex2i (drawRect.max.x, drawRect.max.y);
    glTexCoord2f (0, 0);
    glVertex2i (drawRect.min.x, drawRect.max.y);
    glEnd ();

    //
    // Disable texture mapping and Cg shading and draw the text overlay
    // that indicates the frame rate and exposure settings.
    //

    if (showTextOverlay)
    {
	glActiveTexture (GL_TEXTURE0);
	glDisable (GL_TEXTURE_2D);
	glActiveTexture (GL_TEXTURE1);
	glDisable (GL_TEXTURE_2D);
	glActiveTexture (GL_TEXTURE2);
	glDisable (GL_TEXTURE_2D);
	glActiveTexture (GL_TEXTURE3);
	glDisable (GL_TEXTURE_3D);
	cgGLDisableProfile (cgProfile);
	glShadeModel (GL_FLAT);

	stringstream ss;

	ss << setw (6) << frameNumber << " ";

	if (timer.playState == RUNNING)
	{
	    ss << setiosflags (ios_base::fixed) <<
		  setprecision (2) << setw (7) <<
		  timer.actualFrameRate() << " fps";
	}
	else
	{
	    ss << "      pause";
	}

	ss << "  " <<
	      setiosflags (ios_base::showpos | ios_base::fixed) <<
	      setprecision (1) << setw (5) <<
	      exposure << " stops";

	if (ib.rgbMode)
	    ss << "  RGB";
	else
	    ss << "  YC";

	if (!enableCtl)
	    ss << "  CTL off";

	drawStringWithBorder (20, 20, ss.str().c_str());
    }

    checkGlErrors ("drawFrame");
}


void
redrawWindow ()
{
    //
    // Display the next image on the screen
    //

    //
    // Exit if the file reading thread has terminated.
    //

    if (ib.exitSemaphore2.tryWait())
	exit (1);

    //
    // Wait until it is time to display the next image
    //

    timer.waitUntilNextFrameIsDue();

    //
    // Wait until the file reading thread has made the next frame available
    //

    if (timer.playState == RUNNING || timer.playState == PREPARE_TO_PAUSE)
	ib.fullBuffersSemaphore.wait();

    if (timer.playState == PREPARE_TO_PAUSE)
	timer.playState = PAUSE;

    //
    // Draw the frame
    //

    frameNumber = ib.frameNumber (i);
    drawFrame ();

    //
    // Return the image buffer to the file reading thread
    //

    if (timer.playState == RUNNING || timer.playState == PREPARE_TO_RUN)
    {
	i = (i + 1) % ib.numBuffers();
	ib.emptyBuffersSemaphore.post();
    }

    if (timer.playState == PREPARE_TO_RUN)
	timer.playState = RUNNING;

    //
    // Flush and ѕwap buffers to make the frame visible
    //

    glFlush();
    glutSwapBuffers();

    //
    // Make sure this function gets called again immediately
    //

    if (timer.playState == RUNNING || timer.playState == PREPARE_TO_RUN)
	glutPostRedisplay();
}


void
handleKeypress (unsigned char key, int, int)
{
    if (key == 'q' || key == 0x1b)
    {
	//
	// Quit: In order to make sure that the file reading thread
	// won't crash by trying to use shared resources while we
	// exit, we first tell the file reading thread to exit.
	// Then we wait until the file reading thread signals that
	// it has received the exit command.  At this point, it is
	// safe to exit.
	//

	ib.exitSemaphore1.post();
	ib.emptyBuffersSemaphore.post();
	ib.exitSemaphore2.wait();

	exit (0);
    }

    if (key == '>' || key == '.' || key == '<' || key == ',')
    {
	//
	// Change exposure: 1 f-stop brighter or darker
	//

	if ((key == '>' || key == '.') && exposure < 10)
	    exposure += 1;

	if ((key == '<' || key == ',') && exposure > -10)
	    exposure -= 1;

	CGparameter emParam = cgGetNamedParameter (cgProgram, "expMult");
	cgSetParameter1f (emParam, pow (2.0f, exposure));
	glutPostRedisplay();
    }

#if HAVE_CTL_INTERPRETER

    if (key == 'c' || key == 'C')
    {
	//
	// Toggle CTL transforms on/off
	//

	CGparameter enableLutParam =
		cgGetNamedParameter (cgProgram, "enableLut");

	enableCtl = !enableCtl;
	cgSetParameter1f (enableLutParam, enableCtl? 1.0: 0.0);
	glutPostRedisplay();
    }

#endif

    if (key == 'o' || key == 'O')
    {
	//
	// Toggle text overlay on/off
	// 

	showTextOverlay = !showTextOverlay;
	glutPostRedisplay();
    }

    if (key == 'p' || key == 'P' || key == 'l' || key == 'L')
    {
	//
	// Toggle between playing forward and pause
	//

	if (timer.playState == RUNNING && ib.forward)
	    timer.playState = PREPARE_TO_PAUSE;

	if (timer.playState == PAUSE)
	    timer.playState = PREPARE_TO_RUN;

	ib.forward = true;
	glutPostRedisplay();
    }

    if (key == 'h' || key == 'H')
    {
	//
	// Toggle between playing backward and pause
	//

	if (timer.playState == RUNNING && !ib.forward)
	    timer.playState = PREPARE_TO_PAUSE;

	if (timer.playState == PAUSE)
	    timer.playState = PREPARE_TO_RUN;

	ib.forward = false;
	glutPostRedisplay();
    }

    if (key == 'j' || key == 'J' || key == 'k' || key == 'K')
    {
	//
	// Step one frame forward or backward
	//

	if (timer.playState == RUNNING || timer.playState == PREPARE_TO_PAUSE)

	    ib.fullBuffersSemaphore.wait();

	if (key == 'k' || key == 'K')
	    ib.forward = true;
	else
	    ib.forward = false;
	
	timer.playState = PAUSE;

	int newFrameNumber;

	if (ib.forward)
	{
	    if (frameNumber >= lastFrameNumber)
		newFrameNumber = firstFrameNumber;
	    else
		newFrameNumber = frameNumber + 1;
	}
	else
	{
	    if (ib.frameNumber(i) <= firstFrameNumber)
		newFrameNumber = lastFrameNumber;
	    else
		newFrameNumber = frameNumber - 1;
	}

	while (ib.frameNumber(i) != newFrameNumber)
	{
	    i = (i + 1) % ib.numBuffers();
	    ib.emptyBuffersSemaphore.post();
	    ib.fullBuffersSemaphore.wait();
	}

	glutPostRedisplay();
    }

    if (key == 'f' || key == 'F')
    {
	//
	// Toggle full-screen mode on/off
	//

	fullScreenMode = !fullScreenMode;

	if (fullScreenMode)
	    glutFullScreen();
	else
	    glutReshapeWindow (glWindowWidth, glWindowHeight);

	glutPostRedisplay();
    }
}


} // namespace

void
playExr (const char fileNameTemplate[],
         int firstFrame,
	 int lastFrame,
	 int numThreads,
	 float fps,
	 float xyScale,
	 const vector<string> &transformNames,
	 bool useHwTexInterpolation)
{
    //
    // Set the number of threads the IlmImf library
    // will use internally for OpenEXR file reading.
    //

    OPENEXR_IMF_NAMESPACE::setGlobalThreadCount (numThreads);

    //
    // Allocate buffers for the images, initialize semaphores
    // for synchronization between the file reading thread
    // and the display loop in the main thread
    //

    firstFrameNumber = firstFrame;
    lastFrameNumber = lastFrame;

    Header header;
    initializeImageBuffers (ib, header, fileNameTemplate);

    //
    // Determine the playback frame rate.
    //

    if (fps < 0)
    {
	if (hasFramesPerSecond (header) && framesPerSecond (header) >= 1)
	    fps = framesPerSecond (header);
	else
	    fps = 24;
    }

    //
    // Compute on-screen window sizes
    //

    computeWindowSizes (header.dataWindow(),
			header.displayWindow(),
			header.pixelAspectRatio(),
			xyScale);

    //
    // Create an OpenGL window
    //

    glutInitDisplayMode (GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize (glWindowWidth, glWindowHeight);
    glutCreateWindow (fileNameTemplate);
    glutKeyboardFunc (handleKeypress);
    glutReshapeFunc (handleReshape);
    glutDisplayFunc (redrawWindow);

    //
    // Verify that OpenGL supports the extensions we need
    //

    initAndCheckGlExtensions();

    //
    // Initialize textures and Cg shaders
    //

    float xMin;
    float xMax;
    float m;
    float t;

    init3DLut (transformNames, header, xMin, xMax, m, t);

    hwTexInterpolation = useHwTexInterpolation;

    if (ib.rgbMode)
    {
	initShaderRgb (xMin, xMax, m, t);
	initTexturesRgb();
    }
    else
    {
	initShaderLuminanceChroma (xMin, xMax, m, t);
	initTexturesLuminanceChroma();
    }

    //
    // Start the file reading thread and the display loop
    //

    FileReadingThread frt (fileNameTemplate, firstFrame, lastFrame, ib);
    timer.playState = (firstFrame != lastFrame)? RUNNING: PREPARE_TO_PAUSE;
    timer.setDesiredFrameRate (fps);
    glutMainLoop();
}
