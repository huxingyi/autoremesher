///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2012, Industrial Light & Magic, a division of Lucas
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

#ifndef INCLUDED_GLWINDOW3D_H
#define INCLUDED_GLWINDOW3D_H

//----------------------------------------------------------------------------
//
//        class GlWindow3d -- reconstructs deep image in a 3D OpenGl window
//
//----------------------------------------------------------------------------
#include "namespaceAlias.h"


#include <stdio.h>

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif

#include <math.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>

#include <ImfRgba.h>
#include <ImfArray.h>
#include <ImfNamespace.h>

#define FPS (1.0/24.0)  // frames per second

class GlWindow : public Fl_Gl_Window
{
public:

    GlWindow (int x,int y,
              int w,int h,
              const char *l,
              const IMF::Rgba pixels[],
              float* dataZ[],
              unsigned int sampleCount[],
              int dx, int dy, // data window
              float zmax, float zmin,
              float farPlane  // zfar plane in Deep 3D window
              );
    ~GlWindow ();

    void Perspective (GLdouble focal, GLdouble aspect,
                      GLdouble zNear, GLdouble zFar);
    void ReshapeViewport();
    void GlInit();
    void draw();
    int handle (int event);

protected:
    const IMF::Rgba *                    _rawPixels;
    float**                              _dataZ;
    unsigned int *                       _sampleCount;
    int                                  _dx;
    int                                  _dy;
    float                                _zmax;
    float                                _zmin;
    float                                _farPlane;

private:
    double      _zoom;
    double      _translateX;
    double      _translateY;
    double      _scaleZ;
    double      _fitTran;
    double      _fitScale;
    double      _elevation;     // for rotation
    double      _azimuth;       // for rotation
    int         _mouseX;
    int         _mouseY;
    int         _mouseStartX;
    int         _mouseStartY;
    int         _inverted;      // for rotation
    int         _displayFactor; // for display pixel samples

    // TIMER CALLBACK
    // Handles rotation of the object
    //
    static void Timer_CallBack (void *data)
    {
        GlWindow *glwin = (GlWindow*)data;
        glwin->redraw();
        Fl::repeat_timeout(FPS, Timer_CallBack, data);
    }
};

#endif
