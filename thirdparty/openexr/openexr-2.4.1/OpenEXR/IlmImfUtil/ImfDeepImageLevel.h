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

#ifndef INCLUDED_IMF_DEEP_IMAGE_LEVEL_H
#define INCLUDED_IMF_DEEP_IMAGE_LEVEL_H

//----------------------------------------------------------------------------
//
//      class DeepImageLevel
//      class DeepImageLevel::Iterator
//      class DeepImageLevel::ConstIterator
//
//      For an explanation of images, levels and channels,
//      see the comments in header file Image.h.
//
//----------------------------------------------------------------------------

#include "ImfDeepImageChannel.h"
#include "ImfSampleCountChannel.h"
#include "ImfImageLevel.h"
#include "ImfUtilExport.h"

#include <string>
#include <map>

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_ENTER

class DeepImage;

class DeepImageLevel : public ImageLevel
{
  public:

    //
    // Access to the image to which the level belongs.
    //

    IMFUTIL_EXPORT
    DeepImage &                 deepImage ();
    IMFUTIL_EXPORT
    const DeepImage &           deepImage () const;


    //
    // Access to deep channels by name:
    //
    // findChannel(n)           returns a pointer to the image channel with
    //                          name n, or 0 if no such channel exists.
    //
    // channel(n)               returns a reference to the image channel with
    //                          name n, or throws an Iex::ArgExc exception if
    //                          no such channel exists.
    //
    // findTypedChannel<T>(n)   returns a pointer to the image channel with
    //                          name n and type T, or 0 if no such channel
    //                          exists.
    //
    // typedChannel(n)          returns a reference to the image channel with
    //                          name n and type T, or throws an Iex::ArgExc
    //                          exception if no such channel exists.
    //

	
    IMFUTIL_EXPORT
    DeepImageChannel *          findChannel (const std::string& name);
	IMFUTIL_EXPORT
    const DeepImageChannel *    findChannel (const std::string& name) const;

    IMFUTIL_EXPORT
    DeepImageChannel &          channel (const std::string& name);
    IMFUTIL_EXPORT
    const DeepImageChannel &    channel (const std::string& name) const;

    template <class T>
    TypedDeepImageChannel<T> *       findTypedChannel
                                        (const std::string& name);

    template <class T>
    const TypedDeepImageChannel<T> * findTypedChannel
                                        (const std::string& name) const;

    template <class T>
    TypedDeepImageChannel<T> &       typedChannel
                                        (const std::string& name);

    template <class T>
    const TypedDeepImageChannel<T> & typedChannel
                                        (const std::string& name) const;
    
    //
    // Iterator-style access to deep channels
    //

    typedef std::map <std::string, DeepImageChannel *> ChannelMap;

	class Iterator;
	class ConstIterator;

	IMFUTIL_EXPORT
    Iterator        begin();
	IMFUTIL_EXPORT
    ConstIterator   begin() const;

	IMFUTIL_EXPORT
    Iterator        end();
	IMFUTIL_EXPORT
    ConstIterator   end() const;

    //
    // Access to the sample count channel
    //

	IMFUTIL_EXPORT
    SampleCountChannel &            sampleCounts();
	IMFUTIL_EXPORT
    const SampleCountChannel &      sampleCounts() const;

  private:
    
    friend class DeepImage;
    friend class SampleCountChannel;

    //
    // The constructor and destructor are private.
    // Deep image levels exist only as part of a deep image.
    //

     DeepImageLevel (DeepImage& image,
                     int xLevelNumber,
                     int yLevelNumber,
                     const IMATH_NAMESPACE::Box2i& dataWindow);

    ~DeepImageLevel ();

    void         setSamplesToZero (size_t i,
                                   unsigned int oldNumSamples,
                                   unsigned int newNumSamples);

    void         moveSampleList (size_t i,
                                 unsigned int oldNumSamples,
                                 unsigned int newNumSamples,
                                 size_t newSampleListPosition);

    void         moveSamplesToNewBuffer (const unsigned int * oldNumSamples,
                                         const unsigned int * newNumSamples,
                                         const size_t * newSampleListPositions);

    void         initializeSampleLists ();

    virtual void resize (const IMATH_NAMESPACE::Box2i& dataWindow);

    virtual void shiftPixels (int dx, int dy);

    virtual void insertChannel (const std::string& name,
                                PixelType type,
                                int xSampling,
                                int ySampling,
                                bool pLinear);

    virtual void eraseChannel (const std::string& name);

    virtual void clearChannels ();

    virtual void renameChannel (const std::string &oldName,
                                const std::string &newName);

    virtual void renameChannels (const RenamingMap &oldToNewNames);

    ChannelMap          _channels;
    SampleCountChannel  _sampleCounts;
};


class DeepImageLevel::Iterator
{
  public:

    IMFUTIL_EXPORT
    Iterator ();
    IMFUTIL_EXPORT
    Iterator (const DeepImageLevel::ChannelMap::iterator& i);


    //
    // Advance the iterator
    //

