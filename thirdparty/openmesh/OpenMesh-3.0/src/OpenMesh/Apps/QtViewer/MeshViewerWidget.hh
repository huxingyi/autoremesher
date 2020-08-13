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

#ifndef OPENMESHAPPS_VIEWERWIDGET_HH
#define OPENMESHAPPS_VIEWERWIDGET_HH

//== INCLUDES =================================================================

#include <QWidget>
#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <OpenMesh/Tools/Utils/getopt.h>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include <OpenMesh/Apps/QtViewer/MeshViewerWidgetT.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>


//== CLASS DEFINITION =========================================================

using namespace OpenMesh;  
using namespace OpenMesh::Attributes;

struct MyTraits : public OpenMesh::DefaultTraits
{
  HalfedgeAttributes(OpenMesh::Attributes::PrevHalfedge);
};

typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits>  MyMesh;



//== CLASS DEFINITION =========================================================

class MeshViewerWidget : public MeshViewerWidgetT<MyMesh>
{
    Q_OBJECT
public:
    /// default constructor
    MeshViewerWidget(QWidget* parent=0) : MeshViewerWidgetT<MyMesh>(parent)
    {}
    OpenMesh::IO::Options& options() { return _options; }
    const OpenMesh::IO::Options& options() const { return _options; }
    void setOptions(const OpenMesh::IO::Options& opts) { _options = opts; }

    void open_mesh_gui(QString fname)
    {
        OpenMesh::Utils::Timer t;
        t.start();
        if ( fname.isEmpty() || !open_mesh(fname.toLocal8Bit(), _options) )
        {
            QString msg = "Cannot read mesh from file:\n '";
            msg += fname;
            msg += "'";
            QMessageBox::critical( NULL, windowTitle(), msg);
        }
        t.stop();
        std::cout << "Loaded mesh in ~" << t.as_string() << std::endl;
    }
    void open_texture_gui(QString fname)
    {
        if ( fname.isEmpty() || !open_texture( fname.toLocal8Bit() ) )
        {
            QString msg = "Cannot load texture image from file:\n '";
            msg += fname;
            msg += "'\n\nPossible reasons:\n";
            msg += "- Mesh file didn't provide texture coordinates\n";
            msg += "- Texture file does not exist\n";
            msg += "- Texture file is not accessible.\n";
            QMessageBox::warning( NULL, windowTitle(), msg );
        }
    }

public slots:
    void query_open_mesh_file() {
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
    void query_open_texture_file() {
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
private:
    OpenMesh::IO::Options _options;
};


#endif
