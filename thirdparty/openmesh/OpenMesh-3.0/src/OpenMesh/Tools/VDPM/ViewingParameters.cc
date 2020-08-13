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
//  CLASS newClass - IMPLEMENTATION
//
//=============================================================================


//== INCLUDES =================================================================

#include <OpenMesh/Tools/VDPM/ViewingParameters.hh>

//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace VDPM {


//== IMPLEMENTATION ========================================================== 


ViewingParameters::
ViewingParameters()
{
  fovy_ = 45.0f;
  aspect_ = 1.0f;
  tolerance_square_ = 0.001f;
}

void
ViewingParameters::
update_viewing_configurations()
{
  // |a11 a12 a13|-1       |  a33a22-a32a23  -(a33a12-a32a13)   a23a12-a22a13 |
  // |a21 a22 a23| = 1/DET*|-(a33a21-a31a23)   a33a11-a31a13  -(a23a11-a21a13)|
  // |a31 a32 a33|         |  a32a21-a31a22  -(a32a11-a31a12)   a22a11-a21a12 |
  //  DET  =  a11(a33a22-a32a23)-a21(a33a12-a32a13)+a31(a23a12-a22a13)

  float   invdet;  
  float   a11, a12, a13, a21, a22, a23, a31, a32, a33;
  Vec3f inv_rot[3], trans;

  a11      = (float) modelview_matrix_[0]; 
  a12      = (float) modelview_matrix_[4]; 
  a13      = (float) modelview_matrix_[8];     
  trans[0] = (float) modelview_matrix_[12];

  a21      = (float) modelview_matrix_[1];
  a22      = (float) modelview_matrix_[5];
  a23      = (float) modelview_matrix_[9];
  trans[1] = (float) modelview_matrix_[13];
  
  a31      = (float) modelview_matrix_[2];
  a32      = (float) modelview_matrix_[6];
  a33      = (float) modelview_matrix_[10];
  trans[2] = (float) modelview_matrix_[14];

  invdet=a11*(a33*a22-a32*a23) - a21*(a33*a12-a32*a13) + a31*(a23*a12-a22*a13);
  invdet= (float) 1.0/invdet;

  (inv_rot[0])[0] =  (a33*a22-a32*a23) * invdet;
  (inv_rot[0])[1] = -(a33*a12-a32*a13) * invdet;
  (inv_rot[0])[2] =  (a23*a12-a22*a13) * invdet;
  (inv_rot[1])[0] = -(a33*a21-a31*a23) * invdet;
  (inv_rot[1])[1] =  (a33*a11-a31*a13) * invdet;
  (inv_rot[1])[2] = -(a23*a11-a21*a13) * invdet;
  (inv_rot[2])[0] =  (a32*a21-a31*a22) * invdet;
  (inv_rot[2])[1] = -(a32*a11-a31*a12) * invdet;
  (inv_rot[2])[2] =  (a22*a11-a21*a12) * invdet;

  eye_pos_   = - Vec3f(dot(inv_rot[0], trans), 
		      dot(inv_rot[1], trans), 
		      dot(inv_rot[2], trans));
  right_dir_ =   Vec3f(a11, a12, a13);
  up_dir_    =   Vec3f(a21, a22, a23);
  view_dir_  = - Vec3f(a31, a32, a33);

  Vec3f normal[4];  
  //float	aspect = width() / height();
  float	half_theta = fovy() * 0.5f;
  float	half_phi = atanf(aspect() * tanf(half_theta));
  
  float sin1 = sinf(half_theta);
  float cos1 = cosf(half_theta);
  float sin2 = sinf(half_phi);
  float cos2 = cosf(half_phi);
  
  normal[0] =  cos2 * right_dir_ + sin2 * view_dir_;
  normal[1] = -cos1 * up_dir_    - sin1 * view_dir_;
  normal[2] = -cos2 * right_dir_ + sin2 * view_dir_;
  normal[3] =  cos1 * up_dir_    - sin1 * view_dir_;

  for (int i=0; i<4; i++)
    frustum_plane_[i] = Plane3d(normal[i], eye_pos_);
}

void
ViewingParameters::
PrintOut()
{
  std::cout << "  ModelView matrix: " << std::endl;
  std::cout << "    |" << modelview_matrix_[0] << " " << modelview_matrix_[4] << " " << modelview_matrix_[8] << " " << modelview_matrix_[12] << "|" << std::endl;
  std::cout << "    |" << modelview_matrix_[1] << " " << modelview_matrix_[5] << " " << modelview_matrix_[9] << " " << modelview_matrix_[13] << "|" << std::endl;
  std::cout << "    |" << modelview_matrix_[2] << " " << modelview_matrix_[6] << " " << modelview_matrix_[10] << " " << modelview_matrix_[14] << "|" << std::endl;
  std::cout << "    |" << modelview_matrix_[3] << " " << modelview_matrix_[7] << " " << modelview_matrix_[11] << " " << modelview_matrix_[15] << "|" << std::endl; 
  std::cout << "  Fovy: " << fovy_ << std::endl;
  std::cout << "  Aspect: " << aspect_ << std::endl;
  std::cout << "  Tolerance^2: " << tolerance_square_ << std::endl;
  std::cout << "  Eye Pos: " << eye_pos_ << std::endl;
  std::cout << "  Right dir: " << right_dir_ << std::endl;
  std::cout << "  Up dir: " << up_dir_ << std::endl;
  std::cout << "  View dir: " << view_dir_ << std::endl;
}

//=============================================================================
} // namespace VDPM
} // namespace OpenMesh
//=============================================================================
