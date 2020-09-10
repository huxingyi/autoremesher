///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004, Industrial Light & Magic, a division of Lucas
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

#ifndef INCLUDED_IMAGE_H
#define INCLUDED_IMAGE_H

//----------------------------------------------------------------------------
//
//	Classes for storing OpenEXR images in memory.
//
//----------------------------------------------------------------------------

#include <ImfPixelType.h>
#include <ImfFrameBuffer.h>
#include <ImfArray.h>
#include <ImathBox.h>
#include <half.h>

#include <string>
#include <map>

#include "namespaceAlias.h"


class Image;


class ImageChannel
{
  public:

    friend class Image;

    ImageChannel (Image &image);
    virtual ~ImageChannel();

    virtual IMF::Slice	slice () const = 0;

    Image &		image ()		{return _image;}
    const Image &	image () const		{return _image;}

  private:

    virtual void	resize (int width, int height) = 0;

    Image &		_image;
};


template <class T>
class TypedImageChannel: public ImageChannel
{
  public:
    
    TypedImageChannel (Image &image, int width, int height);
    virtual ~TypedImageChannel ();
    
    IMF::PixelType	pixelType () const;

    virtual IMF::Slice	slice () const;

    T &				operator () (int x, int y);
    const T &			operator () (int x, int y) const;

  private:

    virtual void		resize (int width, int height);

    IMF::Array2D<T>	_pixels;
};


typedef TypedImageChannel<half>		HalfChannel;
typedef TypedImageChannel<float>	FloatChannel;
typedef TypedImageChannel<unsigned int>	UIntChannel;


class Image
{
  public:

    Image ();
    Image (const IMATH_NAMESPACE::Box2i &dataWindow);
   ~Image ();

    const IMATH_NAMESPACE::Box2i &	dataWindow () const;
    void			resize (const IMATH_NAMESPACE::Box2i &dataWindow);

    int				width () const;
    int				height () const;

    void			addChannel (const std::string &name,
        			            IMF::PixelType type);

    ImageChannel &		channel (const std::string &name);
    const ImageChannel &		channel (const std::string &name) const;

    template <class T>
    TypedImageChannel<T> &	typedChannel (const std::string &name);

    template <class T>
    const TypedImageChannel<T> &	typedChannel (const std::string &name) const;

  private:

    typedef std::map <std::string, ImageChannel *> ChannelMap;

    IMATH_NAMESPACE::Box2i		_dataWindow;
    ChannelMap			_channels;
};


//
// Implementation of templates and inline functions.
//

template <class T>
TypedImageChannel<T>::TypedImageChannel (Image &image, int width, int height):
    ImageChannel (image),
    _pixels (height, width)
{
    // empty
}


template <class T>
TypedImageChannel<T>::~TypedImageChannel ()
{
    // empty
}


template <>
inline OPENEXR_IMF_INTERNAL_NAMESPACE::PixelType
HalfChannel::pixelType () const
{
    return OPENEXR_IMF_INTERNAL_NAMESPACE::HALF;
}


template <>
inline OPENEXR_IMF_INTERNAL_NAMESPACE::PixelType
FloatChannel::pixelType () const
{
    return OPENEXR_IMF_INTERNAL_NAMESPACE::FLOAT;
}


template <>
inline OPENEXR_IMF_INTERNAL_NAMESPACE::PixelType
UIntChannel::pixelType () const
{
    return OPENEXR_IMF_INTERNAL_NAMESPACE::UINT;
}


template <class T>
OPENEXR_IMF_INTERNAL_NAMESPACE::Slice
TypedImageChannel<T>::slice () const
{
    const IMATH_NAMESPACE::Box2i &dw = image().dataWindow();

    return OPENEXR_IMF_INTERNAL_NAMESPACE::Slice::Make (
        pixelType(),
        &_pixels[0][0],
        dw,
        sizeof (T));
}


template <class T>
inline const T &
TypedImageChannel<T>::operator () (int x, int y) const
{
    return _pixels[y][x];
}


template <class T>
inline T &
TypedImageChannel<T>::operator () (int x, int y)
{
    return _pixels[y][x];
}


template <class T>
void
TypedImageChannel<T>::resize (int width, int height)
{
    _pixels.resizeEraseUnsafe (height, width);
}


inline const IMATH_NAMESPACE::Box2i &
Image::dataWindow () const
{
    return _dataWindow;
}


inline int
Image::width () const
{
    return _dataWindow.max.x - _dataWindow.min.x + 1;
}


inline int
Image::height () const
{
    return _dataWindow.max.y - _dataWindow.min.y + 1;
}


template <class T>
TypedImageChannel<T> &
Image::typedChannel (const std::string &name)
{
    return dynamic_cast <TypedImageChannel<T>&> (channel (name));
}


template <class T>
const TypedImageChannel<T> &
Image::typedChannel (const std::string &name) const
{
    return dynamic_cast <const TypedImageChannel<T>&> (channel (name));
}


#endif
