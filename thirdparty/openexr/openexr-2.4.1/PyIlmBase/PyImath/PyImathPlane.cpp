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


#include "PyImathPlane.h"
#include "PyImathDecorators.h"
#include "PyImathExport.h"
#include <Python.h>
#include <boost/python.hpp>
#include <boost/python/make_constructor.hpp>
#include <boost/format.hpp>
#include "PyImath.h"
#include "PyImathVec.h"
#include "PyImathMathExc.h"
#include <Iex.h>

namespace PyImath{
using namespace boost::python;
using namespace IMATH_NAMESPACE;

template <class T> struct PlaneName {static const char *value;};
template <> const char *PlaneName<float>::value = "Plane3f";
template <> const char *PlaneName<double>::value = "Plane3d";

template <class T>
static Plane3<T> *Plane3_construct_default()
{
    Vec3<T> normal(T (1), T (0), T (0));
    return new Plane3<T>(normal, T (0));
}

template <class T>
static Plane3<T> *Plane3_plane_construct(const object &planeObj)
{
    MATH_EXC_ON;
    extract < Plane3<float> > ef (planeObj);
    extract < Plane3<double> > ed (planeObj);

    Plane3<T> *p = 0;

    if (ef.check())
    {
        Plane3<float> efp = ef();
        p = new Plane3<T>;
        p->normal = efp.normal;
        p->distance = efp.distance;
    }

    else if (ed.check())
    {
        Plane3<double> edp = ed();
        p = new Plane3<T>;
        p->normal = edp.normal;
        p->distance = edp.distance;
    }

    else
    {
        THROW(IEX_NAMESPACE::LogicExc, "invalid parameter passed to Plane constructor");
    }
    
    return p;
}

template <class T>
static Plane3<T> *Plane3_tuple_constructor1(const tuple &t, T distance)
{
    MATH_EXC_ON;
    if(t.attr("__len__")() == 3)
    {
        Vec3<T> normal;
        normal.x = extract<T>(t[0]);
        normal.y = extract<T>(t[1]);
        normal.z = extract<T>(t[2]);
        
        return new Plane3<T>(normal, distance);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Plane3 expects tuple of length 3");
}

template <class T>
static Plane3<T> *Plane3_tuple_constructor2(const tuple &t0, const tuple &t1)
{
    MATH_EXC_ON;
    if(t0.attr("__len__")() == 3 && t1.attr("__len__")() == 3)
    {
        Vec3<T> point, normal;
        point.x = extract<T>(t0[0]);
        point.y = extract<T>(t0[1]);
        point.z = extract<T>(t0[2]);
        
        normal.x = extract<T>(t1[0]);
        normal.y = extract<T>(t1[1]);
        normal.z = extract<T>(t1[2]);
        
        return new Plane3<T>(point, normal);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Plane3 expects tuples of length 3");
}

template <class T>
static Plane3<T> *Plane3_tuple_constructor3(const tuple &t0, const tuple &t1, const tuple &t2)
{
    MATH_EXC_ON;
    if(t0.attr("__len__")() == 3 && t1.attr("__len__")() == 3 && t2.attr("__len__")() == 3)
    {
        Vec3<T> point0, point1, point2;
        point0.x = extract<T>(t0[0]);
        point0.y = extract<T>(t0[1]);
        point0.z = extract<T>(t0[2]);
        
        point1.x = extract<T>(t1[0]);
        point1.y = extract<T>(t1[1]);
        point1.z = extract<T>(t1[2]);

        point2.x = extract<T>(t2[0]);
        point2.y = extract<T>(t2[1]);
        point2.z = extract<T>(t2[2]); 
        
        return new Plane3<T>(point0, point1, point2);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Plane3 expects tuple of length 3");
}

template <class T>
static Plane3<T>
mul (const Plane3<T> &plane, const Matrix44<T> &M)
{
    MATH_EXC_ON;
    return plane * M;
}

template <class T>
static void
set1 (Plane3<T> &plane, const Vec3<T> &v, T t)
{
    MATH_EXC_ON;
    plane.set (v, t);
}

template <class T>
static void
set2 (Plane3<T> &plane, const Vec3<T> &v1, const Vec3<T> &v2)
{
    MATH_EXC_ON;
    plane.set (v1, v2);
}

template <class T>
static void
set3 (Plane3<T> &plane, const Vec3<T> &v1, const Vec3<T> &v2, const Vec3<T> &v3)
{
    MATH_EXC_ON;
    plane.set (v1, v2, v3);
}

template <class T>
static std::string Plane3_str(const Plane3<T> &plane)
{
    std::stringstream stream;

    PyObject *normalObj = V3<T>::wrap (plane.normal);
    PyObject *normalReprObj = PyObject_Repr (normalObj);
#if PY_MAJOR_VERSION > 2
    std::string normalReprStr = PyUnicode_AsUTF8 (normalReprObj);
#else
    std::string normalReprStr = PyString_AsString (normalReprObj);
#endif
    Py_DECREF (normalReprObj);
    Py_DECREF (normalObj);

    stream << PlaneName<T>::value << "(" << normalReprStr << ", " 
           << plane.distance << ")";
    return stream.str();
}

// Non-specialized repr is same as str
template <class T>
static std::string Plane3_repr(const Plane3<T> &plane)
{
    return Plane3_str(plane);
}

// Specialization for float to full precision
template <>
std::string Plane3_repr(const Plane3<float> &plane)
{
    PyObject *normalObj = V3<float>::wrap (plane.normal);
    PyObject *normalReprObj = PyObject_Repr (normalObj);
#if PY_MAJOR_VERSION > 2
    std::string normalReprStr = PyUnicode_AsUTF8 (normalReprObj);
#else
    std::string normalReprStr = PyString_AsString (normalReprObj);
#endif
    Py_DECREF (normalReprObj);
    Py_DECREF (normalObj);

    return (boost::format("%s(%s, %.9g)")
                        % PlaneName<float>::value
                        % normalReprStr.c_str()
                        % plane.distance).str();
}

// Specialization for double to full precision
template <>
std::string Plane3_repr(const Plane3<double> &plane)
{
    PyObject *normalObj = V3<double>::wrap (plane.normal);
    PyObject *normalReprObj = PyObject_Repr (normalObj);
#if PY_MAJOR_VERSION > 2
    std::string normalReprStr = PyUnicode_AsUTF8 (normalReprObj);
#else
    std::string normalReprStr = PyString_AsString (normalReprObj);
#endif
    Py_DECREF (normalReprObj);
    Py_DECREF (normalObj);

    return (boost::format("%s(%s, %.17g)")
                        % PlaneName<double>::value
                        % normalReprStr.c_str()
                        % plane.distance).str();
}


template <class T>
static T
distance(Plane3<T> &plane)
{
    return plane.distance;
}

template <class T>
static Vec3<T>
normal(Plane3<T> &plane)
{
    return plane.normal;
}

template <class T>
static void
setNormal(Plane3<T> &plane, const Vec3<T> &normal)
{
    MATH_EXC_ON;
    plane.normal = normal.normalized();
}

template <class T>
static void
setDistance(Plane3<T> &plane, const T &distance)
{
    plane.distance = distance;
}

template <class T, class S>
static object intersectT(const Plane3<T> &plane, const Line3<S> &line)
{
    MATH_EXC_ON;
    T param;
    Line3<T> l;
    l.pos = line.pos;
    l.dir = line.dir;

    if(plane.intersectT(l, param))
        return object(param);
    
    return object();
}

template <class T>
static bool
intersect2(const Plane3<T> &plane, const Line3<T> &line, Vec3<T> &intersection)
{
    MATH_EXC_ON;
    return plane.intersect(line, intersection);
}

template <class T, class S>
static object
intersect1(const Plane3<T> &plane, const Line3<S> &line)
{
    MATH_EXC_ON;
    Vec3<T> intersection;
    Line3<T> l;
    l.pos = line.pos;
    l.dir = line.dir;
    if(plane.intersect(l, intersection))
        return object(intersection);
    
    return object();
    
}

template <class T>
static void
setTuple1(Plane3<T> &plane, const tuple &t, T distance)
{
    MATH_EXC_ON;
    if(t.attr("__len__")() == 3)
    {
        Vec3<T> normal;
        normal.x = extract<T>(t[0]);
        normal.y = extract<T>(t[1]);
        normal.z = extract<T>(t[2]);
        
        plane.set(normal, distance);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Plane3 expects tuple of length 3");    
}

template <class T>
static void
setTuple2(Plane3<T> &plane, const tuple &t0, const tuple &t1)
{
    MATH_EXC_ON;
    if(t0.attr("__len__")() == 3 && t1.attr("__len__")() == 3)
    {
        Vec3<T> point, normal;
        point.x = extract<T>(t0[0]);
        point.y = extract<T>(t0[1]);
        point.z = extract<T>(t0[2]);
        
        normal.x = extract<T>(t1[0]);
        normal.y = extract<T>(t1[1]);
        normal.z = extract<T>(t1[2]);
        
        plane.set(point, normal);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Plane3 expects tuples of length 3");
}

template <class T>
static void
setTuple3(Plane3<T> &plane, const tuple &t0, const tuple &t1, const tuple &t2)
{
    MATH_EXC_ON;
    if(t0.attr("__len__")() == 3 && t1.attr("__len__")() == 3 && t2.attr("__len__")() == 3)
    {
        Vec3<T> point0, point1, point2;
        point0.x = extract<T>(t0[0]);
        point0.y = extract<T>(t0[1]);
        point0.z = extract<T>(t0[2]);
        
        point1.x = extract<T>(t1[0]);
        point1.y = extract<T>(t1[1]);
        point1.z = extract<T>(t1[2]);

        point2.x = extract<T>(t2[0]);
        point2.y = extract<T>(t2[1]);
        point2.z = extract<T>(t2[2]); 
        
        plane.set(point0, point1, point2);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Plane3 expects tuple of length 3");
}

template <class T>
static Vec3<T>
reflectPoint(Plane3<T> &plane, const Vec3<T> &p)
{
    MATH_EXC_ON;
    return plane.reflectPoint(p);
}

template <class T>
static Vec3<T>
reflectPointTuple(Plane3<T> &plane, const tuple &t)
{
    MATH_EXC_ON;
    Vec3<T> point;
    if(t.attr("__len__")() == 3)
    {
        point.x = extract<T>(t[0]);
        point.y = extract<T>(t[1]);
        point.z = extract<T>(t[2]); 
        
        return plane.reflectPoint(point);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Plane3 expects tuple of length 3");
}

template <class T>
static T
distanceTo(Plane3<T> &plane, const Vec3<T> &v)
{
    MATH_EXC_ON;
    return plane.distanceTo(v);
}

template <class T>
static T
distanceToTuple(Plane3<T> &plane, const tuple &t)
{
    MATH_EXC_ON;
    Vec3<T> point;
    if(t.attr("__len__")() == 3)
    {
        point.x = extract<T>(t[0]);
        point.y = extract<T>(t[1]);
        point.z = extract<T>(t[2]); 
        
        return plane.distanceTo(point);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Plane3 expects tuple of length 3");    
}

template <class T>
static Vec3<T>
reflectVector(Plane3<T> &plane, const Vec3<T> &v)
{
    MATH_EXC_ON;
    return plane.reflectVector(v);
}

template <class T>
static Vec3<T>
reflectVectorTuple(Plane3<T> &plane, const tuple &t)
{
    MATH_EXC_ON;
    Vec3<T> point;
    if(t.attr("__len__")() == 3)
    {
        point.x = extract<T>(t[0]);
        point.y = extract<T>(t[1]);
        point.z = extract<T>(t[2]); 
        
        return plane.reflectVector(point);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Plane3 expects tuple of length 3");
}

template <class T>
static bool
equal(const Plane3<T> &p1, const Plane3<T> &p2)
{
    if(p1.normal == p2.normal && p1.distance == p2.distance)
        return true;
    else
        return false;
}

template <class T>
static bool
notequal(const Plane3<T> &p1, const Plane3<T> &p2)
{
    if(p1.normal != p2.normal || p1.distance != p2.distance)
        return true;
    else
        return false;
}

template <class T>
static Plane3<T>
negate(const Plane3<T> &plane)
{
    MATH_EXC_ON;
    Plane3<T> p;
    p.set(-plane.normal, -plane.distance);
    
    return p;
}



template <class T>
class_<Plane3<T> >
register_Plane()
{
    const char *name = PlaneName<T>::value;
    
    class_< Plane3<T> > plane_class(name);
    plane_class
        .def("__init__",make_constructor(Plane3_construct_default<T>),"initialize normal to  (1,0,0), distance to 0")
        .def("__init__",make_constructor(Plane3_tuple_constructor1<T>))
        .def("__init__",make_constructor(Plane3_tuple_constructor2<T>))
        .def("__init__",make_constructor(Plane3_tuple_constructor3<T>))
        .def("__init__",make_constructor(Plane3_plane_construct<T>))
        .def(init<const Vec3<T> &, T>("Plane3(normal, distance) construction"))
        .def(init<const Vec3<T> &, const Vec3<T> &>("Plane3(point, normal) construction"))
        .def(init<const Vec3<T> &, const Vec3<T> &, const Vec3<T> &>("Plane3(point1, point2, point3) construction"))
        .def("__eq__", &equal<T>)
        .def("__ne__", &notequal<T>)
        .def("__mul__", &mul<T>)
        .def("__neg__", &negate<T>)
        .def("__str__", &Plane3_str<T>)
        .def("__repr__", &Plane3_repr<T>)
        
        .def_readwrite("normal", &Plane3<T>::normal)
        .def_readwrite("distance", &Plane3<T>::distance)
        
        .def("normal", &normal<T>, "normal()",
             "pl.normal() -- returns the normal of plane pl")
             
        .def("distance", &distance<T>, "distance()",
        	 "pl.distance() -- returns the signed distance\n"
			 "of plane pl from the coordinate origin")
        
        .def("setNormal", &setNormal<T>, "setNormal()",
        	 "pl.setNormal(n) -- sets the normal of plane\n"
			 "pl to n.normalized()")
             
        .def("setDistance", &setDistance<T>, "setDistance()",
        	 "pl.setDistance(d) -- sets the signed distance\n"
			 "of plane pl from the coordinate origin to d")
             
        .def("set", &set1<T>, "set()",
        	 "pl.set(n,d) -- sets the normal and the signed\n"
			 "   distance of plane pl to n and d\n"
			 "\n"
			 "pl.set(p,n) -- sets the normal of plane pl to\n"
			 "   n.normalized() and adjusts the distance of\n"
			 "   pl from the coordinate origin so that pl\n"
			 "   passes through point p\n"
			 "\n"
			 "pl.set(p1,p2,p3) -- sets the normal of plane pl\n"
			 "   to (p2-p1)%(p3-p1)).normalized(), and adjusts\n"
			 "   the distance of pl from the coordinate origin\n"
			 "   so that pl passes through points p1, p2 and p3")
             
        .def("set", &set2<T>, "set()")
        .def("set", &set3<T>, "set()")
        
        .def("set", &setTuple1<T>, "set()")
        .def("set", &setTuple2<T>, "set()")
        .def("set", &setTuple3<T>, "set()")        
        
        .def("intersect", &intersect2<T>,
        	 "pl.intersect(ln, pt) -- returns true if the line intersects\n"
             "the plane, false if it doesn't.  The point where plane\n"
			 "pl and line ln intersect is stored in pt")
             
        .def("intersect", &intersect1<T,float>,
        	 "pl.intersect(ln) -- returns the point where plane\n"
			 "pl and line ln intersect, or None if pl and ln do\n"
			 "not intersect")
        .def("intersect", &intersect1<T,double>,
        	 "pl.intersect(ln) -- returns the point where plane\n"
			 "pl and line ln intersect, or None if pl and ln do\n"
			 "not intersect")
             
        .def("intersectT", &intersectT<T, float>,
             "pl.intersectT(ln) -- computes the intersection,\n"
			 "i, of plane pl and line ln, and returns t, so that\n"
			 "ln.pos() + t * ln.dir() == i.\n"
			 "If pl and ln do not intersect, pl.intersectT(ln)\n"
			 "returns None.\n") 
             
        .def("intersectT", &intersectT<T,double>)
             
        .def("distanceTo", &distanceTo<T>, "distanceTo()",
        	 "pl.distanceTo(p) -- returns the signed distance\n"
			 "between plane pl and point p (positive if p is\n"
			 "on the side of pl where the pl's normal points)\n")
        
        .def("distanceTo", &distanceToTuple<T>)
             
        .def("reflectPoint", &reflectPoint<T>, "reflectPoint()",
        	 "pl.reflectPoint(p) -- returns the image,\n"
			 "q, of point p after reflection on plane pl:\n"
			 "the distance between p and q is twice the\n"
			 "distance between p and pl, and the line from\n"
			 "p to q is parallel to pl's normal.")
             
        .def("reflectPoint", &reflectPointTuple<T>)
             
        .def("reflectVector", &reflectVector<T>, "reflectVector()",
        	 "pl.reflectVector(v) -- returns the direction\n"
			 "of a ray with direction v after reflection on\n"
			 "plane pl")
        .def("reflectVector", &reflectVectorTuple<T>)
        
        ;

    decoratecopy(plane_class);

    return plane_class;
}

template PYIMATH_EXPORT class_<Plane3<float> > register_Plane<float>();
template PYIMATH_EXPORT class_<Plane3<double> > register_Plane<double>();

} //namespace PyIMath
