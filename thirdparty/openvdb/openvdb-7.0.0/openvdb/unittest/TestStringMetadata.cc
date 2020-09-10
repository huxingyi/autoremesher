// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/Exceptions.h>
#include <openvdb/Metadata.h>

class TestStringMetadata : public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestStringMetadata);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

    void test();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestStringMetadata);

void
TestStringMetadata::test()
{
    using namespace openvdb;

    Metadata::Ptr m(new StringMetadata("testing"));
    Metadata::Ptr m2 = m->copy();

    CPPUNIT_ASSERT(dynamic_cast<StringMetadata*>(m.get()) != 0);
    CPPUNIT_ASSERT(dynamic_cast<StringMetadata*>(m2.get()) != 0);

    CPPUNIT_ASSERT(m->typeName().compare("string") == 0);
    CPPUNIT_ASSERT(m2->typeName().compare("string") == 0);

    StringMetadata *s = dynamic_cast<StringMetadata*>(m.get());
    CPPUNIT_ASSERT(s->value().compare("testing") == 0);
    s->value() = "testing2";
    CPPUNIT_ASSERT(s->value().compare("testing2") == 0);

    m2->copy(*s);

    s = dynamic_cast<StringMetadata*>(m2.get());
    CPPUNIT_ASSERT(s->value().compare("testing2") == 0);
}
