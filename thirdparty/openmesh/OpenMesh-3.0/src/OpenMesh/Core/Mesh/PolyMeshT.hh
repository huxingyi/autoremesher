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
//  CLASS PolyMeshT
//
//=============================================================================


#ifndef OPENMESH_POLYMESHT_HH
#define OPENMESH_POLYMESHT_HH


//== INCLUDES =================================================================


#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Geometry/MathDefs.hh>
#include <OpenMesh/Core/Mesh/PolyConnectivity.hh>
#include <OpenMesh/Core/Mesh/FinalMeshItemsT.hh>
#include <vector>


//== NAMESPACES ===============================================================


namespace OpenMesh {

//== CLASS DEFINITION =========================================================


/** \class PolyMeshT PolyMeshT.hh <OpenMesh/Mesh/PolyMeshT.hh>

    Base type for a polygonal mesh.

    This is the base class for a polygonal mesh. It is parameterized
    by a mesh kernel that is given as a template argument. This class
    inherits all methods from its mesh kernel.

    \param Kernel: template argument for the mesh kernel
    \note You should use the predefined mesh-kernel combinations in
    \ref mesh_types_group
    \see \ref mesh_type
*/

template <class Kernel>
class PolyMeshT : public Kernel
{
public:

  /// Self type. Used to specify iterators/circulators.
  typedef PolyMeshT<Kernel>                   This;
  //--- item types ---

  //@{
  /// Determine whether this is a PolyMeshT or TriMeshT ( This function does not check the per face vertex count! It only checks if the datatype is PolyMeshT or TriMeshT )
  enum { IsPolyMesh = 1 };
  enum { IsTriMesh  = 0 };
  static bool is_polymesh() { return true;  }
  static bool is_trimesh()  { return false; }
  //@}

  /// \name Mesh Items
  //@{
  /// Scalar type
  typedef typename Kernel::Scalar    Scalar;
  /// Coordinate type
  typedef typename Kernel::Point     Point;
  /// Normal type
  typedef typename Kernel::Normal    Normal;
  /// Color type
  typedef typename Kernel::Color     Color;
  /// TexCoord1D type
  typedef typename Kernel::TexCoord1D  TexCoord1D;
  /// TexCoord2D type
  typedef typename Kernel::TexCoord2D  TexCoord2D;
  /// TexCoord3D type
  typedef typename Kernel::TexCoord3D  TexCoord3D;
  /// Vertex type
  typedef typename Kernel::Vertex    Vertex;
  /// Halfedge type
  typedef typename Kernel::Halfedge  Halfedge;
  /// Edge type
  typedef typename Kernel::Edge      Edge;
  /// Face type
  typedef typename Kernel::Face      Face;
  //@}

  //--- handle types ---

  /// Handle for referencing the corresponding item
  typedef typename Kernel::VertexHandle       VertexHandle;
  typedef typename Kernel::HalfedgeHandle     HalfedgeHandle;
  typedef typename Kernel::EdgeHandle         EdgeHandle;
  typedef typename Kernel::FaceHandle         FaceHandle;



  typedef typename Kernel::VertexIter                 VertexIter;
  typedef typename Kernel::HalfedgeIter               HalfedgeIter;
  typedef typename Kernel::EdgeIter                   EdgeIter;
  typedef typename Kernel::FaceIter                   FaceIter;

  typedef typename Kernel::ConstVertexIter            ConstVertexIter;
  typedef typename Kernel::ConstHalfedgeIter          ConstHalfedgeIter;
  typedef typename Kernel::ConstEdgeIter              ConstEdgeIter;
  typedef typename Kernel::ConstFaceIter              ConstFaceIter;
  //@}

  //--- circulators ---

  /** \name Mesh Circulators
      Refer to OpenMesh::Mesh::Iterators or \ref mesh_iterators
      for documentation.
  */
  //@{
  /// Circulator
  typedef typename Kernel::VertexVertexIter          VertexVertexIter;
  typedef typename Kernel::VertexOHalfedgeIter       VertexOHalfedgeIter;
  typedef typename Kernel::VertexIHalfedgeIter       VertexIHalfedgeIter;
  typedef typename Kernel::VertexEdgeIter            VertexEdgeIter;
  typedef typename Kernel::VertexFaceIter            VertexFaceIter;
  typedef typename Kernel::FaceVertexIter            FaceVertexIter;
  typedef typename Kernel::FaceHalfedgeIter          FaceHalfedgeIter;
  typedef typename Kernel::FaceEdgeIter              FaceEdgeIter;
  typedef typename Kernel::FaceFaceIter              FaceFaceIter;

