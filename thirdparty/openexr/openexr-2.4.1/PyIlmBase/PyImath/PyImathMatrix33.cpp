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

#define BOOST_PYTHON_MAX_ARITY 17

#include "PyImathMatrix.h"
#include "PyImathExport.h"
#include "PyImathDecorators.h"
#include <Python.h>
#include <boost/python.hpp>
#include <boost/python/make_constructor.hpp>
#include <boost/format.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/raw_function.hpp>
#include "PyImath.h"
#include "PyImathVec.h"
#include "PyImathMathExc.h"
#include <ImathVec.h>
#include <ImathMatrixAlgo.h>
#include <Iex.h>

namespace PyImath {

template<> const char *PyImath::M33fArray::name() { return "M33fArray"; }
template<> const char *PyImath::M33dArray::name() { return "M33dArray"; }

using namespace boost::python;
using namespace IMATH_NAMESPACE;

template <class T, int len>
struct MatrixRow {
    explicit MatrixRow(T *data) : _data(data) {}
    T & operator [] (int i) { return _data[i]; }
    T *_data;

    static const char *name;
    static void register_class()
    {
        typedef PyImath::StaticFixedArray<MatrixRow,T,len> MatrixRow_helper;
        class_<MatrixRow> matrixRow_class(name,no_init);
        matrixRow_class
            .def("__len__", MatrixRow_helper::len)
            .def("__getitem__", MatrixRow_helper::getitem,return_value_policy<copy_non_const_reference>())
            .def("__setitem__", MatrixRow_helper::setitem)
            ;
    }
};

template <> const char *MatrixRow<float,3>::name = "M33fRow";
template <> const char *MatrixRow<double,3>::name = "M33dRow";


template <class Container, class Data, int len>
struct IndexAccessMatrixRow {
    typedef MatrixRow<Data,len> result_type;
    static MatrixRow<Data,len> apply(Container &c, int i) { return MatrixRow<Data,len>(c[i]); }
};

template <class T> struct Matrix33Name { static const char *value; };
template<> const char *Matrix33Name<float>::value  = "M33f";
template<> const char *Matrix33Name<double>::value = "M33d";

template <class T>
static std::string Matrix33_str(const Matrix33<T> &v)
{
    std::stringstream stream;
    stream << Matrix33Name<T>::value << "(";
    for (int row = 0; row < 3; row++)
    {
        stream << "(";
	for (int col = 0; col < 3; col++)
	{
	    stream << v[row][col];
            stream << (col != 2 ? ", " : "");
	}
        stream << ")" << (row != 2 ? ", " : "");
    }
    stream << ")";
    return stream.str();
}

// Non-specialized repr is same as str
template <class T>
static std::string Matrix33_repr(const Matrix33<T> &v)
{
    return Matrix33_str(v);
}

// Specialization for float to full precision
template <>
std::string Matrix33_repr(const Matrix33<float> &v)
{
    return (boost::format("%s((%.9g, %.9g, %.9g), (%.9g, %.9g, %.9g), (%.9g, %.9g, %.9g))")
                        % Matrix33Name<float>::value
                        % v[0][0] % v[0][1] % v[0][2]
                        % v[1][0] % v[1][1] % v[1][2]
                        % v[2][0] % v[2][1] % v[2][2]).str();
}

// Specialization for double to full precision
template <>
std::string Matrix33_repr(const Matrix33<double> &v)
{
    return (boost::format("%s((%.17g, %.17g, %.17g), (%.17g, %.17g, %.17g), (%.17g, %.17g, %.17g))")
                        % Matrix33Name<double>::value
                        % v[0][0] % v[0][1] % v[0][2]
                        % v[1][0] % v[1][1] % v[1][2]
                        % v[2][0] % v[2][1] % v[2][2]).str();
}

template <class T>
static const Matrix33<T> &
invert33 (Matrix33<T> &m, bool singExc = true)
{
    MATH_EXC_ON;
    return m.invert(singExc);
}

template <class T>
static Matrix33<T>
inverse33 (Matrix33<T> &m, bool singExc = true)
{
    MATH_EXC_ON;
    return m.inverse(singExc);
}

template <class T>
static const Matrix33<T> &
gjInvert33 (Matrix33<T> &m, bool singExc = true)
{
    MATH_EXC_ON;
    return m.gjInvert(singExc);
}

template <class T>
static Matrix33<T>
gjInverse33 (Matrix33<T> &m, bool singExc = true)
{
    MATH_EXC_ON;
    return m.gjInverse(singExc);
}

template <class T, class U>
static const Matrix33<T> &
iadd33(Matrix33<T> &m, const Matrix33<U> &m2)
{
    MATH_EXC_ON;
    Matrix33<T> m3;
    m3.setValue (m2);
    return m += m3;
}

template <class T>
static const Matrix33<T> &
iadd33T(Matrix33<T> &mat, T a)
{
    MATH_EXC_ON;
    return mat += a;
}

template <class T>
static Matrix33<T>
add33(Matrix33<T> &m, const Matrix33<T> &m2)
{
    MATH_EXC_ON;
    return m + m2;
}

template <class T, class U>
static const Matrix33<T> &
isub33(Matrix33<T> &m, const Matrix33<U> &m2)
{
    MATH_EXC_ON;
    Matrix33<T> m3;
    m3.setValue (m2);
    return m -= m3;
}

template <class T>
static const Matrix33<T> &
isub33T(Matrix33<T> &mat, T a)
{
    MATH_EXC_ON;
    return mat -= a;
}

template <class T>
static Matrix33<T>
sub33(Matrix33<T> &m, const Matrix33<T> &m2)
{
    MATH_EXC_ON;
    return m - m2;
}

template <class T>
static const Matrix33<T> &
negate33 (Matrix33<T> &m)
{
    MATH_EXC_ON;
    return m.negate();
}

template <class T>
static Matrix33<T>
neg33 (Matrix33<T> &m)
{
    MATH_EXC_ON;
    return -m;
}

template <class T>
static const Matrix33<T> &
imul33T(Matrix33<T> &m, const T &t)
{
    MATH_EXC_ON;
    return m *= t;
}

template <class T>
static Matrix33<T>
mul33T(Matrix33<T> &m, const T &t)
{
    MATH_EXC_ON;
    return m * t;
}

template <class T>
static Matrix33<T>
rmul33T(Matrix33<T> &m, const T &t)
{
    MATH_EXC_ON;
    return t * m;
}

template <class T>
static const Matrix33<T> &
idiv33T(Matrix33<T> &m, const T &t)
{
    MATH_EXC_ON;
    return m /= t;
}

template <class T>
static Matrix33<T>
div33T(Matrix33<T> &m, const T &t)
{
    MATH_EXC_ON;
    return m / t;
}

template <class T>
static void 
extractAndRemoveScalingAndShear33(Matrix33<T> &mat, IMATH_NAMESPACE::Vec2<T> &dstScl, IMATH_NAMESPACE::Vec2<T> &dstShr, int exc = 1)
{
    MATH_EXC_ON;
    T dstShrTmp;
    IMATH_NAMESPACE::extractAndRemoveScalingAndShear(mat, dstScl, dstShrTmp, exc);

    dstShr.setValue(dstShrTmp, T (0));
}

template <class T>
static void
extractEuler(Matrix33<T> &mat, Vec2<T> &dstObj)
{
    MATH_EXC_ON;
    T dst;
    IMATH_NAMESPACE::extractEuler(mat, dst);
    dstObj.setValue(dst, T (0));
}

template <class T>
static int
extractSHRT33(Matrix33<T> &mat, Vec2<T> &s, Vec2<T> &h, Vec2<T> &r, Vec2<T> &t, int exc = 1)
{
    MATH_EXC_ON;
    T hTmp, rTmp;
    
    int b = IMATH_NAMESPACE::extractSHRT(mat, s, hTmp, rTmp, t, exc);
    
    h.setValue(hTmp, T (0));
    r.setValue(rTmp, T (0));
    
    return b;
}

template <class T>
static void
extractScaling33(Matrix33<T> &mat, Vec2<T> &dst, int exc = 1)
{
    MATH_EXC_ON;
    IMATH_NAMESPACE::extractScaling(mat, dst, exc);
}

template <class T>
void
outerProduct33(Matrix33<T> &mat, const Vec3<T> &a, const Vec3<T> &b)
{
    MATH_EXC_ON;
    mat = IMATH_NAMESPACE::outerProduct(a,b);
}

template <class T>
static void
extractScalingAndShear33(Matrix33<T> &mat, Vec2<T> &dstScl, Vec2<T> &dstShr, int exc = 1)
{
    MATH_EXC_ON;
    T dstShrTmp;
    IMATH_NAMESPACE::extractScalingAndShear(mat, dstScl, dstShrTmp, exc);
    
    dstShr.setValue(dstShrTmp, T (0));
}

template <class TV,class TM>
static void
multDirMatrix33(Matrix33<TM> &mat, const Vec2<TV> &src, Vec2<TV> &dst)
{
    MATH_EXC_ON;
    mat.multDirMatrix(src, dst);    
}

template <class TV,class TM>
static Vec2<TV>
multDirMatrix33_return_value(Matrix33<TM> &mat, const Vec2<TV> &src)
{
    MATH_EXC_ON;
    Vec2<TV> dst;
    mat.multDirMatrix(src, dst);    
    return dst;
}

template <class TV,class TM>
static FixedArray<Vec2<TV> >
multDirMatrix33_array(Matrix33<TM> &mat, const FixedArray<Vec2<TV> >&src)
{
    MATH_EXC_ON;
    size_t len = src.len();
    FixedArray<Vec2<TV> > dst(len);
    for (size_t i=0; i<len; ++i) mat.multDirMatrix(src[i], dst[i]);    
    return dst;
}

template <class TV,class TM>
static void
multVecMatrix33(Matrix33<TM> &mat, const Vec2<TV> &src, Vec2<TV> &dst)
{
    MATH_EXC_ON;
    mat.multVecMatrix(src, dst);    
}

template <class TV,class TM>
static Vec2<TV>
multVecMatrix33_return_value(Matrix33<TM> &mat, const Vec2<TV> &src)
{
    MATH_EXC_ON;
    Vec2<TV> dst;
    mat.multVecMatrix(src, dst);    
    return dst;
}

template <class TV,class TM>
static FixedArray<Vec2<TV> >
multVecMatrix33_array(Matrix33<TM> &mat, const FixedArray<Vec2<TV> >&src)
{
    MATH_EXC_ON;
    size_t len = src.len();
    FixedArray<Vec2<TV> > dst(len);
    for (size_t i=0; i<len; ++i) mat.multVecMatrix(src[i], dst[i]);    
    return dst;
}

template <class T>
static int
removeScaling33(Matrix33<T> &mat, int exc = 1)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::removeScaling(mat, exc);
}


