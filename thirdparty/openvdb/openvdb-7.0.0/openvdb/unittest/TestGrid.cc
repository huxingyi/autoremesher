// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <openvdb/Exceptions.h>
#include <openvdb/openvdb.h>
#include <openvdb/Types.h>
#include <openvdb/util/Name.h>
#include <openvdb/math/Transform.h>
#include <openvdb/Grid.h>
#include <openvdb/tree/Tree.h>
#include <openvdb/util/CpuTimer.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <memory> // for std::make_unique

#define ASSERT_DOUBLES_EXACTLY_EQUAL(expected, actual) \
    CPPUNIT_ASSERT_DOUBLES_EQUAL((expected), (actual), /*tolerance=*/0.0);

class TestGrid: public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestGrid);
    CPPUNIT_TEST(testGridRegistry);
    CPPUNIT_TEST(testConstPtr);
    CPPUNIT_TEST(testGetGrid);
    CPPUNIT_TEST(testIsType);
    CPPUNIT_TEST(testTransform);
    CPPUNIT_TEST(testCopyGrid);
    CPPUNIT_TEST(testValueConversion);
    CPPUNIT_TEST(testClipping);
    CPPUNIT_TEST(testApply);
    CPPUNIT_TEST_SUITE_END();

    void testGridRegistry();
    void testConstPtr();
    void testGetGrid();
    void testIsType();
    void testTransform();
    void testCopyGrid();
    void testValueConversion();
    void testClipping();
    void testApply();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestGrid);


////////////////////////////////////////


class ProxyTree: public openvdb::TreeBase
{
public:
    using ValueType = int;
    using BuildType = int;
    using LeafNodeType = void;
    using ValueAllCIter = void;
    using ValueAllIter = void;
    using ValueOffCIter = void;
    using ValueOffIter = void;
    using ValueOnCIter = void;
    using ValueOnIter = void;
    using TreeBasePtr = openvdb::TreeBase::Ptr;
    using Ptr = openvdb::SharedPtr<ProxyTree>;
    using ConstPtr = openvdb::SharedPtr<const ProxyTree>;

    static const openvdb::Index DEPTH;
    static const ValueType backg;

    ProxyTree() {}
    ProxyTree(const ValueType&) {}
    ProxyTree(const ProxyTree&) = default;
    ~ProxyTree() override = default;

    static const openvdb::Name& treeType() { static const openvdb::Name s("proxy"); return s; }
    const openvdb::Name& type() const override { return treeType(); }
    openvdb::Name valueType() const override { return "proxy"; }
    const ValueType& background() const { return backg; }

    TreeBasePtr copy() const override { return TreeBasePtr(new ProxyTree(*this)); }

    void readTopology(std::istream& is, bool = false) override { is.seekg(0, std::ios::beg); }
    void writeTopology(std::ostream& os, bool = false) const override { os.seekp(0); }

    void readBuffers(std::istream& is,
        const openvdb::CoordBBox&, bool /*saveFloatAsHalf*/=false) override { is.seekg(0); }
    void readNonresidentBuffers() const override {}
    void readBuffers(std::istream& is, bool /*saveFloatAsHalf*/=false) override { is.seekg(0); }
    void writeBuffers(std::ostream& os, bool /*saveFloatAsHalf*/=false) const override
        { os.seekp(0, std::ios::beg); }

    bool empty() const { return true; }
    void clear() {}
    void prune(const ValueType& = 0) {}
    void clip(const openvdb::CoordBBox&) {}
    void clipUnallocatedNodes() override {}
#if OPENVDB_ABI_VERSION_NUMBER >= 4
    openvdb::Index32 unallocatedLeafCount() const override { return 0; }
#endif

