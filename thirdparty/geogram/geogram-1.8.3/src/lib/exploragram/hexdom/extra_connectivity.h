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
