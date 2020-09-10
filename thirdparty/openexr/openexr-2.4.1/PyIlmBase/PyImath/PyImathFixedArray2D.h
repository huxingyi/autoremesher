///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2007-2011, Industrial Light & Magic, a division of Lucas
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

#ifndef _PyImathFixedArray2D_h_
#define _PyImathFixedArray2D_h_

#include <boost/python.hpp>
#include <boost/operators.hpp>
#include <boost/shared_array.hpp>
#include <boost/any.hpp>
#include <Iex.h>
#include <iostream>
#include "PyImathFixedArray.h"
#include "PyImathOperators.h"
#include <ImathVec.h>

namespace PyImath {

template <class T>
class FixedArray2D
{
    T *                 _ptr;
    IMATH_NAMESPACE::Vec2<size_t> _length;
    IMATH_NAMESPACE::Vec2<size_t> _stride;
    size_t              _size; //flattened size of the array

    // this handle optionally stores a shared_array to allocated array data
    // so that everything is freed properly on exit.
    boost::any _handle;

  public:

    FixedArray2D(T *ptr, Py_ssize_t lengthX, Py_ssize_t lengthY, Py_ssize_t strideX = 1)
        : _ptr(ptr), _length(lengthX, lengthY), _stride(strideX, lengthX), _handle()
    {
        if (lengthX < 0 || lengthY < 0)
            throw IEX_NAMESPACE::LogicExc("Fixed array 2d lengths must be non-negative");
        if (strideX <= 0)
            throw IEX_NAMESPACE::LogicExc("Fixed array 2d strides must be positive");
        initializeSize();
        //std::cout << "fixed array external construct" << std::endl;
        // nothing
    }

    FixedArray2D(T *ptr, Py_ssize_t lengthX, Py_ssize_t lengthY, Py_ssize_t strideX, Py_ssize_t strideY)
        : _ptr(ptr), _length(lengthX, lengthY), _stride(strideX, strideY), _handle()
    {
        if (lengthX < 0 || lengthY < 0)
            throw IEX_NAMESPACE::LogicExc("Fixed array 2d lengths must be non-negative");
        if (strideX <= 0 || strideY < 0)
            throw IEX_NAMESPACE::LogicExc("Fixed array 2d strides must be positive");
        initializeSize();
        //std::cout << "fixed array external construct" << std::endl;
        // nothing
    }

    FixedArray2D(T *ptr, Py_ssize_t lengthX, Py_ssize_t lengthY, Py_ssize_t strideX, Py_ssize_t strideY, boost::any handle) 
        : _ptr(ptr), _length(lengthX, lengthY), _stride(strideX, strideY), _handle(handle)
    {
        initializeSize();
        //std::cout << "fixed array external construct with handle" << std::endl;
        // nothing
    }

    explicit FixedArray2D(Py_ssize_t lengthX, Py_ssize_t lengthY)
        : _ptr(0), _length(lengthX, lengthY), _stride(1, lengthX), _handle()
    {
        if (lengthX < 0 || lengthY < 0)
            throw IEX_NAMESPACE::LogicExc("Fixed array 2d lengths must be non-negative");
        initializeSize();
        T tmp = FixedArrayDefaultValue<T>::value();
        boost::shared_array<T> a(new T[_size]);
        for (size_t i=0; i<_size; ++i) a[i] = tmp;
        _handle = a;
        _ptr = a.get();
    }

    explicit FixedArray2D(const IMATH_NAMESPACE::V2i& length)
        : _ptr(0), _length(length), _stride(1, length.x), _handle()
    {
        if (length.x < 0 || length.y < 0)
            throw IEX_NAMESPACE::LogicExc("Fixed array 2d lengths must be non-negative");
        initializeSize();
        T tmp = FixedArrayDefaultValue<T>::value();
        boost::shared_array<T> a(new T[_size]);
        for (size_t i=0; i<_size; ++i) a[i] = tmp;
        _handle = a;
        _ptr = a.get();
    }

    FixedArray2D(const T &initialValue, Py_ssize_t lengthX, Py_ssize_t lengthY)
        : _ptr(0), _length(lengthX, lengthY), _stride(1, lengthX), _handle()
    {
        if (lengthX < 0 || lengthY < 0)
            throw IEX_NAMESPACE::LogicExc("Fixed array 2d lengths must be non-negative");
        initializeSize();
        boost::shared_array<T> a(new T[_size]);
        for (size_t i=0; i<_size; ++i) a[i] = initialValue;
        _handle = a;
        _ptr = a.get();
    }
    void initializeSize()
    {
        _size = _length.x*_length.y;
    }

    template <class S>
    explicit FixedArray2D(const FixedArray2D<S> &other)
        : _ptr(0), _length(other.len()), _stride(1, other.len().x), _handle()
    {
        initializeSize();
        boost::shared_array<T> a(new T[_size]);
        size_t z = 0;
        for (size_t j = 0; j < _length.y; ++j)
            for (size_t i = 0; i < _length.x; ++i)
                a[z++] = T(other(i,j));
        _handle = a;
        _ptr = a.get();
    }

    FixedArray2D(const FixedArray2D &other)
        : _ptr(other._ptr), _length(other._length), _stride(other._stride), _size(other._size), _handle(other._handle)
    {
        //std::cout << "fixed array copy consturct construct" << std::endl;
        // nothing
    }
        
