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



//----------------------------------------------------------------------------
//
//	class ImageView
//
//----------------------------------------------------------------------------

#include "ImageView.h"

#include <ImathMath.h>
#include <ImathFun.h>
#include <ImathLimits.h>
#include <halfFunction.h>


#include <algorithm>
#include <stdio.h>

#if defined _WIN32
    #include <windows.h>
    #include <FL/Fl.H>
    #include <GL/gl.h>
#elif defined __APPLE__
    #include <FL/Fl.H>
    #include <OpenGL/gl.h>
#else
    #include <FL/Fl.H>
    #include <GL/gl.h>
#endif

using std::min;
using std::max;
using std::cout;
using std::endl;
using std::cerr;
using namespace IMATH;

ImageView::ImageView (int x, int y,
                      int w, int h,
                      const char label[],
                      const IMF::Rgba pixels[],
                      float* dataZ[],
                      unsigned int sampleCount[],
                      int zsize,
                      int dw, int dh,
                      int dx, int dy,
                      Fl_Box *rgbaBox,
                      float farPlane,
                      float gamma,
                      float exposure,
                      float defog,
                      float kneeLow,
                      float kneeHigh)
:
    Fl_Gl_Window (x, y, w, h, label),
    _gamma (gamma),
    _exposure (exposure),
    _defog (defog),
    _kneeLow (kneeLow),
    _kneeHigh (kneeHigh),
    _rawPixels (pixels),
    _dataZ (dataZ),
    _sampleCount (sampleCount),
    _fogR (0),
    _fogG (0),
    _fogB (0),
    _farPlane (farPlane),
    _dw (dw),
    _dh (dh),
    _dx (dx),
    _dy (dy),
    _zsize (zsize),
    _rgbaBox (rgbaBox),
    _screenPixels (dw * dh * 3)
{
    computeFogColor();
    updateScreenPixels();

    //
    // initialize z value chart
    //
    _chartwin = new Fl_Window (600, 300);
    _chartwin->label("Deep Pixel Display");

    _chart = new Fl_Chart (20, 20,
                           _chartwin->w()-40,
                           _chartwin->h()-40,
                           "Sample #");
    _chartMax = new Fl_Chart (20, 20,
                           _chartwin->w()-40,
                           _chartwin->h()-40,
                           "");
    _chartMin = new Fl_Chart (20, 20,
                           _chartwin->w()-40,
                           _chartwin->h()-40,
                           "");

    findZbound();

    //
    // initialize Deep 3d window
    //
    _gl3d = NULL;
}

void
ImageView::setExposure (float exposure)
{
    _exposure = exposure;
    updateScreenPixels();
    redraw();
}


void
ImageView::setDefog (float defog)
{
    _defog = defog;
    updateScreenPixels();
    redraw();
}


void
ImageView::setKneeLow (float kneeLow)
{
    _kneeLow = kneeLow;
    updateScreenPixels();
    redraw();
}

void
ImageView::findZbound()
{
    //
    // find zmax and zmin values of deep data to set bound
    //
    float zmax  = limits<float>::min();
    float zmin = limits<float>::max();
    _maxCount = 0;

    for (int k = 0; k < _zsize; k++)
    {
        float* z = _dataZ[k];
        unsigned int count = _sampleCount[k];

        if (_maxCount < count)
            _maxCount = count;

        for (unsigned int i = 0; i < count; i++)
        {
            double val = double(z[i]);
            if (val > zmax && val < _farPlane)
                zmax = val;
            if (val < zmin)
                zmin = val;
        }
    }

    if ( zmax > zmin)
    {
        cout << "z max: "<< zmax << ", z min: " << zmin << endl;
        _chart->bounds (zmin, zmax);
    }

    _zmax = zmax;
    _zmin = zmin;
}

