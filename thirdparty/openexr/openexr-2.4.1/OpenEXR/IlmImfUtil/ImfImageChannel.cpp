///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014, Industrial Light & Magic, a division of Lucas
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
//      class ImageChannel
//
//----------------------------------------------------------------------------

#include "ImfImageChannel.h"
#include "ImfImageLevel.h"
#include <Iex.h>

using namespace IMATH_NAMESPACE;
using namespace IEX_NAMESPACE;
using namespace std;

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_ENTER


ImageChannel::ImageChannel
    (ImageLevel &level,
     int xSampling,
     int ySampling,
     bool pLinear)
:
    _level (level),
    _xSampling (xSampling),
    _ySampling (ySampling),
    _pLinear (pLinear),
    _pixelsPerRow (0),
    _pixelsPerColumn (0),
    _numPixels (0)
{
    // empty
}


ImageChannel::~ImageChannel ()
{
    // empty
}


Channel
ImageChannel::channel () const
{
    return Channel (pixelType(), xSampling(), ySampling(), pLinear());
}


void
ImageChannel::resize ()
{
    const Box2i& dataWindow = level().dataWindow();

    if (dataWindow.min.x % _xSampling || dataWindow.min.y % _ySampling)
    {
        throw ArgExc ("The minimum x and y coordinates of the data window "
                      "of an image level must be multiples of the x and y "
                      "subsampling factors of all channels in the image.");
    }

    int width =  dataWindow.max.x - dataWindow.min.x + 1;
    int height = dataWindow.max.y - dataWindow.min.y + 1;

    if (width % _xSampling || height % _ySampling)
    {
        throw ArgExc ("The width and height of the data window of an image "
                      "level must be multiples of the x and y subsampling "
                      "factors of all channels in the image.");
    }

    _pixelsPerRow =    width  / _xSampling;
    _pixelsPerColumn = height / _ySampling;
    _numPixels = _pixelsPerRow * _pixelsPerColumn;
}


void
ImageChannel::boundsCheck (int x, int y) const
{
    const Box2i &dataWindow = level().dataWindow();

    if (x < dataWindow.min.x || x > dataWindow.max.x ||
        y < dataWindow.min.y || y > dataWindow.max.y)
    {
        THROW (ArgExc,
               "Attempt to access a pixel at location "
               "(" << x << ", " << y << ") in an image whose data window is "
               "(" << dataWindow.min.x << ", " << dataWindow.min.y << ") - "
               "(" << dataWindow.max.x << ", " << dataWindow.max.y << ").");
    }

    if (x % _xSampling || y % _ySampling)
    {
        THROW (ArgExc,
               "Attempt to access a pixel at location "
               "(" << x << ", " << y << ") in a channel whose x and y sampling "
               "rates are " << _xSampling << " and " << _ySampling << ".  The "
               "pixel coordinates are not divisible by the sampling rates.");
    }
}


OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_EXIT
