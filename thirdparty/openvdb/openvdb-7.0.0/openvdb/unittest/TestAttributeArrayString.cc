// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/points/AttributeArrayString.h>
#include <openvdb/util/CpuTimer.h>

#include <openvdb/openvdb.h>

#include <iostream>

using namespace openvdb;
using namespace openvdb::points;

class TestAttributeArrayString: public CppUnit::TestCase
{
public:
    virtual void setUp() { openvdb::initialize(); }
    virtual void tearDown() { openvdb::uninitialize(); }

    CPPUNIT_TEST_SUITE(TestAttributeArrayString);
    CPPUNIT_TEST(testStringMetaInserter);
    CPPUNIT_TEST(testStringAttribute);
    CPPUNIT_TEST(testStringAttributeHandle);
    CPPUNIT_TEST(testStringAttributeWriteHandle);
    CPPUNIT_TEST(testProfile);

    CPPUNIT_TEST_SUITE_END();

    void testStringMetaInserter();
    void testStringAttribute();
    void testStringAttributeHandle();
    void testStringAttributeWriteHandle();
    void testProfile();

}; // class TestAttributeArrayString

CPPUNIT_TEST_SUITE_REGISTRATION(TestAttributeArrayString);


////////////////////////////////////////


namespace {

bool
matchingNamePairs(const openvdb::NamePair& lhs,
                  const openvdb::NamePair& rhs)
{
    if (lhs.first != rhs.first)     return false;
    if (lhs.second != rhs.second)     return false;

    return true;
}

} // namespace


////////////////////////////////////////


void
TestAttributeArrayString::testStringMetaInserter()
{
    MetaMap metadata;

    StringMetaInserter inserter(metadata);

    { // insert one value
        inserter.insert("test");
        CPPUNIT_ASSERT_EQUAL(metadata.metaCount(), size_t(1));
        StringMetadata::Ptr meta = metadata.getMetadata<StringMetadata>("string:0");
        CPPUNIT_ASSERT(meta);
        CPPUNIT_ASSERT_EQUAL(meta->value(), openvdb::Name("test"));
    }

    { // insert another value
        inserter.insert("test2");
        CPPUNIT_ASSERT_EQUAL(metadata.metaCount(), size_t(2));
        StringMetadata::Ptr meta = metadata.getMetadata<StringMetadata>("string:0");
        CPPUNIT_ASSERT(meta);
        CPPUNIT_ASSERT_EQUAL(meta->value(), openvdb::Name("test"));
        meta = metadata.getMetadata<StringMetadata>("string:1");
        CPPUNIT_ASSERT(meta);
        CPPUNIT_ASSERT_EQUAL(meta->value(), openvdb::Name("test2"));
    }

    // remove a value and reset the cache

    metadata.removeMeta("string:1");
    inserter.resetCache();

    { // re-insert value
        inserter.insert("test3");
        CPPUNIT_ASSERT_EQUAL(metadata.metaCount(), size_t(2));
        StringMetadata::Ptr meta = metadata.getMetadata<StringMetadata>("string:0");
        CPPUNIT_ASSERT(meta);
        CPPUNIT_ASSERT_EQUAL(meta->value(), openvdb::Name("test"));
        meta = metadata.getMetadata<StringMetadata>("string:1");
        CPPUNIT_ASSERT(meta);
        CPPUNIT_ASSERT_EQUAL(meta->value(), openvdb::Name("test3"));
    }

    { // insert and remove to create a gap
        inserter.insert("test4");
        CPPUNIT_ASSERT_EQUAL(metadata.metaCount(), size_t(3));
        metadata.removeMeta("string:1");
        inserter.resetCache();
        CPPUNIT_ASSERT_EQUAL(metadata.metaCount(), size_t(2));
        StringMetadata::Ptr meta = metadata.getMetadata<StringMetadata>("string:0");
        CPPUNIT_ASSERT(meta);
        CPPUNIT_ASSERT_EQUAL(meta->value(), openvdb::Name("test"));
        meta = metadata.getMetadata<StringMetadata>("string:2");
        CPPUNIT_ASSERT(meta);
        CPPUNIT_ASSERT_EQUAL(meta->value(), openvdb::Name("test4"));
    }

    { // insert to fill gap
        inserter.insert("test10");
        CPPUNIT_ASSERT_EQUAL(metadata.metaCount(), size_t(3));
        StringMetadata::Ptr meta = metadata.getMetadata<StringMetadata>("string:0");
        CPPUNIT_ASSERT(meta);
        CPPUNIT_ASSERT_EQUAL(meta->value(), openvdb::Name("test"));
        meta = metadata.getMetadata<StringMetadata>("string:1");
        CPPUNIT_ASSERT(meta);
        CPPUNIT_ASSERT_EQUAL(meta->value(), openvdb::Name("test10"));
        meta = metadata.getMetadata<StringMetadata>("string:2");
        CPPUNIT_ASSERT(meta);
        CPPUNIT_ASSERT_EQUAL(meta->value(), openvdb::Name("test4"));
    }

    { // insert existing value
        CPPUNIT_ASSERT_EQUAL(metadata.metaCount(), size_t(3));
        inserter.insert("test10");
        CPPUNIT_ASSERT_EQUAL(metadata.metaCount(), size_t(3));
    }

    metadata.removeMeta("string:0");
    metadata.removeMeta("string:2");
    inserter.resetCache();

    { // insert other value and string metadata
        metadata.insertMeta("int:1", Int32Metadata(5));
        metadata.insertMeta("irrelevant", StringMetadata("irrelevant"));
        inserter.resetCache();
        CPPUNIT_ASSERT_EQUAL(metadata.metaCount(), size_t(3));
        inserter.insert("test15");
        CPPUNIT_ASSERT_EQUAL(metadata.metaCount(), size_t(4));
        StringMetadata::Ptr meta = metadata.getMetadata<StringMetadata>("string:0");
        CPPUNIT_ASSERT(meta);
        CPPUNIT_ASSERT_EQUAL(meta->value(), openvdb::Name("test15"));
        meta = metadata.getMetadata<StringMetadata>("string:1");
        CPPUNIT_ASSERT(meta);
        CPPUNIT_ASSERT_EQUAL(meta->value(), openvdb::Name("test10"));
    }
}


