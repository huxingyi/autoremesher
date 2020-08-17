/*
 *  Copyright (c) 2012-2014, Bruno Levy
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  * Neither the name of the ALICE Project-Team nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     Bruno.Levy@inria.fr
 *     http://www.loria.fr/~levy
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 */
#include <geogram/points/principal_axes.h>

namespace GEO {

    PrincipalAxes3d::PrincipalAxes3d() {
    }

    void PrincipalAxes3d::begin() {
	nb_points_ = 0;
	sum_weights_ = 0;
	center_[0] = 0.0;
	center_[1] = 0.0;
	center_[2] = 0.0;
	M_[0] = M_[1] = M_[2] = M_[3] = M_[4] = M_[5] = 0.0;
    }

    void PrincipalAxes3d::end() {
	center_[0] /= sum_weights_ ;
	center_[1] /= sum_weights_ ;
	center_[2] /= sum_weights_ ;
	    
	// If the system is under-determined, 
	//   return the trivial basis.
	if(nb_points_ < 4) {
	    axis_[0] = vec3(1,0,0) ;
	    axis_[1] = vec3(0,1,0) ;
	    axis_[2] = vec3(0,0,1) ;
	    eigen_value_[0] = 1.0 ;
	    eigen_value_[1] = 1.0 ;
	    eigen_value_[2] = 1.0 ;
	} else {
	    double x = center_[0] ;
	    double y = center_[1] ;
	    double z = center_[2] ;
		
	    M_[0] = M_[0]/sum_weights_ - x*x ;
	    M_[1] = M_[1]/sum_weights_ - x*y ;
	    M_[2] = M_[2]/sum_weights_ - y*y ;
	    M_[3] = M_[3]/sum_weights_ - x*z ;
	    M_[4] = M_[4]/sum_weights_ - y*z ;
	    M_[5] = M_[5]/sum_weights_ - z*z ;
		
	    if( M_[0] <= 0 ) {
		M_[0] = 1.e-30 ; 
	    }
	    if( M_[2] <= 0 ) {
		M_[2] = 1.e-30 ; 
	    }
	    if( M_[5] <= 0 ) {
		M_[5] = 1.e-30 ; 
	    }
		
	    double eigen_vectors[9] ;
	    MatrixUtil::semi_definite_symmetric_eigen(
		M_, 3, eigen_vectors, eigen_value_
	    ) ;
		
	    axis_[0] = vec3(
		eigen_vectors[0], eigen_vectors[1], eigen_vectors[2]
	    );
		
	    axis_[1] = vec3(
		eigen_vectors[3], eigen_vectors[4], eigen_vectors[5]
	    );
        
	    axis_[2] = vec3(
		eigen_vectors[6], eigen_vectors[7], eigen_vectors[8]
	    );
        
	    // Normalize the eigen vectors
		
	    for(int i=0; i<3; i++) {
		axis_[i] = normalize(axis_[i]) ;
	    }
	}
    }

    void PrincipalAxes3d::add_point(const vec3& p, double weight) {
	center_[0] += p.x * weight ;
	center_[1] += p.y * weight ;
	center_[2] += p.z * weight ;
	    
	double x = p.x ;
	double y = p.y ; 
	double z = p.z ;
	    
	M_[0] += weight * x*x ;
	M_[1] += weight * x*y ;
	M_[2] += weight * y*y ;
	M_[3] += weight * x*z ;
	M_[4] += weight * y*z ;
	M_[5] += weight * z*z ;
	    
	nb_points_++ ;
	sum_weights_ += weight ;
    }
    
    
}




