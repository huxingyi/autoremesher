// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @file points/AttributeArrayString.cc

#include "AttributeArrayString.h"

#include <openvdb/Metadata.h>
#include <openvdb/MetaMap.h>

#include <tbb/parallel_sort.h>

#include <string>

namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace points {


namespace {

    bool isStringMeta(const Name& key, const Metadata::ConstPtr& meta)
    {
        // ensure the metadata is StringMetadata
        if (meta->typeName() != "string")           return false;
        // string attribute metadata must have a key that starts "string:"
        if (key.compare(0, 7, "string:") != 0)      return false;

        return true;
    }

    Name getStringKey(const StringIndexType index)
    {
        return "string:" + std::to_string(index - 1);
    }

    StringIndexType getStringIndex(const Name& key)
    {
        const Name indexStr = key.substr(7, key.size() - 7);
        // extract the index as an unsigned integer
        return static_cast<StringIndexType>(std::stoul(indexStr)) + 1;
    }

} // namespace


////////////////////////////////////////


// StringMetaInserter implementation


StringMetaInserter::StringMetaInserter(MetaMap& metadata)
    : mMetadata(metadata)
    , mIdBlocks()
    , mValues()
{
    // populate the cache
    resetCache();
}


void StringMetaInserter::insert(const Name& name)
{
    using IterT = std::vector<std::pair<Index, Index>>::iterator;

    // if name already exists, do nothing

    if (mValues.count(name))  return;

    // look through the id blocks for the next available index

    Index index = 1;
    IterT iter = mIdBlocks.begin();
    for (; iter != mIdBlocks.end(); ++iter) {
        const Index start = iter->first;
        const Index end = start + iter->second;

        if (index < start || index >= end) break;
        index = end;
    }

    // index now holds the next valid index. if it's 1 (the beginning
    // iterator) no initial block exists - add it

    IterT block;
    if (iter == mIdBlocks.begin()) {
        block = mIdBlocks.insert(iter, {1, 1});
        iter = std::next(block);
    }
    else {
        // accumulate the id block size where the next index is going
        block = std::prev(iter);
        block->second += 1;
    }

    // see if this block and the next block can be compacted

    if (iter != mIdBlocks.end() &&
        block->second + 1 == iter->first) {
        block->second += iter->second;
        mIdBlocks.erase(iter);
    }

    // insert into metadata

    const Name key = getStringKey(index);
    mMetadata.insertMeta(key, StringMetadata(name));

    // update the cache

    mValues.emplace(name);
}


void StringMetaInserter::resetCache()
{
    mValues.clear();
    mIdBlocks.clear();

    std::vector<Index> stringIndices;

    for (auto it = mMetadata.beginMeta(), itEnd = mMetadata.endMeta(); it != itEnd; ++it) {
        const Name& key = it->first;
        const Metadata::ConstPtr meta = it->second;

        // ensure the metadata is StringMetadata and key starts "string:"
        if (!isStringMeta(key, meta))   continue;

        // extract index
        stringIndices.emplace_back(getStringIndex(key));

        // extract value from metadata and add to cache
        const StringMetadata* stringMeta = static_cast<const StringMetadata*>(meta.get());
        assert(stringMeta);
        mValues.insert(stringMeta->value());
    }

    if (stringIndices.empty()) return;

    tbb::parallel_sort(stringIndices.begin(), stringIndices.end());

    // bucket string indices

    Index key = stringIndices.front();
    Index size = 0;

    // For each id, see if it's adjacent id is sequentially increasing and continue to
    // track how many are until we find a value that isn't. Store the start and length
    // of each of these blocks. For example, the following container could be created
    // consisting of 3 elements:
    //   key  ->  size
    //   -------------
    //   7    ->  1000  (values 7->1007)
    //   1020 ->  5     (values 1020->1025)
    //   2013 ->  30    (values 2013->2043)
    // Note that the end value is exclusive (values 1007, 1025 and 2043 do not exist
    // given the above example)

    for (const Index id : stringIndices) {
        if (key + size != id) {
            assert(size > 0);
            mIdBlocks.emplace_back(key, size);
            size = 0;
            key = id;
        }
        ++size;
    }

    // add the last block
    mIdBlocks.emplace_back(key, size);
}


////////////////////////////////////////

// StringAttributeHandle implementation


StringAttributeHandle::Ptr
StringAttributeHandle::create(const AttributeArray& array, const MetaMap& metadata, const bool preserveCompression)
{
    return std::make_shared<StringAttributeHandle>(array, metadata, preserveCompression);
}


StringAttributeHandle::StringAttributeHandle(const AttributeArray& array,
                                             const MetaMap& metadata,
                                             const bool preserveCompression)
        : mHandle(array, preserveCompression)
        , mMetadata(metadata)
{
    if (!isString(array)) {
        OPENVDB_THROW(TypeError, "Cannot create a StringAttributeHandle for an attribute array that is not a string.");
    }
}


Name StringAttributeHandle::get(Index n, Index m) const
{
    Name name;
    this->get(name, n, m);
    return name;
}


void StringAttributeHandle::get(Name& name, Index n, Index m) const
{
    StringIndexType index = mHandle.get(n, m);

    // index zero is reserved for an empty string

    if (index == 0) {
        name = "";
        return;
    }

    const Name key = getStringKey(index);

    // key is assumed to exist in metadata

    openvdb::StringMetadata::ConstPtr meta = mMetadata.getMetadata<StringMetadata>(key);

    if (!meta) {
        OPENVDB_THROW(LookupError, "String attribute cannot be found with index - \"" << index << "\".");
    }

    name = meta->value();
}

const AttributeArray& StringAttributeHandle::array() const
{
    return mHandle.array();
}


////////////////////////////////////////

// StringAttributeWriteHandle implementation

StringAttributeWriteHandle::Ptr
StringAttributeWriteHandle::create(AttributeArray& array, const MetaMap& metadata, const bool expand)
{
    return std::make_shared<StringAttributeWriteHandle>(array, metadata, expand);
}


StringAttributeWriteHandle::StringAttributeWriteHandle(AttributeArray& array,
                                                       const MetaMap& metadata,
                                                       const bool expand)
    : StringAttributeHandle(array, metadata, /*preserveCompression=*/ false)
    , mWriteHandle(array, expand)
{
    // populate the cache
    resetCache();
}


void StringAttributeWriteHandle::expand(bool fill)
{
    mWriteHandle.expand(fill);
}


void StringAttributeWriteHandle::collapse()
{
    // zero is used for an empty string
    mWriteHandle.collapse(0);
}


void StringAttributeWriteHandle::collapse(const Name& name)
{
    Index index = getIndex(name);
    mWriteHandle.collapse(index);
}


bool StringAttributeWriteHandle::compact()
{
    return mWriteHandle.compact();
}


void StringAttributeWriteHandle::fill(const Name& name)
{
    Index index = getIndex(name);
    mWriteHandle.fill(index);
}


void StringAttributeWriteHandle::set(Index n, const Name& name)
{
    Index index = getIndex(name);
    mWriteHandle.set(n, /*stride*/0, index);
}


void StringAttributeWriteHandle::set(Index n, Index m, const Name& name)
{
    Index index = getIndex(name);
    mWriteHandle.set(n, m, index);
}


void StringAttributeWriteHandle::resetCache()
{
    mCache.clear();

    // re-populate the cache

    for (auto it = mMetadata.beginMeta(), itEnd = mMetadata.endMeta(); it != itEnd; ++it) {
        const Name& key = it->first;
        const Metadata::Ptr meta = it->second;

        // ensure the metadata is StringMetadata and key starts "string:"
        if (!isStringMeta(key, meta))   continue;

        const auto* stringMeta = static_cast<StringMetadata*>(meta.get());
        assert(stringMeta);

        // remove "string:"
        Index index = getStringIndex(key);

        // add to the cache
        mCache[stringMeta->value()] = index;
    }
}


AttributeArray& StringAttributeWriteHandle::array()
{
    return mWriteHandle.array();
}


bool StringAttributeWriteHandle::contains(const Name& name) const
{
    // empty strings always have an index at index zero
    if (name.empty())   return true;
    return mCache.find(name) != mCache.end();
}


Index StringAttributeWriteHandle::getIndex(const Name& name) const
{
    // zero used for an empty string
    if (name.empty())   return Index(0);

    auto it = mCache.find(name);

    if (it == mCache.end()) {
        OPENVDB_THROW(LookupError, "String does not exist in Metadata, insert it and reset the cache - \"" << name << "\".");
    }

    return it->second;
}


////////////////////////////////////////


} // namespace points
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb
