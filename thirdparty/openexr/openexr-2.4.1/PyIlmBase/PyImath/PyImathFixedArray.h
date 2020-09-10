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

#ifndef _PyImathFixedArray_h_
#define _PyImathFixedArray_h_

#include <boost/python.hpp>
#include <boost/operators.hpp>
#include <boost/shared_array.hpp>
#include <boost/any.hpp>
#include <Iex.h>
#include <iostream>
#include <IexMathFloatExc.h>
#include "PyImathUtil.h"
#include <PyIlmBaseConfigInternal.h>

#ifdef PYIMATH_ENABLE_EXCEPTIONS
# define PY_IMATH_LEAVE_PYTHON IEX_NAMESPACE::MathExcOn mathexcon (IEX_NAMESPACE::IEEE_OVERFLOW | \
                                                        IEX_NAMESPACE::IEEE_DIVZERO |  \
                                                        IEX_NAMESPACE::IEEE_INVALID);  \
                              PyImath::PyReleaseLock pyunlock;
# define PY_IMATH_RETURN_PYTHON mathexcon.handleOutstandingExceptions()
#else
# define PY_IMATH_LEAVE_PYTHON PyImath::PyReleaseLock pyunlock;
# define PY_IMATH_RETURN_PYTHON
#endif

namespace PyImath {

//
// Utility class for a runtime-specified fixed length array type in python
//
template <class T>
struct FixedArrayDefaultValue
{
    static T value();
};

enum Uninitialized {UNINITIALIZED};

template <class T>
class FixedArray
{
    T *     _ptr;
    size_t  _length;
    size_t  _stride;

    // this handle optionally stores a shared_array to allocated array data
    // so that everything is freed properly on exit.
    boost::any _handle;

    boost::shared_array<size_t> _indices; // non-NULL iff I'm a masked reference
    size_t                      _unmaskedLength;


  public:
    typedef T   BaseType;

    FixedArray(T *ptr, Py_ssize_t length, Py_ssize_t stride = 1)
        : _ptr(ptr), _length(length), _stride(stride), _handle(), _unmaskedLength(0)
    {
        if (length < 0)
        {
            throw IEX_NAMESPACE::LogicExc("Fixed array length must be non-negative");
        }
        if (stride <= 0)
        {
            throw IEX_NAMESPACE::LogicExc("Fixed array stride must be positive");
        }
        // nothing
    }

    FixedArray(T *ptr, Py_ssize_t length, Py_ssize_t stride, boost::any handle) 
        : _ptr(ptr), _length(length), _stride(stride), _handle(handle), _unmaskedLength(0)
    {
        if (_length < 0)
        {
            throw IEX_NAMESPACE::LogicExc("Fixed array length must be non-negative");
        }
        if (stride <= 0)
        {
            throw IEX_NAMESPACE::LogicExc("Fixed array stride must be positive");
        }
        // nothing
    }

    explicit FixedArray(Py_ssize_t length)
        : _ptr(0), _length(length), _stride(1), _handle(), _unmaskedLength(0)
    {
        if (_length < 0) {
            throw IEX_NAMESPACE::LogicExc("Fixed array length must be non-negative");
        }
        boost::shared_array<T> a(new T[length]);
        T tmp = FixedArrayDefaultValue<T>::value();
        for (Py_ssize_t i=0; i<length; ++i) a[i] = tmp;
        _handle = a;
        _ptr = a.get();
    }

    FixedArray(Py_ssize_t length,Uninitialized)
        : _ptr(0), _length(length), _stride(1), _handle(), _unmaskedLength(0)
    {
        if (_length < 0) {
            throw IEX_NAMESPACE::LogicExc("Fixed array length must be non-negative");
        }
        boost::shared_array<T> a(new T[length]);
        _handle = a;
        _ptr = a.get();
    }

    FixedArray(const T &initialValue, Py_ssize_t length)
        : _ptr(0), _length(length), _stride(1), _handle(), _unmaskedLength(0)
    {
        if (_length < 0) {
            throw IEX_NAMESPACE::LogicExc("Fixed array length must be non-negative");
        }
        boost::shared_array<T> a(new T[length]);
        for (Py_ssize_t i=0; i<length; ++i) a[i] = initialValue;
        _handle = a;
        _ptr = a.get();
    }

