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




#ifndef OPENMESHAPPS_PROGVIEWERWIDGET_HH
#define OPENMESHAPPS_PROGVIEWERWIDGET_HH


//== INCLUDES =================================================================

#include <QTimer>
#include <OpenMesh/Apps/QtViewer/MeshViewerWidgetT.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Mesh/Attributes.hh>
#include <string>



//== CLASS DEFINITION =========================================================


using namespace OpenMesh;
using namespace OpenMesh::Attributes;


struct ProgTraits : public OpenMesh::DefaultTraits
{
  VertexAttributes  ( OpenMesh::Attributes::Normal       |
                      OpenMesh::Attributes::Status       );
  EdgeAttributes    ( OpenMesh::Attributes::Status       );
  HalfedgeAttributes( OpenMesh::Attributes::PrevHalfedge );
  FaceAttributes    ( OpenMesh::Attributes::Normal       |
		      OpenMesh::Attributes::Status       );  
};

  
typedef OpenMesh::TriMesh_ArrayKernelT<ProgTraits>  MyMesh;
typedef MeshViewerWidgetT<MyMesh>                   MeshViewerWidgetProgBase;


//== CLASS DEFINITION =========================================================

	      

class ProgViewerWidget : public MeshViewerWidgetProgBase
{
  Q_OBJECT
   
public:

  typedef MeshViewerWidgetProgBase Base;
  typedef ProgViewerWidget This;


public:   
  /// default constructor
  explicit ProgViewerWidget(QWidget* _parent=0)
    : MeshViewerWidgetProgBase(_parent),
      n_base_vertices_(0),
      n_base_faces_(0),
      n_detail_vertices_(0),
      n_max_vertices_(0)
  {
    timer_ = new QTimer(this);

    connect( timer_, SIGNAL(timeout()), SLOT(animate()) );
  }

  /// destructor
  ~ProgViewerWidget()
  {
    delete timer_;
  }

  /// open progressive mesh
  void open_prog_mesh(const char* _filename);

protected slots:

void animate( void );
   
private:

  QTimer *timer_;
   
  struct PMInfo 
  {
    MyMesh::Point        p0;
    MyMesh::VertexHandle v0, v1, vl, vr;
  };
  typedef std::vector<PMInfo>          PMInfoContainer;
  typedef PMInfoContainer::iterator    PMInfoIter;

  /// refine mesh up to _n vertices
  void refine(unsigned int _n);

  /// coarsen mesh down to _n vertices
  void coarsen(unsigned int _n);

  virtual void keyPressEvent(QKeyEvent* _event) override;

  // mesh data
  bool              animateRefinement_;
  PMInfoContainer   pminfos_;
  PMInfoIter        pmiter_;
  size_t            n_base_vertices_, n_base_faces_, n_detail_vertices_;
  size_t            n_max_vertices_;
};


//=============================================================================
#endif // OPENMESHAPPS_PROGVIEWERWIDGET_HH defined
//=============================================================================

