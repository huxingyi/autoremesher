#ifndef INCLUDED_IEXMATHFPU_H
#define INCLUDED_IEXMATHFPU_H

///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1997, Industrial Light & Magic, a division of Lucas
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


//------------------------------------------------------------------------
//
//	Functions to control floating point exceptions.
//
//------------------------------------------------------------------------

#include "IexMathIeeeExc.h"
#include "IexNamespace.h"

IEX_INTERNAL_NAMESPACE_HEADER_ENTER


//-----------------------------------------
// setFpExceptions() defines which floating
// point exceptions cause SIGFPE signals.
//-----------------------------------------

void setFpExceptions (int when = (IEEE_OVERFLOW | IEEE_DIVZERO | IEEE_INVALID));


//----------------------------------------
// fpExceptions() tells you which floating
// point exceptions cause SIGFPE signals.
//----------------------------------------

int fpExceptions ();


//------------------------------------------
// setFpExceptionHandler() defines a handler
// that will be called when SIGFPE occurs.
//------------------------------------------

extern "C" typedef void (* FpExceptionHandler) (int type, const char explanation[]);

void setFpExceptionHandler (FpExceptionHandler handler);

// -----------------------------------------
// handleExceptionsSetInRegisters() examines
// the exception registers and calls the
// floating point exception handler if the
// bits are set.  This function exists to 
// allow trapping of exception register states
// that can get set though no SIGFPE occurs.
// -----------------------------------------

void handleExceptionsSetInRegisters();


IEX_INTERNAL_NAMESPACE_HEADER_EXIT


#endif
