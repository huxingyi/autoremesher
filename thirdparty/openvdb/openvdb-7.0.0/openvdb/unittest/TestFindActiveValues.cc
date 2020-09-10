///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) Ken Museth
//
// All rights reserved. This software is distributed under the
// Mozilla Public License 2.0 ( http://www.mozilla.org/MPL/2.0/ )
//
// Redistributions of source code must retain the above copyright
// and license notice and the following restrictions and disclaimer.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// IN NO EVENT SHALL THE COPYRIGHT HOLDERS' AND CONTRIBUTORS' AGGREGATE
// LIABILITY FOR ALL CLAIMS REGARDLESS OF THEIR BASIS EXCEED US$250.00.
//
///////////////////////////////////////////////////////////////////////////

#include <cstdio> // for remove()
#include <fstream>
#include <sstream>
#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/Exceptions.h>
#include <openvdb/Types.h>
#include <openvdb/openvdb.h>
#include <openvdb/util/CpuTimer.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/FindActiveValues.h>
#include "util.h" // for unittest_util::makeSphere()

#define ASSERT_DOUBLES_EXACTLY_EQUAL(expected, actual) \
    CPPUNIT_ASSERT_DOUBLES_EQUAL((expected), (actual), /*tolerance=*/0.0);


class TestFindActiveValues: public CppUnit::TestFixture
{
public:
    void setUp() override { openvdb::initialize(); }
    void tearDown() override { openvdb::uninitialize(); }

    CPPUNIT_TEST_SUITE(TestFindActiveValues);
    CPPUNIT_TEST(testBasic);
    CPPUNIT_TEST(testSphere1);
    CPPUNIT_TEST(testSphere2);
    CPPUNIT_TEST(testSparseBox);
    CPPUNIT_TEST(testDenseBox);
    CPPUNIT_TEST(testBenchmarks);
    CPPUNIT_TEST_SUITE_END();

    void testBasic();
    void testSphere1();
    void testSphere2();
    void testSparseBox();
    void testDenseBox();
    void testBenchmarks();
};


CPPUNIT_TEST_SUITE_REGISTRATION(TestFindActiveValues);

void
TestFindActiveValues::testBasic()
{
    const float background = 5.0f;
    openvdb::FloatTree tree(background);
    const openvdb::Coord min(-1,-2,30), max(20,30,55);
    const openvdb::CoordBBox bbox(min[0], min[1], min[2],
                                  max[0], max[1], max[2]);

    CPPUNIT_ASSERT(openvdb::tools::noActiveValues(tree, bbox));

    tree.setValue(min.offsetBy(-1), 1.0f);
    CPPUNIT_ASSERT(openvdb::tools::noActiveValues(tree, bbox));
    tree.setValue(max.offsetBy( 1), 1.0f);
    CPPUNIT_ASSERT(openvdb::tools::noActiveValues(tree, bbox));
    tree.setValue(min, 1.0f);
    CPPUNIT_ASSERT(openvdb::tools::anyActiveValues(tree, bbox));
    tree.setValue(max, 1.0f);
    CPPUNIT_ASSERT(openvdb::tools::anyActiveValues(tree, bbox));
}

void
TestFindActiveValues::testSphere1()
{
    const openvdb::Vec3f center(0.5f, 0.5f, 0.5f);
    const float radius = 0.3f;
    const int dim = 100, half_width = 3;
    const float voxel_size = 1.0f/dim;

    openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create(/*background=*/half_width*voxel_size);
    const openvdb::FloatTree& tree = grid->tree();
    grid->setTransform(openvdb::math::Transform::createLinearTransform(/*voxel size=*/voxel_size));
    unittest_util::makeSphere<openvdb::FloatGrid>(
        openvdb::Coord(dim), center, radius, *grid, unittest_util::SPHERE_SPARSE_NARROW_BAND);

    const int c = int(0.5f/voxel_size);
    const openvdb::CoordBBox a(openvdb::Coord(c), openvdb::Coord(c+ 8));
    CPPUNIT_ASSERT(!tree.isValueOn(openvdb::Coord(c)));
    CPPUNIT_ASSERT(!openvdb::tools::anyActiveValues(tree, a));

    const openvdb::Coord d(c + int(radius/voxel_size), c, c);
    CPPUNIT_ASSERT(tree.isValueOn(d));
    const auto b = openvdb::CoordBBox::createCube(d, 4);
    CPPUNIT_ASSERT(openvdb::tools::anyActiveValues(tree, b));

    const openvdb::CoordBBox e(openvdb::Coord(0), openvdb::Coord(dim));
    CPPUNIT_ASSERT(openvdb::tools::anyActiveValues(tree, e));
}

