// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/Exceptions.h>
#include <openvdb/Metadata.h>

class TestInt64Metadata : public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestInt64Metadata);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

    void test();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestInt64Metadata);

void
TestInt64Metadata::test()
{
    using namespace openvdb;

    Metadata::Ptr m(new Int64Metadata(123));
    Metadata::Ptr m2 = m->copy();

    CPPUNIT_ASSERT(dynamic_cast<Int64Metadata*>(m.get()) != 0);
    CPPUNIT_ASSERT(dynamic_cast<Int64Metadata*>(m2.get()) != 0);

    CPPUNIT_ASSERT(m->typeName().compare("int64") == 0);
    CPPUNIT_ASSERT(m2->typeName().compare("int64") == 0);

    Int64Metadata *s = dynamic_cast<Int64Metadata*>(m.get());
    CPPUNIT_ASSERT(s->value() == 123);
    s->value() = 456;
    CPPUNIT_ASSERT(s->value() == 456);

    m2->copy(*s);

    s = dynamic_cast<Int64Metadata*>(m2.get());
    CPPUNIT_ASSERT(s->value() == 456);
}