template <class T>
static int
removeScalingAndShear33(Matrix33<T> &mat, int exc = 1)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::removeScalingAndShear(mat, exc);
}

template <class T>
static const Matrix33<T> &
rotate33(Matrix33<T> &mat, const T &r)
{
    MATH_EXC_ON;
    return mat.rotate(r);    
}


template <class T>
static Matrix33<T>
sansScaling33(const Matrix33<T> &mat, bool exc = true)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::sansScaling(mat, exc);
}

template <class T>
static Matrix33<T>
sansScalingAndShear33(const Matrix33<T> &mat, bool exc = true)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::sansScalingAndShear(mat, exc);
}

template <class T>
static const Matrix33<T> &
scaleSc33(Matrix33<T> &mat, const T &s)
{
    MATH_EXC_ON;
    Vec2<T> sVec(s, s);
    return mat.scale(sVec);
}

template <class T>
static const Matrix33<T> &
scaleV33(Matrix33<T> &mat, const Vec2<T> &s)
{
    MATH_EXC_ON;
    return mat.scale(s);
}

template <class T>
static const Matrix33<T> &
scale33Tuple(Matrix33<T> &mat, const tuple &t)
{
    MATH_EXC_ON;
    if(t.attr("__len__")() == 2)
    {
        Vec2<T> s;
        s.x = extract<T>(t[0]);
        s.y = extract<T>(t[1]);
        
        return mat.scale(s);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "m.scale needs tuple of length 2");
}

