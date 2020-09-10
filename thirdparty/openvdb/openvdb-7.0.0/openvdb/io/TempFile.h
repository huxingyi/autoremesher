// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @file TempFile.h

#ifndef OPENVDB_IO_TEMPFILE_HAS_BEEN_INCLUDED
#define OPENVDB_IO_TEMPFILE_HAS_BEEN_INCLUDED

#include <openvdb/version.h>
#include <memory>
#include <ostream>


namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace io {

/// Output stream to a unique temporary file
class OPENVDB_API TempFile: public std::ostream
{
public:
    /// @brief Create and open a unique file.
    /// @details On UNIX systems, the file is created in the directory specified by
    /// the environment variable @c OPENVDB_TEMP_DIR, if that variable is defined,
    /// or else in the directory specified by @c TMPDIR, if that variable is defined.
    /// Otherwise (and on non-UNIX systems), the file is created in the system default
    /// temporary directory.
    TempFile();
    ~TempFile();

    /// Return the path to the temporary file.
    const std::string& filename() const;

    /// Return @c true if the file is open for writing.
    bool is_open() const;

    /// Close the file.
    void close();

private:
    struct TempFileImpl;
    std::unique_ptr<TempFileImpl> mImpl;
};

} // namespace io
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb

#endif // OPENVDB_IO_TEMPFILE_HAS_BEEN_INCLUDED
