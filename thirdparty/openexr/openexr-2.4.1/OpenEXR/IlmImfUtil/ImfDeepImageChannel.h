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

#ifndef INCLUDED_IMF_DEEP_IMAGE_CHANNEL_H
#define INCLUDED_IMF_DEEP_IMAGE_CHANNEL_H

//----------------------------------------------------------------------------
//
//      class DeepImageChannel,
//      template class TypedDeepImageChannel<T>
//
//      For an explanation of images, levels and channels,
//      see the comments in header file Image.h.
//
//----------------------------------------------------------------------------

#include "ImfImageChannel.h"
#include "ImfSampleCountChannel.h"
#include "ImfImageLevel.h"
#include "ImfUtilExport.h"

#include "ImfDeepFrameBuffer.h"

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_ENTER

class DeepImageLevel;
class SampleCountChannel;

//
// Image channels:
//
// A TypedDeepImageChannel<T> holds the pixel data for a single channel
// of one level of a deep image.  Each pixel in the channel contains an
// array of n samples of type T, where T is either half, float or
// unsigned int, and n is stored in a separate sample count channel.
// Sample storage is allocated only for pixels within the data window
// of the level.
//

class DeepImageChannel: public ImageChannel
{
  public:

    //
    // Construct an OpenEXR frame buffer slice for this channel.
    // This function is needed reading an image from an OpenEXR
    // file and for saving an image in an OpenEXR file.
    // 

    virtual DeepSlice           slice () const = 0;

    //
    // Access to the image level to which this channel belongs.
    //

	IMFUTIL_EXPORT DeepImageLevel &            deepLevel();
	IMFUTIL_EXPORT const DeepImageLevel &      deepLevel() const;


    //
    // Access to the sample count channel for this deep channel.
    //

	IMFUTIL_EXPORT SampleCountChannel &        sampleCounts();
	IMFUTIL_EXPORT const SampleCountChannel &  sampleCounts() const;


  protected:

    friend class DeepImageLevel;

    IMFUTIL_EXPORT DeepImageChannel (DeepImageLevel &level, bool pLinear);
    IMFUTIL_EXPORT virtual ~DeepImageChannel();

    virtual void setSamplesToZero
                        (size_t i,
                         unsigned int oldNumSamples,
                         unsigned int newNumSamples) = 0;

    virtual void moveSampleList
                        (size_t i,
                         unsigned int oldNumSamples,
                         unsigned int newNumSamples,
                         size_t newSampleListPosition) = 0;

    virtual void moveSamplesToNewBuffer
                        (const unsigned int * oldNumSamples,
                         const unsigned int * newNumSamples,
                         const size_t * newSampleListPositions) = 0;

    virtual void initializeSampleLists () = 0;

    IMFUTIL_EXPORT virtual void resize ();

    virtual void resetBasePointer () = 0;
};


template <class T>
class TypedDeepImageChannel: public DeepImageChannel
{
  public:
    
    //
    // The OpenEXR pixel type of this channel (HALF, FLOAT or UINT).
    //

    virtual PixelType   pixelType () const;

    
    //
    // Construct an OpenEXR frame buffer slice for this channel.
    // This function is needed reading an image from an OpenEXR
    // file and for saving an image in an OpenEXR file.
    // 

    virtual DeepSlice   slice () const;


    //
    // Access to the pixel at pixel space location (x, y), without bounds
    // checking.  Accessing a location outside the data window of the image
    // level results in undefined behavior.
    //
    // The pixel contains a pointer to an array of samples to type T.  The
    // number of samples in this array is sampleCounts().at(x,y).
    //

    T *                 operator () (int x, int y);
    const T *           operator () (int x, int y) const;


    //
    // Access to the pixel at pixel space location (x, y), with bounds
    // checking.  Accessing a location outside the data window of the
    // image level throws an Iex::ArgExc exception.
    //

    T *                 at (int x, int y);
    const T *           at (int x, int y) const;

    //
    // Faster access to all pixels in a single horizontal row of the
    // channel.  Access is not bounds checked; accessing out of bounds
    // rows or pixels results in undefined behavior.
    //
    // Rows are numbered from 0 to pixelsPerColumn()-1, and each row
    // contains pixelsPerRow() values.  The number of samples in
    // row(r)[i] is sampleCounts().row(r)[i].
    //

    T * const *         row (int r);
    const T * const *   row (int r) const;

  private:
    
    friend class DeepImageLevel;