void
ImageView::setPixels(const IMF::Rgba pixels[/* w*h */],
                     float* dataZ[/* w*h */],
                     unsigned int sampleCount[/* w*h */],
                     int zsize,
                     int dw, int dh, int dx, int dy)
{
    //
    // update data of imageview
    //
    _rawPixels = pixels;
    _dw = dw;
    _dh = dh;
    _dx = dx;
    _dy = dy;
    _dataZ = dataZ;
    _sampleCount = sampleCount;
    _zsize = zsize;

    _screenPixels.resizeErase(dw*dh*3);

    findZbound();

    //
    // update Deep 3d window
    //
    GlWindow* temp;
    temp = _gl3d;
    _gl3d = NULL;

    if (_gl3d != NULL){
        delete temp;
    }

    updateScreenPixels();
    redraw();
}

void
ImageView::clearDataDisplay()
{
    _chart->clear();

    if (_gl3d != NULL)
        _gl3d->hide();
}

void
ImageView::setKneeHigh (float kneeHigh)
{
    _kneeHigh = kneeHigh;
    updateScreenPixels();
    redraw();
}


void
ImageView::draw()
{
    if (!valid())
    {
        glLoadIdentity();
        glViewport (0, 0, w(), h());
        glOrtho(0, w(), h(), 0, -1, 1);
    }

    glClearColor (0.3, 0.3, 0.3, 1.0);
    glClear (GL_COLOR_BUFFER_BIT);

    if (_dx + _dw <= 0 || _dx >= w())
        return;

    for (int y = 0; y < _dh; ++y)
    {
        if (y + _dy < 0 || y + _dy >= h())
            continue;

        glRasterPos2i (max (0, _dx), y + _dy + 1);

        glDrawPixels (_dw + min (0, _dx),			     // width
                      1,					     // height
                      GL_RGB,					     // format
                      GL_UNSIGNED_BYTE,				     // type
                      _screenPixels +				     // pixels
                      static_cast <ptrdiff_t> ((y * _dw - min (0, _dx)) * 3));
    }
}


void
ImageView::computeFogColor ()
{
    _fogR = 0;
    _fogG = 0;
    _fogB = 0;

    for (int j = 0; j < _dw * _dh; ++j)
    {
        const IMF::Rgba &rp = _rawPixels[j];

        if (rp.r.isFinite())
            _fogR += rp.r;

        if (rp.g.isFinite())
            _fogG += rp.g;

        if (rp.b.isFinite())
            _fogB += rp.b;
    }

    _fogR /= _dw * _dh;
    _fogG /= _dw * _dh;
    _fogB /= _dw * _dh;
}

void
ImageView::drawChart (int x, int y, bool initChart)
{
    if (x >= 0 && x < w() && y >= 0 && y < h())
    {
        int px = x - _dx;
        int py = y - _dy;

        if (px >= 0 && px < _dw && py >= 0 && py < _dh)
        {
            float* z = _dataZ[py * _dw + px];
            unsigned int count = _sampleCount[py * _dw + px];

            cout << "\nsample Count: " << count << endl;
            cout << "x: " << px << ", y: " << py << endl;

            for (unsigned int i = 0; i < count; i++)
            {
                printf ("pixel Z value  %d: %.3f\n", i, float(z[i]));
            }

            const IMF::Rgba &p = _rawPixels[py * _dw + px];

            cout << "R = " << p.r << ", G = " << p.g << ","
            " B = " << p.b <<endl;

            //
            // draw the chart
            //
            drawChartRef();

            for (unsigned int i = 0; i < count; i++)
            {
                double val = double(z[i]);
                if (val < _farPlane)
                {
                    static char val_str[20];
                    sprintf (val_str, "%.3lf", val);
                    _chart->add (val, val_str, FL_BLUE);
                }
            }

            redraw();

            if (initChart)
            {
                _chartwin->resizable (_chartwin);
                _chartwin->set_non_modal(); // make chart on top

                if (!_chartwin->shown())
                    _chartwin->show();
            }
        }
    }
}

