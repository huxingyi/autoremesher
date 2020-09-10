// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <openvdb/Exceptions.h>
#include <openvdb/Types.h>
#include <openvdb/tree/NodeManager.h>
#include <openvdb/tree/LeafManager.h>
#include "util.h" // for unittest_util::makeSphere()
#include <cppunit/extensions/HelperMacros.h>


class TestNodeManager: public CppUnit::TestFixture
{
public:
    void setUp() override { openvdb::initialize(); }
    void tearDown() override { openvdb::uninitialize(); }

    CPPUNIT_TEST_SUITE(TestNodeManager);
    CPPUNIT_TEST(testAll);
    CPPUNIT_TEST_SUITE_END();

    void testAll();
};


CPPUNIT_TEST_SUITE_REGISTRATION(TestNodeManager);


namespace {

template<typename TreeT>
struct NodeCountOp {
    NodeCountOp() : nodeCount(TreeT::DEPTH, 0), totalCount(0)
    {
    }
    NodeCountOp(const NodeCountOp&, tbb::split)
        : nodeCount(TreeT::DEPTH, 0), totalCount(0)
    {
    }
    void join(const NodeCountOp& other)
    {
        for (size_t i = 0; i < nodeCount.size(); ++i) {
            nodeCount[i] += other.nodeCount[i];
        }
        totalCount += other.totalCount;
    }
    // do nothing for the root node
    void operator()(const typename TreeT::RootNodeType&)
    {
    }
    // count the internal and leaf nodes
    template<typename NodeT>
    void operator()(const NodeT&)
    {
        ++(nodeCount[NodeT::LEVEL]);
        ++totalCount;
    }
    std::vector<openvdb::Index64> nodeCount;
    openvdb::Index64 totalCount;
};// NodeCountOp

}//unnamed namespace

void
TestNodeManager::testAll()
{
    using openvdb::CoordBBox;
    using openvdb::Coord;
    using openvdb::Vec3f;
    using openvdb::Index64;
    using openvdb::FloatGrid;
    using openvdb::FloatTree;

    const Vec3f center(0.35f, 0.35f, 0.35f);
    const float radius = 0.15f;
    const int dim = 128, half_width = 5;
    const float voxel_size = 1.0f/dim;

    FloatGrid::Ptr grid = FloatGrid::create(/*background=*/half_width*voxel_size);
    FloatTree& tree = grid->tree();
    grid->setTransform(openvdb::math::Transform::createLinearTransform(/*voxel size=*/voxel_size));

    unittest_util::makeSphere<FloatGrid>(Coord(dim), center,
                                         radius, *grid, unittest_util::SPHERE_SPARSE_NARROW_BAND);

    CPPUNIT_ASSERT_EQUAL(4, int(FloatTree::DEPTH));
    CPPUNIT_ASSERT_EQUAL(3, int(openvdb::tree::NodeManager<FloatTree>::LEVELS));

    std::vector<Index64> nodeCount;
    for (openvdb::Index i=0; i<FloatTree::DEPTH; ++i) nodeCount.push_back(0);
    for (FloatTree::NodeCIter it = tree.cbeginNode(); it; ++it) ++(nodeCount[it.getLevel()]);

    //for (size_t i=0; i<nodeCount.size(); ++i) {//includes the root node
    //    std::cerr << "Level=" << i << " nodes=" << nodeCount[i] << std::endl;
    //}

    {// test tree constructor
        openvdb::tree::NodeManager<FloatTree> manager(tree);

        //for (openvdb::Index i=0; i<openvdb::tree::NodeManager<FloatTree>::LEVELS; ++i) {
        //    std::cerr << "Level=" << i << " nodes=" << manager.nodeCount(i) << std::endl;
        //}

        Index64 totalCount = 0;
        for (openvdb::Index i=0; i<FloatTree::RootNodeType::LEVEL; ++i) {//exclude root in nodeCount
            //std::cerr << "Level=" << i << " expected=" << nodeCount[i]
            //          << " cached=" << manager.nodeCount(i) << std::endl;
            CPPUNIT_ASSERT_EQUAL(nodeCount[i], manager.nodeCount(i));
            totalCount += nodeCount[i];
        }
        CPPUNIT_ASSERT_EQUAL(totalCount, manager.nodeCount());

        // test the map reduce functionality
        NodeCountOp<FloatTree> bottomUpOp;
        NodeCountOp<FloatTree> topDownOp;
        manager.reduceBottomUp(bottomUpOp);
        manager.reduceTopDown(topDownOp);
        for (openvdb::Index i=0; i<FloatTree::RootNodeType::LEVEL; ++i) {//exclude root in nodeCount
            CPPUNIT_ASSERT_EQUAL(bottomUpOp.nodeCount[i], manager.nodeCount(i));
            CPPUNIT_ASSERT_EQUAL(topDownOp.nodeCount[i], manager.nodeCount(i));
        }
        CPPUNIT_ASSERT_EQUAL(bottomUpOp.totalCount, manager.nodeCount());
        CPPUNIT_ASSERT_EQUAL(topDownOp.totalCount, manager.nodeCount());
    }

    {// test LeafManager constructor
        typedef openvdb::tree::LeafManager<FloatTree> LeafManagerT;
        LeafManagerT manager1(tree);
        CPPUNIT_ASSERT_EQUAL(nodeCount[0], Index64(manager1.leafCount()));
        openvdb::tree::NodeManager<LeafManagerT> manager2(manager1);
        Index64 totalCount = 0;
        for (openvdb::Index i=0; i<FloatTree::RootNodeType::LEVEL; ++i) {//exclude root in nodeCount
            //std::cerr << "Level=" << i << " expected=" << nodeCount[i]
            //          << " cached=" << manager2.nodeCount(i) << std::endl;
            CPPUNIT_ASSERT_EQUAL(nodeCount[i], Index64(manager2.nodeCount(i)));
            totalCount += nodeCount[i];
        }
        CPPUNIT_ASSERT_EQUAL(totalCount, Index64(manager2.nodeCount()));

        // test the map reduce functionality
        NodeCountOp<FloatTree> bottomUpOp;
        NodeCountOp<FloatTree> topDownOp;
        manager2.reduceBottomUp(bottomUpOp);
        manager2.reduceTopDown(topDownOp);
        for (openvdb::Index i=0; i<FloatTree::RootNodeType::LEVEL; ++i) {//exclude root in nodeCount
            CPPUNIT_ASSERT_EQUAL(bottomUpOp.nodeCount[i], manager2.nodeCount(i));
            CPPUNIT_ASSERT_EQUAL(topDownOp.nodeCount[i], manager2.nodeCount(i));
        }
        CPPUNIT_ASSERT_EQUAL(bottomUpOp.totalCount, manager2.nodeCount());
        CPPUNIT_ASSERT_EQUAL(topDownOp.totalCount, manager2.nodeCount());
    }

}
