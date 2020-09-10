// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#ifndef OPENVDB_IO_STREAM_HAS_BEEN_INCLUDED
#define OPENVDB_IO_STREAM_HAS_BEEN_INCLUDED

#include "Archive.h"
#include <iosfwd>
#include <memory>


namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace io {

class GridDescriptor;


/// Grid archive associated with arbitrary input and output streams (not necessarily files)
class OPENVDB_API Stream: public Archive
{
public:
    /// @brief Read grids from an input stream.
    /// @details If @a delayLoad is true, map the contents of the input stream
    /// into memory and enable delayed loading of grids.
    /// @note Define the environment variable @c OPENVDB_DISABLE_DELAYED_LOAD
    /// to disable delayed loading unconditionally.
    explicit Stream(std::istream&, bool delayLoad = true);

    /// Construct an archive for stream output.
    Stream();
    /// Construct an archive for output to the given stream.
    explicit Stream(std::ostream&);

    Stream(const Stream&);
    Stream& operator=(const Stream&);

    ~Stream() override;

    /// @brief Return a copy of this archive.
    Archive::Ptr copy() const override;

    /// Return the file-level metadata in a newly created MetaMap.
    MetaMap::Ptr getMetadata() const;

    /// Return pointers to the grids that were read from the input stream.
    GridPtrVecPtr getGrids();

    /// @brief Write the grids in the given container to this archive's output stream.
    /// @throw ValueError if this archive was constructed without specifying an output stream.
    void write(const GridCPtrVec&, const MetaMap& = MetaMap()) const override;

    /// @brief Write the grids in the given container to this archive's output stream.
    /// @throw ValueError if this archive was constructed without specifying an output stream.
    template<typename GridPtrContainerT>
    void write(const GridPtrContainerT&, const MetaMap& = MetaMap()) const;

private:
    /// Create a new grid of the type specified by the given descriptor,
    /// then populate the grid from the given input stream.
    /// @return the newly created grid.
    GridBase::Ptr readGrid(const GridDescriptor&, std::istream&) const;

    void writeGrids(std::ostream&, const GridCPtrVec&, const MetaMap&) const;


    struct Impl;
    std::unique_ptr<Impl> mImpl;
};


////////////////////////////////////////


template<typename GridPtrContainerT>
inline void
Stream::write(const GridPtrContainerT& container, const MetaMap& metadata) const
{
    GridCPtrVec grids;
    std::copy(container.begin(), container.end(), std::back_inserter(grids));
    this->write(grids, metadata);
}

} // namespace io
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb

#endif // OPENVDB_IO_STREAM_HAS_BEEN_INCLUDED
