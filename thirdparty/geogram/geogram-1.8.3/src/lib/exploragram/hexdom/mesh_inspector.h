/*
 *  Copyright (c) 2000-2022 Inria
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
 *  Contact: Bruno Levy
 *
 *     https://www.inria.fr/fr/bruno-levy
 *
 *     Inria,
 *     Domaine de Voluceau,
 *     78150 Le Chesnay - Rocquencourt
 *     FRANCE
 *
 */

#ifndef H_HEXDOM_ALGO_MESH_INSPECTOR_H
#define H_HEXDOM_ALGO_MESH_INSPECTOR_H

#include <exploragram/basic/common.h>
#include <exploragram/hexdom/mesh_utils.h>

namespace GEO {
    
    void EXPLORAGRAM_API get_facet_stats(Mesh* m, const char * msg = "get_facet_stats", bool export_attribs = false);
    
    bool EXPLORAGRAM_API surface_is_tetgenifiable(Mesh* m);
    
    bool EXPLORAGRAM_API volume_is_tetgenifiable(Mesh* m);
    
    bool EXPLORAGRAM_API surface_is_manifold(Mesh* m, std::string& msg);
    
    bool EXPLORAGRAM_API volume_boundary_is_manifold(Mesh* m, std::string& msg);
    
    double EXPLORAGRAM_API tet_vol(vec3 A, vec3 B, vec3 C, vec3 D);
    
    void EXPLORAGRAM_API get_hex_proportion(Mesh*m, double &nb_hex_prop, double &vol_hex_prop);

    bool EXPLORAGRAM_API have_negative_tet_volume(Mesh*m);
  
    /**
    * unit vector: weighted sum of normal of a triangle fan around the barycenter
    */
    inline vec3 facet_normal(Mesh* m, index_t f) {
        index_t nbv = m->facets.nb_vertices(f);
        vec3 bary = facet_bary(m, f);
        vec3 res(0, 0, 0);
        FOR(fv, nbv) res = res + cross(X(m)[m->facets.vertex(f, fv)] - bary, X(m)[m->facets.vertex(f, next_mod(fv, nbv))] - bary);
        return normalize(res);
    }
}
#endif
