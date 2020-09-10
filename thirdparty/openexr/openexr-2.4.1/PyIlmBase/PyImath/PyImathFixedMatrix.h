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

#ifndef _PyImathFixedMatrix_h_
#define _PyImathFixedMatrix_h_

#include <boost/python.hpp>
#include <iostream>
#include "PyImathFixedArray.h"
#include "PyImathOperators.h"

#ifdef __clang__
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
namespace PyImath {

//
// Utility class for a runtime-specified fixed sized matrix type in python
//
template <class T>
class FixedMatrix
{
    T *     _ptr;
    int     _rows;
    int     _cols;
    int     _rowStride;
    int     _colStride;
    int *   _refcount; // refcount if allocated, null if externally allocated

  public:

    FixedMatrix(T *ptr, int rows, int cols, int rowStride = 1, int colStride = 1) 
        : _ptr(ptr), _rows(rows), _cols(cols),
          _rowStride(rowStride), _colStride(colStride), _refcount(0)
    {
        // nothing
    }

    FixedMatrix(int rows, int cols)
        : _ptr(new T[rows*cols]), _rows(rows), _cols(cols),
          _rowStride(1), _colStride(1), _refcount(new int(1))
    {
        // nothing
    }

    FixedMatrix(const FixedMatrix &other)
        : _ptr(other._ptr), _rows(other._rows), _cols(other._cols),
          _rowStride(other._rowStride), _colStride(other._colStride),
          _refcount(other._refcount)
    {
        if (_refcount) *_refcount += 1;
    }
        
    const FixedMatrix &
    operator = (const FixedMatrix &other)
    {
        if (&other == this) return *this;
        unref();
        _ptr = other._ptr;
        _rows = other._rows;
        _cols = other._cols;
        _rowStride = other._rowStride;
        _colStride = other._colStride;
        _refcount = other._refcount;

        if (_refcount) *_refcount += 1;
        return *this;
    }

    void
    unref()
    {
        if (_refcount) {
            *_refcount -= 1;
            if (*_refcount == 0) {
                delete [] _ptr;
                delete _refcount;
            }
        }
        _ptr = 0;
        _rows = 0;
        _cols = 0;
        _rowStride = 0;
        _colStride = 0;
        _refcount = 0;
    }

    ~FixedMatrix()
    {
        unref();
    }
    
    Py_ssize_t convert_index(int index) const
    {
        if (index < 0) index += _rows;
        if (index >= _rows || index < 0) {
            PyErr_SetString(PyExc_IndexError, "Index out of range");
            boost::python::throw_error_already_set();
        }
        return index;
    }

    void extract_slice_indices(PyObject *index, Py_ssize_t &start, Py_ssize_t &end, Py_ssize_t &step, Py_ssize_t &slicelength) const
    {
        slicelength = 0;
        if (PySlice_Check(index)) {
#if PY_MAJOR_VERSION > 2
            PyObject *slice = index;
#else
            PySliceObject *slice = reinterpret_cast<PySliceObject *>(index);
#endif
            if (PySlice_GetIndicesEx(slice,_rows,&start,&end,&step,&slicelength) == -1) {
		    boost::python::throw_error_already_set();
            }
#if PY_MAJOR_VERSION > 2
        } else if (PyLong_Check(index)) {
            Py_ssize_t i = convert_index(PyLong_AsSsize_t(index));
#else
        } else if (PyInt_Check(index)) {
            Py_ssize_t i = convert_index(PyInt_AS_LONG(index));
#endif
            start = i; end = i+1; step = 1; slicelength = 1;
        } else {
            PyErr_SetString(PyExc_TypeError, "Object is not a slice");
	    boost::python::throw_error_already_set();
        }
        //std::cout << "Slice indices are " << start << " " << end << " " << step << " " << slicelength << std::endl;
    }

    const FixedArray<T> * getitem(int index) const
    {
        return new FixedArray<T>(const_cast<T *>(&_ptr[convert_index(index)*_rowStride*_cols*_colStride]),_cols,_colStride);
    }

