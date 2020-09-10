// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/tools/TopologyToLevelSet.h>


class TopologyToLevelSet: public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TopologyToLevelSet);
    CPPUNIT_TEST(testConversion);
    CPPUNIT_TEST_SUITE_END();

    void testConversion();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TopologyToLevelSet);

void
TopologyToLevelSet::testConversion()
{
    typedef openvdb::tree::Tree4<bool, 5, 4, 3>::Type   Tree543b;
    typedef openvdb::Grid<Tree543b>                     BoolGrid;

    typedef openvdb::tree::Tree4<float, 5, 4, 3>::Type  Tree543f;
    typedef openvdb::Grid<Tree543f>                     FloatGrid;

    /////

    const float voxelSize = 0.1f;
    const openvdb::math::Transform::Ptr transform =
            openvdb::math::Transform::createLinearTransform(voxelSize);

    BoolGrid maskGrid(false);
    maskGrid.setTransform(transform);

    // Define active region
    maskGrid.fill(openvdb::CoordBBox(openvdb::Coord(0), openvdb::Coord(7)), true);
    maskGrid.tree().voxelizeActiveTiles();

    FloatGrid::Ptr sdfGrid = openvdb::tools::topologyToLevelSet(maskGrid);

    CPPUNIT_ASSERT(sdfGrid.get() != NULL);
    CPPUNIT_ASSERT(!sdfGrid->empty());
    CPPUNIT_ASSERT_EQUAL(int(openvdb::GRID_LEVEL_SET), int(sdfGrid->getGridClass()));

    // test inside coord value
    CPPUNIT_ASSERT(sdfGrid->tree().getValue(openvdb::Coord(3,3,3)) < 0.0f);

    // test outside coord value
    CPPUNIT_ASSERT(sdfGrid->tree().getValue(openvdb::Coord(10,10,10)) > 0.0f);
}

