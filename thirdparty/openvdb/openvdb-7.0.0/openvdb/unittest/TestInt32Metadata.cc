// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/Exceptions.h>
#include <openvdb/Metadata.h>

class TestInt32Metadata : public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestInt32Metadata);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

    void test();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestInt32Metadata);

void
TestInt32Metadata::test()
{
    using namespace openvdb;

    Metadata::Ptr m(new Int32Metadata(123));
    Metadata::Ptr m2 = m->copy();

    CPPUNIT_ASSERT(dynamic_cast<Int32Metadata*>(m.get()) != 0);
    CPPUNIT_ASSERT(dynamic_cast<Int32Metadata*>(m2.get()) != 0);

    CPPUNIT_ASSERT(m->typeName().compare("int32") == 0);
    CPPUNIT_ASSERT(m2->typeName().compare("int32") == 0);

    Int32Metadata *s = dynamic_cast<Int32Metadata*>(m.get());
    CPPUNIT_ASSERT(s->value() == 123);
    s->value() = 456;
    CPPUNIT_ASSERT(s->value() == 456);

    m2->copy(*s);

    s = dynamic_cast<Int32Metadata*>(m2.get());
    CPPUNIT_ASSERT(s->value() == 456);
}
