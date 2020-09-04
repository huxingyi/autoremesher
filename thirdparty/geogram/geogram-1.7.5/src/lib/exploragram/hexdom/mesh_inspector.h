/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2015 INRIA - Project ALICE
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact for Graphite: Bruno Levy - Bruno.Levy@inria.fr
 *  Contact for this Plugin: Nicolas Ray - nicolas.ray@inria.fr
 *
 *     Project ALICE
 *     LORIA, INRIA Lorraine,
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs.
 *
 * As an exception to the GPL, Graphite can be linked with the following
 * (non-GPL) libraries:
 *     Qt, tetgen, SuperLU, WildMagic and CGAL
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
