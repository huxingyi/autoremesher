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

#include <exploragram/hexdom/extra_connectivity.h>
#include <exploragram/hexdom/mesh_utils.h>

namespace GEO {

        FacetsExtraConnectivity::FacetsExtraConnectivity(Mesh * p_m){
                m = p_m;
                reset();
        }

        void FacetsExtraConnectivity::reset(){
                index_t nbc = m->facet_corners.nb();
                c2f.resize(nbc);
                c2c.resize(nbc);
                v2c.resize(m->vertices.nb());
                FOR(f, m->facets.nb()) FOR(fc, m->facets.nb_corners(f)){
                        index_t c = m->facets.corner(f, fc);
                        c2f[c] = f;
                        c2c[c] = c;
                        v2c[m->facets.vertex(f, fc)] = c;
                }
                FOR(f, m->facets.nb()) FOR(fc, m->facets.nb_corners(f)){
                        index_t c = m->facets.corner(f, fc);
                        c2c[c] = v2c[m->facets.vertex(f, fc)];
                        v2c[m->facets.vertex(f, fc)] = c;
                }
        }

        index_t FacetsExtraConnectivity::org(index_t corner_id){ return m->facet_corners.vertex(corner_id); }
        index_t FacetsExtraConnectivity::dest(index_t corner_id){ return m->facet_corners.vertex(next(corner_id)); }

        index_t FacetsExtraConnectivity::opposite(index_t corner_id){
                index_t cir = corner_id;
                index_t result = NOT_AN_ID; // not found
                do {
                        index_t candidate = prev(cir);
                        if ((org(candidate) == dest(corner_id)) && (dest(candidate) == org(corner_id))){
                                if (result == NOT_AN_ID) result = candidate;
                                else return NOT_AN_ID; // found more than one
                        }
                        if (cir != corner_id && dest(corner_id) == dest(cir))
                                return NOT_AN_ID; // the edge is non manifold
                        cir = c2c[cir];
                } while (cir != corner_id);
                return result;
        }
        index_t FacetsExtraConnectivity::next_around_vertex(index_t  cir) { return opposite(prev(cir)); }

        index_t FacetsExtraConnectivity::facet(index_t corner_id)       { return c2f[corner_id]; }
        index_t FacetsExtraConnectivity::local_id(index_t corner_id)    { return corner_id - m->facets.corners_begin(c2f[corner_id]); }

        index_t FacetsExtraConnectivity::next(index_t corner_id)                {
                index_t fc = local_id(corner_id);
                index_t offset = corner_id - fc;
                return offset + next_mod(fc, m->facets.nb_corners(c2f[corner_id]));
        }
        index_t FacetsExtraConnectivity::prev(index_t corner_id)                {
                index_t fc = local_id(corner_id);
                index_t offset = corner_id - fc;
                return offset + prev_mod(fc, m->facets.nb_corners(c2f[corner_id]));
        }

        vec3 FacetsExtraConnectivity::geom(index_t corner_id){
                return X(m)[dest(corner_id)] - X(m)[org(corner_id)];
        }









		FacetsExtraConnectivityWithInvalidFacets::FacetsExtraConnectivityWithInvalidFacets(Mesh * p_m) {
			m = p_m;
			facet_is_valid.bind(m->facets.attributes(), "is_valid");
			FOR(f, m->facets.nb()) facet_is_valid[f] = true;
			reset();
		}

		void FacetsExtraConnectivityWithInvalidFacets::reset() {
			//plop(m->facets.nb());
			index_t nbc = m->facet_corners.nb();
			c2f.resize(nbc,NOT_AN_ID);
			c2c.resize(nbc, NOT_AN_ID);
			v2c.resize(m->vertices.nb(), NOT_AN_ID);
			FOR(f, m->facets.nb()) FOR(fc, m->facets.nb_corners(f)) {
				index_t c = m->facets.corner(f, fc);
				if (facet_is_valid[f]) { // everything is NOT_AN_ID for invalid facet and associated corner 
					c2f[c] = f;
					c2c[c] = c;
					v2c[m->facets.vertex(f, fc)] = c;
				}
			}
			FOR(f, m->facets.nb()) {
				if (!facet_is_valid[f]) continue;
				FOR(fc, m->facets.nb_corners(f)) {
					index_t c = m->facets.corner(f, fc);
					c2c[c] = v2c[m->facets.vertex(f, fc)];
					v2c[m->facets.vertex(f, fc)] = c;
				}
			}
		}

