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



#ifndef OPENMESH_APPS_VDPMSTREAMING_CLIENT_VDPMCLIENTVIEWERWIDGET_HH
#define OPENMESH_APPS_VDPMSTREAMING_CLIENT_VDPMCLIENTVIEWERWIDGET_HH

//== INCLUDES =================================================================

#include <QTimer>
#include <QTcpSocket>
#include <QDataStream>
#include <iostream>
#include <string>

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Geometry/Plane3d.hh>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include <OpenMesh/Tools/VDPM/StreamingDef.hh>
#include <OpenMesh/Tools/VDPM/ViewingParameters.hh>
#include <OpenMesh/Tools/VDPM/VHierarchy.hh>
#include <OpenMesh/Tools/VDPM/VFront.hh>

#include <OpenMesh/Apps/VDProgMesh/Streaming/Client/MeshViewerWidgetT.hh>
#include <OpenMesh/Apps/VDProgMesh/Streaming/Client/MyMesh.hh>
//#include <OpenMesh/Apps/VDProgMesh/Streaming/Client/VDPMClientSession.hh>



typedef MeshViewerWidgetT<MyMesh>                 MeshViewerWidget;



using OpenMesh::VDPM::VDPMStreamingPhase;
using OpenMesh::VDPM::kVSplitHeader;
using OpenMesh::VDPM::kVSplits;
using OpenMesh::VDPM::kBaseMesh;

using OpenMesh::VDPM::Plane3d;

using OpenMesh::VDPM::VFront;
using OpenMesh::VDPM::VHierarchy;
using OpenMesh::VDPM::VHierarchyNodeIndex;
using OpenMesh::VDPM::VHierarchyNodeHandle;
using OpenMesh::VDPM::ViewingParameters;
using OpenMesh::VDPM::set_debug_print;


//== CLASS DEFINITION =========================================================


class VDPMClientViewerWidget : public MeshViewerWidget
{

  Q_OBJECT

public:
  VDPMClientViewerWidget(QWidget *_parent=0, const char *_name=0)
    : MeshViewerWidget(_parent, _name)
  {
    set_debug_print(true);
    adaptive_mode_ = false;
    
    qSessionTimer_ = new QTimer(this);
    qSocket_ = new QTcpSocket(this);
    streaming_phase_ = kBaseMesh;
    session_running_ = false;

    
    connect(qSessionTimer_, SIGNAL(timeout()),
      this, SLOT(session_timer_check()));

    connect(qSessionTimer_, SIGNAL(timeout()),
      this, SLOT(socketReadyRead()));
    
    // connect signal-slots about QSocket
    connect(qSocket_, SIGNAL(connected()), 
	    this, SLOT(socketConnected()));

    connect(qSocket_, SIGNAL(connectionClosed()), 
	    this, SLOT(socketConnectionClosed()));

    connect(qSocket_, SIGNAL(readyRead()), 
	    this, SLOT(socketReadyRead()));

    connect(qSocket_, SIGNAL(error( QAbstractSocket::SocketError )), 
	    this, SLOT(socketError( QAbstractSocket::SocketError )));


    look_around_mode_ = false;
    frame_ = 0;
    n_viewpoints_ = 60;

    global_timer_.reset();
    global_timer_.start();
    render_timer_.reset();
    refinement_timer_.reset();
    session_timer_.reset();

    qAnimationTimer_ = new QTimer(this);
   
    connect(qAnimationTimer_, SIGNAL(timeout()),
      this, SLOT(look_around()));
    //connect(qAnimationTimer_, SIGNAL(timeout()),
    //  this, SLOT(print_statistics()));


    uplink_file = fopen("uplink.txt", "w");
    downlink_file = fopen("downlink.txt", "w");

    render_file = fopen("render.txt", "w");
    refinement_file = fopen("refinement.txt", "w");
    session_file = fopen("session.txt", "w");

    vd_streaming_ = true;
    max_transmitted_datasize_ = 0;
    transmitted_datasize_ = 0;    
  }

  ~VDPMClientViewerWidget()
  {
    fclose(uplink_file);
    fclose(downlink_file);

    fclose(render_file);
    fclose(refinement_file);
    fclose(session_file);
  }


  void connectToServer( std::string& _server_name, 
			int _port= VDPM_STREAMING_PORT )
  {
    qSocket_->connectToHost( _server_name.c_str(), _port );
  }

  void openBaseMesh( std::string& _base_mesh )
  {
    open_vd_base_mesh( _base_mesh.c_str() );
    std::cout << "spm file: " << _base_mesh << std::endl;
  }

// socket related slots
private slots:

  void closeConnection()
  {
    close();
    if (qSocket_->state() == QAbstractSocket::ClosingState)     // we have a delayed close.
    {      
      connect(this, SIGNAL(delayedCloseFinished()), SLOT(socketClosed()));
    }
    else                                 // the qSocket is closed.
    {      
      socketClosed();
    }
  }

  void socketReadyRead()
  {
    switch( streaming_phase_)
    {
      case kVSplits:      receive_vsplit_packets(); break;
      case kVSplitHeader: receive_vsplit_header();  break;
      case kBaseMesh:     receive_base_mesh();      break;
    }

  }

  void socketConnected()
  {
    std::cout << "Connected to server" << std::endl;
  }