    void getIndexRange(openvdb::CoordBBox&) const override {}
    bool evalLeafBoundingBox(openvdb::CoordBBox& bbox) const override
        { bbox.min() = bbox.max() = openvdb::Coord(0, 0, 0); return false; }
    bool evalActiveVoxelBoundingBox(openvdb::CoordBBox& bbox) const override
        { bbox.min() = bbox.max() = openvdb::Coord(0, 0, 0); return false; }
    bool evalActiveVoxelDim(openvdb::Coord& dim) const override
        { dim = openvdb::Coord(0, 0, 0); return false; }
    bool evalLeafDim(openvdb::Coord& dim) const override
        { dim = openvdb::Coord(0, 0, 0); return false; }

    openvdb::Index treeDepth() const override { return 0; }
    openvdb::Index leafCount() const override { return 0; }
#if OPENVDB_ABI_VERSION_NUMBER >= 7
    std::vector<openvdb::Index32> nodeCount() const override
        { return std::vector<openvdb::Index32>(DEPTH, 0); }
#endif
    openvdb::Index nonLeafCount() const override { return 0; }
    openvdb::Index64 activeVoxelCount() const override { return 0UL; }
    openvdb::Index64 inactiveVoxelCount() const override { return 0UL; }
    openvdb::Index64 activeLeafVoxelCount() const override { return 0UL; }
    openvdb::Index64 inactiveLeafVoxelCount() const override { return 0UL; }
    openvdb::Index64 activeTileCount() const override { return 0UL; }
};

const openvdb::Index ProxyTree::DEPTH = 0;
const ProxyTree::ValueType ProxyTree::backg = 0;

using ProxyGrid = openvdb::Grid<ProxyTree>;


////////////////////////////////////////

void
TestGrid::testGridRegistry()
{
    using namespace openvdb::tree;

    using TreeType = Tree<RootNode<InternalNode<LeafNode<float, 3>, 2> > >;
    using GridType = openvdb::Grid<TreeType>;

    openvdb::GridBase::clearRegistry();

    CPPUNIT_ASSERT(!GridType::isRegistered());
    GridType::registerGrid();
    CPPUNIT_ASSERT(GridType::isRegistered());
    CPPUNIT_ASSERT_THROW(GridType::registerGrid(), openvdb::KeyError);
    GridType::unregisterGrid();
    CPPUNIT_ASSERT(!GridType::isRegistered());
    CPPUNIT_ASSERT_NO_THROW(GridType::unregisterGrid());
    CPPUNIT_ASSERT(!GridType::isRegistered());
    CPPUNIT_ASSERT_NO_THROW(GridType::registerGrid());
    CPPUNIT_ASSERT(GridType::isRegistered());

    openvdb::GridBase::clearRegistry();
}


void
TestGrid::testConstPtr()
{
    using namespace openvdb;

    GridBase::ConstPtr constgrid = ProxyGrid::create();

    CPPUNIT_ASSERT_EQUAL(Name("proxy"), constgrid->type());
}


void
TestGrid::testGetGrid()
{
    using namespace openvdb;

    GridBase::Ptr grid = FloatGrid::create(/*bg=*/0.0);
    GridBase::ConstPtr constGrid = grid;

    CPPUNIT_ASSERT(grid->baseTreePtr());

    CPPUNIT_ASSERT(!gridPtrCast<DoubleGrid>(grid));
    CPPUNIT_ASSERT(!gridPtrCast<DoubleGrid>(grid));

    CPPUNIT_ASSERT(gridConstPtrCast<FloatGrid>(constGrid));
    CPPUNIT_ASSERT(!gridConstPtrCast<DoubleGrid>(constGrid));
}


void
TestGrid::testIsType()
{
    using namespace openvdb;

    GridBase::Ptr grid = FloatGrid::create();
    CPPUNIT_ASSERT(grid->isType<FloatGrid>());
    CPPUNIT_ASSERT(!grid->isType<DoubleGrid>());
}