    const FixedArray2D &
    operator = (const FixedArray2D &other)
    {
        if (&other == this) return *this;

        //std::cout << "fixed array assign" << std::endl;

        _ptr = other._ptr;
        _length = other._length;
        _stride = other._stride;
        _handle = other._handle;

        _size = _length.x*_length.y;

        return *this;
    }

    ~FixedArray2D()
    {
        //std::cout << "fixed array delete" << std::endl;
    }

    const boost::any & handle() { return _handle; }

    size_t canonical_index(Py_ssize_t index, size_t length) const
    {
        if (index < 0) index += length;
        if ((size_t) index >= length || index < 0) {
            PyErr_SetString(PyExc_IndexError, "Index out of range");
            boost::python::throw_error_already_set();
        }
        return index;
    }

    void extract_slice_indices(PyObject *index, size_t length, size_t &start, size_t &end, Py_ssize_t &step, size_t &slicelength) const
    {
        if (PySlice_Check(index)) {
#if PY_MAJOR_VERSION > 2
            PyObject *slice = index;
#else
            PySliceObject *slice = reinterpret_cast<PySliceObject *>(index);
#endif
            Py_ssize_t s, e, sl;
            if (PySlice_GetIndicesEx(slice,length,&s,&e,&step,&sl) == -1) {
                boost::python::throw_error_already_set();
            }
            if (s < 0 || e < 0 || sl < 0) {
                throw IEX_NAMESPACE::LogicExc("Slice extraction produced invalid start, end, or length indices");
            }
            start = s;
            end = e;
            slicelength = sl;
#if PY_MAJOR_VERSION > 2
        } else if (PyLong_Check(index)) {
            size_t i = canonical_index(PyLong_AsSsize_t(index), length);
#else
        } else if (PyInt_Check(index)) {
            size_t i = canonical_index(PyInt_AsSsize_t(index), length);
#endif
            start = i; end = i+1; step = 1; slicelength = 1;
        } else {
            PyErr_SetString(PyExc_TypeError, "Object is not a slice");
	    boost::python::throw_error_already_set();
        }
        //std::cout << "Slice indices are " << start << " " << end << " " << step << " " << slicelength << std::endl;
    }

    // return_internal_reference doesn't seem to work with non-class types
    typedef typename boost::mpl::if_<boost::is_class<T>,T&,T>::type get_type;
//    get_type    getitem(Py_ssize_t index) const { return _ptr[canonical_index(index)*_stride]; }
    //FIXME: const does not work here with at least IMATH_NAMESPACE::Color4, why it works for V3fArray?
    get_type getitem(Py_ssize_t i, Py_ssize_t j) //const
    {
        return (*this)(canonical_index(i, _length.x), canonical_index(j, _length.y));
    }

    //FIXME: anyway to seperate 2:3,4:5 from 2,4? we'd like to return int for the second one, and also 1d array for 2, 4:5 or 2:3, 4
    FixedArray2D getslice(PyObject *index) const
    {
        if (PyTuple_Check(index) && PyTuple_Size(index) == 2)
        {
            size_t startx=0, endx=0, slicelengthx=0;
            size_t starty=0, endy=0, slicelengthy=0;
            Py_ssize_t stepx=0;
            Py_ssize_t stepy=0;
            extract_slice_indices(PyTuple_GetItem(index, 0),_length.x,startx,endx,stepx,slicelengthx);
            extract_slice_indices(PyTuple_GetItem(index, 1),_length.y,starty,endy,stepy,slicelengthy);
            FixedArray2D f(slicelengthx, slicelengthy);
            for (size_t j=0,z=0; j<slicelengthy; j++)
                for (size_t i=0; i<slicelengthx; ++i)
                    f._ptr[z++] = (*this)(startx+i*stepx, starty+j*stepy);
            return f;
        }
        else
        {
            PyErr_SetString(PyExc_TypeError, "Slice syntax error");
            boost::python::throw_error_already_set();
        }
        return FixedArray2D(0,0);
    }

    //FIXME: for 2D array, cannot reduce the size, or maybe returning 1D array?
    FixedArray2D getslice_mask(const FixedArray2D<int> &mask) const
    {
//         size_t len = match_dimension(mask);
//         size_t slicelength = 0;
//         for (size_t i=0; i<len; ++i) if (mask[i]) slicelength++;
//         FixedArray2D f(slicelength, _length.y);
//         for (size_t i=0,z=0; i<len; ++i) {
//             if (mask[i]) {
//                 for (size_t j = 0; j < _length.y; j++)
//                     f._ptr[z++] = (*this)(i,j);
//             }
//         }
//         return f;
        IMATH_NAMESPACE::Vec2<size_t> len = match_dimension(mask);
        FixedArray2D f(len);
        for (size_t j=0; j<len.y; j++)
            for (size_t i=0; i<len.x; i++)
                if (mask(i,j))
                    f(i,j) = (*this)(i,j);
        return f;
    }

//     void setitem(const boost::python::tuple& index, const T &data)
//     {
//         Py_ssize_t i = boost::python::extract<Py_ssize_t>(index[0]);
//         Py_ssize_t j = boost::python::extract<Py_ssize_t>(index[1]);
//         (*this)(i,j) = data;
//     }
    void
    setitem_scalar(PyObject *index, const T &data)
    {
        if (!PyTuple_Check(index) || PyTuple_Size(index) != 2)
        {
            PyErr_SetString(PyExc_TypeError, "Slice syntax error");
            boost::python::throw_error_already_set();
        }

        size_t startx=0, endx=0, slicelengthx=0;
        size_t starty=0, endy=0, slicelengthy=0;
        Py_ssize_t stepx=0;
        Py_ssize_t stepy=0;
        extract_slice_indices(PyTuple_GetItem(index, 0),_length.x,startx,endx,stepx,slicelengthx);
        extract_slice_indices(PyTuple_GetItem(index, 1),_length.y,starty,endy,stepy,slicelengthy);
        for (size_t j=0; j<slicelengthy; j++)
            for (size_t i=0; i<slicelengthx; ++i)
                (*this)(startx+i*stepx, starty+j*stepy) = data;
    }

