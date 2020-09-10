// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/Types.h>
#include <openvdb/openvdb.h>
#include <openvdb/tools/GridOperators.h>

#define ASSERT_DOUBLES_EXACTLY_EQUAL(expected, actual) \
    CPPUNIT_ASSERT_DOUBLES_EQUAL((expected), (actual), /*tolerance=*/1e-6);

namespace {
const int GRID_DIM = 10;
}


class TestCurl: public CppUnit::TestFixture
{
public:
    virtual void setUp() { openvdb::initialize(); }
    virtual void tearDown() { openvdb::uninitialize(); }

    CPPUNIT_TEST_SUITE(TestCurl);
    CPPUNIT_TEST(testISCurl);                    // Gradient in Index Space
    CPPUNIT_TEST(testISCurlStencil);
    CPPUNIT_TEST(testWSCurl);                    // Gradient in World Space
    CPPUNIT_TEST(testWSCurlStencil);
    CPPUNIT_TEST(testCurlTool);                  // Gradient tool
    CPPUNIT_TEST(testCurlMaskedTool);            // Gradient tool

    CPPUNIT_TEST_SUITE_END();

    void testISCurl();
    void testISCurlStencil();
    void testWSCurl();
    void testWSCurlStencil();
    void testCurlTool();
    void testCurlMaskedTool();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestCurl);


void
TestCurl::testCurlTool()
{
    using namespace openvdb;

    VectorGrid::Ptr inGrid = VectorGrid::create();
    const VectorTree& inTree = inGrid->tree();
    CPPUNIT_ASSERT(inTree.empty());

    VectorGrid::Accessor inAccessor = inGrid->getAccessor();
    int dim = GRID_DIM;
    for (int x = -dim; x<dim; ++x) {
        for (int y = -dim; y<dim; ++y) {
            for (int z = -dim; z<dim; ++z) {
                inAccessor.setValue(Coord(x,y,z),
                    VectorTree::ValueType(float(y), float(-x), 0.f));
            }
        }
    }
    CPPUNIT_ASSERT(!inTree.empty());
    CPPUNIT_ASSERT_EQUAL(math::Pow3(2*dim), int(inTree.activeVoxelCount()));

    VectorGrid::Ptr curl_grid = tools::curl(*inGrid);
    CPPUNIT_ASSERT_EQUAL(math::Pow3(2*dim), int(curl_grid->activeVoxelCount()));

    VectorGrid::ConstAccessor curlAccessor = curl_grid->getConstAccessor();
    --dim;//ignore boundary curl vectors
    for (int x = -dim; x<dim; ++x) {
        for (int y = -dim; y<dim; ++y) {
            for (int z = -dim; z<dim; ++z) {
                Coord xyz(x,y,z);
                VectorTree::ValueType v = inAccessor.getValue(xyz);
                //std::cout << "vec(" << xyz << ")=" << v << std::endl;
                ASSERT_DOUBLES_EXACTLY_EQUAL( y,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-x,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[2]);
                v = curlAccessor.getValue(xyz);
                //std::cout << "curl(" << xyz << ")=" << v << std::endl;
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);
            }
        }
    }
}


void
TestCurl::testCurlMaskedTool()
{
    using namespace openvdb;

    VectorGrid::Ptr inGrid = VectorGrid::create();
    const VectorTree& inTree = inGrid->tree();
    CPPUNIT_ASSERT(inTree.empty());

    VectorGrid::Accessor inAccessor = inGrid->getAccessor();
    int dim = GRID_DIM;
    for (int x = -dim; x<dim; ++x) {
        for (int y = -dim; y<dim; ++y) {
            for (int z = -dim; z<dim; ++z) {
                inAccessor.setValue(Coord(x,y,z),
                    VectorTree::ValueType(float(y), float(-x), 0.f));
            }
        }
    }
    CPPUNIT_ASSERT(!inTree.empty());
    CPPUNIT_ASSERT_EQUAL(math::Pow3(2*dim), int(inTree.activeVoxelCount()));

    openvdb::CoordBBox maskBBox(openvdb::Coord(0), openvdb::Coord(dim));
    BoolGrid::Ptr maskGrid = BoolGrid::create(false);
    maskGrid->fill(maskBBox, true /*value*/, true /*activate*/);

    openvdb::CoordBBox testBBox(openvdb::Coord(-dim+1), openvdb::Coord(dim));
    BoolGrid::Ptr testGrid = BoolGrid::create(false);
    testGrid->fill(testBBox, true, true);

    testGrid->topologyIntersection(*maskGrid);


    VectorGrid::Ptr curl_grid = tools::curl(*inGrid, *maskGrid);
    CPPUNIT_ASSERT_EQUAL(math::Pow3(dim), int(curl_grid->activeVoxelCount()));

    VectorGrid::ConstAccessor curlAccessor = curl_grid->getConstAccessor();
    --dim;//ignore boundary curl vectors
    for (int x = -dim; x<dim; ++x) {
        for (int y = -dim; y<dim; ++y) {
            for (int z = -dim; z<dim; ++z) {
                Coord xyz(x,y,z);
                VectorTree::ValueType v = inAccessor.getValue(xyz);

                ASSERT_DOUBLES_EXACTLY_EQUAL( y,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-x,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[2]);

                v = curlAccessor.getValue(xyz);
                if (maskBBox.isInside(xyz)) {
                    ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                    ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                    ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);
                } else {
                    // get the background value outside masked region
                    ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                    ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                    ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[2]);
                }
            }
        }
    }
}


