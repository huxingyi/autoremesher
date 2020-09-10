// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file File.h

#ifndef OPENVDB_IO_FILE_HAS_BEEN_INCLUDED
#define OPENVDB_IO_FILE_HAS_BEEN_INCLUDED

#include <openvdb/version.h>
#include "io.h" // for MappedFile::Notifier
#include "Archive.h"
#include "GridDescriptor.h"
#include <algorithm> // for std::copy()
#include <iosfwd>
#include <iterator> // for std::back_inserter()
#include <map>
#include <memory>
#include <string>


class TestFile;
class TestStream;

namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace io {

/// Grid archive associated with a file on disk
class OPENVDB_API File: public Archive
{
public:
    using NameMap = std::multimap<Name, GridDescriptor>;
    using NameMapCIter = NameMap::const_iterator;

    explicit File(const std::string& filename);
    ~File() override;

    /// @brief Copy constructor
    /// @details The copy will be closed and will not reference the same
    /// file descriptor as the original.
    File(const File& other);
    /// @brief Assignment
    /// @details After assignment, this File will be closed and will not
    /// reference the same file descriptor as the source File.
    File& operator=(const File& other);

    /// @brief Return a copy of this archive.
    /// @details The copy will be closed and will not reference the same
    /// file descriptor as the original.
    SharedPtr<Archive> copy() const override;

    /// @brief Return the name of the file with which this archive is associated.
    /// @details The file does not necessarily exist on disk yet.
    const std::string& filename() const;

    /// @brief Open the file, read the file header and the file-level metadata,
    /// and populate the grid descriptors, but do not load any grids into memory.
    /// @details If @a delayLoad is true, map the file into memory and enable delayed loading
    /// of grids, and if a notifier is provided, call it when the file gets unmapped.
    /// @note Define the environment variable @c OPENVDB_DISABLE_DELAYED_LOAD to disable
    /// delayed loading unconditionally.
    /// @throw IoError if the file is not a valid VDB file.
    /// @return @c true if the file's UUID has changed since it was last read.
    /// @see setCopyMaxBytes
    bool open(bool delayLoad = true, const MappedFile::Notifier& = MappedFile::Notifier());

    /// Return @c true if the file has been opened for reading.
    bool isOpen() const;

    /// Close the file once we are done reading from it.
    void close();

    /// @brief Return this file's current size on disk in bytes.
    /// @throw IoError if the file size cannot be determined.
    Index64 getSize() const;

    /// @brief Return the size in bytes above which this file will not be
    /// automatically copied during delayed loading.
    Index64 copyMaxBytes() const;
    /// @brief If this file is opened with delayed loading enabled, make a private copy
    /// of the file if its size in bytes is less than the specified value.
    /// @details Making a private copy ensures that the file can't change on disk
    /// before it has been fully read.
    /// @warning If the file is larger than this size, it is the user's responsibility
    /// to ensure that it does not change on disk before it has been fully read.
    /// Undefined behavior and/or a crash might result otherwise.
    /// @note Copying is enabled by default, but it can be disabled for individual files
    /// by setting the maximum size to zero bytes.  A default size limit can be specified
    /// by setting the environment variable @c OPENVDB_DELAYED_LOAD_COPY_MAX_BYTES
    /// to the desired number of bytes.
    void setCopyMaxBytes(Index64 bytes);

    /// Return @c true if a grid of the given name exists in this file.
    bool hasGrid(const Name&) const;

    /// Return (in a newly created MetaMap) the file-level metadata.
    MetaMap::Ptr getMetadata() const;

    /// Read the entire contents of the file and return a list of grid pointers.
    GridPtrVecPtr getGrids() const;

    /// @brief Read just the grid metadata and transforms from the file and return a list
    /// of pointers to grids that are empty except for their metadata and transforms.
    /// @throw IoError if this file is not open for reading.
    GridPtrVecPtr readAllGridMetadata();

    /// @brief Read a grid's metadata and transform only.
    /// @return A pointer to a grid that is empty except for its metadata and transform.
    /// @throw IoError if this file is not open for reading.
    /// @throw KeyError if no grid with the given name exists in this file.
    GridBase::Ptr readGridMetadata(const Name&);

    /// Read an entire grid, including all of its data blocks.
    GridBase::Ptr readGrid(const Name&);
    /// @brief Read a grid, including its data blocks, but only where it
    /// intersects the given world-space bounding box.
    GridBase::Ptr readGrid(const Name&, const BBoxd&);

    /// @todo GridPtrVec readAllGrids(const Name&)

    /// @brief Write the grids in the given container to the file whose name
    /// was given in the constructor.
    void write(const GridCPtrVec&, const MetaMap& = MetaMap()) const override;

    /// @brief Write the grids in the given container to the file whose name
    /// was given in the constructor.
    template<typename GridPtrContainerT>
    void write(const GridPtrContainerT&, const MetaMap& = MetaMap()) const;

    /// A const iterator that iterates over all names in the file. This is only
    /// valid once the file has been opened.
    class NameIterator
    {
    public:
        NameIterator(const NameMapCIter& iter): mIter(iter) {}
        NameIterator(const NameIterator&) = default;
        ~NameIterator() {}

        NameIterator& operator++() { mIter++; return *this; }

        bool operator==(const NameIterator& iter) const { return mIter == iter.mIter; }
        bool operator!=(const NameIterator& iter) const { return mIter != iter.mIter; }

        Name operator*() const { return this->gridName(); }

        Name gridName() const { return GridDescriptor::nameAsString(mIter->second.uniqueName()); }

    private:
        NameMapCIter mIter;
    };

    /// @return a NameIterator to iterate over all grid names in the file.
    NameIterator beginName() const;

    /// @return the ending iterator for all grid names in the file.
    NameIterator endName() const;

private:
    /// Read in all grid descriptors that are stored in the given stream.
    void readGridDescriptors(std::istream&);

    /// @brief Return an iterator to the descriptor for the grid with the given name.
    /// If the name is non-unique, return an iterator to the first matching descriptor.
    NameMapCIter findDescriptor(const Name&) const;

    /// Return a newly created, empty grid of the type specified by the given grid descriptor.
    GridBase::Ptr createGrid(const GridDescriptor&) const;

    /// @brief Read a grid, including its data blocks, but only where it
    /// intersects the given world-space bounding box.
    GridBase::Ptr readGridByName(const Name&, const BBoxd&);

    /// Read in and return the partially-populated grid specified by the given grid descriptor.
    GridBase::ConstPtr readGridPartial(const GridDescriptor&, bool readTopology) const;

    /// Read in and return the grid specified by the given grid descriptor.
    GridBase::Ptr readGrid(const GridDescriptor&) const;
    /// Read in and return the region of the grid specified by the given grid descriptor
    /// that intersects the given world-space bounding box.
    GridBase::Ptr readGrid(const GridDescriptor&, const BBoxd&) const;
    /// Read in and return the region of the grid specified by the given grid descriptor
    /// that intersects the given index-space bounding box.
    GridBase::Ptr readGrid(const GridDescriptor&, const CoordBBox&) const;

    /// @brief Partially populate the given grid by reading its metadata and transform and,
    /// if the grid is not an instance, its tree structure, but not the tree's leaf nodes.
    void readGridPartial(GridBase::Ptr, std::istream&, bool isInstance, bool readTopology) const;

    /// @brief Retrieve a grid from @c mNamedGrids.  Return a null pointer
    /// if @c mNamedGrids was not populated (because this file is random-access).
    /// @throw KeyError if no grid with the given name exists in this file.
    GridBase::Ptr retrieveCachedGrid(const Name&) const;

    void writeGrids(const GridCPtrVec&, const MetaMap&) const;

    MetaMap::Ptr fileMetadata();
    MetaMap::ConstPtr fileMetadata() const;

    const NameMap& gridDescriptors() const;
    NameMap& gridDescriptors();

    std::istream& inputStream() const;

    friend class ::TestFile;
    friend class ::TestStream;

    struct Impl;
    std::unique_ptr<Impl> mImpl;
};


////////////////////////////////////////


inline void
File::write(const GridCPtrVec& grids, const MetaMap& meta) const
{
    this->writeGrids(grids, meta);
}


template<typename GridPtrContainerT>
inline void
File::write(const GridPtrContainerT& container, const MetaMap& meta) const
{
    GridCPtrVec grids;
    std::copy(container.begin(), container.end(), std::back_inserter(grids));
    this->writeGrids(grids, meta);
}

} // namespace io
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb

#endif // OPENVDB_IO_FILE_HAS_BEEN_INCLUDED
