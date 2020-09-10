// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @file OpenVDBData.cc
/// @author FX R&D OpenVDB team


#include "OpenVDBData.h"
#include <openvdb/io/Stream.h>

#include <maya/MGlobal.h>
#include <maya/MIOStream.h>
#include <maya/MPoint.h>
#include <maya/MArgList.h>

#include <limits>
#include <iostream>

////////////////////////////////////////


const MTypeId OpenVDBData::id(0x00108A50);
const MString OpenVDBData::typeName("OpenVDBData");


////////////////////////////////////////


void*
OpenVDBData::creator()
{
    return new OpenVDBData();
}


OpenVDBData::OpenVDBData(): MPxData()
{
}


OpenVDBData::~OpenVDBData()
{
}


void
OpenVDBData::copy(const MPxData& other)
{
    if (other.typeId() == OpenVDBData::id) {
        const OpenVDBData& rhs = static_cast<const OpenVDBData&>(other);
        if (&mGrids != &rhs.mGrids) {
            // shallow-copy the grids from the rhs container.
            mGrids.clear();
            insert(rhs.mGrids);
        }
    }
}


MTypeId
OpenVDBData::typeId() const
{
    return OpenVDBData::id;
}


MString
OpenVDBData::name() const
{
    return OpenVDBData::typeName;
}


MStatus
OpenVDBData::readASCII(const MArgList&, unsigned&)
{
    return MS::kFailure;
}


MStatus
OpenVDBData::writeASCII(ostream&)
{
    return MS::kFailure;
}


MStatus
OpenVDBData::readBinary(istream& in, unsigned)
{
    auto grids = openvdb::io::Stream(in).getGrids();
    mGrids.clear();
    insert(*grids);
    return in.fail() ? MS::kFailure : MS::kSuccess;
}


MStatus
OpenVDBData::writeBinary(ostream& out)
{
    openvdb::io::Stream(out).write(mGrids);
    return out.fail() ? MS::kFailure : MS::kSuccess;
}


////////////////////////////////////////


size_t
OpenVDBData::numberOfGrids() const
{
    return mGrids.size();
}


const openvdb::GridBase&
OpenVDBData::grid(size_t index) const
{
    return *(mGrids[index]);
}


openvdb::GridBase::ConstPtr
OpenVDBData::gridPtr(size_t index) const
{
    return mGrids[index];
}


void
OpenVDBData::duplicate(const OpenVDBData& rhs)
{
    mGrids.clear();
    for (const auto& gridPtr: rhs.mGrids) {
        mGrids.push_back(gridPtr->copyGrid());
    }
}

void
OpenVDBData::insert(const openvdb::GridBase::ConstPtr& grid)
{
    mGrids.push_back(grid);
}


void
OpenVDBData::insert(const openvdb::GridBase& grid)
{
     mGrids.push_back(grid.copyGrid());
}


void
OpenVDBData::insert(const openvdb::GridPtrVec& rhs)
{
    mGrids.reserve(mGrids.size() + rhs.size());
    for (const auto& gridPtr: rhs) {
        mGrids.push_back(gridPtr->copyGrid());
    }
}

void
OpenVDBData::insert(const openvdb::GridCPtrVec& rhs)
{
    mGrids.reserve(mGrids.size() + rhs.size());
    for (const auto& gridPtr: rhs) {
        mGrids.push_back(gridPtr->copyGrid());
    }
}


void
OpenVDBData::write(const openvdb::io::File& file,
    const openvdb::MetaMap& metadata) const
{
    file.write(mGrids, metadata);
}


////////////////////////////////////////