    FixedArray(FixedArray& f, const FixedArray<int>& mask) 
        : _ptr(f._ptr), _stride(f._stride), _handle(f._handle)
    {
        if (f.isMaskedReference())
        {
            throw IEX_NAMESPACE::NoImplExc("Masking an already-masked FixedArray not supported yet (SQ27000)");
        }

        size_t len = f.match_dimension(mask);
        _unmaskedLength = len;

        size_t reduced_len = 0;
        for (size_t i = 0; i < len; ++i)
            if (mask[i])
                reduced_len++;

        _indices.reset(new size_t[reduced_len]);

        for (size_t i = 0, j = 0; i < len; ++i)
        {
            if (mask[i])
            {
                _indices[j] = i;
                j++;
            }
        }

        _length = reduced_len;
    }

    template <class S>
    explicit FixedArray(const FixedArray<S> &other)
        : _ptr(0), _length(other.len()), _stride(1), _handle(), _unmaskedLength(other.unmaskedLength())
    {
        boost::shared_array<T> a(new T[_length]);
        for (size_t i=0; i<_length; ++i) a[i] = T(other[i]);
        _handle = a;
        _ptr = a.get();

        if (_unmaskedLength)
        {
            _indices.reset(new size_t[_length]);

            for (size_t i = 0; i < _length; ++i)
                _indices[i] = other.raw_ptr_index(i);
        }
    }

    FixedArray(const FixedArray &other)
        : _ptr(other._ptr), _length(other._length), _stride(other._stride),
          _handle(other._handle),
          _indices(other._indices),
          _unmaskedLength(other._unmaskedLength)
    {
    }
        
    const FixedArray &
    operator = (const FixedArray &other)
    {
        if (&other == this) return *this;

        _ptr = other._ptr;
        _length = other._length;
        _stride = other._stride;
        _handle = other._handle;
        _unmaskedLength = other._unmaskedLength;
        _indices = other._indices;

        return *this;
    }

    ~FixedArray()
    {
        // nothing
    }

    const boost::any & handle() { return _handle; }

    //
    // Make an index suitable for indexing into an array in c++ from
    // a python index, which can be negative for indexing relative to
    // the end of an array
    //
    size_t canonical_index(Py_ssize_t index) const
    {
        if (index < 0) index += len();
        if (index >= len() || index < 0) {
            PyErr_SetString(PyExc_IndexError, "Index out of range");
            boost::python::throw_error_already_set();
        }
        return index; // still a virtual index if this is a masked reference array
    }

    void extract_slice_indices(PyObject *index, size_t &start, size_t &end, Py_ssize_t &step, size_t &slicelength) const
    {
        if (PySlice_Check(index)) {
#if PY_MAJOR_VERSION > 2
            PyObject *slice = index;
#else
            PySliceObject *slice = reinterpret_cast<PySliceObject *>(index);
#endif
            Py_ssize_t s,e,sl;
            if (PySlice_GetIndicesEx(slice,_length,&s,&e,&step,&sl) == -1) {
                boost::python::throw_error_already_set();
            }
            // e can be -1 if the iteration is backwards with a negative slice operator [::-n] (n > 0).
            if (s < 0 || e < -1 || sl < 0) {
                throw IEX_NAMESPACE::LogicExc("Slice extraction produced invalid start, end, or length indices");
            }
            start = s;
            end = e;
            slicelength = sl;
#if PY_MAJOR_VERSION > 2
        } else if (PyLong_Check(index)) {
            size_t i = canonical_index(PyLong_AsSsize_t(index));
#else
        } else if (PyInt_Check(index)) {
            size_t i = canonical_index(PyInt_AsSsize_t(index));
#endif
            start = i; end = i+1; step = 1; slicelength = 1;
        } else {
            PyErr_SetString(PyExc_TypeError, "Object is not a slice");
	    boost::python::throw_error_already_set();
        }
    }

    // return_internal_reference doesn't seem to work with non-class types
    typedef typename boost::mpl::if_<boost::is_class<T>,T&,T>::type get_type;
    get_type       getitem(Py_ssize_t index) { return (*this)[canonical_index(index)]; }
    typedef typename boost::mpl::if_<boost::is_class<T>,const T&,T>::type get_type_const;
    get_type_const getitem(Py_ssize_t index) const { return (*this)[canonical_index(index)]; }
    FixedArray  getslice(PyObject *index) const
    {
        size_t start=0, end=0, slicelength=0;
        Py_ssize_t step;
        extract_slice_indices(index,start,end,step,slicelength);
        FixedArray f(slicelength);

        if (_indices)
        {
            for (size_t i=0; i<slicelength; ++i)
                f._ptr[i] = _ptr[raw_ptr_index(start+i*step)*_stride];
        }
        else
        {
            for (size_t i=0; i<slicelength; ++i)
                f._ptr[i] = _ptr[(start+i*step)*_stride];
        }
        return f;
    }