void
TestGrid::testTransform()
{
    ProxyGrid grid;

    // Verify that the grid has a valid default transform.
    CPPUNIT_ASSERT(grid.transformPtr());

    // Verify that a null transform pointer is not allowed.
    CPPUNIT_ASSERT_THROW(grid.setTransform(openvdb::math::Transform::Ptr()),
        openvdb::ValueError);

    grid.setTransform(openvdb::math::Transform::createLinearTransform());

    CPPUNIT_ASSERT(grid.transformPtr());

    // Verify that calling Transform-related Grid methods (Grid::voxelSize(), etc.)
    // is the same as calling those methods on the Transform.

    CPPUNIT_ASSERT(grid.transform().voxelSize().eq(grid.voxelSize()));
    CPPUNIT_ASSERT(grid.transform().voxelSize(openvdb::Vec3d(0.1, 0.2, 0.3)).eq(
        grid.voxelSize(openvdb::Vec3d(0.1, 0.2, 0.3))));

    CPPUNIT_ASSERT(grid.transform().indexToWorld(openvdb::Vec3d(0.1, 0.2, 0.3)).eq(
        grid.indexToWorld(openvdb::Vec3d(0.1, 0.2, 0.3))));
    CPPUNIT_ASSERT(grid.transform().indexToWorld(openvdb::Coord(1, 2, 3)).eq(
        grid.indexToWorld(openvdb::Coord(1, 2, 3))));
    CPPUNIT_ASSERT(grid.transform().worldToIndex(openvdb::Vec3d(0.1, 0.2, 0.3)).eq(
        grid.worldToIndex(openvdb::Vec3d(0.1, 0.2, 0.3))));
}


void
TestGrid::testCopyGrid()
{
    using namespace openvdb;

    // set up a grid
    const float fillValue1=5.0f;
    FloatGrid::Ptr grid1 = createGrid<FloatGrid>(/*bg=*/fillValue1);
    FloatTree& tree1 = grid1->tree();
    tree1.setValue(Coord(-10,40,845), 3.456f);
    tree1.setValue(Coord(1,-50,-8), 1.0f);

    // create a new grid, copying the first grid
    GridBase::Ptr grid2 = grid1->deepCopy();

    // cast down to the concrete type to query values
    FloatTree& tree2 = gridPtrCast<FloatGrid>(grid2)->tree();

    // compare topology
    CPPUNIT_ASSERT(tree1.hasSameTopology(tree2));
    CPPUNIT_ASSERT(tree2.hasSameTopology(tree1));

    // trees should be equal
    ASSERT_DOUBLES_EXACTLY_EQUAL(fillValue1, tree2.getValue(Coord(1,2,3)));
    ASSERT_DOUBLES_EXACTLY_EQUAL(3.456f, tree2.getValue(Coord(-10,40,845)));
    ASSERT_DOUBLES_EXACTLY_EQUAL(1.0f, tree2.getValue(Coord(1,-50,-8)));

    // change 1 value in tree2
    Coord changeCoord(1, -500, -8);
    tree2.setValue(changeCoord, 1.0f);

    // topology should no longer match
    CPPUNIT_ASSERT(!tree1.hasSameTopology(tree2));
    CPPUNIT_ASSERT(!tree2.hasSameTopology(tree1));

    // query changed value and make sure it's different between trees
    ASSERT_DOUBLES_EXACTLY_EQUAL(fillValue1, tree1.getValue(changeCoord));
    ASSERT_DOUBLES_EXACTLY_EQUAL(1.0f, tree2.getValue(changeCoord));

#if OPENVDB_ABI_VERSION_NUMBER >= 7
    // shallow-copy a const grid but supply a new transform and meta map
    CPPUNIT_ASSERT_EQUAL(1.0, grid1->transform().voxelSize().x());
    CPPUNIT_ASSERT_EQUAL(size_t(0), grid1->metaCount());
    CPPUNIT_ASSERT_EQUAL(Index(2), grid1->tree().leafCount());

    math::Transform::Ptr xform(math::Transform::createLinearTransform(/*voxelSize=*/0.25));
    MetaMap meta;
    meta.insertMeta("test", Int32Metadata(4));

    FloatGrid::ConstPtr constGrid1 = ConstPtrCast<const FloatGrid>(grid1);

    GridBase::ConstPtr grid3 = constGrid1->copyGridReplacingMetadataAndTransform(meta, xform);
    const FloatTree& tree3 = gridConstPtrCast<FloatGrid>(grid3)->tree();

    CPPUNIT_ASSERT_EQUAL(0.25, grid3->transform().voxelSize().x());
    CPPUNIT_ASSERT_EQUAL(size_t(1), grid3->metaCount());
    CPPUNIT_ASSERT_EQUAL(Index(2), tree3.leafCount());
    CPPUNIT_ASSERT_EQUAL(long(3), constGrid1->constTreePtr().use_count());
#endif
}