void
ImageView::drawChartRef ()
{
    _chart->clear();
    _chart->bounds(_zmin, _zmax);

    _chart->type (FL_LINE_CHART);
    _chart->label("Sample #");

    _chartMax->clear();
    _chartMax->type (FL_SPIKE_CHART);
    static char val_str[20];
    sprintf (val_str, "Zmax : %.3lf", _zmax);
    _chartMax->label(val_str);
    _chartMax->align(FL_ALIGN_TOP_LEFT);
    _chartMax->box(FL_NO_BOX);

    _chartMin->clear();
    _chartMin->type (FL_SPIKE_CHART);
    static char val_str1[20];
    sprintf (val_str1, "Zmin : %.3lf", _zmin);
    _chartMin->label(val_str1);
    _chartMin->align(FL_ALIGN_BOTTOM_LEFT);
    _chartMin->box(FL_NO_BOX);
    
}


int
ImageView::handle (int event)
{

    if (event == FL_PUSH)
    {
        // We want to get the other associated events for this widget so
        // return a non-zero value here.
        return 1;
    }

    if (event == FL_MOVE)
    {
        //
        // Print the red, green and blue values of
        // the pixel at the current cursor location.
        //

        int x = Fl::event_x();
        int y = Fl::event_y();

        if (x >= 0 && x < w() && y >= 0 && y < h())
        {
            int px = x - _dx;
            int py = y - _dy;

            if (px >= 0 && px < _dw && py >= 0 && py < _dh)
            {
                const IMF::Rgba &p = _rawPixels[py * _dw + px];

                sprintf (_rgbaBoxLabel,
                         "r = %.3g   g = %.3g   b = %.3g",
                         float (p.r), float (p.g), float (p.b));
            }
            else
            {
                sprintf (_rgbaBoxLabel, " ");
            }

            _rgbaBox->label (_rgbaBoxLabel);


            if (_chartwin->shown() && _zsize > 0)
            {
                int x = Fl::event_x();
                int y = Fl::event_y();
                drawChart (x, y, false);
            }
        }
    }

    if (event == FL_RELEASE && Fl::event_button() == FL_LEFT_MOUSE)
    {
        //
        // Open a sample chart and print the z values of
        // the pixel at the current cursor location
        //

        if(_zsize > 0)
        {
            int x = Fl::event_x();
            int y = Fl::event_y();

            drawChart (x, y, true);
        }

    }

    if (event == FL_RELEASE && Fl::event_button() == FL_RIGHT_MOUSE)
    {
        if(_zsize > 0)
        {
            if (_gl3d == NULL)
            {
                //
                // initialize Deep 3d display
                //

                _gl3d = new GlWindow(10, 10, 500, 500,
                                     "3D View", _rawPixels,
                                     _dataZ, _sampleCount,
                                     _dw, _dh, _zmax, _zmin,
                                     _farPlane);

                _gl3d->show();
            }
            else
            {
                _gl3d->show();
            }
        }
    }

    return Fl_Gl_Window::handle (event);
}


namespace {

//
// Conversion from raw pixel data to data for the OpenGL frame buffer:
//
//  1) Compensate for fogging by subtracting defog
//     from the raw pixel values.
//
//  2) Multiply the defogged pixel values by
//     2^(exposure + 2.47393).
//
//  3) Values that are now 1.0 are called "middle gray".
//     If defog and exposure are both set to 0.0, then
//     middle gray corresponds to a raw pixel value of 0.18.
//     In step 6, middle gray values will be mapped to an
//     intensity 3.5 f-stops below the display's maximum
//     intensity.
//
//  4) Apply a knee function.  The knee function has two
//     parameters, kneeLow and kneeHigh.  Pixel values
//     below 2^kneeLow are not changed by the knee
//     function.  Pixel values above kneeLow are lowered
//     according to a logarithmic curve, such that the
//     value 2^kneeHigh is mapped to 2^3.5.  (In step 6,
//     this value will be mapped to the the display's
//     maximum intensity.)
//
//  5) Gamma-correct the pixel values, according to the
//     screen's gamma.  (We assume that the gamma curve
//     is a simple power function.)
//
//  6) Scale the values such that middle gray pixels are
//     mapped to a frame buffer value that is 3.5 f-stops
//     below the display's maximum intensity. (84.65 if
//     the screen's gamma is 2.2)
//
//  7) Clamp the values to [0, 255].
//


float
knee (double x, double f)
{
    return float (IMATH::Math<double>::log (x * f + 1) / f);
}


float
findKneeF (float x, float y)
{
    float f0 = 0;
    float f1 = 1;

    while (knee (x, f1) > y)
    {
        f0 = f1;
        f1 = f1 * 2;
    }

    for (int i = 0; i < 30; ++i)
    {
        float f2 = (f0 + f1) / 2;
        float y2 = knee (x, f2);

        if (y2 < y)
            f1 = f2;
        else
            f0 = f2;
    }

    return (f0 + f1) / 2;
}


struct Gamma
{
    float g, m, d, kl, f, s;

