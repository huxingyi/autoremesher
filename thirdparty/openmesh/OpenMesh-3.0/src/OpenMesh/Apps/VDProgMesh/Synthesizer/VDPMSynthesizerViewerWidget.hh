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

//=============================================================================
//
//  CLASS newClass
//
//=============================================================================


#ifndef OPENMESH_VDPROGMESH_VDPMSYNTHESIZERVIEWERWIDGET_HH
#define OPENMESH_VDPROGMESH_VDPMSYNTHESIZERVIEWERWIDGET_HH


//== INCLUDES =================================================================

#include <string>
#include <QTimer>

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Apps/QtViewer/MeshViewerWidgetT.hh>

#include <OpenMesh/Tools/VDPM/MeshTraits.hh>
#include <OpenMesh/Tools/VDPM/StreamingDef.hh>
#include <OpenMesh/Tools/VDPM/ViewingParameters.hh>
#include <OpenMesh/Tools/VDPM/VHierarchy.hh>
#include <OpenMesh/Tools/VDPM/VFront.hh>


//== FORWARDDECLARATIONS ======================================================


//== NAMESPACES ===============================================================

namespace OpenMesh {


//== CLASS DEFINITION =========================================================

	      
/** \class newClass newClass.hh <OpenMesh/.../newClass.hh>

    Brief Description.
  
    A more elaborate description follows.
*/



typedef TriMesh_ArrayKernelT<VDPM::MeshTraits>	VDPMMesh;
typedef MeshViewerWidgetT<VDPMMesh>		MeshViewerWidget;


  // using view dependent progressive mesh 

  using VDPM::Plane3d;
  using VDPM::VFront;
  using VDPM::VHierarchy;
  using VDPM::VHierarchyNode;
  using VDPM::VHierarchyNodeIndex;
  using VDPM::VHierarchyNodeHandle;
  using VDPM::VHierarchyNodeHandleContainer;
  using VDPM::ViewingParameters;


//== CLASS DEFINITION =========================================================


class VDPMSynthesizerViewerWidget : public MeshViewerWidget
{
public:

  typedef MeshViewerWidget Base;

public:

  VDPMSynthesizerViewerWidget(QWidget* _parent=0, const char* _name=0);

  ~VDPMSynthesizerViewerWidget();

  /// open view-dependent progressive mesh
  void open_vd_prog_mesh(const char* _filename);


private:

  QString             qFilename_;
  VHierarchy          vhierarchy_;
  VFront              vfront_;
  ViewingParameters   viewing_parameters_;
  float               kappa_square_;
  bool                adaptive_mode_;

  unsigned int        n_base_vertices_;
  unsigned int        n_base_edges_;
  unsigned int        n_base_faces_;
  unsigned int        n_details_;

    
private:

  bool outside_view_frustum(const OpenMesh::Vec3f &pos, float radius);

  bool oriented_away(float sin_square, 
		     float distance_square, 
		     float product_value);

  bool screen_space_error(float mue_square, 
			  float sigma_square, 
			  float distance_square, 
			  float product_value);

  void update_viewing_parameters();

  virtual void keyPressEvent(QKeyEvent* _event);

protected:

  /// inherited drawing method
  virtual void draw_scene(const std::string& _draw_mode);

public:

  void adaptive_refinement();	

  bool qrefine(VHierarchyNodeHandle _node_handle);		

  void force_vsplit(VHierarchyNodeHandle _node_handle);

  bool ecol_legal(VHierarchyNodeHandle _parent_handle, 
		  VDPMMesh::HalfedgeHandle& v0v1);

  void get_active_cuts(VHierarchyNodeHandle _node_handle, 
		       VDPMMesh::VertexHandle &vl, VDPMMesh::VertexHandle &vr);

  void vsplit(VHierarchyNodeHandle _node_handle, 
	      VDPMMesh::VertexHandle vl, VDPMMesh::VertexHandle vr);

  void ecol(VHierarchyNodeHandle _parent_handle, 
	    const VDPMMesh::HalfedgeHandle& v0v1);

  void init_vfront();
 
};


//=============================================================================
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESHAPPS_VDPMSYNTHESIZERVIEWERWIDGET_HH defined
//=============================================================================
