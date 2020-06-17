#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <iostream>
#include <list>
#include <stdint.h>


namespace {

class OpenMeshSRBinary : public testing::Test {

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

/* Check if len is swapped correctly
 * when storing strings using the binary serializer.
 *
 */
TEST_F(OpenMeshSRBinary, CheckStringSwap) {
  std::string testString = "OpenMesh String";
  std::stringstream stream("");
  OpenMesh::IO::binary<std::string>::store(stream,testString,true);
  std::stringstream stream2("");
  OpenMesh::IO::binary<std::string>::store(stream2,testString,false);
  std::string res2 = stream2.str();
  std::string res = stream.str();
  uint16_t len, len2;
  stream.read((char*)&len,2);

  stream2.read( (char*)&len2,2);

  EXPECT_EQ(len2,testString.length());
  EXPECT_EQ(len,(testString.length()>>8)|(testString.length()<<8));
  EXPECT_NE(len,len2);
}

/* Check if storing and restoring a string gives proper result
 * Do that with and without swapping the byte order
 */
TEST_F(OpenMeshSRBinary, StringStoreRestore) {


  std::string testString = "OpenMesh String";
  std::stringstream stream("");
  OpenMesh::IO::binary<std::string>::store(stream,testString,true);
  std::stringstream stream2("");
  OpenMesh::IO::binary<std::string>::store(stream2,testString,false);


  std::string restored1, restored2;
  OpenMesh::IO::binary<std::string>::restore(stream, restored1 , true);
  OpenMesh::IO::binary<std::string>::restore(stream2, restored2 , false);

  EXPECT_EQ(restored1.length(), restored2.length());
  EXPECT_EQ(restored1.length(), testString.length());
  for(size_t i = 0 ; i < testString.length() ; ++i)
  {
    EXPECT_EQ(restored1[i] , testString[i]);
    EXPECT_EQ(restored2[i] , testString[i]);
  }


}



}
