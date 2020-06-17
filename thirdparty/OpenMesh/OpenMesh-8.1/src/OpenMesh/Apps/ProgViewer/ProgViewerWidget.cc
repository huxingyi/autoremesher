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




//== INCLUDES =================================================================

#ifdef _MSC_VER
#  pragma warning(disable: 4267 4311)
#endif

#include <iostream>
#include <fstream>
// --------------------
#include <QApplication>
#include <QFileInfo>
#include <QKeyEvent>
// --------------------
#include <OpenMesh/Apps/ProgViewer/ProgViewerWidget.hh>
#include <OpenMesh/Core/IO/BinaryHelper.hh>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Utils/Endian.hh>
#include <OpenMesh/Tools/Utils/Timer.hh>
// --------------------

#ifdef ARCH_DARWIN
 #include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

using namespace Qt;
//== IMPLEMENTATION ========================================================== 


void 
ProgViewerWidget::open_prog_mesh(const char* _filename)
{
   MyMesh::Point  p;
   unsigned int   i, i0, i1, i2;
   unsigned int   v1, vl, vr;
   char           c[10];

   std::ifstream  ifs(_filename, std::ios::binary);
   if (!ifs)
   {
      std::cerr << "read error\n";
      exit(1);
   }

   //
   bool swap = OpenMesh::Endian::local() != OpenMesh::Endian::LSB;

   // read header
   ifs.read(c, 8); c[8] = '\0';
   if (std::string(c) != std::string("ProgMesh"))
   {
      std::cerr << "Wrong file format.\n";
      exit(1);
   }
   OpenMesh::IO::binary<size_t>::restore( ifs, n_base_vertices_,   swap );
   OpenMesh::IO::binary<size_t>::restore( ifs, n_base_faces_,      swap );
   OpenMesh::IO::binary<size_t>::restore( ifs, n_detail_vertices_, swap );

   n_max_vertices_    = n_base_vertices_ + n_detail_vertices_;

   // load base mesh
   mesh_.clear();

   for (i=0; i<n_base_vertices_; ++i)
   {
     OpenMesh::IO::binary<MyMesh::Point>::restore( ifs, p, swap );
     mesh_.add_vertex(p);
   }
  
   for (i=0; i<n_base_faces_; ++i)
   {
      OpenMesh::IO::binary<unsigned int>::restore( ifs, i0, swap);
      OpenMesh::IO::binary<unsigned int>::restore( ifs, i1, swap);
      OpenMesh::IO::binary<unsigned int>::restore( ifs, i2, swap);
      mesh_.add_face(mesh_.vertex_handle(i0), 
                     mesh_.vertex_handle(i1), 
                     mesh_.vertex_handle(i2));
   }

   
   // load progressive detail
   for (i=0; i<n_detail_vertices_; ++i)
   {
     OpenMesh::IO::binary<MyMesh::Point>::restore( ifs, p, swap );
     OpenMesh::IO::binary<unsigned int>::restore( ifs, v1, swap );
     OpenMesh::IO::binary<unsigned int>::restore( ifs, vl, swap );
     OpenMesh::IO::binary<unsigned int>::restore( ifs, vr, swap );

     PMInfo pminfo;
     pminfo.p0 = p;
     pminfo.v1 = MyMesh::VertexHandle(v1);
     pminfo.vl = MyMesh::VertexHandle(vl);
     pminfo.vr = MyMesh::VertexHandle(vr);
     pminfos_.push_back(pminfo);
   }
   pmiter_ = pminfos_.begin();
 

   // update face and vertex normals
   mesh_.update_face_normals();
   mesh_.update_vertex_normals();   

   // bounding box
   MyMesh::ConstVertexIter  
     vIt(mesh_.vertices_begin()), 
     vEnd(mesh_.vertices_end());

   MyMesh::Point bbMin, bbMax;

   bbMin = bbMax = mesh_.point(*vIt);
   for (; vIt!=vEnd; ++vIt)
   {
      bbMin.minimize(mesh_.point(*vIt));
      bbMax.maximize(mesh_.point(*vIt));
   }

   // set center and radius
   set_scene_pos(0.5f*(bbMin + bbMax), 0.5*(bbMin - bbMax).norm());

   // info
   std::cerr << mesh_.n_vertices() << " vertices, "
             << mesh_.n_edges()    << " edge, "
             << mesh_.n_faces()    << " faces, "
             << n_detail_vertices_ << " detail vertices\n";

   setWindowTitle( QFileInfo(_filename).fileName() );
}


