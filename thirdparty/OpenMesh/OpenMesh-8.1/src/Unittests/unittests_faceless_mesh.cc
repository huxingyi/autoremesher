
#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <iostream>

namespace {

class OpenMeshFacelessMesh : public OpenMeshBase {

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

///*
// * ====================================================================
// * Define tests below
// * ====================================================================
// */
//
// */
//TEST_F(OpenMeshFacelessMesh, TestCirculatorsAndIterators) {
//
// This setup is not supported by OpenMesh, we keep this test, if somebody creates
// a connectivity class for faceless graph setup.
////  mesh_.clear();
////
////  // Add some vertices
////  Mesh::VertexHandle vhandle[4];
////
////  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
////  vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
////  vhandle[2] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
////  vhandle[3] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
////
////
////  Mesh::HalfedgeHandle heh00 = mesh_.new_edge(vhandle[0], vhandle[1]);
////  Mesh::HalfedgeHandle heh10 = mesh_.new_edge(vhandle[1], vhandle[2]);
////
////
////  // Halfedge Handles do not work in this setting!
////  Mesh::HalfedgeHandle invalid_heh = mesh_.next_halfedge_handle(heh00);
//
////  //second he circulators does not work
////
////  auto invalid_heh_circ = mesh_.cvoh_iter(vh1);
////
////  //third, and most important, split_edge fails
////
////  Mesh::VertexHandle vh12 = mesh_.new_vertex(Mesh::Point(1.5, 0, 0));
////  mesh_.split_edge(mesh_.edge_handle(heh10), vh12); //runtime error
//
//}

}
