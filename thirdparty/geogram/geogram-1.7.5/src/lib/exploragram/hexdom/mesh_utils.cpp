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

#include <exploragram/hexdom/mesh_utils.h>
#include <exploragram/hexdom/geometry.h>
#include <exploragram/hexdom/mesh_inspector.h>
#include <exploragram/hexdom/extra_connectivity.h>
#include <geogram/points/colocate.h>
#include <geogram/mesh/mesh_tetrahedralize.h>
#include <geogram/delaunay/delaunay.h>

namespace GEO {

    void compute_3D_edge_cot_w(Mesh* m, Attribute<index_t>& v2e, double anisoZ_cotW){

	Attribute<double> cot_w(m->edges.attributes(), "cot_w");
	FOR(e, m->edges.nb()) cot_w[e] = 0;
	FOR(c, m->cells.nb()){
	    vec3 pts[4];
	    FOR(cv, 4)pts[cv] = X(m)[m->cells.vertex(c, cv)];
	    double aniso[6] = { 1, 1, anisoZ_cotW, 0, 0, 0 };
	    CoTan3D cot(pts, aniso);
	    FOR(cv, 4){
		index_t v = m->cells.vertex(c, cv);
		index_t start = v2e[v];
		index_t end = m->edges.nb();
		if (v + 1 < m->vertices.nb()) end = v2e[v + 1];
		for (index_t e = start; e < end; e++){
		    geo_assert(v == m->edges.vertex(e, 0));
		    FOR(cv2, 4){
			if (cv == cv2) continue;
			index_t v2 = m->cells.vertex(c, cv2);
			if (v2 == m->edges.vertex(e, 1)){
			    FOR(cot_e, 6){
				if ((cot.org(cot_e) != cv || cot.dest(cot_e) != cv2)
				    && (cot.dest(cot_e) != cv || cot.org(cot_e) != cv2))
				    continue;
				cot_w[e] += cot.w[cot_e];
			    }
			}
		    }
		}
	    }
	}
	// normalize a bit
	double sum = 0;
	FOR(e, m->edges.nb()) sum += cot_w[e];
	FOR(e, m->edges.nb()) cot_w[e] *= double(m->edges.nb()) / sum;
    }

    void kill_isolated_vertices(Mesh* m){
	vector<index_t> to_kill(m->vertices.nb(), NOT_AN_ID);
	FOR(e, m->edges.nb()) FOR(ev, 2) to_kill[m->edges.vertex(e, ev)] = 0;
	FOR(f, m->facets.nb()) FOR(fv, m->facets.nb_vertices(f)) to_kill[m->facets.vertex(f, fv)] = 0;
	FOR(c, m->cells.nb()) FOR(cv, m->cells.nb_vertices(c)) to_kill[m->cells.vertex(c, cv)] = 0;
	m->vertices.delete_elements(to_kill);
    }


    void merge_vertices(Mesh* m, double eps){
	vector<index_t> to_kill(m->vertices.nb(), 0);
	vector<index_t> old2new(m->vertices.nb());
	Geom::colocate(m->vertices.point_ptr(0), 3, m->vertices.nb(), old2new, eps);
	FOR(e, m->edges.nb()) FOR(ev, 2) m->edges.set_vertex(e, ev, old2new[m->edges.vertex(e, ev)]);
	FOR(f, m->facets.nb()) FOR(fv, m->facets.nb_vertices(f)) m->facets.set_vertex(f, fv, old2new[m->facets.vertex(f, fv)]);
	FOR(c, m->cells.nb()) FOR(cv, m->cells.nb_vertices(c)) m->cells.set_vertex(c, cv, old2new[m->cells.vertex(c, cv)]);
	FOR(v, m->vertices.nb()) if (old2new[v] != v) to_kill[v] = NOT_AN_ID;
	m->vertices.delete_elements(to_kill);
    }

