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

#ifndef INCLUDED_IMF_IMAGE_LEVEL_H
#define INCLUDED_IMF_IMAGE_LEVEL_H

//----------------------------------------------------------------------------
//
//      class ImageLevel
//
//      For an explanation of images, levels and channels,
//      see the comments in header file Image.h.
//
//----------------------------------------------------------------------------

#include "ImfUtilExport.h"
#include "ImfImageChannel.h"
#include "ImfImageChannelRenaming.h"
#include <ImathBox.h>
#include <string>

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_ENTER

class Image;


class ImageLevel
{
  public:

    //
    // Access to the image to which the level belongs.
    //


    Image &                     image ()                {return _image;}
    const Image &               image () const          {return _image;}


    //
    // Access to the level number and the data window of this level.
    //

    int                         xLevelNumber () const   {return _xLevelNumber;}
    int                         yLevelNumber () const   {return _yLevelNumber;}

    const IMATH_NAMESPACE::Box2i & dataWindow () const  {return _dataWindow;}


  protected:
    
    friend class Image;

    IMFUTIL_EXPORT
    ImageLevel (Image& image,
                int xLevelNumber,
                int yLevelNumber);

    IMFUTIL_EXPORT
    virtual ~ImageLevel ();

    IMFUTIL_EXPORT
    virtual void    resize (const IMATH_NAMESPACE::Box2i& dataWindow);

    IMFUTIL_EXPORT
    virtual void    shiftPixels (int dx, int dy);

    virtual void    insertChannel (const std::string& name,
                                   PixelType type,
                                   int xSampling,
                                   int ySampling,
                                   bool pLinear) = 0;

    virtual void    eraseChannel (const std::string& name) = 0;

    virtual void    clearChannels () = 0;

    virtual void    renameChannel (const std::string &oldName,
                                   const std::string &newName) = 0;

    virtual void    renameChannels (const RenamingMap &oldToNewNames) = 0;

    IMFUTIL_EXPORT
    void            throwChannelExists(const std::string& name) const;
    IMFUTIL_EXPORT
    void            throwBadChannelName(const std::string& name) const;
    IMFUTIL_EXPORT 
    void            throwBadChannelNameOrType (const std::string& name) const;

  private:

    ImageLevel (const ImageLevel &);                // not implemented
    ImageLevel & operator = (const ImageLevel &);   // not implemented

    Image &                 _image;
    int                     _xLevelNumber;
    int                     _yLevelNumber;
    IMATH_NAMESPACE::Box2i  _dataWindow;
};


OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_EXIT

#endif
