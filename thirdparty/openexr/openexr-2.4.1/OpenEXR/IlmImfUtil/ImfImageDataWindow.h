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

#ifndef INCLUDED_IMF_IMAGE_DATA_WINDOW_H
#define INCLUDED_IMF_IMAGE_DATA_WINDOW_H

//----------------------------------------------------------------------------
//
//      enum DataWindowSource,
//      function dataWindowForFile()
//
//----------------------------------------------------------------------------

#include "ImfUtilExport.h"
#include <ImfNamespace.h>
#include <ImathBox.h>

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_ENTER


enum DataWindowSource
{
    USE_IMAGE_DATA_WINDOW,
    USE_HEADER_DATA_WINDOW
};


//
// Given the an image, i, an OpenEXR file header, h, and a data window
// source flag, d, dataWindowForFile(i,h,d) returns i.dataWindow() if d
// is USE_IMAGE_DATA_WINDOW, or the intersection of i.dataWindow() and
// h.dataWindow() if d is USE_HEADER_DATA_WINDOW.
//

class Image;
class Header;

IMFUTIL_EXPORT
IMATH_NAMESPACE::Box2i
dataWindowForFile (const Header &hdr, const Image &img, DataWindowSource dws);


OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_EXIT

#endif
