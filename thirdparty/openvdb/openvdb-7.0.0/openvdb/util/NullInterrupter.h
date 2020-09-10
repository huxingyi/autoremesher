// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file NullInterrupter.h

#ifndef OPENVDB_UTIL_NULL_INTERRUPTER_HAS_BEEN_INCLUDED
#define OPENVDB_UTIL_NULL_INTERRUPTER_HAS_BEEN_INCLUDED

#include <openvdb/version.h>

namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace util {

/// @brief Dummy NOOP interrupter class defining interface
///
/// This shows the required interface for the @c InterrupterType template argument
/// using by several threaded applications (e.g. tools/PointAdvect.h). The host
/// application calls start() at the beginning of an interruptible operation, end()
/// at the end of the operation, and wasInterrupted() periodically during the operation.
/// If any call to wasInterrupted() returns @c true, the operation will be aborted.
/// @note This Dummy interrupter will NEVER interrupt since wasInterrupted() always
/// returns false!
struct NullInterrupter
{
    /// Default constructor
    NullInterrupter () {}
    /// Signal the start of an interruptible operation.
    /// @param name  an optional descriptive name for the operation
    void start(const char* name = nullptr) { (void)name; }
    /// Signal the end of an interruptible operation.
    void end() {}
    /// Check if an interruptible operation should be aborted.
    /// @param percent  an optional (when >= 0) percentage indicating
    ///     the fraction of the operation that has been completed
    /// @note this method is assumed to be thread-safe. The current
    /// implementation is clearly a NOOP and should compile out during
    /// optimization!
    inline bool wasInterrupted(int percent = -1) { (void)percent; return false; }
};

/// This method allows NullInterrupter::wasInterrupted to be compiled
/// out when client code only has a pointer (vs reference) to the interrupter.
///
/// @note This is a free-standing function since C++ doesn't allow for
/// partial template specialization (in client code of the interrupter).
template <typename T>
inline bool wasInterrupted(T* i, int percent = -1) { return i && i->wasInterrupted(percent); }

/// Specialization for NullInterrupter
template<>
inline bool wasInterrupted<util::NullInterrupter>(util::NullInterrupter*, int) { return false; }

} // namespace util
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb

#endif // OPENVDB_UTIL_NULL_INTERRUPTER_HAS_BEEN_INCLUDED