template <class T>
static const Matrix33<T> &
setRotation33(Matrix33<T> &mat, const T &r)
{
    MATH_EXC_ON;
    return mat.setRotation(r);    
}

template <class T>
static const Matrix33<T> &
setScaleSc33(Matrix33<T> &mat, const T &s)
{
    MATH_EXC_ON;
    Vec2<T> sVec(s, s);
    return mat.setScale(sVec);
}

template <class T>
static const Matrix33<T> &
setScaleV33(Matrix33<T> &mat, const Vec2<T> &s)
{
    MATH_EXC_ON;
    return mat.setScale(s);
}

template <class T>
static const Matrix33<T> &
setScale33Tuple(Matrix33<T> &mat, const tuple &t)
{
    MATH_EXC_ON;
    if(t.attr("__len__")() == 2)
    {
        Vec2<T> s;
        s.x = extract<T>(t[0]);
        s.y = extract<T>(t[1]);
        
        return mat.setScale(s);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "m.setScale needs tuple of length 2");
}

template <class T>
static const Matrix33<T> &
setShearSc33(Matrix33<T> &mat, const T &h)
{
    MATH_EXC_ON;
    Vec2<T> hVec(h, T(0));    
    return mat.setShear(hVec);
}

template <class T>
static const Matrix33<T> &
setShearV33(Matrix33<T> &mat, const Vec2<T> &h)
{
    MATH_EXC_ON;
    return mat.setShear(h);
}

template <class T>
static const Matrix33<T> &
setShear33Tuple(Matrix33<T> &mat, const tuple &t)
{
    MATH_EXC_ON;
    if(t.attr("__len__")() == 2)
    {
        Vec2<T> h;
        h.x = extract<T>(t[0]);
        h.y = extract<T>(t[1]);
        
        return mat.setShear(h);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "m.shear needs tuple of length 2");
}

template <class T>
static const Matrix33<T> &
setTranslation33(Matrix33<T> &mat, const Vec2<T> &t)
{
    MATH_EXC_ON;
    return mat.setTranslation(t);
}

template <class T>
static const Matrix33<T> &
setTranslation33Tuple(Matrix33<T> &mat, const tuple &t)
{
    MATH_EXC_ON;
    if(t.attr("__len__")() == 2)
    {
        Vec2<T> trans;
        trans.x = extract<T>(t[0]);
        trans.y = extract<T>(t[1]);
        
        return mat.setTranslation(trans);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "m.translate needs tuple of length 2");
}

template <class T>
static const Matrix33<T> &
setTranslation33Obj(Matrix33<T> &mat, const object &o)
{
    MATH_EXC_ON;
    Vec2<T> v;
    if (PyImath::V2<T>::convert (o.ptr(), &v))
    {
        return mat.setTranslation(v);
    }
    else
    {
        THROW(IEX_NAMESPACE::ArgExc, "m.setTranslation expected V2 argument");
        return mat;
    }   
}

template <class T>
static void
setValue33(Matrix33<T> &mat, const Matrix33<T> &value)
{
    MATH_EXC_ON;
    mat.setValue(value);
}