    FixedArray getslice_mask(const FixedArray<int>& mask)
    {
        FixedArray f(*this, mask);
        return f;
    }

    void
    setitem_scalar(PyObject *index, const T &data)
    {
        size_t start=0, end=0, slicelength=0;
        Py_ssize_t step;
        extract_slice_indices(index,start,end,step,slicelength);

        if (_indices)
        {
            for (size_t i=0; i<slicelength; ++i)
                _ptr[raw_ptr_index(start+i*step)*_stride] = data;
        }
        else
        {
            for (size_t i=0; i<slicelength; ++i)
                _ptr[(start+i*step)*_stride] = data;
        }
    }

    void
    setitem_scalar_mask(const FixedArray<int> &mask, const T &data)
    {
        size_t len = match_dimension(mask, false);

        if (_indices)
        {
            for (size_t i = 0; i < len; ++i)
                _ptr[raw_ptr_index(i)*_stride] = data;
        }
        else
        {
            for (size_t i=0; i<len; ++i)
                if (mask[i]) _ptr[i*_stride] = data;
        }
    }

    void
    setitem_vector(PyObject *index, const FixedArray &data)
    {
        size_t start=0, end=0, slicelength=0;
        Py_ssize_t step;
        extract_slice_indices(index,start,end,step,slicelength);
        
        // we have a valid range of indices
        if ((size_t)data.len() != slicelength) {
            PyErr_SetString(PyExc_IndexError, "Dimensions of source do not match destination");
	    boost::python::throw_error_already_set();
        }

        if (_indices)
        {
            for (size_t i=0; i<slicelength; ++i)
                _ptr[raw_ptr_index(start+i*step)*_stride] = data[i];
        }
        else
        {
            for (size_t i=0; i<slicelength; ++i)
                _ptr[(start+i*step)*_stride] = data[i];
        }
    }

    void
    setitem_vector_mask(const FixedArray<int> &mask, const FixedArray &data)
    {
        // We could relax this but this restriction if there's a good
        // enough reason too.

        if (_indices)
        {
            throw IEX_NAMESPACE::ArgExc("We don't support setting item masks for masked reference arrays.");
        }

        size_t len = match_dimension(mask);
        if ((size_t)data.len() == len)
        {
            for (size_t i = 0; i < len; ++i)
                if (mask[i]) _ptr[i*_stride] = data[i];
        }
        else
        {
            size_t count = 0;
            for (size_t i = 0; i < len; ++i)
                if (mask[i]) count++;

            if ((size_t)data.len() != count) {
                throw IEX_NAMESPACE::ArgExc("Dimensions of source data do not match destination either masked or unmasked");
            }

            Py_ssize_t dataIndex = 0;
            for (size_t i = 0; i < len; ++i)
            {
                if (mask[i])
                {
                    _ptr[i*_stride] = data[dataIndex];
                    dataIndex++;
                }
            }
        }
    }

    // exposed as Py_ssize_t for compatilbity with standard python sequences
    Py_ssize_t len() const     { return _length; }
    size_t      stride() const { return _stride; }

    // no bounds checking on i!
    T& operator [] (size_t i)
    {
        return _ptr[(_indices ? raw_ptr_index(i) : i) * _stride];
    }

    // no bounds checking on i!
    const T& operator [] (size_t i) const
    {
        return _ptr[(_indices ? raw_ptr_index(i) : i) * _stride];
    }

    // no mask conversion or bounds checking on i!
    T& direct_index(size_t i)
    {
        return _ptr[i*_stride];
    }

    // no mask conversion or bounds checking on i!
    const T& direct_index (size_t i) const
    {
        return _ptr[i*_stride];
    }

    bool isMaskedReference() const {return _indices.get() != 0;}
    size_t unmaskedLength() const {return _unmaskedLength;}

    // Conversion of indices to raw pointer indices.
    // This should only be called when this is a masked reference.
    // No safety checks done for performance.
    size_t raw_ptr_index(size_t i) const
    {
        assert(isMaskedReference());
        assert(i < _length);
        assert(_indices[i] >= 0 && _indices[i] < _unmaskedLength);
        return _indices[i];
    }

