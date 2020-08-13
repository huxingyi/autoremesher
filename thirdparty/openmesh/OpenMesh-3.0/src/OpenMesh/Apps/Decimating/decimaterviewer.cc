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
#include <OpenMesh/Tools/Utils/getopt.h>
#include <qapplication.h>
#include <qmessagebox.h>

#include "DecimaterViewerWidget.hh"

#ifdef ARCH_DARWIN
#include <glut.h>
#else
#include <GL/glut.h>
#endif

void usage_and_exit(int xcode);


int main(int argc, char **argv)
{
#if defined(OM_USE_OSG) && OM_USE_OSG
  osg::osgInit(argc, argv);
#endif
  
  // OpenGL check
  QApplication::setColorSpec( QApplication::CustomColor );
  QApplication app(argc,argv);
  
#if !defined(__APPLE__)
  glutInit(&argc,argv);
#endif

  if ( !QGLFormat::hasOpenGL() ) {
    QString msg = "System has no OpenGL support!";
    QMessageBox::critical( NULL, "OpenGL", msg + argv[1] );
    return -1;
  }


  int c;
  OpenMesh::IO::Options opt;
  
  while ( (c=getopt(argc,argv,"s"))!=-1 )
  {
     switch(c)
     {
       case 's': opt += OpenMesh::IO::Options::Swap; break;
       case 'h':
          usage_and_exit(0);
       default:
          usage_and_exit(1);
     }
  }
  // create widget
  DecimaterViewerWidget w(0);  
//  app.setMainWidget(&w);

  w.resize(400, 400);
  w.show(); 

  // load scene
  if ( optind < argc )  
  {
     if ( ! w.open_mesh(argv[optind], opt) )
     {
        QString msg = "Cannot read mesh from file:\n '";
        msg += argv[optind];
        msg += "'";
        QMessageBox::critical( NULL, w.windowTitle(), msg );
        return 1;
     }
  }

  if ( ++optind < argc )
  {
     if ( ! w.open_texture( argv[optind] ) )
     {
         QString msg = "Cannot load texture image from file:\n '";
        msg += argv[optind];
        msg += "'\n\nPossible reasons:\n";
        msg += "- Mesh file didn't provide texture coordinates\n";
        msg += "- Texture file does not exist\n";
        msg += "- Texture file is not accessible.\n";
        QMessageBox::warning( NULL, w.windowTitle(), msg );
     }
  }

  return app.exec();
}

void usage_and_exit(int xcode)
{
   std::cout << "Usage: DecimaterGui [-s] [mesh] [texture]\n" << std::endl;
   std::cout << "Options:\n"
             << "  -s\n"
             << "    Reverse byte order, when reading binary files.\n"
             << "    Press 'h' when the application is running for more options.\n"
             << std::endl;
   exit(xcode);
}
