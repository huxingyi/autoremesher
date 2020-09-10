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
#include "PyImathTask.h"

namespace PyImath {
template<> const char PYIMATH_EXPORT *PyImath::M44fArray::name() { return "M44fArray"; }
template<> const char PYIMATH_EXPORT *PyImath::M44dArray::name() { return "M44dArray"; }

using namespace boost::python;
using namespace IMATH_NAMESPACE;
using namespace PyImath;

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

template <> const char *MatrixRow<float,4>::name = "M44fRow";
template <> const char *MatrixRow<double,4>::name = "M44dRow";


template <class Container, class Data, int len>
struct IndexAccessMatrixRow {
    typedef MatrixRow<Data,len> result_type;
    static MatrixRow<Data,len> apply(Container &c, int i) { return MatrixRow<Data,len>(c[i]); }
};

template <class T> struct Matrix44Name { static const char *value; };
template<> const char *Matrix44Name<float>::value  = "M44f";
template<> const char *Matrix44Name<double>::value = "M44d";

template <class T>
static std::string Matrix44_str(const Matrix44<T> &v)
{
    std::stringstream stream;
    stream << Matrix44Name<T>::value << "(";
    for (int row = 0; row < 4; row++)
    {
        stream << "(";
	for (int col = 0; col < 4; col++)
	{
	    stream << v[row][col];
            stream << (col != 3 ? ", " : "");
	}
        stream << ")" << (row != 3 ? ", " : "");
    }
    stream << ")";
    return stream.str();
}

// Non-specialized repr is same as str
template <class T>
static std::string Matrix44_repr(const Matrix44<T> &v)
{
    return Matrix44_str(v);
}

// Specialization for float to full precision
template <>
std::string Matrix44_repr(const Matrix44<float> &v)
{
    return (boost::format("%s((%.9g, %.9g, %.9g, %.9g), (%.9g, %.9g, %.9g, %.9g), (%.9g, %.9g, %.9g, %.9g), (%.9g, %.9g, %.9g, %.9g))")
                        % Matrix44Name<float>::value
                        % v[0][0] % v[0][1] % v[0][2] % v[0][3]
                        % v[1][0] % v[1][1] % v[1][2] % v[1][3]
                        % v[2][0] % v[2][1] % v[2][2] % v[2][3]
                        % v[3][0] % v[3][1] % v[3][2] % v[3][3]).str();
}

// Specialization for double to full precision
template <>
std::string Matrix44_repr(const Matrix44<double> &v)
{
    return (boost::format("%s((%.17g, %.17g, %.17g, %.17g), (%.17g, %.17g, %.17g, %.17g), (%.17g, %.17g, %.17g, %.17g), (%.17g, %.17g, %.17g, %.17g))")
                        % Matrix44Name<double>::value
                        % v[0][0] % v[0][1] % v[0][2] % v[0][3]
                        % v[1][0] % v[1][1] % v[1][2] % v[1][3]
                        % v[2][0] % v[2][1] % v[2][2] % v[2][3]
                        % v[3][0] % v[3][1] % v[3][2] % v[3][3]).str();
}

template <class T>
static const Matrix44<T> &
invert44 (Matrix44<T> &m, bool singExc = true)
{
    MATH_EXC_ON;
    return m.invert(singExc);
}

template <class T>
static Matrix44<T>
inverse44 (Matrix44<T> &m, bool singExc = true)
{
    MATH_EXC_ON;
    return m.inverse(singExc);
}

template <class T>
static const Matrix44<T> &
gjInvert44 (Matrix44<T> &m, bool singExc = true)
{
    MATH_EXC_ON;
    return m.gjInvert(singExc);
}

template <class T>
static Matrix44<T>
gjInverse44 (Matrix44<T> &m, bool singExc = true)
{
    MATH_EXC_ON;
    return m.gjInverse(singExc);
}

template <class T, class U>
static const Matrix44<T> &
iadd44(Matrix44<T> &m, const Matrix44<U> &m2)
{
    MATH_EXC_ON;
    Matrix44<T> m3;
    m3.setValue (m2);
    return m += m3;
}

template <class T>
static const Matrix44<T> &
iadd44T(Matrix44<T> &mat, T a)
{
    MATH_EXC_ON;
    return mat += a;
}

template <class T>
static Matrix44<T>
add44(Matrix44<T> &m, const Matrix44<T> &m2)
{
    MATH_EXC_ON;
    return m + m2;
}

template <class T, class U>
static const Matrix44<T> &
isub44(Matrix44<T> &m, const Matrix44<U> &m2)
{
    MATH_EXC_ON;
    Matrix44<T> m3;
    m3.setValue (m2);
    return m -= m3;
}

template <class T>
static const Matrix44<T> &
isub44T(Matrix44<T> &mat, T a)
{
    MATH_EXC_ON;
    return mat -= a;
}

template <class T>
static Matrix44<T>
sub44(Matrix44<T> &m, const Matrix44<T> &m2)
{
    MATH_EXC_ON;
    return m - m2;
}

template <class T>
static const Matrix44<T> &
negate44 (Matrix44<T> &m)
{
    MATH_EXC_ON;
    return m.negate();
}

template <class T>
static Matrix44<T>
neg44 (Matrix44<T> &m)
{
    MATH_EXC_ON;
    return -m;
}

template <class T>
static const Matrix44<T> &
imul44T(Matrix44<T> &m, const T &t)
{
    MATH_EXC_ON;
    return m *= t;
}

template <class T>
static Matrix44<T>
mul44T(Matrix44<T> &m, const T &t)
{
    MATH_EXC_ON;
    return m * t;
}

template <class T>
static Matrix44<T>
rmul44T(Matrix44<T> &m, const T &t)
{
    MATH_EXC_ON;
    return t * m;
}

template <class T>
static const Matrix44<T> &
idiv44T(Matrix44<T> &m, const T &t)
{
    MATH_EXC_ON;
    return m /= t;
}

template <class T>
static Matrix44<T>
div44T(Matrix44<T> &m, const T &t)
{
    MATH_EXC_ON;
    return m / t;
}

template <class T>
static void 
extractAndRemoveScalingAndShear44(Matrix44<T> &mat, Vec3<T> &dstScl, Vec3<T> &dstShr, int exc = 1)
{
    MATH_EXC_ON;
    IMATH_NAMESPACE::extractAndRemoveScalingAndShear(mat, dstScl, dstShr, exc);
}

template <class T>
static void
extractEulerXYZ(Matrix44<T> &mat, IMATH_NAMESPACE::Vec3<T> &dst)
{
    MATH_EXC_ON;
    IMATH_NAMESPACE::extractEulerXYZ(mat, dst);
}

template <class T>
static void
extractEulerZYX(Matrix44<T> &mat, IMATH_NAMESPACE::Vec3<T> &dst)
{
    MATH_EXC_ON;
    IMATH_NAMESPACE::extractEulerZYX(mat, dst);
}

template <class T>
static int
extractSHRT44(Matrix44<T> &mat, Vec3<T> &s, Vec3<T> &h, Vec3<T> &r, Vec3<T> &t, int exc = 1)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::extractSHRT(mat, s, h, r, t, exc);
}

