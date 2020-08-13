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

/** \file Uniform/Composite/CompositeT.hh
    
 */

//=============================================================================
//
//  CLASS CompositeT
//
//=============================================================================

#ifndef OPENMESH_SUBDIVIDER_UNIFORM_COMPOSITE_HH
#define OPENMESH_SUBDIVIDER_UNIFORM_COMPOSITE_HH


//== INCLUDES =================================================================

#include <string>
#include <vector>
// --------------------
#include <OpenMesh/Tools/Subdivider/Uniform/SubdividerT.hh>

//== NAMESPACE ================================================================

namespace OpenMesh   { // BEGIN_NS_OPENMESH
namespace Subdivider { // BEGIN_NS_DECIMATER
namespace Uniform    { // BEGIN_NS_UNIFORM


//== CLASS DEFINITION =========================================================

/** This class provides the composite subdivision rules for the uniform case.
 *
 *  To create a subdivider derive from this class and overload the functions
 *  name() and apply_rules(). In the latter one call the wanted rules.
 *
 *  For details on the composite scheme refer to
 *  - <a
 *  href="http://cm.bell-labs.com/who/poswald/sqrt3.pdf">P. Oswald,
 *  P. Schroeder "Composite primal/dual sqrt(3)-subdivision schemes",
 *  CAGD 20, 3, 2003, 135--164</a>

 *  \note Not all rules are implemented!
 *  \see class Adaptive::CompositeT
 */
template <typename MeshType, typename RealType=float > 
class CompositeT : public SubdividerT< MeshType, RealType >
{
public:

  typedef RealType                                real_t;
  typedef MeshType                                mesh_t;
  typedef SubdividerT< mesh_t, real_t >           parent_t;

public:

  CompositeT(void) : parent_t(), p_mesh_(NULL) {}
  CompositeT(MeshType& _mesh) : parent_t(_mesh), p_mesh_(NULL) {};
  virtual ~CompositeT() { }

public: // inherited interface

  virtual const char *name( void ) const = 0;

protected: // inherited interface

  bool prepare( MeshType& _m );

  bool subdivide( MeshType& _m, size_t _n, const bool _update_points = true  )
  {
    assert( p_mesh_ == &_m );

    while(_n--)
    {
      apply_rules();
      commit(_m);
    }
   
    return true;
  }

#ifdef NDEBUG
  bool cleanup( MeshType& ) 
#else
  bool cleanup( MeshType& _m ) 
#endif
  { 
    assert( p_mesh_ == &_m );
    p_mesh_=NULL; 
    return true; 
  }

protected:

  /// Assemble here the rule sequence, by calling the constructor
  /// of the wanted rules.
  virtual void apply_rules(void) = 0;

protected:

  /// Move vertices to new positions after the rules have been applied
  /// to the mesh (called by subdivide()).
  void commit( MeshType &_m)
  {
    typename MeshType::VertexIter v_it;

    for (v_it=_m.vertices_begin(); v_it != _m.vertices_end(); ++v_it)
      _m.set_point(*v_it, _m.data(*v_it).position());
  }

  
public:

  /// Abstract base class for coefficient functions
  struct Coeff
  {
    virtual ~Coeff() { }
    virtual double operator() (size_t _valence) = 0;
  };


protected:

  typedef typename MeshType::Scalar         scalar_t;
  typedef typename MeshType::VertexHandle   VertexHandle;
  typedef typename MeshType::FaceHandle     FaceHandle;
  typedef typename MeshType::EdgeHandle     EdgeHandle;
  typedef typename MeshType::HalfedgeHandle HalfedgeHandle;

  /// \name Uniform composite subdivision rules
  //@{

  
  void Tvv3(); ///< Split Face, using Vertex information (1-3 split)
  void Tvv4(); ///< Split Face, using Vertex information (1-4 split)
  void Tfv();  ///< Split Face, using Face Information

  void FF();                 ///< Face to face averaging.
  void FFc(Coeff& _coeff);   ///< Weighted face to face averaging.
  void FFc(scalar_t _c);     ///< Weighted face to face averaging.

  void FV();                 ///< Face to vertex averaging.
  void FVc(Coeff& _coeff);   ///< Weighted face to vertex Averaging with flaps 
  void FVc(scalar_t _c);     ///< Weighted face to vertex Averaging with flaps 

  void FE();                 ///< Face to edge averaging.

  void VF();                 ///< Vertex to Face Averaging.
  void VFa(Coeff& _coeff);   ///< Vertex to Face Averaging, weighted.
  void VFa(scalar_t _alpha); ///< Vertex to Face Averaging, weighted.

  void VV();                  ///< Vertex to vertex averaging.
  void VVc(Coeff& _coeff);    ///< Vertex to vertex averaging, weighted.
  void VVc(scalar_t _c);      ///< Vertex to vertex averaging, weighted.

  void VE();                  ///< VE Step (Vertex to Edge Averaging)

  
  void VdE();             ///< Vertex to edge averaging, using diamond of edges.
  void VdEc(scalar_t _c); ///< Weighted vertex to edge averaging, using diamond of edges
  
  /// Weigthed vertex to edge averaging, using diamond of edges for
  /// irregular vertices.
  void VdEg(Coeff& _coeff);
  /// Weigthed vertex to edge averaging, using diamond of edges for
  /// irregular vertices.
  void VdEg(scalar_t _gamma);

  void EF();               ///< Edge to face averaging.
  
  void EV();               ///< Edge to vertex averaging.
  void EVc(Coeff& _coeff); ///< Weighted edge to vertex averaging.
  void EVc(scalar_t _c);   ///< Weighted edge to vertex averaging.

  void EdE();              ///< Edge to edge averaging w/ flap rule.
  void EdEc(scalar_t _c);  ///< Weighted edge to edge averaging w/ flap rule.


  //@}

  void corner_cutting(HalfedgeHandle _heh);

  VertexHandle split_edge(HalfedgeHandle _heh);

private:

  MeshType* p_mesh_;

};


//=============================================================================
} // END_NS_UNIFORM
} // END_NS_SUBDIVIDER
} // END_NS_OPENMESH
//=============================================================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(OPENMESH_SUBDIVIDER_UNIFORM_COMPOSITE_CC)
#define OPENMESH_SUBDIVIDER_TEMPLATES
#include "CompositeT.cc"
#endif
//=============================================================================
#endif // COMPOSITET_HH defined
//=============================================================================

