///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2011, Industrial Light & Magic, a division of Lucas
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

#ifndef INCLUDED_PYIMATH_DECORATORS_H
#define INCLUDED_PYIMATH_DECORATORS_H

#include <boost/python.hpp>

namespace PyImath
{

// These function add __copy__ and __deepcopy__ methods
// to python classes by simply wrapping the copy constructors
// This interface is needed for using these classes with
// the python copy module.

template <class T>
static T
copy(const T& x)
{
    return T(x);
}

template <class T>
static T
deepcopy(const T& x, boost::python::dict&)
{
    return copy(x);
}

template <class T, class X1, class X2, class X3>
boost::python::class_<T,X1,X2,X3>&
decoratecopy(boost::python::class_<T,X1,X2,X3>& cls)
{
    cls.def("__copy__",&copy<T>);
    cls.def("__deepcopy__",&deepcopy<T>);
    return cls;
}

} // namespace PyImath

#endif // INCLUDED_PYIMATH_DECORATORS_H

