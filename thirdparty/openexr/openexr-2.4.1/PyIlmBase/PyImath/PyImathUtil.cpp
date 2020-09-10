///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2011, Industrial Light & Magic, a division of Lucas
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

#include "PyImathUtil.h"
#include <Iex.h>
#include <boost/python.hpp>
#include <pystate.h>

namespace PyImath {

PyAcquireLock::PyAcquireLock()
{
    _gstate = PyGILState_Ensure();
}

PyAcquireLock::~PyAcquireLock()
{
    PyGILState_Release(_gstate);
}

#ifdef PLATFORM_LINUX
// On Windows, this extern is not needed and produces a symbol mismatch at link time.
// We should verify that it's still needed on Linux for Python 2.6.
extern "C" PyThreadState *_PyThreadState_Current;
#endif

static bool
pyHaveLock()
{
#if PY_MAJOR_VERSION > 2
    return PyGILState_Check() != 0;
#else
    // This is very much dependent on the current Python
    // implementation of this functionality.  If we switch versions of
    // Python and the implementation changes, we'll have to change
    // this code as well and introduce a #define for the Python
    // version.
    
    if (!Py_IsInitialized())
	throw IEX_NAMESPACE::LogicExc("PyReleaseLock called without the interpreter initialized");

    PyThreadState *myThreadState = PyGILState_GetThisThreadState();

    // If the interpreter is initialized the gil is held if the
    // current thread's thread state is the current thread state
    return myThreadState != 0 && myThreadState == _PyThreadState_Current;
#endif
}

PyReleaseLock::PyReleaseLock()
{
    // only call PyEval_SaveThread if we have the interpreter lock held,
    // otherwise PyReleaseLock is a no-op.
    if (pyHaveLock())
        _save = PyEval_SaveThread();
    else
        _save = 0;
}

PyReleaseLock::~PyReleaseLock()
{
    if (_save != 0)
        PyEval_RestoreThread(_save);
}

} // namespace PyImath
