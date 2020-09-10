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

template <class T> struct Vec4Name      { static const char *value(); };

// create a new default constructor that initializes Vec3<T> to zero.
template <class T>
static Vec4<T> * Vec4_construct_default()
{
    return new Vec4<T>(T(0),T(0),T(0),T(0));
}

template <class T>
static Vec4<T> * Vec4_object_constructor1(const object &obj)
{
    Vec4<T> res;
    extract<Vec4<int> >     e1(obj);
    extract<Vec4<float> >   e2(obj);
    extract<Vec4<double> >  e3(obj);
    extract<tuple>          e4(obj);
    extract<double>         e5(obj);
    extract<list>           e6(obj);
    
    if(e1.check())      { res = e1(); }
    else if(e2.check()) { res = e2(); }
    else if(e3.check()) { res = e3(); }
    else if(e4.check())
    {
        tuple t = e4();
        if(t.attr("__len__")() == 4)
        {
            res.x = extract<T>(t[0]);
            res.y = extract<T>(t[1]);
            res.z = extract<T>(t[2]);
            res.w = extract<T>(t[3]);
        }
        else
            THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 4");
        
    }
    else if(e5.check()) { T a = (T) e5(); res = IMATH_NAMESPACE::Vec4<T>(a, a, a, a); }
    else if(e6.check())
    {
        list l = e6();
        if(l.attr("__len__")() == 4)
        {
            res.x = extract<T>(l[0]);
            res.y = extract<T>(l[1]);
            res.z = extract<T>(l[2]);
            res.w = extract<T>(l[3]);
        }
        else
            THROW(IEX_NAMESPACE::LogicExc, "list must have length of 4");
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to Vec4 constructor");
    
    Vec4<T> *v = new Vec4<T>;
    *v = res;
    
    return v;

}

template <class T>
static Vec4<T> * Vec4_object_constructor2(const object &obj1, const object &obj2, const object &obj3, const object& obj4)
{
    extract<double>    e1(obj1);
    extract<double>    e2(obj2);
    extract<double>    e3(obj3);
    extract<double>    e4(obj4);
    Vec4<T> *v = new Vec4<T>;
    
    if(e1.check()) { v->x = (T) e1();}
    else { THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to Vec4 constructor"); }
    
    if(e2.check()) { v->y = (T) e2();}
    else { THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to Vec4 constructor"); }    

    if(e3.check()) { v->z = (T) e3();}
    else { THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to Vec4 constructor"); } 

    if(e4.check()) { v->w = (T) e4();}
    else { THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to Vec4 constructor"); } 
    
    return v;
}



// Implementations of str and repr are same here,
// but we'll specialize repr for float and double to make them exact.
template <class T>
static std::string Vec4_str(const Vec4<T> &v)
{
    std::stringstream stream;
    stream << Vec4Name<T>::value() << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    return stream.str();
}
template <class T>
static std::string Vec4_repr(const Vec4<T> &v)
{
    std::stringstream stream;
    stream << Vec4Name<T>::value() << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    return stream.str();
}

template <class T>
static T
Vec4_dot(const IMATH_NAMESPACE::Vec4<T> &v, const IMATH_NAMESPACE::Vec4<T> &other) 
{ 
    MATH_EXC_ON;
    return v.dot(other);
}

template <class T>
static FixedArray<T>
Vec4_dot_Vec4Array(const IMATH_NAMESPACE::Vec4<T> &va, const FixedArray<IMATH_NAMESPACE::Vec4<T> > &vb) 
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
Vec4_length(const IMATH_NAMESPACE::Vec4<T> &v) 
{ 
    MATH_EXC_ON;
    return v.length();
}

template <class T>
static T
Vec4_length2(const IMATH_NAMESPACE::Vec4<T> &v) 
{ 
    MATH_EXC_ON;
    return v.length2();
}

template <class T>
static const Vec4<T> &
Vec4_normalize(IMATH_NAMESPACE::Vec4<T> &v) 
{ 
    MATH_EXC_ON;
    return v.normalize();
}

template <class T>
static const Vec4<T> &
Vec4_normalizeExc(IMATH_NAMESPACE::Vec4<T> &v) 
{ 
    MATH_EXC_ON;
    return v.normalizeExc();
}

template <class T>
static const Vec4<T> &
Vec4_normalizeNonNull(IMATH_NAMESPACE::Vec4<T> &v) 
{ 
    MATH_EXC_ON;
    return v.normalizeNonNull();
}

template <class T>
static Vec4<T>
Vec4_normalized(const IMATH_NAMESPACE::Vec4<T> &v) 
{ 
    MATH_EXC_ON;
    return v.normalized();
}

template <class T>
static Vec4<T>
Vec4_normalizedExc(const IMATH_NAMESPACE::Vec4<T> &v) 
{ 
    MATH_EXC_ON;
    return v.normalizedExc();
}

template <class T>
static Vec4<T>
Vec4_normalizedNonNull(const IMATH_NAMESPACE::Vec4<T> &v) 
{ 
    MATH_EXC_ON;
    return v.normalizedNonNull();
}

template <class T>
static const Vec4<T> &
Vec4_negate(IMATH_NAMESPACE::Vec4<T> &v) 
{ 
    MATH_EXC_ON;
    return v.negate();
}

template <class T>
static Vec4<T>
orthogonal(const Vec4<T> &v, const Vec4<T> &v0)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::orthogonal(v, v0);
}

template <class T>
static Vec4<T>
project(const Vec4<T> &v, const Vec4<T> &v0)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::project(v0, v);
}

template <class T>
static Vec4<T>
reflect(const Vec4<T> &v, const Vec4<T> &v0)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::reflect(v, v0);
}

template <class T>
static void
setValue(Vec4<T> &v, T a, T b, T c, T d)
{
    v.x = a;
    v.y = b;
    v.z = c;
    v.w = d;
}

template <class T>
static Vec4<T>
Vec4_add (const Vec4<T> &v, const Vec4<T> &w)
{
    MATH_EXC_ON;
    return v + w;
}

template <class T>
static Vec4<T>
Vec4_sub (const Vec4<T> &v, const Vec4<T> &w)
{
    MATH_EXC_ON;
    return v - w;
}

template <class T>
static Vec4<T>
Vec4_neg (const Vec4<T> &v)
{
    MATH_EXC_ON;
    return -v;
}

template <class T, class U>
static Vec4<T>
Vec4_mul (const Vec4<T> &v, Vec4<U> &w)
{
    MATH_EXC_ON;
    Vec4<T> w2 (w);
    return v * w2;
}

template <class T>
static Vec4<T>
Vec4_mulT (const Vec4<T> &v, T t)
{
    MATH_EXC_ON;
    return v * t;
}

template <class T>
static FixedArray<IMATH_NAMESPACE::Vec4<T> >
Vec4_mulTArray (const Vec4<T> &v, const FixedArray<T> &t)
{
    PY_IMATH_LEAVE_PYTHON;
    size_t len = t.len();
    FixedArray<IMATH_NAMESPACE::Vec4<T> > retval(len);
    for (size_t i=0; i<len; ++i) retval[i] = v*t[i];
    return retval;
}

template <class T>
static FixedArray<IMATH_NAMESPACE::Vec4<T> >
Vec4_rmulTArray (const Vec4<T> &v, const FixedArray<T> &t)
{
    return Vec4_mulTArray(v,t);
}

template <class T,class S>
static Vec4<T>
Vec4_div (Vec4<T> &v, Vec4<S> &w)
{
    MATH_EXC_ON;
    return v / w;
}

template <class T>
static Vec4<T>
Vec4_rmulT (Vec4<T> &v, T t)
{
    MATH_EXC_ON;
    return t * v;
}

template <class T, class U>
static const Vec4<T> &
Vec4_imulV(Vec4<T> &v, const Vec4<U> &w)
{
    MATH_EXC_ON;
    return v *= w;
}

template <class T>
static const Vec4<T> &
Vec4_imulT(IMATH_NAMESPACE::Vec4<T> &v, T t) 
{ 
    MATH_EXC_ON;
    return v *= t;
}

template <class T, class U>
static Vec4<T>
Vec4_mulM44 (Vec4<T> &v, const Matrix44<U> &m)
{
    MATH_EXC_ON;
    return v * m;
}

template <class T>
static const Vec4<T> &
Vec4_idivObj(IMATH_NAMESPACE::Vec4<T> &v, const object &o) 
{ 
    MATH_EXC_ON;
    Vec4<T> v2;
    if (PyImath::V4<T>::convert (o.ptr(), &v2))
    {
        return v /= v2;
    }
    else
    {
        extract<double> e(o);
        if (e.check())
            return v /= (T) e();
        else
            THROW (IEX_NAMESPACE::ArgExc, "V4 division expects an argument "
                   "convertible to a V4");
    }
}

template <class T>
static Vec4<T>
Vec4_subT(const Vec4<T> &v, T a)
{
    MATH_EXC_ON;
    Vec4<T> w;
    setValue(w, (T) (v.x - a), (T) (v.y - a), (T) (v.z - a), (T) (v.w - a));
    return w;
}

template <class T,class BoostPyType>
static Vec4<T>
Vec4_subTuple(const Vec4<T> &v, const BoostPyType &t)
{
    MATH_EXC_ON;
    Vec4<T> w;
    
    if(t.attr("__len__")() == 4)
    {
        w.x = v.x - extract<T>(t[0]);
        w.y = v.y - extract<T>(t[1]);
        w.z = v.z - extract<T>(t[2]);
        w.w = v.w - extract<T>(t[3]);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 4");
    
    return w;
}

template <class T>
static Vec4<T>
Vec4_rsubT(const Vec4<T> &v, T a)
{
    MATH_EXC_ON;
    Vec4<T> w(a - v.x, a - v.y, a - v.z, a - v.w);
    return w;
}

template <class T, class BoostPyType>
static Vec4<T>
Vec4_rsubTuple(const Vec4<T> &v, const BoostPyType &t)
{
    MATH_EXC_ON;
    Vec4<T> w;
    
    if(t.attr("__len__")() == 4)
    {
        w.x = extract<T>(t[0]) - v.x;
        w.y = extract<T>(t[1]) - v.y;
        w.z = extract<T>(t[2]) - v.z;
        w.w = extract<T>(t[3]) - v.w;
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 4");
    
    return w;
}

template <class T, class BoostPyType>
static Vec4<T>
Vec4_addTuple(const Vec4<T> &v, const BoostPyType &t)
{
    MATH_EXC_ON;
    Vec4<T> w;
    
    if(t.attr("__len__")() == 4)
    {
        w.x = v.x + extract<T>(t[0]);
        w.y = v.y + extract<T>(t[1]);
        w.z = v.z + extract<T>(t[2]);
        w.w = v.w + extract<T>(t[3]);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 4");
    
    return w;
}

template <class T>
static Vec4<T>
Vec4_addT(const Vec4<T> &v, T a)
{
    MATH_EXC_ON;
    Vec4<T> w;
    setValue(w, (T) (v.x + a), (T) (v.y + a), (T) (v.z + a), (T) (v.w + a));
    return w;
}

template <class T, class U>
static Vec4<T>
Vec4_addV(const Vec4<T> &v, const Vec4<U> &w)
{
    MATH_EXC_ON;
    return v + w;
}

template <class T, class U>
static const Vec4<T> &
Vec4_iaddV(Vec4<T> &v, const Vec4<U> &w)
{
    MATH_EXC_ON;
    return v += w;
}

template <class T, class U>
static Vec4<T>
Vec4_subV(const Vec4<T> &v, const Vec4<U> &w)
{
    MATH_EXC_ON;
    return v - w;
}

template <class T, class U>
static const Vec4<T> &
Vec4_isubV(Vec4<T> &v, const Vec4<U> &w)
{
    MATH_EXC_ON;
    return v -= w;
}

template <class T>
static Vec4<T>
mult(const Vec4<T> &v, tuple t)
{
    MATH_EXC_ON;
    Vec4<T> w;
    
    if(t.attr("__len__")() == 1){
        w.x = v.x*extract<T>(t[0]);
        w.y = v.y*extract<T>(t[0]);
        w.z = v.z*extract<T>(t[0]);
        w.w = v.w*extract<T>(t[0]);
    }        
    else if(t.attr("__len__")() == 4){
        w.x = v.x*extract<T>(t[0]);
        w.y = v.y*extract<T>(t[1]);
        w.z = v.z*extract<T>(t[2]);
        w.w = v.w*extract<T>(t[3]);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 1 or 4");
    
    return w;
}

template <class T, class U>
static const Vec4<T> &
Vec4_imulM44 (Vec4<T> &v, const Matrix44<U> &m)
{
    MATH_EXC_ON;
    return v *= m;
}

template <class T, class BoostPyType>
static Vec4<T>
Vec4_divTuple(const Vec4<T> &v, const BoostPyType &t)
{
    if(t.attr("__len__")() == 4)
    {
        T x = extract<T>(t[0]);
        T y = extract<T>(t[1]);
        T z = extract<T>(t[2]);
        T w = extract<T>(t[3]);
        if(x != T(0) && y != T(0) && z != T(0) && w != T(0))
            return Vec4<T>(v.x / x, v.y / y, v.z / z, v.w / w);
        else
            THROW(IEX_NAMESPACE::MathExc, "Division by zero");
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Vec4 expects tuple of length 4");
}

template <class T, class BoostPyType>
static Vec4<T>
Vec4_rdivTuple(const Vec4<T> &v, const BoostPyType &t)
{
    MATH_EXC_ON;
    Vec4<T> res;
    if(t.attr("__len__")() == 4)
    {
        T x = extract<T>(t[0]);
        T y = extract<T>(t[1]);
        T z = extract<T>(t[2]);
        T w = extract<T>(t[3]);
            
        if(v.x != T(0) && v.y != T(0) && v.z != T(0) && v.w != T(0)){
            setValue(res, (T) (x / v.x), (T) (y / v.y), (T) (z / v.z), (T) (w / v.w));
        }
        else
            THROW(IEX_NAMESPACE::MathExc, "Division by zero");
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 4");
    
    return res;
}

template <class T>
static Vec4<T>
Vec4_divT(const Vec4<T> &v, T a)
{
    MATH_EXC_ON;
    Vec4<T> res;
    if(a != T(0)){
        setValue(res, (T) (v.x / a), (T) (v.y / a), (T) (v.z / a), (T) (v.w / a));
    }
    else
        THROW(IEX_NAMESPACE::MathExc, "Division by zero");

    return res;
}

template <class T>
static Vec4<T>
Vec4_rdivT(const Vec4<T> &v, T a)
{
    MATH_EXC_ON;
    Vec4<T> res;
    if(v.x != T(0) && v.y != T(0) && v.z != T(0) && v.w != T(0)){
        setValue(res, (T) (a / v.x), (T) (a / v.y), (T) (a / v.z), (T) (a / v.w));
    }
    else
        THROW(IEX_NAMESPACE::MathExc, "Division by zero");

    return res;
}

template <class T>
static Vec4<T>
Vec4_Vec4_mulT(const Vec4<T>& v, const Vec4<T>& w)
{
    MATH_EXC_ON;
    return v*w;
}

template <class T>
static Vec4<T>
Vec4_Vec4_divT(const Vec4<T>& v, const Vec4<T>& w)
{
    MATH_EXC_ON;
    return v/w;
}

template <class T>
static bool
lessThan(const Vec4<T> &v, const object &obj)
{
    extract<Vec4<T> > e1(obj);
    extract<tuple> e2(obj);
    
    Vec4<T> res;
    if(e1.check())
    {
        res = e1();
    }
    else if(e2.check())
    {
        tuple t = e2();
        T x = extract<T>(t[0]);
        T y = extract<T>(t[1]);
        T z = extract<T>(t[2]);
        T w = extract<T>(t[3]);
        setValue(res,x,y,z,w);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to operator <");
    
    bool isLessThan = (v.x <= res.x && v.y <= res.y && v.z <= res.z && v.w <= res.w)
                    && v != res;
    
    return isLessThan;
}

template <class T>
static bool
greaterThan(const Vec4<T> &v, const object &obj)
{
    extract<Vec4<T> > e1(obj);
    extract<tuple> e2(obj);
    
    Vec4<T> res;
    if(e1.check())
    {
        res = e1();
    }
    else if(e2.check())
    {
        tuple t = e2();
        T x = extract<T>(t[0]);
        T y = extract<T>(t[1]);
        T z = extract<T>(t[2]);
        T w = extract<T>(t[3]);
        setValue(res,x,y,z,w);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to operator >");
    
    bool isGreaterThan = (v.x >= res.x && v.y >= res.y && v.z >= res.z && v.w >= res.w)
                       && v != res;

    return isGreaterThan;
}

template <class T>
static bool
lessThanEqual(const Vec4<T> &v, const object &obj)
{
    extract<Vec4<T> > e1(obj);
    extract<tuple> e2(obj);
    
    Vec4<T> res;
    if(e1.check())
    {
        res = e1();
    }
    else if(e2.check())
    {
        tuple t = e2();
        T x = extract<T>(t[0]);
        T y = extract<T>(t[1]);
        T z = extract<T>(t[2]);
        T w = extract<T>(t[2]);
        setValue(res,x,y,z,w);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to operator <=");
    
    bool isLessThanEqual = (v.x <= res.x && v.y <= res.y && v.z <= res.z && v.w <= res.w);
                   
    return isLessThanEqual;
}

template <class T>
static bool
greaterThanEqual(const Vec4<T> &v, const object &obj)
{
    extract<Vec4<T> > e1(obj);
    extract<tuple> e2(obj);
    
    Vec4<T> res;
    if(e1.check())
    {
        res = e1();
    }
    else if(e2.check())
    {
        tuple t = e2();
        T x = extract<T>(t[0]);
        T y = extract<T>(t[1]);
        T z = extract<T>(t[2]);
        T w = extract<T>(t[3]);
        setValue(res,x,y,z,w);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to operator >=");
    
    bool isGreaterThanEqual = (v.x >= res.x && v.y >= res.y && v.z >= res.z && v.w >= res.w);

    return isGreaterThanEqual;
}


template <class T>
static bool
equalWithAbsErrorObj(const Vec4<T> &v, const object &obj1, const object &obj2)
{    
    extract<Vec4<int> >    e1(obj1);
    extract<Vec4<float> >  e2(obj1);
    extract<Vec4<double> > e3(obj1);
    
    extract<tuple>         e4(obj1);
    extract<double>        e5(obj2);
    
    Vec4<T> res;
    if(e1.check())      { res = e1(); }
    else if(e2.check()) { res = e2(); }
    else if(e3.check()) { res = e3(); }
    else if(e4.check())
    {    
        tuple t = e4();
        if(t.attr("__len__")() == 4)
        {
            res.x = extract<T>(t[0]);
            res.y = extract<T>(t[1]);
            res.z = extract<T>(t[2]);
            res.z = extract<T>(t[3]);
        }
        else
            THROW(IEX_NAMESPACE::LogicExc, "tuple of length 4 expected");
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to equalWithAbsError");
    
    if(e5.check())      { return v.equalWithAbsError(res, (T) e5()); }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to equalWithAbsError");
}

template <class T>
static bool
equalWithRelErrorObj(const Vec4<T> &v, const object &obj1, const object &obj2)
{    
    extract<Vec4<int> >    e1(obj1);
    extract<Vec4<float> >  e2(obj1);
    extract<Vec4<double> > e3(obj1);
    
    extract<tuple>         e4(obj1);    
    extract<double>        e5(obj2);
    
    Vec4<T> res;
    if(e1.check())      { res = e1(); }
    else if(e2.check()) { res = e2(); }
    else if(e3.check()) { res = e3(); }
    else if(e4.check())
    {    
        tuple t = e4();
        if(t.attr("__len__")() == 4)
        {
            res.x = extract<T>(t[0]);
            res.y = extract<T>(t[1]);
            res.z = extract<T>(t[2]);
            res.w = extract<T>(t[3]);
        }
        else
            THROW(IEX_NAMESPACE::LogicExc, "tuple of length 4 expected");
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to equalWithRelError");
    
    if(e5.check())      { return v.equalWithRelError(res, (T) e5()); }
    else
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameters passed to equalWithRelError");    
    
}


template <class T>
static bool
equal(const Vec4<T> &v, const tuple &t)
{
    Vec4<T> res;
    if(t.attr("__len__")() == 4)
    {
        res.x = extract<T>(t[0]);
        res.y = extract<T>(t[1]);
        res.z = extract<T>(t[2]);
        res.w = extract<T>(t[3]);
        
        return (v == res);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple of length 4 expected");    
}

template <class T>
static bool
notequal(const Vec4<T> &v, const tuple &t)
{
    Vec4<T> res;
    if(t.attr("__len__")() == 4)
    {
        res.x = extract<T>(t[0]);
        res.y = extract<T>(t[1]);
        res.z = extract<T>(t[2]);
        res.w = extract<T>(t[3]);
        
        return (v != res);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple of length 4 expected");    
}

template <class T>
class_<Vec4<T> >
register_Vec4()
{
    typedef PyImath::StaticFixedArray<Vec4<T>,T,4> Vec4_helper;
    class_<Vec4<T> > vec4_class(Vec4Name<T>::value(), Vec4Name<T>::value(),init<Vec4<T> >("copy construction"));
    vec4_class
        .def("__init__",make_constructor(Vec4_construct_default<T>),"initialize to (0,0,0,0)")
        .def("__init__",make_constructor(Vec4_object_constructor1<T>))
        .def("__init__",make_constructor(Vec4_object_constructor2<T>))
        .def_readwrite("x", &Vec4<T>::x)
        .def_readwrite("y", &Vec4<T>::y)
        .def_readwrite("z", &Vec4<T>::z)
        .def_readwrite("w", &Vec4<T>::w)
	.def("baseTypeEpsilon", &Vec4<T>::baseTypeEpsilon,"baseTypeEpsilon() epsilon value of the base type of the vector")
        .staticmethod("baseTypeEpsilon")
	.def("baseTypeMax", &Vec4<T>::baseTypeMax,"baseTypeMax() max value of the base type of the vector")
        .staticmethod("baseTypeMax")
	.def("baseTypeMin", &Vec4<T>::baseTypeMin,"baseTypeMin() min value of the base type of the vector")
        .staticmethod("baseTypeMin")
	.def("baseTypeSmallest", &Vec4<T>::baseTypeSmallest,"baseTypeSmallest() smallest value of the base type of the vector")
        .staticmethod("baseTypeSmallest")
	.def("dimensions", &Vec4<T>::dimensions,"dimensions() number of dimensions in the vector")
        .staticmethod("dimensions")
	.def("dot", &Vec4_dot<T>,"v1.dot(v2) inner product of the two vectors")
	.def("dot", &Vec4_dot_Vec4Array<T>,"v1.dot(v2) array inner product")
    
	.def("equalWithAbsError", &Vec4<T>::equalWithAbsError,
         "v1.equalWithAbsError(v2) true if the elements "
         "of v1 and v2 are the same with an absolute error of no more than e, "
         "i.e., abs(v1[i] - v2[i]) <= e")
        .def("equalWithAbsError", &equalWithAbsErrorObj<T>)
             
	.def("equalWithRelError", &Vec4<T>::equalWithRelError,
         "v1.equalWithAbsError(v2) true if the elements "
         "of v1 and v2 are the same with an absolute error of no more than e, "
         "i.e., abs(v1[i] - v2[i]) <= e * abs(v1[i])")
        .def("equalWithRelError", &equalWithRelErrorObj<T>)
         
	.def("length", &Vec4_length<T>,"length() magnitude of the vector")
	.def("length2", &Vec4_length2<T>,"length2() square magnitude of the vector")
	.def("normalize", &Vec4_normalize<T>,return_internal_reference<>(),
         "v.normalize() destructively normalizes v and returns a reference to it")
	.def("normalizeExc", &Vec4_normalizeExc<T>,return_internal_reference<>(),
         "v.normalizeExc() destructively normalizes V and returns a reference to it, throwing an exception if length() == 0")
	.def("normalizeNonNull", &Vec4_normalizeNonNull<T>,return_internal_reference<>(),
         "v.normalizeNonNull() destructively normalizes V and returns a reference to it, faster if lngth() != 0")
	.def("normalized", &Vec4_normalized<T>, "v.normalized() returns a normalized copy of v")
	.def("normalizedExc", &Vec4_normalizedExc<T>, "v.normalizedExc() returns a normalized copy of v, throwing an exception if length() == 0")
	.def("normalizedNonNull", &Vec4_normalizedNonNull<T>, "v.normalizedNonNull() returns a normalized copy of v, faster if lngth() != 0")
	.def("__len__", Vec4_helper::len)
	.def("__getitem__", Vec4_helper::getitem,return_value_policy<copy_non_const_reference>())
	.def("__setitem__", Vec4_helper::setitem)
        .def("negate", &Vec4_negate<T>, return_internal_reference<>())
        .def("orthogonal", &orthogonal<T>)
        .def("project", &project<T>)
        .def("reflect", &reflect<T>)
        .def("setValue", &setValue<T>)    
        .def("__neg__", &Vec4_neg<T>)
        .def("__mul__", &Vec4_mul<T, int>)
        .def("__mul__", &Vec4_mul<T, float>)
        .def("__mul__", &Vec4_mul<T, double>)
        .def("__mul__", &Vec4_mulT<T>)
        .def("__mul__", &Vec4_mulTArray<T>)
        .def("__rmul__", &Vec4_rmulT<T>)
        .def("__rmul__", &Vec4_rmulTArray<T>)
        .def("__imul__", &Vec4_imulV<T, int>,return_internal_reference<>())
        .def("__imul__", &Vec4_imulV<T, float>,return_internal_reference<>())
        .def("__imul__", &Vec4_imulV<T, double>,return_internal_reference<>())
        .def("__imul__", &Vec4_imulT<T>,return_internal_reference<>())
        .def("__div__", &Vec4_Vec4_divT<T>)
        .def("__truediv__", &Vec4_Vec4_divT<T>)
        .def("__mul__", &Vec4_mulM44<T, float>)
        .def("__mul__", &Vec4_mulM44<T, double>)
        .def("__mul__", &Vec4_Vec4_mulT<T>)
        .def("__div__", &Vec4_div<T,int>)
        .def("__div__", &Vec4_div<T,float>)
        .def("__div__", &Vec4_div<T,double>)
        .def("__div__", &Vec4_divTuple<T,tuple>)
        .def("__div__", &Vec4_divTuple<T,list>)
        .def("__div__", &Vec4_divT<T>)
        .def("__truediv__", &Vec4_div<T,int>)
        .def("__truediv__", &Vec4_div<T,float>)
        .def("__truediv__", &Vec4_div<T,double>)
        .def("__truediv__", &Vec4_divTuple<T,tuple>)
        .def("__truediv__", &Vec4_divTuple<T,list>)
        .def("__truediv__", &Vec4_divT<T>)
        .def("__rdiv__", &Vec4_rdivTuple<T,tuple>)
        .def("__rdiv__", &Vec4_rdivTuple<T,list>)
        .def("__rdiv__", &Vec4_rdivT<T>)
        .def("__idiv__", &Vec4_idivObj<T>,return_internal_reference<>())
        .def("__itruediv__", &Vec4_idivObj<T>,return_internal_reference<>())
        .def("__xor__", &Vec4_dot<T>)
        .def(self == self)
        .def(self != self)
        .def("__add__", &Vec4_add<T>)
        .def("__add__", &Vec4_addV<T, int>)
        .def("__add__", &Vec4_addV<T, float>)
        .def("__add__", &Vec4_addV<T, double>)
        .def("__add__", &Vec4_addT<T>)
        .def("__add__", &Vec4_addTuple<T,tuple>)
        .def("__add__", &Vec4_addTuple<T,list>)
        .def("__radd__", &Vec4_addT<T>)
        .def("__radd__", &Vec4_addTuple<T,tuple>)
        .def("__radd__", &Vec4_addTuple<T,list>)
        .def("__radd__", &Vec4_add<T>)
        .def("__iadd__", &Vec4_iaddV<T, int>, return_internal_reference<>())
        .def("__iadd__", &Vec4_iaddV<T, float>, return_internal_reference<>())
        .def("__iadd__", &Vec4_iaddV<T, double>, return_internal_reference<>())
        .def("__sub__", &Vec4_sub<T>)
        .def("__sub__", &Vec4_subV<T, int>)
        .def("__sub__", &Vec4_subV<T, float>)
        .def("__sub__", &Vec4_subV<T, double>)
        .def("__sub__", &Vec4_subT<T>)
        .def("__sub__", &Vec4_subTuple<T,tuple>)
        .def("__sub__", &Vec4_subTuple<T,list>)
        .def("__rsub__", &Vec4_rsubT<T>)
        .def("__rsub__", &Vec4_rsubTuple<T,tuple>)
        .def("__rsub__", &Vec4_rsubTuple<T,list>)
        .def("__isub__", &Vec4_isubV<T, int>, return_internal_reference<>())
        .def("__isub__", &Vec4_isubV<T, float>, return_internal_reference<>())
        .def("__isub__", &Vec4_isubV<T, double>, return_internal_reference<>())
        .def("__mul__", &mult<T>)
        .def("__rmul__", &mult<T>)
        .def("__imul__", &Vec4_imulM44<T, float>, return_internal_reference<>())
        .def("__imul__", &Vec4_imulM44<T, double>, return_internal_reference<>())
        .def("__lt__", &lessThan<T>)
        .def("__gt__", &greaterThan<T>)
        .def("__le__", &lessThanEqual<T>)
        .def("__ge__", &greaterThanEqual<T>)
        .def("__eq__", &equal<T>)
        .def("__ne__", &notequal<T>)
	//.def(self_ns::str(self))
	.def("__str__",&Vec4_str<T>)
	.def("__repr__",&Vec4_repr<T>)
        ;

    decoratecopy(vec4_class);

    //add_swizzle3_operators(v3f_class);
    return vec4_class;
}



}  // namespace PyImath

#endif    // _PyImathVec4Impl_h_