void
TestCurl::testISCurl()
{
    using namespace openvdb;

    VectorGrid::Ptr inGrid = VectorGrid::create();
    const VectorTree& inTree = inGrid->tree();
    CPPUNIT_ASSERT(inTree.empty());

    VectorGrid::Accessor inAccessor = inGrid->getAccessor();
    int dim = GRID_DIM;
    for (int x = -dim; x<dim; ++x) {
        for (int y = -dim; y<dim; ++y) {
            for (int z = -dim; z<dim; ++z) {
                inAccessor.setValue(Coord(x,y,z),
                    VectorTree::ValueType(float(y), float(-x), 0.f));
            }
        }
    }
    CPPUNIT_ASSERT(!inTree.empty());
    CPPUNIT_ASSERT_EQUAL(math::Pow3(2*dim), int(inTree.activeVoxelCount()));

    VectorGrid::Ptr curl_grid = tools::curl(*inGrid);
    CPPUNIT_ASSERT_EQUAL(math::Pow3(2*dim), int(curl_grid->activeVoxelCount()));

    --dim;//ignore boundary curl vectors
    // test unit space operators
    VectorGrid::ConstAccessor inConstAccessor = inGrid->getConstAccessor();
    for (int x = -dim; x<dim; ++x) {
        for (int y = -dim; y<dim; ++y) {
            for (int z = -dim; z<dim; ++z) {
                Coord xyz(x,y,z);
                VectorTree::ValueType v = inAccessor.getValue(xyz);
                ASSERT_DOUBLES_EXACTLY_EQUAL( y,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-x,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[2]);

                v = math::ISCurl<math::CD_2ND>::result(inConstAccessor, xyz);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);

                v = math::ISCurl<math::FD_1ST>::result(inConstAccessor, xyz);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);

                v = math::ISCurl<math::BD_1ST>::result(inConstAccessor, xyz);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);
            }
        }
    }

    --dim;//ignore boundary curl vectors
    for (int x = -dim; x<dim; ++x) {
        for (int y = -dim; y<dim; ++y) {
            for (int z = -dim; z<dim; ++z) {
                Coord xyz(x,y,z);
                VectorTree::ValueType v = inAccessor.getValue(xyz);

                ASSERT_DOUBLES_EXACTLY_EQUAL( y,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-x,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[2]);
                v = math::ISCurl<math::CD_4TH>::result(inConstAccessor, xyz);

                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);

                v = math::ISCurl<math::FD_2ND>::result(inConstAccessor, xyz);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);

                v = math::ISCurl<math::BD_2ND>::result(inConstAccessor, xyz);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);
            }
        }
    }

    --dim;//ignore boundary curl vectors
    for (int x = -dim; x<dim; ++x) {
        for (int y = -dim; y<dim; ++y) {
            for (int z = -dim; z<dim; ++z) {
                Coord xyz(x,y,z);
                VectorTree::ValueType v = inAccessor.getValue(xyz);

                ASSERT_DOUBLES_EXACTLY_EQUAL( y, v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-x, v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0, v[2]);
                v = math::ISCurl<math::CD_6TH>::result(inConstAccessor, xyz);

                ASSERT_DOUBLES_EXACTLY_EQUAL( 0, v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0, v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2, v[2]);

                v = math::ISCurl<math::FD_3RD>::result(inConstAccessor, xyz);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0, v[0]);
                CPPUNIT_ASSERT_DOUBLES_EQUAL( 0, v[1], /*tolerance=*/0.00001);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(-2, v[2], /*tolerance=*/0.00001);

                v = math::ISCurl<math::BD_3RD>::result(inConstAccessor, xyz);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0, v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0, v[1]);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(-2, v[2], /*tolerance=*/0.00001);
            }
        }
    }
}