    void facets_smooth_geom(Mesh* m, std::vector<bool>& lock_v, double fit_coeff) {
	vector<vec3> P(m->vertices.nb(), vec3(0, 0, 0));
	vector<index_t> val(m->vertices.nb(), 0);
	FOR(f, m->facets.nb()) {
	    vec3 bary = facet_bary(m, f);
	    FOR(fv, m->facets.nb_vertices(f)) {
		index_t v = m->facets.vertex(f, fv);
		P[v] = P[v] + bary;
                    val[v]++;
	    }
	}
	FOR(v, m->vertices.nb()) if (!lock_v[v]) X(m)[v] = fit_coeff*X(m)[v] + (1. - fit_coeff) * (1. / double(val[v]))*P[v];
    }

    void cells_smooth_geom(Mesh* m, std::vector<bool>& lock_v, double fit_coeff) {
	vector<vec3> P(m->vertices.nb(), vec3(0, 0, 0));
	vector<index_t> val(m->vertices.nb(), 0);
	FOR(c, m->cells.nb()) {
	    vec3 bary = cell_bary(m, c);
	    FOR(cv, m->cells.nb_vertices(c)) {
		index_t v = m->cells.vertex(c, cv);
		P[v] = P[v] + bary;
		val[v]++;
	    }
	}
	FOR(v, m->vertices.nb()) if (!lock_v[v]) X(m)[v] = fit_coeff*X(m)[v] + (1. - fit_coeff) * (1. / double(val[v]))*P[v];
    }

    void facets_smooth_geom(Mesh* m, double fit_coeff) {
	std::vector<bool> lock_v(m->vertices.nb(), false);
	facets_smooth_geom(m, lock_v, fit_coeff);
    }

    void cells_smooth_geom(Mesh* m, double fit_coeff) {
	std::vector<bool> lock_v(m->vertices.nb(), false);
	cells_smooth_geom(m, lock_v, fit_coeff);
    }


    
    void create_non_manifold_facet_adjacence(Mesh* m) {
	FacetsExtraConnectivity fec(m);
	FOR(f, m->facets.nb())FOR(lh, m->facets.nb_vertices(f)) m->facets.set_adjacent(f, lh, NOT_AN_ID);
	FOR(h, m->facet_corners.nb()) {
	    if (m->facets.adjacent(fec.facet(h), fec.local_id(h)) != NOT_AN_ID) continue;
	    index_t cir = h;
	    index_t best_candidate = NOT_AN_ID;
	    double bestdot = 2;
	    do {
		index_t candidate = fec.prev(cir);
		if (candidate != NOT_AN_ID
		    && m->facets.adjacent(fec.facet(candidate), fec.local_id(candidate)) == NOT_AN_ID)
		    if ((fec.org(candidate) == fec.dest(h)) && (fec.dest(candidate) == fec.org(h))) {
			double curdot = dot(facet_normal(m, fec.facet(h)), -facet_normal(m, fec.facet(candidate)));
			if (curdot < bestdot) {
			    best_candidate = candidate;
			    bestdot = curdot;
			}
		    }
		cir = fec.c2c[cir];
	    } while (cir != h);
	    if (best_candidate != NOT_AN_ID) {
		m->facets.set_adjacent(fec.facet(h), fec.local_id(h), fec.facet(best_candidate));
		m->facets.set_adjacent(fec.facet(best_candidate), fec.local_id(best_candidate), fec.facet(h));

		geo_assert(fec.org(h) == fec.dest(best_candidate));
		geo_assert(fec.dest(h) == fec.org(best_candidate));
	    }
	}
    }

