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


#ifndef OPENMESH_VDPM_STREAMINGDEF_HH
#define OPENMESH_VDPM_STREAMINGDEF_HH

//== INCLUDES =================================================================

//== FORWARDDECLARATIONS ======================================================

//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace VDPM {


//== CLASS DEFINITION =========================================================

#define VDPM_STREAMING_PORT 4096

#define DEBUG_COUT
//#define QDATASTREAM

#ifdef DEBUG_COUT
static bool debug_print_;
static bool debug_print()               { return debug_print_; }
static void set_debug_print(bool flag)  { debug_print_ = flag; }
#endif

enum VDPMDownLinkStatus     { kStarted, kFinished, kStartable };
enum VDPMStreamingPhase     { kBaseMesh, kVSplitHeader, kVSplits };
enum VDPMClientMode         { kStatic, kDynamic };
enum VHierarchySearchMode   { kBruteForce, kUseHashing };


//=============================================================================
} // namespace VDPM
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_VDPROGMESH_VDPMSTREAMINGDEF_HH defined
//=============================================================================


