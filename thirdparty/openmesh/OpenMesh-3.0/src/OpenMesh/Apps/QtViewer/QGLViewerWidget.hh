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


#ifndef OPENMESHAPPS_QGLVIEWERWIDGET_HH
#define OPENMESHAPPS_QGLVIEWERWIDGET_HH


//== INCLUDES =================================================================


#include <OpenMesh/Core/Geometry/VectorT.hh>
#include <QtOpenGL/qgl.h>
#include <string>
#include <vector>
#include <map>


//== FORWARD DECLARATIONS =====================================================

class QMenu;
class QActionGroup;
class QAction;

//== CLASS DEFINITION =========================================================

  
class QGLViewerWidget : public QGLWidget
{

  Q_OBJECT
  
public:
  typedef QGLWidget Super;
   
  // Default constructor.
  QGLViewerWidget( QWidget* _parent=0 );

  // 
  QGLViewerWidget( QGLFormat& _fmt, QWidget* _parent=0 );

  // Destructor.
  virtual ~QGLViewerWidget();

private:

  void init(void);

public:

  /* Sets the center and size of the whole scene. 
     The _center is used as fixpoint for rotations and for adjusting
     the camera/viewer (see view_all()). */
  void set_scene_pos( const OpenMesh::Vec3f& _center, float _radius );  

  /* view the whole scene: the eye point is moved far enough from the
     center so that the whole scene is visible. */
  void view_all();

  /// add draw mode to popup menu, and return the QAction created
  QAction *add_draw_mode(const std::string& _s);

  /// delete draw mode from popup menu
  void del_draw_mode(const std::string& _s);

  const std::string& current_draw_mode() const 
  { return draw_mode_ ? draw_mode_names_[draw_mode_-1] : nomode_; }

  float radius() const { return radius_; }
  const OpenMesh::Vec3f& center() const { return center_; }

  const GLdouble* modelview_matrix() const  { return modelview_matrix_;  }
  const GLdouble* projection_matrix() const { return projection_matrix_; }

  float fovy() const { return 45.0f; }

  QAction* findAction(const char *name);
  void addAction(QAction* action, const char* name);
  void removeAction(const char* name);
  void removeAction(QAction* action);

protected:

  // draw the scene: will be called by the painGL() method.
  virtual void draw_scene(const std::string& _draw_mode);

  double performance(void);
  
  void setDefaultMaterial(void);
  void setDefaultLight(void);

private slots:  

  // popup menu clicked
  void slotDrawMode(QAction *_mode);  
  void slotSnapshot( void );

  
private: // inherited

  // initialize OpenGL states (triggered by Qt)
  void initializeGL();

  // draw the scene (triggered by Qt)
  void paintGL();

  // handle resize events (triggered by Qt)
  void resizeGL( int w, int h );

protected:
   
  // Qt mouse events
  virtual void mousePressEvent( QMouseEvent* );
  virtual void mouseReleaseEvent( QMouseEvent* );
  virtual void mouseMoveEvent( QMouseEvent* );
  virtual void wheelEvent( QWheelEvent* );
  virtual void keyPressEvent( QKeyEvent* );

private:
   
  // updates projection matrix
  void update_projection_matrix();

  // translate the scene and update modelview matrix
  void translate(const OpenMesh::Vec3f& _trans);

  // rotate the scene (around its center) and update modelview matrix
  void rotate(const OpenMesh::Vec3f& _axis, float _angle);

  OpenMesh::Vec3f  center_;
  float            radius_;
	      
  GLdouble    projection_matrix_[16],
              modelview_matrix_[16];


  // popup menu for draw mode selection
  QMenu*               popup_menu_;
  QActionGroup*        draw_modes_group_;
  typedef std::map<QString,QAction*> ActionMap;
  ActionMap            names_to_actions;
  unsigned int              draw_mode_;
  unsigned int              n_draw_modes_;
  std::vector<std::string>  draw_mode_names_;
  static std::string        nomode_;



  // virtual trackball: map 2D screen point to unit sphere
  bool map_to_sphere(const QPoint& _point, OpenMesh::Vec3f& _result);

  QPoint           last_point_2D_;
  OpenMesh::Vec3f  last_point_3D_;
  bool             last_point_ok_;

};


//=============================================================================
#endif // OPENMESHAPPS_QGLVIEWERWIDGET_HH
//=============================================================================

