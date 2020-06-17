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
#  pragma warning(disable: 4267 4311 4305)
#endif

#include <iomanip>

#include <GL/glut.h>
// #include <qnamespace.h>
#include <QApplication>
#include <QMenu>
#include <QCursor>
#include <QMouseEvent>

#include <OpenMesh/Apps/VDProgMesh/Streaming/Client/QGLViewerWidget.hh>
#include <OpenMesh/Tools/Utils/Timer.hh>

#if !defined(M_PI)
#  define M_PI 3.1415926535897931
#endif

using namespace OpenMesh;


//== IMPLEMENTATION ========================================================== 


QGLViewerWidget::QGLViewerWidget( QWidget* _parent, const char* _name )
  : QGLWidget( _parent )
{    
  // qt stuff
  setWindowTitle(_name);
//   setBackgroundMode( NoBackground ); 
  setFocusPolicy(Qt::StrongFocus);
  setAcceptDrops( true );  
  setCursor(Qt::PointingHandCursor);


  // popup menu
  popup_menu_ = new QMenu("Draw Mode Menu", this);

  connect( popup_menu_, SIGNAL(activated(int)), 
	   this, SLOT(slotPopupMenu(int)));    


  // init draw modes
  n_draw_modes_ = 0;
  add_draw_mode("Wireframe");
  add_draw_mode("Solid Flat");
  add_draw_mode("Solid Smooth");

  // for example
  add_draw_mode("Colored");

  slotPopupMenu(2);
}


//----------------------------------------------------------------------------


