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

/** \file SubdividerT.hh
    
 */

//=============================================================================
//
//  CLASS SubdividerT
//
//=============================================================================

#ifndef OPENMESH_SUBDIVIDER_UNIFORM_SUDIVIDERT_HH
#define OPENMESH_SUBDIVIDER_UNIFORM_SUDIVIDERT_HH

//== INCLUDE ==================================================================

#include <OpenMesh/Core/System/config.hh>
#include <OpenMesh/Core/Utils/Noncopyable.hh>
#if defined(_DEBUG) || defined(DEBUG)
// Makes life lot easier, when playing/messing around with low-level topology
// changing methods of OpenMesh
#  include <OpenMesh/Tools/Utils/MeshCheckerT.hh>
#  define ASSERT_CONSISTENCY( T, m ) \
     assert(OpenMesh::Utils::MeshCheckerT<T>(m).check())
#else
#  define ASSERT_CONSISTENCY( T, m )
#endif

//== NAMESPACE ================================================================

namespace OpenMesh   {
namespace Subdivider {
namespace Uniform    {


//== CLASS DEFINITION =========================================================

/** Abstract base class for uniform subdivision algorithms.
 *
 *  A derived class must overload the following functions:
 *  -# name()
 *  -# prepare()
 *  -# subdivide()
 *  -# cleanup()
 */
template <typename MeshType, typename RealType=float>
class SubdividerT : private Utils::Noncopyable
{
public:

  typedef MeshType mesh_t;
  typedef RealType real_t;

public:

  /// \name Constructors
  //@{
  /// Constructor to be used with interface 2
  /// \see attach(), operator()(size_t), detach()
  SubdividerT(void) : attached_(NULL) { }

  /// Constructor to be used with interface 1 (calls attach())
  /// \see operator()( MeshType&, size_t )
  SubdividerT( MeshType &_m ) : attached_(NULL) {  attach(_m); }

  //@}

  /// Descructor (calls detach())
  virtual ~SubdividerT() 
  { detach(); }

  /// Return name of subdivision algorithm
  virtual const char *name( void ) const = 0;


public: /// \name Interface 1

  //@{
  /// Subdivide the mesh \c _m \c _n times.
  /// \see SubdividerT(MeshType&)
  bool operator () ( MeshType& _m, size_t _n , const bool _update_points  = true)
  {    
    return prepare(_m) && subdivide( _m, _n , _update_points ) && cleanup( _m );
  }
  //@}

public: /// \name Interface 2
  //@{
  /// Attach mesh \c _m to self
  /// \see SubdividerT(), operator()(size_t), detach()
  bool attach( MeshType& _m )
  {
    if ( attached_ == &_m )
      return true;

    detach();
    if (prepare( _m ))
    {
      attached_ = &_m;
      return true;
    }
    return false;
  }

  /// Subdivide the attached \c _n times.
  /// \see SubdividerT(), attach(), detach()
  bool operator()( size_t _n , const bool _update_points = true)
  {
    return attached_ ? subdivide( *attached_, _n , _update_points) : false;
  }

  /// Detach an eventually attached mesh.
  /// \see SubdividerT(), attach(), operator()(size_t)
  void detach(void)
  {
    if ( attached_ )
    {
      cleanup( *attached_ );
      attached_ = NULL;
    }
  }
  //@}

protected: 

  /// \name Overload theses methods
  //@{
  /// Prepare mesh, e.g. add properties
  virtual bool prepare( MeshType& _m ) = 0;

  /// Subdivide mesh \c _m \c _n times
  virtual bool subdivide( MeshType& _m, size_t _n, const bool _update_points = true) = 0;

  /// Cleanup mesh after usage, e.g. remove added properties
  virtual bool cleanup( MeshType& _m ) = 0;
  //@}

private:
 
  MeshType *attached_;

};

//=============================================================================
} // namespace Uniform
} // namespace Subdivider
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_SUBDIVIDER_UNIFORM_SUBDIVIDERT_HH
//=============================================================================