//-----------------------------------------------------------------------------


void ProgViewerWidget::refine(unsigned int _n)
{
  size_t n_vertices = mesh_.n_vertices();

  while (n_vertices < _n && pmiter_ != pminfos_.end())
  {
    pmiter_->v0 = mesh_.add_vertex(pmiter_->p0);
    mesh_.vertex_split(pmiter_->v0,
		       pmiter_->v1,
		       pmiter_->vl,
		       pmiter_->vr);
    ++pmiter_;
    ++n_vertices;
  }
  
  mesh_.update_face_normals();
  mesh_.update_vertex_normals();

  std::cerr << n_vertices << " vertices\n";
}


//-----------------------------------------------------------------------------


void ProgViewerWidget::coarsen(unsigned int _n)
{
  size_t n_vertices = mesh_.n_vertices();

  while (n_vertices > _n && pmiter_ != pminfos_.begin())
  {
    --pmiter_;

    MyMesh::HalfedgeHandle hh = 
      mesh_.find_halfedge(pmiter_->v0, pmiter_->v1);

    mesh_.collapse(hh);

    --n_vertices;
  }

  mesh_.garbage_collection();
  mesh_.update_face_normals();
  mesh_.update_vertex_normals();

  std::cerr << n_vertices << " vertices\n";
}


//-----------------------------------------------------------------------------

void ProgViewerWidget::keyPressEvent(QKeyEvent* _event)
{
  switch (_event->key())
  {
    case Key_Minus:
      if ( _event->modifiers() & ShiftModifier)
	coarsen(mesh_.n_vertices()-1);
      else
	coarsen((unsigned int)(0.9*mesh_.n_vertices()));
      updateGL();
      break;
      
    case Key_Plus:
      if (_event->modifiers() & ShiftModifier)
	refine(mesh_.n_vertices()+1);
      else
	refine((unsigned int)(std::max( 1.1*mesh_.n_vertices(),
                                        mesh_.n_vertices()+1.0) ));
      updateGL();
      break;
      
    case Key_Home:
      coarsen(n_base_vertices_);
      updateGL();
      break;
      
    case Key_A:
      if (timer_->isActive())
      {
	timer_->stop();
	std::cout << "animation stopped!" << std::endl;
      }
       else
       {
         timer_->setSingleShot(true);
	 timer_->start(0);
	 std::cout << "animation started!" << std::endl;
       }
      break;
      
    case Key_End:
      refine(n_base_vertices_ + n_detail_vertices_);
      updateGL();
      break;

    case Key_P:
    {      
      const size_t refine_max = 100000;
      const size_t n_loop     = 5;

      OpenMesh::Utils::Timer t;
      size_t count;

      coarsen(0);         count = mesh_.n_vertices();
      refine(refine_max); count = mesh_.n_vertices() - count;

      t.start();
      for (size_t i=0; i<n_loop; ++i)
      {
        coarsen(0);
        refine(100000);
      }
      t.stop();
      
      std::cout << "# collapses/splits: " << 2*(n_loop+1)*count << " in "
                << t.as_string() << std::endl;
      std::cout << "# collapses or splits per seconds: " 
                << 2*(n_loop+1)*count/t.seconds() << "\n";

      coarsen(0);


      updateGL();

      break;
    }
    
    case Key_S:
      if (OpenMesh::IO::write_mesh( mesh_, "result.off" ))
	std::clog << "Current mesh stored in 'result.off'\n";
      break;    
      
    default:
      this->Base::keyPressEvent(_event);
  }
}

void ProgViewerWidget::animate( void )   
{
   if (animateRefinement_)
   {
      refine((unsigned int)( 1.1*(mesh_.n_vertices()+1) ));
      if ( mesh_.n_vertices() > n_base_vertices_+(0.5*n_detail_vertices_))
         animateRefinement_ = false;
   }
   else
   {
      coarsen((unsigned int)(0.9*(mesh_.n_vertices()-1)));
      if ( mesh_.n_vertices() == n_base_vertices_ )
         animateRefinement_ = true;
   }
   updateGL();
   timer_->setSingleShot(true);
   timer_->start(300);
}

//=============================================================================