template <class T>
static const Matrix33<T> &
shearSc33(Matrix33<T> &mat, const T &h)
{
    MATH_EXC_ON;
    Vec2<T> hVec(h, T (0));
    
    return mat.shear(hVec);
}

template <class T>
static const Matrix33<T> &
shearV33(Matrix33<T> &mat, const Vec2<T> &h)
{
    MATH_EXC_ON;
    return mat.shear(h);
}

template <class T>
static const Matrix33<T> &
shear33Tuple(Matrix33<T> &mat, const tuple &t)
{
    MATH_EXC_ON;
    if(t.attr("__len__")() == 2)
    {
        Vec2<T> h;
        h.x = extract<T>(t[0]);
        h.y = extract<T>(t[1]);
        
        return mat.shear(h);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "m.shear needs tuple of length 2");
}

template <class T>
static const Matrix33<T> &
translate33(Matrix33<T> &mat, const object &t)
{
    MATH_EXC_ON;
    Vec2<T> v;
    if (PyImath::V2<T>::convert (t.ptr(), &v))
    {
        return mat.translate(v);
    }
    else
    {
        THROW(IEX_NAMESPACE::ArgExc, "m.translate expected V2 argument");
        return mat;
    }   
}

template <class T>
static const Matrix33<T> &
translate33Tuple(Matrix33<T> &mat, const tuple &t)
{
    MATH_EXC_ON;
    if(t.attr("__len__")() == 2)
    {
        Vec2<T> trans;
        trans.x = extract<T>(t[0]);
        trans.y = extract<T>(t[1]);
        
        return mat.translate(trans);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "m.translate needs tuple of length 2");
}

template <class T>
static Matrix33<T>
subtractTL33(Matrix33<T> &mat, T a)
{
    MATH_EXC_ON;
    Matrix33<T> m(mat.x);
    for(int i = 0; i < 3; ++i)
        for(int j = 0; j < 3; ++j)
            m.x[i][j] -= a;
    
    return m;
}

template <class T>
static Matrix33<T>
subtractTR33(Matrix33<T> &mat, T a)
{
    MATH_EXC_ON;
    Matrix33<T> m(mat.x);
    for(int i = 0; i < 3; ++i)
        for(int j = 0; j < 3; ++j)
            m.x[i][j] = a - m.x[i][j];
    
    return m;
}


template <class T>
static Matrix33<T>
add33T(Matrix33<T> &mat, T a)
{
    MATH_EXC_ON;
    Matrix33<T> m(mat.x);
    for(int i = 0; i < 3; ++i)
        for(int j = 0; j < 3; ++j)
            m.x[i][j] += a;
    
    return m;
}

template <class S, class T>
static Matrix33<T>
mul33(Matrix33<T> &mat1, Matrix33<S> &mat2)
{
    MATH_EXC_ON;
    Matrix33<T> mat2T;
    mat2T.setValue (mat2);
    return mat1 * mat2T;
}

template <class S, class T>
static Matrix33<T>
rmul33(Matrix33<T> &mat2, Matrix33<S> &mat1)
{
    MATH_EXC_ON;
    Matrix33<T> mat1T;
    mat1T.setValue (mat1);
    return mat1T * mat2;
}

template <class S, class T>
static const Matrix33<T> &
imul33(Matrix33<T> &mat1, Matrix33<S> &mat2)
{
    MATH_EXC_ON;
    Matrix33<T> mat2T;
    mat2T.setValue (mat2);
    return mat1 *= mat2T;
}

template <class T>
static bool
lessThan33(Matrix33<T> &mat1, const Matrix33<T> &mat2)
{
    for(int i = 0; i < 3; ++i){
        for(int j = 0; j < 3; ++j){
            if(mat1[i][j] > mat2[i][j]){
                return false;
            }
        }
    }
    
    return (mat1 != mat2);            
}

template <class T>
static bool
lessThanEqual33(Matrix33<T> &mat1, const Matrix33<T> &mat2)
{
    for(int i = 0; i < 3; ++i){
        for(int j = 0; j < 3; ++j){
            if(mat1[i][j] > mat2[i][j]){
                return false;
            }
        }
    }
    
    return true;            
}

template <class T>
static bool
greaterThan33(Matrix33<T> &mat1, const Matrix33<T> &mat2)
{
    for(int i = 0; i < 3; ++i){
        for(int j = 0; j < 3; ++j){
            if(mat1[i][j] < mat2[i][j]){
                std::cout << mat1[i][j] << " " << mat2[i][j] << std::endl;
                return false;
            }
        }
    }
    
    return (mat1 != mat2);            
}

template <class T>
static bool
greaterThanEqual33(Matrix33<T> &mat1, const Matrix33<T> &mat2)
{
    for(int i = 0; i < 3; ++i){
        for(int j = 0; j < 3; ++j){
            if(mat1[i][j] < mat2[i][j]){
                return false;
            }
        }
    }
    
    return true;            
}

template <class T>
static tuple
singularValueDecomposition33(const Matrix33<T>& m, bool forcePositiveDeterminant = false)
{
    IMATH_NAMESPACE::Matrix33<T> U, V;
    IMATH_NAMESPACE::Vec3<T> S;
    IMATH_NAMESPACE::jacobiSVD (m, U, S, V, IMATH_NAMESPACE::limits<T>::epsilon(), forcePositiveDeterminant);
    return make_tuple (U, S, V);
}