void
TestGrid::testValueConversion()
{
    using namespace openvdb;

    const Coord c0(-10, 40, 845), c1(1, -50, -8), c2(1, 2, 3);
    const float fval0 = 3.25f, fval1 = 1.0f, fbkgd = 5.0f;

    // Create a FloatGrid.
    FloatGrid fgrid(fbkgd);
    FloatTree& ftree = fgrid.tree();
    ftree.setValue(c0, fval0);
    ftree.setValue(c1, fval1);

    // Copy the FloatGrid to a DoubleGrid.
    DoubleGrid dgrid(fgrid);
    DoubleTree& dtree = dgrid.tree();
    // Compare topology.
    CPPUNIT_ASSERT(dtree.hasSameTopology(ftree));
    CPPUNIT_ASSERT(ftree.hasSameTopology(dtree));
    // Compare values.
    ASSERT_DOUBLES_EXACTLY_EQUAL(double(fbkgd), dtree.getValue(c2));
    ASSERT_DOUBLES_EXACTLY_EQUAL(double(fval0), dtree.getValue(c0));
    ASSERT_DOUBLES_EXACTLY_EQUAL(double(fval1), dtree.getValue(c1));

    // Copy the FloatGrid to a BoolGrid.
    BoolGrid bgrid(fgrid);
    BoolTree& btree = bgrid.tree();
    // Compare topology.
    CPPUNIT_ASSERT(btree.hasSameTopology(ftree));
    CPPUNIT_ASSERT(ftree.hasSameTopology(btree));
    // Compare values.
    CPPUNIT_ASSERT_EQUAL(bool(fbkgd), btree.getValue(c2));
    CPPUNIT_ASSERT_EQUAL(bool(fval0), btree.getValue(c0));
    CPPUNIT_ASSERT_EQUAL(bool(fval1), btree.getValue(c1));

    // Copy the FloatGrid to a Vec3SGrid.
    Vec3SGrid vgrid(fgrid);
    Vec3STree& vtree = vgrid.tree();
    // Compare topology.
    CPPUNIT_ASSERT(vtree.hasSameTopology(ftree));
    CPPUNIT_ASSERT(ftree.hasSameTopology(vtree));
    // Compare values.
    CPPUNIT_ASSERT_EQUAL(Vec3s(fbkgd), vtree.getValue(c2));
    CPPUNIT_ASSERT_EQUAL(Vec3s(fval0), vtree.getValue(c0));
    CPPUNIT_ASSERT_EQUAL(Vec3s(fval1), vtree.getValue(c1));

    // Verify that a Vec3SGrid can't be copied to an Int32Grid
    // (because an Int32 can't be constructed from a Vec3S).
    CPPUNIT_ASSERT_THROW(Int32Grid igrid2(vgrid), openvdb::TypeError);

    // Verify that a grid can't be converted to another type with a different
    // tree configuration.
    using DTree23 = tree::Tree3<double, 2, 3>::Type;
    using DGrid23 = Grid<DTree23>;
    CPPUNIT_ASSERT_THROW(DGrid23 d23grid(fgrid), openvdb::TypeError);
}


////////////////////////////////////////