////////////////////////////////////////


void
TestAttributeArrayString::testStringAttribute()
{
    { // Typed class API

        const Index count = 50;
        StringAttributeArray attr(count);

        CPPUNIT_ASSERT(!attr.isTransient());
        CPPUNIT_ASSERT(!attr.isHidden());
        CPPUNIT_ASSERT(isString(attr));

        attr.setTransient(true);
        CPPUNIT_ASSERT(attr.isTransient());
        CPPUNIT_ASSERT(!attr.isHidden());
        CPPUNIT_ASSERT(isString(attr));

        attr.setHidden(true);
        CPPUNIT_ASSERT(attr.isTransient());
        CPPUNIT_ASSERT(attr.isHidden());
        CPPUNIT_ASSERT(isString(attr));

        attr.setTransient(false);
        CPPUNIT_ASSERT(!attr.isTransient());
        CPPUNIT_ASSERT(attr.isHidden());
        CPPUNIT_ASSERT(isString(attr));

        StringAttributeArray attrB(attr);

        CPPUNIT_ASSERT(matchingNamePairs(attr.type(), attrB.type()));
        CPPUNIT_ASSERT_EQUAL(attr.size(), attrB.size());
        CPPUNIT_ASSERT_EQUAL(attr.memUsage(), attrB.memUsage());
        CPPUNIT_ASSERT_EQUAL(attr.isUniform(), attrB.isUniform());
        CPPUNIT_ASSERT_EQUAL(attr.isTransient(), attrB.isTransient());
        CPPUNIT_ASSERT_EQUAL(attr.isHidden(), attrB.isHidden());
        CPPUNIT_ASSERT_EQUAL(isString(attr), isString(attrB));

#if OPENVDB_ABI_VERSION_NUMBER >= 6
        AttributeArray& baseAttr(attr);
        CPPUNIT_ASSERT_EQUAL(Name(typeNameAsString<StringIndexType>()), baseAttr.valueType());
        CPPUNIT_ASSERT_EQUAL(Name("str"), baseAttr.codecType());
        CPPUNIT_ASSERT_EQUAL(Index(4), baseAttr.valueTypeSize());
        CPPUNIT_ASSERT_EQUAL(Index(4), baseAttr.storageTypeSize());
        CPPUNIT_ASSERT(!baseAttr.valueTypeIsFloatingPoint());
#endif
    }

    { // IO
        const Index count = 50;
        StringAttributeArray attrA(count);

        for (unsigned i = 0; i < unsigned(count); ++i) {
            attrA.set(i, int(i));
        }

        attrA.setHidden(true);

        std::ostringstream ostr(std::ios_base::binary);
        attrA.write(ostr);

        StringAttributeArray attrB;

        std::istringstream istr(ostr.str(), std::ios_base::binary);
        attrB.read(istr);

        CPPUNIT_ASSERT(matchingNamePairs(attrA.type(), attrB.type()));
        CPPUNIT_ASSERT_EQUAL(attrA.size(), attrB.size());
        CPPUNIT_ASSERT_EQUAL(attrA.memUsage(), attrB.memUsage());
        CPPUNIT_ASSERT_EQUAL(attrA.isUniform(), attrB.isUniform());
        CPPUNIT_ASSERT_EQUAL(attrA.isTransient(), attrB.isTransient());
        CPPUNIT_ASSERT_EQUAL(attrA.isHidden(), attrB.isHidden());
        CPPUNIT_ASSERT_EQUAL(isString(attrA), isString(attrB));

        for (unsigned i = 0; i < unsigned(count); ++i) {
            CPPUNIT_ASSERT_EQUAL(attrA.get(i), attrB.get(i));
        }
    }
}


