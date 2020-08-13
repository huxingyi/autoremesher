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
 *   $Revision: 693 $                                                         *
 *   $Date: 2012-09-23 16:25:16 +0200 (So, 23 Sep 2012) $                   *
 *                                                                           *
\*===========================================================================*/


//=============================================================================
//
//  Helper Functions for generating a random number between 0.0 and 1.0 with
//  a garantueed resolution
//
//=============================================================================


#ifndef OPENMESH_UTILS_RANDOMNUMBERGENERATOR_HH
#define OPENMESH_UTILS_RANDOMNUMBERGENERATOR_HH


//== INCLUDES =================================================================


#include <OpenMesh/Core/System/config.h>
#include <cstdlib>



//== NAMESPACES ===============================================================


namespace OpenMesh {


//=============================================================================


/**  Generate a random number between 0.0 and 1.0 with a guaranteed resolution
 *   ( Number of possible values )
 *
 * Especially useful on windows, as there MAX_RAND is often only 32k which is
 * not enough resolution for a lot of applications
 */
class OPENMESHDLLEXPORT RandomNumberGenerator
{
public:

  /** \brief Constructor
  *
  * @param _resolution specifies the desired resolution for the random number generated
  */
  RandomNumberGenerator(const size_t _resolution);

  /// returns a random double between 0.0 and 1.0 with a guaranteed resolution
  double getRand() const;

  double resolution() const;

private:

  /// desired resolution
  const size_t resolution_;

  /// number of "blocks" of RAND_MAX that make up the desired _resolution
  size_t iterations_;

  /// maximum random number generated, which is used for normalization
  double maxNum_;
};

//=============================================================================
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_UTILS_RANDOMNUMBERGENERATOR_HH defined
//=============================================================================

