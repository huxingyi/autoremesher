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
 *   $Revision$                                                         *
 *   $Date$                   *
 *                                                                           *
\*===========================================================================*/


#if !defined(USE_SOQT)
#  define USE_SOQT 0
#endif

//== INCLUDES =================================================================


#include <cstdlib>
//
// Attention! must include this before including inventor files!
// There some dependencies not solved yet!
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
//
#include <Inventor/SoDB.h>
#if USE_SOQT
#  include <qapplication.h>
#  include <Inventor/Qt/SoQt.h>
#  include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
#else
#  include <Inventor/Xt/SoXt.h>
#  include <Inventor/Xt/viewers/SoXtExaminerViewer.h>
#endif
#include <Inventor/nodes/SoSeparator.h>
//
#include <OpenMesh/Apps/IvViewer/SoOpenMeshNodeT.hh>


//== CLASS DEFINITION =========================================================


struct MyTraits : public OpenMesh::DefaultTraits
{
   VertexAttributes(OpenMesh::Attributes::Normal);
   HalfedgeAttributes(OpenMesh::Attributes::PrevHalfedge);
   FaceAttributes(OpenMesh::Attributes::Normal);
};

  
typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits>  MyMesh;
typedef OpenMesh::SoOpenMeshNodeT<MyMesh>         MyNode;



//== IMPLEMENTATION ===========================================================



int main(int argc, char **argv)
{
   OpenMesh::IO::Options opt;

#if USE_SOQT
   QApplication app(argc,argv);

   SoQt::init( argv[0] );

   MyNode::initClass();
   SoQtExaminerViewer *myViewer = new SoQtExaminerViewer();


   // Read a mesh
   MyMesh  mesh;
   if (argc > 1 && OpenMesh::IO::read_mesh(mesh, argv[1], opt))
   {
    if (!opt.check( OpenMesh::IO::Options::FaceNormal))
      mesh.update_face_normals();
      
      SoSeparator* root = new SoSeparator();
      root->addChild(new MyNode(&mesh));
      
      myViewer->setSceneGraph(root);
   }

   QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));
   
   myViewer->show();
   SoQt::mainLoop();
#else
  // Inventor stuff  
  Widget myWindow = SoXt::init(argv[0]);
  MyNode::initClass();
  SoXtExaminerViewer *myViewer = 
    new SoXtExaminerViewer(myWindow);

   

  // Read a mesh
  MyMesh  mesh;
  if (argc > 1 && OpenMesh::IO::read_mesh(mesh, argv[1], opt))
  {
    if (!opt.check( OpenMesh::IO::Options::FaceNormal))
      mesh.update_face_normals();

    SoSeparator* root = new SoSeparator();
    root->addChild(new MyNode(&mesh));

    myViewer->setSceneGraph(root);
  }

  myViewer->show();
  SoXt::show(myWindow);
  SoXt::mainLoop();
#endif
}


//=============================================================================