template <class T>
static void
extractScaling44(Matrix44<T> &mat, Vec3<T> &dst, int exc = 1)
{
    MATH_EXC_ON;
    IMATH_NAMESPACE::extractScaling(mat, dst, exc);
}

template <class T>
static void
extractScalingAndShear44(Matrix44<T> &mat, Vec3<T> &dstScl, Vec3<T> &dstShr, int exc = 1)
{
    MATH_EXC_ON;
    IMATH_NAMESPACE::extractScalingAndShear(mat, dstScl, dstShr, exc);
}

template <class TV,class TM>
static void
multDirMatrix44(Matrix44<TM> &mat, const Vec3<TV> &src, Vec3<TV> &dst)
{
    MATH_EXC_ON;
    mat.multDirMatrix(src, dst);    
}

template <class TV,class TM>
static Vec3<TV>
multDirMatrix44_return_value(Matrix44<TM> &mat, const Vec3<TV> &src)
{
    MATH_EXC_ON;
    Vec3<TV> dst;
    mat.multDirMatrix(src, dst);    
    return dst;
}

template <class T1, class T2>
struct op_multDirMatrix {

    static inline void apply(const Matrix44<T2>& m, const Vec3<T1>& src, Vec3<T1>& dst)
    {
        m.multDirMatrix(src,dst);
    }
};

template <class T1, class T2>
struct op_multVecMatrix {

    static inline void apply(const Matrix44<T2>& m, const Vec3<T1>& src, Vec3<T1>& dst)
    {
        m.multVecMatrix(src,dst);
    }
};

template <class T1,class T2, class Op>
struct MatrixVecTask : public Task
{
    const Matrix44<T2> &mat;
    const FixedArray<Vec3<T1> >& src;
    FixedArray<Vec3<T1> >& dst;

    MatrixVecTask(const Matrix44<T2> &m, const FixedArray<Vec3<T1> >& s, FixedArray<Vec3<T1> >& d)
        : mat(m), src(s), dst(d) {}

    void execute(size_t start, size_t end)
    {
        for(size_t p = start; p < end; ++p) 
            Op::apply(mat,src[p],dst[p]);
    }
};

template <class TV,class TM>
static FixedArray<Vec3<TV> >
multDirMatrix44_array(Matrix44<TM> &mat, const FixedArray<Vec3<TV> >&src)
{
    MATH_EXC_ON;
    size_t len = src.len();
    FixedArray<Vec3<TV> > dst(len);

    MatrixVecTask<TV,TM,op_multDirMatrix<TV,TM> > task(mat,src,dst);
    dispatchTask(task,len);

    return dst;
}

template <class TV,class TM>
static void
multVecMatrix44(Matrix44<TM> &mat, const Vec3<TV> &src, Vec3<TV> &dst)
{
    MATH_EXC_ON;
    mat.multVecMatrix(src, dst);    
}

template <class TV,class TM>
static Vec3<TV>
multVecMatrix44_return_value(Matrix44<TM> &mat, const Vec3<TV> &src)
{
    MATH_EXC_ON;
    Vec3<TV> dst;
    mat.multVecMatrix(src, dst);    
    return dst;
}


