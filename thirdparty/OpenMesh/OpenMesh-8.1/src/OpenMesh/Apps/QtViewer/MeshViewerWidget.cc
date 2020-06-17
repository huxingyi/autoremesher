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



#define OPENMESHAPPS_MESHVIEWERWIDGET_CC

//== INCLUDES =================================================================

#include <OpenMesh/Apps/QtViewer/MeshViewerWidget.hh>


//== IMPLEMENTATION ========================================================== 

/// default constructor
MeshViewerWidget::MeshViewerWidget(QWidget* parent)  : MeshViewerWidgetT<MyMesh>(parent)
{}

 void MeshViewerWidget::open_mesh_gui(QString fname)
{
    OpenMesh::Utils::Timer t;
    t.start();
    if ( fname.isEmpty() || !open_mesh(fname.toLocal8Bit(), _options) )
    {
        QString msg = "Cannot read mesh from file:\n '";
        msg += fname;
        msg += "'";
        QMessageBox::critical( nullptr, windowTitle(), msg);
    }
    t.stop();
    std::cout << "Loaded mesh in ~" << t.as_string() << std::endl;
    
}

void MeshViewerWidget::open_texture_gui(QString fname)
{
    if ( fname.isEmpty() || !open_texture( fname.toLocal8Bit() ) )
    {
        QString msg = "Cannot load texture image from file:\n '";
        msg += fname;
        msg += "'\n\nPossible reasons:\n";
        msg += "- Mesh file didn't provide texture coordinates\n";
        msg += "- Texture file does not exist\n";
        msg += "- Texture file is not accessible.\n";
        QMessageBox::warning( nullptr, windowTitle(), msg );
    }
}

void MeshViewerWidget::query_open_mesh_file() {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open mesh file"),
        tr(""),
        tr("OBJ Files (*.obj);;"
        "OFF Files (*.off);;"
        "STL Files (*.stl);;"
        "All Files (*)"));
    if (!fileName.isEmpty())
        open_mesh_gui(fileName);
}

void MeshViewerWidget::query_open_texture_file() {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open texture file"),
        tr(""),
        tr("PNG Files (*.png);;"
        "BMP Files (*.bmp);;"
        "GIF Files (*.gif);;"
        "JPEG Files (*.jpg);;"
        "TIFF Files (*.tif);;"
        "All Files (*)"));
    if (!fileName.isEmpty())
        open_texture_gui(fileName);
}

//=============================================================================