		index_t FacetsExtraConnectivityWithInvalidFacets::org(index_t corner_id) { return m->facet_corners.vertex(corner_id); }
		index_t FacetsExtraConnectivityWithInvalidFacets::dest(index_t corner_id) { return m->facet_corners.vertex(next(corner_id)); }

		index_t FacetsExtraConnectivityWithInvalidFacets::opposite(index_t corner_id) {
			index_t cir = corner_id;
			index_t result = NOT_AN_ID; // not found
			do {
				index_t candidate = prev(cir);
				if ((org(candidate) == dest(corner_id)) && (dest(candidate) == org(corner_id))) {
					if (result == NOT_AN_ID) result = candidate;
					else return NOT_AN_ID; // found more than one
				}
				if (cir != corner_id && dest(corner_id) == dest(cir))
					return NOT_AN_ID; // the edge is non manifold
				cir = c2c[cir];
			} while (cir != corner_id);
			return result;
		}
		//index_t FacetsExtraConnectivityWithInvalidFacets::next_around_vertex(index_t  cir) { return opposite(prev(cir)); }

		index_t FacetsExtraConnectivityWithInvalidFacets::facet(index_t corner_id) { return c2f[corner_id]; }
		index_t FacetsExtraConnectivityWithInvalidFacets::local_id(index_t corner_id) { return corner_id - m->facets.corners_begin(c2f[corner_id]); }

		index_t FacetsExtraConnectivityWithInvalidFacets::next(index_t corner_id) {
			index_t fc = local_id(corner_id);
			index_t offset = corner_id - fc;
			return offset + next_mod(fc, m->facets.nb_corners(c2f[corner_id]));
		}
		index_t FacetsExtraConnectivityWithInvalidFacets::prev(index_t corner_id) {
			index_t fc = local_id(corner_id);
			index_t offset = corner_id - fc;
			return offset + prev_mod(fc, m->facets.nb_corners(c2f[corner_id]));
		}

		vec3 FacetsExtraConnectivityWithInvalidFacets::geom(index_t corner_id) {
			return X(m)[dest(corner_id)] - X(m)[org(corner_id)];
		}










        void halfedge_manip_example(Mesh* m){
                FacetsExtraConnectivity fec(m);
                FOR(c, m->facet_corners.nb()){// we can directly loop over each halfedges
                        // turning around a facet
                        index_t cir = c;
                        do {
                                cir = fec.next(cir);
                        } while (cir != c);

                        //iterate on vertex incident
                        cir = c;
                        do {
                                cir = fec.c2c[cir];
                        } while (cir != c);

                        //turning around a manifold vertex
                        cir = c;
                        do {
                                cir = fec.next_around_vertex(cir);
                        } while (cir != c && cir != NOT_AN_ID);
                }
        }




        void create_facet_adjacence(Mesh* m, bool has_border){
                FacetsExtraConnectivity qfec(m);
                FOR(h, m->facet_corners.nb()){
                        index_t opp = qfec.opposite(h);
                        if (!has_border && opp == NOT_AN_ID) GEO::Logger::out("HexDom")  << "PANIC MODE, INPUT IS NON MANIFOLD !!!  --- check if the surface has border" <<  std::endl;
                        else {
                                m->facets.set_adjacent(qfec.facet(h), qfec.local_id(h), qfec.facet(opp));
                                m->facets.set_adjacent(qfec.facet(opp), qfec.local_id(opp), qfec.facet(h));
                        }
                }
        }





        void cell_edges_in_RCS(Mesh* m, vector<index_t>& offset_from_org, vector<index_t>& dest){
                geo_assert(m->cells.are_simplices());
                dest.clear();
                offset_from_org.clear();
                offset_from_org.reserve(m->vertices.nb());
                vector<index_t> v2cc(m->vertices.nb(), NOT_AN_ID);  // v2cc maps vertices to cell corner
                vector<index_t> next(4 * m->cells.nb(), NOT_AN_ID);     // next chains cell corners

                FOR(c, m->cells.nb()) FOR(cv, 4){
                        index_t v = m->cells.vertex(c, cv);
                        next[4 * c + cv] = v2cc[v];
                        v2cc[v] = 4 * c + cv;
                }
                FOR(v, m->vertices.nb()){
                        offset_from_org.push_back(dest.size());
                        for (index_t i = v2cc[v]; i != NOT_AN_ID; i = next[i])
                                FOR(lc, 4) if (i % 4 != lc)
                                dest.push_back(m->cells.vertex(i / 4, lc));
                        std::sort(dest.begin() + int(offset_from_org.back()), dest.end());
                        vector<index_t>::iterator last = std::unique(dest.begin() + int(offset_from_org.back()), dest.end());
                        dest.resize(size_t(last - dest.begin()));
                }
                offset_from_org.push_back(dest.size());
        }




