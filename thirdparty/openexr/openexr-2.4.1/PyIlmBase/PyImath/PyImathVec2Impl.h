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

#ifndef _PyImathVec2Impl_h_
#define _PyImathVec2Impl_h_

//
// This .C file was turned into a header file so that instantiations
// of the various V2* types can be spread across multiple files in
// order to work around MSVC limitations.
//

#include "PyImathVec.h"
#include "PyImathDecorators.h"
#include <Python.h>
#include <boost/python.hpp>
#include <boost/python/make_constructor.hpp>
#include <boost/format.hpp>
#include "PyImath.h"
#include "PyImathBox.h"
#include <ImathVec.h>
#include <ImathVecAlgo.h>
#include <Iex.h>
#include "PyImathMathExc.h"
#include <boost/cast.hpp>
#include "PyImathOperators.h"
#include "PyImathVecOperators.h"

namespace PyImath {
using namespace boost::python;
using namespace IMATH_NAMESPACE;

template <class T> struct Vec2Name { static const char *value; };

// create a new default constructor that initializes Vec2<T> to zero.
template <class T>
static Vec2<T> * Vec2_construct_default()
{
    return new Vec2<T>(T(0),T(0));
}

template <class T,class BoostPyType>
static Vec2<T> * Vec2_tuple_constructor(const BoostPyType &t)
{
    if(t.attr("__len__")() == 1)
        return new Vec2<T>(extract<T>(t[0]), extract<T>(t[0]));
    else if(t.attr("__len__")() == 2)
        return new Vec2<T>(extract<T>(t[0]), extract<T>(t[1]));
    else
        THROW(IEX_NAMESPACE::LogicExc, "Vec2 constructor expects tuple of length 1 or 2");
}

template <class T>
static Vec2<T> * Vec2_object_constructor1(const object &obj)
{
    Vec2<T> w;
    extract<Vec2<int> >     e1(obj);
    extract<Vec2<float> >   e2(obj);
    extract<Vec2<double> >  e3(obj);
    extract<tuple>          e4(obj);
    extract<double>         e5(obj);
    extract<list>           e6(obj);
    
    if(e1.check()){ w = e1(); }
    else if(e2.check()) { w = e2(); }
    else if(e3.check()) { w = e3(); }
    else if(e4.check())
    {
        tuple t = e4();
        if(t.attr("__len__")() == 2)
        {
            w.x = extract<T>(t[0]);
            w.y = extract<T>(t[1]);
        }
        else
            THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 2");
    }
    else if(e5.check()) { T a = e5(); w.setValue(a, a); }
    else if(e6.check())
    {
        list l = e6();
        if(l.attr("__len__")() == 2)
        {
            w.x = extract<T>(l[0]);
            w.y = extract<T>(l[1]);
        }
        else
            THROW(IEX_NAMESPACE::LogicExc, "list must have length of 2");
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to Vec2 constructor");
    
    Vec2<T> *v = new Vec2<T>;
    *v = w;
    
    return v;

}

template <class T>
static Vec2<T> * Vec2_object_constructor2(const object &obj1, const object &obj2)
{
    extract<double> e1(obj1);
    extract<double> e2(obj2);
    
    Vec2<T> *v = new Vec2<T>;
    
    if(e1.check()) { v->x = boost::numeric_cast<T>(e1());}
    else { THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to Vec2 constructor"); }
    
    if(e2.check()) { v->y = boost::numeric_cast<T>(e2());}
    else { THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to Vec2 constructor"); }    
    
    return v;
}

// Implementations of str and repr are same here,
// but we'll specialize repr for float and double to make them exact.
template <class T>
static std::string Vec2_str(const Vec2<T> &v)
{
    std::stringstream stream;
    stream << Vec2Name<T>::value << "(" << v.x << ", " << v.y << ")";
    return stream.str();
}
template <class T>
static std::string Vec2_repr(const Vec2<T> &v)
{
    std::stringstream stream;
    stream << Vec2Name<T>::value << "(" << v.x << ", " << v.y << ")";
    return stream.str();
}

template <class T>
static T
Vec2_cross(const IMATH_NAMESPACE::Vec2<T> &v, const IMATH_NAMESPACE::Vec2<T> &other) 
{ 
    MATH_EXC_ON;
    return v.cross(other);
}

template <class T>
static FixedArray<T>
Vec2_cross_Vec2Array(const IMATH_NAMESPACE::Vec2<T> &va, const FixedArray<IMATH_NAMESPACE::Vec2<T> > &vb) 
{ 
    PY_IMATH_LEAVE_PYTHON;
    size_t len = vb.len(); 
    FixedArray<T> f(len); 
    for (size_t i = 0; i < len; ++i) 
        f[i] = va.cross(vb[i]); 
    return f; 
}

template <class T>
static T
Vec2_dot(const IMATH_NAMESPACE::Vec2<T> &v, const IMATH_NAMESPACE::Vec2<T> &other) 
{ 
    MATH_EXC_ON;
    return v.dot(other);
}

template <class T>
static FixedArray<T>
Vec2_dot_Vec2Array(const IMATH_NAMESPACE::Vec2<T> &va, const FixedArray<IMATH_NAMESPACE::Vec2<T> > &vb) 
{ 
    PY_IMATH_LEAVE_PYTHON;
    size_t len = vb.len(); 
    FixedArray<T> f(len); 
    for (size_t i = 0; i < len; ++i) 
        f[i] = va.dot(vb[i]); 
    return f; 
}

template <class T>
static T
Vec2_length(const IMATH_NAMESPACE::Vec2<T> &v) 
{ 
    MATH_EXC_ON;
    return v.length();
}

template <class T>
static T
Vec2_length2(const IMATH_NAMESPACE::Vec2<T> &v) 
{ 
    MATH_EXC_ON;
    return v.length2();
}

template <class T>
static const Vec2<T> &
Vec2_normalize(IMATH_NAMESPACE::Vec2<T> &v) 
{ 
    MATH_EXC_ON;
    return v.normalize();
}

template <class T>
static const Vec2<T> &
Vec2_normalizeExc(IMATH_NAMESPACE::Vec2<T> &v) 
{ 
    MATH_EXC_ON;
    return v.normalizeExc();
}

template <class T>
static const Vec2<T> &
Vec2_normalizeNonNull(IMATH_NAMESPACE::Vec2<T> &v) 
{ 
    MATH_EXC_ON;
    return v.normalizeNonNull();
}

template <class T>
static Vec2<T>
Vec2_normalized(const IMATH_NAMESPACE::Vec2<T> &v) 
{ 
    MATH_EXC_ON;
    return v.normalized();
}

template <class T>
static Vec2<T>
Vec2_normalizedExc(const IMATH_NAMESPACE::Vec2<T> &v) 
{ 
    MATH_EXC_ON;
    return v.normalizedExc();
}

template <class T>
static Vec2<T>
Vec2_normalizedNonNull(const IMATH_NAMESPACE::Vec2<T> &v) 
{ 
    MATH_EXC_ON;
    return v.normalizedNonNull();
}

template <class T>
static Vec2<T>
closestVertex(Vec2<T> &p, const Vec2<T> &v0, const Vec2<T> &v1, const Vec2<T> &v2)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::closestVertex(v0, v1, v2, p);
}

template <class T>
static const Vec2<T> &
Vec2_negate(IMATH_NAMESPACE::Vec2<T> &v) 
{ 
    MATH_EXC_ON;
    return v.negate();
}

template <class T>
static Vec2<T>
orthogonal(const Vec2<T> &v, const Vec2<T> &v0)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::orthogonal(v, v0);
}

template <class T>
static Vec2<T>
project(const Vec2<T> &v, const Vec2<T> &v0)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::project(v0, v);
}

template <class T>
static Vec2<T>
reflect(const Vec2<T> &v, const Vec2<T> &v0)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::reflect(v, v0);
}