template <class TV,class TM>
static FixedArray<Vec3<TV> >
multVecMatrix44_array(Matrix44<TM> &mat, const FixedArray<Vec3<TV> >&src)
{
    MATH_EXC_ON;
    size_t len = src.len();
    FixedArray<Vec3<TV> > dst(len);

    MatrixVecTask<TV,TM,op_multVecMatrix<TV,TM> > task(mat,src,dst);
    dispatchTask(task,len);

    return dst;
}

template <class T>
static int
removeScaling44(Matrix44<T> &mat, int exc = 1)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::removeScaling(mat, exc);
}

template <class T>
static int
removeScalingAndShear44(Matrix44<T> &mat, int exc = 1)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::removeScalingAndShear(mat, exc);
}

template <class T>
static Matrix44<T>
sansScaling44(const Matrix44<T> &mat, bool exc = true)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::sansScaling(mat, exc);
}

template <class T>
static Matrix44<T>
sansScalingAndShear44(const Matrix44<T> &mat, bool exc = true)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::sansScalingAndShear(mat, exc);
}


template <class T>
static const Matrix44<T> &
scaleSc44(Matrix44<T> &mat, const T &s)
{
    MATH_EXC_ON;
    Vec3<T> sVec(s, s, s);
    return mat.scale(sVec);
}

template <class T>
static const Matrix44<T> &
scaleV44(Matrix44<T> &mat, const Vec3<T> &s)
{
    MATH_EXC_ON;
    return mat.scale(s);
}

template <class T>
static const Matrix44<T> &
scale44Tuple(Matrix44<T> &mat, const tuple &t)
{
    MATH_EXC_ON;
    if(t.attr("__len__")() == 3)
    {
        Vec3<T> s;
        s.x = extract<T>(t[0]);
        s.y = extract<T>(t[1]);
        s.z = extract<T>(t[2]);
        
        return mat.scale(s);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "m.scale needs tuple of length 3");
}

template <class T>
static const Matrix44<T> &
rotateV44(Matrix44<T> &mat, const Vec3<T> &r)
{
    MATH_EXC_ON;
    return mat.rotate(r);
}

template <class T>
static const Matrix44<T> &
rotationMatrix44(Matrix44<T> &mat, const object &fromObj, const object &toObj)
{
    MATH_EXC_ON;
    Vec3<T> from, to;
    if (PyImath::V3<T>::convert (fromObj.ptr(), &from) &&
        PyImath::V3<T>::convert (toObj.ptr(), &to))
    {
        Matrix44<T> rot = IMATH_NAMESPACE::rotationMatrix(from, to);
        return mat.setValue(rot);
    }
    else
    {
        THROW(IEX_NAMESPACE::ArgExc, "m.rotationMatrix expected V3 arguments");
    }   
}

template <class T>
static const Matrix44<T> &
rotationMatrixWithUp44(Matrix44<T> &mat, const object &fromObj, const object &toObj,
                       const object &upObj)
{
    MATH_EXC_ON;
    Vec3<T> from, to, up;
    if (PyImath::V3<T>::convert (fromObj.ptr(), &from) &&
        PyImath::V3<T>::convert (toObj.ptr(), &to) &
        PyImath::V3<T>::convert (upObj.ptr(), &up))
    {
        Matrix44<T> rot = IMATH_NAMESPACE::rotationMatrixWithUpDir(from, to, up);
        return mat.setValue(rot);
    }
    else
    {
        THROW(IEX_NAMESPACE::ArgExc, "m.rotationMatrix expected V3 arguments");
    }   
}

template <class T>
static const Matrix44<T> &
setScaleSc44(Matrix44<T> &mat, const T &s)
{
    MATH_EXC_ON;
    Vec3<T> sVec(s, s, s);
    return mat.setScale(sVec);
}

template <class T>
static const Matrix44<T> &
setScaleV44(Matrix44<T> &mat, const Vec3<T> &s)
{
    MATH_EXC_ON;
    return mat.setScale(s);
}

template <class T>
static const Matrix44<T> &
setScale44Tuple(Matrix44<T> &mat, const tuple &t)
{
    MATH_EXC_ON;
    if(t.attr("__len__")() == 3)
    {
        Vec3<T> s;
        s.x = extract<T>(t[0]);
        s.y = extract<T>(t[1]);
        s.z = extract<T>(t[2]);
        
        return mat.setScale(s);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "m.translate needs tuple of length 3");
}


template <class T>
static const Matrix44<T> &
setShearV44(Matrix44<T> &mat, const Vec3<T> &sVec)
{
    MATH_EXC_ON;
    IMATH_NAMESPACE::Shear6<T> shear(sVec[0], sVec[1], sVec[2], T (0), T (0), T (0));
    return mat.setShear(shear);
}

template <class T>
static const Matrix44<T> &
setShearS44(Matrix44<T> &mat, const Shear6<T> &s)
{
    MATH_EXC_ON;
    return mat.setShear(s);
}

template <class T>
static const Matrix44<T> &
setShear44Tuple(Matrix44<T> &mat, const tuple &t)
{    
    MATH_EXC_ON;
    if(t.attr("__len__")() == 3)
    {
        Vec3<T> s;
        s.x = extract<T>(t[0]);
        s.y = extract<T>(t[1]);
        s.z = extract<T>(t[2]);
        Shear6<T> shear(s);
        
        return mat.setShear(shear);
    }
    else if(t.attr("__len__")() == 6)
    {
        Shear6<T> shear;
        for(int i = 0; i < 6; ++i)
        {
            shear[i] = extract<T>(t[i]);
        }
        
        return mat.setShear(shear);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "m.setShear needs tuple of length 3 or 6");
}

