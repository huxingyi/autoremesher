#include <gtest/gtest.h>

#include <Unittests/unittests_common.hh>


namespace {


class OpenMeshTypeTest_Poly : public OpenMeshBasePoly {

    protected:

        // This function is called before each test is run
        virtual void SetUp() {

            // Do some initial stuff with the member data here...
        }

        // This function is called after all tests are through
        virtual void TearDown() {

            // Do some final stuff with the member data here...
        }
};

class OpenMeshTypeTest_Triangle : public OpenMeshBase {

    protected:

        // This function is called before each test is run
        virtual void SetUp() {

            // Do some initial stuff with the member data here...
        }

        // This function is called after all tests are through
        virtual void TearDown() {

            // Do some final stuff with the member data here...
        }

};


/*
 * ====================================================================
 * Define tests below
 * ====================================================================
 */

TEST_F(OpenMeshTypeTest_Triangle, testTypeFunctions) {


  EXPECT_TRUE(mesh_.is_trimesh()) << "Type Error!";
  EXPECT_FALSE(mesh_.is_polymesh()) << "Type Error!";
}


TEST_F(OpenMeshTypeTest_Poly, testTypeFunctions) {


  EXPECT_FALSE(mesh_.is_trimesh()) << "Type Error!";
  EXPECT_TRUE(mesh_.is_polymesh()) << "Type Error!";
}


}
