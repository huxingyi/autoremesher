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
//  CLASS SubdivideWidget - IMPLEMENTATION
//
//=============================================================================

#ifndef SUBDIVIDEWIDGET_CC
#define SUBDIVIDEWIDGET_CC

//== INCLUDES =================================================================


// Qt
#include <QApplication>
#include <QFileDialog>
#include <QButtonGroup>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QMessageBox>

// OpenMesh
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyConnectivity.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/CompositeLoopT.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/CompositeSqrt3T.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/LoopT.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/Sqrt3T.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/Sqrt3InterpolatingSubdividerLabsikGreinerT.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/ModifiedButterFlyT.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/CatmullClarkT.hh>

#include <OpenMesh/Apps/Subdivider/SubdivideWidget.hh>


using namespace OpenMesh::Subdivider;

//== IMPLEMENTATION ==========================================================


SubdivideWidget::
SubdivideWidget(QWidget* _parent, const char* _name)
  : QWidget(_parent),
    timer_(NULL), animate_step_(0), max_animate_steps_(4), msecs_(0)
{

  setWindowTitle( QString(_name) );

  QVBoxLayout* vbox = new QVBoxLayout();

  cur_topo_type = SOP_Undefined;
  // sel_topo_type will be set when adding the radio button.;

  // examiner widget
  viewer_widget_ = new MeshViewerWidget();

  vbox->addWidget(viewer_widget_);

  QHBoxLayout* hbox = new QHBoxLayout();
//   hbox->setFixedSize(400, 30);
  vbox->addLayout(hbox);

  // insert subdivision pushbutton
  QPushButton* subdiv_button = new QPushButton( "Subdivide");
  subdiv_button->setMinimumWidth(50);
  QObject::connect( subdiv_button, SIGNAL( clicked() ),
                    this, SLOT( subdiv_slot() ) );
  hbox->addWidget(subdiv_button);

  // insert load pushbutton
  QPushButton* load_button = new QPushButton( "Load Mesh");
  load_button->setMinimumWidth(50);
  QObject::connect( load_button, SIGNAL( clicked() ),
                    this, SLOT( load_slot() ) );
  hbox->addWidget(load_button);

  // insert save pushbutton
  QPushButton* save_button = new QPushButton( "Save Mesh");
  save_button->setMinimumWidth(50);
  QObject::connect( save_button, SIGNAL( clicked() ),
                    this, SLOT( save_slot() ) );
  hbox->addWidget(save_button);


  // insert reset pushbutton
  QPushButton* reset_button = new QPushButton( "Reset");
  reset_button->setMinimumWidth(50);
  QObject::connect( reset_button, SIGNAL( clicked() ),
                    this, SLOT( reset_slot() ) );
  hbox->addWidget(reset_button);


  // Create an exclusive button group: Topology Operators
//   QButtonGroup *bgrp1 = new QButtonGroup( 1, QGroupBox::Vertical,"Subdivision Operators:");
  
  QButtonGroup* buttonGroup = new QButtonGroup();

  buttonGroup->setExclusive( TRUE );

  // insert 2 radiobuttons
  QRadioButton* radio1 = new QRadioButton( "Comp. Loop" );
  QRadioButton* radio2 = new QRadioButton( "Comp. SQRT(3)" );
  QRadioButton* radio3 = new QRadioButton( "Loop" );
  QRadioButton* radio4 = new QRadioButton( "Sqrt(3)" );
  QRadioButton* radio5 = new QRadioButton( "Interpolating Sqrt3" );
  QRadioButton* radio6 = new QRadioButton( "Modified Butterfly" );
  // QRadioButton* radio7 = new QRadioButton( "Catmull Clark" ); // Disabled, as it needs a quad mesh!
  radio3->setChecked( TRUE );
  sel_topo_type = SOP_UniformLoop;

  buttonGroup->addButton(radio1, SOP_UniformCompositeLoop);
  buttonGroup->addButton(radio2, SOP_UniformCompositeSqrt3);
  buttonGroup->addButton(radio3, SOP_UniformLoop);
  buttonGroup->addButton(radio4, SOP_UniformSqrt3);
  buttonGroup->addButton(radio5, SOP_UniformInterpolatingSqrt3);
  buttonGroup->addButton(radio6, SOP_ModifiedButterfly);
  //buttonGroup->addButton(radio7, SOP_CatmullClark);

  vbox->addWidget(radio1);
  vbox->addWidget(radio2);
  vbox->addWidget(radio3);
  vbox->addWidget(radio4);
  vbox->addWidget(radio5);
  vbox->addWidget(radio6);
  // vbox->addWidget(radio7);

  QObject::connect( buttonGroup, SIGNAL( buttonPressed(int) ),
                          this, SLOT( slot_select_sop(int) ) );


  status_bar = new QStatusBar();
  status_bar->setFixedHeight(20);
  status_bar->showMessage("0 Faces, 0 Edges, 0 Vertices");
  vbox->addWidget(status_bar);


  setLayout(vbox);


  // animation
  timer_ = new QTimer(this);
  connect( timer_, SIGNAL( timeout() ), this, SLOT( animate_slot() ) );

  // --------------------

  subdivider_[SOP_UniformCompositeLoop]      = new Uniform::CompositeLoopT<Mesh>;
  subdivider_[SOP_UniformCompositeSqrt3]     = new Uniform::CompositeSqrt3T<Mesh>;
  subdivider_[SOP_UniformLoop]               = new Uniform::LoopT<Mesh>;
  subdivider_[SOP_UniformSqrt3]              = new Uniform::Sqrt3T<Mesh>;
  subdivider_[SOP_UniformInterpolatingSqrt3] = new Uniform::InterpolatingSqrt3LGT< Mesh >;
  subdivider_[SOP_ModifiedButterfly]         = new Uniform::ModifiedButterflyT<Mesh>;
  subdivider_[SOP_CatmullClark]              = new Uniform::CatmullClarkT<Mesh>;

}