    void
    setitem_scalar_mask(const FixedArray2D<int> &mask, const T &data)
    {
        IMATH_NAMESPACE::Vec2<size_t> len = match_dimension(mask);
        for (size_t j = 0; j < len.y; j++)
            for (size_t i=0; i<len.x; ++i)
                if (mask(i,j))
                    (*this)(i,j) = data;
    }

    void
    setitem_vector(PyObject *index, const FixedArray2D &data)
    {
        //TODO:sanity check
        size_t startx=0, endx=0, slicelengthx=0;
        size_t starty=0, endy=0, slicelengthy=0;
        Py_ssize_t stepx=0;
        Py_ssize_t stepy=0;
        extract_slice_indices(PyTuple_GetItem(index, 0),_length.x,startx,endx,stepx,slicelengthx);
        extract_slice_indices(PyTuple_GetItem(index, 1),_length.y,starty,endy,stepy,slicelengthy);
        // we have a valid range of indices
        if (data.len() != IMATH_NAMESPACE::Vec2<size_t>(slicelengthx, slicelengthy)) {
            PyErr_SetString(PyExc_IndexError, "Dimensions of source do not match destination");
            boost::python::throw_error_already_set();
        }
        for (size_t i=0; i<slicelengthx; ++i)
            for (size_t j=0; j<slicelengthy; ++j)
                (*this)(startx+i*stepx, starty+j*stepy) = data(i,j);
    }

    void
    setitem_vector_mask(const FixedArray2D<int> &mask, const FixedArray2D &data)
    {
        IMATH_NAMESPACE::Vec2<size_t> len = match_dimension(mask);
        if (data.len() == len) {
            for (size_t j = 0; j < len.y; j++)
                for (size_t i=0; i<len.x; ++i)
                    if (mask(i,j))
                        (*this)(i,j) = data(i,j);
        } else {
            PyErr_SetString(PyExc_IndexError, "Dimensions of source data do not match destination");
            boost::python::throw_error_already_set();
        }
    }

    void
    setitem_array1d_mask(const FixedArray2D<int> &mask, const FixedArray<T> &data)
    {
        IMATH_NAMESPACE::Vec2<size_t> len = match_dimension(mask);
        if ((size_t) data.len() == len.x*len.y) {
            for (size_t j = 0, z = 0; j < len.y; j++)
                for (size_t i=0; i<len.x; ++i, ++z)
                    if (mask(i,j))
                        (*this)(i,j) = data[z];
        } else {
            size_t count = 0;
            for (size_t j = 0, z = 0; j < len.y; j++)
                for (size_t i=0; i<len.x; ++i, ++z)
                    if (mask(i,j)) count++;

            if ((size_t) data.len() != count) {
                PyErr_SetString(PyExc_IndexError, "Dimensions of source data do not match destination either masked or unmasked");
                boost::python::throw_error_already_set();
            }

            for (size_t j = 0, z = 0; j < len.y; j++)
                for (size_t i=0; i<len.x; ++i)
                    if (mask(i,j))
                        (*this)(i,j) = data[z++];
        }
    }

    void
    setitem_array1d(PyObject *index, const FixedArray<T> &data)
    {
        //TODO:sanity check
        size_t startx=0, endx=0, slicelengthx=0;
        size_t starty=0, endy=0, slicelengthy=0;
        Py_ssize_t stepx=0;
        Py_ssize_t stepy=0;
        extract_slice_indices(PyTuple_GetItem(index, 0),_length.x,startx,endx,stepx,slicelengthx);
        extract_slice_indices(PyTuple_GetItem(index, 1),_length.y,starty,endy,stepy,slicelengthy);
        // we have a valid range of indices
        if ((size_t) data.len() != slicelengthx*slicelengthy) {
            PyErr_SetString(PyExc_IndexError, "Dimensions of source data do not match destination");
            boost::python::throw_error_already_set();
        }
        for (size_t j=0, z=0; j<slicelengthy; ++j)
            for (size_t i=0; i<slicelengthx; ++i, ++z)
                (*this)(startx+i*stepx, starty+j*stepy) = data[z];
    }

