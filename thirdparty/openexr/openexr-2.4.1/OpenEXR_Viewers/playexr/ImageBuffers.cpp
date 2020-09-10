///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006, Industrial Light & Magic, a division of Lucas
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
//	class ImageBuffers
//
//-----------------------------------------------------------------------------

#include "ImageBuffers.h"
#include <assert.h>


using namespace OPENEXR_IMF_NAMESPACE;
using namespace ILMTHREAD_NAMESPACE;
using namespace IMATH_NAMESPACE;


ImageBuffers::ImageBuffers ():
    forward (true),
    rgbMode (false),
    emptyBuffersSemaphore (NUM_BUFFERS),
    fullBuffersSemaphore (0),
    exitSemaphore1 (0),
    exitSemaphore2 (0)
{
    // empty
}


int			
ImageBuffers::numBuffers ()
{
    return NUM_BUFFERS;
}


OPENEXR_IMF_NAMESPACE::FrameBuffer &
ImageBuffers::frameBuffer (int i)
{
    assert (i >= 0 && i < NUM_BUFFERS);
    return _frameBuffers[i];
}


char * &
ImageBuffers::pixels (int i, int channel)
{
    assert (i >= 0 && i < NUM_BUFFERS && channel >= 0 && channel < 3);
    return _pixels[i][channel];
}


int &	
ImageBuffers::frameNumber (int i)
{
    assert (i >= 0 && i < NUM_BUFFERS);
    return _frameNumbers[i];
}
