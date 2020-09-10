// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#ifndef OPENVDB_DELAYED_LOAD_METADATA_HAS_BEEN_INCLUDED
#define OPENVDB_DELAYED_LOAD_METADATA_HAS_BEEN_INCLUDED

#include <openvdb/Metadata.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>


namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace io {

/// @brief Store a buffer of data that can be optionally used
/// during reading for faster delayed-load I/O performance
class OPENVDB_API DelayedLoadMetadata: public Metadata
{
public:
    using Ptr = SharedPtr<DelayedLoadMetadata>;
    using ConstPtr = SharedPtr<const DelayedLoadMetadata>;
    using MaskType = int8_t;
    using CompressedSizeType = int64_t;

    DelayedLoadMetadata() = default;
    DelayedLoadMetadata(const DelayedLoadMetadata& other);
    ~DelayedLoadMetadata() override = default;

    Name typeName() const override;
    Metadata::Ptr copy() const override;
    void copy(const Metadata&) override;
    std::string str() const override;
    bool asBool() const override;
    Index32 size() const override;

    static Name staticTypeName() { return "__delayedload"; }

    static Metadata::Ptr createMetadata()
    {
        Metadata::Ptr ret(new DelayedLoadMetadata);
        return ret;
    }

    static void registerType()
    {
        Metadata::registerType(DelayedLoadMetadata::staticTypeName(),
                               DelayedLoadMetadata::createMetadata);
    }

    static void unregisterType()
    {
        Metadata::unregisterType(DelayedLoadMetadata::staticTypeName());
    }

    static bool isRegisteredType()
    {
        return Metadata::isRegisteredType(DelayedLoadMetadata::staticTypeName());
    }

    /// @brief Delete the contents of the mask and compressed size arrays
    void clear();
    /// @brief Return @c true if both arrays are empty
    bool empty() const;

    /// @brief Resize the mask array
    void resizeMask(size_t size);
    /// @brief Resize the compressed size array
    void resizeCompressedSize(size_t size);

    /// @brief Return the mask value for a specific index
    /// @note throws if index is out-of-range or DelayedLoadMask not registered
    MaskType getMask(size_t index) const;
    /// @brief Set the mask value for a specific index
    /// @note throws if index is out-of-range
    void setMask(size_t index, const MaskType& value);

    /// @brief Return the compressed size value for a specific index
    /// @note throws if index is out-of-range or DelayedLoadMask not registered
    CompressedSizeType getCompressedSize(size_t index) const;
    /// @brief Set the compressed size value for a specific index
    /// @note throws if index is out-of-range
    void setCompressedSize(size_t index, const CompressedSizeType& value);

protected:
    void readValue(std::istream&, Index32 numBytes) override;
    void writeValue(std::ostream&) const override;

private:
    std::vector<MaskType> mMask;
    std::vector<CompressedSizeType> mCompressedSize;
}; // class DelayedLoadMetadata


} // namespace io
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb

#endif // OPENVDB_DELAYED_LOAD_METADATA_HAS_BEEN_INCLUDED
