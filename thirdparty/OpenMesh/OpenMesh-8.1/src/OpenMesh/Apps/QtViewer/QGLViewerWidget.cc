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
#include <iostream>
#include <sstream>
#include <algorithm>
// --------------------

// --------------------
#include <QApplication>
#include <QMenu>
#include <QCursor>
#include <QImage>
#include <QDateTime>
#include <QMouseEvent>
// --------------------
#include <OpenMesh/Apps/QtViewer/QGLViewerWidget.hh>
#include <OpenMesh/Tools/Utils/Timer.hh>

#if !defined(M_PI)
#  define M_PI 3.1415926535897932
#endif

const double TRACKBALL_RADIUS = 0.6;


using namespace Qt;
using namespace OpenMesh;


//== IMPLEMENTATION ========================================================== 

std::string QGLViewerWidget::nomode_ = "";

//----------------------------------------------------------------------------

QGLViewerWidget::QGLViewerWidget( QWidget* _parent )
  : QGLWidget( _parent )
{    
  init();
}

//----------------------------------------------------------------------------

QGLViewerWidget::
QGLViewerWidget( QGLFormat& _fmt, QWidget* _parent )
  : QGLWidget( _fmt, _parent )
{
  init();
}


//----------------------------------------------------------------------------

void 
QGLViewerWidget::init(void)
{
  // qt stuff
  setAttribute(Qt::WA_NoSystemBackground, true);
  setFocusPolicy(Qt::StrongFocus);
  setAcceptDrops( true );  
  setCursor(PointingHandCursor);


  // popup menu

  popup_menu_ = new QMenu(this);
  draw_modes_group_ = new QActionGroup(this);

  connect( draw_modes_group_, SIGNAL(triggered(QAction*)),
	   this, SLOT(slotDrawMode(QAction*)));


  // init draw modes
  n_draw_modes_ = 0;
  //draw_mode_ = 3;
  QAction *a;
  a = add_draw_mode("Wireframe");
  a->setShortcut(QKeySequence(Key_W));
  add_draw_mode("Solid Flat");
  a = add_draw_mode("Solid Smooth");
  a->setShortcut(QKeySequence(Key_S));
  a->setChecked(true);

  slotDrawMode(a);
}


//----------------------------------------------------------------------------

QGLViewerWidget::~QGLViewerWidget()
{
}


//----------------------------------------------------------------------------

void 
QGLViewerWidget::setDefaultMaterial(void)
{
  GLfloat mat_a[] = {0.1f, 0.1f, 0.1f, 1.0f};
  GLfloat mat_d[] = {0.7f, 0.7f, 0.5f, 1.0f};
  GLfloat mat_s[] = {1.0f, 1.0f, 1.0f, 1.0f};
  GLfloat shine[] = {120.0f};
  
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   mat_a);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   mat_d);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  mat_s);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);
}


//----------------------------------------------------------------------------

