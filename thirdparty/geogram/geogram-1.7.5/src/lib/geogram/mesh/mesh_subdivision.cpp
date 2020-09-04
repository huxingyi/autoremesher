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

#include <geogram/mesh/mesh_subdivision.h>
#include <geogram/mesh/mesh.h>
#include <geogram/bibliography/bibliography.h>

namespace GEO {

    MeshSplitCallbacks::MeshSplitCallbacks(Mesh* mesh) : mesh_(mesh) {
    }

    MeshSplitCallbacks::~MeshSplitCallbacks() {
    }

    index_t MeshSplitCallbacks::create_vertex() {
	index_t result = mesh_->vertices.create_vertex();
	Memory::clear(
	    mesh_->vertices.point_ptr(result),
	    sizeof(double)*mesh_->vertices.dimension()
	);
	return result;
    }
    
    void MeshSplitCallbacks::scale_vertex(index_t v, double s) {
	double* p = mesh_->vertices.point_ptr(v);
	FOR(c,mesh_->vertices.dimension()) {
	    p[c] *= s;
	}
    }

    void MeshSplitCallbacks::zero_vertex(index_t v) {
	double* p = mesh_->vertices.point_ptr(v);
	FOR(c,mesh_->vertices.dimension()) {
	    p[c] = 0.0;
	}
    }
    
    void MeshSplitCallbacks::madd_vertex(index_t v1, double s, index_t v2) {
	double* p1 = mesh_->vertices.point_ptr(v1);
	const double* p2 = mesh_->vertices.point_ptr(v2);
	FOR(c,mesh_->vertices.dimension()) {
	    p1[c] += s * p2[c];
	}
    }

    /*************************************************************************/
    
    void mesh_split_triangles(
	Mesh& M, index_t facets_begin, index_t facets_end,
	MeshSplitCallbacks* cb
    ) {
	geo_assert(M.facets.are_simplices());

	MeshSplitCallbacks default_cb(&M);
	if(cb == nullptr) {
	    cb = &default_cb;
	}
	
	if(facets_end == index_t(-1)) {
	    facets_end = M.facets.nb();
	}
	
	index_t nv0 = M.vertices.nb();
	index_t nf0 = M.facets.nb();

	// Compute corner to new vertex mapping
	vector<index_t> ctov(M.facet_corners.nb(), NO_VERTEX);
	index_t nbnewv=0;
	for(index_t f=facets_begin; f<facets_end; ++f) {
	    for(index_t c=M.facets.corners_begin(f);
		c<M.facets.corners_end(f); ++c
	    ) {
		if(ctov[c] == index_t(-1)) {
		    ctov[c] = nbnewv;
		    index_t f2 = M.facet_corners.adjacent_facet(c);
		    if(f2 != NO_FACET) {
			for(index_t c2=M.facets.corners_begin(f2);
			    c2!=M.facets.corners_end(f2); ++c2) {
			    if(M.facet_corners.adjacent_facet(c2) == f) {
				ctov[c2] = nbnewv;
				break;
			    }
			}
		    }
		    ++nbnewv;
		}
	    }	    
	}

	// Create vertices
	M.vertices.create_vertices(nbnewv);
	for(index_t f=facets_begin; f<facets_end; ++f) {
	    for(index_t c1=M.facets.corners_begin(f);
		c1<M.facets.corners_end(f); ++c1
	    ) {
		index_t c2 = M.facets.next_corner_around_facet(f,c1);
		index_t v1 = M.facet_corners.vertex(c1);
		index_t v2 = M.facet_corners.vertex(c2);
		index_t v12 = ctov[c1] + nv0;
		cb->zero_vertex(v12);
		cb->madd_vertex(v12, 0.5, v1);
		cb->madd_vertex(v12, 0.5, v2);
	    }
	}

	// Create facets
	M.facets.create_triangles(3*(facets_end - facets_begin));
	for(index_t f=facets_begin; f<facets_end; ++f) {
	    index_t v1 = M.facets.vertex(f,0);
	    index_t v2 = M.facets.vertex(f,1);	
	    index_t v3 = M.facets.vertex(f,2);
	    index_t v12 = ctov[M.facets.corners_begin(f)    ] + nv0;
	    index_t v23 = ctov[M.facets.corners_begin(f) + 1] + nv0;
	    index_t v31 = ctov[M.facets.corners_begin(f) + 2] + nv0;
	    
	    M.facets.set_vertex(f,0,v31);
	    M.facets.set_vertex(f,1,v12);
	    M.facets.set_vertex(f,2,v23);

	    M.facets.attributes().copy_item(nf0+3*(f-facets_begin),f);
	    M.facets.set_vertex(nf0+3*(f-facets_begin),0,v1);
	    M.facets.set_vertex(nf0+3*(f-facets_begin),1,v12);
	    M.facets.set_vertex(nf0+3*(f-facets_begin),2,v31);

	    M.facets.attributes().copy_item(nf0+3*(f-facets_begin)+1,f);
	    M.facets.set_vertex(nf0+3*(f-facets_begin)+1,0,v12);
	    M.facets.set_vertex(nf0+3*(f-facets_begin)+1,1,v2);
	    M.facets.set_vertex(nf0+3*(f-facets_begin)+1,2,v23);

	    M.facets.attributes().copy_item(nf0+3*(f-facets_begin)+2,f);
	    M.facets.set_vertex(nf0+3*(f-facets_begin)+2,0,v31);
	    M.facets.set_vertex(nf0+3*(f-facets_begin)+2,1,v23);
	    M.facets.set_vertex(nf0+3*(f-facets_begin)+2,2,v3);	    	    
	}
	M.facets.connect();
    }