  typedef typename Kernel::ConstVertexVertexIter     ConstVertexVertexIter;
  typedef typename Kernel::ConstVertexOHalfedgeIter  ConstVertexOHalfedgeIter;
  typedef typename Kernel::ConstVertexIHalfedgeIter  ConstVertexIHalfedgeIter;
  typedef typename Kernel::ConstVertexEdgeIter       ConstVertexEdgeIter;
  typedef typename Kernel::ConstVertexFaceIter       ConstVertexFaceIter;
  typedef typename Kernel::ConstFaceVertexIter       ConstFaceVertexIter;
  typedef typename Kernel::ConstFaceHalfedgeIter     ConstFaceHalfedgeIter;
  typedef typename Kernel::ConstFaceEdgeIter         ConstFaceEdgeIter;
  typedef typename Kernel::ConstFaceFaceIter         ConstFaceFaceIter;
  //@}


  // --- constructor/destructor
  PolyMeshT() {}
  virtual ~PolyMeshT() {}

  /** Uses default copy and assignment operator.
      Use them to assign two meshes of \b equal type.
      If the mesh types vary, use PolyMeshT::assign() instead. */

   // --- creation ---
  inline VertexHandle new_vertex()
  { return Kernel::new_vertex(); }

  inline VertexHandle new_vertex(const Point& _p)
  {
    VertexHandle vh(Kernel::new_vertex());
    this->set_point(vh, _p);
    return vh;
  }

  inline VertexHandle add_vertex(const Point& _p)
  { return new_vertex(_p); }

  // --- normal vectors ---

  /** \name Normal vector computation
  */
  //@{

  /** \brief Compute normals for all primitives
   *
   * Calls update_face_normals() , update_halfedge_normals()  and update_vertex_normals() if
   * the normals (i.e. the properties) exist.
   *
   * \note Face normals are required to compute vertex and halfedge normals!
   */
  void update_normals();

  /// Update normal for face _fh
  void update_normal(FaceHandle _fh)
  { this->set_normal(_fh, calc_face_normal(_fh)); }

  /** \brief Update normal vectors for all faces.
   *
   * \attention Needs the Attributes::Normal attribute for faces.
   *            Call request_face_normals() before using it!
   */
  void update_face_normals();

  /** Calculate normal vector for face _fh. */
  virtual Normal calc_face_normal(FaceHandle _fh) const;

  /** Calculate normal vector for face (_p0, _p1, _p2). */
  Normal calc_face_normal(const Point& _p0, const Point& _p1,
                                            const Point& _p2) const;
  /// calculates the average of the vertices defining _fh
  void calc_face_centroid(FaceHandle _fh, Point& _pt) const {
      _pt = calc_face_centroid(_fh);
  }

  /// Computes and returns the average of the vertices defining _gh
  Point calc_face_centroid(FaceHandle _fh) const;

  /// Update normal for halfedge _heh
  void update_normal(HalfedgeHandle _heh, const double _feature_angle = 0.8)
  { this->set_normal(_heh, calc_halfedge_normal(_heh)); }

  /** \brief Update normal vectors for all halfedges.
   *
   * Uses the existing face normals to compute halfedge normals
   *
   * \note Face normals have to be computed first!
   *
   * \attention Needs the Attributes::Normal attribute for faces and halfedges.
   *            Call request_face_normals() and request_halfedge_normals() before using it!
   */
  void update_halfedge_normals(const double _feature_angle = 0.8);

  /** \brief Calculate halfedge normal for one specific halfedge
   *
   * Calculate normal vector for halfedge _heh.
   *
   * \note Face normals have to be computed first!
   *
   * \attention Needs the Attributes::Normal attribute for faces and vertices.
   *            Call request_face_normals() and request_halfedge_normals() before using it!
   *
   * @param _heh Handle of the halfedge
   * @param _feature_angle If the dihedral angle across this edge is greater than this value, the edge is considered as a feature edge (angle in radians) 
   */
  virtual Normal calc_halfedge_normal(HalfedgeHandle _heh, const double _feature_angle = 0.8) const;


