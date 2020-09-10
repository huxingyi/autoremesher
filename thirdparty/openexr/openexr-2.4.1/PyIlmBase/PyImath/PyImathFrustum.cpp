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

#include "PyImathFrustum.h"
#include "PyImathDecorators.h"
#include "PyImathExport.h"
#include <Python.h>
#include <boost/python.hpp>
#include <boost/format.hpp>
#include "PyImath.h"
#include "PyImathMathExc.h"
#include "PyImathVec.h"
#include <Iex.h>

namespace PyImath{
using namespace boost::python;
using namespace IMATH_NAMESPACE;

template <class T> struct FrustumName {static const char *value;};
template <> const char *FrustumName<float>::value = "Frustumf";
template <> const char *FrustumName<double>::value = "Frustumd";

template <class T> struct FrustumTestName {static const char *value;};
template <> const char *FrustumTestName<float>::value = "FrustumTestf";
template <> const char *FrustumTestName<double>::value = "FrustumTestd";


template <class T>
static std::string Frustum_repr(const Frustum<T> &f)
{
    std::stringstream stream;
    stream << FrustumName<T>::value << "(" << f.nearPlane() << ", " << f.farPlane() << ", "
           << f.left() << ", " << f.right() << ", " << f.top() << ", "
           << f.bottom() << ", " << f.orthographic() << ")";    
    return stream.str();
}

template <class T>
static void
modifyNearAndFar(Frustum<T> &f, T nearPlane, T farPlane)
{
    MATH_EXC_ON;
    f.modifyNearAndFar (nearPlane, farPlane);
}

template <class T>
static T
fovx(Frustum<T> &f)
{
    MATH_EXC_ON;
    return f.fovx();
}

template <class T>
static T
fovy(Frustum<T> &f)
{
    MATH_EXC_ON;
    return f.fovy();
}

template <class T>
static T
aspect(Frustum<T> &f)
{
    MATH_EXC_ON;
    return f.aspect();
}

template <class T>
static Matrix44<T>
projectionMatrix(Frustum<T> &f)
{
    MATH_EXC_ON;
    return f.projectionMatrix();
}

template <class T>
static Frustum<T>
window (Frustum<T> &f, T l, T r, T b, T t)
{
    MATH_EXC_ON;
    return f.window(l, r, b, t);
}

template <class T>
static Line3<T>
projectScreenToRay (Frustum<T> &f, const Vec2<T> &p)
{
    MATH_EXC_ON;
    return f.projectScreenToRay(p);
}

template <class T>
static Line3<T>
projectScreenToRayTuple(Frustum<T> &f, const tuple &t)
{
    MATH_EXC_ON;
    if(t.attr("__len__")() == 2)
    {
        Vec2<T> point;
        point.x = extract<T>(t[0]);
        point.y = extract<T>(t[1]);
        return f.projectScreenToRay(point);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "projectScreenToRay expects tuple of length 2");
    
}

template <class T>
static Vec2<T>
projectPointToScreen (Frustum<T> &f, const Vec3<T> &p)
{
    MATH_EXC_ON;
    return f.projectPointToScreen(p);
}

template <class T>
static Vec2<T>
projectPointToScreenTuple(Frustum<T> &f, const tuple &t)
{
    MATH_EXC_ON;
    if(t.attr("__len__")() == 3)
    {
        Vec3<T> point;
        point.x = extract<T>(t[0]);
        point.y = extract<T>(t[1]);
        point.z = extract<T>(t[2]);
        return f.projectPointToScreen(point);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "projectPointToScreen expects tuple of length 3");
    
}

template <class T>
static Vec2<T>
projectPointToScreenObj(Frustum<T> &f, const object &o)
{
    MATH_EXC_ON;
    Vec3<T> v;
    if (PyImath::V3<T>::convert (o.ptr(), &v))
        return f.projectPointToScreen(v);
    else
        THROW(IEX_NAMESPACE::LogicExc, "projectPointToScreen expects tuple of length 3");
}

template <class T>
static T
ZToDepth(Frustum<T> &f, long z, long min, long max)
{
    MATH_EXC_ON;
    return f.ZToDepth(z, min, max);
}

template <class T>
static T
normalizedZToDepth(Frustum<T> &f, T z)
{
    MATH_EXC_ON;
    return f.normalizedZToDepth(z);
}

template <class T>
static long
DepthToZ(Frustum<T> &f, T depth, long min, long max)
{
    MATH_EXC_ON;
    return f.DepthToZ(depth, min, max);
}

template <class T>
static T
worldRadius(Frustum<T> &f, const Vec3<T> &p, T radius)
{
    MATH_EXC_ON;
    return f.worldRadius(p, radius);
}

template <class T>
static T
worldRadiusTuple(Frustum<T> &f, const tuple &t, T radius)
{
    MATH_EXC_ON;
    if(t.attr("__len__")() == 3)
    {
        Vec3<T> point;
        point.x = extract<T>(t[0]);
        point.y = extract<T>(t[1]);
        point.z = extract<T>(t[2]);
        return f.worldRadius(point, radius);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "worldRadius expects tuple of length 3");
}

template <class T>
static T
screenRadius(Frustum<T> &f, const Vec3<T> &p, T radius)
{
    MATH_EXC_ON;
    return f.screenRadius(p, radius);
}

template <class T>
static T
screenRadiusTuple(Frustum<T> &f, const tuple &t, T radius)
{
    MATH_EXC_ON;
    if(t.attr("__len__")() == 3)
    {
        Vec3<T> point;
        point.x = extract<T>(t[0]);
        point.y = extract<T>(t[1]);
        point.z = extract<T>(t[2]);
        return f.screenRadius(point, radius);
    }
    else
        THROW(IEX_NAMESPACE::LogicExc, "screenRadius expects tuple of length 3");
}

template <class T>
static void
planes1(Frustum<T> &f, Plane3<T> *p)
{
    MATH_EXC_ON;
    f.planes(p);
}

template <class T>
static void
planes2(Frustum<T> &f, Plane3<T> *p, const Matrix44<T> &m)
{
    MATH_EXC_ON;
    f.planes(p, m);
}

template <class T>
static tuple
planes3(Frustum<T> &f, const Matrix44<T> &mat)
{
    MATH_EXC_ON;
    IMATH_NAMESPACE::Plane3<T> p[6];
    f.planes(p,mat);
    
    tuple t = make_tuple(p[0],p[1],p[2],p[3],p[4],p[5]);
    
    return t;
}

template <class T>
static tuple
planes4(Frustum<T> &f)
{
    MATH_EXC_ON;
    IMATH_NAMESPACE::Plane3<T> p[6];
    f.planes(p);
    
    tuple t = make_tuple(p[0],p[1],p[2],p[3],p[4],p[5]);
    
    return t;
}

template <class T>
class_<Frustum<T> >
register_Frustum()
{
    void (IMATH_NAMESPACE::Frustum<T>::*set1)(T,T,T,T,T,T,bool) = &IMATH_NAMESPACE::Frustum<T>::set;
    void (IMATH_NAMESPACE::Frustum<T>::*set2)(T,T,T,T,T)        = &IMATH_NAMESPACE::Frustum<T>::set;
    const char *name = FrustumName<T>::value;
    
    class_< Frustum<T> > frustum_class(name,name,init<Frustum<T> >("copy construction"));
    frustum_class
        .def(init<>("Frustum() default construction"))
        .def(init<T,T,T,T,T,T,bool>("Frustum(nearPlane,farPlane,left,right,top,bottom,ortho) construction"))
        .def(init<T,T,T,T,T>("Frustum(nearPlane,farPlane,fovx,fovy,aspect) construction"))
        .def(self == self) // NOSONAR - suppress SonarCloud bug report.
        .def(self != self) // NOSONAR - suppress SonarCloud bug report.
        .def("__repr__",&Frustum_repr<T>)
        .def("set", set1,
        	 "F.set(nearPlane, farPlane, left, right, top, bottom, "
	 		 "[ortho])\n"
			 "F.set(nearPlane, farPlane, fovx, fovy, aspect)       "
			 "         -- sets the entire state of "
			 "frustum F as specified.  Only one of "
			 "fovx or fovy may be non-zero.")             
        .def("set", set2)
        
        .def("modifyNearAndFar", &modifyNearAndFar<T>,
        	 "F.modifyNearAndFar(nearPlane, farPlane) -- modifies "
			 "the already-valid frustum F as specified")
             
        .def("setOrthographic", &Frustum<T>::setOrthographic,
        	 "F.setOrthographic(b) -- modifies the "
			 "already-valid frustum F to be orthographic "
			 "or not")
             
        .def("nearPlane", &Frustum<T>::nearPlane,
        	 "F.nearPlane() -- returns the coordinate of the "
			 "near clipping plane of frustum F")
             
        .def("farPlane", &Frustum<T>::farPlane, 
        	 "F.farPlane() -- returns the coordinate of the "
			 "far clipping plane of frustum F")
             
        // The following two functions provide backwards compatibility
        // with the previous API for this class.

        .def("near", &Frustum<T>::nearPlane,
        	 "F.near() -- returns the coordinate of the "
			 "near clipping plane of frustum F")
             
        .def("far", &Frustum<T>::farPlane, 
        	 "F.far() -- returns the coordinate of the "
			 "far clipping plane of frustum F")
             
        .def("left", &Frustum<T>::left,
        	 "F.left() -- returns the left coordinate of "
			 "the near clipping window of frustum F")
             
        .def("right", &Frustum<T>::right,
        	 "F.right() -- returns the right coordinate of "
			 "the near clipping window of frustum F")
             
        .def("top", &Frustum<T>::top,
	 		 "F.top() -- returns the top coordinate of "
			 "the near clipping window of frustum F")
             
        .def("bottom", &Frustum<T>::bottom,
  			 "F.bottom() -- returns the bottom coordinate "
			 "of the near clipping window of frustum F")
             
        .def("orthographic", &Frustum<T>::orthographic,
        	 "F.orthographic() -- returns whether frustum "
			 "F is orthographic or not")
             
        .def("planes", planes1<T>,
	 		 "F.planes([M]) -- returns a sequence of 6 "
			 "Plane3s, the sides of the frustum F "
			 "(top, right, bottom, left, nearPlane, farPlane), "
			 "optionally transformed by the matrix M "
	 		 "if specified")
        .def("planes", planes2<T>)
        .def("planes", planes3<T>)
        .def("planes", planes4<T>)
        
        .def("fovx", &fovx<T>,
        	 "F.fovx() -- derives and returns the "
	 		 "x field of view (in radians) for frustum F")
             
        .def("fovy", &fovy<T>,
        	 "F.fovy() -- derives and returns the "
	 		 "y field of view (in radians) for frustum F")
             
        .def("aspect", &aspect<T>,
        	 "F.aspect() -- derives and returns the "
	 		 "aspect ratio for frustum F")
             
        .def("projectionMatrix", &projectionMatrix<T>,
        	 "F.projectionMatrix() -- derives and returns "
	 		 "the projection matrix for frustum F")
             
        .def("window", &window<T>,
        	 "F.window(l,r,b,t) -- takes a rectangle in "
	 		 "the screen space (i.e., -1 <= l <= r <= 1, "
			 "-1 <= b <= t <= 1) of F and returns a new "
			 "Frustum whose near clipping-plane window "
	 		 "is that rectangle in local space")
             
        .def("projectScreenToRay", &projectScreenToRay<T>, 
        	 "F.projectScreenToRay(V) -- returns a Line3 "
	 		 "through V, a V2 point in screen space")
             
        .def("projectScreenToRay", &projectScreenToRayTuple<T>)
             
        .def("projectPointToScreen", &projectPointToScreen<T>, 
        	 "F.projectPointToScreen(V) -- returns the "
			 "projection of V3 V into screen space")
             
        .def("projectPointToScreen", &projectPointToScreenTuple<T>)

        .def("projectPointToScreen", &projectPointToScreenObj<T>)
             
        .def("ZToDepth", &ZToDepth<T>,
        	 "F.ZToDepth(z, zMin, zMax) -- returns the "
			 "depth (Z in the local space of the "
			 "frustum F) corresponding to z (a result of "
			 "transformation by F's projection matrix) "
			 "after normalizing z to be between zMin "
			 "and zMax")
             
        .def("normalizedZToDepth", &normalizedZToDepth<T>,
        	 "F.normalizedZToDepth(z) -- returns the "
			 "depth (Z in the local space of the "
			 "frustum F) corresponding to z (a result of "
	 		 "transformation by F's projection matrix), "
	 		 "which is assumed to have been normalized "
	 		 "to [-1, 1]")
             
        .def("DepthToZ", &DepthToZ<T>,
        	 "F.DepthToZ(depth, zMin, zMax) -- converts "
			 "depth (Z in the local space of the frustum "
			 "F) to z (a result of  transformation by F's "
			 "projection matrix) which is normalized to "
			 "[zMin, zMax]")
             
        .def("worldRadius", &worldRadius<T>,
        	 "F.worldRadius(V, r) -- returns the radius "
			 "in F's local space corresponding to the "
	 		 "point V and radius r in screen space")
             
        .def("worldRadius", &worldRadiusTuple<T>)
             
        .def("screenRadius", &screenRadius<T>,
        	 "F.screenRadius(V, r) -- returns the radius "
			 "in screen space corresponding to "
			 "the point V and radius r in F's local "
			 "space")
             
        .def("screenRadius", &screenRadiusTuple<T>)

        ;

    decoratecopy(frustum_class);

    return frustum_class;
}

template <class T,class T2>
struct IsVisibleTask : public Task
{
    const IMATH_NAMESPACE::FrustumTest<T>& frustumTest;
    const PyImath::FixedArray<T2>& points;
    PyImath::FixedArray<int>& results;

