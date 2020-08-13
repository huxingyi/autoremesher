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

#ifndef OPENMESH_APP_VDPMSTREAMING_SERVER_VDPMSERVERSOCKET_HH
#define OPENMESH_APP_VDPMSTREAMING_SERVER_VDPMSERVERSOCKET_HH

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QVBoxLayout>

#include <OpenMesh/Apps/VDProgMesh/Streaming/Server/VDPMServerSession.hh>
#include <OpenMesh/Tools/VDPM/StreamingDef.hh>
#include <iostream>


class VDPMServerSocket : public QTcpServer
{
  Q_OBJECT

public:

  VDPMServerSocket(QObject *parent=0)
    : QTcpServer(parent)
  {
    setMaxPendingConnections(1);
    if ( !listen(QHostAddress::Any, VDPM_STREAMING_PORT)  )
    {
      std::cerr << "Failed to bind to port " << VDPM_STREAMING_PORT << std::endl;
      exit(1);
    }

    connect(this, SIGNAL(newConnection()),this,SLOT(newConnections()));
  }

public slots:

  void newConnections()
  {
    VDPMServerSession *s = new VDPMServerSession(nextPendingConnection(), this);
    //s->set_vdpm();
    emit newConnect(s);
    std::cout << "new connection"<< /*socket << */std::endl;
  }

signals:

  void newConnect(VDPMServerSession*);

};



#endif //OPENMESH_APP_VDPMSTREAMING_SERVER_VDPMSERVERSOCKET_HH defined
