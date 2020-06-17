/* ========================================================================= *
 *                                                                           *
 *                               OpenMesh                                    *
 *           Copyright (c) 2001-2015, RWTH-Aachen University                 *
 *           Department of Computer Graphics and Multimedia                  *
 *                          All rights reserved.                             *
 *                            www.openmesh.org                               *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * This file is part of OpenMesh.                                            *
 *---------------------------------------------------------------------------*
 *                                                                           *
 * Redistribution and use in source and binary forms, with or without        *
 * modification, are permitted provided that the following conditions        *
 * are met:                                                                  *
 *                                                                           *
 * 1. Redistributions of source code must retain the above copyright notice, *
 *    this list of conditions and the following disclaimer.                  *
 *                                                                           *
 * 2. Redistributions in binary form must reproduce the above copyright      *
 *    notice, this list of conditions and the following disclaimer in the    *
 *    documentation and/or other materials provided with the distribution.   *
 *                                                                           *
 * 3. Neither the name of the copyright holder nor the names of its          *
 *    contributors may be used to endorse or promote products derived from   *
 *    this software without specific prior written permission.               *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED *
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A           *
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,  *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,       *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR        *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              *
 *                                                                           *
 * ========================================================================= */

#include <iostream>
// -------------------- OpenMesh
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Mesh/Status.hh>

// ----------------------------------------------------------------------------

struct MyTraits : public OpenMesh::DefaultTraits
{
};


typedef OpenMesh::PolyMesh_ArrayKernelT<MyTraits>  MyMesh;


// ----------------------------------------------------------------------------
// Build a simple cube and delete it except one face
  
int main()
{
  MyMesh mesh;

  // the request has to be called before a vertex/face/edge can be deleted. it grants access to the status attribute
  mesh.request_face_status();
  mesh.request_edge_status();
  mesh.request_vertex_status();

  // generate vertices

  MyMesh::VertexHandle vhandle[8];
  MyMesh::FaceHandle   fhandle[6];

  vhandle[0] = mesh.add_vertex(MyMesh::Point(-1, -1,  1));
  vhandle[1] = mesh.add_vertex(MyMesh::Point( 1, -1,  1));
  vhandle[2] = mesh.add_vertex(MyMesh::Point( 1,  1,  1));
  vhandle[3] = mesh.add_vertex(MyMesh::Point(-1,  1,  1));
  vhandle[4] = mesh.add_vertex(MyMesh::Point(-1, -1, -1));
  vhandle[5] = mesh.add_vertex(MyMesh::Point( 1, -1, -1));
  vhandle[6] = mesh.add_vertex(MyMesh::Point( 1,  1, -1));
  vhandle[7] = mesh.add_vertex(MyMesh::Point(-1,  1, -1));


  // generate (quadrilateral) faces

  std::vector<MyMesh::VertexHandle>  tmp_face_vhandles;

  tmp_face_vhandles.clear();
  tmp_face_vhandles.push_back(vhandle[0]);
  tmp_face_vhandles.push_back(vhandle[1]);
  tmp_face_vhandles.push_back(vhandle[2]);
  tmp_face_vhandles.push_back(vhandle[3]);
  fhandle[0] = mesh.add_face(tmp_face_vhandles);
 
  tmp_face_vhandles.clear();
  tmp_face_vhandles.push_back(vhandle[7]);
  tmp_face_vhandles.push_back(vhandle[6]);
  tmp_face_vhandles.push_back(vhandle[5]);
  tmp_face_vhandles.push_back(vhandle[4]);
  fhandle[1] = mesh.add_face(tmp_face_vhandles);
 
  tmp_face_vhandles.clear();
  tmp_face_vhandles.push_back(vhandle[1]);
  tmp_face_vhandles.push_back(vhandle[0]);
  tmp_face_vhandles.push_back(vhandle[4]);
  tmp_face_vhandles.push_back(vhandle[5]);
  fhandle[2] = mesh.add_face(tmp_face_vhandles);
 

  tmp_face_vhandles.clear();
  tmp_face_vhandles.push_back(vhandle[2]);
  tmp_face_vhandles.push_back(vhandle[1]);
  tmp_face_vhandles.push_back(vhandle[5]);
  tmp_face_vhandles.push_back(vhandle[6]);
  fhandle[3] = mesh.add_face(tmp_face_vhandles);


  tmp_face_vhandles.clear();
  tmp_face_vhandles.push_back(vhandle[3]);
  tmp_face_vhandles.push_back(vhandle[2]);
  tmp_face_vhandles.push_back(vhandle[6]);
  tmp_face_vhandles.push_back(vhandle[7]);
  fhandle[4] = mesh.add_face(tmp_face_vhandles);
 

  tmp_face_vhandles.clear();
  tmp_face_vhandles.push_back(vhandle[0]);
  tmp_face_vhandles.push_back(vhandle[3]);
  tmp_face_vhandles.push_back(vhandle[7]);
  tmp_face_vhandles.push_back(vhandle[4]);
  fhandle[5] = mesh.add_face(tmp_face_vhandles);

  // And now delete all faces and vertices
  // except face (vh[7], vh[6], vh[5], vh[4])
  // whose handle resides in fhandle[1]

   
  // Delete face 0
  mesh.delete_face(fhandle[0], false);
  // ... face 2
  mesh.delete_face(fhandle[2], false);
  // ... face 3
  mesh.delete_face(fhandle[3], false);
  // ... face 4
  mesh.delete_face(fhandle[4], false);
  // ... face 5
  mesh.delete_face(fhandle[5], false);
  

  // If isolated vertices result in a face deletion
  // they have to be deleted manually. If you want this
  // to happen automatically, change the second parameter
  // to true.

  // Now delete the isolated vertices 0, 1, 2 and 3
  mesh.delete_vertex(vhandle[0], false);
  mesh.delete_vertex(vhandle[1], false);
  mesh.delete_vertex(vhandle[2], false);
  mesh.delete_vertex(vhandle[3], false);

  // Delete all elements that are marked as deleted
  // from memory.
  mesh.garbage_collection();

  // write mesh to output.obj
  try {
  	if ( !OpenMesh::IO::write_mesh(mesh, "output.off") ) {
	  std::cerr << "Cannot write mesh to file 'output.off'" << std::endl;
  	  return 1;
        }
  }
  catch( std::exception& x )
  {
    std::cerr << x.what() << std::endl;
    return 1;
  }
 
  return 0;
}
