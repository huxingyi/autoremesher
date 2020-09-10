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


#include "PyImathRandom.h"
#include "PyImathDecorators.h"
#include <Python.h>
#include <boost/python.hpp>
#include <boost/format.hpp>
#include <boost/python/make_constructor.hpp>
#include "PyImath.h"
#include "PyImathMathExc.h"
#include "PyImathFixedArray.h"


namespace PyImath{
using namespace boost::python;

template <class Rand, class T>
static T
nextf2 (Rand &rand, T min, T max)
{
    MATH_EXC_ON;
    return rand.nextf(min, max);
}

template <class Rand>
static float
nextGauss (Rand &rand)
{
    MATH_EXC_ON;
    return gaussRand(rand);
}

template <class T, class Rand>
static IMATH_NAMESPACE::Vec3<T> nextGaussSphere(Rand &rand, const IMATH_NAMESPACE::Vec3<T> &v)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::gaussSphereRand<IMATH_NAMESPACE::Vec3<T>,Rand>(rand);
}
template <class T, class Rand>
static IMATH_NAMESPACE::Vec2<T> nextGaussSphere(Rand &rand, const IMATH_NAMESPACE::Vec2<T> &v)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::gaussSphereRand<IMATH_NAMESPACE::Vec2<T>,Rand>(rand);
}

template <class T, class Rand>
static IMATH_NAMESPACE::Vec3<T> nextHollowSphere(Rand &rand, const IMATH_NAMESPACE::Vec3<T> &v)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::hollowSphereRand<IMATH_NAMESPACE::Vec3<T>,Rand>(rand);
}

template <class T, class Rand>
static IMATH_NAMESPACE::Vec2<T> nextHollowSphere(Rand &rand, const IMATH_NAMESPACE::Vec2<T> &v)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::hollowSphereRand<IMATH_NAMESPACE::Vec2<T>,Rand>(rand);
}

template <class T, class Rand>
static IMATH_NAMESPACE::Vec3<T> nextSolidSphere(Rand &rand, const IMATH_NAMESPACE::Vec3<T> &v)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::solidSphereRand<IMATH_NAMESPACE::Vec3<T>,Rand>(rand);
}

template <class T, class Rand>
static IMATH_NAMESPACE::Vec2<T> nextSolidSphere(Rand &rand, const IMATH_NAMESPACE::Vec2<T> &v)
{
    MATH_EXC_ON;
    return IMATH_NAMESPACE::solidSphereRand<IMATH_NAMESPACE::Vec2<T>,Rand>(rand);
}

template <class Rand>
static Rand *Rand_constructor1(unsigned long int seed)
{
    return new Rand(seed);
}

template <class Rand>
static Rand *Rand_constructor2(Rand rand)
{
    Rand *r = new Rand();
    *r = rand;
    
    return r;
}

template <class T, class Rand>
static PyImath::FixedArray<IMATH_NAMESPACE::Vec3<T> >
hollowSphereRand(Rand &rand, int num)
{
    MATH_EXC_ON;
    PyImath::FixedArray<IMATH_NAMESPACE::Vec3<T> >  retval(num);
    for (int i=0; i<num; ++i) {
        retval[i] = IMATH_NAMESPACE::hollowSphereRand<IMATH_NAMESPACE::Vec3<T>,Rand>(rand);
    }
    return retval;
}

template <class T, class Rand>
static PyImath::FixedArray<IMATH_NAMESPACE::Vec3<T> >
solidSphereRand(Rand &rand, int num)
{
    MATH_EXC_ON;
    PyImath::FixedArray<IMATH_NAMESPACE::Vec3<T> >  retval(num);
    for (int i=0; i<num; ++i) {
        retval[i] = IMATH_NAMESPACE::solidSphereRand<IMATH_NAMESPACE::Vec3<T>,Rand>(rand);
    }
    return retval;
}