template <class T>
static void
setValue(Vec2<T> &v, T a, T b)
{
    v.x = a;
    v.y = b;
}

template <class T>
static Vec2<T>
Vec2_add (const Vec2<T> &v, const Vec2<T> &w)
{
    MATH_EXC_ON;
    return v + w;
}

template <class T>
static Vec2<T>
Vec2_sub (const Vec2<T> &v, const Vec2<T> &w)
{
    MATH_EXC_ON;
    return v - w;
}

template <class T>
static Vec2<T>
Vec2_neg (const Vec2<T> &v)
{
    MATH_EXC_ON;
    return -v;
}

template <class T, class U>
static Vec2<T>
Vec2_mul (const Vec2<T> &v, const Vec2<U> &w)
{
    MATH_EXC_ON;
    Vec2<T> w2 (w);
    return v * w2;
}

template <class T>
static Vec2<T>
Vec2_mulT (const Vec2<T> &v, T t)
{
    MATH_EXC_ON;
    return v * t;
}

template <class T>
static FixedArray<IMATH_NAMESPACE::Vec2<T> >
Vec2_mulTArray (const Vec2<T> &v, const FixedArray<T> &t)
{
    PY_IMATH_LEAVE_PYTHON;
    size_t len = t.len();
    FixedArray<IMATH_NAMESPACE::Vec2<T> > retval(len);
    for (size_t i=0; i<len; ++i) retval[i] = v*t[i];
    return retval;
}

