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

#include <exploragram/hexdom/preprocessing.h>
#include <geogram/mesh/mesh_tetrahedralize.h>

#include <exploragram/hexdom/quadmesher.h> // for debug output

namespace GEO {

    static vec3 triangle_normal(const Mesh& M, index_t f){
        vec3 pt[3];
        for (index_t v = 0; v < 3; v++) pt[v] = M.vertices.point(M.facets.vertex(f,v));
        return cross(normalize(pt[1] - pt[0]), normalize(pt[2] - pt[0]));
    }

    static bool get_a_triangle_patch(
            const Mesh& M, 
            index_t facet, 
            std::vector<bool>& tri_patch_flag,
            index_t& patch_size,
            double cos_angle,
            index_t nb_tri_min_in_patch) {

        /* Add facet in the patch */
        tri_patch_flag[facet] = true;
        patch_size += 1;

        /* Check stop condition */
        if (patch_size >= nb_tri_min_in_patch) {
            return true;
        }

        /* Recursive call */
        vec3 n = normalize(triangle_normal(M, facet));
        for (index_t le = 0; le < 3; ++le) {
            index_t a = M.facets.adjacent(facet, le);
            if (a == GEO::NO_FACET) continue;
            vec3 n_a = normalize(triangle_normal(M, a));
            if (dot(n, n_a) > cos_angle) {
                if (!tri_patch_flag[a]) 
                    get_a_triangle_patch(M, a, tri_patch_flag, patch_size, cos_angle, nb_tri_min_in_patch);
            }
        }
        return patch_size >= nb_tri_min_in_patch;
    }

    static void generate_facet_is_in_patch_attribute(const Mesh& M) {
        geo_assert(M.facets.nb() > 0);

        /* The input constrained are only computed from "valid" triangles
         * - triangles are considered valid if they are in a smooth patch of at
         *    least nb_tri_min_in_patch triangles
         * - two adjacent triangles are in the same patch if the dot product of their
         *   normals is superior to cos_angle */

        /* Parameters */
        index_t nb_tri_min_in_patch = 8;
        const double cos_angle = 0.975;

        if (M.facets.nb() < 100) nb_tri_min_in_patch = 4; /* for small models */

        Attribute<int> is_in_patch(M.facets.attributes(), "is_valid");
        is_in_patch.fill(0);

        std::vector<bool> is_flagged(M.facets.nb());
        for(index_t f = 0; f < M.facets.nb(); ++f) {
            if (is_in_patch[f]) continue;
            index_t patch_size = 0;
            std::fill(is_flagged.begin(), is_flagged.end(), false); /* reinitialize to zero */
            bool ok = get_a_triangle_patch(M, f, is_flagged, patch_size, cos_angle, nb_tri_min_in_patch);
            if (ok) { /* A patch starting at f has been found */
                for (index_t i = 0; i < is_flagged.size(); ++i) {
                    /* triangles in the patch are keep for imposing constraints */
                    if (is_flagged[i]) is_in_patch[i] = 1;
                }
            }
        }
    }

    static void compute_input_constraints(Mesh* m, bool relaxed = false) {

        Attribute<mat3> B(m->vertices.attributes(), "B");
        Attribute<vec3> lockB(m->vertices.attributes(), "lockB");// how many vectors are locked
        Attribute<vec3> U(m->vertices.attributes(), "U");
        Attribute<vec3> lockU(m->vertices.attributes(), "lockU");// how many dimensions are locked

        // init all normal for each vertex
        vector<vector<vec3> > normals(m->vertices.nb());
        vector<vector<double> > weight(m->vertices.nb());

        /* Compute the facet normals and store them at vertices */
        if (!relaxed) {
            FOR(c, m->cells.nb()) FOR(cf, 4) {
                if ((m->cells.adjacent(c, cf) != NO_CELL)) continue;
                vec3 n = tet_facet_cross(m, c, cf);
                if (n.length2() > 1e-10) {
                    FOR(cfv, 3) {
                        normals[m->cells.facet_vertex(c, cf, cfv)].push_back(normalize(n));
                        weight[m->cells.facet_vertex(c, cf, cfv)].push_back(n.length());
                    }
                }
            }
        } else {
            /* The relaxation of constraints is achieved by storing only the normals of "valid" 
             * triangles.
             * They are flagged via the facet attribute is_valid
             * One possibility for flag them is to use generate_facet_is_in_patch_attribute()
             * See the method for tweaking the parameters. */
            m->cells.compute_borders();
            generate_facet_is_in_patch_attribute(*m);
            Attribute<int> is_valid(m->facets.attributes(), "is_valid");

            for(index_t f = 0; f < m->facets.nb(); ++f) {
                if (!is_valid[f]) continue;
                vec3 n = facet_normal(m, f);
                if (n.length2() > 1e-10) {
                    FOR(lv, 3) {
                        normals[m->facets.vertex(f, lv)].push_back(normalize(n));
                        weight[m->facets.vertex(f, lv)].push_back(n.length());
                    }
                }
            }
        }
        /* Build the constraints */
        FOR(v, m->vertices.nb()) {
            vector<vec3>& n = normals[v];
            vector<double>& w = weight[v];

            B[v].load_identity();
            lockB[v] = vec3(0, 0, 0);
            lockU[v] = vec3(0, 0, 0);
            if (n.size() > 0) {
                B[v] = Frame::representative_frame(n, w);// rot_to_B(representative_frame(n, w));
                AxisPermutation ap;
                ap.make_col2_equal_to_z(B[v], n[0]);
                B[v] = Frame(B[v]).apply_permutation(ap);
                FOR(i, n.size()) FOR(a, 3) 
                    if (std::abs(dot(col(B[v], a), n[i])) > .7) {
                        lockU[v][a] = 1;
                        lockB[v][a] = 1;
                    }
				if (lockB[v].length2() == 1) lockB[v] = vec3(0, 0, 1); // it may not always be true (happened once)
				if (lockB[v].length2() > 1) lockB[v] = vec3(1, 1, 1);
				U[v] = vec3(0, 0, 0);
            }
			
			//FOR(i,3) FOR(j,3) B[v](i,j) *= 5.;
        }

        if (relaxed) {
            // m->facets.clear(false); // Keep the surface part for debugging
        }
    }

