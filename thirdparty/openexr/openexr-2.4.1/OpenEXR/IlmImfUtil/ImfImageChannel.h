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

#ifndef INCLUDED_IMF_IMAGE_CHANNEL_H
#define INCLUDED_IMF_IMAGE_CHANNEL_H

//----------------------------------------------------------------------------
//
//      class ImageChannel
//
//      For an explanation of images, levels and channels,
//      see the comments in header file Image.h.
//
//----------------------------------------------------------------------------

#include "ImfUtilExport.h"

#include <ImfPixelType.h>
#include <ImfFrameBuffer.h>
#include <ImfChannelList.h>
#include <ImathBox.h>
#include <half.h>

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_ENTER

class ImageLevel;

//
// Image channels:
//
// An image channel holds the pixel data for a single channel of one level
// of an image.  Separate classes for flat and deep channels are derived
// from the ImageChannel base class.
//

class ImageLevel;

class ImageChannel
{
  public:

    //
    // The OpenEXR pixel type of this channel (HALF, FLOAT or UINT).
    //

    virtual PixelType   pixelType () const = 0;

    //
    // Generate an OpenEXR channel for this image channel.
    //
    
    IMFUTIL_EXPORT
    Channel             channel () const;


    //
    // Access to x and y sampling rates, "perceptually linear" flag,
    // and the number of pixels that are stored in this channel.
    // 

    int                 xSampling () const          {return _xSampling;}
    int                 ySampling () const          {return _ySampling;}
    bool                pLinear () const            {return _pLinear;}
    int                 pixelsPerRow () const       {return _pixelsPerRow;}
    int                 pixelsPerColumn () const    {return _pixelsPerColumn;}
    size_t              numPixels () const          {return _numPixels;}


    //
    // Access to the image level to which this channel belongs.
    //

    ImageLevel &        level ()                    {return _level;}
    const ImageLevel &  level () const              {return _level;}

  protected:

    IMFUTIL_EXPORT
    ImageChannel (ImageLevel &level,
                  int xSampling,
                  int ySampling,
                  bool pLinear);

    IMFUTIL_EXPORT
    virtual ~ImageChannel();

    IMFUTIL_EXPORT
    virtual void        resize ();

    IMFUTIL_EXPORT
	void                boundsCheck(int x, int y) const;

  private:

    ImageChannel (const ImageChannel &);                // not implemented
    ImageChannel & operator = (const ImageChannel &);   // not implemented

    ImageLevel &        _level;
    int                 _xSampling;
    int                 _ySampling;
    bool                _pLinear;
    int                 _pixelsPerRow;
    int                 _pixelsPerColumn;
    size_t              _numPixels;
};


OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_EXIT

#endif