template <class T>
static FixedArray<IMATH_NAMESPACE::Vec2<T> >
Vec2_rmulTArray (const Vec2<T> &v, const FixedArray<T> &t)
{
    return Vec2_mulTArray(v,t);
}

template <class T,class S>
static Vec2<T>
Vec2_div (Vec2<T> &v, Vec2<S> &w)
{
    MATH_EXC_ON;
    return v / w;
}

template <class T>
static Vec2<T>
Vec2_rmulT (Vec2<T> &v, T t)
{
    MATH_EXC_ON;
    return t * v;
}

template <class T, class U>
static const Vec2<T> &
Vec2_imulV(Vec2<T> &v, const Vec2<U> &w)
{
    MATH_EXC_ON;
    return v *= w;
}

template <class T>
static const Vec2<T> &
Vec2_imulT(IMATH_NAMESPACE::Vec2<T> &v, T t) 
{ 
    MATH_EXC_ON;
    return v *= t;
}

template <class T, class U>
static Vec2<T>
Vec2_mulM33 (Vec2<T> &v, const Matrix33<U> &m)
{
    MATH_EXC_ON;
    return v * m;
}

template <class T>
static const Vec2<T> &
Vec2_idivObj(IMATH_NAMESPACE::Vec2<T> &v, const object &o) 
{ 
    MATH_EXC_ON;
    Vec2<T> v2;
    if (PyImath::V2<T>::convert (o.ptr(), &v2))
    {
        return v /= v2;
    }
    else
    {
        extract<double> e(o);
        if (e.check())
            return v /= e();
        else
            THROW (IEX_NAMESPACE::ArgExc, "V2 division expects an argument"
                   "convertible to a V2");
    }
}

template <class T>
static Vec2<T>
Vec2_subT(const Vec2<T> &v, T a)
{
    MATH_EXC_ON;
    Vec2<T> w;
    w.setValue(v.x - a, v.y - a);
    return w;
}