void
TestFindActiveValues::testSphere2()
{
    const openvdb::Vec3f center(0.0f);
    const float radius = 0.5f;
    const int dim = 400, halfWidth = 3;
    const float voxelSize = 2.0f/dim;
    auto grid  = openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(radius, center, voxelSize, halfWidth);
    openvdb::FloatTree& tree = grid->tree();

    {//test center
        const openvdb::CoordBBox bbox(openvdb::Coord(0), openvdb::Coord(8));
        CPPUNIT_ASSERT(!tree.isValueOn(openvdb::Coord(0)));
        //openvdb::util::CpuTimer timer("\ncenter");
        CPPUNIT_ASSERT(!openvdb::tools::anyActiveValues(tree, bbox));
        //timer.stop();
    }
    {//test on sphere
        const openvdb::Coord d(int(radius/voxelSize), 0, 0);
        CPPUNIT_ASSERT(tree.isValueOn(d));
        const auto bbox = openvdb::CoordBBox::createCube(d, 4);
        //openvdb::util::CpuTimer timer("\non sphere");
        CPPUNIT_ASSERT(openvdb::tools::anyActiveValues(tree, bbox));
        //timer.stop();
    }
    {//test full domain
        const openvdb::CoordBBox bbox(openvdb::Coord(-4000), openvdb::Coord(4000));
        //openvdb::util::CpuTimer timer("\nfull domain");
        CPPUNIT_ASSERT(openvdb::tools::anyActiveValues(tree, bbox));
        //timer.stop();
        openvdb::tools::FindActiveValues<openvdb::FloatTree> op(tree);
        CPPUNIT_ASSERT(op.count(bbox) == tree.activeVoxelCount());
    }
    {// find largest inscribed cube in index space containing NO active values
        openvdb::tools::FindActiveValues<openvdb::FloatTree> op(tree);
        auto bbox = openvdb::CoordBBox::createCube(openvdb::Coord(0), 1);
        //openvdb::util::CpuTimer timer("\nInscribed cube (class)");
        int count = 0;
        while(op.none(bbox)) {
            ++count;
            bbox.expand(1);
        }
        //const double t = timer.stop();
        //std::cerr << "Inscribed bbox = " << bbox << std::endl;
        const int n = int(openvdb::math::Sqrt(openvdb::math::Pow2(radius-halfWidth*voxelSize)/3.0f)/voxelSize) + 1;
        //std::cerr << "n=" << n << std::endl;
        CPPUNIT_ASSERT( bbox.max() == openvdb::Coord( n));
        CPPUNIT_ASSERT( bbox.min() == openvdb::Coord(-n));
        //openvdb::util::printTime(std::cerr, t/count, "time per lookup ", "\n", true, 4, 3);
    }
    {// find largest inscribed cube in index space containing NO active values
        auto bbox = openvdb::CoordBBox::createCube(openvdb::Coord(0), 1);
        //openvdb::util::CpuTimer timer("\nInscribed cube (func)");
        int count = 0;
        while(!openvdb::tools::anyActiveValues(tree, bbox)) {
            bbox.expand(1);
            ++count;
        }
        //const double t = timer.stop();
        //std::cerr << "Inscribed bbox = " << bbox << std::endl;
        const int n = int(openvdb::math::Sqrt(openvdb::math::Pow2(radius-halfWidth*voxelSize)/3.0f)/voxelSize) + 1;
        //std::cerr << "n=" << n << std::endl;
        //openvdb::util::printTime(std::cerr, t/count, "time per lookup ", "\n", true, 4, 3);
        CPPUNIT_ASSERT( bbox.max() == openvdb::Coord( n));
        CPPUNIT_ASSERT( bbox.min() == openvdb::Coord(-n));
    }
}

void
TestFindActiveValues::testSparseBox()
{
    {//test active tiles in a sparsely filled box
        const int half_dim = 256;
        const openvdb::CoordBBox bbox(openvdb::Coord(-half_dim), openvdb::Coord(half_dim));
        openvdb::FloatTree tree;
        CPPUNIT_ASSERT(tree.activeTileCount() == 0);
        CPPUNIT_ASSERT(tree.getValueDepth(openvdb::Coord(0)) == -1);//background value
        openvdb::tools::FindActiveValues<openvdb::FloatTree> op(tree);
        tree.sparseFill(bbox, 1.0f, true);
        op.update(tree);//tree was modified so op needs to be updated
        CPPUNIT_ASSERT(tree.activeTileCount() > 0);
        CPPUNIT_ASSERT(tree.getValueDepth(openvdb::Coord(0)) == 1);//upper internal tile value
        for (int i=1; i<half_dim; ++i) {
            CPPUNIT_ASSERT(op.any(openvdb::CoordBBox::createCube(openvdb::Coord(-half_dim), i)));
        }
        CPPUNIT_ASSERT(op.count(bbox) == bbox.volume());

        auto bbox2 = openvdb::CoordBBox::createCube(openvdb::Coord(-half_dim), 1);
        //double t = 0.0;
        //openvdb::util::CpuTimer timer;
        for (bool test = true; test; ) {
            //timer.restart();
            test = op.any(bbox2);
            //t = std::max(t, timer.restart());
            if (test) bbox2.translate(openvdb::Coord(1));
        }
        //std::cerr << "bbox = " << bbox2 << std::endl;
        //openvdb::util::printTime(std::cout, t, "The slowest sparse test ", "\n", true, 4, 3);
        CPPUNIT_ASSERT(bbox2 == openvdb::CoordBBox::createCube(openvdb::Coord(half_dim + 1), 1));
    }
}

