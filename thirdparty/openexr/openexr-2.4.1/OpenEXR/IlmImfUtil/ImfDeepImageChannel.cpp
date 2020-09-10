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
//      class DeepImageChannel
//
//----------------------------------------------------------------------------

#include "ImfDeepImageChannel.h"
#include "ImfDeepImageLevel.h"
#include <Iex.h>

using namespace IMATH_NAMESPACE;
using namespace IEX_NAMESPACE;
using namespace std;

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_ENTER


DeepImageChannel::DeepImageChannel
    (DeepImageLevel &level,
     bool pLinear)
:
    ImageChannel (level, 1, 1, pLinear)
{
    // empty
}


DeepImageChannel::~DeepImageChannel ()
{
    // empty
}

DeepImageLevel &
DeepImageChannel::deepLevel ()
{
    return static_cast <DeepImageLevel &> (level());
}


const DeepImageLevel &
DeepImageChannel::deepLevel () const
{
    return static_cast <const DeepImageLevel &> (level());
}


SampleCountChannel &
DeepImageChannel::sampleCounts ()
{
    return deepLevel().sampleCounts();
}


const SampleCountChannel &
DeepImageChannel::sampleCounts () const
{
    return deepLevel().sampleCounts();
}


void
DeepImageChannel::resize ()
{
    ImageChannel::resize();
}


OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_EXIT
