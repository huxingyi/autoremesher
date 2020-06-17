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




#ifndef OPENMESHAPPS_QGLVIEWERWIDGET_HH
#define OPENMESHAPPS_QGLVIEWERWIDGET_HH


//== INCLUDES =================================================================


#include <OpenMesh/Core/Geometry/VectorT.hh>
#include <QGLWidget>
#include <QMenu>
#include <string>
#include <vector>


//== FORWARD DECLARATIONS =====================================================


class QPopupMenu;


//== CLASS DEFINITION =========================================================

  
class QGLViewerWidget : public QGLWidget
{

  Q_OBJECT
  
public:
   
  // Default constructor.
  QGLViewerWidget( QWidget* _parent=0, const char* _name=0 );

  // Destructor.
  virtual ~QGLViewerWidget();

  /* Sets the center and size of the whole scene. 
     The _center is used as fixpoint for rotations and for adjusting
     the camera/viewer (see view_all()). */
  void set_scene_pos( const OpenMesh::Vec3f& _center, float _radius );  

  /* view the whole scene: the eye point is moved far enough from the
     center so that the whole scene is visible. */
  void view_all();

  /// add draw mode to popup menu
  void add_draw_mode(std::string _s);

  float radius() const { return radius_; }
  const OpenMesh::Vec3f& center() const { return center_; }

  const GLdouble* modelview_matrix() const  { return modelview_matrix_;  }
  const GLdouble* projection_matrix() const { return projection_matrix_; }
  void set_modelview_matrix(const GLdouble _modelview_matrix[16])
  { memcpy(modelview_matrix_, _modelview_matrix, 16*sizeof(GLdouble)); }
  void set_projection_matrix(const GLdouble _projection_matrix[16])
  { memcpy(projection_matrix_, _projection_matrix, 16*sizeof(GLdouble)); }

  float fovy() const { return 45.0f; }

protected:

  // draw the scene: will be called by the painGL() method.
  virtual void draw_scene(const std::string& _draw_mode);

  double performance(void);

private slots:  

  // popup menu clicked
  void slotPopupMenu(int _id);  
  
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

protected:
  // translate the scene and update modelview matrix
  void translate(const OpenMesh::Vec3f& _trans);

  // rotate the scene (around its center) and update modelview matrix
  void rotate(const OpenMesh::Vec3f& _axis, float _angle);

  OpenMesh::Vec3f  center_;
  float            radius_;
	      
  GLdouble    projection_matrix_[16],
              modelview_matrix_[16];


  // popup menu for draw mode selection
  QMenu*                    popup_menu_;
  unsigned int              draw_mode_;
  unsigned int              n_draw_modes_;
  std::vector<std::string>  draw_mode_names_;



  // virtual trackball: map 2D screen point to unit sphere
  bool map_to_sphere(const QPoint& _point, OpenMesh::Vec3f& _result);

  QPoint           last_point_2D_;
  OpenMesh::Vec3f  last_point_3D_;
  bool             last_point_ok_;

};


//=============================================================================
#endif // OPENMESHAPPS_QGLVIEWERWIDGET_HH
//=============================================================================

