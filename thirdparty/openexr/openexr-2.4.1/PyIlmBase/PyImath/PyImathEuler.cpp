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

#include "PyImathEuler.h"
#include "PyImathDecorators.h"
#include "PyImathExport.h"
#include <Python.h>
#include <boost/python.hpp>
#include <boost/python/make_constructor.hpp>
#include <boost/format.hpp>
#include "PyImath.h"
#include "PyImathMathExc.h"
#include <ImathVec.h>
#include <Iex.h>
#include "PyImathOperators.h"

// XXX incomplete array wrapping, docstrings missing

namespace PyImath {
template<> const char *PyImath::EulerfArray::name() { return "EulerfArray"; }
template<> const char *PyImath::EulerdArray::name() { return "EulerdArray"; }
}

namespace PyImath {
using namespace boost::python;
using namespace IMATH_NAMESPACE;

template <class T> struct EulerName { static const char *value; };
template<> const char *EulerName<float>::value  = "Eulerf";
template<> const char *EulerName<double>::value = "Eulerd";

template <class T>
static std::string nameOfOrder(typename IMATH_NAMESPACE::Euler<T>::Order order)
{
    switch(order)
    {
        case IMATH_NAMESPACE::Euler<T>::XYZ:
            return "EULER_XYZ";
        case IMATH_NAMESPACE::Euler<T>::XZY:
            return "EULER_XZY";
        case IMATH_NAMESPACE::Euler<T>::YZX:
            return "EULER_YZX";
        case IMATH_NAMESPACE::Euler<T>::YXZ:
            return "EULER_YXZ";
        case IMATH_NAMESPACE::Euler<T>::ZXY:
            return "EULER_ZXY";
        case IMATH_NAMESPACE::Euler<T>::ZYX:
            return "EULER_ZYX";
        case IMATH_NAMESPACE::Euler<T>::XZX:
            return "EULER_XZX";
        case IMATH_NAMESPACE::Euler<T>::XYX:
            return "EULER_XYX";
        case IMATH_NAMESPACE::Euler<T>::YXY:
            return "EULER_YXY";
        case IMATH_NAMESPACE::Euler<T>::YZY:
            return "EULER_YZY";
        case IMATH_NAMESPACE::Euler<T>::ZYZ:
            return "EULER_ZYZ";
        case IMATH_NAMESPACE::Euler<T>::ZXZ:
            return "EULER_ZXZ";
        case IMATH_NAMESPACE::Euler<T>::XYZr:
            return "EULER_XYZr";
        case IMATH_NAMESPACE::Euler<T>::XZYr:
            return "EULER_XZYr";
        case IMATH_NAMESPACE::Euler<T>::YZXr:
            return "EULER_YZXr";
        case IMATH_NAMESPACE::Euler<T>::YXZr:
            return "EULER_YXZr";
        case IMATH_NAMESPACE::Euler<T>::ZXYr:
            return "EULER_ZXYr";
        case IMATH_NAMESPACE::Euler<T>::ZYXr:
            return "EULER_ZYXr";
        case IMATH_NAMESPACE::Euler<T>::XZXr:
            return "EULER_XZXr";
        case IMATH_NAMESPACE::Euler<T>::XYXr:
            return "EULER_XYXr";
        case IMATH_NAMESPACE::Euler<T>::YXYr:
            return "EULER_YXYr";
        case IMATH_NAMESPACE::Euler<T>::YZYr:
            return "EULER_YZYr";
        case IMATH_NAMESPACE::Euler<T>::ZYZr:
            return "EULER_ZYZr";
        case IMATH_NAMESPACE::Euler<T>::ZXZr:
            return "EULER_ZXZr";
        default:
            break;
    }
    
    return "";
}

template <class T>
static std::string Euler_str(const Euler<T> &e)
{
    std::stringstream stream;
    stream << EulerName<T>::value << "(" << e.x << ", " << e.y << ", " << e.z << ", " 
           << nameOfOrder<T> (e.order()) << ")";
    return stream.str();
}

// Non-specialized repr is same as str
template <class T>
static std::string Euler_repr(const Euler<T> &e)
{
    return Euler_str(e);
}

// Specialization for float to full precision
template <>
std::string Euler_repr(const Euler<float> &e)
{
    return (boost::format("%s(%.9g, %.9g, %.9g, %s)")
                        % EulerName<float>::value
                        % e.x % e.y % e.z
                        % nameOfOrder<float>(e.order()).c_str()).str();
}

// Specialization for double to full precision
template <>
std::string Euler_repr(const Euler<double> &e)
{
    return (boost::format("%s(%.17g, %.17g, %.17g, %s)")
                        % EulerName<double>::value
                        % e.x % e.y % e.z
                        % nameOfOrder<double>(e.order()).c_str()).str();
}


template <class T>
static bool
equal(const Euler<T> &e0, const Euler<T> &e1)
{
    if(e0.x == e1.x && e0.y == e1.y && e0.z == e1.z && (e0.order())==(e1.order()))
        return true;
    else
        return false;
}

template <class T>
static bool
notequal(const Euler<T> &e0, const Euler<T> &e1)
{
    if(e0.x != e1.x || e0.y != e1.y || e0.z != e1.z || (e0.order()) != (e1.order()))
    {
        return true;
    }
    else
        return false;
}

template <class T>
static IMATH_NAMESPACE::Vec3 <int> getAngleOrder(Euler <T> &euler)
{
    int i, j, k;
    euler.angleOrder(i, j, k);
    return IMATH_NAMESPACE::Vec3 <int> (i, j, k);
}

template <class T>
static void
setXYZTuple(Euler<T> &euler, const tuple &t)
{
    MATH_EXC_ON;
    Vec3<T> v;
    if(t.attr("__len__")() == 3)
    {
        v.x = extract<T>(t[0]);
        v.y = extract<T>(t[1]);
        v.z = extract<T>(t[2]); 
        
        euler.setXYZVector(v);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Color3 expects tuple of length 3");    
}

// needed to convert Eulerf::Order to Euler<T>::Order
template <class T>
static typename Euler<T>::Order interpretOrder(typename IMATH_NAMESPACE::Eulerf::Order order)
{
    typename Euler<T>::Order o = Euler<T>::XYZ;
    switch(order)
    {
        case IMATH_NAMESPACE::Eulerf::XYZ:
        {
            o = Euler<T>::XYZ;
        }break;
        case IMATH_NAMESPACE::Eulerf::XZY:
        {
            o = Euler<T>::XZY;
        }break;
        case IMATH_NAMESPACE::Eulerf::YZX:
        {
            o = Euler<T>::YZX;
        }break;
        case IMATH_NAMESPACE::Eulerf::YXZ:
        {
            o = Euler<T>::YXZ;
        }break;
        case IMATH_NAMESPACE::Eulerf::ZXY:
        {
            o = Euler<T>::ZXY;
        }break;
        case IMATH_NAMESPACE::Eulerf::ZYX:
        {
            o = Euler<T>::ZYX;
        }break;
        case IMATH_NAMESPACE::Eulerf::XZX:
        {
            o = Euler<T>::XZX;
        }break;
        case IMATH_NAMESPACE::Eulerf::XYX:
        {
            o = Euler<T>::XYX;
        }break;
        case IMATH_NAMESPACE::Eulerf::YXY:
        {
            o = Euler<T>::YXY;
        }break;
        case IMATH_NAMESPACE::Eulerf::YZY:
        {
            o = Euler<T>::YZY;
        }break;
        case IMATH_NAMESPACE::Eulerf::ZYZ:
        {
            o = Euler<T>::ZYZ;
        }break;
        case IMATH_NAMESPACE::Eulerf::ZXZ:
        {
            o = Euler<T>::ZXZ;
        }break;
        case IMATH_NAMESPACE::Eulerf::XYZr:
        {
            o = Euler<T>::XYZr;
        }break;
        case IMATH_NAMESPACE::Eulerf::XZYr:
        {
            o = Euler<T>::XZYr;
        }break;
        case IMATH_NAMESPACE::Eulerf::YZXr:
        {
            o = Euler<T>::YZXr;
        }break;
        case IMATH_NAMESPACE::Eulerf::YXZr:
        {
            o = Euler<T>::YXZr;
        }break;
        case IMATH_NAMESPACE::Eulerf::ZXYr:
        {
            o = Euler<T>::ZXYr;
        }break;
        case IMATH_NAMESPACE::Eulerf::ZYXr:
        {
            o = Euler<T>::ZYXr;
        }break;
        case IMATH_NAMESPACE::Eulerf::XZXr:
        {
            o = Euler<T>::XZXr;
        }break;
        case IMATH_NAMESPACE::Eulerf::XYXr:
        {
            o = Euler<T>::XYXr;
        }break;
        case IMATH_NAMESPACE::Eulerf::YXYr:
        {
            o = Euler<T>::YXYr;
        }break;
        case IMATH_NAMESPACE::Eulerf::YZYr:
        {
            o = Euler<T>::YZYr;
        }break;
        case IMATH_NAMESPACE::Eulerf::ZYZr:
        {
            o = Euler<T>::ZYZr;
        }break;
        case IMATH_NAMESPACE::Eulerf::ZXZr:
        {
            o = Euler<T>::ZXZr;
        }break;            
        default:
            break;
    }
    
    return o;
}

// needed to convert Eulerf::Axis to Euler<T>::Axis
template <class T>
static typename Euler<T>::Axis interpretAxis(typename IMATH_NAMESPACE::Eulerf::Axis axis)
{
    if (axis == IMATH_NAMESPACE::Eulerf::X)
        return Euler<T>::X;
    else if (axis == IMATH_NAMESPACE::Eulerf::Y)
        return Euler<T>::Y;
    else
        return Euler<T>::Z;
}

template <class T>
static Euler<T> *
eulerConstructor1(const Vec3<T> &v, typename IMATH_NAMESPACE::Eulerf::Order order)
{
    typename Euler<T>::Order o = interpretOrder<T>(order);
    return new Euler<T>(v, o);
}

template <class T>
static Euler<T> *
eulerConstructor1a(const Vec3<T> &v)
{
    return eulerConstructor1 (v, IMATH_NAMESPACE::Eulerf::Default);
}

template <class T>
static Euler<T> *
eulerConstructor1b(const Vec3<T> &v, int iorder)
{
    typename Euler<T>::Order o = typename Euler<T>::Order (iorder);
    return new Euler<T>(v, o);
}

template <class T>
static Euler<T> *
eulerConstructor2(T i, T j, T k, typename IMATH_NAMESPACE::Eulerf::Order order)
{
    typename Euler<T>::Order o = interpretOrder<T>(order);
    return new Euler<T>(i, j, k, o);
}

template <class T>
static Euler<T> *
eulerConstructor2a(T i, T j, T k)
{
    return eulerConstructor2 (i, j, k, IMATH_NAMESPACE::Eulerf::Default);
}

template <class T>
static Euler<T> *
eulerConstructor2b(T i, T j, T k, int iorder)
{
    typename Euler<T>::Order o = typename Euler<T>::Order (iorder);
    return new Euler<T>(i, j, k, o);
}

template <class T>
static Euler<T> *
eulerConstructor3(const Matrix33<T> &mat, typename IMATH_NAMESPACE::Eulerf::Order order)
{
    typename Euler<T>::Order o = interpretOrder<T>(order);
    return new Euler<T>(mat, o);
}

template <class T>
static Euler<T> *
eulerConstructor3a(const Matrix33<T> &mat)
{
    return eulerConstructor3 (mat, IMATH_NAMESPACE::Eulerf::Default);
}

template <class T>
static Euler<T> *
eulerConstructor3b(const Matrix33<T> &mat, int iorder)
{
    typename Euler<T>::Order o = typename Euler<T>::Order (iorder);
    return new Euler<T>(mat, o);
}

template <class T>
static Euler<T> *
eulerConstructor4(const Matrix44<T> &mat, typename IMATH_NAMESPACE::Eulerf::Order order)
{
    typename Euler<T>::Order o = interpretOrder<T>(order);
    return new Euler<T>(mat, o);
}

template <class T>
static Euler<T> *
eulerConstructor4a(const Matrix44<T> &mat)
{
    return eulerConstructor4 (mat, IMATH_NAMESPACE::Eulerf::Default);
}

template <class T>
static Euler<T> *
eulerConstructor4b(const Matrix44<T> &mat, int iorder)
{
    typename Euler<T>::Order o = typename Euler<T>::Order (iorder);
    return new Euler<T>(mat, o);
}

template <class T>
static Euler<T> *
eulerConstructor5(typename IMATH_NAMESPACE::Eulerf::Order order)
{
    typename Euler<T>::Order o = interpretOrder<T>(order);
    return new Euler<T>(o);
}

template <class T>
static Euler<T> *
eulerConstructor5a()
{
    typename Euler<T>::Order o = interpretOrder<T>(IMATH_NAMESPACE::Eulerf::Default);
    return new Euler<T>(o);
}

template <class T>
static Euler<T> *
eulerConstructor5b(int iorder)
{
    typename Euler<T>::Order o = typename Euler<T>::Order (iorder);
    return new Euler<T>(o);
}

template <class T>
static Euler<T> *
eulerConstructor6(T x, T y, T z)
{
    return new Euler<T>(Vec3<T>(x,y,z));
}

template <class T>
static Euler<T> *
eulerConstructor7(const Quat<T> &quat, typename IMATH_NAMESPACE::Eulerf::Order order)
{
    Euler<T> *e = eulerConstructor5<T>(order);
    e->extract(quat);
    return e;
}

template <class T>
static Euler<T> *
eulerConstructor7a(const Quat<T> &quat)
{
    return eulerConstructor7(quat, IMATH_NAMESPACE::Eulerf::Default);
}

template <class T>
static Euler<T> *
eulerConstructor7b(const Quat<T> &quat, int iorder)
{
    Euler<T> *e = eulerConstructor5b<T>(iorder);
    e->extract(quat);
    return e;
}

template <class T, class S>
static Euler<T> *
eulerConversionConstructor(const Euler<S> &euler)
{
    MATH_EXC_ON;
    Euler<T> *e = new Euler<T>;
    *e = euler;
    return e;
}

template <class T>
static void
eulerMakeNear(Euler<T> &euler, Euler<T> &target)
{
    MATH_EXC_ON;
    euler.makeNear (target);
}

template <class T>
static void
eulerSetOrder(Euler<T> &euler, typename IMATH_NAMESPACE::Eulerf::Order order)
{
    typename Euler<T>::Order o = interpretOrder<T>(order);
    euler.setOrder (o);
}
 
template <class T>
static void
eulerSet(Euler<T> &euler, IMATH_NAMESPACE::Eulerf::Axis axis, int relative, int parityEven, int firstRepeats)
{
    MATH_EXC_ON;
    typename Euler<T>::Axis a = interpretAxis<T>(axis);
    euler.set (a, relative, parityEven, firstRepeats);
}

template <class T>
static void 
extract1(Euler<T> &euler, const Matrix33<T> &m)
{
    MATH_EXC_ON;
    euler.extract(m);
}

template <class T>
static void 
extract2(Euler<T> &euler, const Matrix44<T> &m)
{
    MATH_EXC_ON;
    euler.extract(m);
}

template <class T>
static void 
extract3(Euler<T> &euler, const Quat<T> &q)
{
    MATH_EXC_ON;
    euler.extract(q);
}

template <class T>
static Matrix33<T>
toMatrix33(Euler<T> &euler)
{
    MATH_EXC_ON;
    return euler.toMatrix33();
}

template <class T>
static Matrix44<T>
toMatrix44(Euler<T> &euler)
{
    MATH_EXC_ON;
    return euler.toMatrix44();
}

template <class T>
static Quat<T>
toQuat(Euler<T> &euler)
{
    MATH_EXC_ON;
    return euler.toQuat();
}

template <class T>
static Vec3<T>
toXYZVector(Euler<T> &euler)
{
    MATH_EXC_ON;
    return euler.toXYZVector();
}

template <class T>
class_<Euler<T>,bases<IMATH_NAMESPACE::Vec3<T> > >
register_Euler()
{
    class_<Euler<T>,bases<Vec3<T> > > euler_class(EulerName<T>::value,EulerName<T>::value,init<Euler<T> >("copy construction"));
    euler_class
        .def(init<>("imath Euler default construction"))
        .def("__init__", make_constructor(eulerConstructor1<T>))
        .def("__init__", make_constructor(eulerConstructor1a<T>))
        .def("__init__", make_constructor(eulerConstructor1b<T>))
        .def("__init__", make_constructor(eulerConstructor2<T>))
        .def("__init__", make_constructor(eulerConstructor2a<T>))
        .def("__init__", make_constructor(eulerConstructor2b<T>))
        .def("__init__", make_constructor(eulerConstructor3<T>),
             "Euler-from-matrix construction assumes, but does\n"
             "not verify, that the matrix includes no shear or\n"
             "non-uniform scaling.  If necessary, you can fix\n"
             "the matrix by calling the removeScalingAndShear()\n"
             "function.\n")
        .def("__init__", make_constructor(eulerConstructor3a<T>))
        .def("__init__", make_constructor(eulerConstructor3b<T>))
        .def("__init__", make_constructor(eulerConstructor4<T>))
        .def("__init__", make_constructor(eulerConstructor4a<T>))
        .def("__init__", make_constructor(eulerConstructor4b<T>))
        .def("__init__", make_constructor(eulerConstructor5<T>))
        .def("__init__", make_constructor(eulerConstructor5a<T>))
        .def("__init__", make_constructor(eulerConstructor5b<T>))
        .def("__init__", make_constructor(eulerConstructor6<T>))
        .def("__init__", make_constructor(eulerConstructor7<T>))
        .def("__init__", make_constructor(eulerConstructor7a<T>))
        .def("__init__", make_constructor(eulerConstructor7b<T>))
        .def("__init__", make_constructor(eulerConversionConstructor<T, float>))
        .def("__init__", make_constructor(eulerConversionConstructor<T, double>))
        
        .def("angleOrder", &getAngleOrder<T>, "angleOrder() set the angle order")
        
        .def("frameStatic", &Euler<T>::frameStatic, 
             "e.frameStatic() -- returns true if the angles of e\n"
             "are measured relative to a set of fixed axes,\n"
             "or false if the angles of e are measured relative to\n"
             "each other\n")
            
        .def("initialAxis", &Euler<T>::initialAxis, 
             "e.initialAxis() -- returns the initial rotation\n"
             "axis of e (EULER_X_AXIS, EULER_Y_AXIS, EULER_Z_AXIS)")
        
        .def("initialRepeated", &Euler<T>::initialRepeated,
             "e.initialRepeated() -- returns 1 if the initial\n"
             "rotation axis of e is repeated (for example,\n"
             "e.order() == EULER_XYX); returns 0 if the initial\n"
             "rotation axis is not repeated.\n")
             
        .def("makeNear", &eulerMakeNear<T>,
             "e.makeNear(t) -- adjusts Euler e so that it\n"
             "represents the same rotation as before, but the\n"
             "individual angles of e differ from the angles of\n"
             "t by as little as possible.\n"
             "This method might not make sense if e.order()\n"
             "and t.order() are different\n")
        
        .def("order", &Euler<T>::order,
             "e.order() -- returns the rotation order in e\n"
             "(EULER_XYZ, EULER_XZY, ...)")
        
        .def("parityEven", &Euler<T>::parityEven, 
             "e.parityEven() -- returns the parity of the\n"
             "axis permutation of e\n")
        
        .def("set", &eulerSet<T>,
             "e.set(i,r,p,f) -- sets the rotation order in e\n"
             "according to the following flags:\n"
             "\n"
             "   i   initial axis (EULER_X_AXIS,\n"
             "       EULER_Y_AXIS or EULER_Z_AXIS)\n"
             "\n"
             "   r   rotation angles are measured relative\n"
             "       to each other (r == 1), or relative to a\n"
             "       set of fixed axes (r == 0)\n"
             "\n"
             "   p   parity of axis permutation is even (r == 1)\n"
             "       or odd (r == 0)\n"
             "\n"
             "   f   first rotation axis is repeated (f == 1)\n"
             "	or not repeated (f == 0)\n")
        
        .def("setOrder", &eulerSetOrder<T>,
             "e.setOrder(o) -- sets the rotation order in e\n"
             "to o (EULER_XYZ, EULER_XZY, ...)")
             
        .def("setXYZVector", &Euler<T>::setXYZVector,
             "e.setXYZVector(v) -- sets the three rotation\n"
             "angles in e to v[0], v[1], v[2]")
        .def("setXYZVector", &setXYZTuple<T>)
        
        .def("extract", &extract1<T>,
             "e.extract(m) -- extracts the rotation component\n"
             "from 3x3 matrix m and stores the result in e.\n"
             "Assumes that m does not contain shear or non-\n"
             "uniform scaling.  If necessary, you can fix m\n"
             "by calling m.removeScalingAndShear().")
        
        .def("extract", &extract2<T>,
             "e.extract(m) -- extracts the rotation component\n"
             "from 4x4 matrix m and stores the result in e.\n"
             "Assumes that m does not contain shear or non-\n"
             "uniform scaling.  If necessary, you can fix m\n"
             "by calling m.removeScalingAndShear().")
        
        .def("extract", &extract3<T>,
             "e.extract(q) -- extracts the rotation component\n"
             "from quaternion q and stores the result in e")            
        
        .def("toMatrix33", &toMatrix33<T>, "e.toMatrix33() -- converts e into a 3x3 matrix\n")
        
        .def("toMatrix44", &toMatrix44<T>, "e.toMatrix44() -- converts e into a 4x4 matrix\n")
        
        .def("toQuat", &toQuat<T>, "e.toQuat() -- converts e into a quaternion\n")
        
        .def("toXYZVector", &toXYZVector<T>, 
             "e.toXYZVector() -- converts e into an XYZ\n"
             "rotation vector")
        .def("__str__", &Euler_str<T>)
        .def("__repr__", &Euler_repr<T>)
        
        .def("__eq__", &equal<T>)
        .def("__ne__", &notequal<T>)
        ;
    
    // fill in the Euler scope
    {
        scope euler_scope(euler_class);
        enum_<typename Euler<T>::Order> euler_order("Order");
        euler_order
            .value("XYZ",Euler<T>::XYZ)
            .value("XZY",Euler<T>::XZY)
            .value("YZX",Euler<T>::YZX)
            .value("YXZ",Euler<T>::YXZ)
            .value("ZXY",Euler<T>::ZXY)
            .value("ZYX",Euler<T>::ZYX)
            .value("XZX",Euler<T>::XZX)
            .value("XYX",Euler<T>::XYX)
            .value("YXY",Euler<T>::YXY)
            .value("YZY",Euler<T>::YZY)
            .value("ZYZ",Euler<T>::ZYZ)
            .value("ZXZ",Euler<T>::ZXZ)
            .value("XYZr",Euler<T>::XYZr)
            .value("XZYr",Euler<T>::XZYr)
            .value("YZXr",Euler<T>::YZXr)
            .value("YXZr",Euler<T>::YXZr)
            .value("ZXYr",Euler<T>::ZXYr)
            .value("ZYXr",Euler<T>::ZYXr)
            .value("XZXr",Euler<T>::XZXr)
            .value("XYXr",Euler<T>::XYXr)
            .value("YXYr",Euler<T>::YXYr)
            .value("YZYr",Euler<T>::YZYr)
            .value("ZYZr",Euler<T>::ZYZr)
            .value("ZXZr",Euler<T>::ZXZr)

            // don't export these, they're not really part of the public interface
            //.value("Legal",Euler<T>::Legal)
            //.value("Min",Euler<T>::Min)
            //.value("Max",Euler<T>::Max)

            // handle Default seperately since boost sets up a 1-1 mapping for enum values
            //.value("Default",Euler<T>::Default)
            .export_values()
            ;
        // just set it to the XYZ value manually
        euler_scope.attr("Default") = euler_scope.attr("XYZ");

        enum_<typename Euler<T>::Axis>("Axis")
            .value("X",Euler<T>::X)
            .value("Y",Euler<T>::Y)
            .value("Z",Euler<T>::Z)
            .export_values()
            ;

        enum_<typename Euler<T>::InputLayout>("InputLayout")
            .value("XYZLayout",Euler<T>::XYZLayout)
            .value("IJKLayout",Euler<T>::IJKLayout)
            .export_values()
            ;
    }

    decoratecopy(euler_class);

    return euler_class;
}

// XXX fixme - template this
// really this should get generated automatically...

/*
template <class T,int index>
static FixedArray<T>
EulerArray_get(FixedArray<IMATH_NAMESPACE::Euler<T> > &qa)
{
    return FixedArray<T>( &(qa[0].r)+index, qa.len(), 4*qa.stride());
}
*/

template <class T>
static FixedArray<IMATH_NAMESPACE::Euler<T> > *
EulerArray_eulerConstructor7a(const FixedArray<IMATH_NAMESPACE::Quat<T> > &q)
{
    MATH_EXC_ON;
    size_t len = q.len();
    FixedArray<IMATH_NAMESPACE::Euler<T> >* result = new FixedArray<IMATH_NAMESPACE::Euler<T> >(len);
    for (size_t i = 0; i < len; ++i) {
        (*result)[i].extract(q[i]);
    }
    return result;
}

template <class T>
class_<FixedArray<IMATH_NAMESPACE::Euler<T> > >
register_EulerArray()
{
    class_<FixedArray<IMATH_NAMESPACE::Euler<T> > > eulerArray_class = FixedArray<IMATH_NAMESPACE::Euler<T> >::register_("Fixed length array of IMATH_NAMESPACE::Euler");
    eulerArray_class
        //.add_property("x",&EulerArray_get<T,1>)
        //.add_property("y",&EulerArray_get<T,2>)
        //.add_property("z",&EulerArray_get<T,3>)
        .def("__init__", make_constructor(EulerArray_eulerConstructor7a<T>))
        ;

    add_comparison_functions(eulerArray_class);
    PyImath::add_explicit_construction_from_type<IMATH_NAMESPACE::Matrix33<T> >(eulerArray_class);
    PyImath::add_explicit_construction_from_type<IMATH_NAMESPACE::Matrix44<T> >(eulerArray_class);
    return eulerArray_class;
}

template PYIMATH_EXPORT class_<IMATH_NAMESPACE::Euler<float>,bases<IMATH_NAMESPACE::Vec3<float> > > register_Euler<float>();
template PYIMATH_EXPORT class_<IMATH_NAMESPACE::Euler<double>,bases<IMATH_NAMESPACE::Vec3<double> > > register_Euler<double>();

template PYIMATH_EXPORT class_<FixedArray<IMATH_NAMESPACE::Euler<float> > > register_EulerArray<float>();
template PYIMATH_EXPORT class_<FixedArray<IMATH_NAMESPACE::Euler<double> > > register_EulerArray<double>();
}
namespace PyImath {
	template<> PYIMATH_EXPORT IMATH_NAMESPACE::Euler<float> FixedArrayDefaultValue<IMATH_NAMESPACE::Euler<float> >::value() { return IMATH_NAMESPACE::Euler<float>(); }
	template<> PYIMATH_EXPORT IMATH_NAMESPACE::Euler<double> FixedArrayDefaultValue<IMATH_NAMESPACE::Euler<double> >::value() { return IMATH_NAMESPACE::Euler<double>(); }
}
