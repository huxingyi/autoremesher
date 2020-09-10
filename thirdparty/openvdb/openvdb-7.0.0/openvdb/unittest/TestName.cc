// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/Exceptions.h>
#include <openvdb/util/Name.h>

class TestName : public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestName);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST(testIO);
    CPPUNIT_TEST(testMultipleIO);
    CPPUNIT_TEST_SUITE_END();

    void test();
    void testIO();
    void testMultipleIO();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestName);

void
TestName::test()
{
    using namespace openvdb;

    Name name;
    Name name2("something");
    Name name3 = std::string("something2");
    name = "something";

    CPPUNIT_ASSERT(name == name2);
    CPPUNIT_ASSERT(name != name3);
    CPPUNIT_ASSERT(name != Name("testing"));
    CPPUNIT_ASSERT(name == Name("something"));
}

void
TestName::testIO()
{
    using namespace openvdb;

    Name name("some name that i made up");

    std::ostringstream ostr(std::ios_base::binary);

    openvdb::writeString(ostr, name);

    name = "some other name";

    CPPUNIT_ASSERT(name == Name("some other name"));

    std::istringstream istr(ostr.str(), std::ios_base::binary);

    name = openvdb::readString(istr);

    CPPUNIT_ASSERT(name == Name("some name that i made up"));
}

void
TestName::testMultipleIO()
{
    using namespace openvdb;

    Name name("some name that i made up");
    Name name2("something else");

    std::ostringstream ostr(std::ios_base::binary);

    openvdb::writeString(ostr, name);
    openvdb::writeString(ostr, name2);

    std::istringstream istr(ostr.str(), std::ios_base::binary);

    Name n = openvdb::readString(istr), n2 = openvdb::readString(istr);

    CPPUNIT_ASSERT(name == n);
    CPPUNIT_ASSERT(name2 == n2);
}