	double get_cell_average_edge_size( Mesh* mesh) {
		double sum = 0;
		int nb = 0;
		FOR(c, mesh->cells.nb()) FOR(lf, mesh->cells.nb_facets(c)) FOR(lv, mesh->cells.facet_nb_vertices(c, lf))
		{
			index_t v0 = mesh->cells.facet_vertex(c, lf, lv);
			index_t v1 = mesh->cells.facet_vertex(c, lf, (lv + 1) % mesh->cells.facet_nb_vertices(c, lf));
			sum += (mesh->vertices.point(v0) - mesh->vertices.point(v1)).length();
			nb++;
		}
		geo_assert(nb > 0);
		return sum / double(nb);
	}
	double get_facet_average_edge_size( Mesh* m) {
		geo_assert(m->facet_corners.nb() > 0);
		double ave_edge_length = 0;
		FOR(f, m->facets.nb()) FOR(v, m->facets.nb_vertices(f)) ave_edge_length += (X(m)[m->facets.vertex(f, v)] - X(m)[m->facets.vertex(f, (v + 1) % m->facets.nb_vertices(f))]).length();
		return ave_edge_length / double(m->facet_corners.nb());
	}

    vec3 tet_facet_cross(Mesh* m, index_t c, index_t lf){
	vec3 pt[3];
	for (index_t v = 0; v < 3; v++) pt[v] = m->vertices.point(m->cells.facet_vertex(c, lf, v));
	return cross(normalize(pt[1] - pt[0]), normalize(pt[2] - pt[0]));
    }

    /**
     * HalfedgeToTriangleInTet[cv1][cv2] is the local facet (cf) associated to the halfedge going from cv1 to cv2
     */
    static index_t HalfedgeToTriangleInTet[4][4] = {
	{NOT_AN_ID, 2, 3, 1},
	{ 3, NOT_AN_ID, 0, 2 },
	{ 1, 3, NOT_AN_ID, 0 },
	{ 2, 0, 1, NOT_AN_ID }
    };

    index_t next_cell_around_oriented_edge(Mesh* m, index_t cell_id, index_t v_org, index_t v_dest){
	index_t cv_org = NOT_AN_ID;
	index_t cv_dest = NOT_AN_ID;
	FOR(lv, 4) {
	    index_t v = m->cells.vertex(cell_id, lv);
	    if (v == v_org) cv_org = lv;
	    if (v == v_dest) cv_dest = lv;
	}
	geo_assert(cv_org != NOT_AN_ID);
	geo_assert(cv_dest != NOT_AN_ID);
	geo_assert(cv_org != cv_dest);
	return m->cells.adjacent(cell_id, HalfedgeToTriangleInTet[cv_org][cv_dest]);
    }


    /*    __  __                _     _               _______   _       
     *   |  \/  |              | |   (_)             |__   __| | |      
     *   | \  / | __ _ _ __ ___| |__  _ _ __   __ _     | | ___| |_ ___ 
     *   | |\/| |/ _` | '__/ __| '_ \| | '_ \ / _` |    | |/ _ \ __/ __|
     *   | |  | | (_| | | | (__| | | | | | | | (_| |    | |  __/ |_\__	\
     *   |_|  |_|\__,_|_|  \___|_| |_|_|_| |_|\__, |    |_|\___|\__|___/
     *                                         __/ |                    
     *                                        |___/                    
     */
    
    const  index_t tet_edge_vertices[6][2] = { { 0, 1 }, { 0, 2 }, { 0, 3 }, { 1, 2 }, { 1, 3 }, { 2, 3 } };
    
    const index_t MTN = index_t(-1);
    const index_t MT[16][4] = {
	{MTN, MTN, MTN, MTN},               //0 0 0 0
	{ 0, 2, 1, MTN },
	{ 0, 3, 4, MTN },
	{ 1, 3, 4, 2 },
	{ 1, 5, 3, MTN },        //0 1 0 0
	{ 0, 2, 5, 3 },
	{ 1, 0, 4, 5 },
	{ 2, 5, 4, MTN },
	{ 2, 4, 5, MTN },                //1 0 0 0
	{ 0, 4, 5, 1 },
	{ 3, 5, 2, 0 },
	{ 3, 5, 1, MTN },
	{ 1, 2, 4, 3 },                 //1 1 0 0
	{ 0, 4, 3, MTN },
	{ 2, 0, 1, MTN },
	{ MTN,MTN,MTN,MTN }
    };

    
}

