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

#ifndef H_HEXDOM_ALGO_EXTRA_CONNECTIVITY_H
#define H_HEXDOM_ALGO_EXTRA_CONNECTIVITY_H

#include <exploragram/basic/common.h>
#include <geogram/mesh/mesh.h>
#include <cmath>

namespace GEO {

    /**
     * All connectivity informations of an halfedge data structure
     * WARNING: does not support mesh modification
     */
    struct FacetsExtraConnectivity{
	FacetsExtraConnectivity(Mesh * p_m);
	void reset();
	index_t org(index_t corner_id);
	index_t dest(index_t corner_id);
	index_t opposite(index_t corner_id);
	index_t next_around_vertex(index_t  cir);
	index_t facet(index_t corner_id);
	index_t local_id(index_t corner_id);

	index_t next(index_t corner_id);
	index_t prev(index_t corner_id);
	vec3 geom(index_t corner_id);

	Mesh* m;
	vector<index_t> v2c;	// vertex to corner
	vector<index_t> c2f;	// corner to facet
	vector<index_t> c2c;	// corner to next corner sharing the same vertex
    };


	/**
	* All connectivity informations of an halfedge data structure
	* WARNING: does not support mesh modification
	*/
	struct FacetsExtraConnectivityWithInvalidFacets {
		FacetsExtraConnectivityWithInvalidFacets(Mesh * p_m);
		void reset();
		index_t org(index_t corner_id);
		index_t dest(index_t corner_id);
		index_t opposite(index_t corner_id);
		index_t next_around_vertex(index_t  cir);
		index_t facet(index_t corner_id);
		index_t local_id(index_t corner_id);
		index_t next_CCW(index_t h) { return opposite(prev(h)); }
		index_t next_CW(index_t h) { return next(opposite(h)); }


		index_t next(index_t corner_id);
		index_t prev(index_t corner_id);
		vec3 geom(index_t corner_id);

		Mesh* m;
		Attribute<bool> facet_is_valid;
		vector<index_t> v2c;	// vertex to corner
		vector<index_t> c2f;	// corner to facet
		vector<index_t> c2c;	// corner to next corner sharing the same vertex
	};

    void halfedge_manip_example(Mesh* m);

    /**
     * export adjacency defined by shared vertices (for compatibility with other geomgram algo)
     */
    void create_facet_adjacence(Mesh* m, bool has_border = false);

    /**
     * for each edge of a tet mesh, generate an edge in m->edges
     * edges are sorted by id of their first vertex
     * and the first edge a vertex v is stored in Attribute<index_t>(m->vertices.attributes(), "v2e");
     */
    void compute_tet_edge_graph(Mesh* m,vector<index_t>& v2e, bool store_both_directions);
    void restore_v2e(Mesh* m, vector<index_t>& v2e);

    /**
     * store oriented edges in a Row Column Storage structure
     * neigborgs of vertex v are dest[i] with i in offset_from_org[v]...offset_from_org[v+1]
     */
    void cell_edges_in_RCS(Mesh* m, vector<index_t>& offset_from_org, vector<index_t>& dest);

    /**
     * check that v2f[v] gives all facets that
     *               are adjacent to v
     *               are not to be killed
     */
    bool v2f_is_valid(Mesh* m, vector<vector<index_t> >& v2f, vector<index_t> &to_kill);
}
#endif