    IMATH_NAMESPACE::Vec2<size_t> len() const { return _length; }
    IMATH_NAMESPACE::Vec2<size_t> stride() const { return _stride; }
    T       & operator () (size_t i, size_t j)       { return _ptr[_stride.x*(j*_stride.y + i)]; }
    const T & operator () (size_t i, size_t j) const { return _ptr[_stride.x*(j*_stride.y + i)]; }
    size_t totalLen() const { return _size; }
    boost::python::tuple size() const
    {
        return boost::python::make_tuple(_length.x, _length.y);
    }

    static boost::python::class_<FixedArray2D<T> > register_(const char *name, const char *doc)
    {
        // a little tricky, but here we go - class types return internal references
        // but fundemental types just get copied.  this typedef sets up the appropriate
        // call policy for each type.
        typedef typename boost::mpl::if_<
            boost::is_class<T>,
            boost::python::return_internal_reference<>,
            boost::python::default_call_policies>::type call_policy;

        boost::python::class_<FixedArray2D<T> > c(name,doc, boost::python::init<size_t, size_t>(
            "construct an array of the specified length initialized to the default value for the type"));
        c
            .def(boost::python::init<const FixedArray2D<T> &>("construct an array with the same values as the given array"))
            .def(boost::python::init<const T &,size_t,size_t>("construct an array of the specified length initialized to the specified default value"))
            .def("__getitem__", &FixedArray2D<T>::getslice)
            .def("__getitem__", &FixedArray2D<T>::getslice_mask)
//             .def("__getitem__", &FixedArray2D<T>::getitem, call_policy())
            .def("item", &FixedArray2D<T>::getitem, call_policy())
//             .def("__setitem__", &FixedArray2D<T>::setitem)
            .def("__setitem__", &FixedArray2D<T>::setitem_scalar)
            .def("__setitem__", &FixedArray2D<T>::setitem_scalar_mask)
            .def("__setitem__", &FixedArray2D<T>::setitem_vector)
            .def("__setitem__", &FixedArray2D<T>::setitem_vector_mask)
            .def("__setitem__", &FixedArray2D<T>::setitem_array1d)
            .def("__setitem__", &FixedArray2D<T>::setitem_array1d_mask)
            .def("__len__",&FixedArray2D<T>::totalLen)
            .def("size",&FixedArray2D<T>::size)
            .def("ifelse",&FixedArray2D<T>::ifelse_scalar)
            .def("ifelse",&FixedArray2D<T>::ifelse_vector)
            ;
        return c;
    }

//     template <class T2>
//     size_t match_dimension(const FixedArray<T2> &a1) const
//     {
//         if (_length.x != a1.len()) {
//             PyErr_SetString(PyExc_IndexError, "Dimensions of source do not match destination");
//             boost::python::throw_error_already_set();
//         }
//         return _length.x;
//     }

    template <class T2>
    IMATH_NAMESPACE::Vec2<size_t> match_dimension(const FixedArray2D<T2> &a1) const
    {
        if (len() != a1.len()) {
            PyErr_SetString(PyExc_IndexError, "Dimensions of source do not match destination");
            boost::python::throw_error_already_set();
        }
        return len();
    }

    FixedArray2D<T> ifelse_vector(const FixedArray2D<int> &choice, const FixedArray2D<T> &other) {
        IMATH_NAMESPACE::Vec2<size_t> len = match_dimension(choice);
        match_dimension(other);
        FixedArray2D<T> tmp(len); // should use default construction but V3f doens't initialize
        for (size_t j = 0; j < len.y; ++j)
            for (size_t i = 0; i < len.x; ++i)
                tmp(i,j) = choice(i,j) ? (*this)(i,j) : other(i,j);
        return tmp;
    }

