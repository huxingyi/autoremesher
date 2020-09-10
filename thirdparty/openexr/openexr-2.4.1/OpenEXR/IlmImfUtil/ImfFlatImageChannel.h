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

#ifndef INCLUDED_IMF_FLAT_IMAGE_CHANNEL_H
#define INCLUDED_IMF_FLAT_IMAGE_CHANNEL_H

//----------------------------------------------------------------------------
//
//      class FlatImageChannel,
//      template class TypedFlatImageChannel<T>
//
//      For an explanation of images, levels and channels,
//      see the comments in header file Image.h.
//
//----------------------------------------------------------------------------

#include "ImfImageChannel.h"
#include "ImfUtilExport.h"
#include "ImfImageLevel.h"

#include <ImfPixelType.h>
#include <ImfFrameBuffer.h>
#include <ImathBox.h>
#include <half.h>

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_ENTER

class FlatImageLevel;

//
// Image channels:
//
// A TypedFlatImageChannel<T> holds the pixel data for a single channel
// of one level of a flat image.  The pixels in the channel are of type T,
// where T is either half, float or unsigned int.  Storage is allocated
// only for pixels within the data window of the level.
//

class FlatImageChannel: public ImageChannel
{
  public:

    //
    // Construct an OpenEXR frame buffer slice for this channel.
    // This function is needed reading an image from an OpenEXR
    // file and for saving an image in an OpenEXR file.
    // 

    virtual Slice           slice () const = 0;


    //
    // Access to the flat image level to which this channel belongs.
    //

    IMFUTIL_EXPORT
    FlatImageLevel &        flatLevel ();
    IMFUTIL_EXPORT
    const FlatImageLevel &  flatLevel () const;

  protected:

    friend class FlatImageLevel;

    IMFUTIL_EXPORT
    FlatImageChannel (FlatImageLevel &level,
                      int xSampling,
                      int ySampling,
                      bool pLinear);

    IMFUTIL_EXPORT
    virtual ~FlatImageChannel();

    IMFUTIL_EXPORT
    virtual void            resize ();

    virtual void            resetBasePointer () = 0;
};


template <class T>
class TypedFlatImageChannel: public FlatImageChannel
{
  public:
    
    //
    // The OpenEXR pixel type of this channel (HALF, FLOAT or UINT).
    //

    virtual PixelType   pixelType () const;

    
    //
    // Construct an OpenEXR frame buffer slice for this channel.
    // 

    virtual Slice       slice () const;


    //
    // Access to the pixel at pixel space location (x, y), without
    // bounds checking.  Accessing a location outside the data window
    // of the image level results in undefined behavior.
    //

    T &                 operator () (int x, int y);
    const T &           operator () (int x, int y) const;


    //
    // Access to the pixel at pixel space location (x, y), with bounds
    // checking.  Accessing a location outside the data window of the
    // image level throws an Iex::ArgExc exception.
    //

    T &                 at (int x, int y);
    const T &           at (int x, int y) const;

    //
    // Faster access to all pixels in a single horizontal row of the
    // channel.  Rows are numbered from 0 to pixelsPerColumn()-1, and
    // each row contains pixelsPerRow() values.
    // Access is not bounds checked; accessing out of bounds rows or
    // pixels results in undefined behavior.
    //

    T *                 row (int r);
    const T *           row (int r) const;

  private:
    
    friend class FlatImageLevel;

    //
    // The constructor and destructor are not public because flat
    // image channels exist only as parts of a flat image level.
    //

    TypedFlatImageChannel (FlatImageLevel &level,
                           int xSampling,
                           int ySampling,
                           bool pLinear);

    virtual ~TypedFlatImageChannel ();

    virtual void        resize ();

    virtual void        resetBasePointer ();

    T *                 _pixels;        // Pointer to allocated storage
    T *                 _base;          // Base pointer for faster pixel access
};


//
// Channel typedefs for the pixel data types supported by OpenEXR.
//

typedef TypedFlatImageChannel<half>         FlatHalfChannel;
typedef TypedFlatImageChannel<float>        FlatFloatChannel;
typedef TypedFlatImageChannel<unsigned int> FlatUIntChannel;


//-----------------------------------------------------------------------------
// Implementation of templates and inline functions
//-----------------------------------------------------------------------------


template <class T>
TypedFlatImageChannel<T>::TypedFlatImageChannel
    (FlatImageLevel &level,
     int xSampling,
     int ySampling,
     bool pLinear)
:
    FlatImageChannel (level, xSampling, ySampling, pLinear),
    _pixels (0),
    _base (0)
{
    resize();
}


template <class T>
TypedFlatImageChannel<T>::~TypedFlatImageChannel ()
{
    delete [] _pixels;
}


template <>
inline PixelType
FlatHalfChannel::pixelType () const
{
    return HALF;
}


template <>
inline PixelType
FlatFloatChannel::pixelType () const
{
    return FLOAT;
}


template <>
inline PixelType
FlatUIntChannel::pixelType () const
{
    return UINT;
}


template <class T>
Slice
TypedFlatImageChannel<T>::slice () const
{
    return Slice (pixelType(),                 // type
                  (char *) _base,              // base
                  sizeof (T),                  // xStride
                  pixelsPerRow() * sizeof (T), // yStride
                  xSampling(),
                  ySampling());
}


template <class T>
inline T &
TypedFlatImageChannel<T>::operator () (int x, int y)
{
    return _base[(y / ySampling()) * pixelsPerRow() + (x / xSampling())];
}


template <class T>
inline const T &
TypedFlatImageChannel<T>::operator () (int x, int y) const
{
    return _base[(y / ySampling()) * pixelsPerRow() + (x / xSampling())];
}


template <class T>
inline T &
TypedFlatImageChannel<T>::at (int x, int y)
{
    boundsCheck (x, y);
    return _base[(y / ySampling()) * pixelsPerRow() + (x / xSampling())];
}


template <class T>
inline const T &
TypedFlatImageChannel<T>::at (int x, int y) const
{
    boundsCheck (x, y);
    return _base[(y / ySampling()) * pixelsPerRow() + (x / xSampling())];
}


template <class T>
inline T *
TypedFlatImageChannel<T>::row (int r)
{
    return _base + r * pixelsPerRow();
}


template <class T>
inline const T *
TypedFlatImageChannel<T>::row (int n) const
{
    return _base + n * pixelsPerRow();
}


template <class T>
void
TypedFlatImageChannel<T>::resize ()
{
    delete [] _pixels;
    _pixels = 0;

    FlatImageChannel::resize();  // may throw an exception

    _pixels = new T [numPixels()];

    for (size_t i = 0; i < numPixels(); ++i)
        _pixels[i] = T (0);

    resetBasePointer ();
}


template <class T>
void
TypedFlatImageChannel<T>::resetBasePointer ()
{
    _base = _pixels -
            (level().dataWindow().min.y / ySampling()) * pixelsPerRow() -
            (level().dataWindow().min.x / xSampling());
}

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_EXIT

#endif
