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

#ifndef H_HEXDOM_ALGO_MESH_UTILS_H
#define H_HEXDOM_ALGO_MESH_UTILS_H

#include <exploragram/basic/common.h>
#include <exploragram/hexdom/basic.h>
#include <geogram/basic/attributes.h>
#include <geogram/mesh/mesh.h>

namespace GEO {

    inline index_t cell_facet_corner_id(Mesh* m, index_t c, index_t cf, index_t cfc) {
	return m->cells.corner(c, m->cells.descriptor(c).facet_vertex[cf][cfc]);
    }
    
    void EXPLORAGRAM_API compute_3D_edge_cot_w(Mesh* m, Attribute<index_t>& v2e, double anisoZ_cotW);
    
    /**
     * remove vertices that are not referenced in cells/facets/edges
     */
    void EXPLORAGRAM_API kill_isolated_vertices(Mesh* m);

    /**
     * merges vertices that are close (see GEO::colocate for details about distance guaranty)
     * update references to vertices in cells/facets/edges
     */
    void EXPLORAGRAM_API merge_vertices(Mesh* m, double eps);


    void EXPLORAGRAM_API facets_smooth_geom(Mesh* m, std::vector<bool>& lock_v, double fit_coeff = .95);

    void EXPLORAGRAM_API cells_smooth_geom(Mesh* m, std::vector<bool>& lock_v, double fit_coeff = .95);

    void EXPLORAGRAM_API facets_smooth_geom(Mesh* m, double fit_coeff = .95);
    
    void EXPLORAGRAM_API cells_smooth_geom(Mesh* m, double fit_coeff = .95);

    void EXPLORAGRAM_API create_non_manifold_facet_adjacence(Mesh* m);


    inline vec3* X(Mesh* m) { return (vec3*)m->vertices.point_ptr(0); }
    inline vec3 cell_bary(Mesh* m, index_t c){
	vec3 ave(0, 0, 0);
	FOR(lv, m->cells.nb_vertices(c)) ave += m->vertices.point(m->cells.vertex(c, lv));
	return ave / double(m->cells.nb_vertices(c));
    }

    inline vec3 cell_facet_bary(Mesh* m, index_t c, index_t lf){
	vec3 ave(0, 0, 0);
	FOR(lv, m->cells.facet_nb_vertices(c, lf))  ave += m->vertices.point(m->cells.facet_vertex(c, lf, lv));
	return ave / double(m->cells.facet_nb_vertices(c, lf));
    }

    inline vec3 facet_bary(Mesh* m, index_t f){
	vec3 ave(0, 0, 0);
	FOR(lv, m->facets.nb_vertices(f))  ave += m->vertices.point(m->facets.vertex(f, lv));
	return ave / double(m->facets.nb_vertices(f));
    }

	EXPLORAGRAM_API double get_cell_average_edge_size( Mesh* mesh);
	EXPLORAGRAM_API double get_facet_average_edge_size( Mesh* mesh);
	EXPLORAGRAM_API vec3 tet_facet_cross(Mesh* m, index_t c, index_t lf);
    EXPLORAGRAM_API index_t next_cell_around_oriented_edge(Mesh* m, index_t cell_id, index_t v_org, index_t v_dest);




    /*    __  __                _     _               _______   _       
     *   |  \/  |              | |   (_)             |__   __| | |      
     *   | \  / | __ _ _ __ ___| |__  _ _ __   __ _     | | ___| |_ ___ 
     *   | |\/| |/ _` | '__/ __| '_ \| | '_ \ / _` |    | |/ _ \ __/ __|
     *   | |  | | (_| | | | (__| | | | | | | | (_| |    | |  __/ |_\__	\
     *   |_|  |_|\__,_|_|  \___|_| |_|_|_| |_|\__, |    |_|\___|\__|___/
     *                                         __/ |                    
     *                                        |___/                    
     */
    
    extern EXPLORAGRAM_API const index_t tet_edge_vertices[6][2];
    extern EXPLORAGRAM_API const index_t MT[16][4];
    

    inline const index_t* MTcase(double iso, double v0, double v1, double v2, double v3){
	index_t triindex = 0;
	if (v0 < iso) triindex |= 1;
	if (v1 < iso) triindex |= 2;
	if (v2 < iso) triindex |= 4;
	if (v3 < iso) triindex |= 8;
	return &(MT[triindex][0]);
    }


    /**
     * uv is a pointer because it is an optional parameter (can be nullptr)
     */
    inline index_t add_facet_to_mesh(Mesh* m, vector<vec3>& pts, Attribute<vec2>* uv = nullptr, vector<vec2>* lU = nullptr){
	index_t off = m->vertices.create_vertices(pts.size());
	vector<index_t> nv(pts.size());
	FOR(lv ,pts.size()) {
	    nv[lv] = off + lv;
	    m->vertices.point(nv[lv]) = pts[lv];
	}
	index_t f = m->facets.create_polygon(nv);
	if (uv != nullptr) FOR(lc,m->facets.nb_corners(f))//FOR_EACH_HALFEDGE_OF_FACET(m,h,f)
			    (*uv)[m->facets.corner(f,lc)] = (*lU)[lc];
	return f;
    }

    template <class T> inline void get_range(Attribute<T> &attr, double& v_min, double &v_max) {
	v_min = 1e20;
	v_max = -1e20;
	FOR(i, attr.nb_elements()) {
	   v_min = std::min(v_min, attr[i]);
	   v_max = std::max(v_max, attr[i]);
	}
    }
    
}

#endif
