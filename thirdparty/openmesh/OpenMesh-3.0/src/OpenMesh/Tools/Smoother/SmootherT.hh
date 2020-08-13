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

/** \file SmootherT.hh

 */

//=============================================================================
//
//  CLASS SmootherT
//
//=============================================================================

#ifndef OPENMESH_SMOOTHER_SMOOTHERT_HH
#define OPENMESH_SMOOTHER_SMOOTHERT_HH


//== INCLUDES =================================================================

#include <OpenMesh/Core/System/config.hh>
#include <OpenMesh/Core/Utils/Property.hh>
#include <OpenMesh/Core/Utils/Noncopyable.hh>

//== FORWARDDECLARATIONS ======================================================

//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace Smoother {

//== CLASS DEFINITION =========================================================

/** Base class for smoothing algorithms.
 */
template <class Mesh>
class SmootherT : private Utils::Noncopyable
{
public:

  typedef typename Mesh::Scalar        Scalar;
  typedef typename Mesh::Point         Point;
  typedef typename Mesh::Normal        NormalType;
  typedef typename Mesh::VertexHandle  VertexHandle;
  typedef typename Mesh::EdgeHandle    EdgeHandle;

  // initialize smoother
  enum Component {
    Tangential,           ///< Smooth tangential direction
    Normal,               ///< Smooth normal direction
    Tangential_and_Normal ///< Smooth tangential and normal direction
  };

  enum Continuity {
    C0,
    C1,
    C2
  };

public:

  /** \brief constructor & destructor
   *
   * @param _mesh Reference to the mesh
   */
  SmootherT( Mesh& _mesh );
  virtual ~SmootherT();


public:

  /** Initialize smoother
   * \param _comp Determine component to smooth
   * \param _cont Determine Continuity
   */
  void initialize(Component _comp, Continuity _cont);


  //@{
  /// Set local error
  void set_relative_local_error(Scalar _err);
  void set_absolute_local_error(Scalar _err);
  void disable_local_error_check();
  //@}

  /** \brief enable or disable feature handling
   *
   * @param _state true  : If features are selected on the mesh, they will be left unmodified\n
   *               false : Features will be ignored
   */
  void skip_features( bool _state ){ skip_features_ = _state; };

  /// Do _n smoothing iterations
  virtual void smooth(unsigned int _n);



  /// Find active vertices. Resets tagged status !
  void set_active_vertices();


private:

  // single steps of smoothing
  void compute_new_positions();
  void project_to_tangent_plane();
  void local_error_check();
  void move_points();



protected:

  // override these
  virtual void compute_new_positions_C0() = 0;
  virtual void compute_new_positions_C1() = 0;



protected:

  // misc helpers

  const Point& orig_position(VertexHandle _vh) const
  { return mesh_.property(original_positions_, _vh); }

  const NormalType& orig_normal(VertexHandle _vh) const
  { return mesh_.property(original_normals_, _vh); }

  const Point& new_position(VertexHandle _vh) const
  { return mesh_.property(new_positions_, _vh); }

  void set_new_position(VertexHandle _vh, const Point& _p)
  { mesh_.property(new_positions_, _vh) = _p; }

  bool is_active(VertexHandle _vh) const
  { return mesh_.property(is_active_, _vh); }

  Component  component()  const { return component_;  }
  Continuity continuity() const { return continuity_; }

protected:

  Mesh&  mesh_;
  bool   skip_features_;


private:

  Scalar      tolerance_;
  Scalar      normal_deviation_;
  Component   component_;
  Continuity  continuity_;

  OpenMesh::VPropHandleT<Point>      original_positions_;
  OpenMesh::VPropHandleT<NormalType> original_normals_;
  OpenMesh::VPropHandleT<Point>      new_positions_;
  OpenMesh::VPropHandleT<bool>       is_active_;
};


//=============================================================================
} // namespace Smoother
} // namespace OpenMesh
//=============================================================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(OPENMESH_SMOOTHERT_C)
#define OPENMESH_SMOOTHERT_TEMPLATES
#include "SmootherT.cc"
#endif
//=============================================================================
#endif // OPENMESH_SMOOTHER_SMOOTHERT_HH defined
//=============================================================================