    FixedMatrix  getslice(PyObject *index) const
    {
        Py_ssize_t start, end, step, slicelength;
        extract_slice_indices(index,start,end,step,slicelength);
        FixedMatrix f(slicelength,_cols);
        for (int i=0; i<slicelength; ++i) {
            for (int j=0; j<_cols; ++j) {
                f.element(i,j) = element((start+i*step),j);
            }
        }
        return f;
    }

    void
    setitem_scalar(PyObject *index, const T &data)
    {
        Py_ssize_t start, end, step, slicelength;
        extract_slice_indices(index,start,end,step,slicelength);
        for (int i=0; i<slicelength; ++i) {
            for (int j = 0; j < _cols; ++j) {
                element(start+i*step,j) = data;
            }
        }
    }

    void
    setitem_vector(PyObject *index, const FixedArray<T> &data)
    {
        Py_ssize_t start, end, step, slicelength;
        extract_slice_indices(index,start,end,step,slicelength);
        if (data.len() != _cols) {
            PyErr_SetString(PyExc_IndexError, "Dimensions of source do not match destination");
	    boost::python::throw_error_already_set();
        }
        for (int i=0; i<slicelength; ++i) {
            for (int j = 0; j < _cols; ++j) {
                element(start+i*step,j) = data[j];
            }
        }
    }

    void
    setitem_matrix(PyObject *index, const FixedMatrix &data)
    {
        Py_ssize_t start, end, step, slicelength;
        extract_slice_indices(index,start,end,step,slicelength);
        
        // we have a valid range of indices
        if (data.rows() != slicelength || data.cols() != cols()) {
            PyErr_SetString(PyExc_IndexError, "Dimensions of source do not match destination");
	    boost::python::throw_error_already_set();
        }
        for (int i=0; i<slicelength; ++i) {
            for (int j=0; j<cols(); ++j) {
                element(start+i*step,j) = data.element(i,j);
            }
        }
    }

    int         rows() const { return _rows; }
    int         cols() const { return _cols; }
    int         rowStride() const { return _rowStride; }
    int         colStride() const { return _colStride; }

    T & element(int i, int j) { return _ptr[i*_rowStride*_cols*_colStride+j*_colStride]; }
    const T & element(int i, int j) const { return _ptr[i*_rowStride*_cols*_colStride+j*_colStride]; }

    FixedArray<T> operator [] (int i) { return FixedArray<T>(&_ptr[i*_rowStride*_cols*_colStride],_cols,_colStride); }
    const FixedArray<T> operator [] (int i) const { return FixedArray<T>(const_cast<T *>(&_ptr[i*_rowStride*_cols*_colStride]),_cols,_colStride); }

    static boost::python::class_<FixedMatrix<T> > register_(const char *name, const char *doc)
    {
        boost::python::class_<FixedMatrix<T> > c(name,doc, boost::python::init<int,int>("return an unitialized array of the specified rows and cols"));
        c
            .def("__getitem__", &FixedMatrix<T>::getslice)
            .def("__getitem__", &FixedMatrix<T>::getitem, boost::python::return_internal_reference<>()) 
            .def("__setitem__", &FixedMatrix<T>::setitem_scalar)
            .def("__setitem__", &FixedMatrix<T>::setitem_vector)
            .def("__setitem__", &FixedMatrix<T>::setitem_matrix)
            .def("__len__",&FixedMatrix<T>::rows)
            .def("rows",&FixedMatrix<T>::rows)
            .def("columns",&FixedMatrix<T>::cols)
            ;
        return c;
    }

