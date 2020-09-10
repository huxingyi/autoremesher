// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include "GridDescriptor.h"

#include <openvdb/Exceptions.h>
#include <boost/algorithm/string/predicate.hpp> // for boost::ends_with()
#include <boost/algorithm/string/erase.hpp> // for boost::erase_last()
#include <sstream>


namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace io {

namespace {

// In order not to break backward compatibility with existing VDB files,
// grids stored using 16-bit half floats are flagged by adding the following
// suffix to the grid's type name on output.  The suffix is removed on input
// and the grid's "save float as half" flag set accordingly.
const char* HALF_FLOAT_TYPENAME_SUFFIX = "_HalfFloat";

const char* SEP = "\x1e"; // ASCII "record separator"

}


GridDescriptor::GridDescriptor():
    mSaveFloatAsHalf(false),
    mGridPos(0),
    mBlockPos(0),
    mEndPos(0)
{
}

GridDescriptor::GridDescriptor(const Name &name, const Name &type, bool half):
    mGridName(stripSuffix(name)),
    mUniqueName(name),
    mGridType(type),
    mSaveFloatAsHalf(half),
    mGridPos(0),
    mBlockPos(0),
    mEndPos(0)
{
}

GridDescriptor::~GridDescriptor()
{
}

void
GridDescriptor::writeHeader(std::ostream &os) const
{
    writeString(os, mUniqueName);

    Name gridType = mGridType;
    if (mSaveFloatAsHalf) gridType += HALF_FLOAT_TYPENAME_SUFFIX;
    writeString(os, gridType);

    writeString(os, mInstanceParentName);
}

void
GridDescriptor::writeStreamPos(std::ostream &os) const
{
    os.write(reinterpret_cast<const char*>(&mGridPos), sizeof(boost::int64_t));
    os.write(reinterpret_cast<const char*>(&mBlockPos), sizeof(boost::int64_t));
    os.write(reinterpret_cast<const char*>(&mEndPos), sizeof(boost::int64_t));
}

GridBase::Ptr
GridDescriptor::read(std::istream &is)
{
    // Read in the name.
    mUniqueName = readString(is);
    mGridName = stripSuffix(mUniqueName);

    // Read in the grid type.
    mGridType = readString(is);
    if (boost::ends_with(mGridType, HALF_FLOAT_TYPENAME_SUFFIX)) {
        mSaveFloatAsHalf = true;
        boost::erase_last(mGridType, HALF_FLOAT_TYPENAME_SUFFIX);
    }

    if (getFormatVersion(is) >= OPENVDB_FILE_VERSION_GRID_INSTANCING) {
        mInstanceParentName = readString(is);
    }

    // Create the grid of the type if it has been registered.
    if (!GridBase::isRegistered(mGridType)) {
        OPENVDB_THROW(LookupError, "Cannot read grid." <<
            " Grid type " << mGridType << " is not registered.");
    }
    // else
    GridBase::Ptr grid = GridBase::createGrid(mGridType);
    if (grid) grid->setSaveFloatAsHalf(mSaveFloatAsHalf);

    // Read in the offsets.
    is.read(reinterpret_cast<char*>(&mGridPos), sizeof(boost::int64_t));
    is.read(reinterpret_cast<char*>(&mBlockPos), sizeof(boost::int64_t));
    is.read(reinterpret_cast<char*>(&mEndPos), sizeof(boost::int64_t));

    return grid;
}

void
GridDescriptor::seekToGrid(std::istream &is) const
{
    is.seekg(mGridPos, std::ios_base::beg);
}

void
GridDescriptor::seekToBlocks(std::istream &is) const
{
    is.seekg(mBlockPos, std::ios_base::beg);
}

void
GridDescriptor::seekToEnd(std::istream &is) const
{
    is.seekg(mEndPos, std::ios_base::beg);
}


void
GridDescriptor::seekToGrid(std::ostream &os) const
{
    os.seekp(mGridPos, std::ios_base::beg);
}

void
GridDescriptor::seekToBlocks(std::ostream &os) const
{
    os.seekp(mBlockPos, std::ios_base::beg);
}

void
GridDescriptor::seekToEnd(std::ostream &os) const
{
    os.seekp(mEndPos, std::ios_base::beg);
}


////////////////////////////////////////


// static
Name
GridDescriptor::addSuffix(const Name& name, int n)
{
    std::ostringstream ostr;
    ostr << name << SEP << n;
    return ostr.str();
}


// static
Name
GridDescriptor::stripSuffix(const Name& name)
{
    return name.substr(0, name.find(SEP));
}


// static
std::string
GridDescriptor::nameAsString(const Name& name)
{
    std::string::size_type pos = name.find(SEP);
    if (pos == std::string::npos) return name;

    return name.substr(0, pos) + "[" + name.substr(pos + 1) + "]";
}


//static
Name
GridDescriptor::stringAsUniqueName(const std::string& s)
{
    Name ret = s;
    if (!ret.empty() && *ret.rbegin() == ']') { // found trailing ']'
        std::string::size_type pos = ret.find("[");
        // Replace "[N]" with SEP "N".
        if (pos != std::string::npos) {
            ret.resize(ret.size() - 1); // drop trailing ']'
            ret.replace(ret.find("["), 1, SEP);
        }
    }
    return ret;
}

} // namespace io
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb
