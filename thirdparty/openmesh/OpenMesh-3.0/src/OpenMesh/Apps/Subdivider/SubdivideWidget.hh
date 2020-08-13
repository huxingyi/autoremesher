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
  MeshViewerWidget* viewer_widget_;

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