  /** identifies feature edges w.r.t. the minimal dihedral angle for feature edges (in radians) */
  /** and the status feature tag */
  bool is_estimated_feature_edge(HalfedgeHandle _heh, const double _feature_angle) const;

  /// Update normal for vertex _vh
  void update_normal(VertexHandle _vh)
  { this->set_normal(_vh, calc_vertex_normal(_vh)); }

  /** \brief Update normal vectors for all vertices.
   *
   * Uses existing face normals to calculate new vertex normals.
   *
   * \note Face normals have to be computed first!
   *
   * \attention Needs the Attributes::Normal attribute for faces and vertices.
   *            Call request_face_normals() and request_vertex_normals() before using it!
   */
  void update_vertex_normals();

  /** \brief Calculate vertex normal for one specific vertex
   *
   * Calculate normal vector for vertex _vh by averaging normals
   * of adjacent faces.
   *
   * \note Face normals have to be computed first!
   *
   * \attention Needs the Attributes::Normal attribute for faces and vertices.
   *            Call request_face_normals() and request_vertex_normals() before using it!
   *
   * @param _vh Handle of the vertex
   */
  Normal calc_vertex_normal(VertexHandle _vh) const;

  /** Different methods for calculation of the normal at _vh:
      - ..._fast    - the default one - the same as calc vertex_normal()
                    - needs the Attributes::Normal attribute for faces
      - ..._correct - works properly for non-triangular meshes
                    - does not need any attributes
      - ..._loop    - calculates loop surface normals
                    - does not need any attributes */
  void calc_vertex_normal_fast(VertexHandle _vh, Normal& _n) const;
  void calc_vertex_normal_correct(VertexHandle _vh, Normal& _n) const;
  void calc_vertex_normal_loop(VertexHandle _vh, Normal& _n) const;


  //@}

  // --- Geometry API - still in development ---

  /** Calculates the edge vector as the vector defined by
      the halfedge with id #0 (see below)  */
  void calc_edge_vector(EdgeHandle _eh, Normal& _edge_vec) const
  {
    _edge_vec = calc_edge_vector(_eh);
  }

  /** Calculates the edge vector as the vector defined by
      the halfedge with id #0 (see below)  */
  Normal calc_edge_vector(EdgeHandle _eh) const
  {
    return calc_edge_vector(this->halfedge_handle(_eh,0));
  }

  /** Calculates the edge vector as the difference of the
      the points defined by to_vertex_handle() and from_vertex_handle() */
  void calc_edge_vector(HalfedgeHandle _heh, Normal& _edge_vec) const
  {
    _edge_vec = calc_edge_vector(_heh);
  }

  /** Calculates the edge vector as the difference of the
      the points defined by to_vertex_handle() and from_vertex_handle() */
  Normal calc_edge_vector(HalfedgeHandle _heh) const
  {
    return this->point(this->to_vertex_handle(_heh)) -
            this->point(this->from_vertex_handle(_heh));
  }

  // Calculates the length of the edge _eh
  Scalar calc_edge_length(EdgeHandle _eh) const
  { return calc_edge_length(this->halfedge_handle(_eh,0)); }

  /** Calculates the length of the edge _heh
  */
  Scalar calc_edge_length(HalfedgeHandle _heh) const
  { return (Scalar)sqrt(calc_edge_sqr_length(_heh)); }

  Scalar calc_edge_sqr_length(EdgeHandle _eh) const
  { return calc_edge_sqr_length(this->halfedge_handle(_eh,0)); }

  Scalar calc_edge_sqr_length(HalfedgeHandle _heh) const
  {
    Normal edge_vec;
    calc_edge_vector(_heh, edge_vec);
    return edge_vec.sqrnorm();
  }

