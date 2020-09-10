// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/Exceptions.h>
#include <openvdb/openvdb.h>
#include <openvdb/io/DelayedLoadMetadata.h>

class TestDelayedLoadMetadata : public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestDelayedLoadMetadata);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

    void test();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestDelayedLoadMetadata);

void
TestDelayedLoadMetadata::test()
{
    using namespace openvdb::io;

    // registration

    CPPUNIT_ASSERT(!DelayedLoadMetadata::isRegisteredType());

    DelayedLoadMetadata::registerType();

    CPPUNIT_ASSERT(DelayedLoadMetadata::isRegisteredType());

    DelayedLoadMetadata::unregisterType();

    CPPUNIT_ASSERT(!DelayedLoadMetadata::isRegisteredType());

    openvdb::initialize();

    CPPUNIT_ASSERT(DelayedLoadMetadata::isRegisteredType());

    // construction

    DelayedLoadMetadata metadata;

    CPPUNIT_ASSERT(metadata.empty());

    metadata.resizeMask(size_t(2));

    CPPUNIT_ASSERT(!metadata.empty());

    metadata.setMask(0, DelayedLoadMetadata::MaskType(5));
    metadata.setMask(1, DelayedLoadMetadata::MaskType(-3));

    CPPUNIT_ASSERT_EQUAL(metadata.getMask(0), DelayedLoadMetadata::MaskType(5));
    CPPUNIT_ASSERT_EQUAL(metadata.getMask(1), DelayedLoadMetadata::MaskType(-3));

    metadata.resizeCompressedSize(size_t(3));

    metadata.setCompressedSize(0, DelayedLoadMetadata::CompressedSizeType(6));
    metadata.setCompressedSize(1, DelayedLoadMetadata::CompressedSizeType(101));
    metadata.setCompressedSize(2, DelayedLoadMetadata::CompressedSizeType(-13522));

    CPPUNIT_ASSERT_EQUAL(metadata.getCompressedSize(0), DelayedLoadMetadata::CompressedSizeType(6));
    CPPUNIT_ASSERT_EQUAL(metadata.getCompressedSize(1), DelayedLoadMetadata::CompressedSizeType(101));
    CPPUNIT_ASSERT_EQUAL(metadata.getCompressedSize(2), DelayedLoadMetadata::CompressedSizeType(-13522));

    // copy construction

    DelayedLoadMetadata metadataCopy1(metadata);

    CPPUNIT_ASSERT(!metadataCopy1.empty());

    CPPUNIT_ASSERT_EQUAL(metadataCopy1.getMask(0), DelayedLoadMetadata::MaskType(5));
    CPPUNIT_ASSERT_EQUAL(metadataCopy1.getCompressedSize(2), DelayedLoadMetadata::CompressedSizeType(-13522));

    openvdb::Metadata::Ptr baseMetadataCopy2 = metadata.copy();
    DelayedLoadMetadata::Ptr metadataCopy2 =
        openvdb::StaticPtrCast<DelayedLoadMetadata>(baseMetadataCopy2);

    CPPUNIT_ASSERT_EQUAL(metadataCopy2->getMask(0), DelayedLoadMetadata::MaskType(5));
    CPPUNIT_ASSERT_EQUAL(metadataCopy2->getCompressedSize(2), DelayedLoadMetadata::CompressedSizeType(-13522));

    // I/O

    metadata.clear();
    CPPUNIT_ASSERT(metadata.empty());

    const size_t headerInitialSize(sizeof(openvdb::Index32));
    const size_t headerCountSize(sizeof(openvdb::Index32));
    const size_t headerMaskSize(sizeof(openvdb::Index32));
    const size_t headerCompressedSize(sizeof(openvdb::Index32));
    const size_t headerTotalSize(headerInitialSize + headerCountSize + headerMaskSize + headerCompressedSize);

    { // empty buffer
        std::stringstream ss(std::ios_base::out | std::ios_base::in | std::ios_base::binary);
        metadata.write(ss);
        CPPUNIT_ASSERT_EQUAL(ss.tellp(), std::streampos(headerInitialSize));

        DelayedLoadMetadata newMetadata;
        newMetadata.read(ss);
        CPPUNIT_ASSERT(newMetadata.empty());
    }

    { // single value, no compressed sizes
        metadata.clear();
        metadata.resizeMask(size_t(1));
        metadata.setMask(0, DelayedLoadMetadata::MaskType(5));

        std::stringstream ss(std::ios_base::out | std::ios_base::in | std::ios_base::binary);
        metadata.write(ss);
        std::streampos expectedPos(headerTotalSize + sizeof(int8_t));
        CPPUNIT_ASSERT_EQUAL(ss.tellp(), expectedPos);
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(expectedPos)-headerInitialSize, size_t(metadata.size()));

        DelayedLoadMetadata newMetadata;
        newMetadata.read(ss);
        CPPUNIT_ASSERT(!newMetadata.empty());
        CPPUNIT_ASSERT_EQUAL(newMetadata.getMask(0), DelayedLoadMetadata::MaskType(5));
    }

    { // single value, with compressed sizes
        metadata.clear();
        metadata.resizeMask(size_t(1));
        metadata.setMask(0, DelayedLoadMetadata::MaskType(5));

        metadata.resizeCompressedSize(size_t(1));
        metadata.setCompressedSize(0, DelayedLoadMetadata::CompressedSizeType(-10322));

        std::stringstream ss(std::ios_base::out | std::ios_base::in | std::ios_base::binary);
        metadata.write(ss);
        std::streampos expectedPos(headerTotalSize + sizeof(int8_t) + sizeof(int64_t));

        CPPUNIT_ASSERT_EQUAL(expectedPos, ss.tellp());
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(ss.tellp())-headerInitialSize, size_t(metadata.size()));

        DelayedLoadMetadata newMetadata;
        newMetadata.read(ss);
        CPPUNIT_ASSERT(!newMetadata.empty());
        CPPUNIT_ASSERT_EQUAL(newMetadata.getMask(0), DelayedLoadMetadata::MaskType(5));
        CPPUNIT_ASSERT_EQUAL(newMetadata.getCompressedSize(0), DelayedLoadMetadata::CompressedSizeType(-10322));
    }

    { // larger, but compressible buffer
        metadata.clear();

        const size_t size = 1000;

        const size_t uncompressedBufferSize = (sizeof(int8_t)+sizeof(int64_t))*size;

        metadata.resizeMask(size);
        metadata.resizeCompressedSize(size);
        for (size_t i = 0; i < size; i++) {
            metadata.setMask(i,
                DelayedLoadMetadata::MaskType(static_cast<int8_t>((i%32)*2)));
            metadata.setCompressedSize(i,
                DelayedLoadMetadata::CompressedSizeType(static_cast<int64_t>((i%64)*200)));
        }

        std::stringstream ss(std::ios_base::out | std::ios_base::in | std::ios_base::binary);
        metadata.write(ss);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(ss.tellp())-headerInitialSize, size_t(metadata.size()));

        std::streampos uncompressedSize(uncompressedBufferSize + headerTotalSize);