void
TestCurl::testISCurlStencil()
{
    using namespace openvdb;

    VectorGrid::Ptr inGrid = VectorGrid::create();
    const VectorTree& inTree = inGrid->tree();
    CPPUNIT_ASSERT(inTree.empty());

    VectorGrid::Accessor inAccessor = inGrid->getAccessor();
    int dim = GRID_DIM;
    for (int x = -dim; x<dim; ++x) {
        for (int y = -dim; y<dim; ++y) {
            for (int z = -dim; z<dim; ++z) {
                inAccessor.setValue(Coord(x,y,z),
                    VectorTree::ValueType(float(y), float(-x), 0.f));
            }
        }
    }
    CPPUNIT_ASSERT(!inTree.empty());
    CPPUNIT_ASSERT_EQUAL(math::Pow3(2*dim), int(inTree.activeVoxelCount()));

    VectorGrid::Ptr curl_grid = tools::curl(*inGrid);
    CPPUNIT_ASSERT_EQUAL(math::Pow3(2*dim), int(curl_grid->activeVoxelCount()));

    math::SevenPointStencil<VectorGrid> sevenpt(*inGrid);
    math::ThirteenPointStencil<VectorGrid> thirteenpt(*inGrid);
    math::NineteenPointStencil<VectorGrid> nineteenpt(*inGrid);

    // test unit space operators

    --dim;//ignore boundary curl vectors
    for (int x = -dim; x<dim; ++x) {
        for (int y = -dim; y<dim; ++y) {
            for (int z = -dim; z<dim; ++z) {
                Coord xyz(x,y,z);
                sevenpt.moveTo(xyz);

                VectorTree::ValueType v = inAccessor.getValue(xyz);

                ASSERT_DOUBLES_EXACTLY_EQUAL( y,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-x,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[2]);

                v = math::ISCurl<math::CD_2ND>::result(sevenpt);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);

                v = math::ISCurl<math::FD_1ST>::result(sevenpt);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);

                v = math::ISCurl<math::BD_1ST>::result(sevenpt);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);
            }
        }
    }

     --dim;//ignore boundary curl vectors
    for (int x = -dim; x<dim; ++x) {
        for (int y = -dim; y<dim; ++y) {
            for (int z = -dim; z<dim; ++z) {
                Coord xyz(x,y,z);
                thirteenpt.moveTo(xyz);

                VectorTree::ValueType v = inAccessor.getValue(xyz);

                ASSERT_DOUBLES_EXACTLY_EQUAL( y,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-x,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[2]);
                v = math::ISCurl<math::CD_4TH>::result(thirteenpt);

                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);


                v = math::ISCurl<math::FD_2ND>::result(thirteenpt);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);

                v = math::ISCurl<math::BD_2ND>::result(thirteenpt);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);
            }
        }
    }


    --dim;//ignore boundary curl vectors
    for (int x = -dim; x<dim; ++x) {
        for (int y = -dim; y<dim; ++y) {
            for (int z = -dim; z<dim; ++z) {
                Coord xyz(x,y,z);
                nineteenpt.moveTo(xyz);

                VectorTree::ValueType v = inAccessor.getValue(xyz);

                ASSERT_DOUBLES_EXACTLY_EQUAL( y,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-x,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[2]);
                v = math::ISCurl<math::CD_6TH>::result(nineteenpt);

                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);

                v = math::ISCurl<math::FD_3RD>::result(nineteenpt);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(-2,v[2], /*tolerance=*/0.00001);

                v = math::ISCurl<math::BD_3RD>::result(nineteenpt);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(-2,v[2], /*tolerance=*/0.00001);
            }
        }
    }
}

