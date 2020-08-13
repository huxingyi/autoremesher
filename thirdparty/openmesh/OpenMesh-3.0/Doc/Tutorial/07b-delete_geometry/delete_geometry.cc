/*===========================================================================*\
 *                                                                           *
 *                               OpenMesh                                    *
 *      Copyright (C) 2001-2014 by Computer Graphics Group, RWTH Aachen      *
 *                           www.openmesh.org                                *
 *                                                                           *
 *---------------------------------------------------------------------------*
 *  This file is part of OpenMesh.                                           *
 *                                                                           *
 *  OpenMesh is free software: you can redistribute it and/or modify         *
 *  it under the terms of the GNU Lesser General Public License as           *
 *  published by the Free Software Foundation, either version 3 of           *
 *  the License, or (at your option) any later version with the              *
 *  following exceptions:                                                    *
 *                                                                           *
 *  If other files instantiate templates or use macros                       *
 *  or inline functions from this file, or you compile this file and         *
 *  link it with other files to produce an executable, this file does        *
 *  not by itself cause the resulting executable to be covered by the        *
 *  GNU Lesser General Public License. This exception does not however       *
 *  invalidate any other reasons why the executable file might be            *
 *  covered by the GNU Lesser General Public License.                        *
 *                                                                           *
 *  OpenMesh is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU Lesser General Public License for more details.                      *
 *                                                                           *
 *  You should have received a copy of the GNU LesserGeneral Public          *
 *  License along with OpenMesh.  If not,                                    *
 *  see <http://www.gnu.org/licenses/>.                                      *
 *                                                                           *
\*===========================================================================*/

/*===========================================================================*\
 *                                                                           *
 *   $Revision: 736 $                                                         *
 *   $Date: 2012-10-08 09:30:49 +0200 (Mo, 08. Okt 2012) $                   *
 *                                                                           *
\*===========================================================================*/


#include <iostream>
// -------------------- OpenMesh
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Mesh/Status.hh>

// ----------------------------------------------------------------------------

struct MyTraits : public OpenMesh::DefaultTraits
{
  VertexAttributes(OpenMesh::Attributes::Status);
  FaceAttributes(OpenMesh::Attributes::Status);
  EdgeAttributes(OpenMesh::Attributes::Status);
};


typedef OpenMesh::PolyMesh_ArrayKernelT<MyTraits>  MyMesh;


// ----------------------------------------------------------------------------
// Build a simple cube and delete it except one face
  
int main()
{
  MyMesh mesh;

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
