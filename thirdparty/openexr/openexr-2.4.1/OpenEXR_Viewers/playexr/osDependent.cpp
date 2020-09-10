///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2007, Industrial Light & Magic, a division of Lucas
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
//	OpenGL related code and definitions
//	that depend on the operating system.
//
//----------------------------------------------------------------------------

#include "osDependent.h"
#include <stdlib.h>
#include <iostream>

using namespace std;


void
initAndCheckGlExtensions ()
{
    #ifdef PLAYEXR_USE_APPLE_FLOAT_PIXELS

	if (!glutExtensionSupported ("GL_APPLE_float_pixels"))
	{
	    cerr << "This program requires OpenGL support for "
		    "16-bit floating-point textures." << endl;
	    exit (1);
	}

    #else

	if (!glutExtensionSupported ("GL_ARB_texture_float") ||
	    !glutExtensionSupported ("GL_ARB_half_float_pixel"))
	{
	    cerr << "This program requires OpenGL support for "
		    "16-bit floating-point textures." << endl;
	    exit (1);
	}

    #endif

    if (!glutExtensionSupported ("GL_ARB_fragment_shader"))
    {
	cerr << "This program requires OpenGL support for "
		"fragment shaders and the Cg shading language." << endl;
	exit (1);
    }

    #ifdef WIN32

	GLenum err = glewInit();

	if (GLEW_OK != err)
	{
	    cerr << "Cannot initialize "
		    "glew: " << glewGetErrorString (err) << endl;
	    exit (1);
	}

    #endif
}
