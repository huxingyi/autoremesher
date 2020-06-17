#pragma once

/// Get an internal name for a type
/// Important, this is depends on compilers and versions, do NOT use in file formats!
/// This provides property type safety when only limited RTTI is available
/// Solution adapted from OpenVolumeMesh

#include <string>
#include <typeinfo>

namespace OpenMesh {

template <typename T>
std::string get_type_name()
{
#ifdef _MSC_VER
    // MSVC'S type_name returns only a friendly name with name() method,
    // to get a unique name use raw_name() method instead
    return typeid(T).raw_name();
#else
    // GCC and clang curently return mangled name as name(), there is no raw_name() method
    return typeid(T).name();
#endif
}

}
