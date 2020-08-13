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

#ifndef OPENMESH_VDPROGMESH_VIEWINGPARAMETERS_HH
#define OPENMESH_VDPROGMESH_VIEWINGPARAMETERS_HH


//== INCLUDES =================================================================

#include <OpenMesh/Core/Geometry/VectorT.hh>
#include <OpenMesh/Core/Geometry/Plane3d.hh>


//== FORWARDDECLARATIONS ======================================================


//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace VDPM {

//== CLASS DEFINITION =========================================================

	      
/** \todo ViewerParameters documentation
 */
class OPENMESHDLLEXPORT ViewingParameters
{
private:
  double    modelview_matrix_[16];
  float     fovy_;
  float     aspect_;
  float     tolerance_square_;

  Vec3f   eye_pos_;
  Vec3f   right_dir_;
  Vec3f   up_dir_;
  Vec3f   view_dir_;

  Plane3d           frustum_plane_[4];

public:

  ViewingParameters();

  void increase_tolerance()           { tolerance_square_ *= 5.0f; }
  void decrease_tolerance()           { tolerance_square_ /= 5.0f; }  

  float fovy() const                  { return  fovy_; }
  float aspect() const                { return  aspect_; }
  float tolerance_square() const      { return  tolerance_square_; } 
  
  void set_fovy(float _fovy)                            { fovy_ = _fovy; }
  void set_aspect(float _aspect)                        { aspect_ = _aspect; }
  void set_tolerance_square(float _tolerance_square)    { tolerance_square_ = _tolerance_square; }

  const Vec3f& eye_pos() const    { return eye_pos_; }
  const Vec3f& right_dir() const  { return right_dir_; }
  const Vec3f& up_dir() const     { return up_dir_; }
  const Vec3f& view_dir() const   { return view_dir_; }
  Vec3f& eye_pos()                { return eye_pos_; }
  Vec3f& right_dir()              { return right_dir_; }
  Vec3f& up_dir()                 { return up_dir_; }
  Vec3f& view_dir()               { return view_dir_; }

  void frustum_planes( Plane3d _plane[4] )
  {
    for (unsigned int i=0; i<4; ++i)
      _plane[i] = frustum_plane_[i];
  }
   
  void get_modelview_matrix(double _modelview_matrix[16])  
  {
    for (unsigned int i=0; i<16; ++i)
      _modelview_matrix[i] = modelview_matrix_[i];
  }  

  void set_modelview_matrix(const double _modelview_matrix[16])
  {
    for (unsigned int i=0; i<16; ++i)
      modelview_matrix_[i] = _modelview_matrix[i];   
  }

  void update_viewing_configurations();

  void PrintOut();
};


//=============================================================================
} // namespace VDPM
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_VDPROGMESH_VIEWINGPARAMETERS_HH defined
//=============================================================================