void
TestAttributeArrayString::testStringAttributeHandle()
{
    MetaMap metadata;

    StringAttributeArray attr(4);
    StringAttributeHandle handle(attr, metadata);

    CPPUNIT_ASSERT_EQUAL(handle.size(), Index(4));
    CPPUNIT_ASSERT_EQUAL(handle.size(), attr.size());
    CPPUNIT_ASSERT_EQUAL(Index(1), handle.stride());
    CPPUNIT_ASSERT(handle.hasConstantStride());

    { // index 0 should always be an empty string
        Name value = handle.get(0);

        CPPUNIT_ASSERT_EQUAL(value, Name(""));
    }

    // set first element to 101

    CPPUNIT_ASSERT(handle.isUniform());

    attr.set(2, 102);

    CPPUNIT_ASSERT(!handle.isUniform());

    { // index 101 does not exist as metadata is empty
        CPPUNIT_ASSERT_EQUAL(handle.get(0), Name(""));
        CPPUNIT_ASSERT_THROW(handle.get(2), LookupError);
    }

    { // add an element to the metadata for 101
        metadata.insertMeta("string:101", StringMetadata("test101"));

        CPPUNIT_ASSERT_EQUAL(handle.get(0), Name(""));

        CPPUNIT_ASSERT_NO_THROW(handle.get(2));
        CPPUNIT_ASSERT_EQUAL(handle.get(2), Name("test101"));

        Name name;
        handle.get(name, 2);

        CPPUNIT_ASSERT_EQUAL(name, Name("test101"));
    }

    { // add a second element to the metadata
        metadata.insertMeta("string:102", StringMetadata("test102"));

        CPPUNIT_ASSERT_EQUAL(handle.get(0), Name(""));

        CPPUNIT_ASSERT_NO_THROW(handle.get(2));
        CPPUNIT_ASSERT_EQUAL(handle.get(2), Name("test101"));

        Name name;
        handle.get(name, 2);

        CPPUNIT_ASSERT_EQUAL(name, Name("test101"));
    }

    { // set two more values in the array
        attr.set(0, 103);
        attr.set(1, 103);

        CPPUNIT_ASSERT_EQUAL(handle.get(0), Name("test102"));
        CPPUNIT_ASSERT_EQUAL(handle.get(1), Name("test102"));
        CPPUNIT_ASSERT_EQUAL(handle.get(2), Name("test101"));
        CPPUNIT_ASSERT_EQUAL(handle.get(3), Name(""));
    }

    { // change a value
        attr.set(1, 102);

        CPPUNIT_ASSERT_EQUAL(handle.get(0), Name("test102"));
        CPPUNIT_ASSERT_EQUAL(handle.get(1), Name("test101"));
        CPPUNIT_ASSERT_EQUAL(handle.get(2), Name("test101"));
        CPPUNIT_ASSERT_EQUAL(handle.get(3), Name(""));
    }

    { // cannot use a StringAttributeHandle with a non-string attribute
        TypedAttributeArray<float> invalidAttr(50);
        CPPUNIT_ASSERT_THROW(StringAttributeHandle(invalidAttr, metadata), TypeError);
    }

    // Test stride and hasConstantStride methods for string handles

    {
        StringAttributeArray attr(3, 2, true);
        StringAttributeHandle handle(attr, metadata);

        CPPUNIT_ASSERT_EQUAL(Index(3), handle.size());
        CPPUNIT_ASSERT_EQUAL(handle.size(), attr.size());
        CPPUNIT_ASSERT_EQUAL(Index(2), handle.stride());
        CPPUNIT_ASSERT(handle.hasConstantStride());
    }

    {
        StringAttributeArray attr(4, 10, false);
        StringAttributeHandle handle(attr, metadata);

        CPPUNIT_ASSERT_EQUAL(Index(10), handle.size());
        CPPUNIT_ASSERT_EQUAL(Index(4), attr.size());
        CPPUNIT_ASSERT_EQUAL(Index(1), handle.stride());
        CPPUNIT_ASSERT(!handle.hasConstantStride());
    }
}


