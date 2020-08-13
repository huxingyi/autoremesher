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

#ifdef _MSC_VER
#  pragma warning(disable: 4267 4311)
#endif

#include <iostream>
#include <fstream>
#include <QApplication>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenuBar>
#include <QFileDialog>

#ifdef ARCH_DARWIN
#include <glut.h>
#else
#include <GL/glut.h>
#endif

#include "MeshViewerWidget.hh"


void create_menu(QMainWindow &w);
void usage_and_exit(int xcode);

int main(int argc, char **argv)
{
  // OpenGL check
  QApplication::setColorSpec( QApplication::CustomColor );
  QApplication app(argc,argv);
#if !defined(__APPLE__)
  glutInit(&argc,argv);
#endif

  if ( !QGLFormat::hasOpenGL() ) {
    QString msg = "System has no OpenGL support!";
    QMessageBox::critical( 0, QString("OpenGL"), msg + QString(argv[1]) );
    return -1;
  }

  int c;
  OpenMesh::IO::Options opt;

  while ( (c=getopt(argc,argv,"hbs"))!=-1 )
  {
     switch(c)
     {
       case 'b': opt += OpenMesh::IO::Options::Binary; break;
       case 'h':
          usage_and_exit(0);
       case 's': opt += OpenMesh::IO::Options::Swap; break;
       default:
          usage_and_exit(1);
     }
  }

  // enable most options for now
  opt += OpenMesh::IO::Options::VertexColor;
  opt += OpenMesh::IO::Options::VertexNormal;
  opt += OpenMesh::IO::Options::VertexTexCoord;
  opt += OpenMesh::IO::Options::FaceColor;
  opt += OpenMesh::IO::Options::FaceNormal;
  opt += OpenMesh::IO::Options::FaceTexCoord;

  // create widget
  QMainWindow mainWin;
  MeshViewerWidget w(&mainWin);
  w.setOptions(opt);
  mainWin.setCentralWidget(&w);

  create_menu(mainWin);

  // static mesh, hence use strips
  w.enable_strips();

  mainWin.resize(640, 480);
  mainWin.show();

  // load scene if specified on the command line
  if ( optind < argc )
  {
    w.open_mesh_gui(argv[optind]);
  }

  if ( ++optind < argc )
  {
      w.open_texture_gui(argv[optind]);
  }

  return app.exec();
}

void create_menu(QMainWindow &w)
{
    using namespace Qt;
    QMenu *fileMenu = w.menuBar()->addMenu(w.tr("&File"));

    QAction* openAct = new QAction(w.tr("&Open mesh..."), &w);
    openAct->setShortcut(w.tr("Ctrl+O"));
    openAct->setStatusTip(w.tr("Open a mesh file"));
    QObject::connect(openAct, SIGNAL(triggered()), w.centralWidget(), SLOT(query_open_mesh_file()));
    fileMenu->addAction(openAct);

    QAction* texAct = new QAction(w.tr("Open &texture..."), &w);
    texAct->setShortcut(w.tr("Ctrl+T"));
    texAct->setStatusTip(w.tr("Open a texture file"));
    QObject::connect(texAct, SIGNAL(triggered()), w.centralWidget(), SLOT(query_open_texture_file()));
    fileMenu->addAction(texAct);
}

void usage_and_exit(int xcode)
{
   std::cout << "Usage: meshviewer [-s] [mesh] [texture]\n" << std::endl;
   std::cout << "Options:\n"
	     << "  -b\n"
	     << "    Assume input to be binary.\n\n"
             << "  -s\n"
             << "    Reverse byte order, when reading binary files.\n"
             << std::endl;
   exit(xcode);
}
