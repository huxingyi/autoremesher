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

#ifndef INCLUDED_IMAGE_H
#define INCLUDED_IMAGE_H

//----------------------------------------------------------------------------
//
//	Classes for storing OpenEXR images in memory.
//
//----------------------------------------------------------------------------

#include <ImfChannelList.h>
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
    virtual void                         black() =0;
  private:

    virtual void	resize () = 0;

    Image &		_image;
};


template <class T>
class TypedImageChannel: public ImageChannel
{
  public:
    
    TypedImageChannel (Image &image, int xSampling, int ySampling);

    virtual ~TypedImageChannel ();
    
    IMF::PixelType	pixelType () const;

    virtual IMF::Slice	slice () const;
    
    
  private:

    virtual void	resize ();
    virtual void black();
    
    int			_xSampling;
    int			_ySampling;
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

   const IMATH_NAMESPACE::Box2i &		dataWindow () const;
   void				resize (const IMATH_NAMESPACE::Box2i &dataWindow);
   
   int				width () const;
   int				height () const;

   void				addChannel (const std::string &name,
					    const IMF::Channel &channel);

   ImageChannel &		channel (const std::string &name);
   const ImageChannel &		channel (const std::string &name) const;

   template <class T>
   TypedImageChannel<T> &	typedChannel (const std::string &name);

   template <class T>
   const TypedImageChannel<T> &	typedChannel (const std::string &name) const;

   
   
  private:

   typedef std::map <std::string, ImageChannel *> ChannelMap;

   IMATH_NAMESPACE::Box2i			_dataWindow;
   ChannelMap			_channels;
};


//
// Implementation of templates and inline functions.
//

template <class T>
TypedImageChannel<T>::TypedImageChannel
    (Image &image,
     int xSampling,
     int ySampling)
:
    ImageChannel (image),
    _xSampling (xSampling),
    _ySampling (ySampling),
    _pixels (0, 0)
{
    if ( _xSampling < 1 || _ySampling < 1 )
        throw IEX_NAMESPACE::ArgExc ("Invalid x/y sampling values");
    resize();
}


template <class T>
TypedImageChannel<T>::~TypedImageChannel ()
{
    // empty
}


template <>
inline IMF::PixelType
HalfChannel::pixelType () const
{
    return IMF::HALF;
}


template <>
inline IMF::PixelType
FloatChannel::pixelType () const
{
    return IMF::FLOAT;
}


template <>
inline IMF::PixelType
UIntChannel::pixelType () const
{
    return IMF::UINT;
}


template <class T>
IMF::Slice
TypedImageChannel<T>::slice () const
{
    const IMATH_NAMESPACE::Box2i &dw = image().dataWindow();
    int w = dw.max.x - dw.min.x + 1;

    return IMF::Slice::Make (
        pixelType(),
        &_pixels[0][0],
        dw,
        sizeof(T),
        (w / _xSampling) * sizeof (T),
        _xSampling,
        _ySampling);
}


template <class T>
void
TypedImageChannel<T>::resize ()
{
    int width  = image().width()  / _xSampling;
    int height = image().height() / _ySampling;

    _pixels.resizeEraseUnsafe (height, width);
}


template <class T>
void
TypedImageChannel<T>::black ()
{
    size_t nx = static_cast<size_t>( image().width() ) / static_cast<size_t>( _xSampling );
    size_t ny = static_cast<size_t>( image().height() ) / static_cast<size_t>( _ySampling );
    memset(&_pixels[0][0],0,nx*ny*sizeof(T));
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
