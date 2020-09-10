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

#ifndef _PyImathQuat_h_
#define _PyImathQuat_h_

#include <Python.h>
#include <boost/python.hpp>
#include "PyImath.h"
#include <ImathQuat.h>
#include <ImathVec.h>

namespace PyImath {

template <class T> boost::python::class_<IMATH_NAMESPACE::Quat<T> > register_Quat();
template <class T> boost::python::class_<PyImath::FixedArray<IMATH_NAMESPACE::Quat<T> > > register_QuatArray();
typedef FixedArray<IMATH_NAMESPACE::Quatf>  QuatfArray;
typedef FixedArray<IMATH_NAMESPACE::Quatd>  QuatdArray;

}

template <class T> inline IMATH_NAMESPACE::Vec3<T> operator * (const IMATH_NAMESPACE::Vec3<T> &v, const IMATH_NAMESPACE::Quat<T> &q) { return v * q.toMatrix33(); }

template <class T> static PyImath::FixedArray<IMATH_NAMESPACE::Vec3<T> >
operator *(const IMATH_NAMESPACE::Vec3<T> &va, const PyImath::FixedArray<IMATH_NAMESPACE::Quat<T> > &vb)
{ size_t len = vb.len(); PyImath::FixedArray<IMATH_NAMESPACE::Vec3<T> > f(len); for (size_t i = 0; i < len; ++i) f[i] = va * vb[i]; return f; }

template <class T> static PyImath::FixedArray<IMATH_NAMESPACE::Vec3<T> >
operator *(const PyImath::FixedArray<IMATH_NAMESPACE::Vec3<T> > &va, const IMATH_NAMESPACE::Quat<T> &vb)
{ size_t len = va.len(); PyImath::FixedArray<IMATH_NAMESPACE::Vec3<T> > f(len); for (size_t i = 0; i < len; ++i) f[i] = va[i] * vb; return f; }

template <class T>
static PyImath::FixedArray<IMATH_NAMESPACE::Vec3<T> >
operator * (const PyImath::FixedArray<IMATH_NAMESPACE::Vec3<T> > &va, const PyImath::FixedArray<IMATH_NAMESPACE::Quat<T> > &vb)
{ size_t len = va.match_dimension(vb); PyImath::FixedArray<IMATH_NAMESPACE::Vec3<T> > f(len); for (size_t i = 0; i < len; ++i) f[i] = va[i] * vb[i]; return f; }

//

namespace PyImath {

// Other code in the Zeno code base assumes the existance of a class with the
// same name as the Imath class, and with static functions wrap() and
// convert() to produce a PyImath object from an Imath object and vice-versa,
// respectively.  The class Boost generates from the Imath class does not
// have these properties, so we define a companion class here.
// The template argument, T, is the element type (e.g.,float, double).

template <class T>
class Q {
  public:
    static PyObject *	wrap (const IMATH_NAMESPACE::Quat<T> &q);
    static int		convert (PyObject *p, IMATH_NAMESPACE::Quat<T> *q);
};

template <class T>
PyObject *
Q<T>::wrap (const IMATH_NAMESPACE::Quat<T> &q)
{
    typename boost::python::return_by_value::apply < IMATH_NAMESPACE::Quat<T> >::type converter;
    PyObject *p = converter (q);
    return p;
}

template <class T>
int
Q<T>::convert (PyObject *p, IMATH_NAMESPACE::Quat<T> *q)
{
    boost::python::extract <IMATH_NAMESPACE::Quatf> extractorQf (p);
    if (extractorQf.check())
    {
        IMATH_NAMESPACE::Quatf qf = extractorQf();
        q->r = T(qf.r);
        q->v.setValue (qf.v);
        return 1;
    }

    boost::python::extract <IMATH_NAMESPACE::Quatd> extractorQd (p);
    if (extractorQd.check())
    {
        IMATH_NAMESPACE::Quatd qd = extractorQd();
        q->r = T(qd.r);
        q->v.setValue (qd.v);
        return 1;
    }

    boost::python::extract <boost::python::tuple> extractorTuple (p);
    if (extractorTuple.check())
    {
        boost::python::tuple t = extractorTuple();
        if (t.attr ("__len__") () == 4)
        {
            // Extracting the tuple elements as doubles and casting them to
            // Ts in setValue() works better than extracting them as Ts from
            // the start.  

            double r = boost::python::extract <double> (t[0]);
            double x = boost::python::extract <double> (t[1]);
            double y = boost::python::extract <double> (t[2]);
            double z = boost::python::extract <double> (t[3]);
            q->r = T(r);
            q->v.setValue (T(x), T(y), T(z));
            return 1;
        }
    }

    return 0;
}

typedef Q<float>	Quatf;
typedef Q<double>	Quatd;

}


#endif
