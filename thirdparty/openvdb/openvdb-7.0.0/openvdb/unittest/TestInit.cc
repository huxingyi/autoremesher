// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/Exceptions.h>
#include <openvdb/openvdb.h>


class TestInit: public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestInit);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

    void test();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestInit);


void
TestInit::test()
{
    using namespace openvdb;

    initialize();

    // data types
    CPPUNIT_ASSERT(DoubleMetadata::isRegisteredType());
    CPPUNIT_ASSERT(FloatMetadata::isRegisteredType());
    CPPUNIT_ASSERT(Int32Metadata::isRegisteredType());
    CPPUNIT_ASSERT(Int64Metadata::isRegisteredType());
    CPPUNIT_ASSERT(StringMetadata::isRegisteredType());
    CPPUNIT_ASSERT(Vec2IMetadata::isRegisteredType());
    CPPUNIT_ASSERT(Vec2SMetadata::isRegisteredType());
    CPPUNIT_ASSERT(Vec2DMetadata::isRegisteredType());
    CPPUNIT_ASSERT(Vec3IMetadata::isRegisteredType());
    CPPUNIT_ASSERT(Vec3SMetadata::isRegisteredType());
    CPPUNIT_ASSERT(Vec3DMetadata::isRegisteredType());

    // map types
    CPPUNIT_ASSERT(math::AffineMap::isRegistered());
    CPPUNIT_ASSERT(math::UnitaryMap::isRegistered());
    CPPUNIT_ASSERT(math::ScaleMap::isRegistered());
    CPPUNIT_ASSERT(math::TranslationMap::isRegistered());
    CPPUNIT_ASSERT(math::ScaleTranslateMap::isRegistered());
    CPPUNIT_ASSERT(math::NonlinearFrustumMap::isRegistered());

    // grid types
    CPPUNIT_ASSERT(BoolGrid::isRegistered());
    CPPUNIT_ASSERT(FloatGrid::isRegistered());
    CPPUNIT_ASSERT(DoubleGrid::isRegistered());
    CPPUNIT_ASSERT(Int32Grid::isRegistered());
    CPPUNIT_ASSERT(Int64Grid::isRegistered());
    CPPUNIT_ASSERT(StringGrid::isRegistered());
    CPPUNIT_ASSERT(Vec3IGrid::isRegistered());
    CPPUNIT_ASSERT(Vec3SGrid::isRegistered());
    CPPUNIT_ASSERT(Vec3DGrid::isRegistered());

    uninitialize();

    CPPUNIT_ASSERT(!DoubleMetadata::isRegisteredType());
    CPPUNIT_ASSERT(!FloatMetadata::isRegisteredType());
    CPPUNIT_ASSERT(!Int32Metadata::isRegisteredType());
    CPPUNIT_ASSERT(!Int64Metadata::isRegisteredType());
    CPPUNIT_ASSERT(!StringMetadata::isRegisteredType());
    CPPUNIT_ASSERT(!Vec2IMetadata::isRegisteredType());
    CPPUNIT_ASSERT(!Vec2SMetadata::isRegisteredType());
    CPPUNIT_ASSERT(!Vec2DMetadata::isRegisteredType());
    CPPUNIT_ASSERT(!Vec3IMetadata::isRegisteredType());
    CPPUNIT_ASSERT(!Vec3SMetadata::isRegisteredType());
    CPPUNIT_ASSERT(!Vec3DMetadata::isRegisteredType());

    CPPUNIT_ASSERT(!math::AffineMap::isRegistered());
    CPPUNIT_ASSERT(!math::UnitaryMap::isRegistered());
    CPPUNIT_ASSERT(!math::ScaleMap::isRegistered());
    CPPUNIT_ASSERT(!math::TranslationMap::isRegistered());
    CPPUNIT_ASSERT(!math::ScaleTranslateMap::isRegistered());
    CPPUNIT_ASSERT(!math::NonlinearFrustumMap::isRegistered());

    CPPUNIT_ASSERT(!BoolGrid::isRegistered());
    CPPUNIT_ASSERT(!FloatGrid::isRegistered());
    CPPUNIT_ASSERT(!DoubleGrid::isRegistered());
    CPPUNIT_ASSERT(!Int32Grid::isRegistered());
    CPPUNIT_ASSERT(!Int64Grid::isRegistered());
    CPPUNIT_ASSERT(!StringGrid::isRegistered());
    CPPUNIT_ASSERT(!Vec3IGrid::isRegistered());
    CPPUNIT_ASSERT(!Vec3SGrid::isRegistered());
    CPPUNIT_ASSERT(!Vec3DGrid::isRegistered());
}