BOOST_PYTHON_FUNCTION_OVERLOADS(invert33_overloads, invert33, 1, 2);
BOOST_PYTHON_FUNCTION_OVERLOADS(inverse33_overloads, inverse33, 1, 2);
BOOST_PYTHON_FUNCTION_OVERLOADS(gjInvert33_overloads, gjInvert33, 1, 2);
BOOST_PYTHON_FUNCTION_OVERLOADS(gjInverse33_overloads, gjInverse33, 1, 2);
BOOST_PYTHON_FUNCTION_OVERLOADS(extractAndRemoveScalingAndShear33_overloads, extractAndRemoveScalingAndShear33, 3, 4)
BOOST_PYTHON_FUNCTION_OVERLOADS(extractSHRT33_overloads, extractSHRT33, 5, 6)
BOOST_PYTHON_FUNCTION_OVERLOADS(extractScaling33_overloads, extractScaling33, 2, 3)
BOOST_PYTHON_FUNCTION_OVERLOADS(extractScalingAndShear33_overloads, extractScalingAndShear33, 3, 4)
BOOST_PYTHON_FUNCTION_OVERLOADS(removeScaling33_overloads, removeScaling33, 1, 2)
BOOST_PYTHON_FUNCTION_OVERLOADS(removeScalingAndShear33_overloads, removeScalingAndShear33, 1, 2)
BOOST_PYTHON_FUNCTION_OVERLOADS(sansScaling33_overloads, sansScaling33, 1, 2)
BOOST_PYTHON_FUNCTION_OVERLOADS(sansScalingAndShear33_overloads, sansScalingAndShear33, 1, 2)
BOOST_PYTHON_FUNCTION_OVERLOADS(outerProduct33_overloads, outerProduct33, 3, 3);

template <class T>
static Matrix33<T> * Matrix3_tuple_constructor(const tuple &t0, const tuple &t1, const tuple &t2)
{
  if(t0.attr("__len__")() == 3 && t1.attr("__len__")() == 3 && t2.attr("__len__")() == 3)
  {
      return new Matrix33<T>(extract<T>(t0[0]),  extract<T>(t0[1]),  extract<T>(t0[2]),
                             extract<T>(t1[0]),  extract<T>(t1[1]),  extract<T>(t1[2]),
                             extract<T>(t2[0]),  extract<T>(t2[1]),  extract<T>(t2[2]));
  }
  else
      THROW(IEX_NAMESPACE::LogicExc, "Matrix33 takes 3 tuples of length 3");
}

template <class T, class S>
static Matrix33<T> *Matrix3_matrix_constructor(const Matrix33<S> &mat)
{
    Matrix33<T> *m = new Matrix33<T>;
    
    for(int i = 0; i < 3; ++i)
        for(int j = 0; j < 3; ++j)
            m->x[i][j] = T (mat.x[i][j]);
    
    return m;
}

