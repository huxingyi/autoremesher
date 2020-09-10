// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <vector>
#include <cppunit/extensions/HelperMacros.h>

#include <openvdb/openvdb.h>
#include <openvdb/Exceptions.h>
#include <openvdb/tools/LevelSetUtil.h>
#include <openvdb/tools/MeshToVolume.h>     // for createLevelSetBox()
#include <openvdb/tools/Composite.h>        // for csgDifference()

class TestLevelSetUtil: public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestLevelSetUtil);
    CPPUNIT_TEST(testSDFToFogVolume);
    CPPUNIT_TEST(testSDFInteriorMask);
    CPPUNIT_TEST(testExtractEnclosedRegion);
    CPPUNIT_TEST(testSegmentationTools);
    CPPUNIT_TEST_SUITE_END();

    void testSDFToFogVolume();
    void testSDFInteriorMask();
    void testExtractEnclosedRegion();
    void testSegmentationTools();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestLevelSetUtil);


////////////////////////////////////////

void
TestLevelSetUtil::testSDFToFogVolume()
{
    openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create(10.0);

    grid->fill(openvdb::CoordBBox(openvdb::Coord(-100), openvdb::Coord(100)), 9.0);
    grid->fill(openvdb::CoordBBox(openvdb::Coord(-50), openvdb::Coord(50)), -9.0);


    openvdb::tools::sdfToFogVolume(*grid);

    CPPUNIT_ASSERT(grid->background() < 1e-7);

    openvdb::FloatGrid::ValueOnIter iter = grid->beginValueOn();
    for (; iter; ++iter) {
        CPPUNIT_ASSERT(iter.getValue() > 0.0);
        CPPUNIT_ASSERT(std::abs(iter.getValue() - 1.0) < 1e-7);
    }
}


void
TestLevelSetUtil::testSDFInteriorMask()
{
    typedef openvdb::FloatGrid          FloatGrid;
    typedef openvdb::BoolGrid           BoolGrid;
    typedef openvdb::Vec3s              Vec3s;
    typedef openvdb::math::BBox<Vec3s>  BBoxs;
    typedef openvdb::math::Transform    Transform;

    BBoxs bbox(Vec3s(0.0, 0.0, 0.0), Vec3s(1.0, 1.0, 1.0));

    Transform::Ptr transform = Transform::createLinearTransform(0.1);

    FloatGrid::Ptr sdfGrid = openvdb::tools::createLevelSetBox<FloatGrid>(bbox, *transform);

    BoolGrid::Ptr maskGrid = openvdb::tools::sdfInteriorMask(*sdfGrid);

    // test inside coord value
    openvdb::Coord ijk = transform->worldToIndexNodeCentered(openvdb::Vec3d(0.5, 0.5, 0.5));
    CPPUNIT_ASSERT(maskGrid->tree().getValue(ijk) == true);

    // test outside coord value
    ijk = transform->worldToIndexNodeCentered(openvdb::Vec3d(1.5, 1.5, 1.5));
    CPPUNIT_ASSERT(maskGrid->tree().getValue(ijk) == false);
}


void
TestLevelSetUtil::testExtractEnclosedRegion()
{
    typedef openvdb::FloatGrid          FloatGrid;
    typedef openvdb::BoolGrid           BoolGrid;
    typedef openvdb::Vec3s              Vec3s;
    typedef openvdb::math::BBox<Vec3s>  BBoxs;
    typedef openvdb::math::Transform    Transform;

    BBoxs regionA(Vec3s(0.0f, 0.0f, 0.0f), Vec3s(3.0f, 3.0f, 3.0f));
    BBoxs regionB(Vec3s(1.0f, 1.0f, 1.0f), Vec3s(2.0f, 2.0f, 2.0f));

    Transform::Ptr transform = Transform::createLinearTransform(0.1);

    FloatGrid::Ptr sdfGrid = openvdb::tools::createLevelSetBox<FloatGrid>(regionA, *transform);
    FloatGrid::Ptr sdfGridB = openvdb::tools::createLevelSetBox<FloatGrid>(regionB, *transform);

    openvdb::tools::csgDifference(*sdfGrid, *sdfGridB);

    BoolGrid::Ptr maskGrid = openvdb::tools::extractEnclosedRegion(*sdfGrid);

    // test inside ls region coord value
    openvdb::Coord ijk = transform->worldToIndexNodeCentered(openvdb::Vec3d(1.5, 1.5, 1.5));
    CPPUNIT_ASSERT(maskGrid->tree().getValue(ijk) == true);

    // test outside coord value
    ijk = transform->worldToIndexNodeCentered(openvdb::Vec3d(3.5, 3.5, 3.5));
    CPPUNIT_ASSERT(maskGrid->tree().getValue(ijk) == false);
}