    template <class T2>
    int match_dimension(const FixedMatrix<T2> &a1) const
    {
        if (rows() != a1.rows() || cols() != a1.cols()) {
            PyErr_SetString(PyExc_IndexError, "Dimensions of source do not match destination");
	    boost::python::throw_error_already_set();
        }
        return rows();
    }
};

// unary operation application
template <template <class,class> class Op, class T1, class Ret>
FixedMatrix<Ret> apply_matrix_unary_op(const FixedMatrix<T1> &a1)
{
    int rows = a1.rows();
    int cols = a1.cols();
    FixedMatrix<Ret> retval(rows,cols);
    for (int i=0;i<rows;++i) for (int j=0; j<cols; ++j) {
        retval.element(i,j) = Op<T1,Ret>::apply(a1.element(i,j));
    }
    return retval;
}

// binary operation application
template <template <class,class,class> class Op, class T1, class T2, class Ret>
FixedMatrix<Ret> apply_matrix_matrix_binary_op(const FixedMatrix<T1> &a1, const FixedMatrix<T2> &a2)
{
    int rows = a1.match_dimension(a2);
    int cols = a1.cols();
    FixedMatrix<Ret> retval(rows,cols);
    for (int i=0;i<rows;++i) for (int j=0; j<cols; ++j) {
        retval.element(i,j) = Op<T1,T2,Ret>::apply(a1.element(i,j),a2.element(i,j));
    }
    return retval;
}

template <template <class,class,class> class Op, class T1, class T2, class Ret>
FixedMatrix<Ret> apply_matrix_scalar_binary_op(const FixedMatrix<T1> &a1, const T2 &a2)
{
    int rows = a1.rows();
    int cols = a1.cols();
    FixedMatrix<Ret> retval(rows,cols);
    for (int i=0;i<rows;++i) for (int j=0; j<cols; ++j) {
        retval.element(i,j) = Op<T1,T2,Ret>::apply(a1.element(i,j),a2);
    }
    return retval;
}

template <template <class,class,class> class Op, class T1, class T2, class Ret>
FixedMatrix<Ret> apply_matrix_scalar_binary_rop(const FixedMatrix<T1> &a1, const T2 &a2)
{
    int rows = a1.rows();
    int cols = a1.cols();
    FixedMatrix<Ret> retval(rows,cols);
    for (int i=0;i<rows;++i) for (int j=0; j<cols; ++j) {
        retval.element(i,j) = Op<T2,T1,Ret>::apply(a2,a1.element(i,j));
    }
    return retval;
}

// in-place binary operation application
template <template <class,class> class Op, class T1, class T2>
FixedMatrix<T1> & apply_matrix_matrix_ibinary_op(FixedMatrix<T1> &a1, const FixedMatrix<T2> &a2)
{
    int rows = a1.match_dimension(a2);
    int cols = a1.cols();
    for (int i=0;i<rows;++i) for (int j=0; j<cols; ++j) {
        Op<T1,T2>::apply(a1.element(i,j),a2.element(i,j));
    }
    return a1;
}

// in-place binary operation application
template <template <class,class> class Op, class T1, class T2>
FixedMatrix<T1> & apply_matrix_scalar_ibinary_op(FixedMatrix<T1> &a1, const T2 &a2)
{
    int rows = a1.rows();
    int cols = a1.cols();
    for (int i=0;i<rows;++i) for (int j=0; j<cols; ++j) {
        Op<T1,T2>::apply(a1.element(i,j),a2);
    }
    return a1;
}

// PyObject* PyNumber_Add(	PyObject *o1, PyObject *o2)
template <class T> static FixedMatrix<T> operator + (const FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_binary_op<op_add,T,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> operator + (const FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_binary_op<op_add,T,T,T>(a0,v1); }
template <class T> static FixedMatrix<T> operator + (const T &v1, const FixedMatrix<T> &a0)              { return a0+v1; }

// PyObject* PyNumber_Subtract(	PyObject *o1, PyObject *o2)
template <class T> static FixedMatrix<T> operator - (const FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_binary_op<op_sub,T,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> operator - (const FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_binary_op<op_sub,T,T,T>(a0,v1); }
template <class T> static FixedMatrix<T> operator - (const T &v1, const FixedMatrix<T> &a0)              { return apply_matrix_scalar_binary_op<op_rsub,T,T,T>(a0,v1); }

// PyObject* PyNumber_Multiply(	PyObject *o1, PyObject *o2)
template <class T> static FixedMatrix<T> operator * (const FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_binary_op<op_mul,T,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> operator * (const FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_binary_op<op_mul,T,T,T>(a0,v1); }
template <class T> static FixedMatrix<T> operator * (const T &v1, const FixedMatrix<T> &a0)              { return a0*v1; }

// PyObject* PyNumber_Divide(	PyObject *o1, PyObject *o2)
template <class T> static FixedMatrix<T> operator / (const FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_binary_op<op_div,T,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> operator / (const FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_binary_op<op_div,T,T,T>(a0,v1); }
// no reversed scalar/matrix divide - no meaning

// PyObject* PyNumber_FloorDivide(	PyObject *o1, PyObject *o2)
// PyObject* PyNumber_TrueDivide(	PyObject *o1, PyObject *o2)
// PyObject* PyNumber_Remainder(	PyObject *o1, PyObject *o2)
template <class T> static FixedMatrix<T> operator % (const FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_binary_op<op_mod,T,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> operator % (const FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_binary_op<op_mod,T,T,T>(a0,v1); }
// no reversed scalar%matrix remainder - no meaning

// PyObject* PyNumber_Divmod(	PyObject *o1, PyObject *o2)

// PyObject* PyNumber_Power(	PyObject *o1, PyObject *o2, PyObject *o3)
template <class T> static FixedMatrix<T> pow_matrix_matrix (const FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_binary_op<op_pow,T,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> pow_matrix_scalar (const FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_binary_op<op_pow,T,T,T>(a0,v1); }
// no reversed scalar/matrix pow - no meaning

// PyObject* PyNumber_Negative(	PyObject *o)
template <class T> static FixedMatrix<T> operator - (const FixedMatrix<T> &a0) { return apply_matrix_unary_op<op_neg,T,T>(a0); }

// PyObject* PyNumber_Positive(	PyObject *o)

// PyObject* PyNumber_Absolute(	PyObject *o)
template <class T> static FixedMatrix<T> abs (const FixedMatrix<T> &a0)        { return apply_matrix_unary_op<op_abs,T,T>(a0); }

// PyObject* PyNumber_Invert(	PyObject *o)
template <class T> static FixedMatrix<T> operator ~ (const FixedMatrix<T> &a0) { return apply_matrix_unary_op<op_inverse,T,T>(a0); }

// PyObject* PyNumber_Lshift(	PyObject *o1, PyObject *o2)
template <class T> static FixedMatrix<T> operator << (const FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_binary_op<op_lshift,T,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> operator << (const FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_binary_op<op_lshift,T,T,T>(a0,v1); }
// no reversed

// PyObject* PyNumber_Rshift(	PyObject *o1, PyObject *o2)
template <class T> static FixedMatrix<T> operator >> (const FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_binary_op<op_rshift,T,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> operator >> (const FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_binary_op<op_rshift,T,T,T>(a0,v1); }
// no reversed

// PyObject* PyNumber_And(	PyObject *o1, PyObject *o2)
template <class T> static FixedMatrix<T> operator & (const FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_binary_op<op_bitand,T,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> operator & (const FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_binary_op<op_bitand,T,T,T>(a0,v1); }
template <class T> static FixedMatrix<T> operator & (const T &v1, const FixedMatrix<T> &a0)              { return a0&v1; }

// PyObject* PyNumber_Xor(	PyObject *o1, PyObject *o2)
template <class T> static FixedMatrix<T> operator ^ (const FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_binary_op<op_xor,T,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> operator ^ (const FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_binary_op<op_xor,T,T,T>(a0,v1); }
template <class T> static FixedMatrix<T> operator ^ (const T &v1, const FixedMatrix<T> &a0)              { return a0^v1; }

// PyObject* PyNumber_Or(	PyObject *o1, PyObject *o2)
template <class T> static FixedMatrix<T> operator | (const FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_binary_op<op_bitor,T,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> operator | (const FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_binary_op<op_bitor,T,T,T>(a0,v1); }
template <class T> static FixedMatrix<T> operator | (const T &v1, const FixedMatrix<T> &a0)              { return a0|v1; }


// PyObject* PyNumber_InPlaceAdd(	PyObject *o1, PyObject *o2)
template <class T> static FixedMatrix<T> & operator += (FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_ibinary_op<op_iadd,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> & operator += (FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_ibinary_op<op_iadd,T,T>(a0,v1); }

// PyObject* PyNumber_InPlaceSubtract(	PyObject *o1, PyObject *o2)
template <class T> static FixedMatrix<T> & operator -= (FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_ibinary_op<op_isub,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> & operator -= (FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_ibinary_op<op_isub,T,T>(a0,v1); }

// PyObject* PyNumber_InPlaceMultiply(	PyObject *o1, PyObject *o2)
template <class T> static FixedMatrix<T> & operator *= (FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_ibinary_op<op_imul,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> & operator *= (FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_ibinary_op<op_imul,T,T>(a0,v1); }

// PyObject* PyNumber_InPlaceDivide(	PyObject *o1, PyObject *o2)
template <class T> static FixedMatrix<T> & operator /= (FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_ibinary_op<op_idiv,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> & operator /= (FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_ibinary_op<op_idiv,T,T>(a0,v1); }

// PyObject* PyNumber_InPlaceFloorDivide(	PyObject *o1, PyObject *o2)
// not implemented

// PyObject* PyNumber_InPlaceTrueDivide(	PyObject *o1, PyObject *o2)
// not implemented

// PyObject* PyNumber_InPlaceRemainder(	PyObject *o1, PyObject *o2)
template <class T> static FixedMatrix<T> & operator %= (FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_ibinary_op<op_imod,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> & operator %= (FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_ibinary_op<op_imod,T,T>(a0,v1); }

// PyObject* PyNumber_InPlacePower(	PyObject *o1, PyObject *o2, PyObject *o3)
template <class T> static FixedMatrix<T> & ipow_matrix_matrix (FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_ibinary_op<op_ipow,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> & ipow_matrix_scalar (FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_ibinary_op<op_ipow,T,T>(a0,v1); }

// PyObject* PyNumber_InPlaceLshift(	PyObject *o1, PyObject *o2)
template <class T> static FixedMatrix<T> & operator <<= (FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_ibinary_op<op_ilshift,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> & operator <<= (FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_ibinary_op<op_ilshift,T,T>(a0,v1); }

// PyObject* PyNumber_InPlaceRshift(	PyObject *o1, PyObject *o2)
template <class T> static FixedMatrix<T> & operator >>= (FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_ibinary_op<op_irshift,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> & operator >>= (FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_ibinary_op<op_irshift,T,T>(a0,v1); }

// PyObject* PyNumber_InPlaceAnd(	PyObject *o1, PyObject *o2)
template <class T> static FixedMatrix<T> & operator &= (FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_ibinary_op<op_ibitand,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> & operator &= (FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_ibinary_op<op_ibitand,T,T>(a0,v1); }

// PyObject* PyNumber_InPlaceXor(	PyObject *o1, PyObject *o2)
template <class T> static FixedMatrix<T> & operator ^= (FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_ibinary_op<op_ixor,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> & operator ^= (FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_ibinary_op<op_ixor,T,T>(a0,v1); }

// PyObject* PyNumber_InPlaceOr(	PyObject *o1, PyObject *o2)
template <class T> static FixedMatrix<T> & operator |= (FixedMatrix<T> &a0, const FixedMatrix<T> &a1) { return apply_matrix_matrix_ibinary_op<op_ibitor,T,T>(a0,a1); }
template <class T> static FixedMatrix<T> & operator |= (FixedMatrix<T> &a0, const T &v1)              { return apply_matrix_scalar_ibinary_op<op_ibitor,T,T>(a0,v1); }

template <class T>
static void add_arithmetic_math_functions(boost::python::class_<FixedMatrix<T> > &c) {
    using namespace boost::python;
    c
        .def("__add__",&apply_matrix_matrix_binary_op<op_add,T,T,T>)
        .def("__add__",&apply_matrix_scalar_binary_op<op_add,T,T,T>)
        .def("__radd__",&apply_matrix_scalar_binary_rop<op_add,T,T,T>)
        .def("__sub__",&apply_matrix_matrix_binary_op<op_sub,T,T,T>)
        .def("__sub__",&apply_matrix_scalar_binary_op<op_sub,T,T,T>)
        .def("__rsub__",&apply_matrix_scalar_binary_op<op_rsub,T,T,T>)
        .def("__mul__",&apply_matrix_matrix_binary_op<op_mul,T,T,T>)
        .def("__mul__",&apply_matrix_scalar_binary_op<op_mul,T,T,T>)
        .def("__rmul__",&apply_matrix_scalar_binary_rop<op_mul,T,T,T>)
        .def("__div__",&apply_matrix_matrix_binary_op<op_div,T,T,T>)
        .def("__div__",&apply_matrix_scalar_binary_op<op_div,T,T,T>)
        .def("__truediv__",&apply_matrix_matrix_binary_op<op_div,T,T,T>)
        .def("__truediv__",&apply_matrix_scalar_binary_op<op_div,T,T,T>)
        .def("__neg__",&apply_matrix_unary_op<op_neg,T,T>)
        .def("__iadd__",&apply_matrix_matrix_ibinary_op<op_iadd,T,T>,return_internal_reference<>())
        .def("__iadd__",&apply_matrix_scalar_ibinary_op<op_iadd,T,T>,return_internal_reference<>())
        .def("__isub__",&apply_matrix_matrix_ibinary_op<op_isub,T,T>,return_internal_reference<>())
        .def("__isub__",&apply_matrix_scalar_ibinary_op<op_isub,T,T>,return_internal_reference<>())
        .def("__imul__",&apply_matrix_matrix_ibinary_op<op_imul,T,T>,return_internal_reference<>())
        .def("__imul__",&apply_matrix_scalar_ibinary_op<op_imul,T,T>,return_internal_reference<>())
        .def("__idiv__",&apply_matrix_matrix_ibinary_op<op_idiv,T,T>,return_internal_reference<>())
        .def("__idiv__",&apply_matrix_scalar_ibinary_op<op_idiv,T,T>,return_internal_reference<>())
        .def("__itruediv__",&apply_matrix_matrix_ibinary_op<op_idiv,T,T>,return_internal_reference<>())
        .def("__itruediv__",&apply_matrix_scalar_ibinary_op<op_idiv,T,T>,return_internal_reference<>())
        ;
}

template <class T>
static void add_pow_math_functions(boost::python::class_<FixedMatrix<T> > &c) {
    using namespace boost::python;
    c
        .def("__pow__",&pow_matrix_scalar<T>)
        .def("__pow__",&pow_matrix_matrix<T>)
        .def("__ipow__",&ipow_matrix_scalar<T>,return_internal_reference<>())
        .def("__ipow__",&ipow_matrix_matrix<T>,return_internal_reference<>())
        ;
}

template <class T>
static void add_mod_math_functions(boost::python::class_<FixedMatrix<T> > &c) {
    using namespace boost::python;
    c
        .def(self % self) // NOSONAR - suppress SonarCloud bug report.
        .def(self % other<T>())
        .def(self %= self) // NOSONAR - suppress SonarCloud bug report.
        .def(self %= other<T>())
        ;
}

template <class T>
static void add_shift_math_functions(boost::python::class_<FixedMatrix<T> > &c) {
    using namespace boost::python;
    c
        .def(self << self) // NOSONAR - suppress SonarCloud bug report.
        .def(self << other<T>())
        .def(self <<= self) // NOSONAR - suppress SonarCloud bug report.
        .def(self <<= other<T>())
        .def(self >> self) // NOSONAR - suppress SonarCloud bug report.
        .def(self >> other<T>())
        .def(self >>= self) // NOSONAR - suppress SonarCloud bug report.
        .def(self >>= other<T>())
        ;
}

template <class T>
static void add_bitwise_math_functions(boost::python::class_<FixedMatrix<T> > &c) {
    using namespace boost::python;
    c
        .def(self & self)
        .def(self & other<T>())
        .def(self &= self) // NOSONAR - suppress SonarCloud bug report.
        .def(self &= other<T>())
        .def(self | self)
        .def(self | other<T>())
        .def(self |= self) // NOSONAR - suppress SonarCloud bug report.
        .def(self |= other<T>())
        .def(self ^ self)
        .def(self ^ other<T>())
        .def(self ^= self) // NOSONAR - suppress SonarCloud bug report.
        .def(self ^= other<T>())
        ;
}


}
#ifdef __clang__
#    pragma clang diagnostic pop
#endif

#endif
