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

#ifndef INCLUDED_OS_DEPENDENT_H
#define INCLUDED_OS_DEPENDENT_H

//----------------------------------------------------------------------------
//
//	OpenGL related code and definitions
//	that depend on the operating system.
//
//----------------------------------------------------------------------------


#ifdef WIN32
    #include <GL/glew.h>
#else
    #define GL_GLEXT_PROTOTYPES
#endif

#if defined __APPLE__

    #include <GLUT/glut.h>
    #include <OpenGL/gl.h>
    #include <Cg/cgGL.h>

    #ifndef GL_HALF_FLOAT_ARB
	#define PLAYEXR_USE_APPLE_FLOAT_PIXELS
    #endif

    #ifndef GL_LUMINANCE16F_ARB
	#define GL_LUMINANCE16F_ARB GL_LUMINANCE_FLOAT16_APPLE
    #endif

    #ifndef GL_RGBA16F_ARB
	#define GL_RGBA16F_ARB GL_RGBA_FLOAT16_APPLE
    #endif
    
    #ifndef GL_HALF_FLOAT_ARB
	#define GL_HALF_FLOAT_ARB GL_HALF_APPLE
    #endif

#else

    #include <GL/glut.h>
    #include <GL/gl.h>
    #include <Cg/cgGL.h>

#endif


void initAndCheckGlExtensions ();


#endif
