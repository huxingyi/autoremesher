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


//----------------------------------------------------------------------------
//
//	Classes for storing OpenEXR images in memory.
//
//----------------------------------------------------------------------------

#include "Image.h"
#include "namespaceAlias.h"

using namespace IMF;
using namespace IMATH_NAMESPACE;
using namespace std;


ImageChannel::ImageChannel (Image &image): _image (image)
{
    // empty
}


ImageChannel::~ImageChannel ()
{
    // empty
}


Image::Image (): _dataWindow (Box2i (V2i (0, 0), V2i (0, 0)))
{
    // empty
}


Image::Image (const Box2i &dataWindow): _dataWindow (dataWindow)
{
    // empty
}


Image::~Image ()
{
    for (ChannelMap::iterator i = _channels.begin(); i != _channels.end(); ++i)
	delete i->second;
}


void			
Image::resize (const IMATH_NAMESPACE::Box2i &dataWindow)
{
    _dataWindow = dataWindow;

    for (ChannelMap::iterator i = _channels.begin(); i != _channels.end(); ++i)
	i->second->resize (width(), height());
}


void
Image::addChannel (const string &name, PixelType type)
{
    switch (type)
    {
      case IMF::HALF:
	_channels[name] = new HalfChannel (*this, width(), height());
	break;

      case IMF::FLOAT:
	_channels[name] = new FloatChannel (*this, width(), height());
	break;

      case IMF::UINT:
	_channels[name] = new UIntChannel (*this, width(), height());
	break;

      default:
	throw IEX_NAMESPACE::ArgExc ("Unknown channel type.");
    }
}


ImageChannel &
Image::channel (const string &name)
{
    return *_channels.find(name)->second;
}


const ImageChannel &
Image::channel (const string &name) const
{
    return *_channels.find(name)->second;
}
