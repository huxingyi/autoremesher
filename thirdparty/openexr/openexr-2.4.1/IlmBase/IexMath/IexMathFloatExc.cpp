///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1997-2012, Industrial Light & Magic, a division of Lucas
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

//-----------------------------------------------------
//
//	A function to control which IEEE floating
//	point exceptions will be translated into
//	C++ MathExc exceptions.
//
//-----------------------------------------------------

#include <IexMathFloatExc.h>
#include <IexMacros.h>
#include <IexMathFpu.h>

#if 0
    #include <iostream>
    #define debug(x) (std::cout << x << std::flush)
#else
    #define debug(x)
#endif

IEX_INTERNAL_NAMESPACE_SOURCE_ENTER


namespace {

void
fpeHandler (int type, const char explanation[])
{
    switch (type)
    {
      case IEEE_OVERFLOW:
	throw OverflowExc (explanation);

      case IEEE_UNDERFLOW:
	throw UnderflowExc (explanation);

      case IEEE_DIVZERO:
	throw DivzeroExc (explanation);

      case IEEE_INEXACT:
	throw InexactExc (explanation);

      case IEEE_INVALID:
	throw InvalidFpOpExc (explanation);
    }

    throw MathExc (explanation);
}

} // namespace


void
mathExcOn (int when)
{
    debug ("mathExcOn (when = 0x" << std::hex << when << ")\n");

    setFpExceptions (when);
    setFpExceptionHandler (fpeHandler);
}


int
getMathExcOn ()
{
    int when = fpExceptions();

    debug ("getMathExcOn () == 0x" << std::hex << when << ")\n");

    return when;
}

MathExcOn::MathExcOn (int when)
: _changed (false)
{
    _saved = getMathExcOn();

    if (_saved != when)
    {
        _changed = true;
        mathExcOn (when);
    }
}

MathExcOn::~MathExcOn ()
{
    if (_changed)
        mathExcOn (_saved);
}

void
MathExcOn::handleOutstandingExceptions()
{
    handleExceptionsSetInRegisters();
}


IEX_INTERNAL_NAMESPACE_SOURCE_EXIT
