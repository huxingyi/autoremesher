// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/Exceptions.h>
#include <openvdb/Metadata.h>

class TestDoubleMetadata : public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestDoubleMetadata);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

    void test();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestDoubleMetadata);

void
TestDoubleMetadata::test()
{
    using namespace openvdb;

    Metadata::Ptr m(new DoubleMetadata(1.23));
    Metadata::Ptr m2 = m->copy();

    CPPUNIT_ASSERT(dynamic_cast<DoubleMetadata*>(m.get()) != 0);
    CPPUNIT_ASSERT(dynamic_cast<DoubleMetadata*>(m2.get()) != 0);

    CPPUNIT_ASSERT(m->typeName().compare("double") == 0);
    CPPUNIT_ASSERT(m2->typeName().compare("double") == 0);

    DoubleMetadata *s = dynamic_cast<DoubleMetadata*>(m.get());
    //CPPUNIT_ASSERT(s->value() == 1.23);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23,s->value(),0);
    s->value() = 4.56;
    //CPPUNIT_ASSERT(s->value() == 4.56);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.56,s->value(),0);

    m2->copy(*s);

    s = dynamic_cast<DoubleMetadata*>(m2.get());
    //CPPUNIT_ASSERT(s->value() == 4.56);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.56,s->value(),0);
}
