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


#ifndef H_HEXDOM_ALGO_PGP_OPT_H
#define H_HEXDOM_ALGO_PGP_OPT_H

#include <exploragram/basic/common.h>
#include <exploragram/hexdom/basic.h> 
#include <geogram/mesh/mesh.h>


namespace GEO {
    
    class EXPLORAGRAM_API PGPopt {
    public:

        // what we need to create the parametric space U
        PGPopt(Mesh* p_m);

        void optimize_corr(double max_corr_prop = .35);
        void optimize_PGP();


        vec3 wish_angle_edge_geom(index_t e, bool inv);
        vec3 wish_angle_corr(index_t e, bool inv);
        vec3 wish_angle(index_t e, bool inv);
        bool is_PGP_singular(index_t c, index_t lf);

        bool face_is_resp(index_t c, index_t lf);


        bool tet_is_PGP_singular_fct(index_t t);


        //// cubecover
		//index_t get_the_only_non_zero_lc(index_t c, index_t cf, Attribute<index_t>& CCedgeid);
		//index_t number_edges_on_cut_graph(Attribute<index_t>& CCedgeid);
		index_t get_non_nulledge(index_t c, index_t cf, Attribute<bool>& nulledge);
		void mark_null_edges(Attribute<bool>& nulledge);

        //bool constraint_boundary is set to false only to compute the correction one form
        void cubcover(bool compute_only_corr =false);
		void grow_ball(Attribute<bool>& tet_in_ball);
		void grow_triangle_ball(vector<bool>& tetface_in_ball);



        index_t edge_from_vertices(index_t v0, index_t v1, bool &inv) {
            inv = v0 > v1;
            if (inv) std::swap(v0, v1);

            index_t start = v2e[v0];
            index_t end = v0 < m->vertices.nb() - 1 ? v2e[v0 + 1] : m->edges.nb();
            for (index_t e = start; e < end; e++) {
                if (m->edges.vertex(e, 1) == v1) return e;
            }
            geo_assert_not_reached;
         
        }
			
		void move_U_to_corner();

/*
        void snap_U_to_round(double eps = 0.05) {
			return;
            FOR(v, m->vertices.nb()) FOR(d, 3) {
                if (std::abs(U[v][d] - round(U[v][d])) < eps) {
                    U[v][d] = round(U[v][d]);
                }
            }
        }
*/



        Mesh* m;
        Attribute<vec3> U;
        Attribute<mat3> B;

        // A PGP solution is not only a mesh + attrib: it also requires these datas
       
        Attribute<vec3> corr;
		//Attribute<vec3i> tij;
		Attribute<vec3> tij;

        vector<index_t> v2e;
        vector<vector<index_t> > v2eopp;
    };

}
#endif
