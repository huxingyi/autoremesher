///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2009-2012, Industrial Light & Magic, a division of Lucas
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

#include "PyImathStringArrayRegister.h"
#include "PyImathStringArray.h"
#include "PyImathExport.h"

namespace PyImath {

using namespace boost::python;

template<class T>
StringArrayT<T>* StringArrayT<T>::createDefaultArray(size_t length)
{
    return StringArrayT<T>::createUniformArray(T(), length);
}

template<class T>
StringArrayT<T>* StringArrayT<T>::createUniformArray(const T& initialValue, size_t length)
{
    typedef boost::shared_array<StringTableIndex> StringTableIndexArrayPtr;
    typedef boost::shared_ptr<StringTableT<T> > StringTablePtr;

    BOOST_STATIC_ASSERT(boost::is_pod<StringTableIndex>::value);

    StringTableIndexArrayPtr indexArray(reinterpret_cast<StringTableIndex*>(new char[sizeof(StringTableIndex)*length]));
    StringTablePtr table(new StringTableT<T>);

    const StringTableIndex index = table->intern(initialValue);

    for(size_t i=0; i<length; ++i)
        indexArray[i] = index;

    return new StringArrayT<T>(*table, indexArray.get(), length, 1, indexArray, table);
}

template<class T>
StringArrayT<T>* StringArrayT<T>::createFromRawArray(const T* rawArray, size_t length)
{
    typedef boost::shared_array<StringTableIndex> StringTableIndexArrayPtr;
    typedef boost::shared_ptr<StringTableT<T> > StringTablePtr;

    BOOST_STATIC_ASSERT(boost::is_pod<StringTableIndex>::value);

    StringTableIndexArrayPtr indexArray(reinterpret_cast<StringTableIndex*>(new char[sizeof(StringTableIndex)*length]));
    StringTablePtr table(new StringTableT<T>);

    for(size_t i=0; i<length; ++i)
        indexArray[i] = table->intern(rawArray[i]);

    return new StringArrayT<T>(*table, indexArray.get(), length, 1, indexArray, table);
}

template<class T>
StringArrayT<T>::StringArrayT(StringTableT<T> &table, StringTableIndex *ptr, size_t length, size_t stride, boost::any tableHandle)
    : super(ptr,length,stride), _table(table), _tableHandle(tableHandle)
{
    // nothing
}

template<class T>
StringArrayT<T>::StringArrayT(StringTableT<T> &table, StringTableIndex *ptr, size_t length, size_t stride, boost::any handle, boost::any tableHandle) 
    : super(ptr,length,stride,handle), _table(table), _tableHandle(tableHandle)
{
    // nothing
}



template<class T>
StringArrayT<T>*
StringArrayT<T>::getslice_string(PyObject *index) const
{
    typedef boost::shared_array<StringTableIndex> StringTableIndexArrayPtr;
    typedef boost::shared_ptr<StringTableT<T> > StringTablePtr;

    BOOST_STATIC_ASSERT(boost::is_pod<StringTableIndex>::value);

    size_t start=0, end=0, slicelength=0;
    Py_ssize_t step;
    extract_slice_indices(index,start,end,step,slicelength);

    StringTableIndexArrayPtr indexArray(reinterpret_cast<StringTableIndex*>(new char[sizeof(StringTableIndex)*slicelength]));
    StringTablePtr table(new StringTableT<T>);

    for(size_t i=0; i<slicelength; ++i)
        indexArray[i] = table->intern(getitem_string(start+i*step));

    return new StringArrayT<T>(*table, indexArray.get(), slicelength, 1, indexArray, table);
}

template<class T>
void
StringArrayT<T>::setitem_string_scalar(PyObject *index, const T &data)
{
    size_t start=0, end=0, slicelength=0;
    Py_ssize_t step;
    extract_slice_indices(index,start,end,step,slicelength);
    StringTableIndex di = _table.intern(data);
    for (size_t i=0; i<slicelength; ++i) {
        (*this)[start+i*step] = di;
    }
}

template<class T>
void
StringArrayT<T>::setitem_string_scalar_mask(const FixedArray<int> &mask, const T &data)
{
    size_t len = match_dimension(mask);
    StringTableIndex di = _table.intern(data);
    for (size_t i=0; i<len; ++i) {
        if (mask[i]) (*this)[i] = di;
    }
}

template<class T>
void
StringArrayT<T>::setitem_string_vector(PyObject *index, const StringArrayT<T> &data)
{
    size_t start=0, end=0, slicelength=0;
    Py_ssize_t step;
    extract_slice_indices(index,start,end,step,slicelength);
        
    // we have a valid range of indices
    if ((size_t) data.len() != slicelength) {
        PyErr_SetString(PyExc_IndexError, "Dimensions of source do not match destination");
        throw_error_already_set();
    }
    for (size_t i=0; i<slicelength; ++i) {
        StringTableIndex di = _table.intern(data._table.lookup(data[i]));
        (*this)[start+i*step] = di;
    }
}

template<class T>
void
StringArrayT<T>::setitem_string_vector_mask(const FixedArray<int> &mask, const StringArrayT<T> &data)
{
    size_t len = match_dimension(mask);
    if ((size_t) data.len() == len) {
        for (size_t i=0; i<len; ++i) {
            if (mask[i]) {
                StringTableIndex di = _table.intern(data._table.lookup(data[i]));
                (*this)[i] = di;
            }
        }
    } else {
        size_t count = 0;
        for (size_t i=0; i<len; ++i) {
            if (mask[i]) count += 1;
        }

        if ((size_t) data.len() != count) {
            PyErr_SetString(PyExc_IndexError, "Dimensions of source data do not match destination either masked or unmasked");
            throw_error_already_set();
        }
            
        size_t dataIndex = 0;
        for (size_t i=0; i<len; ++i) {
            if (mask[i]) {
                StringTableIndex di = _table.intern(data._table.lookup(data[dataIndex]));
                (*this)[i] = di;
                dataIndex += 1;
            }
        }
    }
}

template<class T>
FixedArray<int> operator == (const StringArrayT<T> &a0, const StringArrayT<T> &a1) {
    size_t len = a0.match_dimension(a1);
    FixedArray<int> f(len);
    const StringTableT<T> &t0 = a0.stringTable();
    const StringTableT<T> &t1 = a1.stringTable();
    for (size_t i=0;i<len;++i) {
     f[i] = t0.lookup(a0[i])==t1.lookup(a1[i]); 
    }
    return f;
}

template<class T>
FixedArray<int> operator == (const StringArrayT<T> &a0, const T &v1) {
    size_t len = a0.len();
    FixedArray<int> f(len);
    const StringTableT<T> &t0 = a0.stringTable();
    if (t0.hasString(v1)) {
        StringTableIndex v1i = t0.lookup(v1);
        for (size_t i=0;i<len;++i) {
            f[i] = a0[i]==v1i;
        }
    } else {
        for (size_t i=0;i<len;++i) {
            f[i] = 0;
        }
    }
    return f;
}

template<class T>
FixedArray<int> operator == (const T &v1,const StringArrayT<T> &a0) {
    return a0 == v1;
}

template<class T>
FixedArray<int> operator != (const StringArrayT<T> &a0, const StringArrayT<T> &a1) {
    size_t len = a0.match_dimension(a1);
    FixedArray<int> f(len);
    const StringTableT<T> &t0 = a0.stringTable();
    const StringTableT<T> &t1 = a1.stringTable();
    for (size_t i=0;i<len;++i) {
        f[i] = t0.lookup(a0[i])!=t1.lookup(a1[i]); 
    }
    return f;
}

template<class T>
FixedArray<int> operator != (const StringArrayT<T> &a0, const T &v1) {
    size_t len = a0.len();
    FixedArray<int> f(len);
    const StringTableT<T> &t0 = a0.stringTable();
    if (t0.hasString(v1)) {
        StringTableIndex v1i = t0.lookup(v1);
        for (size_t i=0;i<len;++i) {
            f[i] = a0[i]!=v1i;
        }
    } else {
        for (size_t i=0;i<len;++i) {
            f[i] = 1;
        }
    }
    return f;
}

template<class T>
FixedArray<int> operator != (const T &v1,const StringArrayT<T> &a0) {
    return a0 != v1;
}

template<> PYIMATH_EXPORT StringTableIndex FixedArrayDefaultValue<StringTableIndex>::value() { return StringTableIndex(0); }
template<> PYIMATH_EXPORT const char*      FixedArray<StringTableIndex>::name() { return "StringTableArray"; }

template class PYIMATH_EXPORT StringArrayT<std::string>;
template class PYIMATH_EXPORT StringArrayT<std::wstring>;

template FixedArray<int> operator == (const StringArray& a0, const StringArray& a1);
template FixedArray<int> operator == (const StringArray& a0, const std::string& v1);
template FixedArray<int> operator == (const std::string& a0, const StringArray& v1);
template FixedArray<int> operator != (const StringArray& a0, const StringArray& a1);
template FixedArray<int> operator != (const StringArray& a0, const std::string& v1);
template FixedArray<int> operator != (const std::string& a0, const StringArray& v1);

template FixedArray<int> operator == (const WstringArray& a0, const WstringArray& a1);
template FixedArray<int> operator == (const WstringArray& a0, const std::wstring& v1);
template FixedArray<int> operator == (const std::wstring& a0, const WstringArray& v1);
template FixedArray<int> operator != (const WstringArray& a0, const WstringArray& a1);
template FixedArray<int> operator != (const WstringArray& a0, const std::wstring& v1);
template FixedArray<int> operator != (const std::wstring& a0, const WstringArray& v1);

void register_StringArrays()
{
    typedef StringArrayT<std::string> StringArray;
    typedef StringArrayT<std::wstring> WstringArray;
    
    class_<StringArray> string_array_class =
        class_<StringArray>("StringArray",no_init);
    string_array_class
        .def("__init__", make_constructor(StringArray::createDefaultArray))
        .def("__init__", make_constructor(StringArray::createUniformArray))
        .def("__getitem__", &StringArray::getslice_string, return_value_policy<manage_new_object>()) 
        .def("__getitem__", &StringArray::getitem_string) 
        .def("__setitem__", &StringArray::setitem_string_scalar)
        .def("__setitem__", &StringArray::setitem_string_scalar_mask)
        .def("__setitem__", &StringArray::setitem_string_vector)
        .def("__setitem__", &StringArray::setitem_string_vector_mask)
        .def("__len__",&StringArray::len)
        .def(self == self) // NOSONAR - suppress SonarCloud bug report.
        .def(self == other<std::string>())
        .def(other<std::string>() == self)
        .def(self != self) // NOSONAR - suppress SonarCloud bug report.
        .def(self != other<std::string>())
        .def(other<std::string>() != self)
        ;

    class_<WstringArray> wstring_array_class =
        class_<WstringArray>("WstringArray",no_init);
    wstring_array_class
        .def("__init__", make_constructor(WstringArray::createDefaultArray))
        .def("__init__", make_constructor(WstringArray::createUniformArray))
        .def("__getitem__", &WstringArray::getslice_string, return_value_policy<manage_new_object>()) 
        .def("__getitem__", &WstringArray::getitem_string) 
        .def("__setitem__", &WstringArray::setitem_string_scalar)
        .def("__setitem__", &WstringArray::setitem_string_scalar_mask)
        .def("__setitem__", &WstringArray::setitem_string_vector)
        .def("__setitem__", &WstringArray::setitem_string_vector_mask)
        .def("__len__",&WstringArray::len)
        .def(self == self) // NOSONAR - suppress SonarCloud bug report.
        .def(self == other<std::wstring>())
        .def(other<std::wstring>() == self)
        .def(self != self) // NOSONAR - suppress SonarCloud bug report.
        .def(self != other<std::wstring>())
        .def(other<std::wstring>() != self)
        ;
}

} // namespace PyImath