  /** defines a consistent representation of a sector geometry:
      the halfedge _in_heh defines the sector orientation
      the vertex pointed by _in_heh defines the sector center
      _vec0 and _vec1 are resp. the first and the second vectors defining the sector */
  void calc_sector_vectors(HalfedgeHandle _in_heh, Normal& _vec0, Normal& _vec1) const
  {
    calc_edge_vector(this->next_halfedge_handle(_in_heh), _vec0);//p2 - p1
    calc_edge_vector(this->opposite_halfedge_handle(_in_heh), _vec1);//p0 - p1
  }

  /** calculates the sector angle.\n
   * The vertex pointed by _in_heh defines the sector center
   * The angle will be calculated between the given halfedge and the next halfedge.\n
   * Seen from the center vertex this will be the next halfedge in clockwise direction.\n
      NOTE: only boundary concave sectors are treated correctly */
  Scalar calc_sector_angle(HalfedgeHandle _in_heh) const
  {
    Normal v0, v1;
    calc_sector_vectors(_in_heh, v0, v1);
    Scalar denom = v0.norm()*v1.norm();
    if (is_zero(denom))
    {
      return 0;
    }
    Scalar cos_a = dot(v0 , v1) / denom;
    if (this->is_boundary(_in_heh))
    {//determine if the boundary sector is concave or convex
      FaceHandle fh(this->face_handle(this->opposite_halfedge_handle(_in_heh)));
      Normal f_n(calc_face_normal(fh));//this normal is (for convex fh) OK
      Scalar sign_a = dot(cross(v0, v1), f_n);
      return angle(cos_a, sign_a);
    }
    else
    {
      return acos(sane_aarg(cos_a));
    }
  }

  // calculate the cos and the sin of angle <(_in_heh,next_halfedge(_in_heh))
  /*
  void calc_sector_angle_cos_sin(HalfedgeHandle _in_heh, Scalar& _cos_a, Scalar& _sin_a) const
  {
    Normal in_vec, out_vec;
    calc_edge_vector(_in_heh, in_vec);
    calc_edge_vector(next_halfedge_handle(_in_heh), out_vec);
    Scalar denom = in_vec.norm()*out_vec.norm();
    if (is_zero(denom))
    {
      _cos_a = 1;
      _sin_a = 0;
    }
    else
    {
      _cos_a = dot(in_vec, out_vec)/denom;
      _sin_a = cross(in_vec, out_vec).norm()/denom;
    }
  }
  */
  /** calculates the normal (non-normalized) of the face sector defined by
      the angle <(_in_heh,next_halfedge(_in_heh)) */
  void calc_sector_normal(HalfedgeHandle _in_heh, Normal& _sector_normal) const
  {
    Normal vec0, vec1;
    calc_sector_vectors(_in_heh, vec0, vec1);
    _sector_normal = cross(vec0, vec1);//(p2-p1)^(p0-p1)
  }

  /** calculates the area of the face sector defined by
      the angle <(_in_heh,next_halfedge(_in_heh))
      NOTE: special cases (e.g. concave sectors) are not handled correctly */
  Scalar calc_sector_area(HalfedgeHandle _in_heh) const
  {
    Normal sector_normal;
    calc_sector_normal(_in_heh, sector_normal);
    return sector_normal.norm()/2;
  }

  /** calculates the dihedral angle on the halfedge _heh
      \attention Needs the Attributes::Normal attribute for faces */
  Scalar calc_dihedral_angle_fast(HalfedgeHandle _heh) const
  {
    // Make sure that we have face normals on the mesh
    assert(Kernel::has_face_normals());

    if (this->is_boundary(this->edge_handle(_heh)))
    {//the dihedral angle at a boundary edge is 0
      return 0;
    }
    const Normal& n0 = this->normal(this->face_handle(_heh));
    const Normal& n1 = this->normal(this->face_handle(this->opposite_halfedge_handle(_heh)));
    Normal he;
    calc_edge_vector(_heh, he);
    Scalar da_cos = dot(n0, n1);
    //should be normalized, but we need only the sign
    Scalar da_sin_sign = dot(cross(n0, n1), he);
    return angle(da_cos, da_sin_sign);
  }

  /** calculates the dihedral angle on the edge _eh
      \attention Needs the Attributes::Normal attribute for faces */
  Scalar calc_dihedral_angle_fast(EdgeHandle _eh) const
  { return calc_dihedral_angle_fast(this->halfedge_handle(_eh,0)); }