#ifdef OPENVDB_USE_BLOSC
        // expect a compression ratio of more than 10x
        CPPUNIT_ASSERT(ss.tellp() * 10 < uncompressedSize);
#else
        CPPUNIT_ASSERT(ss.tellp() == uncompressedSize);
#endif

        DelayedLoadMetadata newMetadata;
        newMetadata.read(ss);
        CPPUNIT_ASSERT_EQUAL(metadata.size(), newMetadata.size());
        for (size_t i = 0; i < size; i++) {
            CPPUNIT_ASSERT_EQUAL(metadata.getMask(i), newMetadata.getMask(i));
        }
    }

    // when read as unknown metadata should be treated as temporary metadata

#if OPENVDB_ABI_VERSION_NUMBER >= 5
    {
        metadata.clear();
        metadata.resizeMask(size_t(1));
        metadata.setMask(0, DelayedLoadMetadata::MaskType(5));

        std::stringstream ss(std::ios_base::out | std::ios_base::in | std::ios_base::binary);

        openvdb::MetaMap metamap;
        metamap.insertMeta("delayload", metadata);

        CPPUNIT_ASSERT_EQUAL(size_t(1), metamap.metaCount());

        metamap.writeMeta(ss);

        {
            openvdb::MetaMap newMetamap;
            newMetamap.readMeta(ss);

            CPPUNIT_ASSERT_EQUAL(size_t(1), newMetamap.metaCount());
        }

        {
            DelayedLoadMetadata::unregisterType();

            openvdb::MetaMap newMetamap;
            newMetamap.readMeta(ss);

            CPPUNIT_ASSERT_EQUAL(size_t(0), newMetamap.metaCount());
        }
    }
#endif
}
