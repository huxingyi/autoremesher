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

#include "PyImathLine.h"
#include "PyImathDecorators.h"
#include "PyImathExport.h"
#include <Python.h>
#include <boost/python.hpp>
#include <boost/python/make_constructor.hpp>
#include <boost/format.hpp>
#include "PyImath.h"
#include "PyImathVec.h"
#include "PyImathMathExc.h"
#include <ImathLineAlgo.h>
#include <ImathMatrix.h>
#include <Iex.h>


namespace PyImath{
using namespace boost::python;
using namespace IMATH_NAMESPACE;

template <class T> struct LineName {static const char *value;};
template <> const char *LineName<float>::value = "Line3f";
template <> const char *LineName<double>::value = "Line3d";

template <class T>
static Line3<T> * 
Line3_construct_default()
{
    Vec3<T> point1(T (0), T(0), T(0));
    Vec3<T> point2(T (1), T(0), T(0));
    
    return new Line3<T>(point1, point2);
}

template <class T>
static Line3<T> * 
Line3_tuple_construct(const tuple &t0, const tuple &t1)
{
    Vec3<T> v0, v1;
    if(t0.attr("__len__")() == 3 && t1.attr("__len__")() == 3)
    {
        v0.x = extract<T>(t0[0]);
        v0.y = extract<T>(t0[1]);
        v0.z = extract<T>(t0[2]);

        v1.x = extract<T>(t1[0]);
        v1.y = extract<T>(t1[1]);
        v1.z = extract<T>(t1[2]);
        
        return new Line3<T>(v0, v1);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Line3 expects tuple of length 3");    
}

template <class T, class S>
static Line3<T> *
Line3_line_construct(const Line3<S> &line)
{
    Line3<T> *l = new Line3<T>;
    l->pos = line.pos;
    l->dir = line.dir;
    
    return l;
}

template <class T>
static void
set1(Line3<T> &line, const Vec3<T> &p0, const Vec3<T> &p1)
{
    MATH_EXC_ON;
    line.set (p0, p1);
}

template <class T>
static void
setTuple(Line3<T> &line, const tuple &t0, const tuple &t1)
{
    MATH_EXC_ON;
    Vec3<T> v0, v1;
    if(t0.attr("__len__")() == 3 && t1.attr("__len__")() == 3)
    {
        v0.x = extract<T>(t0[0]);
        v0.y = extract<T>(t0[1]);
        v0.z = extract<T>(t0[2]);

        v1.x = extract<T>(t1[0]);
        v1.y = extract<T>(t1[1]);
        v1.z = extract<T>(t1[2]);
        
        line.set(v0, v1);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Line3 expects tuple of length 3");    
}

template <class T>
static Vec3<T>
pointAt(Line3<T> &line, T t)
{
    MATH_EXC_ON;
    return line.operator()(t);
}

template <class T>
static T
distanceTo1(Line3<T> &line, Vec3<T> &p)
{
    MATH_EXC_ON;
    return line.distanceTo(p);
}

template <class T>
static T
distanceTo2(Line3<T> &line, Line3<T> &other)
{
    MATH_EXC_ON;
    return line.distanceTo(other);
}

template <class T>
static T
distanceToTuple(Line3<T> line, const tuple &t)
{
    Vec3<T> v;
    if(t.attr("__len__")() == 3)
    {
        v.x = extract<T>(t[0]);
        v.y = extract<T>(t[1]);
        v.z = extract<T>(t[2]);
        
        return line.distanceTo(v);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Line3 expects tuple of length 3");      
}

template <class T>
static Vec3<T>
closestPointTo1(Line3<T> line, const Vec3<T> &p)
{
    MATH_EXC_ON;
    return line.closestPointTo(p);
}

template <class T>
static Vec3<T>
closestPointTo2(Line3<T> line, const Line3<T> &other)
{
    MATH_EXC_ON;
    return line.closestPointTo(other);
}
                     
template <class T>
static Vec3<T>
closestPointToTuple(Line3<T> line, const tuple &t)
{
    MATH_EXC_ON;
    Vec3<T> v;
    if(t.attr("__len__")() == 3)
    {
        v.x = extract<T>(t[0]);
        v.y = extract<T>(t[1]);
        v.z = extract<T>(t[2]);
        
        return line.closestPointTo(v);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Line3 expects tuple of length 3");      
}

template <class T>
static Vec3<T>
getPosition(Line3<T> &line)
{
    return line.pos;
}

template <class T>
static void
setPosition(Line3<T> &line, const Vec3<T> &pos)
{
    line.pos = pos;
}

template <class T>
static void
setPositionTuple(Line3<T> &line, const tuple &t)
{
    Vec3<T> pos;
    if(t.attr("__len__")() == 3)
    {
        pos.x = extract<T>(t[0]);
        pos.y = extract<T>(t[1]);
        pos.z = extract<T>(t[2]);
        
        line.pos = pos;
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Line3 expects tuple of length 3");    
}

template <class T>
static Vec3<T>
getDirection(Line3<T> &line)
{
    return line.dir;
}

template <class T>
static void
setDirection(Line3<T> &line, const Vec3<T> &dir)
{
    MATH_EXC_ON;
    line.dir = dir.normalized();
}

template <class T>
static void
setDirectionTuple(Line3<T> &line, const tuple &t)
{
    MATH_EXC_ON;
    Vec3<T> dir;
    if(t.attr("__len__")() == 3)
    {
        dir.x = extract<T>(t[0]);
        dir.y = extract<T>(t[1]);
        dir.z = extract<T>(t[2]);
        
        line.dir = dir.normalized();
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Line3 expects tuple of length 3");    
}

template <class T>
static void
closestPoints1(Line3<T> &line1, const Line3<T> &line2, Vec3<T> &p0, Vec3<T> &p1)
{
    MATH_EXC_ON;
    IMATH_NAMESPACE::closestPoints(line1, line2, p0, p1);
}

template <class T>
static tuple
closestPoints2(Line3<T> &line1, const Line3<T> &line2)
{
    MATH_EXC_ON;
    Vec3<T> p0, p1;
    IMATH_NAMESPACE::closestPoints(line1, line2, p0, p1);
    tuple p0Tuple = make_tuple(p0.x,p0.y,p0.z);
    tuple p1Tuple = make_tuple(p1.x,p1.y,p1.z);

#if !defined(_MSC_VER) || (_MSC_VER <= 1200)
    tuple t = make_tuple(p0Tuple, p1Tuple);
    return t;
#else
    list v3;
    v3.append(p0Tuple);
    v3.append(p1Tuple);
    return tuple(v3);
#endif
}

template <class T>
static Vec3<T>
closestVertex(Line3<T> &line, const Vec3<T> &v0, const Vec3<T> &v1, const Vec3<T> &v2)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::closestVertex(v0, v1, v2, line);
}

template <class T>
static Vec3<T>
closestVertexTuple(Line3<T> &line, const tuple &t0, const tuple &t1, const tuple &t2)
{
    MATH_EXC_ON;
    if(t0.attr("__len__")() == 3 && t1.attr("__len__")() == 3 && t2.attr("__len__")() == 3)
    {
        Vec3<T> v0, v1, v2;
        v0.x = extract<T>(t0[0]);
        v0.y = extract<T>(t0[1]);
        v0.z = extract<T>(t0[2]);
        
        v1.x = extract<T>(t1[0]);
        v1.y = extract<T>(t1[1]);
        v1.z = extract<T>(t1[2]);

        v2.x = extract<T>(t2[0]);
        v2.y = extract<T>(t2[1]);
        v2.z = extract<T>(t2[2]);
        
        return IMATH_NAMESPACE::closestVertex(v0, v1, v2, line);
    }        
    else
        THROW(IEX_NAMESPACE::LogicExc, "Line3 expects tuple of length 3");      
}

template <class T>
static bool
intersect1(Line3<T> &line, const Vec3<T> &v0, const Vec3<T> &v1, const Vec3<T> &v2, 
          Vec3<T> &pt, Vec3<T> &barycentric, bool &front)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::intersect(line, v0, v1, v2, pt, barycentric, front);
}

template <class T>
static object
intersect2(Line3<T> &line, const Vec3<T> &v0, const Vec3<T> &v1, const Vec3<T> &v2)
{    
    MATH_EXC_ON;
    Vec3<T> pt, bar;
    bool front;
    
    if(IMATH_NAMESPACE::intersect(line, v0, v1, v2, pt, bar, front))
    {
        return make_tuple (pt, bar, front);
    }
    else
    {
        return object();
    }
}

template <class T>
static tuple
intersectTuple(Line3<T> &line, const tuple &t0, const tuple &t1, const tuple &t2)
{    

    if(t0.attr("__len__")() == 3 && t1.attr("__len__")() == 3 && t2.attr("__len__")() == 3)
    {   
        Vec3<T> v0, v1, v2, pt, bar;
        bool front;
        v0.x = extract<T>(t0[0]);
        v0.y = extract<T>(t0[1]);
        v0.z = extract<T>(t0[2]);
        
        v1.x = extract<T>(t1[0]);
        v1.y = extract<T>(t1[1]);
        v1.z = extract<T>(t1[2]);

        v2.x = extract<T>(t2[0]);
        v2.y = extract<T>(t2[1]);
        v2.z = extract<T>(t2[2]);
        
        if(IMATH_NAMESPACE::intersect(line, v0, v1, v2, pt, bar, front))
        {
            tuple t = make_tuple(pt, bar, front);
            return t;
        }
        else
        {
            tuple t;
            return t;
        }
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Line3 expects tuple of length 3");
}

template <class T>
static Vec3<T>
rotatePoint(Line3<T> &line, const Vec3<T> &p, const T &r)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::rotatePoint(p, line, r);
}

template <class T>
static Vec3<T>
rotatePointTuple(Line3<T> &line, const tuple &t, const T &r)
{
    MATH_EXC_ON;
    if(t.attr("__len__")() == 3)
    {
        Vec3<T> p;
        p.x = extract<T>(t[0]);
        p.y = extract<T>(t[1]);
        p.z = extract<T>(t[2]);
        
        return IMATH_NAMESPACE::rotatePoint(p, line, r);
    }        
    else
        THROW(IEX_NAMESPACE::LogicExc, "Line3 expects tuple of length 3");      
}

template <class T>
static std::string Line3_repr(const Line3<T> &v)
{
    Vec3<T> v1 = v.pos;
    Vec3<T> v2 = v.pos + v.dir;

    PyObject *v1Obj = V3<T>::wrap (v1);
    PyObject *v1ReprObj = PyObject_Repr (v1Obj);
#if PY_MAJOR_VERSION > 2
    std::string v1ReprStr = PyUnicode_AsUTF8 (v1ReprObj);
#else
    std::string v1ReprStr = PyString_AsString (v1ReprObj);
#endif
    Py_DECREF (v1ReprObj);
    Py_DECREF (v1Obj);

    PyObject *v2Obj = V3<T>::wrap (v2);
    PyObject *v2ReprObj = PyObject_Repr (v2Obj);
#if PY_MAJOR_VERSION > 2
    std::string v2ReprStr = PyUnicode_AsUTF8 (v2ReprObj);
#else
    std::string v2ReprStr = PyString_AsString (v2ReprObj);
#endif
    Py_DECREF (v2ReprObj);
    Py_DECREF (v2Obj);

    std::stringstream stream;
    stream << LineName<T>::value << "(" << v1ReprStr << ", " << v2ReprStr << ")";
    return stream.str();
}

template <class T>
static bool
equal(const Line3<T> &l1, const Line3<T> &l2)
{
    if(l1.pos == l2.pos && l1.dir == l2.dir)
        return true;
    else
        return false;
}

template <class T>
static bool
notequal(const Line3<T> &l1, const Line3<T> &l2)
{
    if(l1.pos != l2.pos || l1.dir != l2.dir)
        return true;
    else
        return false;
}

template <class T>
class_<Line3<T> >
register_Line()
{
    const char *name = LineName<T>::value;
    
    class_<Line3<T> > line_class(name);
    line_class
        .def("__init__", make_constructor(Line3_construct_default<T>), "initialize point to (0,0,0) and direction to (1,0,0)")
        .def("__init__", make_constructor(Line3_tuple_construct<T>))
        .def("__init__", make_constructor(Line3_line_construct<T,float>))
        .def("__init__", make_constructor(Line3_line_construct<T,double>))
        .def(init<const Vec3<float> &, const Vec3<float> &>("Line3(point1, point2) construction"))
        .def(init<const Vec3<double> &, const Vec3<double> &>("Line3(point1, point2) construction"))
        .def(self * Matrix44<T>())
        .def("__eq__", &equal<T>)
        .def("__ne__", &notequal<T>)
        
        .def_readwrite("pos", &Line3<T>::pos)
        .def_readwrite("dir", &Line3<T>::dir)

        .def("pos", &getPosition<T>, 
        "l.pos() -- returns the start point of line l")
        
        .def("dir", &getDirection<T>, 
        "l.dir() -- returns the direction of line l\n")
        
        .def("setPos", &setPosition<T>, 
        "l.setPos(p) -- sets the start point of line l to p")
        .def("setPos", &setPositionTuple<T>)
        
        .def("setDir", &setDirection<T>, 
        "l.setDir(d) -- sets the direction of line l\n"
		"to d.normalized().\n")
        .def("setDir", &setDirectionTuple<T>)
                                            
        .def("set", &set1<T>, 
        "l.set(p1, p2) -- sets the start point\n"
		"and direction of line l by calling\n"
        "   l.setPos (p1)\n"
        "   l.setDir (p2 - p1)\n")
        
        .def("set", &setTuple<T>)
        
        .def("pointAt", &pointAt<T>,
        "l.pointAt(t) -- returns l.pos() + t * l.dir()")
        
        .def("distanceTo", &distanceTo1<T>, 
        "l.distanceTo(p) -- returns the distance from\n"
		"   line l to point p\n")
                                        
        .def("distanceTo", &distanceTo2<T>, 
        "l1.distanceTo(l2) -- returns the distance from\n"
        "   line l1 to line l2\n")
        
        .def("distanceTo", &distanceToTuple<T>)
                                
        .def("closestPointTo", &closestPointTo1<T>, 
        "l.closestPointTo(p) -- returns the point on\n"
		"   line l that is closest to point p\n"
        "\n")
        
        .def("closestPointTo", &closestPointToTuple<T>)
        .def("closestPointTo", &closestPointTo2<T>, 
        "l1.closestPointTo(l2) -- returns the point on\n"
		"   line l1 that is closest to line l2\n")
        
        .def("closestPoints", &closestPoints1<T>, 
        "l1.closestPoints(l2,p0,p1)")    
                                            
        .def("closestPoints", &closestPoints2<T>, 
        "l1.closestPoints(l2) -- returns a tuple with\n"
		"two points:\n"
        "   (l1.closestPoint(l2), l2.closestPoint(l1)\n")
                                             
        .def("closestTriangleVertex", &closestVertex<T>, 
        "l.closestTriangleVertex(v0, v1, v2) -- returns\n"
		"a copy of v0, v1, or v2, depending on which is\n"
        "closest to line l.\n")
        
        .def("closestTriangleVertex", &closestVertexTuple<T>)
        .def("intersectWithTriangle", &intersect2<T>)                                             
        .def("intersectWithTriangle", &intersect1<T>, 
            "l.intersectWithTriangle(v0, v1, v2) -- computes the\n"
			"intersection of line l and triangle (v0, v1, v2).\n"
			"\n"
			"If the line and the triangle do not intersect,\n"
			"None is returned.\n"
			""
			"If the line and the triangle intersect, a tuple\n"
			"(p, b, f) is returned:\n"
			"\n"
			"   p  intersection point in 3D space\n"
			"\n"
			"   b  intersection point in barycentric coordinates\n"
			"\n"
			"   f  1 if the line hits the triangle from the\n"
			"      front (((v2-v1) % (v1-v2)) ^ l.dir() < 0),\n"
			"      0 if the line hits the trianble from the\n"
			"      back\n"
			"\n")
        .def("intersectWithTriangle", &intersectTuple<T>)
            
        .def("rotatePoint", &rotatePoint<T>, 
            "l.rotatePoint(p,r) -- rotates point p around\n"
			"line by angle r (in radians), and returns the\n"
			"result (p is not modified)\n")
        
        .def("rotatePoint", &rotatePointTuple<T>)
        .def("__repr__",&Line3_repr<T>)
        ;

    decoratecopy(line_class);

    return line_class;
}

template PYIMATH_EXPORT class_<Line3<float> > register_Line<float>();
template PYIMATH_EXPORT class_<Line3<double> > register_Line<double>();

} // namespace PyImath