PYIMATH_EXPORT
class_<IMATH_NAMESPACE::Rand32>
register_Rand32()
{
    float (IMATH_NAMESPACE::Rand32::*nextf1)(void) = &IMATH_NAMESPACE::Rand32::nextf;
    
    IMATH_NAMESPACE::Vec3<float> (*nextGaussSphere1)(IMATH_NAMESPACE::Rand32 &, const IMATH_NAMESPACE::Vec3<float> &v) = &nextGaussSphere<float,IMATH_NAMESPACE::Rand32>;
    IMATH_NAMESPACE::Vec3<double> (*nextGaussSphere2)(IMATH_NAMESPACE::Rand32 &, const IMATH_NAMESPACE::Vec3<double> &v) = &nextGaussSphere<double,IMATH_NAMESPACE::Rand32>;
    IMATH_NAMESPACE::Vec2<float> (*nextGaussSphere3)(IMATH_NAMESPACE::Rand32 &, const IMATH_NAMESPACE::Vec2<float> &v) = &nextGaussSphere<float,IMATH_NAMESPACE::Rand32>;
    IMATH_NAMESPACE::Vec2<double> (*nextGaussSphere4)(IMATH_NAMESPACE::Rand32 &, const IMATH_NAMESPACE::Vec2<double> &v) = &nextGaussSphere<double,IMATH_NAMESPACE::Rand32>;
    
    IMATH_NAMESPACE::Vec3<float> (*nextHollowSphere1)(IMATH_NAMESPACE::Rand32 &, const IMATH_NAMESPACE::Vec3<float> &v) = &nextHollowSphere<float,IMATH_NAMESPACE::Rand32>;
    IMATH_NAMESPACE::Vec3<double> (*nextHollowSphere2)(IMATH_NAMESPACE::Rand32 &, const IMATH_NAMESPACE::Vec3<double> &v) = &nextHollowSphere<double,IMATH_NAMESPACE::Rand32>;
    IMATH_NAMESPACE::Vec2<float> (*nextHollowSphere3)(IMATH_NAMESPACE::Rand32 &, const IMATH_NAMESPACE::Vec2<float> &v) = &nextHollowSphere<float,IMATH_NAMESPACE::Rand32>;
    IMATH_NAMESPACE::Vec2<double> (*nextHollowSphere4)(IMATH_NAMESPACE::Rand32 &, const IMATH_NAMESPACE::Vec2<double> &v) = &nextHollowSphere<double,IMATH_NAMESPACE::Rand32>;

    IMATH_NAMESPACE::Vec3<float> (*nextSolidSphere1)(IMATH_NAMESPACE::Rand32 &, const IMATH_NAMESPACE::Vec3<float> &v) = &nextSolidSphere<float,IMATH_NAMESPACE::Rand32>;
    IMATH_NAMESPACE::Vec3<double> (*nextSolidSphere2)(IMATH_NAMESPACE::Rand32 &, const IMATH_NAMESPACE::Vec3<double> &v) = &nextSolidSphere<double,IMATH_NAMESPACE::Rand32>;
    IMATH_NAMESPACE::Vec2<float> (*nextSolidSphere3)(IMATH_NAMESPACE::Rand32 &, const IMATH_NAMESPACE::Vec2<float> &v) = &nextSolidSphere<float,IMATH_NAMESPACE::Rand32>;
    IMATH_NAMESPACE::Vec2<double> (*nextSolidSphere4)(IMATH_NAMESPACE::Rand32 &, const IMATH_NAMESPACE::Vec2<double> &v) = &nextSolidSphere<double,IMATH_NAMESPACE::Rand32>;
    
    class_< IMATH_NAMESPACE::Rand32 > rand32_class("Rand32");
    rand32_class
        .def(init<>("default construction"))
        .def("__init__", make_constructor(Rand_constructor1<IMATH_NAMESPACE::Rand32>))
        .def("__init__", make_constructor(Rand_constructor2<IMATH_NAMESPACE::Rand32>))
        .def("init", &IMATH_NAMESPACE::Rand32::init,
             "r.init(i) -- initialize with integer "
			 "seed i")
             
        .def("nexti", &IMATH_NAMESPACE::Rand32::nexti,
        	 "r.nexti() -- return the next integer "
			 "value in the uniformly-distributed "
			 "sequence")
        .def("nextf", nextf1,
        	 "r.nextf() -- return the next floating-point "
			 "value in the uniformly-distributed "
			 "sequence\n"
             
        	 "r.nextf(float, float) -- return the next floating-point "
			 "value in the uniformly-distributed "
			 "sequence")             
        .def("nextf", &nextf2 <IMATH_NAMESPACE::Rand32, float>)
             
        .def("nextb", &IMATH_NAMESPACE::Rand32::nextb,
	 	     "r.nextb() -- return the next boolean "
			 "value in the uniformly-distributed "
			 "sequence")

        .def("nextGauss", &nextGauss<IMATH_NAMESPACE::Rand32>,
        	 "r.nextGauss() -- returns the next "
			 "floating-point value in the normally "
			 "(Gaussian) distributed sequence")
             
        .def("nextGaussSphere", nextGaussSphere1, 
	 		 "r.nextGaussSphere(v) -- returns the next "
			 "point whose distance from the origin "
			 "has a normal (Gaussian) distribution with "
			 "mean 0 and variance 1.  The vector "
			 "argument, v, specifies the dimension "
			 "and number type.")             
        .def("nextGaussSphere", nextGaussSphere2)             
        .def("nextGaussSphere", nextGaussSphere3)             
        .def("nextGaussSphere", nextGaussSphere4)
        
        .def("nextHollowSphere", nextHollowSphere1,
        	 "r.nextHollowSphere(v) -- return the next "
	 		 "point uniformly distributed on the surface "
	 		 "of a sphere of radius 1 centered at the "
	 		 "origin.  The vector argument, v, specifies "
			 "the dimension and number type.")             
        .def("nextHollowSphere", nextHollowSphere2)             
        .def("nextHollowSphere", nextHollowSphere3)             
        .def("nextHollowSphere", nextHollowSphere4)

        .def("nextSolidSphere", nextSolidSphere1,
        	 "r.nextSolidSphere(v) -- return the next "
			 "point uniformly distributed in a sphere "
			 "of radius 1 centered at the origin.  The "
			 "vector argument, v, specifies the "
			 "dimension and number type.")             
        .def("nextSolidSphere", nextSolidSphere2)             
        .def("nextSolidSphere", nextSolidSphere3)             
        .def("nextSolidSphere", nextSolidSphere4)    
        ;

    def("hollowSphereRand",&hollowSphereRand<float,IMATH_NAMESPACE::Rand32>,"hollowSphereRand(randObj,num) return XYZ vectors uniformly "
        "distributed across the surface of a sphere generated from the given Rand32 object",
        args("randObj","num"));
        
    def("solidSphereRand",&solidSphereRand<float,IMATH_NAMESPACE::Rand32>,"solidSphereRand(randObj,num) return XYZ vectors uniformly "
        "distributed through the volume of a sphere generated from the given Rand32 object",
        args("randObj","num"));

    decoratecopy(rand32_class);

    return rand32_class;
}

