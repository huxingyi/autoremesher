///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1998-2011, Industrial Light & Magic, a division of Lucas
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

#ifndef _PyImathMatrix_h_
#define _PyImathMatrix_h_

#include <Python.h>
#include <boost/python.hpp>
#include "PyImath.h"
#include <ImathMatrix.h>
#include <ImathMatrixAlgo.h>

namespace PyImath {

template <class T> boost::python::class_<IMATH_NAMESPACE::Matrix33<T> > register_Matrix33();
template <class T> boost::python::class_<IMATH_NAMESPACE::Matrix44<T> > register_Matrix44();
template <class T> boost::python::class_<FixedArray<IMATH_NAMESPACE::Matrix44<T> > > register_M44Array();
template <class T> boost::python::class_<FixedArray<IMATH_NAMESPACE::Matrix33<T> > > register_M33Array();
typedef FixedArray<IMATH_NAMESPACE::Matrix33<float> >  M33fArray;
typedef FixedArray<IMATH_NAMESPACE::Matrix33<double> >  M33dArray;
typedef FixedArray<IMATH_NAMESPACE::Matrix44<float> >  M44fArray;
typedef FixedArray<IMATH_NAMESPACE::Matrix44<double> >  M44dArray;

//

// Other code in the Zeno code base assumes the existance of a class with the
// same name as the Imath class, and with static functions wrap() and
// convert() to produce a PyImath object from an Imath object and vice-versa,
// respectively.  The class Boost generates from the Imath class does not
// have these properties, so we define a companion class here.
// The template argument, T, is the element type (e.g.,float, double).

template <class T>
class M33 {
  public:
    static PyObject *	wrap (const IMATH_NAMESPACE::Matrix33<T> &m);
    static int		convert (PyObject *p, IMATH_NAMESPACE::Matrix33<T> *m);
};

template <class T>
class M44 {
  public:
    static PyObject *	wrap (const IMATH_NAMESPACE::Matrix44<T> &m);
    static int		convert (PyObject *p, IMATH_NAMESPACE::Matrix44<T> *m);
};

template <class T>
PyObject *
M33<T>::wrap (const IMATH_NAMESPACE::Matrix33<T> &m)
{
    typename boost::python::return_by_value::apply < IMATH_NAMESPACE::Matrix33<T> >::type converter;
    PyObject *p = converter (m);
    return p;
}

template <class T>
PyObject *
M44<T>::wrap (const IMATH_NAMESPACE::Matrix44<T> &m)
{
    typename boost::python::return_by_value::apply < IMATH_NAMESPACE::Matrix44<T> >::type converter;
    PyObject *p = converter (m);
    return p;
}

template <class T>
int
M33<T>::convert (PyObject *p, IMATH_NAMESPACE::Matrix33<T> *m)
{
    boost::python::extract <IMATH_NAMESPACE::M33f> extractorMf (p);
    if (extractorMf.check())
    {
        IMATH_NAMESPACE::M33f e = extractorMf();
        m->setValue (e);
        return 1;
    }

    boost::python::extract <IMATH_NAMESPACE::M33d> extractorMd (p);
    if (extractorMd.check())
    {
        IMATH_NAMESPACE::M33d e = extractorMd();
        m->setValue (e);
        return 1;
    }

    return 0;
}

template <class T>
int
M44<T>::convert (PyObject *p, IMATH_NAMESPACE::Matrix44<T> *m)
{
    boost::python::extract <IMATH_NAMESPACE::M44f> extractorMf (p);
    if (extractorMf.check())
    {
        IMATH_NAMESPACE::M44f e = extractorMf();
        m->setValue (e);
        return 1;
    }

    boost::python::extract <IMATH_NAMESPACE::M44d> extractorMd (p);
    if (extractorMd.check())
    {
        IMATH_NAMESPACE::M44d e = extractorMd();
        m->setValue (e);
        return 1;
    }

    return 0;
}

template <class Matrix>
boost::python::tuple
jacobiEigensolve(const Matrix& m)
{
    typedef typename Matrix::BaseType T;
    typedef typename Matrix::BaseVecType Vec;

    // For the C++ version, we just assume that the passed-in matrix is
    // symmetric, but we assume that many of our script users are less
    // sophisticated and might get tripped up by this.  Also, the cost
    // of doing this check is likely miniscule compared to the Pythonic
    // overhead.

    // Give a fairly generous tolerance to account for possible epsilon drift:
    const int d = Matrix::dimensions();
    const T tol = std::sqrt(IMATH_NAMESPACE::limits<T>::epsilon());
    for (int i = 0; i < d; ++i)
    {
        for (int j = i+1; j < d; ++j)
        {
            const T Aij = m[i][j],
                    Aji = m[j][i];
            ASSERT (std::abs(Aij - Aji) < tol,
                    IEX_NAMESPACE::ArgExc,
                    "Symmetric eigensolve requires a symmetric matrix (matrix[i][j] == matrix[j][i]).");
        }
    }

    Matrix tmp = m;
    Matrix Q;
    Vec S;
    IMATH_NAMESPACE::jacobiEigenSolver (tmp, S, Q);
    return boost::python::make_tuple (Q, S);
}


typedef M33<float>	M33f;
typedef M33<double>	M33d;

typedef M44<float>	M44f;
typedef M44<double>	M44d;

}

#endif
