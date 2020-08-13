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

/** \file ModProgMeshT.hh

 */

//=============================================================================
//
//  CLASS ModProgMeshT
//
//=============================================================================

#ifndef OPENMESH_TOOLS_MODPROGMESHT_HH
#define OPENMESH_TOOLS_MODPROGMESHT_HH


//== INCLUDES =================================================================

#include <OpenMesh/Tools/Decimater/ModBaseT.hh>
#include <OpenMesh/Core/Utils/Property.hh>


//== NAMESPACE ================================================================

namespace OpenMesh  {
namespace Decimater {


//== CLASS DEFINITION =========================================================


/** Collect progressive mesh information while decimating.
 *
 *  The progressive mesh data is stored in an internal structure, which
 *  can be evaluated after the decimation process and (!) before calling
 *  the garbage collection of the decimated mesh.
 */
template <class MeshT>
class ModProgMeshT : public ModBaseT<MeshT>
{
public:

  DECIMATING_MODULE( ModProgMeshT, MeshT, ProgMesh );

  /** Struct storing progressive mesh information
   *  \see CollapseInfoT, ModProgMeshT
   */
  struct Info
  {
    /// Initializing constructor copies appropriate handles from
    /// collapse information \c _ci.
    Info( const CollapseInfo& _ci )
      : v0(_ci.v0), v1(_ci.v1), vl(_ci.vl),vr(_ci.vr)
    {}

    typename Mesh::VertexHandle v0; ///< See CollapseInfoT::v0
    typename Mesh::VertexHandle v1; ///< See CollapseInfoT::v1
    typename Mesh::VertexHandle vl; ///< See CollapseInfoT::vl
    typename Mesh::VertexHandle vr; ///< See CollapseInfoT::vr

  };

  /// Type of the list storing the progressive mesh info Info.
  typedef std::vector<Info>           InfoList;


public:

   /// Constructor
  ModProgMeshT( MeshT &_mesh ) : Base(_mesh, true)
  {
    Base::mesh().add_property( idx_ );
  }


  /// Destructor
  ~ModProgMeshT()
  {
    Base::mesh().remove_property( idx_ );
  }

  const InfoList&                            pmi() const
  {
    return pmi_;
  }

public: // inherited


  /// Stores collapse information in a queue.
  /// \see infolist()
  void postprocess_collapse(const CollapseInfo& _ci)
  {
    pmi_.push_back( Info( _ci ) );
  }


  bool is_binary(void) const { return true; }


public: // specific methods

  /** Write progressive mesh data to a file in proprietary binary format .pm.
   *
   *  The methods uses the collected data to write a progressive mesh
   *  file. It's a binary format with little endian byte ordering:
   *
   *  - The first 8 bytes contain the word "ProgMesh".
   *  - 32-bit int for the number of vertices \c NV in the base mesh.
   *  - 32-bit int for the number of faces in the base mesh.
   *  - 32-bit int for the number of halfedge collapses (now vertex splits)
   *  - Positions of vertices of the base mesh (32-bit float triplets).<br>
   *    \c [x,y,z][x,y,z]...
   *  - Triplets of indices (32-bit int) for each triangle (index in the
   *    list of vertices of the base mesh defined by the positions.<br>
   *    \c [v0,v1,v2][v0,v1,v2]...
   *  - For each collapse/split a detail information package made of
   *    3 32-bit floats for the positions of vertex \c v0, and 3 32-bit
   *    int indices for \c v1, \c vl, and \c vr.
   *    The index for \c vl or \c vr might be -1, if the face on this side
   *    of the edge does not exists.
   *
   *  \remark Write file before calling the garbage collection of the mesh.
   *  \param _ofname Name of the file, where to write the progressive mesh
   *  \return \c true on success of the operation, else \c false.
   */
  bool write( const std::string& _ofname );
  /// Reference to collected information
  const InfoList& infolist() const { return pmi_; }

private:

  // hide this method form user
  void set_binary(bool _b) {}

  InfoList          pmi_;
  VPropHandleT<int> idx_;
};


//=============================================================================
} // END_NS_DECIMATER
} // END_NS_OPENMESH
//=============================================================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(OPENMESH_DECIMATER_MODPROGMESH_CC)
#define OSG_MODPROGMESH_TEMPLATES
#include "ModProgMeshT.cc"
#endif
//=============================================================================
#endif // OPENMESH_TOOLS_PROGMESHT_HH defined
//=============================================================================