PYIMATH_EXPORT
class_<IMATH_NAMESPACE::Rand48>
register_Rand48()
{
    double (IMATH_NAMESPACE::Rand48::*nextf1)(void) = &IMATH_NAMESPACE::Rand48::nextf;
    
    IMATH_NAMESPACE::Vec3<float> (*nextGaussSphere1)(IMATH_NAMESPACE::Rand48 &, const IMATH_NAMESPACE::Vec3<float> &v) = &nextGaussSphere<float,IMATH_NAMESPACE::Rand48>;
    IMATH_NAMESPACE::Vec3<double> (*nextGaussSphere2)(IMATH_NAMESPACE::Rand48 &, const IMATH_NAMESPACE::Vec3<double> &v) = &nextGaussSphere<double,IMATH_NAMESPACE::Rand48>;
    IMATH_NAMESPACE::Vec2<float> (*nextGaussSphere3)(IMATH_NAMESPACE::Rand48&, const IMATH_NAMESPACE::Vec2<float> &v) = &nextGaussSphere<float,IMATH_NAMESPACE::Rand48>;
    IMATH_NAMESPACE::Vec2<double> (*nextGaussSphere4)(IMATH_NAMESPACE::Rand48 &, const IMATH_NAMESPACE::Vec2<double> &v) = &nextGaussSphere<double,IMATH_NAMESPACE::Rand48>;
    
    IMATH_NAMESPACE::Vec3<float> (*nextHollowSphere1)(IMATH_NAMESPACE::Rand48 &, const IMATH_NAMESPACE::Vec3<float> &v) = &nextHollowSphere<float,IMATH_NAMESPACE::Rand48>;
    IMATH_NAMESPACE::Vec3<double> (*nextHollowSphere2)(IMATH_NAMESPACE::Rand48 &, const IMATH_NAMESPACE::Vec3<double> &v) = &nextHollowSphere<double,IMATH_NAMESPACE::Rand48>;
    IMATH_NAMESPACE::Vec2<float> (*nextHollowSphere3)(IMATH_NAMESPACE::Rand48 &, const IMATH_NAMESPACE::Vec2<float> &v) = &nextHollowSphere<float,IMATH_NAMESPACE::Rand48>;
    IMATH_NAMESPACE::Vec2<double> (*nextHollowSphere4)(IMATH_NAMESPACE::Rand48 &, const IMATH_NAMESPACE::Vec2<double> &v) = &nextHollowSphere<double,IMATH_NAMESPACE::Rand48>;

    IMATH_NAMESPACE::Vec3<float> (*nextSolidSphere1)(IMATH_NAMESPACE::Rand48 &, const IMATH_NAMESPACE::Vec3<float> &v) = &nextSolidSphere<float,IMATH_NAMESPACE::Rand48>;
    IMATH_NAMESPACE::Vec3<double> (*nextSolidSphere2)(IMATH_NAMESPACE::Rand48 &, const IMATH_NAMESPACE::Vec3<double> &v) = &nextSolidSphere<double,IMATH_NAMESPACE::Rand48>;
    IMATH_NAMESPACE::Vec2<float> (*nextSolidSphere3)(IMATH_NAMESPACE::Rand48&, const IMATH_NAMESPACE::Vec2<float> &v) = &nextSolidSphere<float,IMATH_NAMESPACE::Rand48>;
    IMATH_NAMESPACE::Vec2<double> (*nextSolidSphere4)(IMATH_NAMESPACE::Rand48 &, const IMATH_NAMESPACE::Vec2<double> &v) = &nextSolidSphere<double,IMATH_NAMESPACE::Rand48>;
   
    class_< IMATH_NAMESPACE::Rand48 > rand48_class("Rand48");
    rand48_class
        .def(init<>("default construction"))
        .def("__init__", make_constructor(Rand_constructor1<IMATH_NAMESPACE::Rand48>))
        .def("__init__", make_constructor(Rand_constructor2<IMATH_NAMESPACE::Rand48>))
        .def("init", &IMATH_NAMESPACE::Rand48::init,
             "r.init(i) -- initialize with integer "
			 "seed i")
             
        .def("nexti", &IMATH_NAMESPACE::Rand48::nexti,
        	 "r.nexti() -- return the next integer "
			 "value in the uniformly-distributed "
			 "sequence")
             
        .def("nextf", nextf1,
        	 "r.nextf() -- return the next double "
			 "value in the uniformly-distributed "
			 "sequence\n"
             
        	 "r.nextf(double,double) -- return the next double "
			 "value in the uniformly-distributed "
			 "sequence")             
        .def("nextf", &nextf2 <IMATH_NAMESPACE::Rand48, double>)
             
        .def("nextb", &IMATH_NAMESPACE::Rand48::nextb,
	 	     "r.nextb() -- return the next boolean "
			 "value in the uniformly-distributed "
			 "sequence")
 
        .def("nextGauss", &nextGauss<IMATH_NAMESPACE::Rand48>,
        	 "r.nextGauss() -- returns the next "
			 "floating-point value in the normally "
			 "(Gaussian) distributed sequence")
             
        .def("nextGaussSphere", nextGaussSphere1, 
	 		 "r.nextGaussSphere(v) -- returns the next "
			 "point whose distance from the origin "
			 "has a normal (Gaussian) distribution with "
			 "mean 0 and variance 1.  The vector "
			 "argument, v, specifies the dimension "
			 "and number type.")             
        .def("nextGaussSphere", nextGaussSphere2)             
        .def("nextGaussSphere", nextGaussSphere3)             
        .def("nextGaussSphere", nextGaussSphere4)
        
        .def("nextHollowSphere", nextHollowSphere1,
        	 "r.nextHollowSphere(v) -- return the next "
	 		 "point uniformly distributed on the surface "
	 		 "of a sphere of radius 1 centered at the "
	 		 "origin.  The vector argument, v, specifies "
			 "the dimension and number type.")             
        .def("nextHollowSphere", nextHollowSphere2)             
        .def("nextHollowSphere", nextHollowSphere3)             
        .def("nextHollowSphere", nextHollowSphere4)

        .def("nextSolidSphere", nextSolidSphere1,
        	 "r.nextSolidSphere(v) -- return the next "
			 "point uniformly distributed in a sphere "
			 "of radius 1 centered at the origin.  The "
			 "vector argument, v, specifies the "
			 "dimension and number type.")             
        .def("nextSolidSphere", nextSolidSphere2)             
        .def("nextSolidSphere", nextSolidSphere3)             
        .def("nextSolidSphere", nextSolidSphere4) 
        ;

    decoratecopy(rand48_class);

    return rand48_class;
}

//

PyObject *
Rand32::wrap (const IMATH_NAMESPACE::Rand32 &r)
{
    boost::python::return_by_value::apply <IMATH_NAMESPACE::Rand32>::type converter;
    PyObject *p = converter (r);
    return p;
}

PyObject *
Rand48::wrap (const IMATH_NAMESPACE::Rand48 &r)
{
    boost::python::return_by_value::apply <IMATH_NAMESPACE::Rand48>::type converter;
    PyObject *p = converter (r);
    return p;
}

} //namespace PyIMath