template <class T,class BoostPyType>
static Vec2<T>
Vec2_subTuple(const Vec2<T> &v, const BoostPyType &t)
{
    MATH_EXC_ON;
    Vec2<T> w;
    
    if(t.attr("__len__")() == 2)
    {
        w.x = v.x - extract<T>(t[0]);
        w.y = v.y - extract<T>(t[1]);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 2");
    
    return w;
}

template <class T>
static Vec2<T>
Vec2_rsubT(const Vec2<T> &v, T a)
{
    MATH_EXC_ON;
    Vec2<T> w;
    w.setValue(a - v.x, a - v.y);
    return w;
}

template <class T,class BoostPyType>
static Vec2<T>
Vec2_rsubTuple(const Vec2<T> &v, const BoostPyType &t)
{
    MATH_EXC_ON;
    Vec2<T> w;
    
    if(t.attr("__len__")() == 2)
    {
        w.x = extract<T>(t[0]) - v.x;
        w.y = extract<T>(t[1]) - v.y;
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 2");
    
    return w;
}

template <class T,class BoostPyType>
static Vec2<T>
Vec2_addTuple(const Vec2<T> &v, const BoostPyType &t)
{
    MATH_EXC_ON;
    Vec2<T> w;
    
    if(t.attr("__len__")() == 2)
    {
        w.x = v.x + extract<T>(t[0]);
        w.y = v.y + extract<T>(t[1]);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 2");
    
    return w;
}

template <class T>
static Vec2<T>
Vec2_addT(const Vec2<T> &v, T a)
{
    MATH_EXC_ON;
    Vec2<T> w;
    w.setValue(v.x + a, v.y + a);
    return w;
}

template <class T, class U>
static Vec2<T>
Vec2_addV(const Vec2<T> &v, const Vec2<U> &w)
{
    MATH_EXC_ON;
    return v + w;
}

template <class T, class U>
static const Vec2<T> &
Vec2_iaddV(Vec2<T> &v, const Vec2<U> &w)
{
    MATH_EXC_ON;
    return v += w;
}

template <class T, class U>
static Vec2<T>
Vec2_subV(const Vec2<T> &v, const Vec2<U> &w)
{
    MATH_EXC_ON;
    return v - w;
}

template <class T, class U>
static const Vec2<T> &
Vec2_isubV(Vec2<T> &v, const Vec2<U> &w)
{
    MATH_EXC_ON;
    return v -= w;
}

template <class T,class BoostPyType>
static Vec2<T>
Vec2_mulTuple(const Vec2<T> &v, BoostPyType t)
{
    MATH_EXC_ON;
    Vec2<T> w;
    
    if(t.attr("__len__")() == 1){
        w.x = v.x*extract<T>(t[0]);
        w.y = v.y*extract<T>(t[0]);
    }        
    else if(t.attr("__len__")() == 2){
        w.x = v.x*extract<T>(t[0]);
        w.y = v.y*extract<T>(t[1]);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 1 or 2");
    
    return w;
}

template <class T, class U>
static const Vec2<T> &
Vec2_imulM33 (Vec2<T> &v, const Matrix33<U> &m)
{
    MATH_EXC_ON;
    return v *= m;
}

template <class T,class BoostPyType>
static Vec2<T>
Vec2_divTuple(const Vec2<T> &v, const BoostPyType &t)
{
    if(t.attr("__len__")() == 2)
    {
        T x = extract<T>(t[0]);
        T y = extract<T>(t[1]);
        if(x != T(0) && y != T(0))
            return Vec2<T>(v.x / x, v.y / y);
        else
            THROW(IEX_NAMESPACE::MathExc, "Division by zero");
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Vec2 expects tuple of length 2");
}

template <class T,class BoostPyType>
static Vec2<T>
Vec2_rdivTuple(const Vec2<T> &v, const BoostPyType &t)
{
    MATH_EXC_ON;
    Vec2<T> w;
    if(t.attr("__len__")() == 2)
    {
        T x = extract<T>(t[0]);
        T y = extract<T>(t[1]);
            
        if(v.x != T(0) && v.y != T(0)){
            w.setValue(x / v.x, y / v.y);
        }
        else
            THROW(IEX_NAMESPACE::MathExc, "Division by zero");
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 2");
    
    return w;
}

template <class T>
static Vec2<T>
Vec2_divT(const Vec2<T> &v, T a)
{
    MATH_EXC_ON;
    Vec2<T> w;
    if(a != T(0)){
        w.setValue(v.x / a, v.y / a);
    }
    else
        THROW(IEX_NAMESPACE::MathExc, "Division by zero");

    return w;
}

template <class T>
static Vec2<T>
Vec2_rdivT(const Vec2<T> &v, T a)
{
    MATH_EXC_ON;
    Vec2<T> w;
    if(v.x != T(0) && v.y != T(0)){
        w.setValue(a / v.x, a / v.y);
    }
    else
        THROW(IEX_NAMESPACE::MathExc, "Division by zero");

    return w;
}

template <class T>
static bool
lessThan(const Vec2<T> &v, const object &obj)
{
    extract<Vec2<T> > e1(obj);
    extract<tuple> e2(obj);
    
    Vec2<T> w;
    if(e1.check())
    {
        w = e1();
    }
    else if(e2.check())
    {
        tuple t = e2();
        if(t.attr("__len__")() == 2){
            T x = extract<T>(t[0]);
            T y = extract<T>(t[1]);
            w.setValue(x,y);
        }
        else
           THROW(IEX_NAMESPACE::LogicExc, "Vec2 expects tuple of length 2");
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to operator <");
    
    bool isLessThan = (v.x <= w.x && v.y <= w.y)
                    && v != w;
    
    return isLessThan;
}

template <class T>
static bool
greaterThan(const Vec2<T> &v, const object &obj)
{
    extract<Vec2<T> > e1(obj);
    extract<tuple> e2(obj);
    
    Vec2<T> w;
    if(e1.check())
    {
        w = e1();
    }
    else if(e2.check())
    {
        tuple t = e2();
        if(t.attr("__len__")() == 2){
            T x = extract<T>(t[0]);
            T y = extract<T>(t[1]);
            w.setValue(x,y);
        }
        else
           THROW(IEX_NAMESPACE::LogicExc, "Vec2 expects tuple of length 2");
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to operator >");
    
    bool isGreaterThan = (v.x >= w.x && v.y >= w.y)
                       && v != w;

    return isGreaterThan;
}

template <class T>
static bool
lessThanEqual(const Vec2<T> &v, const object &obj)
{
    extract<Vec2<T> > e1(obj);
    extract<tuple> e2(obj);
    
    Vec2<T> w;
    if(e1.check())
    {
        w = e1();
    }
    else if(e2.check())
    {
        tuple t = e2();
        if(t.attr("__len__")() == 2){
            T x = extract<T>(t[0]);
            T y = extract<T>(t[1]);
            w.setValue(x,y);
        }
        else
           THROW(IEX_NAMESPACE::LogicExc, "Vec2 expects tuple of length 2");
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to operator <=");
    
    bool isLessThanEqual = (v.x <= w.x && v.y <= w.y);
                   
    return isLessThanEqual;
}

template <class T>
static bool
greaterThanEqual(const Vec2<T> &v, const object &obj)
{
    extract<Vec2<T> > e1(obj);
    extract<tuple> e2(obj);
    
    Vec2<T> w;
    if(e1.check())
    {
        w = e1();
    }
    else if(e2.check())
    {
        tuple t = e2();
        if(t.attr("__len__")() == 2){
            T x = extract<T>(t[0]);
            T y = extract<T>(t[1]);
            w.setValue(x,y);
        }
        else
           THROW(IEX_NAMESPACE::LogicExc, "Vec2 expects tuple of length 2"); 
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to operator >=");
    
    bool isGreaterThanEqual = (v.x >= w.x && v.y >= w.y);

    return isGreaterThanEqual;
}

template <class T,class BoostPyType>
static void
setItemTuple(FixedArray<IMATH_NAMESPACE::Vec2<T> > &va, Py_ssize_t index, const BoostPyType &t)
{
    if(t.attr("__len__")() == 2)
    {
        Vec2<T> v;
        v.x = extract<T>(t[0]);
        v.y = extract<T>(t[1]);
        va[va.canonical_index(index)] = v;
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple of length 2 expected");
}

template <class T>
static bool
equalWithAbsErrorObj(const Vec2<T> &v, const object &obj1, const object &obj2)
{    
    extract<Vec2<int> >    e1(obj1);
    extract<Vec2<float> >  e2(obj1);
    extract<Vec2<double> > e3(obj1);
    
    extract<tuple>         e4(obj1);
    extract<double>        e5(obj2);
    
    Vec2<T> w;
    if(e1.check())      { w = e1(); }
    else if(e2.check()) { w = e2(); }
    else if(e3.check()) { w = e3(); }
    else if(e4.check())
    {    
        tuple t = e4();
        if(t.attr("__len__")() == 2)
        {
            w.x = extract<T>(t[0]);
            w.y = extract<T>(t[1]);
        }
        else
            THROW(IEX_NAMESPACE::LogicExc, "tuple of length 2 expected");
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to equalWithAbsError");
    
    if(e5.check())      { return v.equalWithAbsError(w, e5()); }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to equalWithAbsError");
}

template <class T>
static bool
equalWithRelErrorObj(const Vec2<T> &v, const object &obj1, const object &obj2)
{    
    extract<Vec2<int> >    e1(obj1);
    extract<Vec2<float> >  e2(obj1);
    extract<Vec2<double> > e3(obj1);
    
    extract<tuple>         e4(obj1);
    extract<double>        e5(obj2);
    
    Vec2<T> w;
    if(e1.check())      { w = e1(); }
    else if(e2.check()) { w = e2(); }
    else if(e3.check()) { w = e3(); }
    else if(e4.check())
    {    
        tuple t = e4();
        if(t.attr("__len__")() == 2)
        {
            w.x = extract<T>(t[0]);
            w.y = extract<T>(t[1]);
        }
        else
            THROW(IEX_NAMESPACE::LogicExc, "tuple of length 2 expected");
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to equalWithRelError");
    
    if(e5.check())      { return v.equalWithRelError(w, e5()); }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to equalWithRelError");
    
}

/*
template <class T>
static bool
equalWithAbsErrorTuple(Vec2<T> &v, const tuple &t, T e)
{
    Vec2<T> w;
    if(t.attr("__len__")() == 2)
    {
        w.x = extract<T>(t[0]);
        w.y = extract<T>(t[1]);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple of length 2 expected");
    
    return v.equalWithAbsError(w, e);
}

template <class T>
static bool
equalWithRelErrorTuple(Vec2<T> &v, const tuple &t, T e)
{
    std::cout << "RelError Tuple called" << std::endl;
    Vec2<T> w;
    if(t.attr("__len__")() == 2)
    {
        w.x = extract<T>(t[0]);
        w.y = extract<T>(t[1]);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple of length 2 expected");
    
    return v.equalWithRelError(w, e);
}
*/
template <class T,class BoostPyType>
static bool
equal(const Vec2<T> &v, const BoostPyType &t)
{
    Vec2<T> w;
    if(t.attr("__len__")() == 2)
    {
        w.x = extract<T>(t[0]);
        w.y = extract<T>(t[1]);
        
        return (v == w);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple of length 2 expected");    
}

template <class T,class BoostPyType>
static bool
notequal(const Vec2<T> &v, const BoostPyType &t)
{
    Vec2<T> w;
    if(t.attr("__len__")() == 2)
    {
        w.x = extract<T>(t[0]);
        w.y = extract<T>(t[1]);
        
        return (v != w);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple of length 2 expected");    
}

template <class T>
class_<Vec2<T> >
register_Vec2()
{
    typedef PyImath::StaticFixedArray<Vec2<T>,T,2> Vec2_helper;

    class_<Vec2<T> > vec2_class(Vec2Name<T>::value, Vec2Name<T>::value,init<Vec2<T> >("copy construction"));
    vec2_class
        .def("__init__",make_constructor(Vec2_construct_default<T>),"initialize to (0,0)")
        .def("__init__",make_constructor(Vec2_object_constructor1<T>))
        .def("__init__",make_constructor(Vec2_object_constructor2<T>))
        .def_readwrite("x", &Vec2<T>::x)
        .def_readwrite("y", &Vec2<T>::y)
        .def("baseTypeEpsilon", &Vec2<T>::baseTypeEpsilon,"baseTypeEpsilon() epsilon value of the base type of the vector")
        .staticmethod("baseTypeEpsilon")
        .def("baseTypeMax", &Vec2<T>::baseTypeMax,"baseTypeMax() max value of the base type of the vector")
        .staticmethod("baseTypeMax")
        .def("baseTypeMin", &Vec2<T>::baseTypeMin,"baseTypeMin() min value of the base type of the vector")
        .staticmethod("baseTypeMin")
        .def("baseTypeSmallest", &Vec2<T>::baseTypeSmallest,"baseTypeSmallest() smallest value of the base type of the vector")
        .staticmethod("baseTypeSmallest")
        .def("cross", &Vec2_cross<T>,"v1.cross(v2) right handed cross product")
        .def("cross", &Vec2_cross_Vec2Array<T>,"v1.cross(v2) right handed array cross product")
        .def("dimensions", &Vec2<T>::dimensions,"dimensions() number of dimensions in the vector")
        .staticmethod("dimensions")
        .def("dot", &Vec2_dot<T>,"v1.dot(v2) inner product of the two vectors")
        .def("dot", &Vec2_dot_Vec2Array<T>,"v1.dot(v2) array inner product")
        .def("equalWithAbsError", &Vec2<T>::equalWithAbsError,
             "v1.equalWithAbsError(v2) true if the elements "
             "of v1 and v2 are the same with an absolute error of no more than e, "
             "i.e., abs(v1[i] - v2[i]) <= e")
        .def("equalWithAbsError", &equalWithAbsErrorObj<T>)

        .def("equalWithRelError", &Vec2<T>::equalWithRelError,
             "v1.equalWithAbsError(v2) true if the elements "
             "of v1 and v2 are the same with an absolute error of no more than e, "
             "i.e., abs(v1[i] - v2[i]) <= e * abs(v1[i])")
        .def("equalWithRelError", &equalWithRelErrorObj<T>)

        .def("length", &Vec2_length<T>,"length() magnitude of the vector")
        .def("length2", &Vec2_length2<T>,"length2() square magnitude of the vector")
        .def("normalize", &Vec2_normalize<T>,return_internal_reference<>(),
             "v.normalize() destructively normalizes v and returns a reference to it")
         
        .def("normalizeExc", &Vec2_normalizeExc<T>,return_internal_reference<>(),
             "v.normalizeExc() destructively normalizes V and returns a reference to it, throwing an exception if length() == 0")
         
        .def("normalizeNonNull", &Vec2_normalizeNonNull<T>,return_internal_reference<>(),
             "v.normalizeNonNull() destructively normalizes V and returns a reference to it, faster if lngth() != 0")
        .def("normalized", &Vec2_normalized<T>, "v.normalized() returns a normalized copy of v")
        .def("normalizedExc", &Vec2_normalizedExc<T>, "v.normalizedExc() returns a normalized copy of v, throwing an exception if length() == 0")
        .def("normalizedNonNull", &Vec2_normalizedNonNull<T>, "v.normalizedNonNull() returns a normalized copy of v, faster if lngth() != 0")
        .def("__len__", Vec2_helper::len)
        .def("__getitem__", Vec2_helper::getitem,return_value_policy<copy_non_const_reference>())
        .def("__setitem__", Vec2_helper::setitem)
        .def("closestVertex", &closestVertex<T>)
        .def("negate", &Vec2_negate<T>, return_internal_reference<>())
        .def("orthogonal", &orthogonal<T>)
        .def("project", &project<T>)
        .def("reflect", &reflect<T>)
        .def("setValue", &setValue<T>)
        .def("__neg__", &Vec2_neg<T>)
        .def("__mul__", &Vec2_mul<T, int>)
        .def("__mul__", &Vec2_mul<T, float>)
        .def("__mul__", &Vec2_mul<T, double>)
        .def("__mul__", &Vec2_mulT<T>)
        .def("__mul__", &Vec2_mulTArray<T>)
        .def("__mul__", &Vec2_mulTuple<T,tuple>)
        .def("__mul__", &Vec2_mulTuple<T,list>)
        .def("__rmul__", &Vec2_rmulT<T>)
        .def("__rmul__", &Vec2_rmulTArray<T>)
        .def("__rmul__", &Vec2_mulTuple<T,tuple>)
        .def("__rmul__", &Vec2_mulTuple<T,list>)
        .def("__imul__", &Vec2_imulV<T, int>,return_internal_reference<>())
        .def("__imul__", &Vec2_imulV<T, float>,return_internal_reference<>())
        .def("__imul__", &Vec2_imulV<T, double>,return_internal_reference<>())
        .def("__imul__", &Vec2_imulT<T>,return_internal_reference<>())
        .def(self * self)
        .def("__mul__", &Vec2_mulM33<T, float>)
        .def("__mul__", &Vec2_mulM33<T, double>)
        .def("__imul__", &Vec2_imulM33<T, float>, return_internal_reference<>())
        .def("__imul__", &Vec2_imulM33<T, double>, return_internal_reference<>())
        .def(self / self) // NOSONAR - suppress SonarCloud bug report.
        .def("__div__", &Vec2_div<T,int>)
        .def("__div__", &Vec2_div<T,float>)
        .def("__div__", &Vec2_div<T,double>)
        .def("__div__", &Vec2_divTuple<T,tuple>)
        .def("__div__", &Vec2_divTuple<T,list>)
        .def("__div__", &Vec2_divT<T>)
        .def("__truediv__", &Vec2_div<T,int>)
        .def("__truediv__", &Vec2_div<T,float>)
        .def("__truediv__", &Vec2_div<T,double>)
        .def("__truediv__", &Vec2_divTuple<T,tuple>)
        .def("__truediv__", &Vec2_divTuple<T,list>)
        .def("__truediv__", &Vec2_divT<T>)
        .def("__rdiv__", &Vec2_rdivTuple<T,tuple>)
        .def("__rdiv__", &Vec2_rdivTuple<T,list>)
        .def("__rdiv__", &Vec2_rdivT<T>)
        .def("__idiv__", &Vec2_idivObj<T>,return_internal_reference<>())
        .def("__itruediv__", &Vec2_idivObj<T>,return_internal_reference<>())
        .def("__xor__", &Vec2_dot<T>)
        .def("__mod__", &Vec2_cross<T>)
        .def(self == self) // NOSONAR - suppress SonarCloud bug report.
        .def(self != self) // NOSONAR - suppress SonarCloud bug report.
        .def("__eq__", &equal<T,tuple>)
        .def("__ne__", &notequal<T,tuple>)
        .def("__add__", &Vec2_add<T>)
        .def("__add__", &Vec2_addV<T, int>)
        .def("__add__", &Vec2_addV<T, float>)
        .def("__add__", &Vec2_addV<T, double>)
        .def("__add__", &Vec2_addT<T>)
        .def("__add__", &Vec2_addTuple<T,tuple>)
        .def("__add__", &Vec2_addTuple<T,list>)
        .def("__radd__", &Vec2_add<T>)
        .def("__radd__", &Vec2_addT<T>)
        .def("__radd__", &Vec2_addTuple<T,tuple>)
        .def("__radd__", &Vec2_addTuple<T,list>)
        .def("__iadd__", &Vec2_iaddV<T, int>, return_internal_reference<>())
        .def("__iadd__", &Vec2_iaddV<T, float>, return_internal_reference<>())
        .def("__iadd__", &Vec2_iaddV<T, double>, return_internal_reference<>())
        .def("__sub__", &Vec2_sub<T>)
        .def("__sub__", &Vec2_subV<T, int>)
        .def("__sub__", &Vec2_subV<T, float>)
        .def("__sub__", &Vec2_subV<T, double>)
        .def("__sub__", &Vec2_subT<T>)
        .def("__sub__", &Vec2_subTuple<T,tuple>)
        .def("__sub__", &Vec2_subTuple<T,list>)
        .def("__rsub__", &Vec2_rsubT<T>)
        .def("__rsub__", &Vec2_rsubTuple<T,tuple>)
        .def("__rsub__", &Vec2_rsubTuple<T,list>)
        .def("__isub__", &Vec2_isubV<T, int>, return_internal_reference<>())
        .def("__isub__", &Vec2_isubV<T, float>, return_internal_reference<>())
        .def("__isub__", &Vec2_isubV<T, double>, return_internal_reference<>())
        .def("__lt__", &lessThan<T>)
        .def("__gt__", &greaterThan<T>)
        .def("__le__", &lessThanEqual<T>)
        .def("__ge__", &greaterThanEqual<T>)
	//.def(self_ns::str(self))
	.def("__str__",&Vec2_str<T>)
	.def("__repr__",&Vec2_repr<T>)
        ;

    decoratecopy(vec2_class);

    //add_swizzle2_operators(v2f_class);
    return vec2_class;
}

// XXX fixme - template this
// really this should get generated automatically...

template <class T,int index>
static FixedArray<T>
Vec2Array_get(FixedArray<IMATH_NAMESPACE::Vec2<T> > &va)
{
    return FixedArray<T>(&va[0][index],va.len(),2*va.stride());
}

template <class T>
static IMATH_NAMESPACE::Vec2<T>
Vec2Array_min(const FixedArray<IMATH_NAMESPACE::Vec2<T> > &a)
{
    Vec2<T> tmp(Vec2<T>(0));
    size_t len = a.len();
    if (len > 0)
        tmp = a[0];
    for (size_t i=1; i < len; ++i)
    {
        if (a[i].x < tmp.x)
            tmp.x = a[i].x;
        if (a[i].y < tmp.y)
            tmp.y = a[i].y;
    }
    return tmp;
}

template <class T>
static IMATH_NAMESPACE::Vec2<T>
Vec2Array_max(const FixedArray<IMATH_NAMESPACE::Vec2<T> > &a)
{
    Vec2<T> tmp(Vec2<T>(0));
    size_t len = a.len();
    if (len > 0)
        tmp = a[0];
    for (size_t i=1; i < len; ++i)
    {
        if (a[i].x > tmp.x)
            tmp.x = a[i].x;
        if (a[i].y > tmp.y)
            tmp.y = a[i].y;
    }
    return tmp;
}

template <class T>
static IMATH_NAMESPACE::Box<IMATH_NAMESPACE::Vec2<T> >
Vec2Array_bounds(const FixedArray<IMATH_NAMESPACE::Vec2<T> > &a)
{
    Box<Vec2<T> > tmp;
    size_t len = a.len();
    for (size_t i=0; i < len; ++i)
        tmp.extendBy(a[i]);
    return tmp;
}

template <class T>
class_<FixedArray<IMATH_NAMESPACE::Vec2<T> > >
register_Vec2Array()
{
    using boost::mpl::true_;
    using boost::mpl::false_;

    class_<FixedArray<IMATH_NAMESPACE::Vec2<T> > > vec2Array_class = FixedArray<IMATH_NAMESPACE::Vec2<T> >::register_("Fixed length array of IMATH_NAMESPACE::Vec2");
    vec2Array_class
        .add_property("x",&Vec2Array_get<T,0>)
        .add_property("y",&Vec2Array_get<T,1>)
        .def("__setitem__", &setItemTuple<T,tuple>)
        .def("__setitem__", &setItemTuple<T,list>)
        .def("min", &Vec2Array_min<T>)
        .def("max", &Vec2Array_max<T>)
        .def("bounds", &Vec2Array_bounds<T>)
        ;

    add_arithmetic_math_functions(vec2Array_class);
    add_comparison_functions(vec2Array_class);

    generate_member_bindings<op_vecLength<IMATH_NAMESPACE::Vec2<T> >     >(vec2Array_class,"length","");
    generate_member_bindings<op_vecLength2<IMATH_NAMESPACE::Vec2<T> >    >(vec2Array_class,"length2","");
    generate_member_bindings<op_vecNormalize<IMATH_NAMESPACE::Vec2<T> >  >(vec2Array_class,"normalize","");
    generate_member_bindings<op_vecNormalized<IMATH_NAMESPACE::Vec2<T> > >(vec2Array_class,"normalized","");

    generate_member_bindings<op_vec2Cross<T>,           true_>(vec2Array_class,"cross","return the cross product of (self,x)",boost::python::args("x"));
    generate_member_bindings<op_vecDot<IMATH_NAMESPACE::Vec2<T> >,true_>(vec2Array_class,"dot","return the inner product of (self,x)",boost::python::args("x"));

    generate_member_bindings<op_mul<IMATH_NAMESPACE::Vec2<T>,T>,  true_>(vec2Array_class,"__mul__" ,"self*x", boost::python::args("x"));
    generate_member_bindings<op_mul<IMATH_NAMESPACE::Vec2<T>,T>,  true_>(vec2Array_class,"__rmul__","x*self", boost::python::args("x"));
    generate_member_bindings<op_imul<IMATH_NAMESPACE::Vec2<T>,T>, true_>(vec2Array_class,"__imul__","self*=x",boost::python::args("x"));
    generate_member_bindings<op_div<IMATH_NAMESPACE::Vec2<T>,T>,  true_>(vec2Array_class,"__div__" ,"self/x", boost::python::args("x"));
    generate_member_bindings<op_div<IMATH_NAMESPACE::Vec2<T>,T>,  true_>(vec2Array_class,"__truediv__" ,"self/x", boost::python::args("x"));
    generate_member_bindings<op_idiv<IMATH_NAMESPACE::Vec2<T>,T>, true_>(vec2Array_class,"__idiv__","self/=x",boost::python::args("x"));
    generate_member_bindings<op_idiv<IMATH_NAMESPACE::Vec2<T>,T>, true_>(vec2Array_class,"__itruediv__","self/=x",boost::python::args("x"));

    decoratecopy(vec2Array_class);

    return vec2Array_class;
}

}

#endif