template <class T>
static const Matrix44<T> &
setTranslation44(Matrix44<T> &mat, const Vec3<T> t)
{
    MATH_EXC_ON;
    return mat.setTranslation(t);
}

template <class T>
static const Matrix44<T> &
setTranslation44Tuple(Matrix44<T> &mat, const tuple &t)
{
    MATH_EXC_ON;
    if(t.attr("__len__")() == 3)
    {
        Vec3<T> trans;
        trans.x = extract<T>(t[0]);
        trans.y = extract<T>(t[1]);
        trans.z = extract<T>(t[2]);
        
        return mat.setTranslation(trans);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "m.translate needs tuple of length 3");
}

template <class T>
static const Matrix44<T> &
setTranslation44Obj(Matrix44<T> &mat, const object &o)
{
    MATH_EXC_ON;
    Vec3<T> v;
    if (PyImath::V3<T>::convert (o.ptr(), &v))
    {
        return mat.setTranslation(v);
    }
    else
    {
        THROW(IEX_NAMESPACE::ArgExc, "m.setTranslation expected V3 argument");
        return mat;
    }   
}

template <class T>
static void
setValue44(Matrix44<T> &mat, const Matrix44<T> &value)
{
    MATH_EXC_ON;
    mat.setValue(value);
}

template <class T>
static const Matrix44<T> &
shearV44(Matrix44<T> &mat, const Vec3<T> &s)
{
    MATH_EXC_ON;
    IMATH_NAMESPACE::Shear6<T> shear(s[0], s[1], s[2], T (0), T (0), T (0));
    return mat.shear(shear);
}

template <class T>
static const Matrix44<T> &
shearS44(Matrix44<T> &mat, const Shear6<T> &s)
{
    MATH_EXC_ON;
    return mat.shear(s);
}

template <class T>
static const Matrix44<T> &
shear44Tuple(Matrix44<T> &mat, const tuple &t)
{    
    MATH_EXC_ON;
    if(t.attr("__len__")() == 3)
    {
        Vec3<T> s;
        s.x = extract<T>(t[0]);
        s.y = extract<T>(t[1]);
        s.z = extract<T>(t[2]);
        Shear6<T> shear(s);

        return mat.shear(shear);
    }
    else if(t.attr("__len__")() == 6)
    {
        Shear6<T> shear;
        for(int i = 0; i < 6; ++i)
        {
            shear[i] = extract<T>(t[i]);
        }
        
        return mat.shear(shear);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "m.shear needs tuple of length 3 or 6");
}


template <class T>
static const Matrix44<T> &
translate44(Matrix44<T> &mat, const object &t)
{
    MATH_EXC_ON;
    Vec3<T> v;
    if (PyImath::V3<T>::convert (t.ptr(), &v))
    {
        return mat.translate(v);
    }
    else
    {
        THROW(IEX_NAMESPACE::ArgExc, "m.translate expected V3 argument");
        return mat;
    }   
}
template <class T>
static const Matrix44<T> &
translate44Tuple(Matrix44<T> &mat, const tuple &t)
{
    MATH_EXC_ON;
    if(t.attr("__len__")() == 3)
    {
        Vec3<T> trans;
        trans.x = extract<T>(t[0]);
        trans.y = extract<T>(t[1]);
        trans.z = extract<T>(t[2]);
        
        return mat.translate(trans);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "m.translate needs tuple of length 3");
}

template <class T>
static Matrix44<T>
subtractTL44(Matrix44<T> &mat, T a)
{
    MATH_EXC_ON;
    Matrix44<T> m(mat.x);
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            m.x[i][j] -= a;
    
    return m;
}

template <class T>
static Matrix44<T>
subtractTR44(Matrix44<T> &mat, T a)
{
    MATH_EXC_ON;
    Matrix44<T> m(mat.x);
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            m.x[i][j] = a - m.x[i][j];
    
    return m;
}


template <class T>
static Matrix44<T>
add44T(Matrix44<T> &mat, T a)
{
    MATH_EXC_ON;
    Matrix44<T> m(mat.x);
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            m.x[i][j] += a;
    
    return m;
}

template <class S, class T>
static Matrix44<T>
mul44(Matrix44<T> &mat1, Matrix44<S> &mat2)
{
    MATH_EXC_ON;
    Matrix44<T> mat2T;
    mat2T.setValue (mat2);
    return mat1 * mat2T;
}

template <class S, class T>
static Matrix44<T>
rmul44(Matrix44<T> &mat2, Matrix44<S> &mat1)
{
    MATH_EXC_ON;
    Matrix44<T> mat1T;
    mat1T.setValue (mat1);
    return mat1T * mat2;
}

template <class S, class T>
static const Matrix44<T> &
imul44(Matrix44<T> &mat1, Matrix44<S> &mat2)
{
    MATH_EXC_ON;
    Matrix44<T> mat2T;
    mat2T.setValue (mat2);
    return mat1 *= mat2T;
}

