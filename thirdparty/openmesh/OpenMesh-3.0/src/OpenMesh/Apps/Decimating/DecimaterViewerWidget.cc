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


//== INCLUDES =================================================================

#ifdef _MSC_VER
#  pragma warning(disable: 4267 4311)
#endif

#include <iostream>
#include <fstream>

#include <qapplication.h>
#include <qdatetime.h>

#include <OpenMesh/Core/IO/BinaryHelper.hh>
#include <OpenMesh/Core/IO/MeshIO.hh>

#include <OpenMesh/Apps/Decimating/DecimaterViewerWidget.hh>


//== IMPLEMENTATION ========================================================== 


//-----------------------------------------------------------------------------

void DecimaterViewerWidget::keyPressEvent(QKeyEvent* _event)
{
  switch (_event->key())
  {
    case Key_H:
      std::cout << "Press '+' to increase the number of decimating steps\n"
                << "Press '-' to decrease the number of decimating steps\n"
                << "Press 'd' to perform the set number of decimating steps\n"
                << "Press 'S' to save the mesh to 'result.off'\n"
                << "Press 'q' or 'Esc' quit the application" << std::endl;
      break;
    case Key_D:
    {
      int rc;
      if ( (rc=decimater_->decimate(steps_)) )
      {
        decimater_->mesh().garbage_collection();
        std::cout << rc << " vertices removed!\n";
        updateGL();
      }
      else
        std::cout << "Decimation failed\n";
      break;
    }

    case Key_Plus:
      ++steps_;
      steps_ = std::min( steps_ , (size_t)( mesh_.n_vertices() / 10 ) );
      updateGL();
      std::cout << "# decimating steps increased to " << steps_ << std::endl;
      break;

    case Key_Minus:
      --steps_;
      steps_ = std::max( steps_ , size_t(1) );
      updateGL();
      std::cout << "# decimating steps increased to " << steps_ << std::endl;
      break;

    case Key_S:
    {
      OpenMesh::IO::Options opt;

      opt += OpenMesh::IO::Options::Binary;

      if (OpenMesh::IO::write_mesh( mesh(), "result.off", opt ))
        std::cout << "mesh saved in 'result.off'\n";
    }
    break;

    case Key_Q:
      qApp->quit();
      break;
    case Key_Escape:
      qApp->quit();      
      break;
    default:
      this->inherited_t::keyPressEvent(_event);
      break;
  }
}

void DecimaterViewerWidget::animate( void )   
{
//    updateGL();
//    timer_->start(300, true);
}

//=============================================================================
