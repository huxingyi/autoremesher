// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @file TempFile.cc

#include "TempFile.h"

#include <openvdb/Exceptions.h>
#ifndef _MSC_VER
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <cstdlib> // for std::getenv(), mkstemp()
#include <sys/types.h> // for mode_t
#include <sys/stat.h> // for mkdir(), umask()
#include <unistd.h> // for access()
#else
#include <fstream> // for std::filebuf
#endif
#include <cstdio> // for std::tmpnam(), L_tmpnam, P_tmpdir
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace io {

struct TempFile::TempFileImpl
{
    const std::string& filename() const { return mPath; }

    bool is_open() const { return mBuffer.is_open(); }

    /// @internal boost::filesystem::unique_path(), etc. might be useful here,
    /// but as of 9/2014, Houdini ships without the Boost.Filesystem library,
    /// which makes it much less convenient to use that library.
#ifndef _MSC_VER
    TempFileImpl(std::ostream& os): mFileDescr(-1) { this->init(os); }

    void init(std::ostream& os)
    {
        std::string fn = this->getTempDir() + "/openvdb_temp_XXXXXX";
        std::vector<char> fnbuf(fn.begin(), fn.end());
        fnbuf.push_back(char(0));

        //const mode_t savedMode = ::umask(~(S_IRUSR | S_IWUSR));
        mFileDescr = ::mkstemp(&fnbuf[0]);
        //::umask(savedMode);
        if (mFileDescr < 0) {
            OPENVDB_THROW(IoError, "failed to generate temporary file");
        }

        mPath.assign(&fnbuf[0]);

        mDevice = DeviceType(mFileDescr, boost::iostreams::never_close_handle);
        mBuffer.open(mDevice);
        os.rdbuf(&mBuffer);

        if (!os.good()) {
            OPENVDB_THROW(IoError, "failed to open temporary file " + mPath);
        }
    }

    void close() { mBuffer.close(); if (mFileDescr >= 0) ::close(mFileDescr); }

    static std::string getTempDir()
    {
        if (const char* dir = std::getenv("OPENVDB_TEMP_DIR")) {
            if (0 != ::access(dir, F_OK)) {
                ::mkdir(dir, S_IRUSR | S_IWUSR | S_IXUSR);
                if (0 != ::access(dir, F_OK)) {
                    OPENVDB_THROW(IoError,
                        "failed to create OPENVDB_TEMP_DIR (" + std::string(dir) + ")");
                }
            }
            return dir;
        }
        if (const char* dir = std::getenv("TMPDIR")) return dir;
        return P_tmpdir;
    }

    using DeviceType = boost::iostreams::file_descriptor_sink;
    using BufferType = boost::iostreams::stream_buffer<boost::iostreams::file_descriptor_sink>;

    std::string mPath;
    DeviceType mDevice;
    BufferType mBuffer;
    int mFileDescr;
#else // _MSC_VER
    // Use only standard library routines; no POSIX.

    TempFileImpl(std::ostream& os) { this->init(os); }

    void init(std::ostream& os)
    {
        char fnbuf[L_tmpnam];
        const char* filename = std::tmpnam(fnbuf);
        if (!filename) {
            OPENVDB_THROW(IoError, "failed to generate name for temporary file");
        }
        /// @todo This is not safe, since another process could open a file
        /// with this name before we do.  Unfortunately, there is no safe,
        /// portable way to create a temporary file.
        mPath = filename;

        const std::ios_base::openmode mode = (std::ios_base::out | std::ios_base::binary);
        os.rdbuf(mBuffer.open(mPath.c_str(), mode));
        if (!os.good()) {
            OPENVDB_THROW(IoError, "failed to open temporary file " + mPath);
        }
    }

    void close() { mBuffer.close(); }

    std::string mPath;
    std::filebuf mBuffer;
#endif // _MSC_VER

private:
    TempFileImpl(const TempFileImpl&); // disable copying
    TempFileImpl& operator=(const TempFileImpl&); // disable assignment
};


TempFile::TempFile(): std::ostream(nullptr), mImpl(new TempFileImpl(*this)) {}
TempFile::~TempFile() { this->close(); }
const std::string& TempFile::filename() const { return mImpl->filename(); }
bool TempFile::is_open() const { return mImpl->is_open(); }
void TempFile::close() { mImpl->close(); }

} // namespace io
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb
