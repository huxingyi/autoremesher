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

#ifndef GEOGRAM_DELAUNAY_PERIODIC
#define GEOGRAM_DELAUNAY_PERIODIC

#include <geogram/basic/common.h>
#include <geogram/basic/string.h>
#include <geogram/basic/assert.h>

/**
 * \file geogram/delaunay/periodic.h
 * \brief Manipulation of indices for 3D periodic space.
 */

namespace GEO {

     /**
      * \brief Utilities for managing 3D periodic space.
      */
     class GEOGRAM_API Periodic {
       public:

	 /**
	  * \brief Gets the instance from a periodic vertex.
	  * \return the instance in 0..26
	  */
	 index_t periodic_vertex_instance(index_t pv) const {
	     geo_debug_assert(pv < nb_vertices_non_periodic_ * 27);
	     return pv / nb_vertices_non_periodic_;
	 }

	 /**
	  * \brief Gets the real vertex from a periodic vertex.
	  * \return the real vertex, in 0..nb_vertices_non_periodic_-1
	  */
	 index_t periodic_vertex_real(index_t pv) const {
	     geo_debug_assert(pv < nb_vertices_non_periodic_ * 27);	     
	     return pv % nb_vertices_non_periodic_;
	 }

	 /**
	  * \brief Gets the real vertex from a periodic vertex.
	  * \return the real vertex, in 0..nb_vertices_non_periodic_-1
	  */
	 signed_index_t periodic_vertex_real(signed_index_t pv) const {
	     geo_debug_assert(pv < signed_index_t(nb_vertices_non_periodic_ * 27));
	     geo_debug_assert(pv != -1);
	     return pv % signed_index_t(nb_vertices_non_periodic_);	     
	 }
	 
	 /**
	  * \brief Makes a periodic vertex from a real vertex and instance.
	  * \param[in] real the real vertex, in 0..nb_vertices_non_periodic_-1
	  * \param[in] instance the instance, in 0..26
	  */
	 index_t make_periodic_vertex(index_t real, index_t instance) const {
	     geo_debug_assert(real < nb_vertices_non_periodic_);
	     geo_debug_assert(instance < 27);
	     return real + nb_vertices_non_periodic_*instance;
	 }

	 /**
	  * \brief Gets the instance from a translation.
	  * \param[in] Tx , Ty , Tz the translation coordinates, in {-1, 0, 1}
	  * \return the instance, in 0..26
	  */
	 static index_t T_to_instance(int Tx, int Ty, int Tz) {
	     geo_debug_assert(Tx >= -1 && Tx <= 1);
	     geo_debug_assert(Ty >= -1 && Ty <= 1);
	     geo_debug_assert(Tz >= -1 && Tz <= 1);	     
	     int i = (Tz+1) + 3*(Ty+1) + 9*(Tx+1);
	     geo_debug_assert(i >= 0 && i < 27);
	     return index_t(reorder_instances[i]);
	 }

	 /**
	  * \brief Gets the translation from a periodic vertex.
	  * \param[in] pv the periodic vertex
	  * \param[out] Tx , Ty , Tz the translation coordinates, in {-1, 0, 1}
	  */
	 void periodic_vertex_get_T(index_t pv, int& Tx, int& Ty, int& Tz) const {
	     geo_debug_assert(pv < nb_vertices_non_periodic_ * 27);	     	     
	     index_t instance = periodic_vertex_instance(pv);
	     Tx = translation[instance][0];
	     Ty = translation[instance][1];
	     Tz = translation[instance][2];
	 }

	 /**
	  * \brief Sets the translation in a periodic vertex.
	  * \param[in,out] pv the periodic vertex
	  * \param[in] Tx , Ty , Tz the translation coordinates, in {-1, 0, 1}
	  */
	 void periodic_vertex_set_T(index_t& pv, int Tx, int Ty, int Tz) const {
	     geo_debug_assert(pv < nb_vertices_non_periodic_ * 27);
	     geo_debug_assert(Tx >= -1 && Tx <= 1);
	     geo_debug_assert(Ty >= -1 && Ty <= 1);
	     geo_debug_assert(Tz >= -1 && Tz <= 1);	     
	     pv = make_periodic_vertex(
		 periodic_vertex_real(pv), T_to_instance(Tx, Ty, Tz)
	     );
	 }

	 std::string periodic_vertex_to_string(index_t v) const {
	     return
		 String::to_string(periodic_vertex_real(v)) + ":" +
		 String::to_string(periodic_vertex_instance(v)) ;
	 }

	 std::string binary_to_string(index_t m) const {
	     std::string s(32,' ');
	     for(index_t i=0; i<32; ++i) {
		 s[i] = ((m & (1u << (31u-i))) != 0) ? '1' : '0'; 
	     }
	     return s;
	 }
	 
	 /**
	  * \brief Gives for each instance the integer translation coordinates
	  *   in {-1,0,1}.
	  * \details The zero translation is the first one (instance 0).
	  */
	 static int translation[27][3];

	 /**
	  * \brief Used to back-map an integer translation to an instance.
	  * \details This maps (Tx+1) + 3*(Ty+1) + 9*(Tz+1) to the associated
	  *  instance id. This indirection is required because we wanted 
	  *  instance 0 to correspond to the 0 translation 
	  *  (rather than (-1,-1,-1) that would require no indirection).
	  */
	 static int reorder_instances[27];

	 /**
	  * \brief Tests whether all the coordinates of the translation vector 
	  *  associated with an instance are 0 or 1.
	  */
	 static bool instance_is_positive[27];

	 /**
	  * \brief Number of real vertices.
	  */
	 index_t nb_vertices_non_periodic_;
     };
    
}

#endif

