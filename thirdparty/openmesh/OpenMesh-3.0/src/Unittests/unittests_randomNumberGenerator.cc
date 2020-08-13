
#include <gtest/gtest.h>
#include <OpenMesh/Core/Utils/RandomNumberGenerator.hh>

namespace {

class RandomNumberGenerator : public testing::Test {

    protected:

        // This function is called before each test is run
        virtual void SetUp() {
            
            // Do some initial stuff with the member data here...
        }

        // This function is called after all tests are through
        virtual void TearDown() {

            // Do some final stuff with the member data here...
        }

    // Member already defined in OpenMeshBase
    //Mesh mesh_;  
};

/*
 * ====================================================================
 * Define tests below
 * ====================================================================
 */

TEST_F(RandomNumberGenerator, RandomNumberGeneratorMaxTestHighres) {


  OpenMesh::RandomNumberGenerator rng(100000000);

  unsigned int lowerZero = 0;
  unsigned int above1    = 0;

  double average = 0.0;

  for ( unsigned int i = 0 ; i < 100000000 ; ++i) {
    double randomNumber = rng.getRand();
    if ( randomNumber < 0.0  )
      lowerZero++;

    if ( randomNumber > 1.0  )
      above1++;

    average += randomNumber;
  }

  average /= 100000000.0;

  EXPECT_EQ(0u, lowerZero ) << "Below zero!";
  EXPECT_EQ(0u, above1 ) << "Above zero!";

  EXPECT_TRUE( (average - 0.5) < 0.01 ) << "Expected value not 0.5";
}

TEST_F(RandomNumberGenerator, RandomNumberGeneratorMaxTestLowres) {

  OpenMesh::RandomNumberGenerator rng(1000);

  unsigned int lowerZero = 0;
  unsigned int above1    = 0;

  double average = 0.0;

  for ( unsigned int i = 0 ; i < 10000000 ; ++i) {

    double randomNumber = rng.getRand();
     if ( randomNumber < 0.0  )
       lowerZero++;

     if ( randomNumber > 1.0  ) {
       above1++;
     }

     average += randomNumber;
  }

  average /= 10000000.0;

  EXPECT_EQ(0u, lowerZero ) << "Below zero!";
  EXPECT_EQ(0u, above1 ) << "Above zero!";

  EXPECT_TRUE( (average - 0.5) < 0.01 ) << "Expected value not 0.5";
}

}
