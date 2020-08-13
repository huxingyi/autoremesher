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

#include <QKeyEvent>
#include <iterator>
#include <OpenMesh/Apps/VDProgMesh/Streaming/Server/VDPMServerViewerWidget.hh>

bool
VDPMServerViewerWidget::
open_vd_prog_mesh(const char *_filename)
{
  ServerSideVDPMListIter  vdpm_it;

  vdpm_it = vdpms_.insert(vdpms_.end(), ServerSideVDPM());

  ServerSideVDPM &vdpm = *vdpm_it;

  return  vdpm.open_vd_prog_mesh(_filename);
}

ServerSideVDPM*
VDPMServerViewerWidget::
get_vdpm(const char _vdpm_name[256])
{
  ServerSideVDPMListIter  vdpm_it;

  for (vdpm_it=vdpms_.begin(); vdpm_it!=vdpms_.end(); ++vdpm_it)
  {
    if (vdpm_it->is_same_name(_vdpm_name) == true)
    {
      return  &(*vdpm_it);
    }
  }

  return  NULL;
}

void
VDPMServerViewerWidget::
keyPressEvent(QKeyEvent* _event)
{
  bool handled(false);

  QString filename;

  switch (_event->key())
  {
    case Qt::Key_D:
      set_debug_print(!debug_print());
      std::cout << "debug print mode "
    << (debug_print() == true ? "on" : "off") << std::endl;
      break;

    case Qt::Key_O:
#if defined(OM_CC_MSVC)
      filename = QFileDialog::getOpenFileName(0, "", "d:/data/models/spm/", "*.spm");
#else
      filename = QFileDialog::getOpenFileName(0, "", "~/data/models/spm/", "*.spm");
#endif
      open_vd_prog_mesh( filename.toStdString().c_str() );
      break;

    case Qt::Key_I:
      std::copy( vdpms_.begin(), vdpms_.end(),
                 std::ostream_iterator<ServerSideVDPM>(std::cout, "\n") );
      break;

    case Qt::Key_V:
      vd_streaming_ = !(vd_streaming_);
      if (vd_streaming_)
        std::cout << "View-dependent streaming mode" << std::endl;
      else
        std::cout << "Sequential streaming mode" << std::endl;

      break;

    case Qt::Key_Q:
    case Qt::Key_Escape:
      qApp->quit();
  }

  if (!handled)
    _event->ignore();
}
