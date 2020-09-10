// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/Exceptions.h>
#include <openvdb/openvdb.h>
#include <openvdb/math/BBox.h>
#include <openvdb/Types.h>
#include <openvdb/math/Transform.h>

typedef float Real;

class TestBBox: public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestBBox);
    CPPUNIT_TEST(testBBox);
    CPPUNIT_TEST(testCenter);
    CPPUNIT_TEST(testExtent);
    CPPUNIT_TEST_SUITE_END();

    void testBBox();
    void testCenter();
    void testExtent();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestBBox);


void
TestBBox::testBBox()
{
    typedef openvdb::Vec3R                     Vec3R;
    typedef openvdb::math::BBox<Vec3R>         BBoxType;

    {
        BBoxType B(Vec3R(1,1,1),Vec3R(2,2,2));

        CPPUNIT_ASSERT(B.isSorted());
        CPPUNIT_ASSERT(B.isInside(Vec3R(1.5,2,2)));
        CPPUNIT_ASSERT(!B.isInside(Vec3R(2,3,2)));
        B.expand(Vec3R(3,3,3));
        CPPUNIT_ASSERT(B.isInside(Vec3R(3,3,3)));
    }

    {
        BBoxType B;
        CPPUNIT_ASSERT(B.empty());
        const Vec3R expected(1);
        B.expand(expected);
        CPPUNIT_ASSERT_EQUAL(expected, B.min());
        CPPUNIT_ASSERT_EQUAL(expected, B.max());
    }
}


void
TestBBox::testCenter()
{
    using namespace openvdb::math;

    const Vec3<double> expected(1.5);

    BBox<openvdb::Vec3R> fbox(openvdb::Vec3R(1.0), openvdb::Vec3R(2.0));
    CPPUNIT_ASSERT_EQUAL(expected, fbox.getCenter());

    BBox<openvdb::Vec3i> ibox(openvdb::Vec3i(1), openvdb::Vec3i(2));
    CPPUNIT_ASSERT_EQUAL(expected, ibox.getCenter());

    openvdb::CoordBBox cbox(openvdb::Coord(1), openvdb::Coord(2));
    CPPUNIT_ASSERT_EQUAL(expected, cbox.getCenter());
}

void
TestBBox::testExtent()
{
    typedef openvdb::Vec3R                     Vec3R;
    typedef openvdb::math::BBox<Vec3R>         BBoxType;

    {
        BBoxType B(Vec3R(-20,0,1),Vec3R(2,2,2));
        CPPUNIT_ASSERT_EQUAL(size_t(2), B.minExtent());
        CPPUNIT_ASSERT_EQUAL(size_t(0), B.maxExtent());
    }
    {
        BBoxType B(Vec3R(1,0,1),Vec3R(2,21,20));
        CPPUNIT_ASSERT_EQUAL(size_t(0), B.minExtent());
        CPPUNIT_ASSERT_EQUAL(size_t(1), B.maxExtent());
    }
    {
        BBoxType B(Vec3R(1,0,1),Vec3R(3,1.5,20));
        CPPUNIT_ASSERT_EQUAL(size_t(1), B.minExtent());
        CPPUNIT_ASSERT_EQUAL(size_t(2), B.maxExtent());
    }
}
