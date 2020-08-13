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
//  CLASS ModQuadricT
//
//=============================================================================

#ifndef OSG_MODQUADRIC_HH
#define OSG_MODQUADRIC_HH


//== INCLUDES =================================================================

#include <float.h>
#include <OpenMesh/Tools/Decimater/ModBaseT.hh>
#include <OpenMesh/Core/Utils/Property.hh>
#include <OpenMesh/Core/Utils/vector_cast.hh>
#include <OpenMesh/Core/Geometry/QuadricT.hh>


//== NAMESPACE ================================================================

namespace OpenMesh  {
namespace Decimater {


//== CLASS DEFINITION =========================================================


/** \brief Mesh decimation module computing collapse priority based on error quadrics.
 *
 *  This module can be used as a binary and non-binary module.
 */
template <class MeshT>
class ModQuadricT : public ModBaseT<MeshT>
{
public:

  // Defines the types Self, Handle, Base, Mesh, and CollapseInfo
  // and the memberfunction name()
  DECIMATING_MODULE( ModQuadricT, MeshT, Quadric );

public:

  /** Constructor
   *  \internal
   */
  ModQuadricT( MeshT &_mesh )
    : Base(_mesh, false)
  {
    unset_max_err();
    Base::mesh().add_property( quadrics_ );
  }


  /// Destructor
  virtual ~ModQuadricT()
  {
    Base::mesh().remove_property(quadrics_);
  }


public: // inherited

  /// Initalize the module and prepare the mesh for decimation.
  virtual void initialize(void);

  /** Compute collapse priority based on error quadrics.
   *
   *  \see ModBaseT::collapse_priority() for return values
   *  \see set_max_err()
   */
  virtual float collapse_priority(const CollapseInfo& _ci)
  {
    using namespace OpenMesh;

    typedef Geometry::QuadricT<double> Q;

    Q q = Base::mesh().property(quadrics_, _ci.v0);
    q += Base::mesh().property(quadrics_, _ci.v1);

    double err = q(_ci.p1);

    //min_ = std::min(err, min_);
    //max_ = std::max(err, max_);

    //double err = q( p );

    return float( (err < max_err_) ? err : float( Base::ILLEGAL_COLLAPSE ) );
  }


  /// Post-process halfedge collapse (accumulate quadrics)
  virtual void postprocess_collapse(const CollapseInfo& _ci)
  {
    Base::mesh().property(quadrics_, _ci.v1) +=
      Base::mesh().property(quadrics_, _ci.v0);
  }

  /// set the percentage of maximum quadric error
  void set_error_tolerance_factor(double _factor);



public: // specific methods

  /** Set maximum quadric error constraint and enable binary mode.
   *  \param _err    Maximum error allowed
   *  \param _binary Let the module work in non-binary mode in spite of the
   *                 enabled constraint.
   *  \see unset_max_err()
   */
  void set_max_err(double _err, bool _binary=true)
  {
    max_err_ = _err;
    Base::set_binary(_binary);
  }

  /// Unset maximum quadric error constraint and restore non-binary mode.
  /// \see set_max_err()
  void unset_max_err(void)
  {
    max_err_ = DBL_MAX;
    Base::set_binary(false);
  }

  /// Return value of max. allowed error.
  double max_err() const { return max_err_; }


private:

  // maximum quadric error
  double max_err_;

  // this vertex property stores a quadric for each vertex
  VPropHandleT< Geometry::QuadricT<double> >  quadrics_;
};

//=============================================================================
} // END_NS_DECIMATER
} // END_NS_OPENMESH
//=============================================================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(OPENMESH_DECIMATER_MODQUADRIC_CC)
#define OSG_MODQUADRIC_TEMPLATES
#include "ModQuadricT.cc"
#endif
//=============================================================================
#endif // OSG_MODQUADRIC_HH defined
//=============================================================================