//-----------------------------------------------------------------------------

void SubdivideWidget::slot_select_sop(int i)
{
  switch(i)
  {
    case SOP_UniformCompositeLoop:
    case SOP_UniformCompositeSqrt3:
    case SOP_UniformLoop:
    case SOP_UniformSqrt3:
    case SOP_UniformInterpolatingSqrt3:
    case SOP_ModifiedButterfly:
    case SOP_CatmullClark:          sel_topo_type = (SOPType)i; break;
    default:                        sel_topo_type = SOP_Undefined;
  }
}


//-----------------------------------------------------------------------------

void SubdivideWidget::keyPressEvent( QKeyEvent *k )
{
   bool timerStopped = false;
   if ( timer_->isActive())
   {
      timer_->stop();
      timerStopped = true;
   }

   switch ( k->key() )
   {
     case Qt::Key_R:                               // reset
       reset_slot();
       break;
     case Qt::Key_S:                               // save
       save_slot();
       break;
     case Qt::Key_L:                               // load
       load_slot();
       break;

     case Qt::Key_A:

       if ( timerStopped )
         break;

       if (timer_->isActive())
       {
         timer_->stop();
       }
       else
       {
         reset_slot();
         timer_->setSingleShot( true );
         timer_->start(0);
       }
       break;

     case ' ':                               // subdivide
       subdiv_slot();
   }
}



//-----------------------------------------------------------------------------


void SubdivideWidget::update()
{
  unsigned int n_faces = viewer_widget_->mesh().n_faces();
  unsigned int n_edges = viewer_widget_->mesh().n_edges();
  unsigned int n_vertices = viewer_widget_->mesh().n_vertices();
  QString message(""), temp;
  message.append(temp.setNum(n_faces));
  message.append(" Faces, ");
  message.append(temp.setNum(n_edges));
  message.append(" Edges, ");
  message.append(temp.setNum(n_vertices));
  message.append(" Vertices.  ");
  if (msecs_)
  {
    message.append(temp.setNum(msecs_/1000.0));
    message.append("s");
  }
  status_bar->showMessage(message);
}


//-----------------------------------------------------------------------------


void SubdivideWidget::reset_slot()
{
  if (cur_topo_type != SOP_Undefined)
    subdivider_[cur_topo_type]->detach();

  viewer_widget_->mesh() = viewer_widget_->orig_mesh();
  viewer_widget_->mesh().update_face_normals();
  viewer_widget_->mesh().update_vertex_normals();
  viewer_widget_->updateGL();
  update();
  cur_topo_type = SOP_Undefined;
}



//-----------------------------------------------------------------------------


void SubdivideWidget::subdiv_slot()
{
  assert( sel_topo_type != SOP_Undefined );

  //QTime t;
  using namespace OpenMesh::Subdivider::Uniform;

  status_bar->showMessage( "processing subdivision step...");

  if (cur_topo_type != sel_topo_type)
  {
    if (cur_topo_type!=SOP_Undefined)
      subdivider_[cur_topo_type]->detach();
    subdivider_[cur_topo_type=sel_topo_type]->attach(viewer_widget_->mesh());
  }

  std::clog << "subdiving...\n";
  (*subdivider_[sel_topo_type])(1);
  std::clog << "subdiving...done\n";

  // Update viewer
  viewer_widget_->mesh().update_normals();
  viewer_widget_->updateGL();

  // Update status bar information
  update();
}


//-----------------------------------------------------------------------------

bool
SubdivideWidget::open_mesh(const char* _filename)
{
  OpenMesh::IO::Options opt;

  if (viewer_widget_->open_mesh(_filename, opt))
  {
    update();
    return true;
  }

  return false;
}


//-----------------------------------------------------------------------------

void
SubdivideWidget::save_slot()
{
  using OpenMesh::IO::IOManager;

  QString write_filter(IOManager().qt_write_filters().c_str());
  QString filename = QFileDialog::getSaveFileName(this, "", "", write_filter);

  if (!filename.isEmpty()){
    if (OpenMesh::IO::write_mesh(viewer_widget_->mesh(), filename.toStdString(),
         OpenMesh::IO::Options::Binary) )
      std::cerr << "ok\n";
    else
      std::cerr << "FAILED\n";
  }
}


//-----------------------------------------------------------------------------

void
SubdivideWidget::load_slot()
{
  using OpenMesh::IO::IOManager;

  QString read_filter(IOManager().qt_read_filters().c_str());
  QString filename =
  QFileDialog::getOpenFileName(this, "", "", read_filter);

  if (!filename.isNull())
  {

    if (cur_topo_type != SOP_Undefined)
      subdivider_[cur_topo_type]->detach();

    OpenMesh::IO::Options opt;
    std::string file( filename.toStdString() );

    if ( !viewer_widget_->open_mesh(file.c_str() , opt) )
    {
      QString msg = "Cannot read mesh from file ";
      QMessageBox::critical( this,"", msg + filename, QMessageBox::Ok );
    }

    update();
    cur_topo_type = SOP_Undefined;
  }
}


//-----------------------------------------------------------------------------

void
SubdivideWidget::animate_slot()
{
  if (++animate_step_ < max_animate_steps_)
  {
    subdiv_slot();
  }
  else
  {
    reset_slot();
    animate_step_ = 0;
  }
  timer_->setSingleShot(true);
  timer_->start( 500 );
}

//=============================================================================
#endif //SUBDIVIDEWIDGET_CC deifined
//=============================================================================
