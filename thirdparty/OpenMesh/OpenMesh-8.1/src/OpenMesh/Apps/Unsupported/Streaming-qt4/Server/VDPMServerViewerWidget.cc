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

  return  nullptr;
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
