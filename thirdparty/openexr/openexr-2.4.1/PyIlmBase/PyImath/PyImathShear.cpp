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


#include "PyImathShear.h"

#include "PyImathPlane.h"
#include "PyImathDecorators.h"
#include "PyImathExport.h"
#include <Python.h>
#include <boost/python.hpp>
#include <boost/python/make_constructor.hpp>
#include <boost/format.hpp>
#include "PyImath.h"
#include "PyImathMathExc.h"
#include <Iex.h>


namespace PyImath{
using namespace boost::python;
using namespace IMATH_NAMESPACE;

template <class T> struct ShearName {static const char *value;};
template <> const char *ShearName<float>::value = "Shear6f";
template <> const char *ShearName<double>::value = "Shear6d";

template <class T>
static std::string Shear_str(const Shear6<T> &v)
{
    std::stringstream stream;
    stream << ShearName<T>::value << "(" 
           << v[0] << ", " << v[1] << ", " << v[2] << ", "
           << v[3] << ", " << v[4] << ", " << v[5] << ")";
    return stream.str();
}

// Non-specialized repr is same as str
template <class T>
static std::string Shear_repr(const Shear6<T> &v)
{
    return Shear_str(v);
}

// Specialization for float to full precision
template <>
std::string Shear_repr(const Shear6<float> &v)
{
    return (boost::format("%s(%.9g, %.9g, %.9g, %.9g, %.9g, %.9g)")
                        % ShearName<float>::value
                        % v[0] % v[1] % v[2]
                        % v[3] % v[4] % v[5]).str();
}

// Specialization for double to full precision
template <>
std::string Shear_repr(const Shear6<double> &v)
{
    return (boost::format("%s(%.17g, %.17g, %.17g, %.17g, %.17g, %.17g)")
                        % ShearName<double>::value
                        % v[0] % v[1] % v[2]
                        % v[3] % v[4] % v[5]).str();
}

template <class T>
static Shear6<T> * shearTupleConstructor(tuple t)
{
    if(t.attr("__len__")() == 3){
        return new Shear6<T>(extract<T>(t[0]), extract<T>(t[1]), extract<T>(t[2]),
                             T(0), T(0), T(0));
    }
    else if(t.attr("__len__")() == 6){
        return new Shear6<T>(extract<T>(t[0]), extract<T>(t[1]), extract<T>(t[2]),
                             extract<T>(t[3]), extract<T>(t[4]), extract<T>(t[5]));        
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Shear6 expects tuple of length 3 or 6");
}

template <class T>
static Shear6<T> * shearConstructor1(T a)
{
    return new Shear6<T>(a, a, a, a, a, a);
}

template <class T, class S>
static Shear6<T> * shearConversionConstructor(const Shear6<S> &shear)
{
    Shear6<T> *s = new Shear6<T>;
    *s = shear;
    return s;
}

template <class T>
static const Shear6<T> &
iadd(Shear6<T> &shear, const Shear6<T> &other)
{
    MATH_EXC_ON;
    return shear += other;
}

template <class T>
static Shear6<T>
add(const Shear6<T> &shear, const Shear6<T> &other)
{
    MATH_EXC_ON;
    return shear + other;
}

template <class T>
static const Shear6<T> &
isub(Shear6<T> &shear, const Shear6<T> &other)
{
    MATH_EXC_ON;
    return shear -= other;
}

template <class T>
static Shear6<T>
sub(const Shear6<T> &shear, const Shear6<T> &other)
{
    MATH_EXC_ON;
    return shear - other;
}

template <class T>
static Shear6<T>
neg(const Shear6<T> &shear)
{
    MATH_EXC_ON;
    return -shear;
}

template <class T>
static const Shear6<T> &
imul(Shear6<T> &shear, const Shear6<T> &other)
{
    MATH_EXC_ON;
    return shear *= other;
}

template <class T>
static const Shear6<T> &
imulT(Shear6<T> &shear, T t)
{
    MATH_EXC_ON;
    return shear *= t;
}

template <class T>
static Shear6<T>
mul(const Shear6<T> &shear, const Shear6<T> &other)
{
    MATH_EXC_ON;
    return shear * other;
}

template <class T>
static Shear6<T>
mulT(const Shear6<T> &shear, T t)
{
    MATH_EXC_ON;
    return shear * t;
}

template <class T>
static const Shear6<T> &
idiv(Shear6<T> &shear, const Shear6<T> &other)
{
    MATH_EXC_ON;
    return shear /= other;
}

template <class T>
static const Shear6<T> &
idivT(Shear6<T> &shear, T t)
{
    MATH_EXC_ON;
    return shear /= t;
}

template <class T>
static Shear6<T>
div(const Shear6<T> &shear, const Shear6<T> &other)
{
    MATH_EXC_ON;
    return shear / other;
}

template <class T>
static Shear6<T>
divT(const Shear6<T> &shear, T t)
{
    MATH_EXC_ON;
    return shear / t;
}

template <class T>
static Shear6<T>
subtract1(Shear6<T> &v, tuple t)
{
    MATH_EXC_ON;
    Shear6<T> w;
    
    if(t.attr("__len__")() == 6){
        w[0] = v[0] - extract<T>(t[0]);
        w[1] = v[1] - extract<T>(t[1]);   
        w[2] = v[2] - extract<T>(t[2]);
        w[3] = v[3] - extract<T>(t[3]);
        w[4] = v[4] - extract<T>(t[4]);   
        w[5] = v[5] - extract<T>(t[5]);
    }        
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 6");
    
    return w;
}

template <class T>
static Shear6<T>
subtract2(Shear6<T> &v, tuple t)
{
    MATH_EXC_ON;
    Shear6<T> w;
    
    if(t.attr("__len__")() == 6){
        w[0] = extract<T>(t[0]) - v[0];
        w[1] = extract<T>(t[1]) - v[1];   
        w[2] = extract<T>(t[2]) - v[2];
        w[3] = extract<T>(t[3]) - v[3];
        w[4] = extract<T>(t[4]) - v[4];   
        w[5] = extract<T>(t[5]) - v[5];
    }        
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 6");
    
    return w;
}

template <class T>
static Shear6<T>
subtractT1(Shear6<T> &v, T a)
{
    MATH_EXC_ON;
    Shear6<T> w;
    
    w[0] = v[0] - a;
    w[1] = v[1] - a;   
    w[2] = v[2] - a;
    w[3] = v[3] - a;
    w[4] = v[4] - a;   
    w[5] = v[5] - a;

    return w;
}

template <class T>
static Shear6<T>
subtractT2(Shear6<T> &v, T a)
{
    MATH_EXC_ON;
    Shear6<T> w;

     w[0] = a - v[0];
     w[1] = a - v[1];   
     w[2] = a - v[2];
     w[3] = a - v[3];
     w[4] = a - v[4];   
     w[5] = a - v[5];

    return w;
}


template <class T>
static Shear6<T>
addTuple(Shear6<T> &v, tuple t)
{
    MATH_EXC_ON;
    Shear6<T> w;
    
    if(t.attr("__len__")() == 6){
        w[0] = v[0] + extract<T>(t[0]);
        w[1] = v[1] + extract<T>(t[1]);   
        w[2] = v[2] + extract<T>(t[2]);
        w[3] = v[3] + extract<T>(t[3]);
        w[4] = v[4] + extract<T>(t[4]);   
        w[5] = v[5] + extract<T>(t[5]);
    }        
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 6");
    
    return w;
}

template <class T>
static Shear6<T>
addT(Shear6<T> &v, T a)
{
    MATH_EXC_ON;
    Shear6<T> w;

     w[0] = v[0] + a;
     w[1] = v[1] + a;   
     w[2] = v[2] + a;
     w[3] = v[3] + a;
     w[4] = v[4] + a;   
     w[5] = v[5] + a;
    
    return w;
}

template <class T>
static Shear6<T>
multTuple(Shear6<T> &v, tuple t)
{
    MATH_EXC_ON;
    Shear6<T> w;
    
    if(t.attr("__len__")() == 6){
        w[0] = v[0] * extract<T>(t[0]);
        w[1] = v[1] * extract<T>(t[1]);   
        w[2] = v[2] * extract<T>(t[2]);
        w[3] = v[3] * extract<T>(t[3]);
        w[4] = v[4] * extract<T>(t[4]);   
        w[5] = v[5] * extract<T>(t[5]);
    }        
    else
        THROW(IEX_NAMESPACE::LogicExc, "tuple must have length of 6");
    
    return w;
}

template <class T>
static Shear6<T>
rdiv(Shear6<T> &v, T a)
{
    MATH_EXC_ON;
    Shear6<T> w;
    
    if(v != Shear6<T>()){
        w[0] = a/v[0];
        w[1] = a/v[1];
        w[2] = a/v[2];
        w[3] = a/v[3];
        w[4] = a/v[4];
        w[5] = a/v[5];
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Division by Zero");
    
    return w;
}

template <class T>
static Shear6<T>
divTuple(Shear6<T> &v, const tuple &t)
{
    MATH_EXC_ON;
    if(t.attr("__len__")() != 6)
        THROW(IEX_NAMESPACE::LogicExc, "Shear6 expects tuple of length 6");
    
    Shear6<T> w;
    for(int i = 0; i < 6; ++i)
    {
        T a = extract<T>(t[i]);
        if(a != T (0))
            w[i] = v[i] / a;
        else
            THROW(IEX_NAMESPACE::LogicExc, "Division by Zero"); 
    }
    
    return w;
}

template <class T>
static Shear6<T>
rdivTuple(Shear6<T> &v, const tuple &t)
{
    MATH_EXC_ON;
    if(t.attr("__len__")() != 6)
        THROW(IEX_NAMESPACE::LogicExc, "Shear6 expects tuple of length 6");
    
    Shear6<T> w;
    for(int i = 0; i < 6; ++i)
    {
        T a = extract<T>(t[i]);
        if(v[i] != T (0))
            w[i] = a / v[i];
        else
            THROW(IEX_NAMESPACE::LogicExc, "Division by Zero"); 
    }
    
    return w;
}

template <class T>
static bool
lessThan(Shear6<T> &v, const Shear6<T> &w)
{
    bool isLessThan = (v[0] <= w[0] && v[1] <= w[1] && v[2] <= w[2] 
                    && v[3] <= w[3] && v[4] <= w[4] && v[5] <= w[5])
                    && v != w;
                   
   return isLessThan;
}

template <class T>
static bool
greaterThan(Shear6<T> &v, const Shear6<T> &w)
{
    bool isGreaterThan = (v[0] >= w[0] && v[1] >= w[1] && v[2] >= w[2] 
                       && v[3] >= w[3] && v[4] >= w[4] && v[5] >= w[5])
                       && v != w;
                   
   return isGreaterThan;
}

template <class T>
static bool
lessThanEqual(Shear6<T> &v, const Shear6<T> &w)
{
    bool isLessThanEqual = (v[0] <= w[0] && v[1] <= w[1] && v[2] <= w[2] 
                         && v[3] <= w[3] && v[4] <= w[4] && v[5] <= w[5]);
                   
   return isLessThanEqual;
}

template <class T>
static bool
greaterThanEqual(Shear6<T> &v, const Shear6<T> &w)
{
    bool isGreaterThanEqual = (v[0] >= w[0] && v[1] >= w[1] && v[2] >= w[2] 
                            && v[3] >= w[3] && v[4] >= w[4] && v[5] >= w[5]);
                   
   return isGreaterThanEqual;
}

template <class T>
static T
getitem(Shear6<T> &shear, int i)
{
    return shear[i];
}

template <class T>
static void
setitem(Shear6<T> &shear, int i, T a)
{
    if(i < 0 || i > 5)
        THROW(IEX_NAMESPACE::LogicExc, "Index out of range");
    
    shear[i] = a;
}

template <class T>
static int
len(Shear6<T> &shear)
{
    return 6;
}



template <class T>
class_<Shear6<T> >
register_Shear()
{
    const char *name = ShearName<T>::value;
    
    void (IMATH_NAMESPACE::Shear6<T>::*setValue1)(T,T,T,T,T,T) = &IMATH_NAMESPACE::Shear6<T>::setValue;
    void (IMATH_NAMESPACE::Shear6<T>::*setValue2)(const Shear6<T> &) = &IMATH_NAMESPACE::Shear6<T>::setValue;
    void (IMATH_NAMESPACE::Shear6<T>::*getValue1)(Shear6<T> &) const = &IMATH_NAMESPACE::Shear6<T>::getValue;
    
    class_<Shear6<T> > shear_class(name, name, init<Shear6<T> >("copy construction"));
    shear_class
        .def(init<>("default construction: (0 0 0 0 0 0)"))
        .def(init<T,T,T>("Shear(XY,XZ,YZ) construction: (XY XZ YZ 0 0 0)"))
        .def(init<const Vec3<float> &>("Shear(v) construction: (v.x v.y v.z 0 0 0)"))
        .def(init<const Vec3<double> &>("Shear(v) construction: (v.x v.y v.z 0 0 0)"))
        .def(init<const Vec3<int> &>("Shear(v) construction: (v.x v.y v.z 0 0 0)"))
        .def(init<T,T,T,T,T,T>("Shear(XY, XZ, YZ, YX, ZX, ZY) construction"))
        .def("__init__", make_constructor(shearConstructor1<T>))
        .def("__init__", make_constructor(shearTupleConstructor<T>),"Construction from tuple")
        .def("__init__", make_constructor(shearConversionConstructor<T,float>))
        .def("__init__", make_constructor(shearConversionConstructor<T,double>))
        .def("__init__", make_constructor(shearConversionConstructor<T,int>))
        .def("__iadd__",&iadd<T>,return_internal_reference<>())
        .def("__add__",&add<T>)
        .def("__isub__",&isub<T>,return_internal_reference<>())
        .def("__sub__",&sub<T>)
        .def("__neg__",&neg<T>)
        .def("__imul__",&imul<T>,return_internal_reference<>())
        .def("__imul__",&imulT<T>,return_internal_reference<>())
        .def("__mul__",&mul<T>)
        .def("__mul__",&mulT<T>)
        .def("__rmul__",&mulT<T>)
        .def("__idiv__",&idiv<T>,return_internal_reference<>())
        .def("__idiv__",&idivT<T>,return_internal_reference<>())
        .def("__itruediv__",&idiv<T>,return_internal_reference<>())
        .def("__itruediv__",&idivT<T>,return_internal_reference<>())
        .def("__div__",&div<T>)
        .def("__div__",&divT<T>)
        .def("__truediv__",&div<T>)
        .def("__truediv__",&divT<T>)
        .def(self == self) // NOSONAR - suppress SonarCloud bug report.
        .def(self != self) // NOSONAR - suppress SonarCloud bug report.
        .def("__str__",&Shear_str<T>)
        .def("__repr__",&Shear_repr<T>)
        .def("setValue", setValue1)
        .def("setValue", setValue2)
        .def("getValue", getValue1)
        .def("negate", &Shear6<T>::negate, return_internal_reference<>())
        .def("baseTypeMin", &Shear6<T>::baseTypeMin)
        .staticmethod("baseTypeMin")
        .def("baseTypeMax", &Shear6<T>::baseTypeMax)
        .staticmethod("baseTypeMax")
        .def("baseTypeSmallest", &Shear6<T>::baseTypeSmallest)
        .staticmethod("baseTypeSmallest")
        .def("baseTypeEpsilon", &Shear6<T>::baseTypeEpsilon)
        .staticmethod("baseTypeEpsilon")
        .def("equalWithAbsError", &Shear6<T>::equalWithAbsError)
        .def("equalWithRelError", &Shear6<T>::equalWithRelError)
        .def("__sub__", &subtract1<T>)
        .def("__sub__", &subtractT1<T>)
        .def("__rsub__", &subtract2<T>)
        .def("__rsub__", &subtractT2<T>)
        .def("__add__", &addTuple<T>)
        .def("__add__", &addT<T>)
        .def("__radd__", &addTuple<T>)
        .def("__radd__", &addT<T>)
        .def("__mul__", &multTuple<T>)
        .def("__rmul__", &multTuple<T>)
        .def("__div__", &divTuple<T>)
        .def("__truediv__", &divTuple<T>)
        .def("__rdiv__", &rdiv<T>)
        .def("__rdiv__", &rdivTuple<T>)
        .def("__lt__", &lessThan<T>)
        .def("__gt__", &greaterThan<T>)
        .def("__le__", &lessThanEqual<T>)
        .def("__ge__", &greaterThanEqual<T>)
        .def("__getitem__", &getitem<T>)
        .def("__setitem__", &setitem<T>)
        .def("__len__", &len<T>)
        ;

    decoratecopy(shear_class);

    return shear_class;
}

template PYIMATH_EXPORT class_<Shear6<float> > register_Shear();
template PYIMATH_EXPORT class_<Shear6<double> > register_Shear();

}//namespace PyIMath