template<typename GridT>
void
validateClippedGrid(const GridT& clipped, const typename GridT::ValueType& fg)
{
    using namespace openvdb;

    using ValueT = typename GridT::ValueType;

    const CoordBBox bbox = clipped.evalActiveVoxelBoundingBox();
    CPPUNIT_ASSERT_EQUAL(4, bbox.min().x());
    CPPUNIT_ASSERT_EQUAL(4, bbox.min().y());
    CPPUNIT_ASSERT_EQUAL(-6, bbox.min().z());
    CPPUNIT_ASSERT_EQUAL(4, bbox.max().x());
    CPPUNIT_ASSERT_EQUAL(4, bbox.max().y());
    CPPUNIT_ASSERT_EQUAL(6, bbox.max().z());
    CPPUNIT_ASSERT_EQUAL(6 + 6 + 1, int(clipped.activeVoxelCount()));
    CPPUNIT_ASSERT_EQUAL(2, int(clipped.constTree().leafCount()));

    typename GridT::ConstAccessor acc = clipped.getConstAccessor();
    const ValueT bg = clipped.background();
    Coord xyz;
    int &x = xyz[0], &y = xyz[1], &z = xyz[2];
    for (x = -10; x <= 10; ++x) {
        for (y = -10; y <= 10; ++y) {
            for (z = -10; z <= 10; ++z) {
                if (x == 4 && y == 4 && z >= -6 && z <= 6) {
                    CPPUNIT_ASSERT_EQUAL(fg, acc.getValue(Coord(4, 4, z)));
                } else {
                    CPPUNIT_ASSERT_EQUAL(bg, acc.getValue(Coord(x, y, z)));
                }
            }
        }
    }
}


// See also TestTools::testClipping()
void
TestGrid::testClipping()
{
    using namespace openvdb;

    const BBoxd clipBox(Vec3d(4.0, 4.0, -6.0), Vec3d(4.9, 4.9, 6.0));

    {
        const float fg = 5.f;
        FloatGrid cube(0.f);
        cube.fill(CoordBBox(Coord(-10), Coord(10)), /*value=*/fg, /*active=*/true);
        cube.clipGrid(clipBox);
        validateClippedGrid(cube, fg);
    }
    {
        const bool fg = true;
        BoolGrid cube(false);
        cube.fill(CoordBBox(Coord(-10), Coord(10)), /*value=*/fg, /*active=*/true);
        cube.clipGrid(clipBox);
        validateClippedGrid(cube, fg);
    }
    {
        const Vec3s fg(1.f, -2.f, 3.f);
        Vec3SGrid cube(Vec3s(0.f));
        cube.fill(CoordBBox(Coord(-10), Coord(10)), /*value=*/fg, /*active=*/true);
        cube.clipGrid(clipBox);
        validateClippedGrid(cube, fg);
    }
    /*
    {// Benchmark multi-threaded copy construction
        openvdb::util::CpuTimer timer;
        openvdb::initialize();
        openvdb::io::File file("/usr/pic1/Data/OpenVDB/LevelSetModels/crawler.vdb");
        file.open();
        openvdb::GridBase::Ptr baseGrid = file.readGrid("ls_crawler");
        file.close();
        openvdb::FloatGrid::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
        //grid->tree().print();
        timer.start("\nCopy construction");
        openvdb::FloatTree fTree(grid->tree());
        timer.stop();

        timer.start("\nBoolean topology copy construction");
        openvdb::BoolTree bTree(grid->tree(), false, openvdb::TopologyCopy());
        timer.stop();

        timer.start("\nBoolean topology union");
        bTree.topologyUnion(fTree);
        timer.stop();
        //bTree.print();
    }
    */
}


////////////////////////////////////////


namespace {

struct GridOp
{
    bool isConst = false;
    template<typename GridT> void operator()(const GridT&) { isConst = true; }
    template<typename GridT> void operator()(GridT&) { isConst = false; }
};

} // anonymous namespace