    IMFUTIL_EXPORT
    Iterator &              operator ++ ();
    IMFUTIL_EXPORT
    Iterator                operator ++ (int);


    //
    // Access to the channel to which the iterator points,
    // and to the name of that channel.
    //

    IMFUTIL_EXPORT
    const std::string &     name () const;
    IMFUTIL_EXPORT
    DeepImageChannel &      channel () const;

  private:

    friend class DeepImageLevel::ConstIterator;

    DeepImageLevel::ChannelMap::iterator _i;
};


class DeepImageLevel::ConstIterator
{
  public:

    IMFUTIL_EXPORT
    ConstIterator ();
    IMFUTIL_EXPORT
    ConstIterator (const DeepImageLevel::ChannelMap::const_iterator& i);
    IMFUTIL_EXPORT
    ConstIterator (const DeepImageLevel::Iterator& other);


    //
    // Advance the iterator
    //

    IMFUTIL_EXPORT
    ConstIterator &             operator ++ ();
    IMFUTIL_EXPORT
    ConstIterator               operator ++ (int);


    //
    // Access to the channel to which the iterator points,
    // and to the name of that channel.
    //

    IMFUTIL_EXPORT
    const std::string &         name () const;
    IMFUTIL_EXPORT
    const DeepImageChannel &    channel () const;

  private:

    friend bool operator ==
        (const ConstIterator &, const ConstIterator &);

    friend bool operator !=
        (const ConstIterator &, const ConstIterator &);

    DeepImageLevel::ChannelMap::const_iterator _i;
};


//-----------------------------------------------------------------------------
// Implementation of inline functions
//-----------------------------------------------------------------------------

template <class T>
TypedDeepImageChannel<T> *
DeepImageLevel::findTypedChannel (const std::string& name)
{
    return dynamic_cast <TypedDeepImageChannel<T>*> (findChannel (name));
}


template <class T>
const TypedDeepImageChannel<T> *
DeepImageLevel::findTypedChannel (const std::string& name) const
{
    return dynamic_cast <const TypedDeepImageChannel<T>*> (findChannel (name));
}


template <class T>
TypedDeepImageChannel<T> &
DeepImageLevel::typedChannel (const std::string& name)
{
    TypedDeepImageChannel<T> * ptr = findTypedChannel<T> (name);

    if (ptr == 0)
        throwBadChannelNameOrType (name);

    return *ptr;
}


template <class T>
const TypedDeepImageChannel<T> &
DeepImageLevel::typedChannel (const std::string& name) const
{
    const TypedDeepImageChannel<T> * ptr = findTypedChannel<T> (name);

    if (ptr == 0)
        throwBadChannelNameOrType (name);

    return *ptr;
}


inline SampleCountChannel &
DeepImageLevel::sampleCounts ()
{
    return _sampleCounts;
}


inline const SampleCountChannel &
DeepImageLevel::sampleCounts () const
{
    return _sampleCounts;
}


inline
DeepImageLevel::Iterator::Iterator (): _i()
{
    // empty
}


inline
DeepImageLevel::Iterator::Iterator
    (const DeepImageLevel::ChannelMap::iterator& i)
:
    _i (i)
{
    // empty
}


inline DeepImageLevel::Iterator &                
DeepImageLevel::Iterator::operator ++ ()
{
    ++_i;
    return *this;
}


inline DeepImageLevel::Iterator  
DeepImageLevel::Iterator::operator ++ (int)
{
    Iterator tmp = *this;
    ++_i;
    return tmp;
}


inline const std::string &
DeepImageLevel::Iterator::name () const
{
    return _i->first;
}


inline DeepImageChannel &        
DeepImageLevel::Iterator::channel () const
{
    return *_i->second;
}


inline
DeepImageLevel::ConstIterator::ConstIterator (): _i()
{
    // empty
}

inline
DeepImageLevel::ConstIterator::ConstIterator
    (const DeepImageLevel::ChannelMap::const_iterator& i): _i (i)
{
    // empty
}


inline
DeepImageLevel::ConstIterator::ConstIterator
    (const DeepImageLevel::Iterator& other): _i (other._i)
{
    // empty
}

inline DeepImageLevel::ConstIterator &
DeepImageLevel::ConstIterator::operator ++ ()
{
    ++_i;
    return *this;
}


inline DeepImageLevel::ConstIterator             
DeepImageLevel::ConstIterator::operator ++ (int)
{
    ConstIterator tmp = *this;
    ++_i;
    return tmp;
}


inline const std::string &
DeepImageLevel::ConstIterator::name () const
{
    return _i->first;
}

inline const DeepImageChannel &  
DeepImageLevel::ConstIterator::channel () const
{
    return *_i->second;
}


inline bool
operator == (const DeepImageLevel::ConstIterator& x,
             const DeepImageLevel::ConstIterator& y)
{
    return x._i == y._i;
}


inline bool
operator != (const DeepImageLevel::ConstIterator& x,
             const DeepImageLevel::ConstIterator& y)
{
    return !(x == y);
}

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_EXIT

#endif
