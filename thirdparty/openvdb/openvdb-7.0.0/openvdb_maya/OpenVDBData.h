// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @file OpenVDBData.h
/// @author FX R&D OpenVDB team


#ifndef OPENVDB_MAYA_DATA_HAS_BEEN_INCLUDED
#define OPENVDB_MAYA_DATA_HAS_BEEN_INCLUDED


#include <openvdb/Platform.h>
#include <openvdb/openvdb.h>

#include <maya/MPxData.h>
#include <maya/MTypeId.h>
#include <maya/MString.h>
#include <maya/MObjectArray.h>

#include <iosfwd>

////////////////////////////////////////


class OpenVDBData : public MPxData
{
public:
    OpenVDBData();
    virtual ~OpenVDBData();

    size_t numberOfGrids() const;

    /// @brief  return a constant reference to the specified grid.
    const openvdb::GridBase& grid(size_t index) const;

    /// @brief  return a constant pointer to the specified grid.
    openvdb::GridBase::ConstPtr gridPtr(size_t index) const;

    /// @brief clears this container and duplicates the @c rhs grid container.
    void duplicate(const OpenVDBData& rhs);

    /// @brief Append the given grid to this container.
    void insert(const openvdb::GridBase::ConstPtr&);
    /// @brief Append a shallow copy of the given grid to this container.
    void insert(const openvdb::GridBase&);
    /// @brief Append shallow copies of the given grids to this container.
    void insert(const openvdb::GridPtrVec&);
    /// @brief Append shallow copies of the given grids to this container.
    void insert(const openvdb::GridCPtrVec&);


    void write(const openvdb::io::File& file,
        const openvdb::MetaMap& = openvdb::MetaMap()) const;


    /// @{
    // required maya interface methods
    static void* creator();

    virtual MStatus readASCII(const MArgList&, unsigned&);
    virtual MStatus writeASCII(ostream&);

    virtual MStatus readBinary(istream&, unsigned length);
    virtual MStatus writeBinary(ostream&);

    virtual void copy(const MPxData&);
    MTypeId typeId() const;
    MString name() const;

    static const MString typeName;
    static const MTypeId id;
    /// @}

private:
    openvdb::GridCPtrVec mGrids;
};


////////////////////////////////////////


#endif // OPENVDB_MAYA_DATA_HAS_BEEN_INCLUDED