    FixedArray2D<T> ifelse_scalar(const FixedArray2D<int> &choice, const T &other) {
        IMATH_NAMESPACE::Vec2<size_t> len = match_dimension(choice);
        FixedArray2D<T> tmp(len); // should use default construction but V3f doens't initialize
        for (size_t j = 0; j < len.y; ++j)
            for (size_t i = 0; i < len.x; ++i)
                tmp(i,j) = choice(i,j) ? (*this)(i,j) : other;
        return tmp;
    }

};
 
// unary operation application
template <template <class,class> class Op, class T1, class Ret>
FixedArray2D<Ret> apply_array2d_unary_op(const FixedArray2D<T1> &a1)
{
    IMATH_NAMESPACE::Vec2<size_t> len = a1.len();
    FixedArray2D<Ret> retval(len.x,len.y);
    for (size_t j=0; j<len.y; ++j) {
        for (size_t i=0;i<len.x;++i) {
            retval(i,j) = Op<T1,Ret>::apply(a1(i,j));
        }
    }
    return retval;
}

// binary operation application
template <template <class,class,class> class Op, class T1, class T2, class Ret>
FixedArray2D<Ret> apply_array2d_array2d_binary_op(const FixedArray2D<T1> &a1, const FixedArray2D<T2> &a2)
{
    IMATH_NAMESPACE::Vec2<size_t> len = a1.match_dimension(a2);
    FixedArray2D<Ret> retval(len.x,len.y);
    for (size_t j=0; j<len.y; ++j) {
        for (size_t i=0;i<len.x;++i) {
            retval(i,j) = Op<T1,T2,Ret>::apply(a1(i,j),a2(i,j));
        }
    }
    return retval;
}

template <template <class,class,class> class Op, class T1, class T2, class Ret>
FixedArray2D<Ret> apply_array2d_scalar_binary_op(const FixedArray2D<T1> &a1, const T2 &a2)
{
    IMATH_NAMESPACE::Vec2<size_t> len = a1.len();
    FixedArray2D<Ret> retval(len.x,len.y);
    for (size_t j=0; j<len.y; ++j) {
        for (size_t i=0;i<len.x;++i) {
            retval(i,j) = Op<T1,T2,Ret>::apply(a1(i,j),a2);
        }
    }
    return retval;
}

template <template <class,class,class> class Op, class T1, class T2, class Ret>
FixedArray2D<Ret> apply_array2d_scalar_binary_rop(const FixedArray2D<T1> &a1, const T2 &a2)
{
    IMATH_NAMESPACE::Vec2<size_t> len = a1.len();
    FixedArray2D<Ret> retval(len.x,len.y);
    for (size_t j=0; j<len.y; ++j) {
        for (size_t i=0;i<len.x;++i) {
            retval(i,j) = Op<T2,T1,Ret>::apply(a2,a1(i,j));
        }
    }
    return retval;
}

// in-place binary operation application
template <template <class,class> class Op, class T1, class T2>
FixedArray2D<T1> & apply_array2d_array2d_ibinary_op(FixedArray2D<T1> &a1, const FixedArray2D<T2> &a2)
{
    IMATH_NAMESPACE::Vec2<size_t> len = a1.match_dimension(a2);
    for (size_t j=0; j<len.y; ++j) {
        for (size_t i=0;i<len.x;++i) {
            Op<T1,T2>::apply(a1(i,j),a2(i,j));
        }
    }
    return a1;
}

// in-place binary operation application
template <template <class,class> class Op, class T1, class T2>
FixedArray2D<T1> & apply_array2d_scalar_ibinary_op(FixedArray2D<T1> &a1, const T2 &a2)
{
    IMATH_NAMESPACE::Vec2<size_t> len = a1.len();
    for (size_t j=0; j<len.y; ++j) {
        for (size_t i=0;i<len.x;++i) {
            Op<T1,T2>::apply(a1(i,j),a2);
        }
    }
    return a1;
}

    
// PyObject* PyNumber_Add(	PyObject *o1, PyObject *o2)
template <class T> static FixedArray2D<T> operator + (const FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_binary_op<op_add,T,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> operator + (const FixedArray2D<T> &a0, const T &v1)               { return apply_array2d_scalar_binary_op<op_add,T,T,T>(a0,v1); }
template <class T> static FixedArray2D<T> operator + (const T &v1, const FixedArray2D<T> &a0)               { return a0+v1; }

// PyObject* PyNumber_Subtract(	PyObject *o1, PyObject *o2)
template <class T> static FixedArray2D<T> operator - (const FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_binary_op<op_sub,T,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> operator - (const FixedArray2D<T> &a0, const T &v1)               { return apply_array2d_scalar_binary_op<op_sub,T,T,T>(a0,v1); }
template <class T> static FixedArray2D<T> operator - (const T &v1, const FixedArray2D<T> &a0)               { return apply_array2d_scalar_binary_op<op_rsub,T,T,T>(a0,v1); }

// PyObject* PyNumber_Multiply(	PyObject *o1, PyObject *o2)
template <class T> static FixedArray2D<T> operator * (const FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_binary_op<op_mul,T,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> operator * (const FixedArray2D<T> &a0, const T &v1)               { return apply_array2d_scalar_binary_op<op_mul,T,T,T>(a0,v1); }
template <class T> static FixedArray2D<T> operator * (const T &v1, const FixedArray2D<T> &a0)               { return a0*v1; }

// PyObject* PyNumber_Divide(	PyObject *o1, PyObject *o2)
template <class T> static FixedArray2D<T> operator / (const FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_binary_op<op_div,T,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> operator / (const FixedArray2D<T> &a0, const T &v1)               { return apply_array2d_scalar_binary_op<op_div,T,T,T>(a0,v1); }
// no reversed scalar/array2d divide - no meaning

// PyObject* PyNumber_FloorDivide(	PyObject *o1, PyObject *o2)
// PyObject* PyNumber_TrueDivide(	PyObject *o1, PyObject *o2)
// PyObject* PyNumber_Remainder(	PyObject *o1, PyObject *o2)
template <class T> static FixedArray2D<T> operator % (const FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_binary_op<op_mod,T,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> operator % (const FixedArray2D<T> &a0, const T &v1)               { return apply_array2d_scalar_binary_op<op_mod,T,T,T>(a0,v1); }
// no reversed scalar%array2d remainder - no meaning

// PyObject* PyNumber_Divmod(	PyObject *o1, PyObject *o2)

// PyObject* PyNumber_Power(	PyObject *o1, PyObject *o2, PyObject *o3)
template <class T> static FixedArray2D<T> pow_array2d_array2d (const FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_binary_op<op_pow,T,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> pow_array2d_scalar (const FixedArray2D<T> &a0, const T &v1)                { return apply_array2d_scalar_binary_op<op_pow,T,T,T>(a0,v1); }
// no reversed scalar/array2d pow - no meaning

// PyObject* PyNumber_Negative(	PyObject *o)
template <class T> static FixedArray2D<T> operator - (const FixedArray2D<T> &a0) { return apply_array2d_unary_op<op_neg,T,T>(a0); }

// PyObject* PyNumber_Positive(	PyObject *o)

// PyObject* PyNumber_Absolute(	PyObject *o)
template <class T> static FixedArray2D<T> abs (const FixedArray2D<T> &a0)        { return apply_array2d_unary_op<op_abs,T,T>(a0); }

// PyObject* PyNumber_Invert(	PyObject *o)
template <class T> static FixedArray2D<T> operator ~ (const FixedArray2D<T> &a0) { return apply_array2d_unary_op<op_inverse,T,T>(a0); }

// PyObject* PyNumber_Lshift(	PyObject *o1, PyObject *o2)
template <class T> static FixedArray2D<T> operator << (const FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_binary_op<op_lshift,T,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> operator << (const FixedArray2D<T> &a0, const T &v1)               { return apply_array2d_scalar_binary_op<op_lshift,T,T,T>(a0,v1); }
// no reversed

// PyObject* PyNumber_Rshift(	PyObject *o1, PyObject *o2)
template <class T> static FixedArray2D<T> operator >> (const FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_binary_op<op_rshift,T,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> operator >> (const FixedArray2D<T> &a0, const T &v1)               { return apply_array2d_scalar_binary_op<op_rshift,T,T,T>(a0,v1); }
// no reversed

// PyObject* PyNumber_And(	PyObject *o1, PyObject *o2)
template <class T> static FixedArray2D<T> operator & (const FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_binary_op<op_bitand,T,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> operator & (const FixedArray2D<T> &a0, const T &v1)               { return apply_array2d_scalar_binary_op<op_bitand,T,T,T>(a0,v1); }
template <class T> static FixedArray2D<T> operator & (const T &v1, const FixedArray2D<T> &a0)               { return a0&v1; }

// PyObject* PyNumber_Xor(	PyObject *o1, PyObject *o2)
template <class T> static FixedArray2D<T> operator ^ (const FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_binary_op<op_xor,T,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> operator ^ (const FixedArray2D<T> &a0, const T &v1)               { return apply_array2d_scalar_binary_op<op_xor,T,T,T>(a0,v1); }
template <class T> static FixedArray2D<T> operator ^ (const T &v1, const FixedArray2D<T> &a0)               { return a0^v1; }

// PyObject* PyNumber_Or(	PyObject *o1, PyObject *o2)
template <class T> static FixedArray2D<T> operator | (const FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_binary_op<op_bitor,T,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> operator | (const FixedArray2D<T> &a0, const T &v1)               { return apply_array2d_scalar_binary_op<op_bitor,T,T,T>(a0,v1); }
template <class T> static FixedArray2D<T> operator | (const T &v1, const FixedArray2D<T> &a0)               { return a0|v1; }


// PyObject* PyNumber_InPlaceAdd(	PyObject *o1, PyObject *o2)
template <class T> static FixedArray2D<T> & operator += (FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_ibinary_op<op_iadd,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> & operator += (FixedArray2D<T> &a0, const T &v1)               { return apply_array2d_scalar_ibinary_op<op_iadd,T,T>(a0,v1); }

// PyObject* PyNumber_InPlaceSubtract(	PyObject *o1, PyObject *o2)
template <class T> static FixedArray2D<T> & operator -= (FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_ibinary_op<op_isub,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> & operator -= (FixedArray2D<T> &a0, const T &v1)               { return apply_array2d_scalar_ibinary_op<op_isub,T,T>(a0,v1); }

// PyObject* PyNumber_InPlaceMultiply(	PyObject *o1, PyObject *o2)
template <class T> static FixedArray2D<T> & operator *= (FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_ibinary_op<op_imul,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> & operator *= (FixedArray2D<T> &a0, const T &v1)               { return apply_array2d_scalar_ibinary_op<op_imul,T,T>(a0,v1); }

// PyObject* PyNumber_InPlaceDivide(	PyObject *o1, PyObject *o2)
template <class T> static FixedArray2D<T> & operator /= (FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_ibinary_op<op_idiv,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> & operator /= (FixedArray2D<T> &a0, const T &v1)               { return apply_array2d_scalar_ibinary_op<op_idiv,T,T>(a0,v1); }

// PyObject* PyNumber_InPlaceFloorDivide(	PyObject *o1, PyObject *o2)
// not implemented

// PyObject* PyNumber_InPlaceTrueDivide(	PyObject *o1, PyObject *o2)
// not implemented

// PyObject* PyNumber_InPlaceRemainder(	PyObject *o1, PyObject *o2)
template <class T> static FixedArray2D<T> & operator %= (FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_ibinary_op<op_imod,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> & operator %= (FixedArray2D<T> &a0, const T &v1)               { return apply_array2d_scalar_ibinary_op<op_imod,T,T>(a0,v1); }

// PyObject* PyNumber_InPlacePower(	PyObject *o1, PyObject *o2, PyObject *o3)
template <class T> static FixedArray2D<T> & ipow_array2d_array2d (FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_ibinary_op<op_ipow,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> & ipow_array2d_scalar (FixedArray2D<T> &a0, const T &v1)                { return apply_array2d_scalar_ibinary_op<op_ipow,T,T>(a0,v1); }

// PyObject* PyNumber_InPlaceLshift(	PyObject *o1, PyObject *o2)
template <class T> static FixedArray2D<T> & operator <<= (FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_ibinary_op<op_ilshift,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> & operator <<= (FixedArray2D<T> &a0, const T &v1)               { return apply_array2d_scalar_ibinary_op<op_ilshift,T,T>(a0,v1); }

// PyObject* PyNumber_InPlaceRshift(	PyObject *o1, PyObject *o2)
template <class T> static FixedArray2D<T> & operator >>= (FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_ibinary_op<op_irshift,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> & operator >>= (FixedArray2D<T> &a0, const T &v1)               { return apply_array2d_scalar_ibinary_op<op_irshift,T,T>(a0,v1); }

// PyObject* PyNumber_InPlaceAnd(	PyObject *o1, PyObject *o2)
template <class T> static FixedArray2D<T> & operator &= (FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_ibinary_op<op_ibitand,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> & operator &= (FixedArray2D<T> &a0, const T &v1)               { return apply_array2d_scalar_ibinary_op<op_ibitand,T,T>(a0,v1); }

// PyObject* PyNumber_InPlaceXor(	PyObject *o1, PyObject *o2)
template <class T> static FixedArray2D<T> & operator ^= (FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_ibinary_op<op_ixor,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> & operator ^= (FixedArray2D<T> &a0, const T &v1)               { return apply_array2d_scalar_ibinary_op<op_ixor,T,T>(a0,v1); }

// PyObject* PyNumber_InPlaceOr(	PyObject *o1, PyObject *o2)
template <class T> static FixedArray2D<T> & operator |= (FixedArray2D<T> &a0, const FixedArray2D<T> &a1) { return apply_array2d_array2d_ibinary_op<op_ibitor,T,T>(a0,a1); }
template <class T> static FixedArray2D<T> & operator |= (FixedArray2D<T> &a0, const T &v1)               { return apply_array2d_scalar_ibinary_op<op_ibitor,T,T>(a0,v1); }

template <class T>
static void add_arithmetic_math_functions(boost::python::class_<FixedArray2D<T> > &c) {
    using namespace boost::python;
    c
        .def("__add__",&apply_array2d_array2d_binary_op<op_add,T,T,T>)
        .def("__add__",&apply_array2d_scalar_binary_op<op_add,T,T,T>)
        .def("__radd__",&apply_array2d_scalar_binary_rop<op_add,T,T,T>)
        .def("__sub__",&apply_array2d_array2d_binary_op<op_sub,T,T,T>)
        .def("__sub__",&apply_array2d_scalar_binary_op<op_sub,T,T,T>)
        .def("__rsub__",&apply_array2d_scalar_binary_op<op_rsub,T,T,T>)
        .def("__mul__",&apply_array2d_array2d_binary_op<op_mul,T,T,T>)
        .def("__mul__",&apply_array2d_scalar_binary_op<op_mul,T,T,T>)
        .def("__rmul__",&apply_array2d_scalar_binary_rop<op_mul,T,T,T>)
        .def("__div__",&apply_array2d_array2d_binary_op<op_div,T,T,T>)
        .def("__div__",&apply_array2d_scalar_binary_op<op_div,T,T,T>)
        .def("__truediv__",&apply_array2d_array2d_binary_op<op_div,T,T,T>)
        .def("__truediv__",&apply_array2d_scalar_binary_op<op_div,T,T,T>)
        .def("__neg__",&apply_array2d_unary_op<op_neg,T,T>)
        .def("__iadd__",&apply_array2d_array2d_ibinary_op<op_iadd,T,T>,return_internal_reference<>())
        .def("__iadd__",&apply_array2d_scalar_ibinary_op<op_iadd,T,T>,return_internal_reference<>())
        .def("__isub__",&apply_array2d_array2d_ibinary_op<op_isub,T,T>,return_internal_reference<>())
        .def("__isub__",&apply_array2d_scalar_ibinary_op<op_isub,T,T>,return_internal_reference<>())
        .def("__imul__",&apply_array2d_array2d_ibinary_op<op_imul,T,T>,return_internal_reference<>())
        .def("__imul__",&apply_array2d_scalar_ibinary_op<op_imul,T,T>,return_internal_reference<>())
        .def("__idiv__",&apply_array2d_array2d_ibinary_op<op_idiv,T,T>,return_internal_reference<>())
        .def("__idiv__",&apply_array2d_scalar_ibinary_op<op_idiv,T,T>,return_internal_reference<>())
        .def("__itruediv__",&apply_array2d_array2d_ibinary_op<op_idiv,T,T>,return_internal_reference<>())
        .def("__itruediv__",&apply_array2d_scalar_ibinary_op<op_idiv,T,T>,return_internal_reference<>())
        ;
}


template <class T>
static void add_pow_math_functions(boost::python::class_<FixedArray2D<T> > &c) {
    using namespace boost::python;
    c
        .def("__pow__",&apply_array2d_array2d_binary_op<op_pow,T,T,T>)
        .def("__pow__",&apply_array2d_scalar_binary_op<op_pow,T,T,T>)
        .def("__rpow__",&apply_array2d_scalar_binary_rop<op_rpow,T,T,T>)
        .def("__ipow__",&apply_array2d_array2d_ibinary_op<op_ipow,T,T>,return_internal_reference<>())
        .def("__ipow__",&apply_array2d_scalar_ibinary_op<op_ipow,T,T>,return_internal_reference<>())
        ;
}

template <class T>
static void add_mod_math_functions(boost::python::class_<FixedArray2D<T> > &c) {
    using namespace boost::python;
    c
        .def("__mod__",&apply_array2d_array2d_binary_op<op_mod,T,T,T>)
        .def("__mod__",&apply_array2d_scalar_binary_op<op_mod,T,T,T>)
        .def("__imod__",&apply_array2d_array2d_ibinary_op<op_imod,T,T>,return_internal_reference<>())
        .def("__imod__",&apply_array2d_scalar_ibinary_op<op_imod,T,T>,return_internal_reference<>())
        ;
}

template <class T>
static void add_shift_math_functions(boost::python::class_<FixedArray2D<T> > &c) {
    using namespace boost::python;
    c
        .def("__lshift__",&apply_array2d_array2d_binary_op<op_lshift,T,T,T>)
        .def("__lshift__",&apply_array2d_scalar_binary_op<op_lshift,T,T,T>)
        .def("__ilshift__",&apply_array2d_array2d_ibinary_op<op_ilshift,T,T>,return_internal_reference<>())
        .def("__ilshift__",&apply_array2d_scalar_ibinary_op<op_ilshift,T,T>,return_internal_reference<>())
        .def("__rshift__",&apply_array2d_array2d_binary_op<op_rshift,T,T,T>)
        .def("__rshift__",&apply_array2d_scalar_binary_op<op_rshift,T,T,T>)
        .def("__irshift__",&apply_array2d_array2d_ibinary_op<op_irshift,T,T>,return_internal_reference<>())
        .def("__irshift__",&apply_array2d_scalar_ibinary_op<op_irshift,T,T>,return_internal_reference<>())
        ;
}

template <class T>
static void add_bitwise_math_functions(boost::python::class_<FixedArray2D<T> > &c) {
    using namespace boost::python;
    c
        .def("__and__",&apply_array2d_array2d_binary_op<op_bitand,T,T,T>)
        .def("__and__",&apply_array2d_scalar_binary_op<op_bitand,T,T,T>)
        .def("__iand__",&apply_array2d_array2d_ibinary_op<op_ibitand,T,T>,return_internal_reference<>())
        .def("__iand__",&apply_array2d_scalar_ibinary_op<op_ibitand,T,T>,return_internal_reference<>())
        .def("__or__",&apply_array2d_array2d_binary_op<op_bitor,T,T,T>)
        .def("__or__",&apply_array2d_scalar_binary_op<op_bitor,T,T,T>)
        .def("__ior__",&apply_array2d_array2d_ibinary_op<op_ibitor,T,T>,return_internal_reference<>())
        .def("__ior__",&apply_array2d_scalar_ibinary_op<op_ibitor,T,T>,return_internal_reference<>())
        .def("__xor__",&apply_array2d_array2d_binary_op<op_xor,T,T,T>)
        .def("__xor__",&apply_array2d_scalar_binary_op<op_xor,T,T,T>)
        .def("__ixor__",&apply_array2d_array2d_ibinary_op<op_ixor,T,T>,return_internal_reference<>())
        .def("__ixor__",&apply_array2d_scalar_ibinary_op<op_ixor,T,T>,return_internal_reference<>())
        ;
}

template <class T>
static void add_comparison_functions(boost::python::class_<FixedArray2D<T> > &c) {
    using namespace boost::python;
    c
        .def("__eq__",&apply_array2d_array2d_binary_op<op_eq,T,T,int>)
        .def("__eq__",&apply_array2d_scalar_binary_op<op_eq,T,T,int>)
        .def("__ne__",&apply_array2d_array2d_binary_op<op_ne,T,T,int>)
        .def("__ne__",&apply_array2d_scalar_binary_op<op_ne,T,T,int>)
        ;
}

template <class T>
static void add_ordered_comparison_functions(boost::python::class_<FixedArray2D<T> > &c) {
    using namespace boost::python;
    c
        .def("__lt__",&apply_array2d_array2d_binary_op<op_lt,T,T,int>)
        .def("__lt__",&apply_array2d_scalar_binary_op<op_lt,T,T,int>)
        .def("__gt__",&apply_array2d_array2d_binary_op<op_gt,T,T,int>)
        .def("__gt__",&apply_array2d_scalar_binary_op<op_gt,T,T,int>)
        .def("__le__",&apply_array2d_array2d_binary_op<op_le,T,T,int>)
        .def("__le__",&apply_array2d_scalar_binary_op<op_le,T,T,int>)
        .def("__ge__",&apply_array2d_array2d_binary_op<op_ge,T,T,int>)
        .def("__ge__",&apply_array2d_scalar_binary_op<op_ge,T,T,int>)
        ;
}

template <class S,class T>
static void add_explicit_construction_from_type(boost::python::class_<FixedArray2D<T> > &c) {
    using namespace boost::python;
    c.def(boost::python::init<FixedArray2D<S> >("copy contents of other array into this one"));
}

}

#endif
