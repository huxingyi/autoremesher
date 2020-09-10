// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @file points/AttributeGroup.cc

#include "AttributeGroup.h"


namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace points {


////////////////////////////////////////

// GroupHandle implementation


GroupHandle::GroupHandle(const GroupAttributeArray& array, const GroupType& offset)
        : mArray(array)
        , mBitMask(static_cast<GroupType>(1 << offset))
{
    assert(isGroup(mArray));

    // load data if delay-loaded

    mArray.loadData();
}


GroupHandle::GroupHandle(const GroupAttributeArray& array, const GroupType& bitMask,
            BitMask)
    : mArray(array)
    , mBitMask(bitMask)
{
    assert(isGroup(mArray));

    // load data if delay-loaded

    mArray.loadData();
}


bool GroupHandle::get(Index n) const
{
    return (mArray.get(n) & mBitMask) == mBitMask;
}


bool GroupHandle::getUnsafe(Index n) const
{
    return (mArray.getUnsafe(n) & mBitMask) == mBitMask;
}


////////////////////////////////////////

// GroupWriteHandle implementation


GroupWriteHandle::GroupWriteHandle(GroupAttributeArray& array, const GroupType& offset)
    : GroupHandle(array, offset)
{
    assert(isGroup(mArray));
}


void GroupWriteHandle::set(Index n, bool on)
{
    const GroupType& value = mArray.get(n);

    GroupAttributeArray& array(const_cast<GroupAttributeArray&>(mArray));

    if (on)     array.set(n, value | mBitMask);
    else        array.set(n, value & ~mBitMask);
}


bool GroupWriteHandle::collapse(bool on)
{
    using ValueT = GroupAttributeArray::ValueType;

    GroupAttributeArray& array(const_cast<GroupAttributeArray&>(mArray));

    array.compact();

    if (this->isUniform()) {
        if (on)     array.collapse(static_cast<ValueT>(array.get(0) | mBitMask));
        else        array.collapse(static_cast<ValueT>(array.get(0) & ~mBitMask));
        return true;
    }

    for (Index i = 0; i < array.size(); i++) {
        if (on)     array.set(i, static_cast<ValueT>(array.get(i) | mBitMask));
        else        array.set(i, static_cast<ValueT>(array.get(i) & ~mBitMask));
    }

    return false;
}


bool GroupWriteHandle::compact()
{
    GroupAttributeArray& array(const_cast<GroupAttributeArray&>(mArray));

    return array.compact();
}


////////////////////////////////////////


} // namespace points
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb
