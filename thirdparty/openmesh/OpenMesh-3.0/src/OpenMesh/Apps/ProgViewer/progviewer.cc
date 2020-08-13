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

#include <OpenMesh/Apps/ProgViewer/ProgViewerWidget.hh>
#include <QString>
#include <QApplication>
#include <QGLWidget>

#ifdef ARCH_DARWIN
#include <glut.h>
#else
#include <GL/glut.h>
#endif

int main(int argc, char **argv)
{
  // OpenGL check
  QApplication::setColorSpec( QApplication::CustomColor );
  QApplication app(argc,argv);

  glutInit(&argc,argv);

  if ( !QGLFormat::hasOpenGL() ) {
    std::cerr << "This system has no OpenGL support.\n";
    return -1;
  }

    
  // create widget
  ProgViewerWidget w(0);
  w.resize(400, 400);
  w.show();
  
  // load scene
  if (argc > 1)  w.open_prog_mesh(argv[1]);


  // print usage info
  std::cout << "\n\n"
	    << "Press  Minus : Coarsen mesh\n"
	    << "       Plus  : Refine mesh\n"
	    << "       Home  : Coarsen down to base mesh\n"
	    << "       End   : Refine up to finest mesh\n"
	    << "\n";

	    
  return app.exec();
}
