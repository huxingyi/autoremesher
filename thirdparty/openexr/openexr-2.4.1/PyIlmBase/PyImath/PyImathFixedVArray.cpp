/////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2013, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above
//   copyright notice, this list of conditions and the following disclaimer
//   in the documentation and/or other materials provided with the
//   distribution.
// * Neither the name of Industrial Light & Magic nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission. 
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

#include "PyImathFixedVArray.h"

#include <boost/python.hpp>
#include <boost/shared_array.hpp>
#include <boost/any.hpp>
#include <Iex.h>
#include "PyImathExport.h"

namespace PyImath {

template <class T>
FixedVArray<T>::FixedVArray (std::vector<T>* ptr, Py_ssize_t length,
                             Py_ssize_t stride)
    : _ptr(ptr), _length(length), _stride(stride),
      _handle(), _unmaskedLength(0)
{
    if (length < 0)
    {
        throw IEX_NAMESPACE::ArgExc("Fixed array length must be non-negative");
    }
    if (stride <= 0)
    {
        throw IEX_NAMESPACE::ArgExc("Fixed array stride must be positive");
    }

    // Nothing else to do (pointer given, so we have the data)
}

template <class T>
FixedVArray<T>::FixedVArray (std::vector<T>* ptr, Py_ssize_t length,
                             Py_ssize_t stride, boost::any handle)
    : _ptr(ptr), _length(length), _stride(stride),
      _handle(handle), _unmaskedLength(0)
{
    if (length < 0)
    {
        throw IEX_NAMESPACE::ArgExc("Fixed array length must be non-negative");
    }
    if (stride <= 0)
    {
        throw IEX_NAMESPACE::ArgExc("Fixed array stride must be positive");
    }

    // Nothing else to do (pointer given, so we have the data)
}

template <class T>
FixedVArray<T>::FixedVArray(Py_ssize_t length)
    : _ptr(0), _length(length), _stride(1), _handle(), _unmaskedLength(0)
{
    if (length < 0)
    {
        throw IEX_NAMESPACE::ArgExc("Fixed array length must be non-negative");
    }

    boost::shared_array<std::vector<T> > a(new std::vector<T>[length]);
 // Initial vectors in the array will be zero-length.
    _handle = a;
    _ptr = a.get();
}

// template <class T>
// FixedVArray<T>::FixedVArray(Py_ssize_t length, Uninitialized)
//     : _ptr(0), _length(length), _stride(1), _handle(), _unmaskedLength(0)
// {
//     if (length < 0)
//     {
//         throw IEX_NAMESPACE::ArgExc("Fixed array length must be non-negative");
//     }
// 
//     boost::shared_array<std::vector<T> > a(new std::vector<T>[length]);
//     _handle = a;
//     _ptr = a.get();
// }

template <class T>
FixedVArray<T>::FixedVArray(const T& initialValue, Py_ssize_t length)
    : _ptr(0), _length(length), _stride(1), _handle(), _unmaskedLength(0)
{
    if (length < 0)
    {
        throw IEX_NAMESPACE::ArgExc("Fixed array length must be non-negative");
    }

    boost::shared_array<std::vector<T> > a(new std::vector<T>[length]);
    for (Py_ssize_t i = 0; i < length; ++i)
    {
        a[i].push_back (initialValue);
    }
    _handle = a;
    _ptr = a.get();
}

template <class T>
FixedVArray<T>::FixedVArray(FixedVArray<T>& other, const FixedArray<int>& mask)
    : _ptr(other._ptr), _stride(other._stride), _handle(other._handle)
{
    if (other.isMaskedReference())
    {
        throw IEX_NAMESPACE::NoImplExc
            ("Masking an already-masked FixedVArray is not supported yet (SQ27000)");
    }

    size_t len = (size_t) other.match_dimension (mask);
    _unmaskedLength = len;

    size_t reduced_len = 0;
    for (size_t i = 0; i < len; ++i)
    {
        if (mask[i])
        {
            reduced_len++;
        }
    }

    _indices.reset (new size_t[reduced_len]);

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

// template <class S>
// explicit FixedVArray(const FixedVArray<S> &other)  // AAJ (WHAT DOES THE TEMPLATE LOOK LIKE?)
// {
// }

template <class T>
FixedVArray<T>::FixedVArray(const FixedVArray<T>& other)
    : _ptr(other._ptr), _length(other._length), _stride(other._stride),
      _handle(other._handle), _indices(other._indices),
      _unmaskedLength(other._unmaskedLength)
{
    // Nothing.
}

template <class T>
const FixedVArray<T> &
FixedVArray<T>::operator = (const FixedVArray<T>& other)  // AAJ (WHY SHALLOW COPY)
{
    if (&other == this)
        return *this;

    _ptr            = other._ptr;
    _length         = other._length;
    _stride         = other._stride;
    _handle         = other._handle;
    _unmaskedLength = other._unmaskedLength;
    _indices        = other._indices;

    return *this;
}

template <class T>
FixedVArray<T>::~FixedVArray()  // AAJ (NOT DELETED BECAUSE SHARED ARRAY THAT HANDLE HAS?)
{
    // Nothing.
}


template <class T>
std::vector<T>&
FixedVArray<T>::operator [] (size_t i)
{
    return _ptr[(_indices ? raw_ptr_index(i) : i) * _stride];
}

template <class T>
const std::vector<T>&
FixedVArray<T>::operator [] (size_t i) const
{
    return _ptr[(_indices ? raw_ptr_index(i) : i) * _stride];
}


namespace {

//
// Make an index suitable for indexing into an array in c++
// from a python index, which can be negative for indexing 
// relative to the end of an array.
//
size_t
canonical_index (Py_ssize_t index, const size_t& totalLength)
{
    if (index < 0)
    {
        index += totalLength;
    }
    if ((size_t) index >= totalLength || index < 0)
    {
        PyErr_SetString (PyExc_IndexError, "Index out of range");
        boost::python::throw_error_already_set();
    }
    return index;  // still a 'virtual' index if this is a masked reference array
}

void
extract_slice_indices (PyObject* index, size_t& start, size_t& end,
                       Py_ssize_t& step, size_t& sliceLength,
                       const size_t& totalLength)
{
    if (PySlice_Check (index))
    {
#if PY_MAJOR_VERSION > 2
        PyObject* slice = index;
#else
        PySliceObject* slice = reinterpret_cast<PySliceObject *>(index);
#endif
        Py_ssize_t s, e, sl;
        if (PySlice_GetIndicesEx(slice, totalLength, &s, &e, &step, &sl) == -1)
        {
            boost::python::throw_error_already_set();
        }
        if (s < 0 || e < -1 || sl < 0)
        {
            throw IEX_NAMESPACE::LogicExc
                  ("Slice extraction produced invalid start, end, or length indices");
        }

        start = s;
        end   = e;
        sliceLength = sl;
    }
#if PY_MAJOR_VERSION > 2
    else if (PyLong_Check (index))
    {
        size_t i = canonical_index (PyLong_AsSsize_t(index), totalLength);
#else
    else if (PyInt_Check (index))
    {
        size_t i = canonical_index (PyInt_AsSsize_t(index), totalLength);
#endif
        start = i;
        end   = i + 1;
        step  = 1;
        sliceLength = 1;
    }
    else
    {
        PyErr_SetString (PyExc_TypeError, "Object is not a slice");
        boost::python::throw_error_already_set();
    }
}

} // namespace


// template <class T>
// typename boost::mpl::if_<boost::is_class<T>,T&,T>::type
// FixedVArray<T>::getitem (Py_ssize_t index)
// {
//     return (*this)[canonical_index (index, _length)];
// }
// 
// template <class T>
// typename boost::mpl::if_<boost::is_class<T>,const T&,T>::type
// FixedVArray<T>::getitem (Py_ssize_t index) const
// {
//     return (*this)[canonical_index (index, _length)];
// }


template <class T>
FixedVArray<T>
FixedVArray<T>::getslice (PyObject* index) const
{
    size_t start       = 0;
    size_t end         = 0;
    size_t sliceLength = 0;
    Py_ssize_t step;
    extract_slice_indices (index, start, end, step, sliceLength, _length);

    FixedVArray<T> f(sliceLength);

    if (_indices)
    {
        for (size_t i = 0; i < sliceLength; ++i)
        {
            f._ptr[i] = _ptr[raw_ptr_index(start + i*step)*_stride];
        }
    }
    else
    {
        for (size_t i = 0; i < sliceLength; ++i)
        {
            f._ptr[i] = _ptr[(start + i*step)*_stride];
        }
    }

    return f;
}

template <class T>
FixedVArray<T>
FixedVArray<T>::getslice_mask (const FixedArray<int>& mask)
{
    return FixedVArray<T> (*this, mask);
}

// template <class T>
// void
// FixedVArray<T>::setitem_scalar (PyObject* index, const T& data)
// {
//     size_t start       = 0;
//     size_t end         = 0;
//     size_t sliceLength = 0;
//     Py_ssize_t step;
//     extract_slice_indices (index, start, end, step, sliceLength, _length);
// 
//     if (_indices)
//     {
//         for (size_t i = 0; i < sliceLength; ++i)
//         {
//             _ptr[raw_ptr_index(start + i*step)*_stride] = data;
//         }
//     }
//     else
//     {
//         for (size_t i = 0; i < sliceLength; ++i)
//         {
//             _ptr[(start + i*step)*_stride] = data;
//         }
//     } 
// }
// 
// template <class T>
// void
// FixedVArray<T>::setitem_scalar_mask (const FixedArray<int>& mask, const T& data)
// {
//     size_t len = match_dimension(mask, false);
// 
//     if (_indices)
//     {
//         for (size_t i = 0; i < len; ++i)
//         {
//             // We don't need to actually look at 'mask' because
//             // match_dimensions has already forced some expected condition.
//             _ptr[raw_ptr_index(i)*_stride] = data;
//         }
//     }
//     else
//     {
//         for (size_t i = 0; i < len; ++i)
//         {
//             if (mask[i])
//             {
//                 _ptr[i*_stride] = data;
//             }
//         }
//     }
// }

template <class T>
void
FixedVArray<T>::setitem_vector (PyObject* index, const FixedVArray<T>& data)
{
    size_t start       = 0;
    size_t end         = 0;
    size_t sliceLength = 0;
    Py_ssize_t step;
    extract_slice_indices (index, start, end, step, sliceLength, _length);

    if ((size_t) data.len() != sliceLength)
    {
        PyErr_SetString (PyExc_IndexError,
                         "Dimensions of source do not match destination");
        boost::python::throw_error_already_set();
    }

    if (_indices)
    {
        for (size_t i = 0; i < sliceLength; ++i)
        {
            _ptr[raw_ptr_index(start + i*step)*_stride] = data[i];
        }
    }
    else
    {
        for (size_t i = 0; i < sliceLength; ++i)
        {
            _ptr[(start + i*step)*_stride] = data[i];
        }
    }
}

template <class T>
void
FixedVArray<T>::setitem_vector_mask (const FixedArray<int>& mask,
                                     const FixedVArray<T>&  data)
{
    // This restriction could be removed if there is a compelling use-case.
    if (_indices)
    {
        throw IEX_NAMESPACE::ArgExc
            ("We don't support setting item masks for masked reference arrays");
    }

    size_t len = match_dimension(mask);

    if ((size_t) data.len() == len)
    {
        for (size_t i = 0; i < len; ++i)
        {
            if (mask[i])
            {
                _ptr[i*_stride] = data[i];
            }
        }
    }
    else
    {
        size_t count = 0;
        for (size_t i = 0; i < len; ++i)
        {
            if (mask[i])
            {
                count++;
            }
        }
        if ((size_t) data.len() != count)
        {
            throw IEX_NAMESPACE::ArgExc
                ("Dimensions of source data do not match destination "
                 "either masked or unmasked");
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

// template <class T>
// FixedVArray<T>
// FixedVArray<T>::ifelse_scalar(const FixedArray<int>& choice, const T& other)
// {
//     size_t len = match_dimension (choice);
// 
//     FixedVArray<T> tmp(len);
//     for (size_t i = 0; i < len; ++i)
//     {
//         tmp[i] = choice[i] ? (*this)[i] : other;
//     }
// 
//     return tmp;
// }

template <class T>
FixedVArray<T>
FixedVArray<T>::ifelse_vector(const FixedArray<int>& choice,
                              const FixedVArray<T>&  other)
{
    size_t len = match_dimension (choice);
    match_dimension (other);

    FixedVArray<T> tmp(len);
    for (size_t i = 0; i < len; ++i)
    {
        tmp[i] = choice[i] ? (*this)[i] : other[i];
    }

    return tmp;
}

template <class T>
size_t
FixedVArray<T>::raw_ptr_index (size_t i) const
{
    assert (isMaskedReference());
    assert (i < _length);
    assert (_indices[i] >= 0 && _indices[i] < _unmaskedLength);

    return _indices[i];
}


// static
template <class T>
boost::python::class_<FixedVArray<T> >
FixedVArray<T>::register_(const char* doc)
{
 // // See 'PyImathFixedArray.h' for some explanation.
 // typedef typename boost::mpl::if_<
 //     boost::is_class<T>,
 //     boost::python::return_internal_reference<>,
 //     boost::python::default_call_policies>::type call_policy;
 // typedef typename boost::mpl::if_<
 //     boost::is_class<T>,
 //     boost::python::return_value_policy<boost::python::copy_const_reference>,
 //     boost::python::default_call_policies>::type const_call_policy;
 // 
 // typename FixedVArray<T>::get_type (FixedVArray<T>::*nonconst_getitem)(Py_ssize_t) =
 //     &FixedVArray<T>::getitem;
 // 
 // typename FixedVArray<T>::get_type_const (FixedVArray<T>::*const_getitem)(Py_ssize_t) const =
 //     &FixedVArray<T>::getitem;

    boost::python::class_<FixedVArray<T> > c (name(), doc,
        boost::python::init<size_t>("Construct a variable array of the "
        "specified length initialized to the default value for the given type"));

    c.def(boost::python::init<const FixedVArray<T> &>("Construct a variable array with the same values as the given array"))
     .def(boost::python::init<const T &, size_t>("Construct a variable array of the specified length initialized to the specified default value"))
     .def("__getitem__", &FixedVArray<T>::getslice)
     .def("__getitem__", &FixedVArray<T>::getslice_mask)
     .def("__setitem__", &FixedVArray<T>::setitem_vector)
     .def("__setitem__", &FixedVArray<T>::setitem_vector_mask)
     .def("__len__",     &FixedVArray<T>::len)
     .def("ifelse",      &FixedVArray<T>::ifelse_vector)
     ;

  // .def("__setitem__", &FixedVArray<T>::setitem_scalar)
  // .def("__setitem__", &FixedVArray<T>::setitem_scalar_mask)
  // .def("__getitem__",    const_getitem, const_call_policy())
  // .def("__getitem__", nonconst_getitem,       call_policy())
  // .def("ifelse",      &FixedVArray<T>::ifelse_scalar)

    return c;
}


// ---- Explicit Class Instantiation ---------------------------------

template class PYIMATH_EXPORT FixedVArray<int>;

} // namespace PyImath
