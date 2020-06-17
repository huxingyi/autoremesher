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



#ifndef OPENMESH_APP_VDPMSTREAMING_SERVER_VDPMSTREAMINGSERVERWIDGET_HH
#define OPENMESH_APP_VDPMSTREAMING_SERVER_VDPMSTREAMINGSERVERWIDGET_HH

#include <QApplication>
#include <QWidget>
#include <QFileDialog>
#include <QString>
#include <OpenMesh/Apps/VDProgMesh/Streaming/Server/ServerSideVDPM.hh>
#include <OpenMesh/Apps/VDProgMesh/Streaming/Server/VDPMServerSocket.hh>
#include <OpenMesh/Apps/VDProgMesh/Streaming/Server/VDPMServerSession.hh>


using OpenMesh::VDPM::set_debug_print;
using OpenMesh::VDPM::debug_print;


class VDPMServerViewerWidget : public QWidget
{
  Q_OBJECT
public:
  //VDPMServerViewerWidget(QWidget *_parent) : QWidget(_parent)
  VDPMServerViewerWidget() : QWidget()
  {
    server = new VDPMServerSocket(this);

//     connect(server, 
// 	    SIGNAL(newConnection()), 
// 	    SLOT(newConnect()));

    vd_streaming_ = true;
  }

private:
  typedef ServerSideVDPMList::iterator  ServerSideVDPMListIter;
  ServerSideVDPMList  vdpms_;
  bool                vd_streaming_;
  VDPMServerSocket    *server;

public:

  ServerSideVDPM* get_vdpm(const char _vdpm_name[256]);


public:

  bool  open_vd_prog_mesh(const char *_filename);
  bool  vd_streaming() const      { return  vd_streaming_; }

private slots:

  void newConnect(VDPMServerSession *s)
  {
    std::cout << "New connection" << std::endl;

//     connect(s, SIGNAL(connectionClosed()), SLOT(connectionClosed()));
  }

  void connectionClosed()
  {
    std::cout << "Client closed connection" << std::endl;
  }

protected:

  virtual void keyPressEvent(QKeyEvent* _event);

};


#endif //OPENMESH_APP_VDPMSTREAMING_SERVER_VDPMSTREAMINGSERVERWIDGET_HH defined


