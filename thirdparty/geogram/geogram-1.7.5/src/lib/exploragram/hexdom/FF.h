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