void
TestGrid::testApply()
{
    using namespace openvdb;

    const GridBase::Ptr
        boolGrid = BoolGrid::create(),
        floatGrid = FloatGrid::create(),
        doubleGrid = DoubleGrid::create(),
        intGrid = Int32Grid::create();

    const GridBase::ConstPtr
        boolCGrid = BoolGrid::create(),
        floatCGrid = FloatGrid::create(),
        doubleCGrid = DoubleGrid::create(),
        intCGrid = Int32Grid::create();

    {
        using AllowedGridTypes = TypeList<>;

        // Verify that the functor is not applied to any of the grids.
        GridOp op;
        CPPUNIT_ASSERT(!boolGrid->apply<AllowedGridTypes>(op));
        CPPUNIT_ASSERT(!boolCGrid->apply<AllowedGridTypes>(op));
        CPPUNIT_ASSERT(!floatGrid->apply<AllowedGridTypes>(op));
        CPPUNIT_ASSERT(!floatCGrid->apply<AllowedGridTypes>(op));
        CPPUNIT_ASSERT(!doubleGrid->apply<AllowedGridTypes>(op));
        CPPUNIT_ASSERT(!doubleCGrid->apply<AllowedGridTypes>(op));
        CPPUNIT_ASSERT(!intGrid->apply<AllowedGridTypes>(op));
        CPPUNIT_ASSERT(!intCGrid->apply<AllowedGridTypes>(op));
    }
    {
        using AllowedGridTypes = TypeList<FloatGrid, FloatGrid, DoubleGrid>;

        // Verify that the functor is applied only to grids of the allowed types
        // and that their constness is respected.
        GridOp op;
        CPPUNIT_ASSERT(!boolGrid->apply<AllowedGridTypes>(op));
        CPPUNIT_ASSERT(!intGrid->apply<AllowedGridTypes>(op));
        CPPUNIT_ASSERT(floatGrid->apply<AllowedGridTypes>(op));  CPPUNIT_ASSERT(!op.isConst);
        CPPUNIT_ASSERT(doubleGrid->apply<AllowedGridTypes>(op)); CPPUNIT_ASSERT(!op.isConst);

        CPPUNIT_ASSERT(!boolCGrid->apply<AllowedGridTypes>(op));
        CPPUNIT_ASSERT(!intCGrid->apply<AllowedGridTypes>(op));
        CPPUNIT_ASSERT(floatCGrid->apply<AllowedGridTypes>(op));  CPPUNIT_ASSERT(op.isConst);
        CPPUNIT_ASSERT(doubleCGrid->apply<AllowedGridTypes>(op)); CPPUNIT_ASSERT(op.isConst);
    }
    {
        using AllowedGridTypes = TypeList<FloatGrid, DoubleGrid>;

        // Verify that rvalue functors are supported.
        int n = 0;
        CPPUNIT_ASSERT(  !boolGrid->apply<AllowedGridTypes>([&n](GridBase&) { ++n; }));
        CPPUNIT_ASSERT(   !intGrid->apply<AllowedGridTypes>([&n](GridBase&) { ++n; }));
        CPPUNIT_ASSERT(  floatGrid->apply<AllowedGridTypes>([&n](GridBase&) { ++n; }));
        CPPUNIT_ASSERT( doubleGrid->apply<AllowedGridTypes>([&n](GridBase&) { ++n; }));
        CPPUNIT_ASSERT( !boolCGrid->apply<AllowedGridTypes>([&n](const GridBase&) { ++n; }));
        CPPUNIT_ASSERT(  !intCGrid->apply<AllowedGridTypes>([&n](const GridBase&) { ++n; }));
        CPPUNIT_ASSERT( floatCGrid->apply<AllowedGridTypes>([&n](const GridBase&) { ++n; }));
        CPPUNIT_ASSERT(doubleCGrid->apply<AllowedGridTypes>([&n](const GridBase&) { ++n; }));
        CPPUNIT_ASSERT_EQUAL(4, n);
    }
}