void
QGLViewerWidget::setDefaultLight(void)
{
  GLfloat pos1[] = { 0.1f,  0.1f, -0.02f, 0.0f};
  GLfloat pos2[] = {-0.1f,  0.1f, -0.02f, 0.0f};
  GLfloat pos3[] = { 0.0f,  0.0f,  0.1f,  0.0f};
  GLfloat col1[] = { 0.7f,  0.7f,  0.8f,  1.0f};
  GLfloat col2[] = { 0.8f,  0.7f,  0.7f,  1.0f};
  GLfloat col3[] = { 1.0f,  1.0f,  1.0f,  1.0f};
 
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
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::initializeGL()
{  
  // OpenGL state
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glDisable( GL_DITHER );
  glEnable( GL_DEPTH_TEST );

  // Material
  setDefaultMaterial();
  
  // Lighting
  glLoadIdentity();
  setDefaultLight();  
  
  // Fog
  GLfloat fogColor[4] = { 0.3f, 0.3f, 0.4f, 1.0f };
  glFogi(GL_FOG_MODE,    GL_LINEAR);
  glFogfv(GL_FOG_COLOR,  fogColor);
  glFogf(GL_FOG_DENSITY, 0.35f);
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
   // glutWireTeapot(0.5);
  }

  else if (_draw_mode == "Solid Flat")
  {
    glEnable(GL_LIGHTING);
    glShadeModel(GL_FLAT);
    //glutSolidTeapot(0.5);
  }

  else if (_draw_mode == "Solid Smooth")
  {
    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    //glutSolidTeapot(0.5);
  }
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::mousePressEvent( QMouseEvent* _event )
{
  // popup menu
  if (_event->button() == RightButton && _event->buttons()== RightButton )
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
  if ( (_event->buttons() == (LeftButton+MidButton)) ||
	  (_event->buttons() == LeftButton && _event->modifiers() == ControlModifier))
  {
    float value_y = radius_ * dy * 3.0 / h;
    translate(Vec3f(0.0, 0.0, value_y));
  }
	

  // move in x,y direction
  else if ( (_event->buttons() == MidButton) ||
			(_event->buttons() == LeftButton && _event->modifiers() == AltModifier) )
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
  else if (_event->buttons() == LeftButton) {

    if (last_point_ok_) {
      if ((newPoint_hitSphere = map_to_sphere(newPoint2D, newPoint3D))) {
        Vec3f axis = last_point_3D_ % newPoint3D;
        if (axis.sqrnorm() < 1e-7) {
          axis = Vec3f(1, 0, 0);
        } else {
          axis.normalize();
        }
        // find the amount of rotation
        Vec3f d = last_point_3D_ - newPoint3D;
        float t = 0.5 * d.norm() / TRACKBALL_RADIUS;
        if (t < -1.0)
          t = -1.0;
        else if (t > 1.0)
          t = 1.0;
        float phi = 2.0 * asin(t);
        float angle = phi * 180.0 / M_PI;
        rotate(axis, angle);
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
    case Key_Print:
      slotSnapshot();
      break;

    case Key_H:
      std::cout << "Keys:\n";
      std::cout << "  Print\tMake snapshot\n";
      std::cout << "  C\tenable/disable back face culling\n";
      std::cout << "  F\tenable/disable fog\n";
      std::cout << "  I\tDisplay information\n";
      std::cout << "  N\tenable/disable display of vertex normals\n";
      std::cout << "  Shift N\tenable/disable display of face normals\n";
      std::cout << "  Shift P\tperformance check\n";
      break;

    case Key_C:
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

    case Key_F:
      if ( glIsEnabled( GL_FOG ) )
      {
	glDisable( GL_FOG );
	std::cout << "Fog: disabled\n";
      }
      else
      {
	glEnable( GL_FOG );
	std::cout << "Fog: enabled\n";
      }
      updateGL();
      break;

    case Key_I:
      std::cout << "Scene radius: " << radius_ << std::endl;
      std::cout << "Scene center: " << center_ << std::endl;
      break;

    case Key_P:
      if (_event->modifiers() & ShiftModifier)
      {
        double fps = performance();      
        std::cout << "fps: " 
                  << std::setiosflags (std::ios_base::fixed)
                  << fps << std::endl;
      }
    break;
    
    case Key_Q:
    case Key_Escape:
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
    // This is actually doing the Sphere/Hyperbolic sheet hybrid thing,
    // based on Ken Shoemake's ArcBall in Graphics Gems IV, 1993.
    double x =  (2.0*_v2D.x() - width())/width();
    double y = -(2.0*_v2D.y() - height())/height();
    double xval = x;
    double yval = y;
    double x2y2 = xval*xval + yval*yval;

    const double rsqr = TRACKBALL_RADIUS*TRACKBALL_RADIUS;
    _v3D[0] = xval;
    _v3D[1] = yval;
    if (x2y2 < 0.5*rsqr) {
        _v3D[2] = sqrt(rsqr - x2y2);
    } else {
        _v3D[2] = 0.5*rsqr/sqrt(x2y2);
    }
    
    return true;
  }


//----------------------------------------------------------------------------


void
QGLViewerWidget::update_projection_matrix()
{
  makeCurrent();
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();

  const double fovY   = 45.0;
  const double aspect = static_cast<double>(width()) /  static_cast<double>(height());
  const double zNear  = 0.01*radius_;
  const double zFar   = 100.0*radius_;

//  Replacement for: gluPerspective(45.0, (GLfloat) width() / (GLfloat) height(), 0.01*radius_, 100.0*radius_);
  const double pi = 3.1415926535897932384626433832795;
  const double fH = tan( fovY / 360 * pi ) * zNear;
  const double fW = fH * aspect;
  glFrustum( -fW, fW, -fH, fH, zNear, zFar );


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
  glFogf( GL_FOG_START,      1.5*_radius );
  glFogf( GL_FOG_END,        3.0*_radius );

  update_projection_matrix();
  view_all();
}


//----------------------------------------------------------------------------


QAction*
QGLViewerWidget::add_draw_mode(const std::string& _s)
{
  ++n_draw_modes_;
  draw_mode_names_.push_back(_s);

  QActionGroup *grp = draw_modes_group_;
  QAction* act = new QAction(tr(_s.c_str()), this);
  act->setCheckable(true);
  act->setData(n_draw_modes_);

  grp->addAction(act);
  popup_menu_->addAction(act);
  addAction(act, _s.c_str());
  
  return act;
}

void QGLViewerWidget::addAction(QAction* act, const char * name)
{
    names_to_actions[name] = act;
    Super::addAction(act);
}
void QGLViewerWidget::removeAction(QAction* act)
{
    ActionMap::iterator it = names_to_actions.begin(), e = names_to_actions.end();
    ActionMap::iterator found = e;
    for(; it!=e; ++it) {
        if (it->second == act) {
            found = it;
            break;
        }
    }
    if (found != e) {
        names_to_actions.erase(found);
}
    popup_menu_->removeAction(act);
    draw_modes_group_->removeAction(act);
    Super::removeAction(act);
}

void QGLViewerWidget::removeAction(const char* name)
{
    QString namestr = QString(name);
    ActionMap::iterator e = names_to_actions.end();

    ActionMap::iterator found = names_to_actions.find(namestr);
    if (found != e) {
        removeAction(found->second);
    }
}

QAction* QGLViewerWidget::findAction(const char* name)
{
    QString namestr = QString(name);
    ActionMap::iterator e = names_to_actions.end();

    ActionMap::iterator found = names_to_actions.find(namestr);
    if (found != e) {
        return found->second;
    }
    return 0;
}

//----------------------------------------------------------------------------


void 
QGLViewerWidget::del_draw_mode(const std::string& _s)
{
    QString cmp = _s.c_str();
    QList<QAction*> actions_ = popup_menu_->actions();
    QList<QAction*>::iterator it=actions_.begin(), e=actions_.end();
    for(; it!=e; ++it) {
        if ((*it)->text() == cmp) { break; }
    }
  
#if _DEBUG
  assert( it != e );
#else
  if ( it == e )
    return;
#endif

  popup_menu_->removeAction(*it);
  //QActionGroup *grp = draw_modes_group_;

}


//----------------------------------------------------------------------------


void
QGLViewerWidget::slotDrawMode(QAction* _mode)
{
  // save draw mode
  draw_mode_ = _mode->data().toInt();
  updateGL();

  // check selected draw mode
  //popup_menu_->setItemChecked(draw_mode_, true);
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
  timer.stop();

  qApp->processEvents();

  timer.cont();
  for (i=0, axis=Vec3f(0,1,0); i<frames; ++i)
  { rotate(axis, angle); paintGL(); swapBuffers(); }
  timer.stop();

  qApp->processEvents();

  timer.cont();
  for (i=0, axis=Vec3f(0,0,1); i<frames; ++i)
  { rotate(axis, angle); paintGL(); swapBuffers(); }
  timer.stop();

  glFinish();
  timer.stop();

  glPopMatrix();
  updateGL();

  fps = ( (3.0 * frames) / timer.seconds() );

  setCursor( PointingHandCursor );

  return fps;
}


void
QGLViewerWidget::slotSnapshot( void )
{  
  QImage image;
  size_t w(width()), h(height());
  GLenum buffer( GL_BACK );

  try
  {
    image = QImage(w, h, QImage::Format_RGB32);
  
    std::vector<GLubyte> fbuffer(3*w*h);

    qApp->processEvents();
    makeCurrent();
    updateGL();
    glFinish();
    
    glReadBuffer( buffer );
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    paintGL();
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, &fbuffer[0] );
    
    unsigned int x,y,offset;
    
    for (y=0; y<h; ++y) {
      for (x=0; x<w; ++x) {
        offset = 3*(y*w + x);
        image.setPixel(x, h-y-1, qRgb(fbuffer[offset],
                                      fbuffer[offset+1],
                                      fbuffer[offset+2]));
      }
    }
    
    
    QString name = "snapshot-";
#if defined(_MSC_VER)
    {
      std::stringstream s;
      QDateTime         dt = QDateTime::currentDateTime();
      s << dt.date().year() 
        << std::setw(2) << std::setfill('0') << dt.date().month() 
        << std::setw(2) << std::setfill('0') << dt.date().day()
        << std::setw(2) << std::setfill('0') << dt.time().hour()
        << std::setw(2) << std::setfill('0') << dt.time().minute()
        << std::setw(2) << std::setfill('0') << dt.time().second();
      name += QString(s.str().c_str());
    }
#else
    name += QDateTime::currentDateTime().toString( "yyMMddhhmmss" );
#endif
    name += ".png";

    image.save( name, "PNG");
  }
  catch( std::bad_alloc& )
  {
    qWarning("Mem Alloc Error");
  }
  
}



//=============================================================================
