// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/Exceptions.h>
#include <openvdb/tree/LeafNode.h>
#include <openvdb/Types.h>
#include <cctype> // for toupper()
#include <iostream>
#include <sstream>

// CPPUNIT_TEST_SUITE() invokes CPPUNIT_TESTNAMER_DECL() to generate a suite name
// from the FixtureType.  But if FixtureType is a templated type, the generated name
// can become long and messy.  This macro overrides the normal naming logic,
// instead invoking FixtureType::testSuiteName(), which should be a static member
// function that returns a std::string containing the suite name for the specific
// template instantiation.
#undef CPPUNIT_TESTNAMER_DECL
#define CPPUNIT_TESTNAMER_DECL( variableName, FixtureType ) \
    CPPUNIT_NS::TestNamer variableName( FixtureType::testSuiteName() )


template<typename T>
class TestLeafIO: public CppUnit::TestCase
{
public:
    static std::string testSuiteName()
    {
        std::string name = openvdb::typeNameAsString<T>();
        if (!name.empty()) name[0] = static_cast<char>(::toupper(name[0]));
        return "TestLeafIO" + name;
    }

    CPPUNIT_TEST_SUITE(TestLeafIO);
    CPPUNIT_TEST(testBuffer);
    CPPUNIT_TEST_SUITE_END();

    void testBuffer();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestLeafIO<int>);
CPPUNIT_TEST_SUITE_REGISTRATION(TestLeafIO<float>);
CPPUNIT_TEST_SUITE_REGISTRATION(TestLeafIO<double>);
CPPUNIT_TEST_SUITE_REGISTRATION(TestLeafIO<bool>);
CPPUNIT_TEST_SUITE_REGISTRATION(TestLeafIO<openvdb::Byte>);
CPPUNIT_TEST_SUITE_REGISTRATION(TestLeafIO<openvdb::Vec3R>);
CPPUNIT_TEST_SUITE_REGISTRATION(TestLeafIO<std::string>);


template<typename T>
void
TestLeafIO<T>::testBuffer()
{
    openvdb::tree::LeafNode<T, 3> leaf(openvdb::Coord(0, 0, 0));

    leaf.setValueOn(openvdb::Coord(0, 1, 0), T(1));
    leaf.setValueOn(openvdb::Coord(1, 0, 0), T(1));

    std::ostringstream ostr(std::ios_base::binary);

    leaf.writeBuffers(ostr);

    leaf.setValueOn(openvdb::Coord(0, 1, 0), T(0));
    leaf.setValueOn(openvdb::Coord(0, 1, 1), T(1));

    std::istringstream istr(ostr.str(), std::ios_base::binary);

    // Since the input stream doesn't include a VDB header with file format version info,
    // tag the input stream explicitly with the current version number.
    openvdb::io::setCurrentVersion(istr);

    leaf.readBuffers(istr);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(T(1), leaf.getValue(openvdb::Coord(0, 1, 0)), /*tolerance=*/0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(T(1), leaf.getValue(openvdb::Coord(1, 0, 0)), /*tolerance=*/0);

    CPPUNIT_ASSERT(leaf.onVoxelCount() == 2);
}


template<>
void
TestLeafIO<std::string>::testBuffer()
{
    openvdb::tree::LeafNode<std::string, 3>
        leaf(openvdb::Coord(0, 0, 0), std::string());

    leaf.setValueOn(openvdb::Coord(0, 1, 0), std::string("test"));
    leaf.setValueOn(openvdb::Coord(1, 0, 0), std::string("test"));

    std::ostringstream ostr(std::ios_base::binary);

    leaf.writeBuffers(ostr);

    leaf.setValueOn(openvdb::Coord(0, 1, 0), std::string("douche"));
    leaf.setValueOn(openvdb::Coord(0, 1, 1), std::string("douche"));

    std::istringstream istr(ostr.str(), std::ios_base::binary);

    // Since the input stream doesn't include a VDB header with file format version info,
    // tag the input stream explicitly with the current version number.
    openvdb::io::setCurrentVersion(istr);

    leaf.readBuffers(istr);

    CPPUNIT_ASSERT_EQUAL(std::string("test"), leaf.getValue(openvdb::Coord(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(std::string("test"), leaf.getValue(openvdb::Coord(1, 0, 0)));

    CPPUNIT_ASSERT(leaf.onVoxelCount() == 2);
}


template<>
void
TestLeafIO<openvdb::Vec3R>::testBuffer()
{
    openvdb::tree::LeafNode<openvdb::Vec3R, 3> leaf(openvdb::Coord(0, 0, 0));

    leaf.setValueOn(openvdb::Coord(0, 1, 0), openvdb::Vec3R(1, 1, 1));
    leaf.setValueOn(openvdb::Coord(1, 0, 0), openvdb::Vec3R(1, 1, 1));

    std::ostringstream ostr(std::ios_base::binary);

    leaf.writeBuffers(ostr);

    leaf.setValueOn(openvdb::Coord(0, 1, 0), openvdb::Vec3R(0, 0, 0));
    leaf.setValueOn(openvdb::Coord(0, 1, 1), openvdb::Vec3R(1, 1, 1));

    std::istringstream istr(ostr.str(), std::ios_base::binary);

    // Since the input stream doesn't include a VDB header with file format version info,
    // tag the input stream explicitly with the current version number.
    openvdb::io::setCurrentVersion(istr);

    leaf.readBuffers(istr);

    CPPUNIT_ASSERT(leaf.getValue(openvdb::Coord(0, 1, 0)) == openvdb::Vec3R(1, 1, 1));
    CPPUNIT_ASSERT(leaf.getValue(openvdb::Coord(1, 0, 0)) == openvdb::Vec3R(1, 1, 1));

    CPPUNIT_ASSERT(leaf.onVoxelCount() == 2);
}
