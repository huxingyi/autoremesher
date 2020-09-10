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

#include "PyImathStringTable.h"
#include <Iex.h>
#include <limits>
#include "PyImathExport.h"

namespace PyImath {

template<class T>
StringTableIndex
StringTableT<T>::lookup(const T &s) const
{
    typedef typename Table::template nth_index<1>::type StringSet;
    const StringSet &strings = _table.template get<1>();

    typename StringSet::const_iterator it = strings.find(s);
    if (it == strings.end()) {
        throw IEX_NAMESPACE::ArgExc("String table access out of bounds");
    }

    return it->i;
}

template<class T>
const T &
StringTableT<T>::lookup(StringTableIndex index) const
{
    typedef typename Table::template nth_index<0>::type IndexSet;
    const IndexSet &indices = _table.template get<0>();

    typename IndexSet::const_iterator it = indices.find(index);
    if (it == indices.end()) {
        throw IEX_NAMESPACE::ArgExc("String table access out of bounds");
    }

    return it->s;
}

template<class T>
StringTableIndex
StringTableT<T>::intern(const T &s)
{
    typedef typename Table::template nth_index<1>::type StringSet;
    const StringSet &strings = _table.template get<1>();

    typename StringSet::const_iterator it = strings.find(s);
    if (it == strings.end()) {
        size_t next_index = _table.size();
        if (next_index > std::numeric_limits<StringTableIndex::index_type>::max()) {
            throw IEX_NAMESPACE::ArgExc("Unable to intern string - string table would exceed maximum size");
        }
        StringTableIndex index = StringTableIndex(StringTableIndex::index_type(next_index));
        _table.insert(StringTableEntry<T>(index,s));
        return index;
    }

    return it->i;
}

template<class T>
size_t
StringTableT<T>::size() const
{
    return _table.size();
}

template<class T>
bool
StringTableT<T>::hasString(const T &s) const
{
    typedef typename Table::template nth_index<1>::type StringSet;
    const StringSet &strings = _table.template get<1>();
    return strings.find(s) != strings.end();
}

template<class T>
bool
StringTableT<T>::hasStringIndex(const StringTableIndex &s) const
{
    typedef typename Table::template nth_index<0>::type IndexSet;
    const IndexSet &indices = _table.template get<0>();
    return indices.find(s) != indices.end();
}

namespace {
template class PYIMATH_EXPORT StringTableDetailT<std::string>;
template class PYIMATH_EXPORT StringTableDetailT<std::wstring>;
}

template class PYIMATH_EXPORT StringTableT<std::string>;
template class PYIMATH_EXPORT StringTableT<std::wstring>;

} // namespace PyImath