void
TestLevelSetUtil::testSegmentationTools()
{
    typedef openvdb::FloatGrid          FloatGrid;
    typedef openvdb::Vec3s              Vec3s;
    typedef openvdb::math::BBox<Vec3s>  BBoxs;
    typedef openvdb::math::Transform    Transform;

    { // Test SDF segmentation

        // Create two sdf boxes with overlapping narrow-bands.
        BBoxs regionA(Vec3s(0.0f, 0.0f, 0.0f), Vec3s(2.0f, 2.0f, 2.0f));
        BBoxs regionB(Vec3s(2.5f, 0.0f, 0.0f), Vec3s(4.3f, 2.0f, 2.0f));

        Transform::Ptr transform = Transform::createLinearTransform(0.1);

        FloatGrid::Ptr sdfGrid = openvdb::tools::createLevelSetBox<FloatGrid>(regionA, *transform);
        FloatGrid::Ptr sdfGridB = openvdb::tools::createLevelSetBox<FloatGrid>(regionB, *transform);

        openvdb::tools::csgUnion(*sdfGrid, *sdfGridB);

        std::vector<FloatGrid::Ptr> segments;

        // This tool will not identify two separate segments when the narrow-bands overlap.
        openvdb::tools::segmentActiveVoxels(*sdfGrid, segments);
        CPPUNIT_ASSERT(segments.size() == 1);

        segments.clear();

        // This tool should properly identify two separate segments
        openvdb::tools::segmentSDF(*sdfGrid, segments);
        CPPUNIT_ASSERT(segments.size() == 2);


        // test inside ls region coord value
        openvdb::Coord ijk = transform->worldToIndexNodeCentered(openvdb::Vec3d(1.5, 1.5, 1.5));
        CPPUNIT_ASSERT(segments[0]->tree().getValue(ijk) < 0.0f);

        // test outside coord value
        ijk = transform->worldToIndexNodeCentered(openvdb::Vec3d(3.5, 3.5, 3.5));
        CPPUNIT_ASSERT(segments[0]->tree().getValue(ijk) > 0.0f);
    }

    { // Test empty SDF grid

        FloatGrid::Ptr sdfGrid = openvdb::FloatGrid::create(/*background=*/10.2f);
        sdfGrid->setGridClass(openvdb::GRID_LEVEL_SET);

        std::vector<FloatGrid::Ptr> segments;
        openvdb::tools::segmentSDF(*sdfGrid, segments);

        CPPUNIT_ASSERT_EQUAL(size_t(1), segments.size());
        CPPUNIT_ASSERT_EQUAL(openvdb::Index32(0), segments[0]->tree().leafCount());
        CPPUNIT_ASSERT_EQUAL(10.2f, segments[0]->background());
    }

    { // Test SDF grid with inactive leaf nodes

        BBoxs bbox(Vec3s(0.0, 0.0, 0.0), Vec3s(1.0, 1.0, 1.0));
        Transform::Ptr transform = Transform::createLinearTransform(0.1);
        FloatGrid::Ptr sdfGrid = openvdb::tools::createLevelSetBox<FloatGrid>(bbox, *transform,
            /*halfwidth=*/5);

        CPPUNIT_ASSERT(sdfGrid->tree().activeVoxelCount() > openvdb::Index64(0));

        // make all active voxels inactive

        for (auto leaf = sdfGrid->tree().beginLeaf(); leaf; ++leaf) {
            for (auto iter = leaf->beginValueOn(); iter; ++iter) {
                leaf->setValueOff(iter.getCoord());
            }
        }

        CPPUNIT_ASSERT_EQUAL(openvdb::Index64(0), sdfGrid->tree().activeVoxelCount());

        std::vector<FloatGrid::Ptr> segments;
        openvdb::tools::segmentSDF(*sdfGrid, segments);

        CPPUNIT_ASSERT_EQUAL(size_t(1), segments.size());
        CPPUNIT_ASSERT_EQUAL(openvdb::Index32(0), segments[0]->tree().leafCount());
        CPPUNIT_ASSERT_EQUAL(sdfGrid->background(), segments[0]->background());
    }

    { // Test fog volume with active tiles

        openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create(0.0);

        grid->fill(openvdb::CoordBBox(openvdb::Coord(0), openvdb::Coord(50)), 1.0);
        grid->fill(openvdb::CoordBBox(openvdb::Coord(60), openvdb::Coord(100)), 1.0);

        CPPUNIT_ASSERT(grid->tree().hasActiveTiles() == true);

        std::vector<FloatGrid::Ptr> segments;
        openvdb::tools::segmentActiveVoxels(*grid, segments);
        CPPUNIT_ASSERT_EQUAL(size_t(2), segments.size());
    }

    { // Test an empty fog volume

        openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create(/*background=*/3.1f);

        CPPUNIT_ASSERT_EQUAL(openvdb::Index32(0), grid->tree().leafCount());

        std::vector<FloatGrid::Ptr> segments;
        openvdb::tools::segmentActiveVoxels(*grid, segments);

        // note that an empty volume should segment into an empty volume
        CPPUNIT_ASSERT_EQUAL(size_t(1), segments.size());
        CPPUNIT_ASSERT_EQUAL(openvdb::Index32(0), segments[0]->tree().leafCount());
        CPPUNIT_ASSERT_EQUAL(3.1f, segments[0]->background());
    }

    { // Test fog volume with two inactive leaf nodes

        openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create(0.0);

        grid->tree().touchLeaf(openvdb::Coord(0,0,0));
        grid->tree().touchLeaf(openvdb::Coord(100,100,100));

        CPPUNIT_ASSERT_EQUAL(openvdb::Index32(2), grid->tree().leafCount());
        CPPUNIT_ASSERT_EQUAL(openvdb::Index64(0), grid->tree().activeVoxelCount());

        std::vector<FloatGrid::Ptr> segments;
        openvdb::tools::segmentActiveVoxels(*grid, segments);

        CPPUNIT_ASSERT_EQUAL(size_t(1), segments.size());
        CPPUNIT_ASSERT_EQUAL(openvdb::Index32(0), segments[0]->tree().leafCount());
    }
}

