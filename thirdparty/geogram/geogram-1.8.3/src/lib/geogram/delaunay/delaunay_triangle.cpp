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

#ifdef GEOGRAM_WITH_TRIANGLE


#include <geogram/delaunay/delaunay_triangle.h>
#include <geogram/mesh/mesh.h>
#include <geogram/bibliography/bibliography.h>

namespace {

    /**
     * \brief Initializes a triangulateio.
     * \param[in] tri a pointer to the triangulateio
     *  structure to be initialized
     */
    void init_triangulateio(struct triangulateio* tri) {
        memset(tri, 0, sizeof(struct triangulateio));
    }

    /**
     * \brief Deallocates the memory used by a triangulateio.
     * \details Only the memory used by the triangulateio is freed,
     *  not the triangulateio struct.
     * \param[in] tri a pointer to the triangulateio
     */
    void free_triangulateio(struct triangulateio* tri) {
        free(tri->pointlist);
        free(tri->pointattributelist);
        free(tri->pointmarkerlist);
        free(tri->trianglelist);
        free(tri->triangleattributelist);
        free(tri->trianglearealist);
        free(tri->neighborlist);
        free(tri->segmentlist);
        free(tri->segmentmarkerlist);
        free(tri->holelist);
        free(tri->regionlist);
        free(tri->edgelist);
        free(tri->edgemarkerlist);
        free(tri->normlist);
        memset(tri, 0, sizeof(struct triangulateio));
    }
    
}

namespace GEO {

    DelaunayTriangle::DelaunayTriangle(
        coord_index_t dimension
    ) : Delaunay(2) {
        if(dimension != 2) {
            throw InvalidDimension(dimension, "DelaunayTriangle", "2");
        }
        init_triangulateio(&triangle_in_);
        init_triangulateio(&triangle_out_);
	
        geo_cite("DBLP:conf/wacg/Shewchuk96");
    }

    bool DelaunayTriangle::supports_constraints() const {
        return true;
    }
    
    void DelaunayTriangle::set_vertices(
        index_t nb_vertices, const double* vertices
    ) {
        if(constraints_ != nullptr) {
            set_vertices_constrained(nb_vertices, vertices);
        } else {
            set_vertices_unconstrained(nb_vertices, vertices);
        }
    }
    
    void DelaunayTriangle::set_vertices_unconstrained(
        index_t nb_vertices, const double* vertices
    ) {
        Delaunay::set_vertices(nb_vertices, vertices);
        free_triangulateio(&triangle_out_);
        triangle_in_.numberofpoints = int(nb_vertices);
        triangle_in_.pointlist = const_cast<double*>(vertices);
        // Q: quiet
        // z: numbering starts from 0
        // n: output neighbors
        triangulate(
            const_cast<char*>("Qzn"), &triangle_in_, &triangle_out_, nullptr
        );
        set_arrays(
            index_t(triangle_out_.numberoftriangles), 
            triangle_out_.trianglelist, triangle_out_.neighborlist
        );
    }

    void DelaunayTriangle::set_vertices_constrained(
        index_t nb_vertices, const double* vertices
    ) {
        // For now, everything is taken from the constraints
        geo_assert(nb_vertices == 0);
        geo_assert(vertices == nullptr);        

        nb_vertices = constraints_->vertices.nb();
        vertices = constraints_->vertices.point_ptr(0);
        
        free_triangulateio(&triangle_out_);
        
        triangle_in_.numberofpoints = int(nb_vertices);
        triangle_in_.pointlist = const_cast<double*>(vertices);
        triangle_in_.numberofsegments = int(constraints_->edges.nb());
        triangle_in_.segmentlist = reinterpret_cast<int*>(const_cast<index_t*>(
            constraints_->edges.vertex_index_ptr(0)
        ));
        
        // Q: quiet
        // z: numbering starts from 0
        // n: output neighbors
        // p: Planar Straight Line Graph
        triangulate(
            const_cast<char*>("Qznp"), &triangle_in_, &triangle_out_, nullptr
        );

        Delaunay::set_vertices(
            index_t(triangle_out_.numberofpoints),
            triangle_out_.pointlist
        );
        
        set_arrays(
            index_t(triangle_out_.numberoftriangles), 
            triangle_out_.trianglelist,
            triangle_out_.neighborlist
        );

        if(triangle_out_.numberofpoints != triangle_in_.numberofpoints) {
            std::cerr << "Triangle: created "
                      <<  triangle_out_.numberofpoints -
                          triangle_in_.numberofpoints
                      << " points"
                      << std::endl;
        }
    }
    
    DelaunayTriangle::~DelaunayTriangle() {
        free_triangulateio(&triangle_out_);
    }

    
}

#endif
