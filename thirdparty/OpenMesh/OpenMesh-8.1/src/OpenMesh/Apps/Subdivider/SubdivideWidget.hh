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



//=============================================================================
//
//  CLASS SubdivideWidget
//
//=============================================================================

#ifndef SUBDIVIDEWIDGET_HH
#define SUBDIVIDEWIDGET_HH


//== INCLUDES =================================================================

#include <QTimer>
#include <QMainWindow>
#include <QStatusBar>
#include <OpenMesh/Apps/Subdivider/MeshViewerWidget.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/SubdividerT.hh>




//== CLASS DEFINITION =========================================================

class SubdivideWidget : public QWidget
{
  Q_OBJECT
  
public:

  typedef OpenMesh::Subdivider::Uniform::SubdividerT< Mesh > Subdivider;

  enum SOPType {
    SOP_UniformCompositeLoop,
    SOP_UniformCompositeSqrt3,
    SOP_UniformLoop,
    SOP_UniformSqrt3,
    SOP_UniformInterpolatingSqrt3,
    SOP_ModifiedButterfly,
    SOP_CatmullClark,
    SOP_Undefined
  };

  typedef std::map< SOPType, Subdivider* > SubdividerPool;

public:
   
  /// constructor
  SubdivideWidget(QWidget* _parent=0, const char* _name=0);

  /// destructor
  ~SubdivideWidget() {};


  /// open mesh from _filename
  bool open_mesh(const char* _filename);

  void keyPressEvent( QKeyEvent *k );

  /// Updates Status Bar Information
  QStatusBar *status_bar;
  void update();

 
private slots:

  void animate_slot(void);
  void slot_select_sop(int i);
  void subdiv_slot();
  void reset_slot();
  void save_slot();
  void load_slot();

private:

  // widgets
  MeshViewerWidgetSubdivider* viewer_widget_;

  QTimer *timer_;
   
  int animate_step_;
  int max_animate_steps_;
  int msecs_;

  // Selected and current subdivision operator
  SOPType sel_topo_type; // selected operator
  SOPType cur_topo_type; // active operator

  // 
  SubdividerPool subdivider_;

};


//=============================================================================
#endif // SUBDIVIDEWIDGET_HH defined
//=============================================================================

