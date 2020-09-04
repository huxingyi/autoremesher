/*
 *  Copyright (c) 2012-2014, Bruno Levy
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
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     Bruno.Levy@inria.fr
 *     http://www.loria.fr/~levy
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 */

#include <geogram/mesh/mesh_smoothing.h>
#include <geogram/mesh/mesh.h>
#include <geogram/NL/nl.h>

namespace GEO {

    void GEOGRAM_API mesh_smooth(Mesh& M) {
	// Chain corners around vertices
	vector<index_t> v2c(M.vertices.nb(), index_t(-1));
	vector<index_t> next_c_around_v(M.facet_corners.nb(), index_t(-1));
	vector<index_t> c2f(M.facet_corners.nb(), index_t(-1));
	for(index_t f: M.facets) {
	    for(index_t c: M.facets.corners(f)) {
		index_t v = M.facet_corners.vertex(c);
		next_c_around_v[c] = v2c[v];
		v2c[v] = c;
		c2f[c] = f;
	    }
	}

	nlNewContext();

	nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);
	nlSolverParameteri(NL_NB_VARIABLES, NLint(M.vertices.nb()));
	nlSolverParameteri(NL_NB_SYSTEMS, NLint(M.vertices.dimension()));
	nlEnable(NL_NORMALIZE_ROWS);
	nlEnable(NL_VARIABLES_BUFFER);
	
	Attribute<bool> v_is_locked(M.vertices.attributes(), "selection");

	nlBegin(NL_SYSTEM);
	
	for(index_t coord=0; coord<M.vertices.dimension(); ++coord) {
	    // Bind directly the variables buffer to the coordinates in
	    // the mesh, to avoid copying data.
	    nlBindBuffer(
		NL_VARIABLES_BUFFER, NLuint(coord),
		M.vertices.point_ptr(0) + coord,
		NLuint(sizeof(double)*M.vertices.dimension())
	    );
	}
	
	for(index_t v: M.vertices) {
	    if(v_is_locked[v]) {
		nlLockVariable(v);
	    }
	}
	
	nlBegin(NL_MATRIX);
	for(index_t v: M.vertices) {
	    nlBegin(NL_ROW);
	    index_t count = 0;
	    for(index_t c = v2c[v]; c != index_t(-1); c = next_c_around_v[c]) {
		index_t f = c2f[c];
		index_t c2 = M.facets.next_corner_around_facet(f,c);
		index_t w = M.facet_corners.vertex(c2);
		nlCoefficient(w, 1.0);
		++count;
	    }
	    nlCoefficient(v, -double(count));
	    nlEnd(NL_ROW);
	}
	nlEnd(NL_MATRIX);
	nlEnd(NL_SYSTEM);

	nlSolve();

	nlDeleteContext(nlGetCurrent());
    }
    
}
