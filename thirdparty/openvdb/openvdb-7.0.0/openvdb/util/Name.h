// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#ifndef OPENVDB_UTIL_NAME_HAS_BEEN_INCLUDED
#define OPENVDB_UTIL_NAME_HAS_BEEN_INCLUDED

#include <openvdb/Platform.h>
#include <openvdb/version.h>
#include <string>
#include <iostream>
#include <vector>

namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {

typedef std::string Name;

inline Name
readString(std::istream& is)
{
    uint32_t size;
    is.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));
    std::string buffer(size, ' ');
    if (size>0) is.read(&buffer[0], size);
    return buffer;
}


inline void
writeString(std::ostream& os, const Name& name)
{
    uint32_t size = uint32_t(name.size());
    os.write(reinterpret_cast<char*>(&size), sizeof(uint32_t));
    os.write(&name[0], size);
}

} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb

#endif // OPENVDB_UTIL_NAME_HAS_BEEN_INCLUDED