    Gamma (float gamma,
           float exposure,
           float defog,
           float kneeLow,
           float kneeHigh);

    float operator () (half h);
};


Gamma::Gamma
(float gamma,
 float exposure,
 float defog,
 float kneeLow,
 float kneeHigh)
:
    g (gamma),
    m (IMATH::Math<float>::pow (2, exposure + 2.47393)),
    d (defog),
    kl (IMATH::Math<float>::pow (2, kneeLow)),
    f (findKneeF (IMATH::Math<float>::pow (2, kneeHigh) - kl,
                  IMATH::Math<float>::pow (2, 3.5) - kl)),
                  s (255.0 * IMATH::Math<float>::pow (2, -3.5 * g))
                  {}


float
Gamma::operator () (half h)
{
    //
    // Defog
    //

    float x = max (0.f, (h - d));

    //
    // Exposure
    //

    x *= m;

    //
    // Knee
    //

    if (x > kl)
        x = kl + knee (x - kl, f);

    //
    // Gamma
    //

    x = IMATH::Math<float>::pow (x, g);

    //
    // Scale and clamp
    //

    return clamp (x * s, 0.f, 255.f);
}


//
//  Dithering: Reducing the raw 16-bit pixel data to 8 bits for the
//  OpenGL frame buffer can sometimes lead to contouring in smooth
//  color ramps.  Dithering with a simple Bayer pattern eliminates
//  visible contouring.
//

unsigned char
dither (float v, int x, int y)
{
    static const float d[4][4] =
    {
     {0.f / 16,  8.f / 16,  2.f / 16, 10.f / 16},
     {12.f / 16,  4.f / 16, 14.f / 16,  6.f / 16},
     {3.f / 16, 11.f / 16,  1.f / 16,  9.f / 16},
     {15.f / 16,  7.f / 16, 13.f / 16,  5.f / 16}
    };

    return (unsigned char) (v + d[y & 3][x & 3]);
}

} // namespace


float
ImageView::findKnee (float x, float y)
{
    return findKneeF (x, y);
}


void
ImageView::updateScreenPixels ()
{
    halfFunction<float>
    rGamma (Gamma (_gamma,
                   _exposure,
                   _defog * _fogR,
                   _kneeLow,
                   _kneeHigh),
                   -HALF_MAX, HALF_MAX,
                   0.f, 255.f, 0.f, 0.f);

    halfFunction<float>
    gGamma (Gamma (_gamma,
                   _exposure,
                   _defog * _fogG,
                   _kneeLow,
                   _kneeHigh),
                   -HALF_MAX, HALF_MAX,
                   0.f, 255.f, 0.f, 0.f);

    halfFunction<float>
    bGamma (Gamma (_gamma,
                   _exposure,
                   _defog * _fogB,
                   _kneeLow,
                   _kneeHigh),
                   -HALF_MAX, HALF_MAX,
                   0.f, 255.f, 0.f, 0.f);


    for (int y = 0; y < _dh; ++y)
    {
        int i = y * _dw;

        for (int x = 0; x < _dw; ++x)
        {
            int j = i + x;
            const IMF::Rgba &rp = _rawPixels[j];
            unsigned char *sp = _screenPixels + j * 3;
            sp[0] = dither (rGamma (rp.r), x, y);
            sp[1] = dither (gGamma (rp.g), x, y);
            sp[2] = dither (bGamma (rp.b), x, y);
        }
    }
}
