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
//      class FlatImageLevel
//
//----------------------------------------------------------------------------

#include "ImfFlatImageLevel.h"
#include "ImfFlatImage.h"
#include <Iex.h>
#include <cassert>

using namespace IMATH_NAMESPACE;
using namespace IEX_NAMESPACE;
using namespace std;

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_ENTER


FlatImageLevel::FlatImageLevel
    (FlatImage& image,
     int xLevelNumber,
     int yLevelNumber,
     const Box2i& dataWindow)
:
    ImageLevel (image, xLevelNumber, yLevelNumber)
{
    resize (dataWindow);
}


FlatImage &
FlatImageLevel::flatImage ()
{
    return static_cast <FlatImage &> (image());
}


const FlatImage &
FlatImageLevel::flatImage () const
{
    return static_cast <const FlatImage &> (image());
}


FlatImageLevel::~FlatImageLevel ()
{
    clearChannels();
}


void			
FlatImageLevel::resize (const Box2i& dataWindow)
{
    //
    // Note: if the following code throws an exception, then the image level
    // may be left in an inconsistent state where some channels have been
    // resized, but others have not.  However, the image to which this level
    // belongs will catch the exception and clean up the mess.
    //

    ImageLevel::resize (dataWindow);

    for (ChannelMap::iterator i = _channels.begin(); i != _channels.end(); ++i)
        i->second->resize();
}


void
FlatImageLevel::shiftPixels (int dx, int dy)
{
    ImageLevel::shiftPixels (dx, dy);

    for (ChannelMap::iterator i = _channels.begin(); i != _channels.end(); ++i)
        i->second->resetBasePointer();
}


void
FlatImageLevel::insertChannel
    (const string& name,
     PixelType type,
     int xSampling,
     int ySampling,
     bool pLinear)
{
    if (_channels.find (name) != _channels.end())
        throwChannelExists (name);

    switch (type)
    {
      case HALF:
	_channels[name] =
            new FlatHalfChannel (*this, xSampling, ySampling, pLinear);
	break;

      case FLOAT:
	_channels[name] =
            new FlatFloatChannel (*this, xSampling, ySampling, pLinear);
	break;

      case UINT:
	_channels[name] =
            new FlatUIntChannel (*this, xSampling, ySampling, pLinear);
	break;

      default:
        assert (false);
    }
}


void
FlatImageLevel::eraseChannel (const string& name)
{
    ChannelMap::iterator i = _channels.find (name);

    if (i != _channels.end())
    {
        delete i->second;
        _channels.erase (i);
    }
}


void
FlatImageLevel::clearChannels ()
{
    for (ChannelMap::iterator i = _channels.begin(); i != _channels.end(); ++i)
	delete i->second;

    _channels.clear();
}


void
FlatImageLevel::renameChannel (const string &oldName, const string &newName)
{
    ChannelMap::iterator oldChannel = _channels.find (oldName);

    assert (oldChannel != _channels.end());
    assert (_channels.find (newName) == _channels.end());

    _channels[newName] = oldChannel->second;
    _channels.erase (oldChannel);
}


void
FlatImageLevel::renameChannels (const RenamingMap &oldToNewNames)
{
    renameChannelsInMap (oldToNewNames, _channels);
}


FlatImageChannel *
FlatImageLevel::findChannel (const string& name)
{
    ChannelMap::iterator i = _channels.find (name);

    if (i != _channels.end())
        return i->second;
    else
        return 0;
}


const FlatImageChannel *
FlatImageLevel::findChannel (const string& name) const
{
    ChannelMap::const_iterator i = _channels.find (name);

    if (i != _channels.end())
        return i->second;
    else
        return 0;
}


FlatImageChannel &
FlatImageLevel::channel (const string& name)
{
    ChannelMap::iterator i = _channels.find (name);

    if (i == _channels.end())
        throwBadChannelName (name);

    return *i->second;
}


const FlatImageChannel &
FlatImageLevel::channel (const string& name) const
{
    ChannelMap::const_iterator i = _channels.find (name);

    if (i == _channels.end())
        throwBadChannelName (name);

    return *i->second;
}


FlatImageLevel::Iterator
FlatImageLevel::begin ()
{
    return _channels.begin();
}


FlatImageLevel::ConstIterator
FlatImageLevel::begin () const
{
    return _channels.begin();
}


FlatImageLevel::Iterator
FlatImageLevel::end ()
{
    return _channels.end();
}


FlatImageLevel::ConstIterator
FlatImageLevel::end () const
{
    return _channels.end();
}


OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_EXIT
