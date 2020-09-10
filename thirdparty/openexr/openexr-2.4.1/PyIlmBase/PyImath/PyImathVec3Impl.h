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

#ifndef _PyImathVec3Impl_h_
#define _PyImathVec3Impl_h_

//
// This .C file was turned into a header file so that instantiations
// of the various V3* types can be spread across multiple files in
// order to work around MSVC limitations.
//

#include "PyImathVec.h"
#include "PyImathDecorators.h"
#include <Python.h>
#include <boost/python.hpp>
#include <boost/python/make_constructor.hpp>
#include <boost/format.hpp>
#include "PyImath.h"
#include <ImathVec.h>
#include <ImathVecAlgo.h>
#include <Iex.h>
#include "PyImathMathExc.h"

namespace PyImath {
using namespace boost::python;
using namespace IMATH_NAMESPACE;

template <class T> struct Vec3Name      { static const char *value(); };

// create a new default constructor that initializes Vec3<T> to zero.
template <class T>
static Vec3<T> * Vec3_construct_default()
{
    return new Vec3<T>(T(0),T(0),T(0));
}

template <class T>
static Vec3<T> * Vec3_object_constructor1(const object &obj)
{
    Vec3<T> w;
    extract<Vec3<int> >     e1(obj);
    extract<Vec3<float> >   e2(obj);
    extract<Vec3<double> >  e3(obj);
    extract<tuple>          e4(obj);
    extract<double>         e5(obj);
    extract<list>           e6(obj);
    
    if(e1.check())      { w = e1(); }
    else if(e2.check()) { w = e2(); }
    else if(e3.check()) { w = e3(); }
    else if(e4.check())
    {
        tuple t = e4();
        if(t.attr("__len__")() == 3)
        {
            w.x = extract<T>(t[0]);
            w.y = extract<T>(t[1]);
            w.z = extract<T>(t[2]);
        }
        else
            THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 3");
        
    }
    else if(e5.check()) { T a = e5(); w.setValue(a, a, a); }
    else if(e6.check())
    {
        list l = e6();
        if(l.attr("__len__")() == 3)
        {
            w.x = extract<T>(l[0]);
            w.y = extract<T>(l[1]);
            w.z = extract<T>(l[2]);
        }
        else
            THROW(IEX_NAMESPACE::LogicExc, "list must have length of 3");
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to Vec3 constructor");
    
    Vec3<T> *v = new Vec3<T>;
    *v = w;
    
    return v;

}

template <class T>
static Vec3<T> * Vec3_object_constructor2(const object &obj1, const object &obj2, const object &obj3)
{
    extract<double>    e1(obj1);
    extract<double>    e2(obj2);
    extract<double>    e3(obj3);
    Vec3<T> *v = new Vec3<T>;
    
    if(e1.check()) { v->x = e1();}
    else { THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to Vec3 constructor"); }
    
    if(e2.check()) { v->y = e2();}
    else { THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to Vec3 constructor"); }    

    if(e3.check()) { v->z = e3();}
    else { THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to Vec3 constructor"); } 
    
    return v;
}



// Implementations of str and repr are same here,
// but we'll specialize repr for float and double to make them exact.
template <class T>
static std::string Vec3_str(const Vec3<T> &v)
{
    std::stringstream stream;
    stream << Vec3Name<T>::value() << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return stream.str();
}
template <class T>
static std::string Vec3_repr(const Vec3<T> &v)
{
    std::stringstream stream;
    stream << Vec3Name<T>::value() << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return stream.str();
}

template <class T>
static IMATH_NAMESPACE::Vec3<T>
Vec3_cross(const IMATH_NAMESPACE::Vec3<T> &v, const IMATH_NAMESPACE::Vec3<T> &other) 
{ 
    MATH_EXC_ON;
    return v.cross(other);
}

template <class T>
static FixedArray<IMATH_NAMESPACE::Vec3<T> >
Vec3_cross_Vec3Array(const IMATH_NAMESPACE::Vec3<T> &va, const FixedArray<IMATH_NAMESPACE::Vec3<T> > &vb) 
{ 
    MATH_EXC_ON;
    size_t len = vb.len(); 
    FixedArray<IMATH_NAMESPACE::Vec3<T> > f(len); 
    for (size_t i = 0; i < len; ++i) 
        f[i] = va.cross(vb[i]); 
    return f; 
}

template <class T>
static T
Vec3_dot(const IMATH_NAMESPACE::Vec3<T> &v, const IMATH_NAMESPACE::Vec3<T> &other) 
{ 
    MATH_EXC_ON;
    return v.dot(other);
}

template <class T>
static FixedArray<T>
Vec3_dot_Vec3Array(const IMATH_NAMESPACE::Vec3<T> &va, const FixedArray<IMATH_NAMESPACE::Vec3<T> > &vb) 
{ 
    MATH_EXC_ON;
    size_t len = vb.len(); 
    FixedArray<T> f(len); 
    for (size_t i = 0; i < len; ++i) 
        f[i] = va.dot(vb[i]); 
    return f; 
}

template <class T>
static T
Vec3_length(const IMATH_NAMESPACE::Vec3<T> &v) 
{ 
    MATH_EXC_ON;
    return v.length();
}

template <class T>
static T
Vec3_length2(const IMATH_NAMESPACE::Vec3<T> &v) 
{ 
    MATH_EXC_ON;
    return v.length2();
}

template <class T>
static const Vec3<T> &
Vec3_normalize(IMATH_NAMESPACE::Vec3<T> &v) 
{ 
    MATH_EXC_ON;
    return v.normalize();
}

template <class T>
static const Vec3<T> &
Vec3_normalizeExc(IMATH_NAMESPACE::Vec3<T> &v) 
{ 
    MATH_EXC_ON;
    return v.normalizeExc();
}

template <class T>
static const Vec3<T> &
Vec3_normalizeNonNull(IMATH_NAMESPACE::Vec3<T> &v) 
{ 
    MATH_EXC_ON;
    return v.normalizeNonNull();
}

template <class T>
static Vec3<T>
Vec3_normalized(const IMATH_NAMESPACE::Vec3<T> &v) 
{ 
    MATH_EXC_ON;
    return v.normalized();
}

template <class T>
static Vec3<T>
Vec3_normalizedExc(const IMATH_NAMESPACE::Vec3<T> &v) 
{ 
    MATH_EXC_ON;
    return v.normalizedExc();
}

template <class T>
static Vec3<T>
Vec3_normalizedNonNull(const IMATH_NAMESPACE::Vec3<T> &v) 
{ 
    MATH_EXC_ON;
    return v.normalizedNonNull();
}

template <class T>
static Vec3<T>
closestVertex(Vec3<T> &p, const Vec3<T> &v0, const Vec3<T> &v1, const Vec3<T> &v2)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::closestVertex(v0, v1, v2, p);
}

template <class T>
static const Vec3<T> &
Vec3_negate(IMATH_NAMESPACE::Vec3<T> &v) 
{ 
    MATH_EXC_ON;
    return v.negate();
}

template <class T>
static Vec3<T>
orthogonal(const Vec3<T> &v, const Vec3<T> &v0)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::orthogonal(v, v0);
}

template <class T>
static Vec3<T>
project(const Vec3<T> &v, const Vec3<T> &v0)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::project(v0, v);
}

template <class T>
static Vec3<T>
reflect(const Vec3<T> &v, const Vec3<T> &v0)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::reflect(v, v0);
}

template <class T>
static void
setValue(Vec3<T> &v, T a, T b, T c)
{
    v.x = a;
    v.y = b;
    v.z = c;
}

template <class T>
static Vec3<T>
Vec3_add (const Vec3<T> &v, const Vec3<T> &w)
{
    MATH_EXC_ON;
    return v + w;
}

template <class T>
static Vec3<T>
Vec3_sub (const Vec3<T> &v, const Vec3<T> &w)
{
    MATH_EXC_ON;
    return v - w;
}

template <class T>
static Vec3<T>
Vec3_neg (const Vec3<T> &v)
{
    MATH_EXC_ON;
    return -v;
}

template <class T, class U>
static Vec3<T>
Vec3_mul (const Vec3<T> &v, Vec3<U> &w)
{
    MATH_EXC_ON;
    Vec3<T> w2 (w);
    return v * w2;
}

template <class T>
static Vec3<T>
Vec3_mulT (const Vec3<T> &v, T t)
{
    MATH_EXC_ON;
    return v * t;
}

template <class T>
static FixedArray<IMATH_NAMESPACE::Vec3<T> >
Vec3_mulTArray (const Vec3<T> &v, const FixedArray<T> &t)
{
    MATH_EXC_ON;
    size_t len = t.len();
    FixedArray<IMATH_NAMESPACE::Vec3<T> > retval(len);
    for (size_t i=0; i<len; ++i) retval[i] = v*t[i];
    return retval;
}

template <class T>
static FixedArray<IMATH_NAMESPACE::Vec3<T> >
Vec3_rmulTArray (const Vec3<T> &v, const FixedArray<T> &t)
{
    return Vec3_mulTArray(v,t);
}

template <class T,class S>
static Vec3<T>
Vec3_div (Vec3<T> &v, Vec3<S> &w)
{
    MATH_EXC_ON;
    return v / w;
}

template <class T>
static Vec3<T>
Vec3_rmulT (Vec3<T> &v, T t)
{
    MATH_EXC_ON;
    return t * v;
}

template <class T, class U>
static const Vec3<T> &
Vec3_imulV(Vec3<T> &v, const Vec3<U> &w)
{
    MATH_EXC_ON;
    return v *= w;
}

template <class T>
static const Vec3<T> &
Vec3_imulT(IMATH_NAMESPACE::Vec3<T> &v, T t) 
{ 
    MATH_EXC_ON;
    return v *= t;
}

template <class T, class U>
static Vec3<T>
Vec3_mulM33 (Vec3<T> &v, const Matrix33<U> &m)
{
    MATH_EXC_ON;
    return v * m;
}

template <class T, class U>
static Vec3<T>
Vec3_mulM44 (Vec3<T> &v, const Matrix44<U> &m)
{
    MATH_EXC_ON;
    return v * m;
}

template <class T>
static const Vec3<T> &
Vec3_idivObj(IMATH_NAMESPACE::Vec3<T> &v, const object &o) 
{ 
    MATH_EXC_ON;
    Vec3<T> v2;
    if (PyImath::V3<T>::convert (o.ptr(), &v2))
    {
        return v /= v2;
    }
    else
    {
        extract<double> e(o);
        if (e.check())
            return v /= e();
        else
            THROW (IEX_NAMESPACE::ArgExc, "V3 division expects an argument"
                   "convertible to a V3");
    }
}

template <class T>
static Vec3<T>
Vec3_subT(const Vec3<T> &v, T a)
{
    MATH_EXC_ON;
    Vec3<T> w;
    w.setValue(v.x - a, v.y - a, v.z - a);
    return w;
}

template <class T,class BoostPyType>
static Vec3<T>
Vec3_subTuple(const Vec3<T> &v, const BoostPyType &t)
{
    MATH_EXC_ON;
    Vec3<T> w;
    
    if(t.attr("__len__")() == 3)
    {
        w.x = v.x - extract<T>(t[0]);
        w.y = v.y - extract<T>(t[1]);
        w.z = v.z - extract<T>(t[2]);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 3");
    
    return w;
}

template <class T>
static Vec3<T>
Vec3_rsubT(const Vec3<T> &v, T a)
{
    MATH_EXC_ON;
    Vec3<T> w;
    w.setValue(a - v.x, a - v.y, a - v.z);
    return w;
}

template <class T, class BoostPyType>
static Vec3<T>
Vec3_rsubTuple(const Vec3<T> &v, const BoostPyType &t)
{
    MATH_EXC_ON;
    Vec3<T> w;
    
    if(t.attr("__len__")() == 3)
    {
        w.x = extract<T>(t[0]) - v.x;
        w.y = extract<T>(t[1]) - v.y;
        w.z = extract<T>(t[2]) - v.z;
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 3");
    
    return w;
}

template <class T, class BoostPyType>
static Vec3<T>
Vec3_addTuple(const Vec3<T> &v, const BoostPyType &t)
{
    MATH_EXC_ON;
    Vec3<T> w;
    
    if(t.attr("__len__")() == 3)
    {
        w.x = v.x + extract<T>(t[0]);
        w.y = v.y + extract<T>(t[1]);
        w.z = v.z + extract<T>(t[2]);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 3");
    
    return w;
}

template <class T>
static Vec3<T>
Vec3_addT(const Vec3<T> &v, T a)
{
    MATH_EXC_ON;
    Vec3<T> w;
    w.setValue(v.x + a, v.y + a, v.z + a);
    return w;
}

template <class T, class U>
static Vec3<T>
Vec3_addV(const Vec3<T> &v, const Vec3<U> &w)
{
    MATH_EXC_ON;
    return v + w;
}

template <class T, class U>
static const Vec3<T> &
Vec3_iaddV(Vec3<T> &v, const Vec3<U> &w)
{
    MATH_EXC_ON;
    return v += w;
}

template <class T, class U>
static Vec3<T>
Vec3_subV(const Vec3<T> &v, const Vec3<U> &w)
{
    MATH_EXC_ON;
    return v - w;
}

template <class T, class U>
static const Vec3<T> &
Vec3_isubV(Vec3<T> &v, const Vec3<U> &w)
{
    MATH_EXC_ON;
    return v -= w;
}

template <class T>
static Vec3<T>
mult(const Vec3<T> &v, tuple t)
{
    MATH_EXC_ON;
    Vec3<T> w;
    
    if(t.attr("__len__")() == 1){
        w.x = v.x*extract<T>(t[0]);
        w.y = v.y*extract<T>(t[0]);
        w.z = v.z*extract<T>(t[0]);
    }        
    else if(t.attr("__len__")() == 3){
        w.x = v.x*extract<T>(t[0]);
        w.y = v.y*extract<T>(t[1]);
        w.z = v.z*extract<T>(t[2]);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 1 or 3");
    
    return w;
}

template <class T, class U>
static const Vec3<T> &
Vec3_imulM44 (Vec3<T> &v, const Matrix44<U> &m)
{
    MATH_EXC_ON;
    return v *= m;
}

template <class T, class BoostPyType>
static Vec3<T>
Vec3_divTuple(const Vec3<T> &v, const BoostPyType &t)
{
    if(t.attr("__len__")() == 3)
    {
        T x = extract<T>(t[0]);
        T y = extract<T>(t[1]);
        T z = extract<T>(t[2]);
        if(x != T(0) && y != T(0) && z != T(0))
            return Vec3<T>(v.x / x, v.y / y, v.z / z);
        else
            THROW(IEX_NAMESPACE::MathExc, "Division by zero");
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Vec3 expects tuple of length 3");
}

template <class T, class BoostPyType>
static Vec3<T>
Vec3_rdivTuple(const Vec3<T> &v, const BoostPyType &t)
{
    MATH_EXC_ON;
    Vec3<T> w;
    if(t.attr("__len__")() == 3)
    {
        T x = extract<T>(t[0]);
        T y = extract<T>(t[1]);
        T z = extract<T>(t[2]);
            
        if(v.x != T(0) && v.y != T(0) && v.z != T(0)){
            w.setValue(x / v.x, y / v.y, z / v.z);
        }
        else
            THROW(IEX_NAMESPACE::MathExc, "Division by zero");
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 3");
    
    return w;
}

template <class T>
static Vec3<T>
Vec3_divT(const Vec3<T> &v, T a)
{
    MATH_EXC_ON;
    Vec3<T> w;
    if(a != T(0)){
        w.setValue(v.x / a, v.y / a, v.z / a);
    }
    else
        THROW(IEX_NAMESPACE::MathExc, "Division by zero");

    return w;
}

template <class T>
static Vec3<T>
Vec3_rdivT(const Vec3<T> &v, T a)
{
    MATH_EXC_ON;
    Vec3<T> w;
    if(v.x != T(0) && v.y != T(0) && v.z != T(0)){
        w.setValue(a / v.x, a / v.y, a / v.z);
    }
    else
        THROW(IEX_NAMESPACE::MathExc, "Division by zero");

    return w;
}

template <class T>
static Vec3<T>
Vec3_Vec3_mulT(const Vec3<T>& v, const Vec3<T>& w)
{
    MATH_EXC_ON;
    return v*w;
}

template <class T>
static Vec3<T>
Vec3_Vec3_divT(const Vec3<T>& v, const Vec3<T>& w)
{
    MATH_EXC_ON;
    return v/w;
}

template <class T>
static bool
lessThan(const Vec3<T> &v, const object &obj)
{
    extract<Vec3<T> > e1(obj);
    extract<tuple> e2(obj);
    
    Vec3<T> w;
    if(e1.check())
    {
        w = e1();
    }
    else if(e2.check())
    {
        tuple t = e2();
        T x = extract<T>(t[0]);
        T y = extract<T>(t[1]);
        T z = extract<T>(t[2]);
        w.setValue(x,y,z);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to operator <");
    
    bool isLessThan = (v.x <= w.x && v.y <= w.y && v.z <= w.z)
                    && v != w;
    
    return isLessThan;
}

template <class T>
static bool
greaterThan(const Vec3<T> &v, const object &obj)
{
    extract<Vec3<T> > e1(obj);
    extract<tuple> e2(obj);
    
    Vec3<T> w;
    if(e1.check())
    {
        w = e1();
    }
    else if(e2.check())
    {
        tuple t = e2();
        T x = extract<T>(t[0]);
        T y = extract<T>(t[1]);
        T z = extract<T>(t[2]);
        w.setValue(x,y,z);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to operator >");
    
    bool isGreaterThan = (v.x >= w.x && v.y >= w.y && v.z >= w.z)
                       && v != w;

    return isGreaterThan;
}

template <class T>
static bool
lessThanEqual(const Vec3<T> &v, const object &obj)
{
    extract<Vec3<T> > e1(obj);
    extract<tuple> e2(obj);
    
    Vec3<T> w;
    if(e1.check())
    {
        w = e1();
    }
    else if(e2.check())
    {
        tuple t = e2();
        T x = extract<T>(t[0]);
        T y = extract<T>(t[1]);
        T z = extract<T>(t[2]);
        w.setValue(x,y,z);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to operator <=");
    
    bool isLessThanEqual = (v.x <= w.x && v.y <= w.y && v.z <= w.z);
                   
    return isLessThanEqual;
}

template <class T>
static bool
greaterThanEqual(const Vec3<T> &v, const object &obj)
{
    extract<Vec3<T> > e1(obj);
    extract<tuple> e2(obj);
    
    Vec3<T> w;
    if(e1.check())
    {
        w = e1();
    }
    else if(e2.check())
    {
        tuple t = e2();
        T x = extract<T>(t[0]);
        T y = extract<T>(t[1]);
        T z = extract<T>(t[2]);
        w.setValue(x,y,z);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to operator >=");
    
    bool isGreaterThanEqual = (v.x >= w.x && v.y >= w.y && v.z >= w.z);

    return isGreaterThanEqual;
}


template <class T>
static bool
equalWithAbsErrorObj(const Vec3<T> &v, const object &obj1, const object &obj2)
{    
    extract<Vec3<int> >    e1(obj1);
    extract<Vec3<float> >  e2(obj1);
    extract<Vec3<double> > e3(obj1);
    
    extract<tuple>         e4(obj1);
    extract<double>        e5(obj2);
    
    Vec3<T> w;
    if(e1.check())      { w = e1(); }
    else if(e2.check()) { w = e2(); }
    else if(e3.check()) { w = e3(); }
    else if(e4.check())
    {    
        tuple t = e4();
        if(t.attr("__len__")() == 3)
        {
            w.x = extract<T>(t[0]);
            w.y = extract<T>(t[1]);
            w.z = extract<T>(t[2]);
        }
        else
            THROW(IEX_NAMESPACE::LogicExc, "tuple of length 3 expected");
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to equalWithAbsError");
    
    if(e5.check())      { return v.equalWithAbsError(w, e5()); }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to equalWithAbsError");
}

template <class T>
static bool
equalWithRelErrorObj(const Vec3<T> &v, const object &obj1, const object &obj2)
{    
    extract<Vec3<int> >    e1(obj1);
    extract<Vec3<float> >  e2(obj1);
    extract<Vec3<double> > e3(obj1);
    
    extract<tuple>         e4(obj1);    
    extract<double>        e5(obj2);
    
    Vec3<T> w;
    if(e1.check())      { w = e1(); }
    else if(e2.check()) { w = e2(); }
    else if(e3.check()) { w = e3(); }
    else if(e4.check())
    {    
        tuple t = e4();
        if(t.attr("__len__")() == 3)
        {
            w.x = extract<T>(t[0]);
            w.y = extract<T>(t[1]);
            w.z = extract<T>(t[2]);
        }
        else
            THROW(IEX_NAMESPACE::LogicExc, "tuple of length 3 expected");
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to equalWithRelError");
    
    if(e5.check())      { return v.equalWithRelError(w, e5()); }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to equalWithRelError");    
    
}


template <class T>
static bool
equal(const Vec3<T> &v, const tuple &t)
{
    Vec3<T> w;
    if(t.attr("__len__")() == 3)
    {
        w.x = extract<T>(t[0]);
        w.y = extract<T>(t[1]);
        w.z = extract<T>(t[2]);
        
        return (v == w);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple of length 3 expected");    
}

template <class T>
static bool
notequal(const Vec3<T> &v, const tuple &t)
{
    Vec3<T> w;
    if(t.attr("__len__")() == 3)
    {
        w.x = extract<T>(t[0]);
        w.y = extract<T>(t[1]);
        w.z = extract<T>(t[2]);
        
        return (v != w);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple of length 3 expected");    
}

template <class T>
class_<Vec3<T> >
register_Vec3()
{
    typedef PyImath::StaticFixedArray<Vec3<T>,T,3> Vec3_helper;

    class_<Vec3<T> > vec3_class(Vec3Name<T>::value(), Vec3Name<T>::value(),init<Vec3<T> >("copy construction"));
    vec3_class
        .def("__init__",make_constructor(Vec3_construct_default<T>),"initialize to (0,0,0)")
        .def("__init__",make_constructor(Vec3_object_constructor1<T>))
        .def("__init__",make_constructor(Vec3_object_constructor2<T>))
        .def_readwrite("x", &Vec3<T>::x)
        .def_readwrite("y", &Vec3<T>::y)
        .def_readwrite("z", &Vec3<T>::z)
	.def("baseTypeEpsilon", &Vec3<T>::baseTypeEpsilon,"baseTypeEpsilon() epsilon value of the base type of the vector")
        .staticmethod("baseTypeEpsilon")
	.def("baseTypeMax", &Vec3<T>::baseTypeMax,"baseTypeMax() max value of the base type of the vector")
        .staticmethod("baseTypeMax")
	.def("baseTypeMin", &Vec3<T>::baseTypeMin,"baseTypeMin() min value of the base type of the vector")
        .staticmethod("baseTypeMin")
	.def("baseTypeSmallest", &Vec3<T>::baseTypeSmallest,"baseTypeSmallest() smallest value of the base type of the vector")
        .staticmethod("baseTypeSmallest")
	.def("cross", &Vec3_cross<T>,"v1.cross(v2) right handed cross product")
	.def("cross", &Vec3_cross_Vec3Array<T>,"v1.cross(v2) right handed array cross product")
	.def("dimensions", &Vec3<T>::dimensions,"dimensions() number of dimensions in the vector")
        .staticmethod("dimensions")
	.def("dot", &Vec3_dot<T>,"v1.dot(v2) inner product of the two vectors")
	.def("dot", &Vec3_dot_Vec3Array<T>,"v1.dot(v2) array inner product")
    
	.def("equalWithAbsError", &Vec3<T>::equalWithAbsError,
         "v1.equalWithAbsError(v2) true if the elements "
         "of v1 and v2 are the same with an absolute error of no more than e, "
         "i.e., abs(v1[i] - v2[i]) <= e")
        .def("equalWithAbsError", &equalWithAbsErrorObj<T>)
             
	.def("equalWithRelError", &Vec3<T>::equalWithRelError,
         "v1.equalWithAbsError(v2) true if the elements "
         "of v1 and v2 are the same with an absolute error of no more than e, "
         "i.e., abs(v1[i] - v2[i]) <= e * abs(v1[i])")
        .def("equalWithRelError", &equalWithRelErrorObj<T>)
         
	.def("length", &Vec3_length<T>,"length() magnitude of the vector")
	.def("length2", &Vec3_length2<T>,"length2() square magnitude of the vector")
	.def("normalize", &Vec3_normalize<T>,return_internal_reference<>(),
         "v.normalize() destructively normalizes v and returns a reference to it")
	.def("normalizeExc", &Vec3_normalizeExc<T>,return_internal_reference<>(),
         "v.normalizeExc() destructively normalizes V and returns a reference to it, throwing an exception if length() == 0")
	.def("normalizeNonNull", &Vec3_normalizeNonNull<T>,return_internal_reference<>(),
         "v.normalizeNonNull() destructively normalizes V and returns a reference to it, faster if lngth() != 0")
	.def("normalized", &Vec3_normalized<T>, "v.normalized() returns a normalized copy of v")
	.def("normalizedExc", &Vec3_normalizedExc<T>, "v.normalizedExc() returns a normalized copy of v, throwing an exception if length() == 0")
	.def("normalizedNonNull", &Vec3_normalizedNonNull<T>, "v.normalizedNonNull() returns a normalized copy of v, faster if lngth() != 0")
	.def("__len__", Vec3_helper::len)
	.def("__getitem__", Vec3_helper::getitem,return_value_policy<copy_non_const_reference>())
	.def("__setitem__", Vec3_helper::setitem)
        .def("closestVertex", &closestVertex<T>)
        .def("negate", &Vec3_negate<T>, return_internal_reference<>())
        .def("orthogonal", &orthogonal<T>)
        .def("project", &project<T>)
        .def("reflect", &reflect<T>)
        .def("setValue", &setValue<T>)    
        .def("__neg__", &Vec3_neg<T>)
        .def("__mul__", &Vec3_mul<T, int>)
        .def("__mul__", &Vec3_mul<T, float>)
        .def("__mul__", &Vec3_mul<T, double>)
        .def("__mul__", &Vec3_mulT<T>)
        .def("__mul__", &Vec3_mulTArray<T>)
        .def("__rmul__", &Vec3_rmulT<T>)
        .def("__rmul__", &Vec3_rmulTArray<T>)
        .def("__imul__", &Vec3_imulV<T, int>,return_internal_reference<>())
        .def("__imul__", &Vec3_imulV<T, float>,return_internal_reference<>())
        .def("__imul__", &Vec3_imulV<T, double>,return_internal_reference<>())
        .def("__imul__", &Vec3_imulT<T>,return_internal_reference<>())
        .def("__div__", &Vec3_Vec3_divT<T>)
        .def("__truediv__", &Vec3_Vec3_divT<T>)
        .def("__mul__", &Vec3_mulM33<T, float>)
        .def("__mul__", &Vec3_mulM33<T, double>)
        .def("__mul__", &Vec3_mulM44<T, float>)
        .def("__mul__", &Vec3_mulM44<T, double>)
        .def("__mul__", &Vec3_Vec3_mulT<T>)
        .def("__div__", &Vec3_div<T,int>)
        .def("__div__", &Vec3_div<T,float>)
        .def("__div__", &Vec3_div<T,double>)
        .def("__div__", &Vec3_divTuple<T,tuple>)
        .def("__div__", &Vec3_divTuple<T,list>)
        .def("__div__", &Vec3_divT<T>)
        .def("__truediv__", &Vec3_div<T,int>)
        .def("__truediv__", &Vec3_div<T,float>)
        .def("__truediv__", &Vec3_div<T,double>)
        .def("__truediv__", &Vec3_divTuple<T,tuple>)
        .def("__truediv__", &Vec3_divTuple<T,list>)
        .def("__truediv__", &Vec3_divT<T>)
        .def("__rdiv__", &Vec3_rdivTuple<T,tuple>)
        .def("__rdiv__", &Vec3_rdivTuple<T,list>)
        .def("__rdiv__", &Vec3_rdivT<T>)
        .def("__idiv__", &Vec3_idivObj<T>,return_internal_reference<>())
        .def("__itruediv__", &Vec3_idivObj<T>,return_internal_reference<>())
        .def("__xor__", &Vec3_dot<T>)
        .def("__mod__", &Vec3_cross<T>)
        .def(self == self)
        .def(self != self)
        .def("__add__", &Vec3_add<T>)
        .def("__add__", &Vec3_addV<T, int>)
        .def("__add__", &Vec3_addV<T, float>)
        .def("__add__", &Vec3_addV<T, double>)
        .def("__add__", &Vec3_addT<T>)
        .def("__add__", &Vec3_addTuple<T,tuple>)
        .def("__add__", &Vec3_addTuple<T,list>)
        .def("__radd__", &Vec3_addT<T>)
        .def("__radd__", &Vec3_addTuple<T,tuple>)
        .def("__radd__", &Vec3_addTuple<T,list>)
        .def("__radd__", &Vec3_add<T>)
        .def("__iadd__", &Vec3_iaddV<T, int>, return_internal_reference<>())
        .def("__iadd__", &Vec3_iaddV<T, float>, return_internal_reference<>())
        .def("__iadd__", &Vec3_iaddV<T, double>, return_internal_reference<>())
        .def("__sub__", &Vec3_sub<T>)
        .def("__sub__", &Vec3_subV<T, int>)
        .def("__sub__", &Vec3_subV<T, float>)
        .def("__sub__", &Vec3_subV<T, double>)
        .def("__sub__", &Vec3_subT<T>)
        .def("__sub__", &Vec3_subTuple<T,tuple>)
        .def("__sub__", &Vec3_subTuple<T,list>)
        .def("__rsub__", &Vec3_rsubT<T>)
        .def("__rsub__", &Vec3_rsubTuple<T,tuple>)
        .def("__rsub__", &Vec3_rsubTuple<T,list>)
        .def("__isub__", &Vec3_isubV<T, int>, return_internal_reference<>())
        .def("__isub__", &Vec3_isubV<T, float>, return_internal_reference<>())
        .def("__isub__", &Vec3_isubV<T, double>, return_internal_reference<>())
        .def("__mul__", &mult<T>)
        .def("__rmul__", &mult<T>)
        .def("__imul__", &Vec3_imulM44<T, float>, return_internal_reference<>())
        .def("__imul__", &Vec3_imulM44<T, double>, return_internal_reference<>())
        .def("__lt__", &lessThan<T>)
        .def("__gt__", &greaterThan<T>)
        .def("__le__", &lessThanEqual<T>)
        .def("__ge__", &greaterThanEqual<T>)
        .def("__eq__", &equal<T>)
        .def("__ne__", &notequal<T>)
	//.def(self_ns::str(self))
	.def("__str__",&Vec3_str<T>)
	.def("__repr__",&Vec3_repr<T>)
        ;

    decoratecopy(vec3_class);

    //add_swizzle3_operators(v3f_class);
    return vec3_class;
}



}  // namespace PyImath

#endif    // _PyImathVec3Impl_h_
