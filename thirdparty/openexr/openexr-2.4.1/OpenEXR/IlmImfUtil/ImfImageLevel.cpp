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
//      class ImageLevel
//
//----------------------------------------------------------------------------

#include "ImfImageLevel.h"
#include <Iex.h>
#include <cassert>

using namespace IMATH_NAMESPACE;
using namespace IEX_NAMESPACE;
using namespace std;

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_ENTER


ImageLevel::ImageLevel
    (Image& image,
     int xLevelNumber,
     int yLevelNumber)
:
    _image (image),
    _xLevelNumber (xLevelNumber),
    _yLevelNumber (yLevelNumber),
    _dataWindow (Box2i (V2i (0, 0), V2i (-1, -1)))
{
    // empty
}


ImageLevel::~ImageLevel ()
{
    // empty
}


void			
ImageLevel::resize (const Box2i& dataWindow)
{
    if (dataWindow.max.x < dataWindow.min.x - 1||
        dataWindow.max.y < dataWindow.min.y - 1)
    {
        THROW (ArgExc,
               "Cannot reset data window for image level to "
               "(" << dataWindow.min.x << ", " << dataWindow.min.y << ") - "
               "(" << dataWindow.max.x << ", " << dataWindow.max.y << "). "
               "The new data window is invalid.");
    }

    _dataWindow = dataWindow;
}


void
ImageLevel::shiftPixels (int dx, int dy)
{
    _dataWindow.min.x += dx;
    _dataWindow.min.y += dy;
    _dataWindow.max.x += dx;
    _dataWindow.max.y += dy;
}


void
ImageLevel::throwChannelExists (const string& name) const
{
    THROW (ArgExc, "Cannot insert a new image channel with "
                   "name \"" << name << "\" into an image level. "
                   "A channel with the same name exists already.");
}


void
ImageLevel::throwBadChannelName (const string& name) const
{
    THROW (ArgExc, "Attempt to access non-existent "
                   "image channel \"" << name << "\".");
}


void
ImageLevel::throwBadChannelNameOrType (const string& name) const
{
    THROW (ArgExc, "Image channel \"" << name << "\" does not exist "
                   "or is not of the expected type.");
}


OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_EXIT