    void mesh_split_quads(
	Mesh& M, index_t facets_begin, index_t facets_end,
	MeshSplitCallbacks* cb	
    ) {
	MeshSplitCallbacks default_cb(&M);
	if(cb == nullptr) {
	    cb = &default_cb;
	}
	
	if(facets_end == index_t(-1)) {
	    facets_end = M.facets.nb();
	}
	
	index_t nv0 = M.vertices.nb();
	index_t nf0 = M.facets.nb();

	// Compute corner to new vertex and facet to new vertex
	// mappings.
	
	vector<index_t> ctov(M.facet_corners.nb(), NO_VERTEX);
	vector<index_t> ftov(M.facets.nb(), NO_VERTEX);
	
	index_t nbnewv=0;
	index_t nbnewf=0;
	
	for(index_t f=facets_begin; f<facets_end; ++f) {
	    ftov[f] = nbnewv;
	    ++nbnewv;
	    for(index_t c=M.facets.corners_begin(f);
		c<M.facets.corners_end(f); ++c
	    ) {
		++nbnewf;
		if(ctov[c] == index_t(-1)) {
		    ctov[c] = nbnewv;
		    index_t f2 = M.facet_corners.adjacent_facet(c);
		    if(f2 != NO_FACET) {
			for(index_t c2=M.facets.corners_begin(f2);
			    c2!=M.facets.corners_end(f2); ++c2) {
			    if(M.facet_corners.adjacent_facet(c2) == f) {
				ctov[c2] = nbnewv;
				break;
			    }
			}
		    }
		    ++nbnewv;
		}
	    }	    
	}

	// Create vertices
	M.vertices.create_vertices(nbnewv);
	for(index_t f=facets_begin; f<facets_end; ++f) {
	    cb->zero_vertex(ftov[f] + nv0);
	    for(index_t c1=M.facets.corners_begin(f);
		c1<M.facets.corners_end(f); ++c1
	    ) {
		index_t c2 = M.facets.next_corner_around_facet(f,c1);
		index_t v1 = M.facet_corners.vertex(c1);
		index_t v2 = M.facet_corners.vertex(c2);
		index_t v12 = ctov[c1] + nv0;

		cb->madd_vertex(ftov[f] + nv0, 1.0, v1);
		
		cb->zero_vertex(v12);
		cb->madd_vertex(v12,0.5,v1);
		cb->madd_vertex(v12,0.5,v2);
	    }
	    double s = 1.0 / double(M.facets.nb_vertices(f));
	    cb->scale_vertex(ftov[f]+nv0, s);
	}

	// Create facets
	M.facets.create_quads(nbnewf);
	index_t cur_f = 0;
	for(index_t f=facets_begin; f<facets_end; ++f) {
	    for(
		index_t c1=M.facets.corners_begin(f);
		c1<M.facets.corners_end(f); ++c1) {
		index_t c0 = M.facets.prev_corner_around_facet(f,c1);
		index_t v = M.facet_corners.vertex(c1);
		index_t v1 = ctov[c0] + nv0;
		index_t v2 = v;
		index_t v3 = ctov[c1] + nv0;
		index_t v4 = ftov[f]  + nv0;
		M.facets.attributes().copy_item(cur_f + nf0, f);
		M.facets.set_vertex(cur_f + nf0, 0, v1);
		M.facets.set_vertex(cur_f + nf0, 1, v2);
		M.facets.set_vertex(cur_f + nf0, 2, v3);
		M.facets.set_vertex(cur_f + nf0, 3, v4);
		++cur_f;
	    }
	}

	vector<index_t> to_delete(M.facets.nb(),0);
	for(index_t f=facets_begin; f<facets_end; ++f) {
	    to_delete[f] = 1;
	}
	M.facets.delete_elements(to_delete);
	M.facets.connect();
    }

