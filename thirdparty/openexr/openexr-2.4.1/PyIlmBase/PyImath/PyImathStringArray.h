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

#ifndef _PyImathStringArray_h_
#define _PyImathStringArray_h_

#include "PyImathFixedArray.h"
#include "PyImathStringTable.h"

//
// A fixed lengy array class for string and wide string type in python.
// The implementation of StringArray is does not follow other FixeArray
// types. StringArray de-duplicate repeated strings using StringTable
// internally for compact memory usage.
//
namespace PyImath {

template <class T>
class StringArrayT : public FixedArray<StringTableIndex>
{
  public:
    typedef T   BaseType;
    typedef FixedArray<StringTableIndex> super;

    static StringArrayT<T>* createDefaultArray(size_t length);
    static StringArrayT<T>* createUniformArray(const T& initialValue, size_t length);
    static StringArrayT<T>* createFromRawArray(const T* rawArray, size_t length);

    StringArrayT(StringTableT<T> &table, StringTableIndex *ptr, size_t length, size_t stride = 1, boost::any tableHandle = boost::any());

    StringArrayT(StringTableT<T> &table, StringTableIndex *ptr, size_t length, size_t stride, boost::any handle, boost::any tableHandle = boost::any()) ;

    const StringTableT<T> & stringTable() const { return _table; }

    T  getitem_string(Py_ssize_t index) const {return _table.lookup(getitem(index)); }
    StringArrayT* getslice_string(PyObject *index) const;

    void setitem_string_scalar(PyObject *index, const T &data);

    void setitem_string_scalar_mask(const FixedArray<int> &mask, const T &data);
    void setitem_string_vector(PyObject *index, const StringArrayT<T> &data);
    void setitem_string_vector_mask(const FixedArray<int> &mask, const StringArrayT<T> &data);

  private:
    typedef StringArrayT<T>     this_type;

    StringTableT<T>             &_table;
    // StringArray can borrow a string table from somewhere else or maintain 
    // its own string table. This handle optionally stores a shared pointer to 
    // a allocated StringTable class instance
    boost::any                  _tableHandle;
};

template<class T>
FixedArray<int> operator == (const StringArrayT<T> &a0, const StringArrayT<T> &a1); 
template<class T>
FixedArray<int> operator == (const StringArrayT<T> &a0, const T &v1);
template<class T>
FixedArray<int> operator == (const T &v1,const StringArrayT<T> &a0);
template<class T>
FixedArray<int> operator != (const StringArrayT<T> &a0, const StringArrayT<T> &a1);
template<class T>
FixedArray<int> operator != (const StringArrayT<T> &a0, const T &v1);
template<class T>
FixedArray<int> operator != (const T &v1,const StringArrayT<T> &a0);

typedef StringArrayT<std::string> StringArray;
typedef StringArrayT<std::wstring> WstringArray;

} // namespace PyImath

#endif