template <class T>
static bool
lessThan44(Matrix44<T> &mat1, const Matrix44<T> &mat2)
{
    for(int i = 0; i < 4; ++i){
        for(int j = 0; j < 4; ++j){
            if(mat1[i][j] > mat2[i][j]){
                return false;
            }
        }
    }
    
    return (mat1 != mat2);
}

template <class T>
static bool
lessThanEqual44(Matrix44<T> &mat1, const Matrix44<T> &mat2)
{
    for(int i = 0; i < 4; ++i){
        for(int j = 0; j < 4; ++j){
            if(mat1[i][j] > mat2[i][j]){
                return false;
            }
        }
    }
    
    return true;
}

template <class T>
static bool
greaterThan44(Matrix44<T> &mat1, const Matrix44<T> &mat2)
{
    for(int i = 0; i < 4; ++i){
        for(int j = 0; j < 4; ++j){
            if(mat1[i][j] < mat2[i][j]){
                return false;
            }
        }
    }
    
    return (mat1 != mat2);
}

template <class T>
static bool
greaterThanEqual44(Matrix44<T> &mat1, const Matrix44<T> &mat2)
{
    for(int i = 0; i < 4; ++i){
        for(int j = 0; j < 4; ++j){
            if(mat1[i][j] < mat2[i][j]){
                return false;
            }
        }
    }
    
    return true;
}

template <class T>
static tuple
singularValueDecomposition44(const Matrix44<T>& m, bool forcePositiveDeterminant = false)
{
    IMATH_NAMESPACE::Matrix44<T> U, V;
    IMATH_NAMESPACE::Vec4<T> S;
    IMATH_NAMESPACE::jacobiSVD (m, U, S, V, IMATH_NAMESPACE::limits<T>::epsilon(), forcePositiveDeterminant);
    return make_tuple (U, S, V);
}

BOOST_PYTHON_FUNCTION_OVERLOADS(invert44_overloads, invert44, 1, 2);
BOOST_PYTHON_FUNCTION_OVERLOADS(inverse44_overloads, inverse44, 1, 2);
BOOST_PYTHON_FUNCTION_OVERLOADS(gjInvert44_overloads, gjInvert44, 1, 2);
BOOST_PYTHON_FUNCTION_OVERLOADS(gjInverse44_overloads, gjInverse44, 1, 2);
BOOST_PYTHON_FUNCTION_OVERLOADS(extractAndRemoveScalingAndShear44_overloads, extractAndRemoveScalingAndShear44, 3, 4)
BOOST_PYTHON_FUNCTION_OVERLOADS(extractSHRT44_overloads, extractSHRT44, 5, 6)
BOOST_PYTHON_FUNCTION_OVERLOADS(extractScaling44_overloads, extractScaling44, 2, 3)
BOOST_PYTHON_FUNCTION_OVERLOADS(extractScalingAndShear44_overloads, extractScalingAndShear44, 3, 4)
BOOST_PYTHON_FUNCTION_OVERLOADS(removeScaling44_overloads, removeScaling44, 1, 2)
BOOST_PYTHON_FUNCTION_OVERLOADS(removeScalingAndShear44_overloads, removeScalingAndShear44, 1, 2)
BOOST_PYTHON_FUNCTION_OVERLOADS(sansScaling44_overloads, sansScaling44, 1, 2)
BOOST_PYTHON_FUNCTION_OVERLOADS(sansScalingAndShear44_overloads, sansScalingAndShear44, 1, 2)

template <class T>
static Matrix44<T> * Matrix4_tuple_constructor(const tuple &t0, const tuple &t1, const tuple &t2, const tuple &t3)
{
  if(t0.attr("__len__")() == 4 && t1.attr("__len__")() == 4 && t2.attr("__len__")() == 4 && t3.attr("__len__")() == 4)
  {
      return new Matrix44<T>(extract<T>(t0[0]),  extract<T>(t0[1]),  extract<T>(t0[2]),  extract<T>(t0[3]),
                             extract<T>(t1[0]),  extract<T>(t1[1]),  extract<T>(t1[2]),  extract<T>(t1[3]),
                             extract<T>(t2[0]),  extract<T>(t2[1]),  extract<T>(t2[2]),  extract<T>(t2[3]),
                             extract<T>(t3[0]),  extract<T>(t3[1]),  extract<T>(t3[2]),  extract<T>(t3[3]));
  }
  else
      THROW(IEX_NAMESPACE::LogicExc, "Matrix44 takes 4 tuples of length 4");
}

template <class T, class S>
static Matrix44<T> *Matrix4_matrix_constructor(const Matrix44<S> &mat)
{
    Matrix44<T> *m = new Matrix44<T>;
    
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            m->x[i][j] = T (mat.x[i][j]);
    
    return m;
}