template <class T>
class_<Matrix33<T> >
register_Matrix33()
{
    typedef PyImath::StaticFixedArray<Matrix33<T>,T,3,IndexAccessMatrixRow<Matrix33<T>,T,3> > Matrix33_helper;

    MatrixRow<T,3>::register_class();
    class_<Matrix33<T> > matrix33_class(Matrix33Name<T>::value, Matrix33Name<T>::value,init<Matrix33<T> >("copy construction"));
    matrix33_class
        .def(init<>("initialize to identity"))
        .def(init<T>("initialize all entries to a single value"))
        .def(init<T,T,T,T,T,T,T,T,T>("make from components"))
        .def("__init__", make_constructor(Matrix3_tuple_constructor<T>))
        .def("__init__", make_constructor(Matrix3_matrix_constructor<T,float>))
        .def("__init__", make_constructor(Matrix3_matrix_constructor<T,double>))
        
	//.def_readwrite("x00", &Matrix33<T>::x[0][0])
	//.def_readwrite("x01", &Matrix33<T>::x[0][1])
	//.def_readwrite("x02", &Matrix33<T>::x[0][2])
	//.def_readwrite("x10", &Matrix33<T>::x[1][0])
	//.def_readwrite("x11", &Matrix33<T>::x[1][1])
	//.def_readwrite("x12", &Matrix33<T>::x[1][2])
	//.def_readwrite("x20", &Matrix33<T>::x[2][0])
	//.def_readwrite("x21", &Matrix33<T>::x[2][1])
	//.def_readwrite("x22", &Matrix33<T>::x[2][2])
        .def("baseTypeEpsilon", &Matrix33<T>::baseTypeEpsilon,"baseTypeEpsilon() epsilon value of the base type of the vector")
        .staticmethod("baseTypeEpsilon")
        .def("baseTypeMax", &Matrix33<T>::baseTypeMax,"baseTypeMax() max value of the base type of the vector")
        .staticmethod("baseTypeMax")
        .def("baseTypeMin", &Matrix33<T>::baseTypeMin,"baseTypeMin() min value of the base type of the vector")
        .staticmethod("baseTypeMin")
        .def("baseTypeSmallest", &Matrix33<T>::baseTypeSmallest,"baseTypeSmallest() smallest value of the base type of the vector")
        .staticmethod("baseTypeSmallest")
        .def("equalWithAbsError", &Matrix33<T>::equalWithAbsError,"m1.equalWithAbsError(m2,e) true if the elements "
             "of v1 and v2 are the same with an absolute error of no more than e, "
             "i.e., abs(m1[i] - m2[i]) <= e")
        .def("equalWithRelError", &Matrix33<T>::equalWithRelError,"m1.equalWithAbsError(m2,e) true if the elements "
             "of m1 and m2 are the same with an absolute error of no more than e, "
             "i.e., abs(m1[i] - m2[i]) <= e * abs(m1[i])")
        // need a different version for matrix data access
        .def("__len__", Matrix33_helper::len)
        .def("__getitem__", Matrix33_helper::getitem)
	//.def("__setitem__", Matrix33_helper::setitem)
        .def("makeIdentity",&Matrix33<T>::makeIdentity,"makeIdentity() make this matrix the identity matrix")
        .def("transpose",&Matrix33<T>::transpose,return_internal_reference<>(),"transpose() transpose this matrix")
        .def("transposed",&Matrix33<T>::transposed,"transposed() return a transposed copy of this matrix")
        .def("invert",&invert33<T>,invert33_overloads("invert() invert this matrix")[return_internal_reference<>()])
        .def("inverse",&inverse33<T>,inverse33_overloads("inverse() return a inverted copy of this matrix"))
        .def("gjInvert",&gjInvert33<T>,gjInvert33_overloads("gjInvert() invert this matrix")[return_internal_reference<>()])
        .def("gjInverse",&gjInverse33<T>,gjInverse33_overloads("gjInverse() return a inverted copy of this matrix"))
        .def("minorOf",&Matrix33<T>::minorOf,"minorOf() return the matrix minor of the (row,col) element of this matrix")
        .def("fastMinor",&Matrix33<T>::fastMinor,"fastMinor() return the matrix minor using the specified rows and columns of this matrix")
        .def("determinant",&Matrix33<T>::determinant,"determinant() return the determinant of this matrix")
        .def(self == self) // NOSONAR - suppress SonarCloud bug report.
        .def(self != self) // NOSONAR - suppress SonarCloud bug report.
        .def("__iadd__", &iadd33<T, float>,return_internal_reference<>())
        .def("__iadd__", &iadd33<T, double>,return_internal_reference<>())
        .def("__iadd__", &iadd33T<T>,return_internal_reference<>())
        .def("__add__", &add33<T>)
        .def("__isub__", &isub33<T, float>,return_internal_reference<>())
        .def("__isub__", &isub33<T, double>,return_internal_reference<>())
        .def("__isub__", &isub33T<T>,return_internal_reference<>())
        .def("__sub__", &sub33<T>)
        .def("negate",&negate33<T>,return_internal_reference<>(),"negate() negate all entries in this matrix")
        .def("__neg__", &neg33<T>)
        .def("__imul__", &imul33T<T>,return_internal_reference<>())
        .def("__mul__", &mul33T<T>)
        .def("__rmul__", &rmul33T<T>)
        .def("__idiv__", &idiv33T<T>,return_internal_reference<>())
        .def("__itruediv__", &idiv33T<T>,return_internal_reference<>())
        .def("__div__", &div33T<T>)
        .def("__truediv__", &div33T<T>)
        .def("__add__", &add33T<T>)
        .def("__radd__", &add33T<T>)
        .def("__sub__", &subtractTL33<T>)
        .def("__rsub__", &subtractTR33<T>)
        .def("__mul__", &mul33<float, T>)
        .def("__mul__", &mul33<double, T>)
        .def("__rmul__", &rmul33<float, T>)
        .def("__rmul__", &rmul33<double, T>)
        .def("__imul__", &imul33<float, T>,return_internal_reference<>())
        .def("__imul__", &imul33<double, T>,return_internal_reference<>())
        .def("__lt__", &lessThan33<T>)
        .def("__le__", &lessThanEqual33<T>)
        .def("__gt__", &greaterThan33<T>)
        .def("__ge__", &greaterThanEqual33<T>)
	//.def(self_ns::str(self))
        .def("__str__",&Matrix33_str<T>)
        .def("__repr__",&Matrix33_repr<T>)
        .def("extractAndRemoveScalingAndShear", &extractAndRemoveScalingAndShear33<T>, 
              extractAndRemoveScalingAndShear33_overloads(
              "M.extractAndRemoveScalingAndShear(scl, shr, "
              "[exc]) -- extracts the scaling component of "
              "M into scl and the shearing component of M "
              "into shr.  Also removes the scaling and "
              "shearing components from M.  "
              "Returns 1 unless the scaling component is "
              "nearly 0, in which case 0 is returned. "
              "If optional arg. exc == 1, then if the "
              "scaling component is nearly 0, then MathExc "
              "is thrown. "))
             
         .def("extractEuler", &extractEuler<T>, 				
              "M.extractEulerZYX(r) -- extracts the "
			  "rotation component of M into r. "
              "Assumes that M contains no shear or "
              "non-uniform scaling; results are "
              "meaningless if it does.")
              
         .def("extractSHRT", &extractSHRT33<T>, extractSHRT33_overloads(				
              "M.extractSHRT(Vs, Vh, Vr, Vt, [exc]) -- "
	          "extracts the scaling component of M into Vs, "
			  "the shearing component of M in Vh (as XY, "
	          "XZ, YZ shear factors), the rotation of M "
	          "into Vr (as Euler angles in the order XYZ), "
	          "and the translaation of M into Vt. "
			  "If optional arg. exc == 1, then if the "
			  "scaling component is nearly 0, then MathExc "
			  "is thrown. "))
              
         .def("extractScaling", &extractScaling33<T>, extractScaling33_overloads("extract scaling"))
         .def("outerProduct", &outerProduct33<T>, outerProduct33_overloads(
              "M.outerProduct(Va,Vb) -- "
                  "Performs the outer product, or tensor product, of two 3D vectors, Va and Vb"))

         .def("extractScalingAndShear", &extractScalingAndShear33<T>, extractScalingAndShear33_overloads("extract scaling"))
        .def("singularValueDecomposition", &singularValueDecomposition33<T>, 
             "Decomposes the matrix using the singular value decomposition (SVD) into three\n"
             "matrices U, S, and V which have the following properties: \n"
             "  1. U and V are both orthonormal matrices, \n"
             "  2. S is the diagonal matrix of singular values, \n"
             "  3. U * S * V.transposed() gives back the original matrix.\n"
             "The result is returned as a tuple [U, S, V].  Note that since S is diagonal we\n"
             "don't need to return the entire matrix, so we return it as a three-vector.  \n"
             "\n"
             "The 'forcePositiveDeterminant' argument can be used to force the U and V^T to\n"
             "have positive determinant (that is, to be proper rotation matrices); if\n"
             "forcePositiveDeterminant is False, then the singular values are guaranteed to\n"
             "be nonnegative but the U and V matrices might contain negative scale along one\n"
             "of the axes; if forcePositiveDeterminant is True, then U and V cannot contain\n"
             "negative scale but S[2] might be negative.  \n"
             "\n"
             "Our SVD implementation uses two-sided Jacobi rotations to iteratively\n"
             "diagonalize the matrix, which should be quite robust and significantly faster\n"
             "than the more general SVD solver in LAPACK.  \n",
             args("matrix", "forcePositiveDeterminant"))
        .def("symmetricEigensolve", &PyImath::jacobiEigensolve<IMATH_NAMESPACE::Matrix33<T> >, 
             "Decomposes the matrix A using a symmetric eigensolver into matrices Q and S \n"
             "which have the following properties: \n"
             "  1. Q is the orthonormal matrix of eigenvectors, \n"
             "  2. S is the diagonal matrix of eigenvalues, \n"
             "  3. Q * S * Q.transposed() gives back the original matrix.\n"
             "\n"
             "IMPORTANT: It is vital that the passed-in matrix be symmetric, or the result \n"
             "won't make any sense.  This function will return an error if passed an \n"
             "unsymmetric matrix.\n"
             "\n"
             "The result is returned as a tuple [Q, S].  Note that since S is diagonal \n"
             "we don't need to return the entire matrix, so we return it as a three-vector. \n"
             "\n"
             "Our eigensolver implementation uses one-sided Jacobi rotations to iteratively \n"
             "diagonalize the matrix, which should be quite robust and significantly faster \n"
             "than the more general symmetric eigenvalue solver in LAPACK.  \n")
         .def("multDirMatrix", &multDirMatrix33<double,T>, "mult matrix")
         .def("multDirMatrix", &multDirMatrix33_return_value<double,T>, "mult matrix")
         .def("multDirMatrix", &multDirMatrix33_array<double,T>, "mult matrix")
         .def("multDirMatrix", &multDirMatrix33<float,T>, "mult matrix")
         .def("multDirMatrix", &multDirMatrix33_return_value<float,T>, "mult matrix")
         .def("multDirMatrix", &multDirMatrix33_array<float,T>, "mult matrix")
         .def("multVecMatrix", &multVecMatrix33<double,T>, "mult matrix")
         .def("multVecMatrix", &multVecMatrix33_return_value<double,T>, "mult matrix")
         .def("multVecMatrix", &multVecMatrix33_array<double,T>, "mult matrix")
         .def("multVecMatrix", &multVecMatrix33<float,T>, "mult matrix")
         .def("multVecMatrix", &multVecMatrix33_return_value<float,T>, "mult matrix")
         .def("multVecMatrix", &multVecMatrix33_array<float,T>, "mult matrix")
         .def("removeScaling", &removeScaling33<T>, removeScaling33_overloads("remove scaling"))

         .def("removeScalingAndShear", &removeScalingAndShear33<T>, removeScalingAndShear33_overloads("remove scaling"))
         .def("sansScaling", &sansScaling33<T>, sansScaling33_overloads("sans scaling"))
         .def("rotate", &rotate33<T>, return_internal_reference<>(),"rotate matrix")

         .def("sansScalingAndShear", &sansScalingAndShear33<T>, sansScalingAndShear33_overloads("sans scaling and shear"))
         .def("scale", &scaleSc33<T>, return_internal_reference<>(),"scale matrix")
         .def("scale", &scaleV33<T>, return_internal_reference<>(),"scale matrix")
         .def("scale", &scale33Tuple<T>, return_internal_reference<>(),"scale matrix")

         .def("setRotation", &setRotation33<T>, return_internal_reference<>(),"setRotation()")
         .def("setScale", &setScaleSc33<T>, return_internal_reference<>(),"setScale()")
         .def("setScale", &setScaleV33<T>, return_internal_reference<>(),"setScale()")
         .def("setScale", &setScale33Tuple<T>, return_internal_reference<>(),"setScale()")

         .def("setShear", &setShearSc33<T>, return_internal_reference<>(),"setShear()")
         .def("setShear", &setShearV33<T>, return_internal_reference<>(),"setShear()")
         .def("setShear", &setShear33Tuple<T>, return_internal_reference<>(),"setShear()")
         
         .def("setTranslation", &setTranslation33<T>, return_internal_reference<>(),"setTranslation()")
         .def("setTranslation", &setTranslation33Tuple<T>, return_internal_reference<>(),"setTranslation()")
         .def("setTranslation", &setTranslation33Obj<T>, return_internal_reference<>(),"setTranslation()")
         .def("setValue", &setValue33<T>, "setValue()")
         .def("shear", &shearSc33<T>, return_internal_reference<>(),"shear()")
         .def("shear", &shearV33<T>, return_internal_reference<>(),"shear()")
         .def("shear", &shear33Tuple<T>, return_internal_reference<>(),"shear()")
         .def("translate", &translate33<T>, return_internal_reference<>(),"translate()")
         .def("translate", &translate33Tuple<T>, return_internal_reference<>(),"translate()")
         .def("translation", &Matrix33<T>::translation, "translation()")
         ;

    decoratecopy(matrix33_class);

    return matrix33_class;
/*
    const Matrix33 &	operator = (const Matrix33 &v);
    const Matrix33 &	operator = (T a);
    T *			getValue ();
    const T *		getValue () const;
    template <class S> void getValue (Matrix33<S> &v) const;
    template <class S> Matrix33 & setValue (const Matrix33<S> &v);
    template <class S> Matrix33 & setTheMatrix (const Matrix33<S> &v);
    template <class S> void multVecMatrix(const Vec2<S> &src, Vec2<S> &dst) const;
    template <class S> void multDirMatrix(const Vec2<S> &src, Vec2<S> &dst) const;
    template <class S> const Matrix33 &	setRotation (S r);
    template <class S> const Matrix33 &	rotate (S r);
    const Matrix33 &	setScale (T s);
    template <class S> const Matrix33 &	setScale (const Vec2<S> &s);
    template <class S> const Matrix33 &	scale (const Vec2<S> &s);
    template <class S> const Matrix33 &	setTranslation (const Vec2<S> &t);
    Vec2<T>		translation () const;
    template <class S> const Matrix33 &	translate (const Vec2<S> &t);
    template <class S> const Matrix33 &	setShear (const S &h);
    template <class S> const Matrix33 &	setShear (const Vec2<S> &h);
    template <class S> const Matrix33 &	shear (const S &xy);
    template <class S> const Matrix33 &	shear (const Vec2<S> &h);
*/
}

