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

#include "PyImathBox.h"
#include "PyImathVec.h"
#include "PyImathMathExc.h"
#include "PyImathDecorators.h"
#include "PyImathExport.h"
#include <boost/python/make_constructor.hpp>
#include <Iex.h>
#include <ImathBoxAlgo.h>
#include "PyImathTask.h"
#include <vector>
#include "PyImathBoxArrayImpl.h"

namespace PyImath {
using namespace boost::python;
using namespace IMATH_NAMESPACE;
using namespace PyImath;

template <class T> struct BoxName { static const char *value; };
template <> const char *BoxName<IMATH_NAMESPACE::V2s>::value = "Box2s";
template <> const char *BoxName<IMATH_NAMESPACE::V2i>::value = "Box2i";
template <> const char *BoxName<IMATH_NAMESPACE::V2f>::value = "Box2f";
template <> const char *BoxName<IMATH_NAMESPACE::V2d>::value = "Box2d";
template <> const char *BoxName<IMATH_NAMESPACE::V3s>::value = "Box3s";
template <> const char *BoxName<IMATH_NAMESPACE::V3i>::value = "Box3i";
template <> const char *BoxName<IMATH_NAMESPACE::V3f>::value = "Box3f";
template <> const char *BoxName<IMATH_NAMESPACE::V3d>::value = "Box3d";

template <> PYIMATH_EXPORT const char *PyImath::Box2sArray::name() { return "Box2sArray"; }
template <> PYIMATH_EXPORT const char *PyImath::Box2iArray::name() { return "Box2iArray"; }
template <> PYIMATH_EXPORT const char *PyImath::Box2fArray::name() { return "Box2fArray"; }
template <> PYIMATH_EXPORT const char *PyImath::Box2dArray::name() { return "Box2dArray"; }
template <> PYIMATH_EXPORT const char *PyImath::Box3sArray::name() { return "Box3sArray"; }
template <> PYIMATH_EXPORT const char *PyImath::Box3iArray::name() { return "Box3iArray"; }
template <> PYIMATH_EXPORT const char *PyImath::Box3fArray::name() { return "Box3fArray"; }
template <> PYIMATH_EXPORT const char *PyImath::Box3dArray::name() { return "Box3dArray"; }

template <class T>
static Box<T> * box2TupleConstructor1(const tuple &t)
{
    if(t.attr("__len__")() == 2)
    {
        // The constructor was called like this:
        // Box2f ((V2f(1,2), V2f(3,4))) or
        // Box2f (((1,2), (3,4)))

        PyObject *t0Obj = extract <object> (t[0])().ptr();
        PyObject *t1Obj = extract <object> (t[1])().ptr();
        T t0, t1;
        if (V2<typename T::BaseType>::convert (t0Obj, &t0) &&
            V2<typename T::BaseType>::convert (t1Obj, &t1))
        {
            return new Box<T> (t0, t1);
        }

        // The constructor was called like this:
        // Box2f ((1,2))

        else
        {
            T point;
            point.x = extract<double>(t[0]);
            point.y = extract<double>(t[1]);
            return new Box<T>(point);
        }
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Invalid input to Box tuple constructor");
}

template <class T>
static Box<T> * box2TupleConstructor2(const tuple &t0, const tuple &t1)
{
    if(t0.attr("__len__")() == 2 && t1.attr("__len__")() == 2)
    {
        T point0, point1;
        point0.x = extract<double>(t0[0]); point0.y = extract<double>(t0[1]);
        point1.x = extract<double>(t1[0]); point1.y = extract<double>(t1[1]);
        
        return new Box<T>(point0, point1);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Invalid input to Box tuple constructor");
}

template <class T, class S>
static Box<T> *boxConstructor(const Box<S> &box)
{
    Box<T> *newBox = new Box<T>;
    newBox->min = box.min;
    newBox->max = box.max;
    
    return newBox;
}

template <class T>
static Box<T> * box3TupleConstructor1(const tuple &t)
{
    if(t.attr("__len__")() == 3)
    {
        // The constructor was called like this:
        // Box3f ((1,2,3))

        T point;
        point.x = extract<double>(t[0]);
        point.y = extract<double>(t[1]);
        point.z = extract<double>(t[2]);
        return new Box<T>(point);
    }

    else if (t.attr("__len__")() == 2)
    {
        // The constructor was called like this:
        // Box3f ((V3f(1,2,3), V3f(4,5,6))) or
        // Box3f (((1,2,3), (4,5,6)))

        PyObject *t0Obj = extract <object> (t[0])().ptr();
        PyObject *t1Obj = extract <object> (t[1])().ptr();
        T t0, t1;
        if (V3<typename T::BaseType>::convert (t0Obj, &t0) &&
            V3<typename T::BaseType>::convert (t1Obj, &t1))
        {
            return new Box<T> (t0, t1);
        }
        else
            THROW(IEX_NAMESPACE::LogicExc, "Invalid input to Box tuple constructor");
    }

    else
        THROW(IEX_NAMESPACE::LogicExc, "Invalid input to Box tuple constructor");
}

template <class T>
static Box<T> * box3TupleConstructor2(const tuple &t0, const tuple &t1)
{
    if(t0.attr("__len__")() == 3 && t1.attr("__len__")() == 3)
    {
        T point0, point1;
        point0.x = extract<double>(t0[0]); 
        point0.y = extract<double>(t0[1]);
        point0.z = extract<double>(t0[2]);
        
        point1.x = extract<double>(t1[0]); 
        point1.y = extract<double>(t1[1]);
        point1.z = extract<double>(t1[2]);
        
        return new Box<T>(point0, point1);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "Invalid input to Box tuple constructor");
}

template <class T>
static std::string Box2_repr(const Box<T> &box)
{
    std::stringstream stream;
    typename boost::python::return_by_value::apply <T>::type converter;

    PyObject *minObj = converter (box.min);
    PyObject *minReprObj = PyObject_Repr (minObj);
#if PY_MAJOR_VERSION > 2
    std::string minReprStr = PyUnicode_AsUTF8 (minReprObj);
#else
    std::string minReprStr = PyString_AsString (minReprObj);
#endif
    Py_DECREF (minReprObj);
    Py_DECREF (minObj);

    PyObject *maxObj = converter (box.max);
    PyObject *maxReprObj = PyObject_Repr (maxObj);
#if PY_MAJOR_VERSION > 2
    std::string maxReprStr = PyUnicode_AsUTF8 (maxReprObj);
#else
    std::string maxReprStr = PyString_AsString (maxReprObj);
#endif
    Py_DECREF (maxReprObj);
    Py_DECREF (maxObj);

    stream << BoxName<T>::value << "(" << minReprStr << ", " << maxReprStr << ")";
    
    return stream.str();
}

template <class T>
static std::string Box3_repr(const Box<T> &box)
{
    std::stringstream stream;
    typename boost::python::return_by_value::apply <T>::type converter;

    PyObject *minObj = converter (box.min);
    PyObject *minReprObj = PyObject_Repr (minObj);
#if PY_MAJOR_VERSION > 2
    std::string minReprStr = PyUnicode_AsUTF8 (minReprObj);
#else
    std::string minReprStr = PyString_AsString (minReprObj);
#endif
    Py_DECREF (minReprObj);
    Py_DECREF (minObj);

    PyObject *maxObj = converter (box.max);
    PyObject *maxReprObj = PyObject_Repr (maxObj);
#if PY_MAJOR_VERSION > 2
    std::string maxReprStr = PyUnicode_AsUTF8 (maxReprObj);
#else
    std::string maxReprStr = PyString_AsString (maxReprObj);
#endif
    Py_DECREF (maxReprObj);
    Py_DECREF (maxObj);

    stream << BoxName<T>::value << "(" << minReprStr << ", " << maxReprStr << ")";
    
    return stream.str();
}

template <class T>
static void boxSetMin(IMATH_NAMESPACE::Box<T> &box, const T &m)
{
    box.min = m;
}

template <class T>
static void boxSetMax(IMATH_NAMESPACE::Box<T> &box, const T &m)
{
    box.max = m;
}

template <class T>
static T
boxMin(IMATH_NAMESPACE::Box<T> &box)
{
    return box.min;
}

template <class T>
static T
boxMax(IMATH_NAMESPACE::Box<T> &box)
{
    return box.max;
}

template <class T>
struct IntersectsTask : public Task
{
    const IMATH_NAMESPACE::Box<T>& box;
    const PyImath::FixedArray<T>& points;
    PyImath::FixedArray<int>& results;

    IntersectsTask(IMATH_NAMESPACE::Box<T>& b, const PyImath::FixedArray<T> &p, PyImath::FixedArray<int> &r)
        : box(b), points(p), results(r) {}

    void execute(size_t start, size_t end)
    {
        for(size_t p = start; p < end; ++p) 
            results[p] = box.intersects(points[p]);
    }
};

template <class T>
struct ExtendByTask : public Task
{
    std::vector<IMATH_NAMESPACE::Box<T> >& boxes;
    const PyImath::FixedArray<T>& points;

    ExtendByTask(std::vector<IMATH_NAMESPACE::Box<T> >& b, const PyImath::FixedArray<T> &p)
        : boxes(b), points(p) {}

    void execute(size_t start, size_t end, int tid)
    {
        for(size_t p = start; p < end; ++p) 
            boxes[tid].extendBy(points[p]);
    }
    void execute(size_t start, size_t end)
    {
        ASSERT(false, IEX_NAMESPACE::NoImplExc, "Box::ExtendBy execute requires a thread id");
    }
};

template <class T>
static void
box_extendBy(IMATH_NAMESPACE::Box<T> &box, const PyImath::FixedArray<T> &points)
{
    size_t numBoxes = workers();
    std::vector<IMATH_NAMESPACE::Box<T> > boxes(numBoxes);
    ExtendByTask<T> task(boxes,points);
    dispatchTask(task,points.len());
    for (size_t i=0; i<numBoxes; ++i) {
        box.extendBy(boxes[i]);
    }
}

template <class T>
PyImath::FixedArray<int>
box_intersects(IMATH_NAMESPACE::Box<T>& box, const PyImath::FixedArray<T>& points)
{
    size_t numPoints = points.len();
    PyImath::FixedArray<int> mask(numPoints);

    IntersectsTask<T> task(box,points,mask);
    dispatchTask(task,numPoints);
    return mask;
}

template <class T>
class_<IMATH_NAMESPACE::Box<T> >
register_Box2()
{
    void (IMATH_NAMESPACE::Box<T>::*extendBy1)(const T&)              = &IMATH_NAMESPACE::Box<T>::extendBy;
    void (IMATH_NAMESPACE::Box<T>::*extendBy2)(const IMATH_NAMESPACE::Box<T>&)  = &IMATH_NAMESPACE::Box<T>::extendBy;
    bool (IMATH_NAMESPACE::Box<T>::*intersects1)(const T&) const              = &IMATH_NAMESPACE::Box<T>::intersects;
    bool (IMATH_NAMESPACE::Box<T>::*intersects2)(const IMATH_NAMESPACE::Box<T>&) const  = &IMATH_NAMESPACE::Box<T>::intersects;
    const char *name = BoxName<T>::value;
    class_<Box<T> > box_class(name);
    box_class
        .def(init<>("Box() create empty box") )
        .def(init<T>("Box(point)create box containing the given point") )
        .def(init<T,T>("Box(point,point) create box continaing min and max") )
        .def("__init__", make_constructor(box2TupleConstructor1<T>), "Box(point) where point is a python tuple")
        .def("__init__", make_constructor(box2TupleConstructor2<T>), "Box(point,point) where point is a python tuple")
        .def("__init__", make_constructor(boxConstructor<T, IMATH_NAMESPACE::V2f>))
        .def("__init__", make_constructor(boxConstructor<T, IMATH_NAMESPACE::V2d>))
        .def("__init__", make_constructor(boxConstructor<T, IMATH_NAMESPACE::V2i>))
        .def_readwrite("min",&Box<T>::min)
        .def_readwrite("max",&Box<T>::max)
        .def("min", &boxMin<T>)
        .def("max", &boxMax<T>)
        .def(self == self) // NOSONAR - suppress SonarCloud bug report.
        .def(self != self) // NOSONAR - suppress SonarCloud bug report.
        .def("__repr__", &Box2_repr<T>)
        .def("makeEmpty",&Box<T>::makeEmpty,"makeEmpty() make the box empty")
        .def("makeInfinite",&Box<T>::makeInfinite,"makeInfinite() make the box cover all space")
        .def("extendBy",extendBy1,"extendBy(point) extend the box by a point")
        .def("extendBy",box_extendBy<T>,"extendBy(array) extend the box the values in the array")
        .def("extendBy",extendBy2,"extendBy(box) extend the box by a box")
        .def("size",&Box<T>::size,"size() size of the box")
        .def("center",&Box<T>::center,"center() center of the box")
        .def("intersects",intersects1,"intersects(point) returns true if the box intersects the given point")
        .def("intersects",intersects2,"intersects(box) returns true if the box intersects the given box")
        .def("majorAxis",&Box<T>::majorAxis,"majorAxis() major axis of the box")
        .def("isEmpty",&Box<T>::isEmpty,"isEmpty() returns true if the box is empty")
        .def("isInfinite",&Box<T>::isInfinite,"isInfinite() returns true if the box covers all space")
        .def("hasVolume",&Box<T>::hasVolume,"hasVolume() returns true if the box has volume")
        .def("setMin",&boxSetMin<T>,"setMin() sets the min value of the box")
        .def("setMax",&boxSetMax<T>,"setMax() sets the max value of the box")
        ;
    return box_class;
}

template <class T, class U>
static IMATH_NAMESPACE::Box<T>
mulM44 (const IMATH_NAMESPACE::Box<T> &b, const Matrix44<U> &m)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::transform (b, m);
}

template <class T, class U>
static const IMATH_NAMESPACE::Box<T> &
imulM44 (IMATH_NAMESPACE::Box<T> &b, const Matrix44<U> &m)
{
    MATH_EXC_ON;
    b = IMATH_NAMESPACE::transform (b, m);
    return b;
}

template <class T>
class_<IMATH_NAMESPACE::Box<T> >
register_Box3()
{
    void (IMATH_NAMESPACE::Box<T>::*extendBy1)(const T&)              = &IMATH_NAMESPACE::Box<T>::extendBy;
    void (IMATH_NAMESPACE::Box<T>::*extendBy2)(const IMATH_NAMESPACE::Box<T>&)  = &IMATH_NAMESPACE::Box<T>::extendBy;
    bool (IMATH_NAMESPACE::Box<T>::*intersects1)(const T&) const              = &IMATH_NAMESPACE::Box<T>::intersects;
    bool (IMATH_NAMESPACE::Box<T>::*intersects2)(const IMATH_NAMESPACE::Box<T>&) const  = &IMATH_NAMESPACE::Box<T>::intersects;
    const char *name = BoxName<T>::value;
    class_<Box<T> > box_class(name);
    box_class
        .def(init<>("Box() create empty box") )
        .def(init<T>("Box(point)create box containing the given point") )
        .def(init<T,T>("Box(point,point) create box continaing min and max") )
        .def("__init__", make_constructor(box3TupleConstructor1<T>), "Box(point) where point is a python tuple")
        .def("__init__", make_constructor(box3TupleConstructor2<T>), "Box(point,point) where point is a python tuple")
        .def("__init__", make_constructor(boxConstructor<T, IMATH_NAMESPACE::V3f>))
        .def("__init__", make_constructor(boxConstructor<T, IMATH_NAMESPACE::V3d>))
        .def("__init__", make_constructor(boxConstructor<T, IMATH_NAMESPACE::V3i>))
        .def_readwrite("min",&Box<T>::min)
        .def_readwrite("max",&Box<T>::max)
        .def(self == self) // NOSONAR - suppress SonarCloud bug report.
        .def(self != self) // NOSONAR - suppress SonarCloud bug report.
        .def("__mul__", &mulM44<T, float>)
        .def("__mul__", &mulM44<T, double>)
        .def("__imul__", &imulM44<T, float>,return_internal_reference<>())
        .def("__imul__", &imulM44<T, double>,return_internal_reference<>())
        .def("min", &boxMin<T>)
        .def("max", &boxMax<T>)
        .def("__repr__", &Box3_repr<T>)
        .def("makeEmpty",&Box<T>::makeEmpty,"makeEmpty() make the box empty")
        .def("makeInfinite",&Box<T>::makeInfinite,"makeInfinite() make the box cover all space")
        .def("extendBy",extendBy1,"extendBy(point) extend the box by a point")
        .def("extendBy",box_extendBy<T>,"extendBy(array) extend the box the values in the array")
        .def("extendBy",extendBy2,"extendBy(box) extend the box by a box")
        .def("size",&Box<T>::size,"size() size of the box")
        .def("center",&Box<T>::center,"center() center of the box")
        .def("intersects",intersects1,"intersects(point) returns true if the box intersects the given point")
        .def("intersects",intersects2,"intersects(box) returns true if the box intersects the given box")
        .def("intersects",box_intersects<T>, "intersects(array) returns an int array where 0 indicates the point is not in the box and 1 indicates that it is")
        .def("majorAxis",&Box<T>::majorAxis,"majorAxis() major axis of the box")
        .def("isEmpty",&Box<T>::isEmpty,"isEmpty() returns true if the box is empty")
        .def("isInfinite",&Box<T>::isInfinite,"isInfinite() returns true if the box covers all space")
        .def("hasVolume",&Box<T>::hasVolume,"hasVolume() returns true if the box has volume")
        .def("setMin",&boxSetMin<T>,"setMin() sets the min value of the box")
        .def("setMax",&boxSetMax<T>,"setMax() sets the max value of the box")
        ;

    decoratecopy(box_class);

    return box_class;
}


template PYIMATH_EXPORT class_<IMATH_NAMESPACE::Box<IMATH_NAMESPACE::V2s> > register_Box2<IMATH_NAMESPACE::V2s>();
template PYIMATH_EXPORT class_<IMATH_NAMESPACE::Box<IMATH_NAMESPACE::V2i> > register_Box2<IMATH_NAMESPACE::V2i>();
template PYIMATH_EXPORT class_<IMATH_NAMESPACE::Box<IMATH_NAMESPACE::V2f> > register_Box2<IMATH_NAMESPACE::V2f>();
template PYIMATH_EXPORT class_<IMATH_NAMESPACE::Box<IMATH_NAMESPACE::V2d> > register_Box2<IMATH_NAMESPACE::V2d>();
template PYIMATH_EXPORT class_<IMATH_NAMESPACE::Box<IMATH_NAMESPACE::V3s> > register_Box3<IMATH_NAMESPACE::V3s>();
template PYIMATH_EXPORT class_<IMATH_NAMESPACE::Box<IMATH_NAMESPACE::V3i> > register_Box3<IMATH_NAMESPACE::V3i>();
template PYIMATH_EXPORT class_<IMATH_NAMESPACE::Box<IMATH_NAMESPACE::V3f> > register_Box3<IMATH_NAMESPACE::V3f>();
template PYIMATH_EXPORT class_<IMATH_NAMESPACE::Box<IMATH_NAMESPACE::V3d> > register_Box3<IMATH_NAMESPACE::V3d>();

}