        void compute_tet_edge_graph(Mesh* m,vector<index_t> & v2e,bool store_both_directions){
                geo_assert(m->cells.are_simplices());
                m->edges.clear();
                v2e.resize(m->vertices.nb());

                //Attribute<index_t> v2e(m->vertices.attributes(), "v2e");

                vector<index_t> adj_;
                vector<index_t> adj_off_;
                adj_off_.reserve(m->vertices.nb() + 1);

                vector<index_t> v2c(m->vertices.nb(), NOT_AN_ID);
                vector<index_t> next(4 * m->cells.nb(), NOT_AN_ID); 

                FOR(c, m->cells.nb()) FOR(cv, 4){
                        index_t v = m->cells.vertex(c, cv);
                        next[4 * c + cv] = v2c[v];
                        v2c[v] = 4 * c + cv;
                }
                FOR(v, m->vertices.nb()){
                        adj_off_.push_back(adj_.size());
                        for (index_t i = v2c[v]; i != NOT_AN_ID; i = next[i]) FOR(lc, 4) if (i % 4 != lc){
                                index_t nv = m->cells.vertex(i / 4, lc);
                                if (nv>v || store_both_directions) adj_.push_back(nv);
                        }
                        std::sort(adj_.begin() + int(adj_off_.back()), adj_.end());
                        vector<index_t>::iterator last = std::unique(adj_.begin() + int(adj_off_.back()), adj_.end());
                        adj_.resize(size_t(last - adj_.begin()));
                }
                adj_off_.push_back(adj_.size());

                index_t offe = m->edges.create_edges(adj_.size());
                FOR(v, m->vertices.nb()){
                        v2e[v] = adj_off_[v];
                        for (index_t e = adj_off_[v]; e < adj_off_[v + 1]; e++){
                                m->edges.set_vertex(offe + e, 0, v);
                                m->edges.set_vertex(offe + e, 1, adj_[e]);
                        }
                }
        }

        void restore_v2e(Mesh* m, vector<index_t>& v2e) {
            v2e.resize(m->vertices.nb());
            FOR(v, m->vertices.nb())v2e[v] = NOT_AN_ID;
            index_t lastv = 0;
            FOR(e, m->edges.nb()) {
                index_t org = m->edges.vertex(e, 0);
                while (lastv <= org) { v2e[lastv] = e; lastv++; }
            }
            while (lastv < m->vertices.nb()) { v2e[lastv] = m->edges.nb(); lastv++; }
        }




        /**
        * check that v2f[v] gives all facets that
        *               are adjacent to v
        *               are not to be killed
        */

        bool v2f_is_valid(Mesh* m, vector<vector<index_t> >& v2f, vector<index_t> &to_kill) {
            vector<vector<bool> > is_ref(m->vertices.nb());
            FOR(v, m->vertices.nb()) is_ref[v].resize(v2f[v].size(), false);
            // check that all facets are referenced
            FOR(f, m->facets.nb()) if (!to_kill[f])
                FOR(lc, m->facets.nb_corners(f)) {
                index_t v = m->facets.vertex(f, lc);
                bool f_is_ref = false;
                FOR(i, v2f[v].size()) if (v2f[v][i] == f) {
                    is_ref[v][i] = true;
                    f_is_ref = true;
                }
                if (!f_is_ref) {
                    GEO::Logger::out("HexDom")  << "facet " << f << " is not referenced in vertex " << v << " !!!" <<  std::endl;
                    return false;
                }
            }
            // check that no extra facet is referenced
            FOR(v, m->vertices.nb()) FOR(i, v2f[v].size()) if (!is_ref[v][i]) {
                GEO::Logger::out("HexDom")  << "one facet of v2f does not exist!!!" <<  std::endl;
                return false;
            }
            return true;
        }
}