template <class T>
class_<Matrix44<T> >
register_Matrix44()
{
    typedef PyImath::StaticFixedArray<Matrix44<T>,T,4,IndexAccessMatrixRow<Matrix44<T>,T,4> > Matrix44_helper;

    MatrixRow<T,4>::register_class();

    class_<Matrix44<T> > matrix44_class(Matrix44Name<T>::value, Matrix44Name<T>::value,init<Matrix44<T> >("copy construction"));
    matrix44_class
        .def(init<>("initialize to identity"))
        .def(init<T>("initialize all entries to a single value"))
        .def("__init__", make_constructor(Matrix4_tuple_constructor<T>),"tuple constructor1")
        .def("__init__", make_constructor(Matrix4_matrix_constructor<T,float>))
        .def("__init__", make_constructor(Matrix4_matrix_constructor<T,double>))
        
        .def(init<T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T>("make from components"))
	//.def_readwrite("x00", &Matrix44<T>::x[0][0])
	//.def_readwrite("x01", &Matrix44<T>::x[0][1])
	//.def_readwrite("x02", &Matrix44<T>::x[0][2])
	//.def_readwrite("x03", &Matrix44<T>::x[0][3])
	//.def_readwrite("x10", &Matrix44<T>::x[1][0])
	//.def_readwrite("x11", &Matrix44<T>::x[1][1])
	//.def_readwrite("x12", &Matrix44<T>::x[1][2])
	//.def_readwrite("x13", &Matrix44<T>::x[1][3])
	//.def_readwrite("x20", &Matrix44<T>::x[2][0])
	//.def_readwrite("x21", &Matrix44<T>::x[2][1])
	//.def_readwrite("x22", &Matrix44<T>::x[2][2])
	//.def_readwrite("x23", &Matrix44<T>::x[2][3])
	//.def_readwrite("x30", &Matrix44<T>::x[3][0])
	//.def_readwrite("x31", &Matrix44<T>::x[3][1])
	//.def_readwrite("x32", &Matrix44<T>::x[3][2])
	//.def_readwrite("x33", &Matrix44<T>::x[3][3])
        .def("baseTypeEpsilon", &Matrix44<T>::baseTypeEpsilon,"baseTypeEpsilon() epsilon value of the base type of the vector")
        .staticmethod("baseTypeEpsilon")
        .def("baseTypeMax", &Matrix44<T>::baseTypeMax,"baseTypeMax() max value of the base type of the vector")
        .staticmethod("baseTypeMax")
        .def("baseTypeMin", &Matrix44<T>::baseTypeMin,"baseTypeMin() min value of the base type of the vector")
        .staticmethod("baseTypeMin")
        .def("baseTypeSmallest", &Matrix44<T>::baseTypeSmallest,"baseTypeSmallest() smallest value of the base type of the vector")
        .staticmethod("baseTypeSmallest")
        .def("equalWithAbsError", &Matrix44<T>::equalWithAbsError,
             "m1.equalWithAbsError(m2,e) true if the elements "
             "of v1 and v2 are the same with an absolute error of no more than e, "
             "i.e., abs(m1[i] - m2[i]) <= e")
         
        .def("equalWithRelError", &Matrix44<T>::equalWithRelError,
             "m1.equalWithAbsError(m2,e) true if the elements "
             "of m1 and m2 are the same with an absolute error of no more than e, "
             "i.e., abs(m1[i] - m2[i]) <= e * abs(m1[i])")
         
        // need a different version for matrix data access
        .def("__len__", Matrix44_helper::len)
        .def("__getitem__", Matrix44_helper::getitem)
	//.def("__setitem__", Matrix44_helper::setitem)
        .def("makeIdentity",&Matrix44<T>::makeIdentity,"makeIdentity() make this matrix the identity matrix")
        .def("transpose",&Matrix44<T>::transpose,return_internal_reference<>(),"transpose() transpose this matrix")
        .def("transposed",&Matrix44<T>::transposed,"transposed() return a transposed copy of this matrix")
        .def("minorOf",&Matrix44<T>::minorOf,"minorOf() return matrix minor of the (row,col) element of this matrix")
        .def("fastMinor",&Matrix44<T>::fastMinor,"fastMinor() return matrix minor using the specified rows and columns of this matrix")
        .def("determinant",&Matrix44<T>::determinant,"determinant() return the determinant of this matrix")
        .def("invert",&invert44<T>,invert44_overloads("invert() invert this matrix")[return_internal_reference<>()])
        .def("inverse",&inverse44<T>,inverse44_overloads("inverse() return a inverted copy of this matrix"))
        .def("gjInvert",&gjInvert44<T>,gjInvert44_overloads("gjInvert() invert this matrix")[return_internal_reference<>()])
        .def("gjInverse",&gjInverse44<T>,gjInverse44_overloads("gjInverse() return a inverted copy of this matrix"))
        .def(self == self) // NOSONAR - suppress SonarCloud bug report.
        .def(self != self) // NOSONAR - suppress SonarCloud bug report.
        .def("__iadd__", &iadd44<T, float>,return_internal_reference<>())
        .def("__iadd__", &iadd44<T, double>,return_internal_reference<>())
        .def("__iadd__", &iadd44T<T>,return_internal_reference<>())
        .def("__add__", &add44<T>)
        .def("__isub__", &isub44<T, float>,return_internal_reference<>())
        .def("__isub__", &isub44<T, double>,return_internal_reference<>())
        .def("__isub__", &isub44T<T>,return_internal_reference<>())
        .def("__sub__", &sub44<T>)
        .def("negate",&negate44<T>,return_internal_reference<>(),"negate() negate all entries in this matrix")
        .def("__neg__", &neg44<T>)
        .def("__imul__", &imul44T<T>,return_internal_reference<>())
        .def("__mul__", &mul44T<T>)
        .def("__rmul__", &rmul44T<T>)
        .def("__idiv__", &idiv44T<T>,return_internal_reference<>())
        .def("__itruediv__", &idiv44T<T>,return_internal_reference<>())
        .def("__div__", &div44T<T>)
        .def("__truediv__", &div44T<T>)
        .def("__add__", &add44T<T>)
        .def("__radd__", &add44T<T>)
        .def("__sub__", &subtractTL44<T>)
        .def("__rsub__", &subtractTR44<T>)
        .def("__mul__", &mul44<float, T>)
        .def("__mul__", &mul44<double, T>)
        .def("__rmul__", &rmul44<float, T>)
        .def("__rmul__", &rmul44<double, T>)
        .def("__imul__", &imul44<float, T>,return_internal_reference<>())
        .def("__imul__", &imul44<double, T>,return_internal_reference<>())
        .def("__lt__", &lessThan44<T>)
        .def("__gt__", &greaterThan44<T>)
        .def("__le__", &lessThanEqual44<T>)
        .def("__ge__", &greaterThanEqual44<T>)
	//.def(self_ns::str(self))
	    .def("__repr__",&Matrix44_repr<T>)
    
        .def("extractAndRemoveScalingAndShear", &extractAndRemoveScalingAndShear44<T>, 
             extractAndRemoveScalingAndShear44_overloads(				
             "M.extractAndRemoveScalingAndShear(scl, shr, "
             "[exc]) -- extracts the scaling component of "
             "M into scl and the shearing component of M "
             "into shr.  Also removes the scaling and "
             "shearing components from M.  "
             "Returns 1 unless the scaling component is "
             "nearly 0, in which case 0 is returned. "
             "If optional arg. exc == 1, then if the "
             "scaling component is nearly 0, then MathExc "
             "is thrown."))
             
        .def("extractEulerXYZ", &extractEulerXYZ<T>, "extract Euler")          
        .def("extractEulerZYX", &extractEulerZYX<T>, "extract Euler")
          
        .def("extractSHRT", &extractSHRT44<T>, extractSHRT44_overloads(
             "M.extractSHRT(Vs, Vh, Vr, Vt, [exc]) -- "
	         "extracts the scaling component of M into Vs, "
			 "the shearing component of M in Vh (as XY, "
	         "XZ, YZ shear factors), the rotation of M "
	         "into Vr (as Euler angles in the order XYZ), "
	         "and the translaation of M into Vt. "
			 "If optional arg. exc == 1, then if the "
             "scaling component is nearly 0, then MathExc "
             "is thrown. "))
                
        .def("extractScaling", &extractScaling44<T>, extractScaling44_overloads("extract scaling"))
        .def("extractScalingAndShear", &extractScalingAndShear44<T>, extractScalingAndShear44_overloads("extract scaling"))
        .def("singularValueDecomposition", &singularValueDecomposition44<T>, 
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
             "negative scale but S[3] might be negative.  \n"
             "\n"
             "Our SVD implementation uses two-sided Jacobi rotations to iteratively\n"
             "diagonalize the matrix, which should be quite robust and significantly faster\n"
             "than the more general SVD solver in LAPACK.  \n",
             args("matrix", "forcePositiveDeterminant"))
        .def("symmetricEigensolve", &PyImath::jacobiEigensolve<IMATH_NAMESPACE::Matrix44<T> >, 
             "Decomposes the matrix A using a symmetric eigensolver into matrices Q and S \n"
             "which have the following properties: \n"
             "  1. Q is the orthonormal matrix of eigenvectors, \n"
             "  2. S is the diagonal matrix of eigenvalues, \n"
             "  3. Q.transposed() * S * Q gives back the original matrix.\n"
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
        .def("multDirMatrix", &multDirMatrix44<double,T>, "mult matrix")
        .def("multDirMatrix", &multDirMatrix44_return_value<double,T>, "mult matrix")
        .def("multDirMatrix", &multDirMatrix44_array<double,T>, "mult matrix")
        .def("multDirMatrix", &multDirMatrix44<float,T>, "mult matrix")
        .def("multDirMatrix", &multDirMatrix44_return_value<float,T>, "mult matrix")
        .def("multDirMatrix", &multDirMatrix44_array<float,T>, "mult matrix")
        .def("multVecMatrix", &multVecMatrix44<double,T>, "mult matrix")
        .def("multVecMatrix", &multVecMatrix44_return_value<double,T>, "mult matrix")
        .def("multVecMatrix", &multVecMatrix44_array<double,T>, "mult matrix")
        .def("multVecMatrix", &multVecMatrix44<float,T>, "mult matrix")
        .def("multVecMatrix", &multVecMatrix44_return_value<float,T>, "mult matrix")
        .def("multVecMatrix", &multVecMatrix44_array<float,T>, "mult matrix")
        .def("removeScaling", &removeScaling44<T>, removeScaling44_overloads("remove scaling"))
        .def("removeScalingAndShear", &removeScalingAndShear44<T>, removeScalingAndShear44_overloads("remove scaling"))
        .def("sansScaling", &sansScaling44<T>, sansScaling44_overloads("sans scaling"))
        .def("sansScalingAndShear", &sansScalingAndShear44<T>, sansScalingAndShear44_overloads("sans scaling and shear"))
        .def("scale", &scaleSc44<T>, return_internal_reference<>(), "scale matrix")
        .def("scale", &scaleV44<T>, return_internal_reference<>(), "scale matrix")
        .def("scale", &scale44Tuple<T>, return_internal_reference<>(), "scale matrix")
        .def("rotate", &rotateV44<T>, return_internal_reference<>(), "rotate matrix")
        .def("rotationMatrix", &rotationMatrix44<T>, return_internal_reference<>(), "rotationMatrix()")
        .def("rotationMatrixWithUpDir", &rotationMatrixWithUp44<T>, return_internal_reference<>(), "roationMatrixWithUp()")
        .def("setScale", &setScaleSc44<T>, return_internal_reference<>(),"setScale()")
        .def("setScale", &setScaleV44<T>, return_internal_reference<>(),"setScale()")
        .def("setScale", &setScale44Tuple<T>, return_internal_reference<>(),"setScale()")

        .def("setShear", &setShearV44<T>, return_internal_reference<>(),"setShear()")
        .def("setShear", &setShearS44<T>, return_internal_reference<>(),"setShear()")
        .def("setShear", &setShear44Tuple<T>, return_internal_reference<>(),"setShear()")
        .def("setTranslation", &setTranslation44<T>, return_internal_reference<>(),"setTranslation()")
        .def("setTranslation", &setTranslation44Tuple<T>, return_internal_reference<>(),"setTranslation()")
        .def("setTranslation", &setTranslation44Obj<T>, return_internal_reference<>(),"setTranslation()")
        .def("setValue", &setValue44<T>, "setValue()")
        .def("shear", &shearV44<T>, return_internal_reference<>(),"shear()")
        .def("shear", &shearS44<T>, return_internal_reference<>(),"shear()")
        .def("shear", &shear44Tuple<T>, return_internal_reference<>(),"shear()")
        .def("translate", &translate44<T>, return_internal_reference<>(),"translate()")
        .def("translate", &translate44Tuple<T>, return_internal_reference<>(),"translate()")
        .def("translation", &Matrix44<T>::translation, "translation()")

        ;

    decoratecopy(matrix44_class);

    return matrix44_class;
/*
    const Matrix44 &	operator = (const Matrix44 &v);
    const Matrix44 &	operator = (T a);
    T *			getValue ();
    const T *		getValue () const;
    template <class S> void getValue (Matrix44<S> &v) const;
    template <class S> Matrix44 & setValue (const Matrix44<S> &v);
    template <class S> Matrix44 & setTheMatrix (const Matrix44<S> &v);
    template <class S> void multVecMatrix(const Vec2<S> &src, Vec2<S> &dst) const;
    template <class S> void multDirMatrix(const Vec2<S> &src, Vec2<S> &dst) const;
    template <class S> const Matrix44 &	setRotation (S r);
    template <class S> const Matrix44 &	rotate (S r);
    const Matrix44 &	setScale (T s);
    template <class S> const Matrix44 &	setScale (const Vec2<S> &s);
    template <class S> const Matrix44 &	scale (const Vec2<S> &s);
    template <class S> const Matrix44 &	setTranslation (const Vec2<S> &t);
    Vec2<T>		translation () const;
    template <class S> const Matrix44 &	translate (const Vec2<S> &t);
    template <class S> const Matrix44 &	setShear (const S &h);
    template <class S> const Matrix44 &	setShear (const Vec2<S> &h);
    template <class S> const Matrix44 &	shear (const S &xy);
    template <class S> const Matrix44 &	shear (const Vec2<S> &h);
*/
}


template <class T>
static void
setM44ArrayItem(FixedArray<IMATH_NAMESPACE::Matrix44<T> > &ma,
                Py_ssize_t index,
                const IMATH_NAMESPACE::Matrix44<T> &m)
{
    ma[ma.canonical_index(index)] = m;
}

template <class T>
class_<FixedArray<IMATH_NAMESPACE::Matrix44<T> > >
register_M44Array()
{
    class_<FixedArray<IMATH_NAMESPACE::Matrix44<T> > > matrixArray_class = FixedArray<IMATH_NAMESPACE::Matrix44<T> >::register_("Fixed length array of IMATH_NAMESPACE::Matrix44");
    matrixArray_class
         .def("__setitem__", &setM44ArrayItem<T>)
        ;
    return matrixArray_class;
}


template PYIMATH_EXPORT class_<IMATH_NAMESPACE::Matrix44<float> > register_Matrix44<float>();
template PYIMATH_EXPORT class_<IMATH_NAMESPACE::Matrix44<double> > register_Matrix44<double>();

template PYIMATH_EXPORT class_<FixedArray<IMATH_NAMESPACE::Matrix44<float> > > register_M44Array<float>();
template PYIMATH_EXPORT class_<FixedArray<IMATH_NAMESPACE::Matrix44<double> > > register_M44Array<double>();

template<> PYIMATH_EXPORT IMATH_NAMESPACE::Matrix44<float> FixedArrayDefaultValue<IMATH_NAMESPACE::Matrix44<float> >::value() { return IMATH_NAMESPACE::Matrix44<float>(); }
template<> PYIMATH_EXPORT IMATH_NAMESPACE::Matrix44<double> FixedArrayDefaultValue<IMATH_NAMESPACE::Matrix44<double> >::value() { return IMATH_NAMESPACE::Matrix44<double>(); }
}
