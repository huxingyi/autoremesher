// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>

#include <openvdb/openvdb.h>
#include <openvdb/tree/Tree.h>
#include <openvdb/tree/LeafNode.h>
#include <openvdb/Types.h>
#include <openvdb/Exceptions.h>


class TestGridBbox: public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestGridBbox);
    CPPUNIT_TEST(testLeafBbox);
    CPPUNIT_TEST(testGridBbox);
    CPPUNIT_TEST_SUITE_END();

    void testLeafBbox();
    void testGridBbox();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestGridBbox);


////////////////////////////////////////


void
TestGridBbox::testLeafBbox()
{
    openvdb::FloatTree tree(/*fillValue=*/256.0f);

    openvdb::CoordBBox bbox;
    CPPUNIT_ASSERT(!tree.evalLeafBoundingBox(bbox));

    // Add values to buffer zero.
    tree.setValue(openvdb::Coord(  0,  9,   9), 2.0);
    tree.setValue(openvdb::Coord(100, 35, 800), 2.5);

    // Coordinates in CoordBBox are inclusive!
    CPPUNIT_ASSERT(tree.evalLeafBoundingBox(bbox));
    CPPUNIT_ASSERT_EQUAL(openvdb::Coord(0,        8,     8), bbox.min());
    CPPUNIT_ASSERT_EQUAL(openvdb::Coord(104-1, 40-1, 808-1), bbox.max());

    // Test negative coordinates.
    tree.setValue(openvdb::Coord(-100, -35, -800), 2.5);

    CPPUNIT_ASSERT(tree.evalLeafBoundingBox(bbox));
    CPPUNIT_ASSERT_EQUAL(openvdb::Coord(-104,   -40,  -800), bbox.min());
    CPPUNIT_ASSERT_EQUAL(openvdb::Coord(104-1, 40-1, 808-1), bbox.max());
}


void
TestGridBbox::testGridBbox()
{
    openvdb::FloatTree tree(/*fillValue=*/256.0f);

    openvdb::CoordBBox bbox;
    CPPUNIT_ASSERT(!tree.evalActiveVoxelBoundingBox(bbox));

    // Add values to buffer zero.
    tree.setValue(openvdb::Coord(  1,  0,   0), 1.5);
    tree.setValue(openvdb::Coord(  0, 12,   8), 2.0);
    tree.setValue(openvdb::Coord(  1, 35, 800), 2.5);
    tree.setValue(openvdb::Coord(100,  0,  16), 3.0);
    tree.setValue(openvdb::Coord(  1,  0,  16), 3.5);

    // Coordinates in CoordBBox are inclusive!
    CPPUNIT_ASSERT(tree.evalActiveVoxelBoundingBox(bbox));
    CPPUNIT_ASSERT_EQUAL(openvdb::Coord(  0,  0,   0), bbox.min());
    CPPUNIT_ASSERT_EQUAL(openvdb::Coord(100, 35, 800), bbox.max());

    // Test negative coordinates.
    tree.setValue(openvdb::Coord(-100, -35, -800), 2.5);

    CPPUNIT_ASSERT(tree.evalActiveVoxelBoundingBox(bbox));
    CPPUNIT_ASSERT_EQUAL(openvdb::Coord(-100,   -35,  -800), bbox.min());
    CPPUNIT_ASSERT_EQUAL(openvdb::Coord(100, 35, 800), bbox.max());
}