QGLViewerWidget::~QGLViewerWidget()
{
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::initializeGL()
{  
  // OpenGL state
  glClearColor(1.0, 1.0, 1.0, 0.0);
  glDisable( GL_DITHER );
  glEnable( GL_DEPTH_TEST );
  glEnable( GL_CULL_FACE );


  // material
   GLfloat mat_a[] = {0.7, 0.6, 0.5, 1.0};
   GLfloat mat_d[] = {0.8, 0.7, 0.6, 1.0};
   GLfloat mat_s[] = {1.0, 1.0, 1.0, 1.0};
   GLfloat shine[] = {120.0};
   
//   GLfloat mat_a[] = {0.2, 0.2, 0.2, 1.0};
//   GLfloat mat_d[] = {0.4, 0.4, 0.4, 1.0};
//   GLfloat mat_s[] = {0.8, 0.8, 0.8, 1.0};
//   GLfloat shine[] = {128.0};
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   mat_a);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   mat_d);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  mat_s);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);

  
  // lighting
  glLoadIdentity();
  
  GLfloat pos1[] = { 0.1, 0.1, -0.02, 0.0};
  GLfloat pos2[] = {-0.1, 0.1, -0.02, 0.0};
  GLfloat pos3[] = { 0.0, 0.0, 0.1, 0.0};
  GLfloat col1[] = {.05, .05, .4, 1.0};
  GLfloat col2[] = {.4, .05, .05, 1.0};
  GLfloat col3[] = {1.0, 1.0, 1.0, 1.0};

  glEnable(GL_LIGHT0);    
  glLightfv(GL_LIGHT0,GL_POSITION, pos1);
  glLightfv(GL_LIGHT0,GL_DIFFUSE,  col1);
  glLightfv(GL_LIGHT0,GL_SPECULAR, col1);
  
  glEnable(GL_LIGHT1);  
  glLightfv(GL_LIGHT1,GL_POSITION, pos2);
  glLightfv(GL_LIGHT1,GL_DIFFUSE,  col2);
  glLightfv(GL_LIGHT1,GL_SPECULAR, col2);
  
  glEnable(GL_LIGHT2);  
  glLightfv(GL_LIGHT2,GL_POSITION, pos3);
  glLightfv(GL_LIGHT2,GL_DIFFUSE,  col3);
  glLightfv(GL_LIGHT2,GL_SPECULAR, col3);
  
  
  // Fog
  GLfloat fogColor[4] = { 0.4, 0.4, 0.5, 1.0 };
  glFogi(GL_FOG_MODE,    GL_LINEAR);
  glFogfv(GL_FOG_COLOR,  fogColor);
  glFogf(GL_FOG_DENSITY, 0.35);
  glHint(GL_FOG_HINT,    GL_DONT_CARE);
  glFogf(GL_FOG_START,    5.0f);
  glFogf(GL_FOG_END,     25.0f);

  // scene pos and size
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix_);
  set_scene_pos(Vec3f(0.0, 0.0, 0.0), 1.0);
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::resizeGL( int _w, int _h )
{
  update_projection_matrix();
  glViewport(0, 0, _w, _h);
  updateGL();
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::paintGL()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode( GL_PROJECTION );
  glLoadMatrixd( projection_matrix_ );
  glMatrixMode( GL_MODELVIEW );
  glLoadMatrixd( modelview_matrix_ );

  if (draw_mode_)
  {
    assert(draw_mode_ <= n_draw_modes_);
    draw_scene(draw_mode_names_[draw_mode_-1]);
  }
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::draw_scene(const std::string& _draw_mode)
{  
  if (_draw_mode == "Wireframe")
  {
    glDisable(GL_LIGHTING);
    glutWireTeapot(0.5);
  }

  else if (_draw_mode == "Solid Flat")
  {
    glEnable(GL_LIGHTING);
    glShadeModel(GL_FLAT);
    glutSolidTeapot(0.5);
  }

  else if (_draw_mode == "Solid Smooth")
  {
    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    glutSolidTeapot(0.5);
  }

}


//----------------------------------------------------------------------------


void
QGLViewerWidget::mousePressEvent( QMouseEvent* _event )
{
  // popup menu
  if (_event->button() == Qt::RightButton)
  {
    popup_menu_->exec(QCursor::pos());
  }

  else 
  {
    last_point_ok_ = map_to_sphere( last_point_2D_=_event->pos(),
				    last_point_3D_ );
  }
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::mouseMoveEvent( QMouseEvent* _event )
{  
  QPoint newPoint2D = _event->pos(); 
  
  if ( (newPoint2D.x()<0) || (newPoint2D.x()>width()) ||
       (newPoint2D.y()<0) || (newPoint2D.y()>height()) ) return;


  // Left button: rotate around center_
  // Middle button: translate object
  // Left & middle button: zoom in/out

  Vec3f  newPoint3D;
  bool   newPoint_hitSphere = map_to_sphere( newPoint2D, newPoint3D );

  float dx = newPoint2D.x() - last_point_2D_.x();
  float dy = newPoint2D.y() - last_point_2D_.y();
  
  float w  = width();
  float h  = height();



  // enable GL context
  makeCurrent();

  
  // move in z direction
  if ( (_event->button() == Qt::LeftButton) && (_event->button() == Qt::MidButton)) 
  {
    float value_y = radius_ * dy * 3.0 / h;
    translate(Vec3f(0.0, 0.0, value_y));
  }
	

  // move in x,y direction
  else if (_event->button() == Qt::MidButton) 
  {
    float z = - (modelview_matrix_[ 2]*center_[0] + 
		 modelview_matrix_[ 6]*center_[1] + 
		 modelview_matrix_[10]*center_[2] + 
		 modelview_matrix_[14]) /
                (modelview_matrix_[ 3]*center_[0] + 
	         modelview_matrix_[ 7]*center_[1] + 
	         modelview_matrix_[11]*center_[2] + 
	         modelview_matrix_[15]);

    float aspect     = w / h;
    float near_plane = 0.01 * radius_;
    float top        = tan(fovy()/2.0f*M_PI/180.0f) * near_plane;
    float right      = aspect*top;

    translate(Vec3f( 2.0*dx/w*right/near_plane*z, 
		    -2.0*dy/h*top/near_plane*z, 
		     0.0f));
  }


	
  // rotate
  else if (_event->button() == Qt::LeftButton) 
  {
    if (last_point_ok_) 
    {
      if ( (newPoint_hitSphere = map_to_sphere(newPoint2D, newPoint3D)) ) 
      {
        Vec3f axis = last_point_3D_ % newPoint3D;
        float cos_angle = (last_point_3D_ | newPoint3D);
        if ( fabs(cos_angle) < 1.0 ) 
        {
          float angle = 2.0 * acos( cos_angle ) * 180.0 / M_PI;
          rotate( axis, angle );
        }
      }
    }
  }


  // remember this point
  last_point_2D_ = newPoint2D;
  last_point_3D_ = newPoint3D;
  last_point_ok_ = newPoint_hitSphere;

  // trigger redraw
  updateGL();
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::mouseReleaseEvent( QMouseEvent* /* _event */ )
{  
   last_point_ok_ = false;
}


//-----------------------------------------------------------------------------


void QGLViewerWidget::wheelEvent(QWheelEvent* _event)
{
  // Use the mouse wheel to zoom in/out

  float d = -(float)_event->delta() / 120.0 * 0.2 * radius_;
  translate(Vec3f(0.0, 0.0, d));
  updateGL();
  _event->accept();
}


//----------------------------------------------------------------------------


void QGLViewerWidget::keyPressEvent( QKeyEvent* _event)
{
  switch( _event->key() )
  {
    case Qt::Key_C:
      if ( glIsEnabled( GL_CULL_FACE ) )
      {
	glDisable( GL_CULL_FACE );
	std::cout << "Back face culling: disabled\n";
      }
      else
      {
	glEnable( GL_CULL_FACE );
	std::cout << "Back face culling: enabled\n";
      }
      updateGL();
      break;

    case Qt::Key_I:
      std::cout << "Radius: " << radius_ << std::endl;
      std::cout << "Center: " << center_ << std::endl;
      break;

    case Qt::Key_Space:
    case Qt::Key_M:
    {
      double fps = performance();      
      std::cout << "fps: " 
#if defined(OM_CC_GCC) && (OM_CC_VERSION < 30000)
		<< std::setiosflags (std::ios::fixed)
#else
		<< std::setiosflags (std::ios_base::fixed)
#endif
		<< fps << std::endl;
    }
    break;
    
    case Qt::Key_Q:
    case Qt::Key_Escape:
      qApp->quit();      
  }
  _event->ignore();
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::translate( const OpenMesh::Vec3f& _trans )
{
  // Translate the object by _trans
  // Update modelview_matrix_
  makeCurrent();
  glLoadIdentity();
  glTranslated( _trans[0], _trans[1], _trans[2] );
  glMultMatrixd( modelview_matrix_ );
  glGetDoublev( GL_MODELVIEW_MATRIX, modelview_matrix_);
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::rotate( const OpenMesh::Vec3f& _axis, float _angle )
{
  // Rotate around center center_, axis _axis, by angle _angle
  // Update modelview_matrix_

  Vec3f t( modelview_matrix_[0]*center_[0] + 
	   modelview_matrix_[4]*center_[1] +
	   modelview_matrix_[8]*center_[2] + 
	   modelview_matrix_[12],
	   modelview_matrix_[1]*center_[0] + 
	   modelview_matrix_[5]*center_[1] +
	   modelview_matrix_[9]*center_[2] + 
	   modelview_matrix_[13],
	   modelview_matrix_[2]*center_[0] + 
	   modelview_matrix_[6]*center_[1] +
	   modelview_matrix_[10]*center_[2] + 
	   modelview_matrix_[14] );
  
  makeCurrent();
  glLoadIdentity();
  glTranslatef(t[0], t[1], t[2]);
  glRotated( _angle, _axis[0], _axis[1], _axis[2]);
  glTranslatef(-t[0], -t[1], -t[2]); 
  glMultMatrixd(modelview_matrix_);
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix_);
}


//----------------------------------------------------------------------------


bool
QGLViewerWidget::map_to_sphere( const QPoint& _v2D, OpenMesh::Vec3f& _v3D )
{
  if ( (_v2D.x() >= 0) && (_v2D.x() <= width()) &&
       (_v2D.y() >= 0) && (_v2D.y() <= height()) ) 
  {
    double x  = (double)(_v2D.x() - 0.5*width())  / (double)width();
    double y  = (double)(0.5*height() - _v2D.y()) / (double)height();
    double sinx         = sin(M_PI * x * 0.5);
    double siny         = sin(M_PI * y * 0.5);
    double sinx2siny2   = sinx * sinx + siny * siny;
    
    _v3D[0] = sinx;
    _v3D[1] = siny;
    _v3D[2] = sinx2siny2 < 1.0 ? sqrt(1.0 - sinx2siny2) : 0.0;
    
    return true;
  }
  else return false;
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::update_projection_matrix()
{
  makeCurrent();
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluPerspective(45.0, (GLfloat) width() / (GLfloat) height(),
		 0.01*radius_, 100.0*radius_);
  glGetDoublev( GL_PROJECTION_MATRIX, projection_matrix_);
  glMatrixMode( GL_MODELVIEW );
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::view_all()
{  
  translate( Vec3f( -(modelview_matrix_[0]*center_[0] + 
		      modelview_matrix_[4]*center_[1] +
		      modelview_matrix_[8]*center_[2] + 
		      modelview_matrix_[12]),
		    -(modelview_matrix_[1]*center_[0] + 
		      modelview_matrix_[5]*center_[1] +
		      modelview_matrix_[9]*center_[2] + 
		      modelview_matrix_[13]),
		    -(modelview_matrix_[2]*center_[0] + 
		      modelview_matrix_[6]*center_[1] +
		      modelview_matrix_[10]*center_[2] + 
		      modelview_matrix_[14] +
		      3.0*radius_) ) );
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::set_scene_pos( const OpenMesh::Vec3f& _cog, float _radius )
{
  center_ = _cog;
  radius_ = _radius;
  glFogf( GL_FOG_START,      _radius );
  glFogf( GL_FOG_END,    4.0*_radius );

  update_projection_matrix();
  view_all();
}


//----------------------------------------------------------------------------


void 
QGLViewerWidget::add_draw_mode(std::string _s)
{
  ++n_draw_modes_;

  // insert in popup menu
  QString str( _s.c_str() );
  popup_menu_->addAction( str );

  // store draw mode
  draw_mode_names_.push_back(_s);
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::slotPopupMenu(int _id)
{
  // set check status
  for (int i=0; i < popup_menu_->actions().count(); ++i)
    popup_menu_->actions()[i]->setChecked( i == _id );

  // save draw mode
  draw_mode_ = _id;
}


//----------------------------------------------------------------------------


double 
QGLViewerWidget::performance()
{
  setCursor( Qt::WaitCursor );

  double fps(0.0);

  makeCurrent();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  OpenMesh::Utils::Timer timer;

  unsigned int  frames = 60;
  const float   angle  = 360.0/(float)frames;
  unsigned int  i;
  Vec3f         axis;

  glFinish();
  timer.start();

  for (i=0, axis=Vec3f(1,0,0); i<frames; ++i)
  { rotate(axis, angle); paintGL(); swapBuffers(); }
  for (i=0, axis=Vec3f(0,1,0); i<frames; ++i)
  { rotate(axis, angle); paintGL(); swapBuffers(); }
  for (i=0, axis=Vec3f(0,0,1); i<frames; ++i)
  { rotate(axis, angle); paintGL(); swapBuffers(); }

  glFinish();
  timer.stop();

  glPopMatrix();
  updateGL();

  fps = ( (3.0 * frames) / timer.seconds() );

  setCursor( Qt::PointingHandCursor );

  return fps;
}

//=============================================================================