void
TestAttributeArrayString::testStringAttributeWriteHandle()
{
    MetaMap metadata;

    StringAttributeArray attr(4);
    StringAttributeWriteHandle handle(attr, metadata);

    { // add some values to metadata
        metadata.insertMeta("string:45", StringMetadata("testA"));
        metadata.insertMeta("string:90", StringMetadata("testB"));
        metadata.insertMeta("string:1000", StringMetadata("testC"));
    }

    { // no string values set
        CPPUNIT_ASSERT_EQUAL(handle.get(0), Name(""));
        CPPUNIT_ASSERT_EQUAL(handle.get(1), Name(""));
        CPPUNIT_ASSERT_EQUAL(handle.get(2), Name(""));
        CPPUNIT_ASSERT_EQUAL(handle.get(3), Name(""));
    }

    { // cache not reset since metadata changed
        CPPUNIT_ASSERT_THROW(handle.set(1, "testB"), LookupError);
    }

    { // empty string always has index 0
        CPPUNIT_ASSERT(handle.contains(""));
    }

    { // cache won't contain metadata until it has been reset
        CPPUNIT_ASSERT(!handle.contains("testA"));
        CPPUNIT_ASSERT(!handle.contains("testB"));
        CPPUNIT_ASSERT(!handle.contains("testC"));
    }

    handle.resetCache();

    { // empty string always has index 0 regardless of cache reset
        CPPUNIT_ASSERT(handle.contains(""));
    }

    { // cache now reset
        CPPUNIT_ASSERT(handle.contains("testA"));
        CPPUNIT_ASSERT(handle.contains("testB"));
        CPPUNIT_ASSERT(handle.contains("testC"));

        CPPUNIT_ASSERT_NO_THROW(handle.set(1, "testB"));

        CPPUNIT_ASSERT_EQUAL(handle.get(0), Name(""));
        CPPUNIT_ASSERT_EQUAL(handle.get(1), Name("testB"));
        CPPUNIT_ASSERT_EQUAL(handle.get(2), Name(""));
        CPPUNIT_ASSERT_EQUAL(handle.get(3), Name(""));
    }

    { // add another value
        handle.set(2, "testC");

        CPPUNIT_ASSERT_EQUAL(handle.get(0), Name(""));
        CPPUNIT_ASSERT_EQUAL(handle.get(1), Name("testB"));
        CPPUNIT_ASSERT_EQUAL(handle.get(2), Name("testC"));
        CPPUNIT_ASSERT_EQUAL(handle.get(3), Name(""));
    }

    handle.resetCache();

    { // compact tests
        CPPUNIT_ASSERT(!handle.compact());
        handle.set(0, "testA");
        handle.set(1, "testA");
        handle.set(2, "testA");
        handle.set(3, "testA");
        CPPUNIT_ASSERT(handle.compact());
        CPPUNIT_ASSERT(handle.isUniform());
    }

    { // expand tests
        CPPUNIT_ASSERT(handle.isUniform());
        handle.expand();
        CPPUNIT_ASSERT(!handle.isUniform());
        CPPUNIT_ASSERT_EQUAL(handle.get(0), Name("testA"));
        CPPUNIT_ASSERT_EQUAL(handle.get(1), Name("testA"));
        CPPUNIT_ASSERT_EQUAL(handle.get(2), Name("testA"));
        CPPUNIT_ASSERT_EQUAL(handle.get(3), Name("testA"));
    }

    { // fill tests
        CPPUNIT_ASSERT(!handle.isUniform());
        handle.set(3, "testB");
        handle.fill("testC");
        CPPUNIT_ASSERT(!handle.isUniform());
        CPPUNIT_ASSERT_EQUAL(handle.get(0), Name("testC"));
        CPPUNIT_ASSERT_EQUAL(handle.get(1), Name("testC"));
        CPPUNIT_ASSERT_EQUAL(handle.get(2), Name("testC"));
        CPPUNIT_ASSERT_EQUAL(handle.get(3), Name("testC"));
    }

    { // collapse tests
        handle.set(2, "testB");
        handle.collapse("testA");
        CPPUNIT_ASSERT(handle.isUniform());
        CPPUNIT_ASSERT_EQUAL(handle.get(0), Name("testA"));
        handle.expand();
        handle.set(2, "testB");
        CPPUNIT_ASSERT(!handle.isUniform());
        handle.collapse();
        CPPUNIT_ASSERT_EQUAL(handle.get(0), Name(""));
    }

    { // empty string tests
        handle.collapse("");
        CPPUNIT_ASSERT_EQUAL(handle.get(0), Name(""));
    }
}