    static boost::python::class_<FixedArray<T> > register_(const char *doc)
    {
        // a little tricky, but here we go - class types return internal references
        // but fundemental types just get copied.  this typedef sets up the appropriate
        // call policy for each type.
        typedef typename boost::mpl::if_<
            boost::is_class<T>,
            boost::python::return_internal_reference<>,
            boost::python::default_call_policies>::type call_policy;

        typedef typename boost::mpl::if_<
            boost::is_class<T>,
            boost::python::return_value_policy<boost::python::copy_const_reference>,
            boost::python::default_call_policies>::type const_call_policy;

        //typename FixedArray<T>::get_type (FixedArray<T>::*nonconst_getitem)(Py_ssize_t)= &FixedArray<T>::getitem;
        //typename FixedArray<T>::get_type_const (FixedArray<T>::*const_getitem)(Py_ssize_t) = &FixedArray<T>::getitem;
        typename FixedArray<T>::get_type (FixedArray<T>::*nonconst_getitem)(Py_ssize_t)= &FixedArray<T>::getitem;
        typename FixedArray<T>::get_type_const (FixedArray<T>::*const_getitem)(Py_ssize_t) const = &FixedArray<T>::getitem;

        boost::python::class_<FixedArray<T> > c(name(),doc, boost::python::init<size_t>("construct an array of the specified length initialized to the default value for the type"));
        c
            .def(boost::python::init<const FixedArray<T> &>("construct an array with the same values as the given array"))
            .def(boost::python::init<const T &,size_t>("construct an array of the specified length initialized to the specified default value"))
            .def("__getitem__", &FixedArray<T>::getslice)
            .def("__getitem__", &FixedArray<T>::getslice_mask)
            .def("__getitem__", const_getitem, const_call_policy())
            .def("__getitem__", nonconst_getitem, call_policy()) 
            .def("__setitem__", &FixedArray<T>::setitem_scalar)
            .def("__setitem__", &FixedArray<T>::setitem_scalar_mask)
            .def("__setitem__", &FixedArray<T>::setitem_vector)
            .def("__setitem__", &FixedArray<T>::setitem_vector_mask)
            .def("__len__",&FixedArray<T>::len)
            .def("ifelse",&FixedArray<T>::ifelse_scalar)
            .def("ifelse",&FixedArray<T>::ifelse_vector)
            ;
        return c;
    }

    template <class T2>
    size_t match_dimension(const FixedArray<T2> &a1, bool strictComparison = true) const
    {
        if (len() == a1.len())
            return len();

        bool throwExc = false;
        if (strictComparison)
            throwExc = true;
        else if (_indices)
        {
            if (_unmaskedLength != (size_t) a1.len())
                throwExc = true;
        }
        else
            throwExc = true;

        if (throwExc)
        {
            throw IEX_NAMESPACE::ArgExc("Dimensions of source do not match destination");
        }

        return len();
    }

    FixedArray<T> ifelse_vector(const FixedArray<int> &choice, const FixedArray<T> &other) {
        size_t len = match_dimension(choice);
        match_dimension(other);
        FixedArray<T> tmp(len); // should use default construction but V3f doens't initialize
        for (size_t i=0; i < len; ++i) tmp[i] = choice[i] ? (*this)[i] : other[i];
        return tmp;
    }

    FixedArray<T> ifelse_scalar(const FixedArray<int> &choice, const T &other) {
        size_t len = match_dimension(choice);
        FixedArray<T> tmp(len); // should use default construction but V3f doens't initialize
        for (size_t i=0; i < len; ++i) tmp[i] = choice[i] ? (*this)[i] : other;
        return tmp;
    }

    // Instantiations of fixed ararys must implement this static member
    static const char *name();
};

//
// Helper struct for arary indexing  with a known compile time length
//
template <class Container, class Data>
struct IndexAccessDefault {
    typedef Data & result_type;
    static Data & apply(Container &c, size_t i) { return c[i]; }
};

template <class Container, class Data, int Length, class IndexAccess = IndexAccessDefault<Container,Data> >
struct StaticFixedArray
{
    static Py_ssize_t len(const Container &) { return Length; }
    static typename   IndexAccess::result_type getitem(Container &c, Py_ssize_t index) { return IndexAccess::apply(c,canonical_index(index)); }
    static void       setitem(Container &c, Py_ssize_t index, const Data &data) { IndexAccess::apply(c,canonical_index(index)) = data; }
    static size_t     canonical_index(Py_ssize_t index)
    {
        if (index < 0) index += Length;
        if (index < 0 || index >= Length) {
            PyErr_SetString(PyExc_IndexError, "Index out of range");
            boost::python::throw_error_already_set();
        }
        return index;
    }
};

}

#endif // _PyImathFixedArray_h_
