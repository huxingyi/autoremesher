// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include "DelayedLoadMetadata.h"

#include <openvdb/points/StreamCompression.h>

#ifdef OPENVDB_USE_BLOSC
#include <blosc.h>

namespace {

inline size_t padMask(size_t bytes)
{
    return size_t(std::ceil(static_cast<float>(bytes+1) /
        sizeof(openvdb::io::DelayedLoadMetadata::MaskType)));
}

inline size_t padCompressedSize(size_t bytes)
{
    return size_t(std::ceil(static_cast<float>(bytes+1) /
        sizeof(openvdb::io::DelayedLoadMetadata::CompressedSizeType)));
}

} // namespace

#endif

namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace io {

DelayedLoadMetadata::DelayedLoadMetadata(const DelayedLoadMetadata& other)
    : Metadata()
    , mMask(other.mMask)
    , mCompressedSize(other.mCompressedSize)
{
}

Name DelayedLoadMetadata::typeName() const
{
    return DelayedLoadMetadata::staticTypeName();
}

Metadata::Ptr DelayedLoadMetadata::copy() const
{
    Metadata::Ptr metadata(new DelayedLoadMetadata());
    metadata->copy(*this);
    return metadata;
}

void DelayedLoadMetadata::copy(const Metadata& other)
{
    const DelayedLoadMetadata* t = dynamic_cast<const DelayedLoadMetadata*>(&other);
    if (t == nullptr) OPENVDB_THROW(TypeError, "Incompatible type during copy");
    mMask = t->mMask;
    mCompressedSize = t->mCompressedSize;
}

std::string DelayedLoadMetadata::str() const
{
    return "";
}

bool DelayedLoadMetadata::asBool() const
{
    return false;
}

Index32 DelayedLoadMetadata::size() const
{
    if (mMask.empty() && mCompressedSize.empty())   return Index32(0);

    // count
    size_t size = sizeof(Index32);

    { // mask
        size += sizeof(Index32);
        size_t compressedSize = compression::bloscCompressedSize(
            reinterpret_cast<const char*>(mMask.data()), mMask.size()*sizeof(MaskType));

        if (compressedSize > 0)     size += compressedSize;
        else                        size += mMask.size()*sizeof(MaskType);
    }
    { // compressed size
        size += sizeof(Index32);
        if (!mCompressedSize.empty()) {
            size_t compressedSize = compression::bloscCompressedSize(
                reinterpret_cast<const char*>(mCompressedSize.data()), mCompressedSize.size()*sizeof(CompressedSizeType));

            if (compressedSize > 0)     size += compressedSize;
            else                        size += mCompressedSize.size()*sizeof(CompressedSizeType);
        }
    }

    return static_cast<Index32>(size);
}

void DelayedLoadMetadata::clear()
{
    mMask.clear();
    mCompressedSize.clear();
}

bool DelayedLoadMetadata::empty() const
{
    return mMask.empty() && mCompressedSize.empty();
}

void DelayedLoadMetadata::resizeMask(size_t size)
{
    mMask.resize(size);
}

void DelayedLoadMetadata::resizeCompressedSize(size_t size)
{
    mCompressedSize.resize(size);
}

DelayedLoadMetadata::MaskType DelayedLoadMetadata::getMask(size_t index) const
{
    assert(DelayedLoadMetadata::isRegisteredType());
    assert(index < mMask.size());
    return mMask[index];
}

void DelayedLoadMetadata::setMask(size_t index, const MaskType& value)
{
    assert(index < mMask.size());
    mMask[index] = value;
}

DelayedLoadMetadata::CompressedSizeType DelayedLoadMetadata::getCompressedSize(size_t index) const
{
    assert(DelayedLoadMetadata::isRegisteredType());
    assert(index < mCompressedSize.size());
    return mCompressedSize[index];
}

void DelayedLoadMetadata::setCompressedSize(size_t index, const CompressedSizeType& value)
{
    assert(index < mCompressedSize.size());
    mCompressedSize[index] = value;
}

void DelayedLoadMetadata::readValue(std::istream& is, Index32 numBytes)
{
    if (numBytes == 0)  return;

    // initial header size
    size_t total = sizeof(Index32);

    Index32 count = 0;
    is.read(reinterpret_cast<char*>(&count), sizeof(Index32));
    total += sizeof(Index32);

    Index32 bytes = 0;
    is.read(reinterpret_cast<char*>(&bytes), sizeof(Index32));
    total += sizeof(Index32);

    if (bytes > Index32(0)) {
        std::unique_ptr<char[]> compressedBuffer(new char[bytes]);
        is.read(reinterpret_cast<char*>(compressedBuffer.get()), bytes);

        total += bytes;

#ifdef OPENVDB_USE_BLOSC
        // pad to include BLOSC_MAX_OVERHEAD
        size_t uncompressedBytes = openvdb::compression::bloscUncompressedSize(compressedBuffer.get());
        const size_t paddedCount = padMask(uncompressedBytes + BLOSC_MAX_OVERHEAD);

        mMask.reserve(paddedCount);
        mMask.resize(count);

        // resize should never modify capacity for smaller vector sizes
        assert(mMask.capacity() >= paddedCount);

        compression::bloscDecompress(reinterpret_cast<char*>(mMask.data()), count*sizeof(MaskType), mMask.capacity()*sizeof(MaskType), compressedBuffer.get());
#endif
    } else {
        mMask.resize(count);
        is.read(reinterpret_cast<char*>(mMask.data()), count*sizeof(MaskType));
        total += count*sizeof(MaskType);
    }

    is.read(reinterpret_cast<char*>(&bytes), sizeof(Index32));

    if (bytes != std::numeric_limits<Index32>::max()) {
        if (bytes > Index32(0)) {
            std::unique_ptr<char[]> compressedBuffer(new char[bytes]);
            is.read(reinterpret_cast<char*>(compressedBuffer.get()), bytes);

            total += size_t(bytes);

#ifdef OPENVDB_USE_BLOSC
            // pad to include BLOSC_MAX_OVERHEAD
            size_t uncompressedBytes = openvdb::compression::bloscUncompressedSize(compressedBuffer.get());
            const size_t paddedCount = padCompressedSize(uncompressedBytes + BLOSC_MAX_OVERHEAD);

            mCompressedSize.reserve(paddedCount);
            mCompressedSize.resize(count);

            // resize should never modify capacity for smaller vector sizes
            assert(mCompressedSize.capacity() >= paddedCount);

            compression::bloscDecompress(reinterpret_cast<char*>(mCompressedSize.data()), count*sizeof(CompressedSizeType), mCompressedSize.capacity()*sizeof(CompressedSizeType), compressedBuffer.get());
#endif
        } else {
            mCompressedSize.resize(count);
            is.read(reinterpret_cast<char*>(mCompressedSize.data()), count*sizeof(CompressedSizeType));
            total += count*sizeof(CompressedSizeType);
        }
    }

    Index32 totalBytes = static_cast<Index32>(total);

    if (totalBytes < numBytes) {
        // Read and discard any unknown bytes at the end of the metadata for forwards-compatibility
        // (without seeking, because the stream might not be seekable).
        const size_t BUFFER_SIZE = 1024;
        std::vector<char> buffer(BUFFER_SIZE);
        for (Index32 bytesRemaining = numBytes - totalBytes; bytesRemaining > 0; ) {
            const Index32 bytesToSkip = std::min<Index32>(bytesRemaining, BUFFER_SIZE);
            is.read(&buffer[0], bytesToSkip);
            bytesRemaining -= bytesToSkip;
        }
    }
}

void DelayedLoadMetadata::writeValue(std::ostream& os) const
{
    // metadata has a limit of 2^32 bytes
    assert(mMask.size() < std::numeric_limits<Index32>::max());
    assert(mCompressedSize.size() < std::numeric_limits<Index32>::max());

    if (mMask.empty() && mCompressedSize.empty())     return;

    assert(mCompressedSize.empty() || (mMask.size() == mCompressedSize.size()));

    Index32 count = static_cast<Index32>(mMask.size());
    os.write(reinterpret_cast<const char*>(&count), sizeof(Index32));

    const Index32 zeroSize(0);
    const Index32 maxSize(std::numeric_limits<Index32>::max());

    { // mask buffer
        size_t compressedBytes(0);
        std::unique_ptr<char[]> compressedBuffer;
        if (compression::bloscCanCompress()) {
            compressedBuffer = compression::bloscCompress(
                reinterpret_cast<const char*>(mMask.data()),
                mMask.size()*sizeof(MaskType), compressedBytes, /*resize=*/false);
        }

        if (compressedBuffer) {
            assert(compressedBytes < std::numeric_limits<Index32>::max());
            Index32 bytes(static_cast<Index32>(compressedBytes));
            os.write(reinterpret_cast<const char*>(&bytes), sizeof(Index32));
            os.write(reinterpret_cast<const char*>(compressedBuffer.get()), compressedBytes);
        }
        else {
            os.write(reinterpret_cast<const char*>(&zeroSize), sizeof(Index32));
            os.write(reinterpret_cast<const char*>(mMask.data()),
                mMask.size()*sizeof(MaskType));
        }
    }

    // compressed size buffer

    if (mCompressedSize.empty()) {
        // write out maximum Index32 value to denote no compressed sizes stored
        os.write(reinterpret_cast<const char*>(&maxSize), sizeof(Index32));
    } else {
        size_t compressedBytes(0);
        std::unique_ptr<char[]> compressedBuffer;
        if (compression::bloscCanCompress()) {
            compressedBuffer = compression::bloscCompress(
                reinterpret_cast<const char*>(mCompressedSize.data()),
                mCompressedSize.size()*sizeof(CompressedSizeType), compressedBytes, /*resize=*/false);
        }

        if (compressedBuffer) {
            assert(compressedBytes < std::numeric_limits<Index32>::max());
            Index32 bytes(static_cast<Index32>(compressedBytes));
            os.write(reinterpret_cast<const char*>(&bytes), sizeof(Index32));
            os.write(reinterpret_cast<const char*>(compressedBuffer.get()), compressedBytes);
        }
        else {
            os.write(reinterpret_cast<const char*>(&zeroSize), sizeof(Index32));
            os.write(reinterpret_cast<const char*>(mCompressedSize.data()),
                mCompressedSize.size()*sizeof(CompressedSizeType));
        }
    }
}

} // namespace io
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb
