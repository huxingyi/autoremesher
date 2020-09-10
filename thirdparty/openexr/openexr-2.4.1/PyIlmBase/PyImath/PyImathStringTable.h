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

#ifndef _PyImathStringTable_h_
#define _PyImathStringTable_h_

#include <string>
#include <stdint.h>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>

namespace PyImath {

// define a separate index type so as not to have
// any confusion between ints and indices
struct StringTableIndex
{
  public:
    typedef uint32_t index_type;

    // the default constructor was a private member before to prevent
    // the empty instantiation. But, it became public now to resolve 
    // a linking error on windows platform. PyImathStringArray is
    // exposed with PYIMATH_EXPORT, This causes to expose all the members
    // of PyImathFixedArray<StringTableIndex> also.
    
    StringTableIndex() : _index(0) {}
    StringTableIndex (const StringTableIndex &si) : _index (si._index) {}
    explicit StringTableIndex (index_type i) : _index (i) {}

    const StringTableIndex & operator = (const StringTableIndex &si)
    { 
        _index = si._index; return *this;
    }

    bool operator == (const StringTableIndex &si) const
    { 
        return _index == si._index;
    }

    bool operator != (const StringTableIndex &si) const
    {
        return _index != si._index;
    }

    // operator less for sorting
    bool operator < (const StringTableIndex &si) const
    {
        return _index < si._index;
    }

    index_type index() const { return _index; }

  private:
    index_type _index;
};

} // namespace PyImath

// Add a type trait for string indices to allow use in an AlignedArray
namespace boost {
    template <> struct is_pod< ::PyImath::StringTableIndex>
    {
        BOOST_STATIC_CONSTANT(bool,value=true); 
    };
} // namespace boost

namespace PyImath {

//
// A string table entry containing a unique index and string
template<class T>
struct StringTableEntry
{
    StringTableEntry(StringTableIndex ii,const T &ss) : i(ii), s(ss) {}
    StringTableIndex i;
    T                s;
};

namespace {

using boost::multi_index_container;
using namespace boost::multi_index;

//
// A map data structure for string strings.
// It exposes two index types : StringTableIndex and string
//
template<class T>
class StringTableDetailT {
    public:
    typedef boost::multi_index_container<
        StringTableEntry<T>,
        indexed_by<
            ordered_unique<member<StringTableEntry<T>,StringTableIndex,&StringTableEntry<T>::i> >,
            ordered_unique<member<StringTableEntry<T>,T,&StringTableEntry<T>::s> >
        > 
    > StringTableContainer;
};

} // namespace

typedef StringTableDetailT<std::string> StringTableDetail;
typedef StringTableDetailT<std::wstring> WStringTableDetail;

//
// Storage class for storing unique string elements.
//
//
template<class T>
class StringTableT
{
  public:

    // look up a string table entry either by value or index
    StringTableIndex    lookup(const T &s) const;
    const T &           lookup(StringTableIndex index) const;

    // return the index to a string table entry, adding if not found
    StringTableIndex    intern(const T &i);

    size_t              size() const;
    bool                hasString(const T &s) const;
    bool                hasStringIndex(const StringTableIndex &s) const;
    
  private:

    typedef typename StringTableDetailT<T>::StringTableContainer Table;
    Table _table;
};

typedef StringTableT<std::string> StringTable;
typedef StringTableT<std::wstring> WStringTable;

} // namespace PyImath

#endif