void
TestCurl::testWSCurl()
{
    using namespace openvdb;

    VectorGrid::Ptr inGrid = VectorGrid::create();
    const VectorTree& inTree = inGrid->tree();
    CPPUNIT_ASSERT(inTree.empty());

    VectorGrid::Accessor inAccessor = inGrid->getAccessor();
    int dim = GRID_DIM;
    for (int x = -dim; x<dim; ++x) {
        for (int y = -dim; y<dim; ++y) {
            for (int z = -dim; z<dim; ++z) {
                inAccessor.setValue(Coord(x,y,z),
                    VectorTree::ValueType(float(y), float(-x), 0.f));
            }
        }
    }
    CPPUNIT_ASSERT(!inTree.empty());
    CPPUNIT_ASSERT_EQUAL(math::Pow3(2*dim), int(inTree.activeVoxelCount()));

    VectorGrid::Ptr curl_grid = tools::curl(*inGrid);
    CPPUNIT_ASSERT_EQUAL(math::Pow3(2*dim), int(curl_grid->activeVoxelCount()));

    // test with a map
    math::AffineMap map;
    math::UniformScaleMap uniform_map;


    // test unit space operators

    --dim;//ignore boundary curl vectors
    for (int x = -dim; x<dim; ++x) {
        for (int y = -dim; y<dim; ++y) {
            for (int z = -dim; z<dim; ++z) {
                Coord xyz(x,y,z);

                VectorTree::ValueType v = inAccessor.getValue(xyz);

                ASSERT_DOUBLES_EXACTLY_EQUAL( y,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-x,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[2]);

                v = math::Curl<math::AffineMap, math::CD_2ND>::result(map, inAccessor, xyz);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);

                v = math::Curl<math::AffineMap, math::FD_1ST>::result(map, inAccessor, xyz);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);

                v = math::Curl<math::AffineMap, math::BD_1ST>::result(map, inAccessor, xyz);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);

                v = math::Curl<math::UniformScaleMap, math::CD_2ND>::result(
                    uniform_map, inAccessor, xyz);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);

                v = math::Curl<math::UniformScaleMap, math::FD_1ST>::result(
                    uniform_map, inAccessor, xyz);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);

                v = math::Curl<math::UniformScaleMap, math::BD_1ST>::result(
                    uniform_map, inAccessor, xyz);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);
            }
        }
    }
}


void
TestCurl::testWSCurlStencil()
{
    using namespace openvdb;

    VectorGrid::Ptr inGrid = VectorGrid::create();
    const VectorTree& inTree = inGrid->tree();
    CPPUNIT_ASSERT(inTree.empty());

    VectorGrid::Accessor inAccessor = inGrid->getAccessor();
    int dim = GRID_DIM;
    for (int x = -dim; x<dim; ++x) {
        for (int y = -dim; y<dim; ++y) {
            for (int z = -dim; z<dim; ++z) {
                inAccessor.setValue(Coord(x,y,z),
                    VectorTree::ValueType(float(y), float(-x), 0.f));
            }
        }
    }
    CPPUNIT_ASSERT(!inTree.empty());
    CPPUNIT_ASSERT_EQUAL(math::Pow3(2*dim), int(inTree.activeVoxelCount()));

    VectorGrid::Ptr curl_grid = tools::curl(*inGrid);
    CPPUNIT_ASSERT_EQUAL(math::Pow3(2*dim), int(curl_grid->activeVoxelCount()));

    // test with a map
    math::AffineMap map;
    math::UniformScaleMap uniform_map;

    math::SevenPointStencil<VectorGrid> sevenpt(*inGrid);
    math::SecondOrderDenseStencil<VectorGrid> dense_2ndOrder(*inGrid);


    // test unit space operators

    --dim;//ignore boundary curl vectors
    for (int x = -dim; x<dim; ++x) {
        for (int y = -dim; y<dim; ++y) {
            for (int z = -dim; z<dim; ++z) {
                Coord xyz(x,y,z);
                sevenpt.moveTo(xyz);
                dense_2ndOrder.moveTo(xyz);

                VectorTree::ValueType v = inAccessor.getValue(xyz);

                ASSERT_DOUBLES_EXACTLY_EQUAL( y,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-x,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[2]);

                v = math::Curl<math::AffineMap, math::CD_2ND>::result(map, dense_2ndOrder);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);

                v = math::Curl<math::AffineMap, math::FD_1ST>::result(map, dense_2ndOrder);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);

                v = math::Curl<math::AffineMap, math::BD_1ST>::result(map, dense_2ndOrder);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);

                v = math::Curl<math::UniformScaleMap, math::CD_2ND>::result(uniform_map, sevenpt);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);

                v = math::Curl<math::UniformScaleMap, math::FD_1ST>::result(uniform_map, sevenpt);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);

                v = math::Curl<math::UniformScaleMap, math::BD_1ST>::result(uniform_map, sevenpt);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[0]);
                ASSERT_DOUBLES_EXACTLY_EQUAL( 0,v[1]);
                ASSERT_DOUBLES_EXACTLY_EQUAL(-2,v[2]);
            }
        }
    }
}
