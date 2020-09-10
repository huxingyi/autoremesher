// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/Exceptions.h>
#include <openvdb/io/GridDescriptor.h>
#include <openvdb/openvdb.h>


class TestGridDescriptor: public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestGridDescriptor);
    CPPUNIT_TEST(testIO);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST(testName);
    CPPUNIT_TEST_SUITE_END();

    void testIO();
    void testCopy();
    void testName();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestGridDescriptor);


void
TestGridDescriptor::testIO()
{
    using namespace openvdb::io;
    using namespace openvdb;

    typedef FloatGrid GridType;

    GridDescriptor gd(GridDescriptor::addSuffix("temperature", 2), GridType::gridType());
    gd.setInstanceParentName("temperature_32bit");

    gd.setGridPos(123);
    gd.setBlockPos(234);
    gd.setEndPos(567);

    // write out the gd.
    std::ostringstream ostr(std::ios_base::binary);

    gd.writeHeader(ostr);
    gd.writeStreamPos(ostr);

    // Read in the gd.
    std::istringstream istr(ostr.str(), std::ios_base::binary);

    // Since the input is only a fragment of a VDB file (in particular,
    // it doesn't have a header), set the file format version number explicitly.
    io::setCurrentVersion(istr);

    GridDescriptor gd2;

    CPPUNIT_ASSERT_THROW(gd2.read(istr), openvdb::LookupError);

    // Register the grid.
    GridBase::clearRegistry();
    GridType::registerGrid();

    // seek back and read again.
    istr.seekg(0, std::ios_base::beg);
    GridBase::Ptr grid;
    CPPUNIT_ASSERT_NO_THROW(grid = gd2.read(istr));

    CPPUNIT_ASSERT_EQUAL(gd.gridName(), gd2.gridName());
    CPPUNIT_ASSERT_EQUAL(gd.uniqueName(), gd2.uniqueName());
    CPPUNIT_ASSERT_EQUAL(gd.gridType(), gd2.gridType());
    CPPUNIT_ASSERT_EQUAL(gd.instanceParentName(), gd2.instanceParentName());
    CPPUNIT_ASSERT(grid.get() != NULL);
    CPPUNIT_ASSERT_EQUAL(GridType::gridType(), grid->type());
    CPPUNIT_ASSERT_EQUAL(gd.getGridPos(), gd2.getGridPos());
    CPPUNIT_ASSERT_EQUAL(gd.getBlockPos(), gd2.getBlockPos());
    CPPUNIT_ASSERT_EQUAL(gd.getEndPos(), gd2.getEndPos());

    // Clear the registry when we are done.
    GridBase::clearRegistry();
}


void
TestGridDescriptor::testCopy()
{
    using namespace openvdb::io;
    using namespace openvdb;

    typedef FloatGrid GridType;

    GridDescriptor gd("temperature", GridType::gridType());
    gd.setInstanceParentName("temperature_32bit");

    gd.setGridPos(123);
    gd.setBlockPos(234);
    gd.setEndPos(567);

    GridDescriptor gd2;

    // do the copy
    gd2 = gd;

    CPPUNIT_ASSERT_EQUAL(gd.gridName(), gd2.gridName());
    CPPUNIT_ASSERT_EQUAL(gd.uniqueName(), gd2.uniqueName());
    CPPUNIT_ASSERT_EQUAL(gd.gridType(), gd2.gridType());
    CPPUNIT_ASSERT_EQUAL(gd.instanceParentName(), gd2.instanceParentName());
    CPPUNIT_ASSERT_EQUAL(gd.getGridPos(), gd2.getGridPos());
    CPPUNIT_ASSERT_EQUAL(gd.getBlockPos(), gd2.getBlockPos());
    CPPUNIT_ASSERT_EQUAL(gd.getEndPos(), gd2.getEndPos());
}


void
TestGridDescriptor::testName()
{
    using openvdb::Name;
    using openvdb::io::GridDescriptor;

    const std::string typ = openvdb::FloatGrid::gridType();

    Name name("test");
    GridDescriptor gd(name, typ);

    // Verify that the grid name and the unique name are equivalent
    // when the unique name has no suffix.
    CPPUNIT_ASSERT_EQUAL(name, gd.gridName());
    CPPUNIT_ASSERT_EQUAL(name, gd.uniqueName());
    CPPUNIT_ASSERT_EQUAL(name, GridDescriptor::nameAsString(name));
    CPPUNIT_ASSERT_EQUAL(name, GridDescriptor::stripSuffix(name));

    // Add a suffix.
    name = GridDescriptor::addSuffix("test", 2);
    gd = GridDescriptor(name, typ);

    // Verify that the grid name and the unique name differ
    // when the unique name has a suffix.
    CPPUNIT_ASSERT_EQUAL(name, gd.uniqueName());
    CPPUNIT_ASSERT(gd.gridName() != gd.uniqueName());
    CPPUNIT_ASSERT_EQUAL(GridDescriptor::stripSuffix(name), gd.gridName());
    CPPUNIT_ASSERT_EQUAL(Name("test[2]"), GridDescriptor::nameAsString(name));

    // As above, but with a longer suffix
    name = GridDescriptor::addSuffix("test", 13);
    gd = GridDescriptor(name, typ);

    CPPUNIT_ASSERT_EQUAL(name, gd.uniqueName());
    CPPUNIT_ASSERT(gd.gridName() != gd.uniqueName());
    CPPUNIT_ASSERT_EQUAL(GridDescriptor::stripSuffix(name), gd.gridName());
    CPPUNIT_ASSERT_EQUAL(Name("test[13]"), GridDescriptor::nameAsString(name));

    // Multiple suffixes aren't supported, but verify that
    // they behave reasonably, at least.
    name = GridDescriptor::addSuffix(name, 4);
    gd = GridDescriptor(name, typ);

    CPPUNIT_ASSERT_EQUAL(name, gd.uniqueName());
    CPPUNIT_ASSERT(gd.gridName() != gd.uniqueName());
    CPPUNIT_ASSERT_EQUAL(GridDescriptor::stripSuffix(name), gd.gridName());
}
