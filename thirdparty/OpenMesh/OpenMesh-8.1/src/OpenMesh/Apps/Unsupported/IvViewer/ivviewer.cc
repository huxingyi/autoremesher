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
