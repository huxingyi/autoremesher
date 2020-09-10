///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002, Industrial Light & Magic, a division of Lucas
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
//      class Slice
//      class FrameBuffer
//
//-----------------------------------------------------------------------------

#include <ImfFrameBuffer.h>
#include "Iex.h"


using namespace std;

#include "ImfNamespace.h"

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_ENTER

Slice::Slice (PixelType t,
              char *b,
              size_t xst,
              size_t yst,
              int xsm,
              int ysm,
              double fv,
              bool xtc,
              bool ytc)
:
    type (t),
    base (b),
    xStride (xst),
    yStride (yst),
    xSampling (xsm),
    ySampling (ysm),
    fillValue (fv),
    xTileCoords (xtc),
    yTileCoords (ytc)
{
    // empty
}

Slice
Slice::Make (
    PixelType                   type,
    const void*                 ptr,
    const IMATH_NAMESPACE::V2i& origin,
    int64_t                     w,
    int64_t                     h,
    size_t                      xStride,
    size_t                      yStride,
    int                         xSampling,
    int                         ySampling,
    double                      fillValue,
    bool                        xTileCoords,
    bool                        yTileCoords)
{
    char* base = reinterpret_cast<char*> (const_cast<void *> (ptr));
    if (xStride == 0)
    {
        switch (type)
        {
            case UINT: xStride = sizeof (uint32_t); break;
            case HALF: xStride = sizeof (uint16_t); break;
            case FLOAT: xStride = sizeof (float); break;
            case NUM_PIXELTYPES:
                THROW (IEX_NAMESPACE::ArgExc, "Invalid pixel type.");
        }
    }
    if (yStride == 0)
        yStride = static_cast<size_t> (w / xSampling) * xStride;

    // data window is an int, so force promote to higher type to avoid
    // overflow for off y (degenerate size checks should be in
    // ImfHeader::sanityCheck, but offset can be large-ish)
    int64_t offx = (static_cast<int64_t> (origin.x) /
                    static_cast<int64_t> (xSampling));
    offx *= static_cast<int64_t> (xStride);

    int64_t offy = (static_cast<int64_t> (origin.y) /
                    static_cast<int64_t> (ySampling));
    offy *= static_cast<int64_t> (yStride);

    return Slice (
        type,
        base - offx - offy,
        xStride,
        yStride,
        xSampling,
        ySampling,
        fillValue,
        xTileCoords,
        yTileCoords);
}

Slice
Slice::Make (
    PixelType                     type,
    const void*                   ptr,
    const IMATH_NAMESPACE::Box2i& dataWindow,
    size_t                        xStride,
    size_t                        yStride,
    int                           xSampling,
    int                           ySampling,
    double                        fillValue,
    bool                          xTileCoords,
    bool                          yTileCoords)
{
    return Make (
        type,
        ptr,
        dataWindow.min,
        static_cast<int64_t> (dataWindow.max.x) -
            static_cast<int64_t> (dataWindow.min.x) + 1,
        static_cast<int64_t> (dataWindow.max.y) -
            static_cast<int64_t> (dataWindow.min.y) + 1,
        xStride,
        yStride,
        xSampling,
        ySampling,
        fillValue,
        xTileCoords,
        yTileCoords);
}

void
FrameBuffer::insert (const char name[], const Slice &slice)
{
    if (name[0] == 0)
    {
        THROW (IEX_NAMESPACE::ArgExc,
               "Frame buffer slice name cannot be an empty string.");
    }

    _map[name] = slice;
}


void
FrameBuffer::insert (const string &name, const Slice &slice)
{
    insert (name.c_str(), slice);
}


Slice &
FrameBuffer::operator [] (const char name[])
{
    SliceMap::iterator i = _map.find (name);

    if (i == _map.end())
    {
        THROW (IEX_NAMESPACE::ArgExc,
               "Cannot find frame buffer slice \"" << name << "\".");
    }

    return i->second;
}


const Slice &
FrameBuffer::operator [] (const char name[]) const
{
    SliceMap::const_iterator i = _map.find (name);

    if (i == _map.end())
    {
        THROW (IEX_NAMESPACE::ArgExc,
               "Cannot find frame buffer slice \"" << name << "\".");
    }

    return i->second;
}


Slice &
FrameBuffer::operator [] (const string &name)
{
    return this->operator[] (name.c_str());
}


const Slice &
FrameBuffer::operator [] (const string &name) const
{
    return this->operator[] (name.c_str());
}


Slice *
FrameBuffer::findSlice (const char name[])
{
    SliceMap::iterator i = _map.find (name);
    return (i == _map.end())? 0: &i->second;
}


const Slice *
FrameBuffer::findSlice (const char name[]) const
{
    SliceMap::const_iterator i = _map.find (name);
    return (i == _map.end())? 0: &i->second;
}


Slice *
FrameBuffer::findSlice (const string &name)
{
    return findSlice (name.c_str());
}


const Slice *
FrameBuffer::findSlice (const string &name) const
{
    return findSlice (name.c_str());
}


FrameBuffer::Iterator
FrameBuffer::begin ()
{
    return _map.begin();
}


FrameBuffer::ConstIterator
FrameBuffer::begin () const
{
    return _map.begin();
}


FrameBuffer::Iterator
FrameBuffer::end ()
{
    return _map.end();
}


FrameBuffer::ConstIterator
FrameBuffer::end () const
{
    return _map.end();
}


FrameBuffer::Iterator
FrameBuffer::find (const char name[])
{
    return _map.find (name);
}


FrameBuffer::ConstIterator
FrameBuffer::find (const char name[]) const
{
    return _map.find (name);
}


FrameBuffer::Iterator
FrameBuffer::find (const string &name)
{
    return find (name.c_str());
}


FrameBuffer::ConstIterator
FrameBuffer::find (const string &name) const
{
    return find (name.c_str());
}


OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_EXIT