    void mesh_split_catmull_clark(Mesh& M, MeshSplitCallbacks* cb) {

	geo_cite("journals/CAD/CatmullRGB");
	
	MeshSplitCallbacks default_cb(&M);
	if(cb == nullptr) {
	    cb = &default_cb;
	}

	vector<index_t> vertex_degree(M.vertices.nb(),0);
	vector<index_t> corner_vertex(M.facet_corners.nb(),NO_VERTEX);
	vector<index_t> facet_vertex(M.facets.nb(), NO_VERTEX);
	std::vector<bool> v_on_border(M.vertices.nb(),false);

	index_t nb_v_orig = M.vertices.nb();
	index_t nb_f_orig = M.facets.nb();
	
	// Create edge and facet vertices
	for(index_t f1: M.facets) {
	    facet_vertex[f1] = cb->create_vertex();
	    for(index_t c1: M.facets.corners(f1)) {
		index_t v = M.facet_corners.vertex(c1);
		++vertex_degree[v];
		index_t f2 = M.facet_corners.adjacent_facet(c1);

		if(f1 < f2 || f2 == NO_FACET) {
		    corner_vertex[c1] = cb->create_vertex();
		    if(f2 != NO_FACET) {
			index_t cn = M.facets.next_corner_around_facet(f1,c1);
			index_t v2 = M.facet_corners.vertex(cn);
			index_t c2 = NO_CORNER;
			for(c2=M.facets.corners_begin(f2);
			    c2 != M.facets.corners_end(f2); ++c2) {
			    if(M.facet_corners.vertex(c2) == v2) {
				break;
			    }
			}
			geo_assert(M.facet_corners.vertex(c2) == v2);
			corner_vertex[c2] = corner_vertex[c1];
		    }
		}
	    }
	}
	
	// Compute facet vertices
	for(index_t f: M.facets) {
	    double f_degree = double(M.facets.nb_vertices(f));
	    for(index_t c: M.facets.corners(f)) {
		index_t v = M.facet_corners.vertex(c);
		cb->madd_vertex(facet_vertex[f], 1.0 / f_degree, v);
		if(M.facet_corners.adjacent_facet(c) == NO_FACET) {
		    v_on_border[v] = true;
		}
	    }		    
	}

	// Compute edge vertices
	for(index_t f: M.facets) {
	    for(index_t c: M.facets.corners(f)) {
		index_t v = M.facet_corners.vertex(c);
		if(M.facet_corners.adjacent_facet(c) == NO_FACET) {
		    cb->madd_vertex(corner_vertex[c], 1.0/2.0, v);
		    index_t c2 = M.facets.next_corner_around_facet(f,c);
		    index_t v2 = M.facet_corners.vertex(c2);
		    cb->madd_vertex(corner_vertex[c], 1.0/2.0, v2);
		} else {
		    cb->madd_vertex(corner_vertex[c], 0.25, v);
		    cb->madd_vertex(corner_vertex[c], 0.25, facet_vertex[f]);
		}
	    }
	}

	// Compute new position of original vertices

	FOR(v, nb_v_orig) {
	    if(v_on_border[v]) {
		continue;
	    }
	    double n = double(vertex_degree[v]);
	    if(n != 0.0) {
		if(!v_on_border[v]) {
		    cb->scale_vertex(v, (n - 3.0) / n);
		}
	    }
	}

	for(index_t f: M.facets) {
	    for(index_t c: M.facets.corners(f)) {
		index_t v = M.facet_corners.vertex(c);
		double n = double(vertex_degree[v]);
		
		// As compared to original Catmull-Clark documentation:
		//   add 4.0 times edge vertex
		//   then remove contribution of facet vertices
		//   (this retrieves the original edges barycenters without
		//    needing intermediary storage).
		
		if(!v_on_border[v]) {
		    index_t f2 = M.facet_corners.adjacent_facet(c);
		    cb->madd_vertex(v,  4.0 / (n*n), corner_vertex[c]);
		    cb->madd_vertex(v, -1.0 / (n*n), facet_vertex[f2]);
		}
	    }	    
	}
	
	// Create new facets
	FOR(f, nb_f_orig) {
	    for(index_t c: M.facets.corners(f)) {
		index_t v = M.facet_corners.vertex(c);
		index_t c2 = M.facets.prev_corner_around_facet(f,c);
		index_t new_f = M.facets.create_quad(
		    corner_vertex[c2],
		    v,
		    corner_vertex[c],
		    facet_vertex[f]
		);
		M.facets.attributes().copy_item(new_f, f);
	    }	    
	}

	// Delete old facets
	vector<index_t> delete_f(nb_f_orig, 1);
	delete_f.resize(M.facets.nb(),0);
	M.facets.delete_elements(delete_f);
	M.facets.connect();
    }
}