    TypedDeepImageChannel (DeepImageLevel &level, bool pLinear);
    virtual ~TypedDeepImageChannel ();

    virtual void setSamplesToZero
                            (size_t i,
                             unsigned int oldNumSamples,
                             unsigned int newNumSamples);

    virtual void moveSampleList
                            (size_t i,
                             unsigned int oldNumSamples,
                             unsigned int newNumSamples,
                             size_t newSampleListPosition);

    virtual void moveSamplesToNewBuffer
                            (const unsigned int * oldNumSamples,
                             const unsigned int * newNumSamples,
                             const size_t * newSampleListPositions);

    virtual void initializeSampleLists ();

    virtual void resize ();

    virtual void resetBasePointer ();

    T **    _sampleListPointers;    // Array of pointers to per-pixel
                                    //sample lists

    T **    _base;                  // Base pointer for faster access
                                    // to entries in _sampleListPointers

    T *     _sampleBuffer;          // Contiguous memory block that
                                    // contains all sample lists for
                                    // this channel
};


//
// Channel typedefs for the pixel data types supported by OpenEXR.
//

typedef TypedDeepImageChannel<half>         DeepHalfChannel;
typedef TypedDeepImageChannel<float>        DeepFloatChannel;
typedef TypedDeepImageChannel<unsigned int> DeepUIntChannel;


//-----------------------------------------------------------------------------
// Implementation of templates and inline functions
//-----------------------------------------------------------------------------

template <class T>
TypedDeepImageChannel<T>::TypedDeepImageChannel
    (DeepImageLevel &level,
     bool pLinear)
:
    DeepImageChannel (level, pLinear),
    _sampleListPointers (0),
    _base (0),
    _sampleBuffer (0)
{
    resize();
}


template <class T>
TypedDeepImageChannel<T>::~TypedDeepImageChannel ()
{
    delete [] _sampleListPointers;
    delete [] _sampleBuffer;
}


template <>
inline PixelType
DeepHalfChannel::pixelType () const
{
    return HALF;
}


template <>
inline PixelType
DeepFloatChannel::pixelType () const
{
    return FLOAT;
}


template <>
inline PixelType
DeepUIntChannel::pixelType () const
{
    return UINT;
}


template <class T>
DeepSlice
TypedDeepImageChannel<T>::slice () const
{
    return DeepSlice (pixelType(),                  // type
                      (char *) _base,               // base
                      sizeof (T*),                  // xStride
                      pixelsPerRow() * sizeof (T*), // yStride
                      sizeof (T),                   // sampleStride
                      xSampling(),
                      ySampling());
}


template <class T>
inline T *
TypedDeepImageChannel<T>::operator () (int x, int y)
{
    return _base[y * pixelsPerRow() + x];
}


template <class T>
inline const T *
TypedDeepImageChannel<T>::operator () (int x, int y) const
{
    return _base[y * pixelsPerRow() + x];
}


template <class T>
inline T *
TypedDeepImageChannel<T>::at (int x, int y)
{
    boundsCheck (x, y);
    return _base[y * pixelsPerRow() + x];
}


template <class T>
inline const T *
TypedDeepImageChannel<T>::at (int x, int y) const
{
    boundsCheck (x, y);
    return _base[y * pixelsPerRow() + x];
}


template <class T>
inline T * const *
TypedDeepImageChannel<T>::row (int r)
{
    return _base + r * pixelsPerRow();
}


template <class T>
inline const T * const *
TypedDeepImageChannel<T>::row (int r) const
{
    return _base + r * pixelsPerRow();
}


template <class T>
void
TypedDeepImageChannel<T>::setSamplesToZero
    (size_t i,
     unsigned int oldNumSamples,
     unsigned int newNumSamples)
{
    //
    // Expand the size of a sample list for a single pixel and
    // set the new samples in the list to 0.
    //
    // i                The position of the affected pixel in
    //                  the channel's _sampleListPointers.
    //
    // oldNumSamples    Original number of samples in the sample list.
    //
    // newNumSamples    New number of samples in the sample list.
    //

    for (unsigned int j = oldNumSamples; j < newNumSamples; ++j)
        _sampleListPointers[i][j] = 0;
}