    IsVisibleTask(const IMATH_NAMESPACE::FrustumTest<T>& ft, const PyImath::FixedArray<T2> &p, PyImath::FixedArray<int> &r)
        : frustumTest(ft), points(p), results(r) {}

    void execute(size_t start, size_t end)
    {
        for(size_t p = start; p < end; ++p)
            results[p] = frustumTest.isVisible(IMATH_NAMESPACE::Vec3<T>(points[p]));
    }
};

template <class T,class T2>
PyImath::FixedArray<int>
frustumTest_isVisible(IMATH_NAMESPACE::FrustumTest<T>& ft, const PyImath::FixedArray<T2>& points)
{
    size_t numPoints = points.len();
    PyImath::FixedArray<int> mask(numPoints);

    IsVisibleTask<T,T2> task(ft,points,mask);
    dispatchTask(task,numPoints);
    return mask;
}

template <class T>
class_<FrustumTest<T> >
register_FrustumTest()
{
    const char *name = FrustumTestName<T>::value;
    
    bool (FrustumTest<T>::*isVisibleS)(const Sphere3<T> &) const = &FrustumTest<T>::isVisible;
    bool (FrustumTest<T>::*isVisibleB)(const Box<Vec3<T> > &) const = &FrustumTest<T>::isVisible;
    bool (FrustumTest<T>::*isVisibleV)(const Vec3<T> &) const = &FrustumTest<T>::isVisible;
    bool (FrustumTest<T>::*completelyContainsS)(const Sphere3<T> &) const = &FrustumTest<T>::completelyContains;
    bool (FrustumTest<T>::*completelyContainsB)(const Box<Vec3<T> > &) const = &FrustumTest<T>::completelyContains;

    class_< FrustumTest<T> > frustumtest_class(name,name,init<const IMATH_NAMESPACE::Frustum<T>&,const IMATH_NAMESPACE::Matrix44<T>&>("create a frustum test object from a frustum and transform"));
    frustumtest_class
        .def("isVisible",isVisibleS)
        .def("isVisible",isVisibleB)
        .def("isVisible",isVisibleV)
        .def("isVisible",&frustumTest_isVisible<T,IMATH_NAMESPACE::V3f>)
        .def("completelyContains",completelyContainsS)
        .def("completelyContains",completelyContainsB)
        ;

    decoratecopy(frustumtest_class);

    return frustumtest_class;
}

template PYIMATH_EXPORT class_<Frustum<float> > register_Frustum<float>();
template PYIMATH_EXPORT class_<Frustum<double> > register_Frustum<double>();
template PYIMATH_EXPORT class_<FrustumTest<float> > register_FrustumTest<float>();
template PYIMATH_EXPORT class_<FrustumTest<double> > register_FrustumTest<double>();
}
