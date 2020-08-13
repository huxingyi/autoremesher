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
 *   $Revision: 520 $                                                         *
 *   $Date: 2012-01-20 15:29:31 +0100 (Fr, 20 Jan 2012) $                   *
 *                                                                           *
\*===========================================================================*/

/** \file CatmullClarkT.hh
 */

//=============================================================================
//
//  CLASS CatmullClarkT
//
//=============================================================================


#ifndef OPENMESH_SUBDIVIDER_UNIFORM_CATMULLCLARKT_HH
#define OPENMESH_SUBDIVIDER_UNIFORM_CATMULLCLARKT_HH


//== INCLUDES =================================================================

#include <OpenMesh/Tools/Subdivider/Uniform/SubdividerT.hh>

// -------------------- STL
#if defined(OM_CC_MIPS)
#  include <math.h>
#else
#  include <cmath>
#endif

//== FORWARDDECLARATIONS ======================================================

//== NAMESPACES ===============================================================

namespace OpenMesh   { // BEGIN_NS_OPENMESH
namespace Subdivider { // BEGIN_NS_SUBVIDER
namespace Uniform    { // BEGIN_NS_UNIFORM

//== CLASS DEFINITION =========================================================


/** \class CatmullClarkT CatmullClarkT.hh
  Based on code from Leon Kos, CAD lab, Mech.Eng., University of Ljubljana, Slovenia
  (http://www.lecad.fs.uni-lj.si/~leon)

  \note Needs a PolyMesh to work on!
*/
template <typename MeshType, typename RealType = float>
class CatmullClarkT : public SubdividerT< MeshType, RealType >
{
public:

  typedef typename MeshType::FaceHandle             FaceHandle;
  typedef typename MeshType::VertexHandle           VertexHandle;
  typedef typename MeshType::EdgeHandle             EdgeHandle;
  typedef typename MeshType::HalfedgeHandle         HalfedgeHandle;

  typedef typename MeshType::Point                  Point;
  typedef typename MeshType::Normal                 Normal;
  typedef typename MeshType::FaceIter               FaceIter;
  typedef typename MeshType::EdgeIter               EdgeIter;
  typedef typename MeshType::VertexIter             VertexIter;

  typedef typename MeshType::VertexEdgeIter         VertexEdgeIter;
  typedef typename MeshType::VertexFaceIter         VertexFaceIter;

  typedef typename MeshType::VOHIter                VOHIter;

  typedef SubdividerT< MeshType, RealType >           parent_t;

  /// Constructor
  CatmullClarkT(  ) : parent_t() {  }

  /// Constructor
  CatmullClarkT(MeshType &_m) : parent_t(_m) {  }

  virtual ~CatmullClarkT() {}

public:

  const char *name() const { return "Uniform CatmullClark"; }

protected:

  /// Initialize properties and weights
  virtual bool prepare( MeshType& _m );

  /// Remove properties and weights
  virtual bool cleanup( MeshType& _m );

  /** \brief Execute n subdivision steps
     *
     * @param _m Mesh to work on
     * @param _n Number of iterations
     * @param _update_points Unused here
     * @return successful?
     */
  virtual bool subdivide( MeshType& _m, size_t _n , const bool _update_points = true);

private:

  //===========================================================================
  /** @name Topology helpers
   * @{ */
  //===========================================================================

  void split_edge( MeshType& _m, const EdgeHandle& _eh);

  void split_face( MeshType& _m, const FaceHandle& _fh);

  void compute_midpoint( MeshType& _m, const EdgeHandle& _eh);

  void update_vertex(MeshType& _m, const  VertexHandle& _vh);

  /** @} */


private:
  OpenMesh::VPropHandleT< Point > vp_pos_; // next vertex pos
  OpenMesh::EPropHandleT< Point > ep_pos_; // new edge pts
  OpenMesh::FPropHandleT< Point > fp_pos_; // new face pts
  OpenMesh::EPropHandleT<double> creaseWeights_;// crease weights

};


//=============================================================================
} // END_NS_UNIFORM
} // END_NS_SUBDIVIDER
} // END_NS_OPENMESH
//=============================================================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(OPENMESH_SUBDIVIDER_UNIFORM_CATMULLCLARK_CC)
#  define OPENMESH_SUBDIVIDER_TEMPLATES
#  include "CatmullClarkT.cc"
#endif
//=============================================================================
#endif // OPENMESH_SUBDIVIDER_UNIFORM_CATMULLCLARKT_HH defined
//=============================================================================