void
TestFindActiveValues::testDenseBox()
{
     {//test active voxels in a densely filled box
      const int half_dim = 256;
      const openvdb::CoordBBox bbox(openvdb::Coord(-half_dim), openvdb::Coord(half_dim));
      openvdb::FloatTree tree;
      CPPUNIT_ASSERT(tree.activeTileCount() == 0);
      CPPUNIT_ASSERT(tree.getValueDepth(openvdb::Coord(0)) == -1);//background value
      tree.denseFill(bbox, 1.0f, true);
      CPPUNIT_ASSERT(tree.activeTileCount() == 0);
      openvdb::tools::FindActiveValues<openvdb::FloatTree> op(tree);
      CPPUNIT_ASSERT(tree.getValueDepth(openvdb::Coord(0)) == 3);// leaf value
      for (int i=1; i<half_dim; ++i) {
          CPPUNIT_ASSERT(op.any(openvdb::CoordBBox::createCube(openvdb::Coord(0), i)));
      }
      CPPUNIT_ASSERT(op.count(bbox) == bbox.volume());

      auto bbox2 = openvdb::CoordBBox::createCube(openvdb::Coord(-half_dim), 1);
      //double t = 0.0;
      //openvdb::util::CpuTimer timer;
      for (bool test = true; test; ) {
          //timer.restart();
          test = op.any(bbox2);
          //t = std::max(t, timer.restart());
          if (test) bbox2.translate(openvdb::Coord(1));
      }
      //std::cerr << "bbox = " << bbox2 << std::endl;
      //openvdb::util::printTime(std::cout, t, "The slowest dense test ", "\n", true, 4, 3);
      CPPUNIT_ASSERT(bbox2 == openvdb::CoordBBox::createCube(openvdb::Coord(half_dim + 1), 1));
    }
}

void
TestFindActiveValues::testBenchmarks()
{
    {//benchmark test against active tiles in a sparsely filled box
      using namespace openvdb;
      const int half_dim = 512, bbox_size = 6;
      const CoordBBox bbox(Coord(-half_dim), Coord(half_dim));
      FloatTree tree;
      tree.sparseFill(bbox, 1.0f, true);
      tools::FindActiveValues<FloatTree> op(tree);
      //double t = 0.0;
      //util::CpuTimer timer;
      for (auto b = CoordBBox::createCube(Coord(-half_dim), bbox_size); true; b.translate(Coord(1))) {
          //timer.restart();
          bool test = op.any(b);
          //t = std::max(t, timer.restart());
          if (!test) break;
      }
      //std::cout << "\n*The slowest sparse test " << t << " milliseconds\n";
      CPPUNIT_ASSERT(op.count(bbox) == bbox.volume());
    }
    {//benchmark test against active voxels in a densely filled box
      using namespace openvdb;
      const int half_dim = 256, bbox_size = 1;
      const CoordBBox bbox(Coord(-half_dim), Coord(half_dim));
      FloatTree tree;
      tree.denseFill(bbox, 1.0f, true);
      tools::FindActiveValues<FloatTree> op(tree);
      //double t = 0.0;
      //openvdb::util::CpuTimer timer;
      for (auto b = CoordBBox::createCube(Coord(-half_dim), bbox_size); true; b.translate(Coord(1))) {
          //timer.restart();
          bool test = op.any(b);
          //t = std::max(t, timer.restart());
          if (!test) break;
      }
      //std::cout << "*The slowest dense test " << t << " milliseconds\n";
      CPPUNIT_ASSERT(op.count(bbox) == bbox.volume());
    }
    {//benchmark test against active voxels in a densely filled box
      using namespace openvdb;
      FloatTree tree;
      tree.denseFill(CoordBBox::createCube(Coord(0), 256), 1.0f, true);
      tools::FindActiveValues<FloatTree> op(tree);
      //openvdb::util::CpuTimer timer("new test");
      CPPUNIT_ASSERT(op.none(CoordBBox::createCube(Coord(256), 1)));
      //timer.stop();
    }
}

// Copyright (c) Ken Museth
// All rights reserved. This software is distributed under the
// Mozilla Public License 2.0 ( http://www.mozilla.org/MPL/2.0/ )