  // calculates the dihedral angle on the halfedge _heh
  Scalar calc_dihedral_angle(HalfedgeHandle _heh) const
  {
    if (this->is_boundary(this->edge_handle(_heh)))
    {//the dihedral angle at a boundary edge is 0
      return 0;
    }
    Normal n0, n1, he;
    calc_sector_normal(_heh, n0);
    calc_sector_normal(this->opposite_halfedge_handle(_heh), n1);
    calc_edge_vector(_heh, he);
    Scalar denom = n0.norm()*n1.norm();
    if (denom == Scalar(0))
    {
      return 0;
    }
    Scalar da_cos = dot(n0, n1)/denom;
    //should be normalized, but we need only the sign
    Scalar da_sin_sign = dot(cross(n0, n1), he);
    return angle(da_cos, da_sin_sign);
  }

  // calculates the dihedral angle on the edge _eh
  Scalar calc_dihedral_angle(EdgeHandle _eh) const
  { return calc_dihedral_angle(this->halfedge_handle(_eh,0)); }

  /** tags an edge as a feature if its dihedral angle is larger than _angle_tresh
      returns the number of the found feature edges, requires edge_status property*/
  unsigned int find_feature_edges(Scalar _angle_tresh = OpenMesh::deg_to_rad(44.0));
  // --- misc ---

  /// Face split (= 1-to-n split)
  inline void split(FaceHandle _fh, const Point& _p)
  { Kernel::split(_fh, add_vertex(_p)); }

  inline void split(FaceHandle _fh, VertexHandle _vh)
  { Kernel::split(_fh, _vh); }

  inline void split(EdgeHandle _eh, const Point& _p)
  { Kernel::split_edge(_eh, add_vertex(_p)); }

  inline void split(EdgeHandle _eh, VertexHandle _vh)
  { Kernel::split_edge(_eh, _vh); }
  
};

/**
 * @brief Cast a mesh with different but identical traits into each other.
 *
 * Example:
 * @code{.cpp}
 * struct TriTraits1 : public OpenMesh::DefaultTraits {
 *   typedef Vec3d Point;
 * };
 * struct TriTraits2 : public OpenMesh::DefaultTraits {
 *   typedef Vec3d Point;
 * };
 * struct TriTraits3 : public OpenMesh::DefaultTraits {
 *   typedef Vec3f Point;
 * };
 *
 * TriMesh_ArrayKernelT<TriTraits1> a;
 * TriMesh_ArrayKernelT<TriTraits2> &b = mesh_cast<TriMesh_ArrayKernelT<TriTraits2>&>(a); // OK
 * TriMesh_ArrayKernelT<TriTraits3> &c = mesh_cast<TriMesh_ArrayKernelT<TriTraits3>&>(a); // ERROR
 * @endcode
 *
 * @see MeshCast
 *
 * @param rhs
 * @return
 */
template<typename LHS, typename KERNEL>
LHS mesh_cast(PolyMeshT<KERNEL> &rhs) {
    return MeshCast<LHS, PolyMeshT<KERNEL>&>::cast(rhs);
}

template<typename LHS, typename KERNEL>
LHS mesh_cast(PolyMeshT<KERNEL> *rhs) {
    return MeshCast<LHS, PolyMeshT<KERNEL>*>::cast(rhs);
}

template<typename LHS, typename KERNEL>
const LHS mesh_cast(const PolyMeshT<KERNEL> &rhs) {
    return MeshCast<LHS, const PolyMeshT<KERNEL>&>::cast(rhs);
}

template<typename LHS, typename KERNEL>
const LHS mesh_cast(const PolyMeshT<KERNEL> *rhs) {
    return MeshCast<LHS, const PolyMeshT<KERNEL>*>::cast(rhs);
}


//=============================================================================
} // namespace OpenMesh
//=============================================================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(OPENMESH_POLYMESH_C)
#  define OPENMESH_POLYMESH_TEMPLATES
#  include "PolyMeshT.cc"
#endif
//=============================================================================
#endif // OPENMESH_POLYMESHT_HH defined
//=============================================================================
