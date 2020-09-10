///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005-2012, Industrial Light & Magic, a division of Lucas
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
//	class Semaphore -- implementation for OSX platform(it don't support unnamed Posix semaphores)
//	std::condition_variable + std::mutex emulation show poor performance
//
//-----------------------------------------------------------------------------

#if defined(__APPLE__)

#include "IlmThreadSemaphore.h"
#include "Iex.h"

ILMTHREAD_INTERNAL_NAMESPACE_SOURCE_ENTER


Semaphore::Semaphore (unsigned int value)
{
    // Calls to dispatch_semaphore_signal must be balanced with calls to wait().
    // Attempting to dispose of a semaphore with a count lower than value causes an EXC_BAD_INSTRUCTION exception.
    _semaphore = dispatch_semaphore_create (0);
    while (value--)
        post ();
}


Semaphore::~Semaphore ()
{
    dispatch_release (_semaphore);
}


void
Semaphore::wait ()
{
    dispatch_semaphore_wait (_semaphore, DISPATCH_TIME_FOREVER);
}


bool
Semaphore::tryWait ()
{
    return dispatch_semaphore_wait (_semaphore, DISPATCH_TIME_NOW) == 0;
}


void
Semaphore::post ()
{
    dispatch_semaphore_signal (_semaphore);
}


int
Semaphore::value () const
{
    throw IEX_NAMESPACE::NoImplExc ("Not implemented on this platform");

    return 0;
}


ILMTHREAD_INTERNAL_NAMESPACE_SOURCE_EXIT

#endif
