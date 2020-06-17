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
