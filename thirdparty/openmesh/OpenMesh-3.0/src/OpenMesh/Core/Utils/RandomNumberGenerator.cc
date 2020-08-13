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
 *   $Revision: 362 $                                                         *
 *   $Date: 2011-01-26 10:21:12 +0100 (Mi, 26 Jan 2011) $                   *
 *                                                                           *
\*===========================================================================*/


//=============================================================================
//
//  Helper Functions for generating a random number between 0.0 and 1.0 with
//  a garantueed resolution
//
//=============================================================================


//== INCLUDES =================================================================


#include <OpenMesh/Core/Utils/RandomNumberGenerator.hh>
#include <cstdlib>


//== NAMESPACES ===============================================================


namespace OpenMesh {


//== IMPLEMENTATION ===========================================================

RandomNumberGenerator::RandomNumberGenerator(const size_t _resolution) :
  resolution_(_resolution),
  iterations_(1),
  maxNum_(RAND_MAX + 1.0)
{
  double tmp = double(resolution_);
  while (tmp > (double(RAND_MAX) + 1.0) ) {
    iterations_++;
    tmp /= (double(RAND_MAX) + 1.0);
  }

  for ( unsigned int i = 0 ; i < iterations_ - 1; ++i ) {
    maxNum_ *= (RAND_MAX + 1.0);
  }
}

//-----------------------------------------------------------------------------

double RandomNumberGenerator::getRand() const {
  double randNum = 0.0;
  for ( unsigned int i = 0 ; i < iterations_; ++i ) {
    randNum *= (RAND_MAX + 1.0);
    randNum += rand();
  }

  return randNum / maxNum_;
}

double RandomNumberGenerator::resolution() const {
  return maxNum_;
}

//=============================================================================
} // namespace OpenMesh
//=============================================================================