void
TestAttributeArrayString::testProfile()
{

#ifdef PROFILE
    struct Timer : public openvdb::util::CpuTimer {};
    const size_t elements = 1000000;
#else
    struct Timer {
        void start(const std::string&) {}
        void stop() {}
    };
    const size_t elements = 10000;
#endif

    MetaMap metadata;
    StringMetaInserter inserter(metadata);

    Timer timer;
    timer.start("StringMetaInserter initialise");

    for (size_t i = 0; i < elements; ++i) {
        inserter.insert("test_string_" + std::to_string(i));
    }

    timer.stop();

    for (size_t i = 0; i < elements/2; ++i) {
        metadata.removeMeta("test_string_" + std::to_string(i*2));
    }

    timer.start("StringMetaInserter resetCache()");

    inserter.resetCache();

    timer.stop();
    timer.start("StringMetaInserter insert duplicates");

    for (size_t i = 0; i < elements; ++i) {
        inserter.insert("test_string_" + std::to_string(i));
    }

    timer.stop();

    openvdb::points::StringAttributeArray attr(elements);
    for (size_t i = 0; i < elements; ++i) {
        attr.set(Index(i), Index(i));
    }

    timer.start("StringAttributeWriteHandle construction");

    openvdb::points::StringAttributeWriteHandle handle(attr, metadata);

    timer.stop();
    timer.start("StringAttributeWriteHandle contains()");

    // half the calls will miss caches
    volatile bool result = false;
    for (size_t i = 0; i < elements/2; ++i) {
        result |= handle.contains("test_string_" + std::to_string(i*4));
    }

    timer.stop();
}
