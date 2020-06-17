#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <iostream>
#include <OpenMesh/Tools/SmartTagger/SmartTaggerT.hh>

namespace {
    
class OpenMeshSmartTagger : public OpenMeshBase {

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

/* Checks SmartTagger on vertices
 */
TEST_F(OpenMeshSmartTagger, SmartTaggerVertices) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[7];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(0,-1, 0));
  vhandle[4] = mesh_.add_vertex(Mesh::Point(2,-1, 0));
  vhandle[5] = mesh_.add_vertex(Mesh::Point(3, 0, 0));


  // Add two faces
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[1]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);

    /* Test setup:
        0 ==== 2
        |\    /|\
        | \  / | \
        |   1  |  5
        | /  \ | /
        |/    \|/
        3 ==== 4

        */


  OpenMesh::SmartTaggerVT< Mesh > tagger(mesh_);


  EXPECT_FALSE( tagger.is_tagged(vhandle[0] ) ) << "Vertex should be untagged after init!";
  EXPECT_FALSE( tagger.is_tagged(vhandle[1] ) ) << "Vertex should be untagged after init!";
  EXPECT_FALSE( tagger.is_tagged(vhandle[2] ) ) << "Vertex should be untagged after init!";
  EXPECT_FALSE( tagger.is_tagged(vhandle[3] ) ) << "Vertex should be untagged after init!";
  EXPECT_FALSE( tagger.is_tagged(vhandle[4] ) ) << "Vertex should be untagged after init!";
  EXPECT_FALSE( tagger.is_tagged(vhandle[5] ) ) << "Vertex should be untagged after init!";

  // Reset tagged flag on all vertices
  tagger.untag_all();

  EXPECT_FALSE( tagger.is_tagged(vhandle[0] ) ) << "Vertex should be untagged after first untag_all!";
  EXPECT_FALSE( tagger.is_tagged(vhandle[1] ) ) << "Vertex should be untagged after first untag_all!";
  EXPECT_FALSE( tagger.is_tagged(vhandle[2] ) ) << "Vertex should be untagged after first untag_all!";
  EXPECT_FALSE( tagger.is_tagged(vhandle[3] ) ) << "Vertex should be untagged after first untag_all!";
  EXPECT_FALSE( tagger.is_tagged(vhandle[4] ) ) << "Vertex should be untagged after first untag_all!";
  EXPECT_FALSE( tagger.is_tagged(vhandle[5] ) ) << "Vertex should be untagged after first untag_all!";


  // Set tagged:
  tagger.set_tag(vhandle[2]);
  tagger.set_tag(vhandle[4]);

  EXPECT_FALSE( tagger.is_tagged(vhandle[0] ) ) << "Vertex should be untagged!";
  EXPECT_FALSE( tagger.is_tagged(vhandle[1] ) ) << "Vertex should be untagged!";
  EXPECT_TRUE( tagger.is_tagged(vhandle[2] ) )  << "Vertex should be tagged!";
  EXPECT_FALSE( tagger.is_tagged(vhandle[3] ) ) << "Vertex should be untagged!";
  EXPECT_TRUE( tagger.is_tagged(vhandle[4] ) )  << "Vertex should be tagged!";
  EXPECT_FALSE( tagger.is_tagged(vhandle[5] ) ) << "Vertex should be untagged!";

  // Reset tagged flag on all vertices
  tagger.untag_all();

  EXPECT_FALSE( tagger.is_tagged(vhandle[0] ) ) << "Vertex should be untagged after second untag_all!";
  EXPECT_FALSE( tagger.is_tagged(vhandle[1] ) ) << "Vertex should be untagged after second untag_all!";
  EXPECT_FALSE( tagger.is_tagged(vhandle[2] ) ) << "Vertex should be untagged after second untag_all!";
  EXPECT_FALSE( tagger.is_tagged(vhandle[3] ) ) << "Vertex should be untagged after second untag_all!";
  EXPECT_FALSE( tagger.is_tagged(vhandle[4] ) ) << "Vertex should be untagged after second untag_all!";
  EXPECT_FALSE( tagger.is_tagged(vhandle[5] ) ) << "Vertex should be untagged after second untag_all!";

}

/* Checks SmartTagger on vertices
 */
TEST_F(OpenMeshSmartTagger, SmartTaggerFaces) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[7];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(0,-1, 0));
  vhandle[4] = mesh_.add_vertex(Mesh::Point(2,-1, 0));
  vhandle[5] = mesh_.add_vertex(Mesh::Point(3, 0, 0));


  // Add two faces
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  Mesh::FaceHandle fh1 = mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[4]);
  Mesh::FaceHandle fh2 = mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[1]);
  Mesh::FaceHandle fh3 = mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[4]);
  Mesh::FaceHandle fh4 = mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[4]);
  Mesh::FaceHandle fh5 = mesh_.add_face(face_vhandles);

    /* Test setup:
        0 ==== 2
        |\    /|\
        | \  / | \
        |   1  |  5
        | /  \ | /
        |/    \|/
        3 ==== 4

        */


  OpenMesh::SmartTaggerFT< Mesh > tagger(mesh_);


  EXPECT_FALSE( tagger.is_tagged( fh1 ) ) << "Face should be untagged after init!";
  EXPECT_FALSE( tagger.is_tagged( fh2 ) ) << "Face should be untagged after init!";
  EXPECT_FALSE( tagger.is_tagged( fh3 ) ) << "Face should be untagged after init!";
  EXPECT_FALSE( tagger.is_tagged( fh4 ) ) << "Face should be untagged after init!";
  EXPECT_FALSE( tagger.is_tagged( fh5 ) ) << "Face should be untagged after init!";


  // Reset tagged flag on all vertices
  tagger.untag_all();

  EXPECT_FALSE( tagger.is_tagged( fh1 ) ) << "Face should be untagged after first untag_all!";
  EXPECT_FALSE( tagger.is_tagged( fh2 ) ) << "Face should be untagged after first untag_all!";
  EXPECT_FALSE( tagger.is_tagged( fh3 ) ) << "Face should be untagged after first untag_all!";
  EXPECT_FALSE( tagger.is_tagged( fh4 ) ) << "Face should be untagged after first untag_all!";
  EXPECT_FALSE( tagger.is_tagged( fh5 ) ) << "Face should be untagged after first untag_all!";



  // Set tagged:
  tagger.set_tag(fh3);
  tagger.set_tag(fh5);


  EXPECT_FALSE( tagger.is_tagged(fh1 ) ) << "Face should be untagged!";
  EXPECT_FALSE( tagger.is_tagged(fh2 ) ) << "Face should be untagged!";
  EXPECT_TRUE( tagger.is_tagged(fh3 ) ) << "Face should be tagged!";
  EXPECT_FALSE( tagger.is_tagged(fh4 ) ) << "Face should be tagged!";
  EXPECT_TRUE( tagger.is_tagged(fh5 ) )  << "Face should be tagged!";


  // Reset tagged flag on all vertices
  tagger.untag_all();

  EXPECT_FALSE( tagger.is_tagged( fh1 ) ) << "Face should be untagged after second untag_all!";
  EXPECT_FALSE( tagger.is_tagged( fh2 ) ) << "Face should be untagged after second untag_all!";
  EXPECT_FALSE( tagger.is_tagged( fh3 ) ) << "Face should be untagged after second untag_all!";
  EXPECT_FALSE( tagger.is_tagged( fh4 ) ) << "Face should be untagged after second untag_all!";
  EXPECT_FALSE( tagger.is_tagged( fh5 ) ) << "Face should be untagged after second untag_all!";

}

}