  void socketConnectionClosed()
  {
    std::cout << "Connection closed by the server" << std::endl;
  }

  void socketClosed()
  {
    std::cout << "Connection closed" << std::endl;
  }

  void socketError(QAbstractSocket::SocketError e)
  {
    std::cout << "Error number " << e << " occurred" << std::endl;
  }

  void look_around();
  void print_statistics();


  void session_timer_check()
  {
    std::cout << "Session Timer works" << std::endl;
  }

// for view-dependent PM 
private:
  VHierarchy          vhierarchy_;
  //unsigned char       tree_id_bits_;
  VFront              vfront_;
  ViewingParameters   viewing_parameters_;
  float               kappa_square_;
  bool                adaptive_mode_;

  unsigned int        n_base_vertices_;
  unsigned int        n_base_edges_;
  unsigned int        n_base_faces_;
  unsigned int        n_details_;

private:

  bool outside_view_frustum(const OpenMesh::Vec3f &pos, float radius);
  bool oriented_away(float sin_square, float distance_square, 
		     float product_value);
  bool screen_space_error(float mue_square, float sigma_square, 
			  float distance_square, float product_value);
  void update_viewing_parameters();

  virtual void keyPressEvent(QKeyEvent *_event);

protected:

  /// inherited drawing method
  virtual void draw_scene(const std::string& _draw_mode);


public:

  void open_vd_prog_mesh(const char* _filename);
  
  unsigned int  num_base_vertices() const     { return  n_base_vertices_; }
  unsigned int  num_base_edges() const        { return  n_base_edges_; }
  unsigned int  num_base_faces() const        { return  n_base_faces_; }
  unsigned int  num_details() const           { return  n_details_; }  

  void adaptive_refinement();	
  bool qrefine(VHierarchyNodeHandle _node_handle);		
  void force_vsplit(VHierarchyNodeHandle _node_handle);
  bool ecol_legal(VHierarchyNodeHandle _parent_handle, MyMesh::HalfedgeHandle& v0v1);

  void get_active_cuts(VHierarchyNodeHandle _node_handle, MyMesh::VertexHandle &vl, MyMesh::VertexHandle &vr);
  void vsplit(VHierarchyNodeHandle _node_handle, MyMesh::VertexHandle vl, MyMesh::VertexHandle vr);
  void ecol(VHierarchyNodeHandle _parent_handle, const MyMesh::HalfedgeHandle& v0v1);

	void init_vfront(); 

  // streaming realted functions
private:
  QTimer              *qSessionTimer_;
  QTcpSocket          *qSocket_;
  QString             qFilename_;
  bool                session_running_;
  VDPMStreamingPhase  streaming_phase_;
  unsigned int        n_vsplit_packets_;

public:
  void connect_to_server();
  bool request_base_mesh();
  bool receive_base_mesh();
  void send_viewing_information();
  void receive_vsplit_header();
  void receive_vsplit_packets();
  void open_vd_base_mesh(const char* _filename);
  void update_vhierarchy(
    const OpenMesh::Vec3f     &_pos,              // 3D position of v0
    const VHierarchyNodeIndex &_v,                // vhierarchy index of v1
    const VHierarchyNodeIndex &_fund_lcut_index,  // vhierarchy index of fundamental lcut
    const VHierarchyNodeIndex &_fund_rcut_index,  // vhierarchy index of fundamental rcut
    const float               _radius[2],         // radius of lchild & rchild
    const OpenMesh::Vec3f     _normal[2],         // normal of lchild & rchild
    const float               _sin_square[2],     // sin_square of lchild & rchild
    const float               _mue_square[2],     // mue_square of lchild & rchild
    const float               _sigma_square[2]    // sigma_square of lchild & rchild
    );

   
  // for example
private:
  QTimer              *qAnimationTimer_;
  QString             qCameraFileName_;
  MyMesh::Point       bbMin_, bbMax_;
  unsigned int        frame_;
  int                 max_transmitted_datasize_;
  int                 transmitted_datasize_;
  bool                vd_streaming_;

  unsigned int        nth_viewpoint_;
  unsigned int        n_viewpoints_;
  bool                look_around_mode_;
  GLdouble            reserved_modelview_matrix_[16];
  GLdouble            reserved_projection_matrix_[16];

  FILE  *uplink_file;
  FILE  *downlink_file;
  FILE  *render_file;
  FILE  *refinement_file;
  FILE  *session_file;

public:
  void save_screen(bool _flag);
  void save_views();
  void load_views(const char *camera_filename);
  void screen_capture(const char *_filename);
  void current_max_resolution();

  OpenMesh::Utils::Timer  global_timer_;
  OpenMesh::Utils::Timer  render_timer_;
  OpenMesh::Utils::Timer  refinement_timer_;
  OpenMesh::Utils::Timer  session_timer_;



#ifdef EXAMPLE_CREATION
  void increase_max_descendents(const VHierarchyNodeIndex &node_index);
  void increase_cur_descendents(VHierarchyNodeHandle _node_handle);
  void __add_children(const VHierarchyNodeIndex &node_index, bool update_current = true);
  void mesh_coloring();
#endif
};

#endif //OPENMESH_APPS_VDPMSTREAMING_CLIENT_VDPMCLIENTVIEWERWIDGET_HH defined