template <class T>
void
TypedDeepImageChannel<T>::moveSampleList
    (size_t i,
     unsigned int oldNumSamples,
     unsigned int newNumSamples,
     size_t newSampleListPosition)
{
    //
    // Resize the sample list for a single pixel and move it to a new
    // position in the sample buffer for this channel.
    // 
    // i                        The position of the affected pixel in
    //                          the channel's _sampleListPointers.
    //
    // oldNumSamples            Original number of samples in sample list.
    //
    // newNumSamples            New number of samples in the sample list.
    //                          If the new number of samples is larger than
    //                          the old number of samples for a given sample
    //                          list, then the end of the new sample list
    //                          is filled with zeroes.  If the new number of
    //                          samples is smaller than the old one, then
    //                          samples at the end of the old sample list
    //                          are discarded.
    //
    // newSampleListPosition    The new position of the sample list in the
    //                          sample buffer.
    //

    T * oldSampleList = _sampleListPointers[i];
    T * newSampleList = _sampleBuffer + newSampleListPosition;

    if (oldNumSamples > newNumSamples)
    {
        for (unsigned int j = 0; j < newNumSamples; ++j)
            newSampleList[j] = oldSampleList[j];
    }
    else
    {
        for (unsigned int j = 0; j < oldNumSamples; ++j)
            newSampleList[j] = oldSampleList[j];

        for (unsigned int j = oldNumSamples; j < newNumSamples; ++j)
            newSampleList[j] = 0;
    }

    _sampleListPointers[i] = newSampleList;
}


template <class T>
void
TypedDeepImageChannel<T>::moveSamplesToNewBuffer
    (const unsigned int * oldNumSamples,
     const unsigned int * newNumSamples,
     const size_t * newSampleListPositions)
{
    //
    // Allocate a new sample buffer for this channel.
    // Copy the sample lists for all pixels into the new buffer.
    // Then delete the old sample buffer.
    //
    // oldNumSamples            Number of samples in each sample list in the
    //                          old sample buffer.
    //
    // newNumSamples            Number of samples in each sample list in
    //                          the new sample buffer.  If the new number
    //                          of samples is larger than the old number of
    //                          samples for a given sample list, then the
    //                          end of the new sample list is filled with
    //                          zeroes.  If the new number of samples is
    //                          smaller than the old one, then samples at
    //                          the end of the old sample list are discarded.
    //
    // newSampleListPositions   The positions of the new sample lists in the
    //                          new sample buffer.
    //

    T * oldSampleBuffer = _sampleBuffer;
    _sampleBuffer = new T [sampleCounts().sampleBufferSize()];

    for (size_t i = 0; i < numPixels(); ++i)
    {
        T * oldSampleList = _sampleListPointers[i];
        T * newSampleList = _sampleBuffer + newSampleListPositions[i];

        if (oldNumSamples[i] > newNumSamples[i])
        {
            for (unsigned int j = 0; j < newNumSamples[i]; ++j)
                newSampleList[j] = oldSampleList[j];
        }
        else
        {
            for (unsigned int j = 0; j < oldNumSamples[i]; ++j)
                newSampleList[j] = oldSampleList[j];

            for (unsigned int j = oldNumSamples[i]; j < newNumSamples[i]; ++j)
                newSampleList[j] = 0;
        }

        _sampleListPointers[i] = newSampleList;
    }

    delete [] oldSampleBuffer;
}


template <class T>
void
TypedDeepImageChannel<T>::initializeSampleLists ()
{
    //
    // Allocate a new set of sample lists for this channel, and
    // construct zero-filled sample lists for the pixels.
    //

    delete [] _sampleBuffer;

    _sampleBuffer = 0;          // set to 0 to prevent double deletion
                                // in case of an exception

    const unsigned int * numSamples = sampleCounts().numSamples();
    const size_t * sampleListPositions = sampleCounts().sampleListPositions();

    _sampleBuffer = new T [sampleCounts().sampleBufferSize()];
    
    resetBasePointer();

    for (size_t i = 0; i < numPixels(); ++i)
    {
        _sampleListPointers[i] = _sampleBuffer + sampleListPositions[i];

        for (unsigned int j = 0; j < numSamples[i]; ++j)
            _sampleListPointers[i][j] = T (0);
    }
}

template <class T>
void
TypedDeepImageChannel<T>::resize ()
{
    DeepImageChannel::resize();

    delete [] _sampleListPointers;
    _sampleListPointers = 0;
    _sampleListPointers = new T * [numPixels()];
    initializeSampleLists();
}


template <class T>
void
TypedDeepImageChannel<T>::resetBasePointer ()
{
    _base = _sampleListPointers -
            level().dataWindow().min.y * pixelsPerRow() -
            level().dataWindow().min.x;
}

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_EXIT

#endif
