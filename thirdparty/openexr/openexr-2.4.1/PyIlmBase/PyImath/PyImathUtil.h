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

#ifndef INCLUDED_PYIMATHUTIL_H
#define INCLUDED_PYIMATHUTIL_H

//----------------------------------------------------------------------------
//
//	PyImath.h -- miscellaneous classes, functions
//	and macros that are useful for Python wrapping
//	of C++ objects.
//
//----------------------------------------------------------------------------

#include "PyImathExport.h"
#include <Python.h>

namespace PyImath {


/**
 * PyAcquireLock ensures that python is prepared for multi-threaded use and
 * ensures that this thread has the global lock.
 *
 * This object must be instantiated (and continue to be in scope) during all
 * threaded api calls.  It assumes the python interpretter is instantiated and
 * multithreading is enabled.
 * 
 * Note: this is not compatible with additional interpreters (calls to
 * Py_NewInterpreter()); 
 */
class PyAcquireLock
{
  public:
    PYIMATH_EXPORT PyAcquireLock();
    PYIMATH_EXPORT ~PyAcquireLock();
  private:
    PyGILState_STATE _gstate;
};


/**
 * This object causes the python global lock to be released for the duration
 * of it's existence.
 *
 * This object should be instantiated (and continue to be in scope) in thread-
 * safe c++ functions called from python.  This call is designed to be
 * instantiated while an AcquireLock is in effect (nested).
 *
 */
class PyReleaseLock
{
  public:
    PYIMATH_EXPORT PyReleaseLock();
    PYIMATH_EXPORT ~PyReleaseLock();
  private:
    PyThreadState *_save;

};

/**
 * This object is safe object wrapper intended to use with boost python objects.
 *
 * This object correctly acquires the python lock for creation, copying and
 * desctruction of the given object.
 *
 */
template <class T>
class PySafeObject
{
  public:
    PySafeObject()
        : _object(0)
    {
        PyAcquireLock pylock;
        _object = new T();
    }

    PySafeObject(const T &value)
        : _object(0)
    {
        PyAcquireLock pylock;
        _object = new T(value);
    }

    ~PySafeObject()
    {
        PyAcquireLock pylock;
        delete _object;
        _object = 0;
    }

    PySafeObject(const PySafeObject &other)
        : _object(0)
    {
        PyAcquireLock pylock;
        _object = new T(*other._object);
    }

    const PySafeObject &
    operator = (const PySafeObject &other)
    {
        if (&other == this) return *this;
        PyAcquireLock pylock;
        *_object = *other._object;
        return *this;
    }

    bool
    operator == (const PySafeObject &other) const
    {
        if (&other == this) return true;
        PyAcquireLock pylock;
        return *_object == *other._object;
    }

    bool
    operator != (const PySafeObject &other) const
    {
        if (&other == this) return false;
        PyAcquireLock pylock;
        return *_object != *other._object;
    }

    T & get() { return *_object; }
    const T & get() const { return *_object; }

  private:

    T *_object;
};

} // namespace PyImath

#endif
