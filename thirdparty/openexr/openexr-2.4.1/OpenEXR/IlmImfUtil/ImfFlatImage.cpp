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
//      class FlatImage
//
//----------------------------------------------------------------------------

#include "ImfFlatImage.h"
#include <Iex.h>
#include <cassert>

using namespace IMATH_NAMESPACE;
using namespace IEX_NAMESPACE;
using namespace std;

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_ENTER


FlatImage::FlatImage ():
    Image ()
{
    resize (Box2i (V2i (0, 0), V2i (-1, -1)), ONE_LEVEL, ROUND_DOWN);
}


FlatImage::FlatImage
    (const Box2i &dataWindow,
     LevelMode levelMode,
     LevelRoundingMode levelRoundingMode)
:
    Image ()
{
    resize (dataWindow, levelMode, levelRoundingMode);
}


FlatImage::~FlatImage ()
{
    // empty
}


FlatImageLevel &
FlatImage::level (int l)
{
    return static_cast <FlatImageLevel &> (Image::level (l));
}


const FlatImageLevel &
FlatImage::level (int l) const
{
    return static_cast <const FlatImageLevel &> (Image::level (l));
}


FlatImageLevel &
FlatImage::level (int lx, int ly)
{
    return static_cast <FlatImageLevel &> (Image::level (lx, ly));
}


const FlatImageLevel &
FlatImage::level (int lx, int ly) const
{
    return static_cast <const FlatImageLevel &> (Image::level (lx, ly));
}


FlatImageLevel *
FlatImage::newLevel (int lx, int ly, const Box2i &dataWindow)
{
    return new FlatImageLevel (*this, lx, ly, dataWindow);
}

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_EXIT