template <class T>
static void
setM33ArrayItem(FixedArray<IMATH_NAMESPACE::Matrix33<T> > &ma,
                Py_ssize_t index,
                const IMATH_NAMESPACE::Matrix33<T> &m)
{
    ma[ma.canonical_index(index)] = m;
}

template <class T>
class_<FixedArray<IMATH_NAMESPACE::Matrix33<T> > >
register_M33Array()
{
    class_<FixedArray<IMATH_NAMESPACE::Matrix33<T> > > matrixArray_class = FixedArray<IMATH_NAMESPACE::Matrix33<T> >::register_("Fixed length array of IMATH_NAMESPACE::Matrix33");
    matrixArray_class
         .def("__setitem__", &setM33ArrayItem<T>)
        ;
    return matrixArray_class;
}

template PYIMATH_EXPORT class_<IMATH_NAMESPACE::Matrix33<float> > register_Matrix33<float>();
template PYIMATH_EXPORT class_<IMATH_NAMESPACE::Matrix33<double> > register_Matrix33<double>();

template PYIMATH_EXPORT class_<FixedArray<IMATH_NAMESPACE::Matrix33<float> > > register_M33Array<float>();
template PYIMATH_EXPORT class_<FixedArray<IMATH_NAMESPACE::Matrix33<double> > > register_M33Array<double>();


template<> PYIMATH_EXPORT IMATH_NAMESPACE::Matrix33<float> FixedArrayDefaultValue<IMATH_NAMESPACE::Matrix33<float> >::value() { return IMATH_NAMESPACE::Matrix33<float>(); }
template<> PYIMATH_EXPORT IMATH_NAMESPACE::Matrix33<double> FixedArrayDefaultValue<IMATH_NAMESPACE::Matrix33<double> >::value() { return IMATH_NAMESPACE::Matrix33<double>(); }
}
