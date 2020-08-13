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

/** \file JacobiLaplaceSmootherT.hh
    
 */


//=============================================================================
//
//  CLASS JacobiLaplaceSmootherT
//
//=============================================================================

#ifndef OPENMESH_JACOBI_LAPLACE_SMOOTHERT_HH
#define OPENMESH_JACOBI_LAPLACE_SMOOTHERT_HH


//== INCLUDES =================================================================

#include <OpenMesh/Tools/Smoother/LaplaceSmootherT.hh>


//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace Smoother {

//== CLASS DEFINITION =========================================================

/** Laplacian Smoothing.
 *
 */
template <class Mesh>
class JacobiLaplaceSmootherT : public LaplaceSmootherT<Mesh>
{
private:
  typedef LaplaceSmootherT<Mesh>            Base;
  
public:

  JacobiLaplaceSmootherT( Mesh& _mesh ) : LaplaceSmootherT<Mesh>(_mesh) {}

  // override: alloc umbrellas
  void smooth(unsigned int _n);


protected:

  virtual void compute_new_positions_C0();
  virtual void compute_new_positions_C1();


private:

  OpenMesh::VPropHandleT<typename Mesh::Normal>   umbrellas_;
  OpenMesh::VPropHandleT<typename Mesh::Normal>   squared_umbrellas_;
};


//=============================================================================
} // namespace Smoother
} // namespace OpenMesh
//=============================================================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(OPENMESH_JACOBI_LAPLACE_SMOOTHERT_C)
#define OPENMESH_JACOBI_LAPLACE_SMOOTHERT_TEMPLATES
#include "JacobiLaplaceSmootherT.cc"
#endif
//=============================================================================
#endif // OPENMESH_JACOBI_LAPLACE_SMOOTHERT_HH defined
//=============================================================================

