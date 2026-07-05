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

#ifndef H_HEXDOM_ALGO_FF_H
#define H_HEXDOM_ALGO_FF_H

#include <exploragram/basic/common.h>
#include <geogram/mesh/mesh.h>

namespace GEO {

    class FFopt {

    public:
        // input object must have B, lockB, U and lockU defined                                     (mandatory)
        // constructor computes the bidirectional edge graph 
        // and init limits of variables pack (num_l_v and num_ln_v)
        FFopt(Mesh* p_m);
        ~FFopt();

        /**
	 * methods in the order yuo have to call them
	 */


        // FF_init() is use to initialize the FF with  http://arxiv.org/abs/1507.03351              (mandatory)
        void FF_init(bool generate_sh=false);

        // FF_smooth is to further optimize the FF with http://dl.acm.org/citation.cfm?id=2366196   (optional)
        void FF_smooth();

        // size of cols(B,d)
	void compute_Bid_norm();

        // brush FF (before solving PGP) and U (after solving PGP)                                  (optional)
        void brush_frame();                         

        // access to the graph to be optimized (either edge or dual edge graph)
        index_t nb_neigs(index_t s) {
            index_t start = v2e[s];
            index_t end = m->edges.nb();
            if (s + 1 < m->vertices.nb()) end = v2e[s + 1];
            return   end - start;
        }
        index_t neig(index_t s, index_t ls) {
            return   m->edges.vertex(ls + v2e[s], 1);
        }

        // in/out members
        Mesh* m;
        vector<index_t> v2e;                         // for each vertex, gives the first edge starting on it

        // vertex ordering
        index_t num_l_v;                // v < num_l_v                          => the frame of vertex v is locked to rot[v]
        index_t num_ln_v;               // num_l_v < v < num_ln_v       => the Z axis rotated by rot[v] should not change
    };
}

#endif