    static void reorder_vertices_according_to_constraints(Mesh* m,bool hibert_sort) {
        Attribute<vec3> lockB(m->vertices.attributes(), "lockB");// how many vectors are locked

        GEO::vector<index_t > ind_map(m->vertices.nb());
        index_t n = 0;
        index_t num_l_v = 0;
        index_t num_ln_v = 0;
		FOR(v, m->vertices.nb()) if (lockB[v][0] == 1) { geo_assert(lockB[v][1] == 1); geo_assert(lockB[v][2] == 1); ind_map[n] = v; n++; }
        num_l_v = n;		
		FOR(v, m->vertices.nb()) if (lockB[v][0] == 0 && lockB[v][2] == 1) { geo_assert(lockB[v][1] == 0);  ind_map[n] = v; n++; }
        num_ln_v = n;
		FOR(v, m->vertices.nb()) if (lockB[v][2] == 0) { ind_map[n] = v; n++; }

        for (index_t i = 0; i < num_l_v; i++)                   geo_assert(lockB[ind_map[i]][0] == 1);
		for (index_t i = num_l_v; i < num_ln_v; i++)            geo_assert(lockB[ind_map[i]][2] == 1);
		for (index_t i = num_ln_v; i < m->vertices.nb(); i++)   geo_assert(lockB[ind_map[i]][2] == 0);

        geo_assert(num_l_v <= num_ln_v && num_ln_v <= m->vertices.nb());

		plop(hibert_sort);
		if (hibert_sort) {
			compute_Hilbert_order(m->vertices.nb(), m->vertices.point_ptr(0), ind_map, 0, num_l_v,3);
			compute_Hilbert_order(m->vertices.nb(), m->vertices.point_ptr(0), ind_map, num_l_v, num_ln_v, 3);
			compute_Hilbert_order(m->vertices.nb(), m->vertices.point_ptr(0), ind_map, num_ln_v, m->vertices.nb(), 3);
		}
        m->vertices.permute_elements(ind_map);          // note: it also updates the cell_corners.vertex... and invert ind_map :(
    }


    void produce_hexdom_input(Mesh* m,std::string& error_msg,bool hilbert_sort, bool relaxed) {
		
		m->edges.clear();
		m->vertices.remove_isolated();

		if (m->cells.nb() == 0) {
			if (m->facets.nb() == 0) throw ("mesh have no cells and no facets");
			mesh_tetrahedralize(*m, true, true, .8);
		}

        if (have_negative_tet_volume(m)) {
            throw ("contains tets with negative volume");
        }

		if (!m->cells.are_simplices()) {
            throw ("cells contains non tet elements");
        }
		
		if (!volume_boundary_is_manifold(m, error_msg)) {
            throw (error_msg.c_str());
        }
		
		if (!volume_is_tetgenifiable(m)) {
            throw (" tetgen is not able to remesh the volume from its boundary");
        }



		// add some attributes
		compute_input_constraints(m, relaxed);
		

		// compute scale
        double wanted_edge_length = get_cell_average_edge_size(m);
		
		Attribute<mat3> B(m->vertices.attributes(), "B");
        FOR(v, m->vertices.nb()) FOR(ij, 9) B[v].data()[ij] *= wanted_edge_length;
        reorder_vertices_according_to_constraints(m,hilbert_sort );

	}


}

