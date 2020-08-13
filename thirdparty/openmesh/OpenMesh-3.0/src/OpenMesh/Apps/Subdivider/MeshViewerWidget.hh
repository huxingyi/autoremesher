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


#ifndef OPENMESHAPPS_MESHVIEWERWIDGET_HH
#define OPENMESHAPPS_MESHVIEWERWIDGET_HH


//== INCLUDES =================================================================


// -------------------- OpenMesh
#include <OpenMesh/Apps/QtViewer/MeshViewerWidgetT.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/Composite/CompositeTraits.hh>

//

typedef OpenMesh::Subdivider::Uniform::CompositeTraits Traits;
typedef OpenMesh::TriMesh_ArrayKernelT<Traits>         Mesh;



//== CLASS DEFINITION =========================================================

	      

class MeshViewerWidget : public MeshViewerWidgetT<Mesh>
{
public:
   typedef MeshViewerWidgetT<Mesh> Base;
   
  /// default constructor
  MeshViewerWidget(QWidget* _parent=0)
    : Base(_parent)
  {}

  /// destructor
  ~MeshViewerWidget() {}

  /// open mesh
  inline bool open_mesh(const char* _filename, OpenMesh::IO::Options _opt)
  {
    if ( Base::open_mesh( _filename, _opt ) )
    {
      orig_mesh_ = mesh_;
      return true;
    }
    return false;
  }

  Mesh& orig_mesh() { return orig_mesh_; }
  const Mesh& orig_mesh() const { return orig_mesh_; }
   
protected:

  Mesh orig_mesh_;

};


//=============================================================================
#endif // OPENMESHAPPS_MESHVIEWERWIDGET_HH defined
//=============================================================================

