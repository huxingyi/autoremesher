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

#include <geogram/mesh/mesh.h>
#include <geogram/basic/permutation.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/algorithm.h>
#include <geogram/basic/string.h>

namespace GEO {

    MeshSubElementsStore::MeshSubElementsStore(Mesh& mesh) :
        mesh_(mesh),
        nb_(0) {
    }

    MeshSubElementsStore::~MeshSubElementsStore() {
    }

    void MeshSubElementsStore::clear_store(
        bool keep_attributes, bool keep_memory
    ) {
        attributes_.clear(keep_attributes, keep_memory);
        nb_ = 0;
    }

    void MeshSubElementsStore::resize_store(index_t new_size) {
        attributes_.resize(new_size);
        nb_ = new_size;
    }
    
    /*************************************************************************/

    MeshElements::MeshElements() {
    }

    MeshElements::~MeshElements() {
    }
    
    /*************************************************************************/
    
    MeshVertices::MeshVertices(Mesh& mesh) :
        MeshSubElementsStore(mesh),
        edges_(mesh.edges),
        facet_corners_(mesh.facet_corners),
        cell_corners_(mesh.cell_corners) {
    }

    MeshVertices::~MeshVertices() {
        if(point_.is_bound()) {
            point_.unbind();
        }
        if(point_fp32_.is_bound()) {
            point_fp32_.unbind();
        }
    }


    void MeshVertices::set_double_precision() {
        if(double_precision()) {
            return;
        }

        index_t dim = dimension();
        
        point_.create_vector_attribute(
            attributes(), "point", dim
        );

        for(index_t i=0; i<point_.nb_elements(); ++i) {
            point_[i] = double(point_fp32_[i]);
        }
        
        point_fp32_.destroy();
    }

    void MeshVertices::set_single_precision() {
        if(single_precision()) {
            return;
        }

        index_t dim = dimension();
        
        point_fp32_.create_vector_attribute(
            attributes(), "point_fp32", dim
        );

        for(index_t i=0; i<point_.nb_elements(); ++i) {
            point_fp32_[i] = float(point_[i]);
        }
        
        point_.destroy();        
    }

    
    void MeshVertices::clear(bool keep_attributes, bool keep_memory) {
        bool singlep = single_precision();
        index_t dim = dimension();
        
        //   We need to unbind point attributes
        // because it is not correct to clear the
        // AttributesManager when an attribute is
        // still bound.
        if(!keep_attributes) {
            if(point_.is_bound()) {
                point_.unbind();
            }
            if(point_fp32_.is_bound()) {
                point_fp32_.unbind();
            }
        }
        
        clear_store(keep_attributes, keep_memory);

        //  Now we can re-create the point attributes.
        if(!keep_attributes) {
            bind_point_attribute(dim,singlep);
        }
    }
    
    void MeshVertices::clear_store(
        bool keep_attributes, bool keep_memory 
    ) {
        MeshSubElementsStore::clear_store(keep_attributes, keep_memory);
    }

    void MeshVertices::resize_store(index_t new_size) {
        MeshSubElementsStore::resize_store(new_size);
    }

    void MeshVertices::delete_elements(
        vector<index_t>& to_delete,
        bool remove_isolated_vertices
    ) {
        // to_delete is reused for re-indexing
        vector<index_t>& old2new = to_delete;
        geo_argused(remove_isolated_vertices);
        if(has_non_zero(to_delete)) {
            index_t cur=0;
            for(index_t i=0; i<old2new.size(); ++i) {
                if(old2new[i] == 0) {
                    old2new[i] = cur;
                    ++cur;
                } else {
                    old2new[i] = index_t(-1);
                }
            }
            attributes_.compress(old2new);
            // cur now contains the new size.
            resize_store(cur);

            for(index_t e=0; e<edges_.nb(); ++e) {
                for(index_t lv=0; lv<2; ++lv) {
                    index_t v = edges_.vertex(e,lv);
                    v = old2new[v];
                    edges_.set_vertex(e,lv,v);
                }
            }
            
            for(index_t c=0; c<facet_corners_.nb(); ++c) {
                index_t v = facet_corners_.vertex(c);
                v = old2new[v];
                facet_corners_.set_vertex(c,v);
            }

            for(index_t c=0; c<cell_corners_.nb(); ++c) {
                index_t v = cell_corners_.vertex(c);
                // Cells can have padding
                if(v == NO_VERTEX) {
                    continue;
                }
                v = old2new[v];
                cell_corners_.set_vertex(c,v);
            }
        }
    }

    void MeshVertices::permute_elements(vector<index_t>& permutation) {
        attributes_.apply_permutation(permutation);
        Permutation::invert(permutation);

        for(index_t e=0; e<edges_.nb(); ++e) {
            for(index_t lv=0; lv<2; ++lv) {
                index_t v = edges_.vertex(e,lv);
                v = permutation[v];
                edges_.set_vertex(e,lv,v);
            }
        }
        
        for(index_t c=0; c<facet_corners_.nb(); ++c) {
            index_t v = facet_corners_.vertex(c);
            v = permutation[v];
            facet_corners_.set_vertex(c,v);
        }

        for(index_t c=0; c<cell_corners_.nb(); ++c) {
            index_t v = cell_corners_.vertex(c);
            // Cells can have padding
            if(v == NO_VERTEX) {
                continue;
            }
            v = permutation[v];
            cell_corners_.set_vertex(c,v);
        }
    }

    void MeshVertices::remove_isolated() {
        vector<index_t> to_delete(nb(),1);

        for(index_t e=0; e<mesh_.edges.nb(); ++e) {
            for(index_t lv=0; lv<2; ++lv) {
                index_t v = mesh_.edges.vertex(e,lv);
                to_delete[v] = 0;
            }
        }
        
        for(index_t f=0; f<mesh_.facets.nb(); ++f) {
            for(index_t co=mesh_.facets.corners_begin(f);
                co<mesh_.facets.corners_end(f); ++co
            ) {
                index_t v = mesh_.facet_corners.vertex(co);
                to_delete[v] = 0;
            }
        }

        for(index_t c=0; c<mesh_.cells.nb(); ++c) {
            for(
                index_t co=mesh_.cells.corners_begin(c);
                co < mesh_.cells.corners_end(c); ++co
            ) {
                index_t v = mesh_.cell_corners.vertex(co);
                to_delete[v] = 0;
            }
        }
        
        delete_elements(to_delete);
    }

    void MeshVertices::bind_point_attribute(
        index_t dim, bool single_precision
    ) {
        if(single_precision) {
            point_fp32_.create_vector_attribute(
                attributes(), "point_fp32", dim
            );
        } else {
            point_.create_vector_attribute(
                attributes(), "point", dim
            );
        }
    }

    void MeshVertices::assign_points(
        vector<double>& points, index_t dim, bool steal_arg
    ) {
        // TODO: implement steal_arg
        geo_argused(steal_arg);
        index_t nb_pts = points.size()/dim;
        geo_assert(dim*nb_pts == points.size());
        assign_points(points.data(), dim, nb_pts);
    }

    void MeshVertices::assign_points(
        const double* points, index_t dim, index_t nb_pts
    ) {
        geo_assert(!single_precision());
        if(dim != dimension() || nb_pts != nb()) {
            clear(true,false);
            set_dimension(dim);
            create_vertices(nb_pts);
        }
        Memory::copy(
            point_ptr(0), points, nb_pts*dim*sizeof(double)
        );
    }

    void MeshVertices::pop() {
        geo_debug_assert(nb() != 0);
        --nb_;
    }
    
    /**************************************************************************/

    MeshEdges::MeshEdges(Mesh& mesh) : MeshSubElementsStore(mesh) {
    }
    
    MeshEdges::~MeshEdges() {
    }
    
    void MeshEdges::delete_elements(
        vector<index_t>& to_delete, bool remove_isolated_vertices
    ) {
        geo_debug_assert(to_delete.size() == nb());

        // "Fast track" if no element should be deleted
        if(!has_non_zero(to_delete)) {
            if(remove_isolated_vertices) {
                mesh_.vertices.remove_isolated();
            }
            return;
        }

        // to_delete is used for both indicating
        // which facets should be deleted and
        // for storing the re-numbering map
        vector<index_t>& edges_old2new = to_delete;
        index_t new_nb_edges = 0;

        for(index_t e = 0; e < nb(); ++e) {
            if(edges_old2new[e] != 0) {
                edges_old2new[e] = NO_FACET;
            } else {
                edges_old2new[e] = new_nb_edges;
                if(new_nb_edges != e) {
                    edge_vertex_[2*new_nb_edges]   = edge_vertex_[2*e];
                    edge_vertex_[2*new_nb_edges+1] = edge_vertex_[2*e+1];
                }
                ++new_nb_edges;
            }
        }

        // Manage facets store and attributes
        attributes().compress(edges_old2new);
        resize_store(new_nb_edges);

        if(remove_isolated_vertices) {
            mesh_.vertices.remove_isolated();
        }
    }
    
    void MeshEdges::permute_elements(vector<index_t>& permutation) {
        attributes_.apply_permutation(permutation);
        Permutation::apply(
            edge_vertex_.data(),
            permutation,
            index_t(sizeof(index_t) * 2)
        );
    }
    
    void MeshEdges::clear(bool keep_attributes, bool keep_memory) {
        clear_store(keep_attributes, keep_memory);
    }

    void MeshEdges::pop() {
        geo_debug_assert(nb() != 0);
        resize_store(nb()-1);
    }

    void MeshEdges::clear_store(
        bool keep_attributes, bool keep_memory
    ) {
        if(keep_memory) {
            edge_vertex_.resize(0);
        } else {
            edge_vertex_.clear();
        }
        MeshSubElementsStore::clear_store(keep_attributes, keep_memory);
    }
        
    void MeshEdges::resize_store(index_t new_size) {
        edge_vertex_.resize(new_size*2,NO_VERTEX);
        MeshSubElementsStore::resize_store(new_size);
    }


    /**************************************************************************/
    
    MeshFacetsStore::MeshFacetsStore(Mesh& mesh) :
        MeshSubElementsStore(mesh),
        is_simplicial_(true) {
        facet_ptr_.push_back(0);
    }

    void MeshFacetsStore::clear_store(
        bool keep_attributes, bool keep_memory
    ) {
        if(keep_memory) {
            facet_ptr_.resize(0);
        } else {
            facet_ptr_.clear();
        }
        MeshSubElementsStore::clear_store(keep_attributes, keep_memory);
        facet_ptr_.push_back(0);
    }

    void MeshFacetsStore::resize_store(index_t new_size) {
        if(!is_simplicial_) {
            facet_ptr_.resize(new_size+1);
        }
        MeshSubElementsStore::resize_store(new_size);
    }

    /**************************************************************************/

    MeshFacetCornersStore::MeshFacetCornersStore(Mesh& mesh) :
        MeshSubElementsStore(mesh),
        vertices_(mesh.vertices),
        facets_(mesh.facets) {
    }

    void MeshFacetCornersStore::clear_store(
        bool keep_attributes, bool keep_memory
    ) {
        if(keep_memory) {
            corner_vertex_.resize(0);
            corner_adjacent_facet_.resize(0);
        } else {
            corner_vertex_.clear();
            corner_adjacent_facet_.clear();
        }
        MeshSubElementsStore::clear_store(keep_attributes, keep_memory);
    }
    
    void MeshFacetCornersStore::resize_store(index_t new_size) {
        corner_vertex_.resize(new_size);
        corner_adjacent_facet_.resize(new_size);
        MeshSubElementsStore::resize_store(new_size);
    }
    
    /**************************************************************************/

    MeshFacets::MeshFacets(Mesh& mesh) :
        MeshFacetsStore(mesh),
        vertices_(mesh.vertices),
        facet_corners_(mesh.facet_corners) {
    }

    void MeshFacets::clear(bool keep_attributes, bool keep_memory) {
        facet_corners_.clear_store(keep_attributes, keep_memory);
        clear_store(keep_attributes, keep_memory);
	is_simplicial();
    }
    
    void MeshFacets::delete_elements(
        vector<index_t>& to_delete,
        bool remove_isolated_vertices
    ) {
        geo_debug_assert(to_delete.size() == nb());

        // "Fast track" if no element should be deleted
        if(!has_non_zero(to_delete)) {
            if(remove_isolated_vertices) {
                mesh_.vertices.remove_isolated();
            }
            return;
        }
        
        // to_delete is used for both indicating
        // which facets should be deleted and
        // for storing the re-numbering map
        vector<index_t>& facets_old2new = to_delete;

        vector<index_t>& corner_vertex = facet_corners_.corner_vertex_;
        vector<index_t>& corner_adjacent_facet =
            facet_corners_.corner_adjacent_facet_;
        
        index_t new_nb_facets = 0;
        index_t new_nb_corners = 0;

        // If there are some corner attributes, we need
        // to compute the index mapping for them.
        vector<index_t> corners_old2new;
        if(facet_corners_.attributes().nb() != 0) {
            corners_old2new.resize(facet_corners_.nb(), index_t(-1));
        }

        for(index_t f = 0; f < nb(); ++f) {
            if(facets_old2new[f] != 0) {
                facets_old2new[f] = NO_FACET;
            } else {
                facets_old2new[f] = new_nb_facets;
                if(!is_simplicial_) {
                    facet_ptr_[new_nb_facets] = new_nb_corners;
                }
                for(index_t co = corners_begin(f); co != corners_end(f); ++co) {
                    if(corners_old2new.size() != 0) {
                        corners_old2new[co] = new_nb_corners;
                    }
                    if(co != new_nb_corners) {
                        corner_vertex[new_nb_corners] =
                            corner_vertex[co];
                        corner_adjacent_facet[new_nb_corners] =
                            corner_adjacent_facet[co];
                    }
                    new_nb_corners++;
                }
                new_nb_facets++;
            }
        }
        
        if(!is_simplicial_) {
            facet_ptr_[new_nb_facets] = new_nb_corners;
        }

        // Map adjacent facets indices
        for(index_t c = 0; c < facet_corners_.nb(); ++c) {
            index_t f = corner_adjacent_facet[c];
            if(f != NO_FACET) {
                corner_adjacent_facet[c] = facets_old2new[f];
            }
        }

        // Manage facets store and attributes
        attributes().compress(facets_old2new);
        resize_store(new_nb_facets);

        // Manage corners store and attributes
        if(corners_old2new.size() != 0) {
            // corners index mapping is computed only if there
            // were some corner attributes.
            facet_corners_.attributes().compress(corners_old2new);
        }
        facet_corners_.resize_store(new_nb_corners);
        
        if(remove_isolated_vertices) {
            mesh_.vertices.remove_isolated();
        }
    }
        
    void MeshFacets::permute_elements(vector<index_t>& permutation) {
        attributes_.apply_permutation(permutation);

        vector<index_t>& corner_vertex = facet_corners_.corner_vertex_;
        vector<index_t>& corner_adjacent_facet =
            facet_corners_.corner_adjacent_facet_;

        if(facet_corners_.attributes().nb() != 0) {
            vector<index_t> facet_corners_permutation;
            facet_corners_permutation.reserve(facet_corners_.nb());

            for(index_t new_f=0; new_f<nb(); ++new_f) {
                index_t old_f = permutation[new_f];
                for(
                    index_t old_c=corners_begin(old_f);
                    old_c<corners_end(old_f); ++old_c) {
                    facet_corners_permutation.push_back(old_c);
                }
            }
            
            facet_corners_.attributes().apply_permutation(
                facet_corners_permutation
            );
        }
        
        if(is_simplicial_) {
            // If the surface is triangulated,
            // everything can be done in-place (great !!)
            
            Permutation::apply(
                corner_vertex.data(),
                permutation,
                index_t(sizeof(index_t) * 3)
            );

            Permutation::apply(
                corner_adjacent_facet.data(),
                permutation,
                index_t(sizeof(index_t) * 3)
            );

            Permutation::invert(permutation);
                
            for(index_t c = 0; c < corner_adjacent_facet.size(); ++c) {
                if(corner_adjacent_facet[c] != NO_FACET) {
                    corner_adjacent_facet[c] =
                        permutation[corner_adjacent_facet[c]];
                }
            }
            
        } else {

            {
                vector<index_t> new_corner_vertex;
                new_corner_vertex.reserve(corner_vertex.size());
                vector<index_t> new_corner_adjacent_facet;
                new_corner_adjacent_facet.reserve(corner_adjacent_facet.size());
                vector<index_t> new_facet_ptr;
                new_facet_ptr.reserve(nb()+1);

                new_facet_ptr.push_back(0);
                for(index_t new_f=0; new_f<nb(); ++new_f) {
                    index_t old_f = permutation[new_f];
                    for(
                        index_t old_c = corners_begin(old_f);
                        old_c < corners_end(old_f); ++old_c
                    ) {
                        new_corner_vertex.push_back(
                            mesh_.facet_corners.vertex(old_c)
                        );
                        new_corner_adjacent_facet.push_back(
                            mesh_.facet_corners.adjacent_facet(old_c)
                        );
                    }
                    new_facet_ptr.push_back(
                        new_facet_ptr[new_facet_ptr.size()-1] +
                        nb_vertices(old_f)
                    );
                }

                corner_vertex.swap(new_corner_vertex);
                corner_adjacent_facet.swap(new_corner_adjacent_facet);
                facet_ptr_.swap(new_facet_ptr);
            }


            Permutation::invert(permutation);
                
            for(index_t c = 0; c < corner_adjacent_facet.size(); ++c) {
                if(corner_adjacent_facet[c] != NO_FACET) {
                    corner_adjacent_facet[c] =
                        permutation[corner_adjacent_facet[c]];
                }
            }
        }
    }

    void MeshFacets::connect() {
        // Chains the corners around each vertex.
        vector<index_t> next_corner_around_vertex(
            facet_corners_.nb(), NO_CORNER
        );
        
        // Gives for each vertex a corner incident to it.
        vector<index_t> v2c(vertices_.nb(), NO_CORNER);
        
        // Gives for each corner the facet incident to it
        // (or use c/3 if the surface is triangulated).
        GEO::vector<index_t> c2f;
        if(!is_simplicial_) {
            c2f.assign(facet_corners_.nb(), NO_FACET);
        }
        
        // Step 1: chain corners around vertices and compute v2c
        for(index_t f = 0; f < nb(); ++f) {
            for(index_t c = corners_begin(f); c < corners_end(f); ++c) {
                index_t v = facet_corners_.vertex(c);
                next_corner_around_vertex[c] = v2c[v];
                v2c[v] = c;
            }
        }
        
        // compute c2f is needed
        if(!is_simplicial_) {
            for(index_t f = 0; f < nb(); ++f) {
                for(index_t c = corners_begin(f); c < corners_end(f); ++c) {
                    c2f[c] = f;
                }
            }
        }
        
        // Step 2: connect
        for(index_t f1 = 0; f1 < nb(); ++f1) {
            for(index_t c1 = corners_begin(f1); c1 < corners_end(f1); ++c1) {
                if(facet_corners_.adjacent_facet(c1) == NO_FACET) {
                    index_t v2 = facet_corners_.vertex(
                        next_corner_around_facet(f1, c1)
                    );
                    //   Traverse all the corners c2 incident to v1, and
                    // find among them the one that is opposite to c1
                    for(
                        index_t c2 = next_corner_around_vertex[c1];
                        c2 != NO_CORNER; c2 = next_corner_around_vertex[c2]
                    ) {
                        if(c2 != c1) {
                            index_t f2 = is_simplicial_ ? c2/3 : c2f[c2];
                            index_t c3 = prev_corner_around_facet(f2, c2);
                            index_t v3 = facet_corners_.vertex(c3);
                            if(v3 == v2) {
                                facet_corners_.set_adjacent_facet(c1, f2);
                                facet_corners_.set_adjacent_facet(c3, f1);
                                break; 
                            }
                        }
                    }
                }
            }
        }
    }

    void MeshFacets::triangulate() {
        if(is_simplicial_) {
            return;
        }
        index_t nb_triangles = 0;
        for(index_t f = 0; f < nb(); f++) {
            nb_triangles += (nb_vertices(f) - 2);
        }
        vector<index_t> new_corner_vertex_index;
        new_corner_vertex_index.reserve(nb_triangles * 3);
        for(index_t f = 0; f < nb(); f++) {
            index_t v0 = facet_corners_.vertex(corners_begin(f));
            for(index_t c = corners_begin(f) + 1;
                c + 1 < corners_end(f); ++c
            ) {
                new_corner_vertex_index.push_back(v0);
                new_corner_vertex_index.push_back(
                    facet_corners_.vertex(c)
                );
                new_corner_vertex_index.push_back(
                    facet_corners_.vertex(c + 1)
                );
            }
        }
        assign_triangle_mesh(new_corner_vertex_index, true);
    }

    void MeshFacets::flip(index_t f) {
        index_t d = nb_vertices(f);
        
        // Allocated on the stack (more multithread-friendly
        // and no need to free)
        index_t* corner_vertex_index =
            (index_t*) alloca(sizeof(index_t) * d);
        
        index_t* corner_adjacent_facet =
            (index_t*) alloca(sizeof(index_t) * d);
        
        index_t c0 = corners_begin(f);
        for(index_t i = 0; i < d; i++) {
            corner_vertex_index[i] = facet_corners_.vertex(c0 + i);
            corner_adjacent_facet[i] = facet_corners_.adjacent_facet(c0 + i);
        }
        for(index_t i = 0; i < d; i++) {
            index_t i_v = d - 1 - i;
            index_t i_f = (i_v == 0) ? d - 1 : i_v - 1;
            facet_corners_.set_vertex(c0 + i, corner_vertex_index[i_v]);
            facet_corners_.set_adjacent_facet(
                c0 + i, corner_adjacent_facet[i_f]
            );
        }
    }

    void MeshFacets::compute_borders() {
        mesh_.edges.clear();
        for(index_t f=0; f<nb(); ++f) {
            for(index_t c1=corners_begin(f); c1!=corners_end(f); ++c1) {
                if(mesh_.facet_corners.adjacent_facet(c1) == NO_FACET) {
                    index_t c2 = next_corner_around_facet(f,c1);
                    mesh_.edges.create_edge(
                        mesh_.facet_corners.vertex(c1),
                        mesh_.facet_corners.vertex(c2)
                    );
                }
            }
        }
    }

    void MeshFacets::assign_triangle_mesh(
        coord_index_t dim,
        vector<double>& vertices,
        vector<index_t>& triangles,
        bool steal_args
    ) {
        vertices_.assign_points(vertices, dim, steal_args);
        assign_triangle_mesh(triangles, steal_args);
    }

    void MeshFacets::assign_triangle_mesh(
        vector<index_t>& triangles,
        bool steal_args
    ) {
        index_t nb_triangles = triangles.size()/3;
	is_simplicial();
        facet_ptr_.clear();
        resize_store(nb_triangles);
        if(steal_args) {
            facet_corners_.corner_vertex_.swap(triangles);
        } else {
            facet_corners_.corner_vertex_ = triangles;
        }
        facet_corners_.resize_store(nb_triangles*3);
        facet_corners_.corner_adjacent_facet_.assign(
            nb_triangles*3, NO_FACET
        );
        attributes().zero();
        facet_corners_.attributes().zero();
    }
    
    void MeshFacets::pop() {
        geo_debug_assert(nb() != 0);
        index_t new_nb_corners =
            is_simplicial_ ? 3*(nb()-1) : facet_ptr_[nb()-1];
        resize_store(nb()-1);
        facet_corners_.resize_store(new_nb_corners);
    }

    
    /**************************************************************************/

    namespace MeshCellDescriptors {

        GEOGRAM_API CellDescriptor tet_descriptor = {
            4,         // nb_vertices
            4,         // nb_facets
            {3,3,3,3}, // nb_vertices in facet
            {          // facets
                {1,3,2},
                {0,2,3},
                {3,1,0},
                {0,1,2}
            },
            6,         // nb_edges
            {          // edges
                {1,2}, {2,3}, {3,1}, {0,1}, {0,2}, {0,3}
            },
            {          // edges adjacent facets
                {0,3}, {0,1}, {0,2}, {2,3}, {3,1}, {1,2}
            }
        };
        

        GEOGRAM_API CellDescriptor hex_descriptor = {
            8,             // nb_vertices
            6,             // nb_facets
            {4,4,4,4,4,4}, // nb_vertices in facet
            {              // facets
                {0,2,6,4},
                {3,1,5,7},
                {1,0,4,5},
                {2,3,7,6},
                {1,3,2,0},
                {4,6,7,5}
            },
            12,            // nb_edges
            {              // edges
                {0,1},{1,3},{3,2},{2,0},{4,5},{5,7},
                {7,6},{6,4},{0,4},{1,5},{3,7},{2,6}
            },
            {              // edges adjacent facets
                {4,2},{4,1},{4,3},{4,0},{2,5},{1,5},
                {3,5},{0,5},{2,0},{1,2},{3,1},{0,3}
            }
        };

        GEOGRAM_API CellDescriptor prism_descriptor = {
            6,             // nb_vertices
            5,             // nb_facets
            {3,3,4,4,4},   // nb_vertices in facet
            {              // facets
                {0,1,2},
                {3,5,4},
                {0,3,4,1},
                {0,2,5,3},
                {1,4,5,2}
            },
            9,             // nb_edges
            {              // edges
                {0,1},{1,2},{2,0},{3,4},{4,5},{5,3},{0,3},{1,4},{2,5}
            },
            {              // edges adjacent facets
                {2,0},{4,0},{3,0},{1,2},{1,4},{1,3},{3,2},{2,4},{4,3}
            }
        };


        GEOGRAM_API CellDescriptor pyramid_descriptor = {
            5,             // nb_vertices
            5,             // nb_facets
            {4,3,3,3,3},   // nb_vertices in facet
            {              // facets
                {0,1,2,3},
                {0,4,1},
                {0,3,4},
                {2,4,3},
                {2,1,4}
            },
            8,             // nb_edges
            {              // edges
                {0,1},{1,2},{2,3},{3,0},{0,4},{1,4},{2,4},{3,4}
            },
            {              // edges adjacent facets
                {1,0},{4,0},{3,0},{2,0},{2,1},{1,4},{4,3},{3,2}
            }
        };

        GEOGRAM_API CellDescriptor connector_descriptor = {
            4,             // nb_vertices
            3,             // nb_facets
            {4,3,3},       // nb_vertices in facet
            {              // facets
                {0,1,2,3},
                {2,1,0},
                {3,2,0}
            },
            5,             // nb_edges
            {              // edges
                {0,1},{1,2},{2,3},{3,0},{0,2}
            },
            {              // edges adjacent facets
                {1,0},{1,0},{2,0},{2,0},{2,1}
            }         
        };

        GEOGRAM_API CellDescriptor* cell_type_to_cell_descriptor[5] = { 
            &tet_descriptor, 
            &hex_descriptor, 
            &prism_descriptor, 
            &pyramid_descriptor, 
            &connector_descriptor
        };

    }
    
    /********************************************************************/
    
    MeshCellsStore::MeshCellsStore(Mesh& mesh) :
        MeshSubElementsStore(mesh),
        is_simplicial_(true) {
        cell_ptr_.push_back(0);
    }

    void MeshCellsStore::clear_store(
        bool keep_attributes, bool keep_memory 
    ) {
        if(keep_memory) {
            cell_ptr_.resize(0);
            cell_type_.resize(0);
        } else {
            cell_ptr_.clear();
            cell_type_.clear();
        }
        cell_ptr_.push_back(0);        
        MeshSubElementsStore::clear_store(keep_attributes, keep_memory);
    }

    void MeshCellsStore::resize_store(index_t new_size) {
        if(!is_simplicial_) {
            cell_ptr_.resize(new_size+1);
            cell_type_.resize(new_size);
        }
        MeshSubElementsStore::resize_store(new_size);
    }
    
    
    /**************************************************************************/

    MeshCellCornersStore::MeshCellCornersStore(Mesh& mesh) :
        MeshSubElementsStore(mesh),
        vertices_(mesh.vertices) {
    }

    void MeshCellCornersStore::clear_store(
        bool keep_attributes, bool keep_memory
    ) {
        if(keep_memory) {
            corner_vertex_.resize(0);
        } else {
            corner_vertex_.clear();
        }
        MeshSubElementsStore::clear_store(keep_attributes, keep_memory);
    }
    
    void MeshCellCornersStore::resize_store(index_t new_size) {
        corner_vertex_.resize(new_size);
        MeshSubElementsStore::resize_store(new_size);
    }

    /**************************************************************************/

    MeshCellFacetsStore::MeshCellFacetsStore(Mesh& mesh) :
        MeshSubElementsStore(mesh),
        vertices_(mesh.vertices),
        cells_(mesh.cells) {
    }

    void MeshCellFacetsStore::clear_store(
        bool keep_attributes, bool keep_memory
    ) {
        if(keep_memory) {
            adjacent_cell_.resize(0);
        } else {
            adjacent_cell_.clear();
        }
        MeshSubElementsStore::clear_store(keep_attributes, keep_memory);
    }
    
    void MeshCellFacetsStore::resize_store(index_t new_size) {
        adjacent_cell_.resize(new_size);
        MeshSubElementsStore::resize_store(new_size);
    }

    
    /**************************************************************************/

    MeshCells::MeshCells(Mesh& mesh) :
        MeshCellsStore(mesh),
        vertices_(mesh.vertices),
        cell_corners_(mesh.cell_corners),
        cell_facets_(mesh.cell_facets) {
    }

    void MeshCells::clear(bool keep_attributes, bool keep_memory) {
        cell_corners_.clear_store(keep_attributes, keep_memory);
        cell_facets_.clear_store(keep_attributes, keep_memory);        
        clear_store(keep_attributes, keep_memory);
        is_simplicial_ = true;        
    }

    
    void MeshCells::delete_elements(
        vector<index_t>& to_delete,
        bool remove_isolated_vertices
    ) {
        // "Fast track" if no element should be deleted
        if(!has_non_zero(to_delete)) {
            if(remove_isolated_vertices) {
                mesh_.vertices.remove_isolated();
            }
            return;
        }

        // to_delete is used for both indicating
        // which facets should be deleted and
        // for storing the re-numbering map
        vector<index_t>& cells_old2new = to_delete;

        vector<index_t>& corner_vertex = cell_corners_.corner_vertex_;
        vector<index_t>& adjacent_cell = cell_facets_.adjacent_cell_;

        index_t new_nb_cells = 0;
        index_t new_nb_corner_facets = 0;

        // If there are some corners or facets
        // attributes, we need to compute the index
        // mapping for them.
        vector<index_t> corner_facets_old2new;
        if(
            cell_corners_.attributes().nb() != 0 ||
            cell_facets_.attributes().nb() != 0) {
            corner_facets_old2new.resize(cell_corners_.nb(), index_t(-1));
        }

        for(index_t c=0; c<nb(); ++c) {
            if(cells_old2new[c] != 0) {
                cells_old2new[c] = NO_CELL;
            } else {
                cells_old2new[c] = new_nb_cells;

                if(!is_simplicial_) {
                    cell_ptr_[new_nb_cells] = new_nb_corner_facets;
                    cell_type_[new_nb_cells] = cell_type_[c];
                }

                index_t b,e;
                if(is_simplicial_) {
                    b = 4*c;
                    e = b+4;
                } else {
                    b = cell_ptr_[c];
                    e = cell_ptr_[c+1];
                }

                for(index_t cof=b; cof<e; ++cof) {
                    if(corner_facets_old2new.size() != 0) {
                        corner_facets_old2new[cof] = new_nb_corner_facets;
                    }
                    if(cof != new_nb_corner_facets) {
                        corner_vertex[new_nb_corner_facets] =
                            corner_vertex[cof];
                        adjacent_cell[new_nb_corner_facets] =
                            adjacent_cell[cof];
                    }
                    ++new_nb_corner_facets;
                }
                ++new_nb_cells;
            }
        }

        if(!is_simplicial_) {
            cell_ptr_[new_nb_cells] = new_nb_corner_facets;
        }

        // Map adjacent cell indices
        for(index_t f=0; f<cell_facets_.nb(); ++f) {
            index_t c = adjacent_cell[f];
            if(c != NO_CELL) {
                adjacent_cell[f] = cells_old2new[c];
            }
        }
        
        // Manage cell store and attributes
        attributes().compress(cells_old2new);
        resize_store(new_nb_cells);
        
        // Manage corners/facets store and attributes
        if(corner_facets_old2new.size() != 0) {
            // Corner/facet index mapping is only computed
            // if there ware some corner or facet attributes
            cell_corners_.attributes().compress(corner_facets_old2new);
            cell_facets_.attributes().compress(corner_facets_old2new);
        }
        cell_corners_.resize_store(new_nb_corner_facets);
        cell_facets_.resize_store(new_nb_corner_facets);
        
        if(remove_isolated_vertices) {
            mesh_.vertices.remove_isolated();
        }
    }
        
    void MeshCells::permute_elements(vector<index_t>& permutation) {
        attributes_.apply_permutation(permutation);

        if(
            cell_corners_.attributes().nb() != 0 ||
            cell_facets_.attributes().nb() != 0
        ) {
            vector<index_t> cell_corner_facets_permutation;
            cell_corner_facets_permutation.reserve(cell_corners_.nb());

            for(index_t new_cell = 0; new_cell<nb(); ++new_cell) {
                index_t old_cell = permutation[new_cell];
                index_t cell_size =
                    std::max(nb_vertices(old_cell), nb_facets(old_cell));
                for(index_t i=0; i<cell_size; ++i) {
		    cell_corner_facets_permutation.push_back(
			corners_begin(old_cell)+i
		    );
                }
            }
            
            if(cell_corners_.attributes().nb() != 0) {
                cell_corners_.attributes().apply_permutation(
                    cell_corner_facets_permutation
                );
            }
            if(cell_facets_.attributes().nb() != 0) {
                cell_facets_.attributes().apply_permutation(
                    cell_corner_facets_permutation
                );
            }
        }

        vector<index_t>& corner_vertex = cell_corners_.corner_vertex_;
        vector<index_t>& facet_adjacent_cell = cell_facets_.adjacent_cell_;
        
        if(is_simplicial_) {
            // in-place permutation !
            
            Permutation::apply(
                corner_vertex.data(),
                permutation,
                index_t(sizeof(index_t) * 4)
            );

            Permutation::apply(
                facet_adjacent_cell.data(),
                permutation,
                index_t(sizeof(index_t) * 4)
            );

            Permutation::invert(permutation);

            for(index_t f = 0; f < facet_adjacent_cell.size(); ++f) {
                if(facet_adjacent_cell[f] != NO_CELL) {
                    facet_adjacent_cell[f] =
                        permutation[facet_adjacent_cell[f]];
                }
            }
        } else {
            // we need to do some copies
            
            vector<index_t> new_cell_ptr(nb()+1);
            vector<index_t> new_corner_vertex(cell_corners_.nb());
            vector<index_t> new_facet_adjacent_cell(cell_facets_.nb());
            
            index_t new_ptr = 0;
            for(index_t new_c=0; new_c<nb(); ++new_c) {
                index_t old_c = permutation[new_c];
                index_t old_ptr = cell_ptr_[old_c];
                index_t cell_size = std::max(
                    nb_vertices(old_c), nb_facets(old_c)
                );
                new_cell_ptr[new_c] = new_ptr;
                for(index_t i=0; i<cell_size; ++i) {
                    new_corner_vertex[new_ptr+i] = corner_vertex[old_ptr+i];
                    new_facet_adjacent_cell[new_ptr+i] =
                        facet_adjacent_cell[old_ptr+i];
                }
                new_ptr += cell_size;
            }
            new_cell_ptr[nb()] = new_ptr;

            Permutation::apply(
                cell_type_.data(), permutation, index_t(sizeof(Numeric::uint8))
            );

            Permutation::invert(permutation);

            for(index_t f = 0; f < new_facet_adjacent_cell.size(); ++f) {
                if(new_facet_adjacent_cell[f] != NO_CELL) {
                    new_facet_adjacent_cell[f] =
                        permutation[new_facet_adjacent_cell[f]];
                }
            }

            corner_vertex.swap(new_corner_vertex);
            facet_adjacent_cell.swap(new_facet_adjacent_cell);
            cell_ptr_.swap(new_cell_ptr);
        }
    }

    
    
    void MeshCells::connect_tets() {
        geo_assert(is_simplicial_);
        if(nb() == 0) {
            return;
        }
        cell_facets_.resize_store(nb() * 4);
        for(index_t f=0; f<cell_facets_.nb(); ++f) {
            cell_facets_.set_adjacent_cell(f,NO_CELL);
        }

        GEO::vector<index_t> next_tet_corner_around_vertex(
            nb() * 4, NO_CORNER
        );
        GEO::vector<index_t> v2c(vertices_.nb(), NO_CORNER);
        
        // Step 1: chain tet corners around vertices and compute v2c
        for(index_t t = 0; t < nb(); ++t) {
            for(index_t lv = 0; lv < 4; ++lv) {
                index_t v = vertex(t, lv);
                next_tet_corner_around_vertex[4 * t + lv] = v2c[v];
                v2c[v] = 4 * t + lv;
            }
        }
        
        // Step 2: connect tets
        for(index_t t1 = 0; t1 < nb(); ++t1) {
            for(index_t lf1 = 0; lf1 < 4; ++lf1) {
                if(adjacent(t1, lf1) == NO_CELL) {
                    index_t v1 = facet_vertex(t1, lf1, 0);
                    index_t v2 = facet_vertex(t1, lf1, 1);
                    index_t v3 = facet_vertex(t1, lf1, 2);
                    for(
                        index_t c2 = v2c[v1]; c2 != NO_CORNER;
                        c2 = next_tet_corner_around_vertex[c2]
                    ) {
                        index_t t2 = c2/4;
                        index_t lf2 = find_tet_facet(t2, v3, v2, v1);
                        if(lf2 != NO_FACET) {
                            set_adjacent(t1, lf1, t2);
                            set_adjacent(t2, lf2, t1);
                            break;
                        }
                    }
                }
            }
        }
    }

    bool MeshCells::facets_match(
        index_t c1, index_t f1, index_t c2, index_t f2
    ) const {
        index_t nbv = facet_nb_vertices(c1,f1);
        if(facet_nb_vertices(c2,f2) != nbv) {
            return false;
        }
        for(index_t offset=0; offset<nbv; ++offset) {
            bool match=true;
            for(index_t v1=0; v1<nbv; ++v1) {
                index_t v2 = (nbv-v1+offset)%nbv;
                if(
                    facet_vertex(c1,f1,v1) !=
                    facet_vertex(c2,f2,v2)
                ) {
                    match=false;
                    break;
                }
            }
            if(match) {
                return true;
            }
        }
        return false;
    }

    /**
     * \brief Tests whether two indices triplets match
     *  up to a circular permutation.
     * \param[in] v1 index of the first vertex of the first triangle
     * \param[in] v2 index of the second vertex of the first triangle
     * \param[in] v3 index of the third vertex of the first triangle
     * \param[in] w1 index of the first vertex of the second triangle
     * \param[in] w2 index of the second vertex of the second triangle
     * \param[in] w3 index of the third vertex of the second triangle
     * \retval true if (\p v1, \p v2, \p v3) = (\p w1, \p w2, \p w3)
     *  up to a circular permutation
     * \retval false otherwise
     */
    inline bool triangles_equal(
        index_t v1, index_t v2, index_t v3,
        index_t w1, index_t w2, index_t w3
    ) {
        return (
            (v1 == w1 && v2 == w2 && v3 == w3) ||
            (v1 == w2 && v2 == w3 && v3 == w1) ||
            (v1 == w3 && v2 == w1 && v3 == w2) 
        );
    }

    bool MeshCells::triangular_facet_matches_quad_facet(
        index_t c1, index_t lf1,
        index_t c2, index_t lf2
    ) const {
        geo_debug_assert(facet_nb_vertices(c1,lf1) == 3);
        geo_debug_assert(facet_nb_vertices(c2,lf2) == 4);
        
        index_t v1 = facet_vertex(c1,lf1,0);
        index_t v2 = facet_vertex(c1,lf1,1);
        index_t v3 = facet_vertex(c1,lf1,2);
        index_t w1 = facet_vertex(c2,lf2,0);
        index_t w2 = facet_vertex(c2,lf2,1);
        index_t w3 = facet_vertex(c2,lf2,2);
        index_t w4 = facet_vertex(c2,lf2,3);

        // Note: subtriangles in (w1,w2,w3,w4) are
        // in reverse order since two facets can be
        // connected only if they have opposite
        // orientations.
        return (
            triangles_equal(v1,v2,v3,w4,w3,w2) ||
            triangles_equal(v1,v2,v3,w3,w2,w1) ||
            triangles_equal(v1,v2,v3,w2,w1,w4) ||            
            triangles_equal(v1,v2,v3,w1,w4,w3)
        ) ;
    }

    bool MeshCells::triangular_facets_have_common_edge(
        index_t c1, index_t f1,
        index_t c2, index_t f2,
        index_t& e1, index_t& e2
    ) const {
        geo_debug_assert(facet_nb_vertices(c1,f1) == 3);
        geo_debug_assert(facet_nb_vertices(c2,f2) == 3);
        for(e1=0; e1<3; ++e1) {
            for(e2=0; e2<3; ++e2) {
                if(
                    facet_vertex(c1, f1, (e1+1)%3) ==
                    facet_vertex(c2, f2, (e2+2)%3)  &&
                    facet_vertex(c1, f1, (e1+2)%3) ==
                    facet_vertex(c2, f2, (e2+1)%3) 
                ) {
                    return true;
                }
            }
        }
        e1 = NO_EDGE;
        e2 = NO_EDGE;
        return false;
    }

    bool MeshCells::create_connector(
        index_t c1, index_t lf1,
        const std::vector< std::pair<index_t, index_t> >& matches
    ) {
        if(matches.size() == 0) {
            return false;
        }
        
        if(matches.size() == 1) {
            GEO::Logger::warn("Mesh")
                << "Found only one triangular facet adjacent to a quad facet"
                << std::endl;
            Attribute<bool> weird(attributes(),"weird");
            weird[c1] = true;
            for(index_t i=0; i<matches.size(); ++i) {
                weird[matches[i].first] = true;
            }
            return false;
        }

        // Find among the matches two facets that have an edge in common
        // Yes, there can be more than two candidate facets with three
        //  vertices in common with the quad facet ! But among them,
        //  only two of them have an edge in common.
        index_t adj_c1 = NO_CELL;
        index_t adj_lf1 = NO_FACET;
        index_t adj_c2 = NO_CELL;
        index_t adj_lf2 = NO_FACET;
        index_t e1 = NO_EDGE;
        index_t e2 = NO_EDGE;

        index_t nb_found=0;
        for(index_t i=0; i<index_t(matches.size()); ++i) {
            for(index_t j=i+1; j<index_t(matches.size()); ++j) {
                index_t cur_e1 = NO_EDGE;
                index_t cur_e2 = NO_EDGE;
                if(triangular_facets_have_common_edge(
                       matches[i].first, matches[i].second,
                       matches[j].first, matches[j].second,
                       cur_e1, cur_e2
                )) {
                    adj_c1 = matches[i].first;
                    adj_lf1 = matches[i].second;
                    adj_c2 = matches[j].first;
                    adj_lf2 = matches[j].second;
                    e1 = cur_e1;
                    e2 = cur_e2;
                    ++nb_found;
                }
            }
        }

        // Sanity check: make sure that we only found a single pair
        // of triangular facets with a common edge that matches the quad.
        if(nb_found > 2) {
            GEO::Logger::warn("Mesh")
                << "Found more than two triangular facets adjacent to a quad"
                << " ( got " << nb_found << ")"
                << std::endl;
            Attribute<bool> weird(attributes(),"weird");
            weird[c1] = true;
            for(index_t i=0; i<matches.size(); ++i) {
                weird[matches[i].first] = true;
            }
            
            return false;
        }

        if(nb_found == 0) {
            GEO::Logger::warn("Mesh")
                << "Triangular facets adjacent to a quad have no common edge"
                << std::endl;
            return false;
        }

        // Sanity check: make sure the triangular facets
        // are on the border.
        if(
            adjacent(adj_c1, adj_lf1) != NO_CELL ||
            adjacent(adj_c2, adj_lf2) != NO_CELL
        ) {
	    /*
            GEO::Logger::warn("Mesh")
                << "Matching tet facets are not on border (\"thick sliver\")"
                << std::endl;
	    */
            return false;
        }

        // v1 and v2 are on the common edge
        index_t v1 = facet_vertex(
            adj_c1, adj_lf1, (e1+1)%3
        );
        
        index_t v2 = facet_vertex(
            adj_c1, adj_lf1, (e1+2)%3
        );
                    
        // w1 and w2 are the opposite vertices
        index_t w1 = facet_vertex(adj_c1, adj_lf1, e1);
        index_t w2 = facet_vertex(adj_c2, adj_lf2, e2);
        
        // Create the connector
        index_t conn = create_connector(
            v1, w2, v2, w1,
            c1, adj_c1, adj_c2
        );

        // Connect the cells with the connector
        set_adjacent(c1, lf1, conn);
        set_adjacent(adj_c1, adj_lf1, conn);
        set_adjacent(adj_c2, adj_lf2, conn);

        return true;
    }
    
    void MeshCells::connect(bool remove_trivial_slivers, bool verbose_if_OK) {
        // "Fast track" for simplicial mesh
        if(is_simplicial_) {
            connect_tets();
            return;
        }

        for(index_t f=0; f<cell_facets_.nb(); ++f) {
            cell_facets_.set_adjacent_cell(f,NO_CELL);
        }

        vector<index_t> next_cell_around_vertex(
            cell_corners_.nb(), NO_CELL
        );
        vector<index_t> v2cell(vertices_.nb(), NO_CELL);

        // Step 1: chain cells around vertices and compute v2cell
        for(index_t c = 0; c < nb(); ++c) {
            for(index_t lv = 0; lv < nb_vertices(c); ++lv) {
                index_t v = vertex(c, lv);
                next_cell_around_vertex[corners_begin(c) + lv] =
                    v2cell[v];
                v2cell[v] = c;
            }
        }

        // Step 2: connect cells
        // (c1,lf1) traverse all the cell facets
        for(index_t c1 = 0; c1 < nb(); ++c1) {
            for(index_t lf1 = 0; lf1 < nb_facets(c1); ++lf1) {

                // If (c1,lf1) is on the border, try to connect it
                if(adjacent(c1, lf1) == NO_CELL) {

                    // v1 is one of the vertices of (c1,lf1)
                    index_t v1 = facet_vertex(c1,lf1,0);

                    // c2 traverses all the cells incident to v1
                    for(
                        index_t c2 = v2cell[v1]; c2 != NO_CELL;
                        c2 = next_cell_around_vertex[
                            corners_begin(c2) +
                            find_cell_vertex(c2,v1)
                        ]
                    ) {

                        // If we find a cell facet lf2 compatible with (c1,lf1)
                        // in c2, then connect (c1,lf1) to c2 and
                        // (c2,lf2) to c1.
                        index_t lf2 = find_cell_facet(c2, c1, lf1);
                        if(lf2 != NO_FACET) {
                            set_adjacent(c1, lf1, c2);
                            set_adjacent(c2, lf2, c1);
                            break;
                        }
                    }
                }
            }
        }

        // Step 3: Create connectors, i.e. artificial cells that represent
        // non-conformal connections between two triangular facets and
        // a quadrangular facet.

        // Backup nb_cells since we are creating new cells (connectors)
        // during this loop.
        index_t nb_cells0 = nb();

        // Keep track of the number of invalid configurations (does
        // not seem to happen anymore, but I keep the code just in case).
        index_t weird=0;

        // For each quadrangular face, we compute the list of candidate
        // triangular faces to be connected with it (a vector of
        // (cell index, facet index) pairs).
        std::vector< std::pair<index_t, index_t> > matches;

        // If remove_trivial_slivers is set, we also detect the trivial
        // slivers, i.e. the slivers that are glued on a quadrilateral facet.
        
        std::vector<index_t> trivial_slivers;
        
        // (c1,f1) traverse all quadrangular cell facets on the border
        for(index_t c1=0; c1 < nb_cells0; ++c1) {
            if(type(c1) == MESH_TET) {
                continue;
            }
            for(index_t lf1=0; lf1<nb_facets(c1); ++lf1) {
                if(
                    facet_nb_vertices(c1,lf1) != 4 ||
                    adjacent(c1,lf1) != NO_CELL
                ) {
                    continue;
                }

                // Now c2 traverses all the cells incident to one of
                // the vertex of (c1,lf1)
                matches.resize(0);
                for(index_t lv1=0; lv1<facet_nb_vertices(c1,lf1); ++lv1) {
                    index_t v1 = facet_vertex(c1,lf1,lv1);
                    for(
                        index_t c2 = v2cell[v1]; c2 != NO_CELL;
                        c2 = next_cell_around_vertex[
                                corners_begin(c2) +
                                find_cell_vertex(c2,v1)
                        ]
                    ) {
                        geo_debug_assert(find_cell_vertex(c2,v1) != NO_VERTEX);
                        if(c2 == c1 || type(c2) == MESH_HEX) {
                            continue;
                        }

                        // Among all the triangular facets of c2, find the ones
                        // that can be connected to (c1,lf1)
                        for(index_t lf2=0; lf2<nb_facets(c2); ++lf2) {
                            if(facet_nb_vertices(c2,lf2) != 3) {
                                continue;
                            }
                            if(triangular_facet_matches_quad_facet(
                                   c2,lf2,c1,lf1
                            )) {
                                matches.push_back(std::make_pair(c2,lf2));
                            } 
                        }
                    }
                }

                // Make sure we get each match once only
                GEO::sort_unique(matches);

                // This should not happen, but we keep this
                // sanity check and notify the user if some
                // connectors could not be created.
                if(
                    matches.size() != 0 &&
                    !create_connector(c1,lf1,matches)
                ) {
                    ++weird;
                }

                if(remove_trivial_slivers) {
                    for(index_t i=0; i<matches.size(); ++i) {
                        if(type(matches[i].first) != MESH_TET) {
                            continue;
                        }
                        for(index_t j=i+1; j<matches.size(); ++j) {
                            if(matches[j].first == matches[i].first) {
                                trivial_slivers.push_back(matches[i].first);
                            }
                        }
                    }
                }
            }
        }
        if(weird != 0) {
            GEO::Logger::warn("Mesh") << "Encountered "
                                      << weird
                                      << " invalid connector configurations"
                                      << std::endl;
        } else {
	    if(verbose_if_OK) {
		GEO::Logger::out("Mesh") << "All connectors are OK"
					 << std::endl;
	    }
	}
        if(remove_trivial_slivers && trivial_slivers.size() != 0) {
            GEO::Logger::warn("Mesh") << "Removing "
                                      << trivial_slivers.size()
                                      << " trivial sliver(s)" << std::endl;

            next_cell_around_vertex.clear();
            v2cell.clear();
            
            vector<index_t> delete_c(nb(),0);
            for(index_t i=0; i<trivial_slivers.size(); ++i) {
                delete_c[trivial_slivers[i]] = 1;
            }
	    // We need to remove the previously generated connectors,
	    // some of them may be wrong if adjacent to a sliver that
	    // was removed.
	    for(index_t c=0; c<nb(); ++c) {
		if(type(c) == MESH_CONNECTOR) {
		    delete_c[c] = 1;
		}
	    }
            delete_elements(delete_c);

            GEO::Logger::warn("Mesh") << "Re-trying to connect cells" << std::endl;
            connect(false,true);
        }
    }

    void MeshCells::compute_borders() {
	Attribute<index_t> facet_cell;
	compute_borders(facet_cell);
    }
    
    void MeshCells::compute_borders(Attribute<index_t>& facet_cell) {
        mesh_.facets.clear(true,false);
        if(is_simplicial_) {
            for(index_t t=0; t<nb(); ++t) {
                for(index_t f=0; f<4; ++f) {
                    if(adjacent(t,f) == NO_CELL) {
                        index_t new_f = mesh_.facets.create_triangle(
                            tet_facet_vertex(t,f,0),
                            tet_facet_vertex(t,f,1),
                            tet_facet_vertex(t,f,2)
                        );
			if(facet_cell.is_bound()) {
			    facet_cell[new_f] = t;
			}
                    }
                }
            }
        } else {
            for(index_t c=0; c<nb(); ++c) {
                for(index_t f=0; f<nb_facets(c); ++f) {
                    if(adjacent(c,f) == NO_CELL) {
			index_t new_f = index_t(-1);
                        switch(facet_nb_vertices(c,f)) {
                        case 3:
                            new_f = mesh_.facets.create_triangle(
                                facet_vertex(c,f,0),
                                facet_vertex(c,f,1),
                                facet_vertex(c,f,2)
                            );
                            break;
                        case 4:
                            new_f = mesh_.facets.create_quad(
                                facet_vertex(c,f,0),
                                facet_vertex(c,f,1),
                                facet_vertex(c,f,2),
                                facet_vertex(c,f,3)
                            );
                            break;
                        default:
                            geo_assert_not_reached;
                        }
			if(facet_cell.is_bound()) {
			    facet_cell[new_f] = c;
			}
                    }
                }
            }
        }
        mesh_.facets.connect();
    }

    void MeshCells::assign_tet_mesh(
        coord_index_t dim,
        vector<double>& vertices,
        vector<index_t>& tets,
        bool steal_args
    ) {
        vertices_.assign_points(vertices, dim, steal_args);
        assign_tet_mesh(tets, steal_args);
    }

    void MeshCells::assign_tet_mesh(
        vector<index_t>& tets,
        bool steal_args
    ) {
        index_t nb_tets = tets.size()/4;
        is_simplicial_ = true;
        cell_ptr_.clear();
        cell_type_.clear();
        if(steal_args) {
            cell_corners_.corner_vertex_.swap(tets);
        } else {
            cell_corners_.corner_vertex_ = tets;
        }
        resize_store(nb_tets);
        cell_corners_.resize_store(nb_tets*4);
        cell_facets_.resize_store(nb_tets*4);
        cell_facets_.adjacent_cell_.assign(
            nb_tets*4, NO_CELL
        );
        attributes().zero();
        cell_corners_.attributes().zero();
        cell_facets_.attributes().zero();
    }

    void MeshCells::pop() {
        geo_debug_assert(nb() != 0);
        index_t corners_facets_new_size = cell_ptr_[nb()-1];
        cell_corners_.resize_store(corners_facets_new_size);
        cell_facets_.resize_store(corners_facets_new_size);
        resize_store(nb()-1);
    }
    
    /**************************************************************************/
    
    Mesh::Mesh(index_t dimension, bool single_precision)
        : vertices(*this),
          edges(*this),
          facets(*this),
          facet_corners(*this),
          cells(*this),
          cell_corners(*this),
          cell_facets(*this)
    {
        vertices.bind_point_attribute(dimension, single_precision);
    }

    Mesh::~Mesh() {
    }
    
    void Mesh::clear(bool keep_attributes, bool keep_memory) {
        vertices.clear(keep_attributes, keep_memory);
        edges.clear(keep_attributes, keep_memory);
        facets.clear(keep_attributes, keep_memory);
        cells.clear(keep_attributes, keep_memory);
    }

    void Mesh::copy(
        const Mesh& rhs,
        bool copy_attributes,
        MeshElementsFlags what
    ) {
        if((what & MESH_VERTICES) == 0) {
            clear(false,false);
            return;
        }
        vertices.copy(rhs.vertices, copy_attributes);
        if((what & MESH_EDGES) != 0) {
            edges.copy(rhs.edges, copy_attributes);
        } else {
            edges.clear(false,false);
        }
        if((what & MESH_FACETS) != 0) {
            facets.copy(rhs.facets, copy_attributes);
            facet_corners.copy(rhs.facet_corners, copy_attributes);
        } else {
            facets.clear(false,false);
        }
        if((what & MESH_CELLS) != 0) {
            cells.copy(rhs.cells, copy_attributes);
            cell_corners.copy(rhs.cell_corners, copy_attributes);
            cell_facets.copy(rhs.cell_facets, copy_attributes);
        } else {
            cells.clear(false,false);
        }
    }
    
    void Mesh::show_stats(const std::string& tag) const {
        index_t nb_borders = 0;
        for(index_t co = 0; co < facet_corners.nb(); ++co) {
            if(facet_corners.adjacent_facet(co) == NO_FACET) {
                nb_borders++;
            }
        }
        
        Logger::out(tag)
            << (vertices.single_precision() ? "(FP32)" : "(FP64)") 
            << " nb_v:" << vertices.nb()
            << " nb_e:" << edges.nb()
            << " nb_f:" << facets.nb()
            << " nb_b:" << nb_borders
            << " tri:" << facets.are_simplices()
            << " dim:" << vertices.dimension()
            << std::endl;

        if(cells.nb() != 0) {
            if(cells.are_simplices()) {
                Logger::out(tag) << " nb_tets:"
                                      << cells.nb() << std::endl;
            } else {
                
                index_t nb_cells_by_type[GEO::MESH_NB_CELL_TYPES];
                for(index_t i=0; i<GEO::MESH_NB_CELL_TYPES; ++i) {
                    nb_cells_by_type[i] = 0;
                }
                
                for(index_t c=0; c<cells.nb(); ++c) {
                    geo_debug_assert(cells.type(c) < GEO::MESH_NB_CELL_TYPES);
                    ++nb_cells_by_type[cells.type(c)];
                }
                
                Logger::out(tag) << " Hybrid - nb_cells:"
                                 << cells.nb() << " "
                                 << " Tet:" << nb_cells_by_type[0]
                                 << " Hex:" << nb_cells_by_type[1]
                                 << " Psm:" << nb_cells_by_type[2]
                                 << " Pmd:" << nb_cells_by_type[3]
                                 << " Cnx:" << nb_cells_by_type[4]
                                 << std::endl;
            }
        }

        display_attributes(tag, "vertices", vertices);
        display_attributes(tag, "edges", edges);        
        display_attributes(tag, "facets", facets);
        display_attributes(tag, "facet_corners", facet_corners);
        display_attributes(tag, "cells", cells);
        display_attributes(tag, "cell_corners", cell_corners);
        display_attributes(tag, "cell_facets", cell_facets);
    }

    void Mesh::assert_is_valid() {
        for(index_t f=0; f<facets.nb(); ++f) {
            for(
                index_t c=facets.corners_begin(f);
                c<facets.corners_end(f); ++c
            ) {
                geo_assert(facet_corners.vertex(c) < vertices.nb());
                index_t f2 = facet_corners.adjacent_facet(c);
                geo_assert(f2 == NO_FACET || f2 < facets.nb());
            }
        }

        for(index_t c=0; c<cells.nb(); ++c) {
            for(index_t lv=0; lv<cells.nb_vertices(c); ++lv) {
                geo_assert(cells.vertex(c,lv) < vertices.nb());
            }
            for(index_t lf=0; lf<cells.nb_facets(c); ++lf) {
                index_t c2 = cells.adjacent(c,lf);
                geo_assert(c2 == NO_CELL || c2 < cells.nb());
            }
        }
    }

    void Mesh::display_attributes(
        const std::string& tag, const std::string& subelement_name,
        const MeshSubElementsStore& subelements 
    ) const {
        if(subelements.attributes().nb() != 0) {
            vector<std::string> names;
            subelements.attributes().list_attribute_names(names);
            std::string names_str;
            for(index_t i=0; i<names.size(); ++i) {
                if(i != 0) {
                    names_str = names_str + ",";
                } 
                names_str = names_str + names[i];
                AttributeStore* store =
                    subelements.attributes().find_attribute_store(names[i]);
                index_t dim = store->dimension();
                if(dim != 1) {
                    names_str += ("[" + String::to_string(dim) + "]");
                }
            }
            Logger::out(tag) << "Attributes on " << subelement_name
                             << ": " << names_str << std::endl;
        }
    }

    index_t Mesh::nb_subelements_types() const {
        return 7;
    }

    MeshSubElementsStore& Mesh::get_subelements_by_index(
        index_t i
    ) {
        switch(i) {
        case 0:
            return vertices;
        case 1:
            return edges;
        case 2:
            return facets;
        case 3:
            return facet_corners;
        case 4:
            return cells;
        case 5:
            return cell_corners;
        case 6:
            return cell_facets;
        default:
            geo_assert_not_reached;
        }
    }

    const MeshSubElementsStore& Mesh::get_subelements_by_index(
        index_t i
    ) const {
        switch(i) {
        case 0:
            return vertices;
        case 1:
            return edges;
        case 2:
            return facets;
        case 3:
            return facet_corners;
        case 4:
            return cells;
        case 5:
            return cell_corners;
        case 6:
            return cell_facets;
        default:
            geo_assert_not_reached;
        }
    }
    
    MeshSubElementsStore& Mesh::get_subelements_by_type(
        MeshElementsFlags what
    ) {
        switch(what) {
        case MESH_VERTICES:
            return vertices;
        case MESH_EDGES:
            return edges;
        case MESH_FACETS:
            return facets;
        case MESH_FACET_CORNERS:
            return facet_corners;
        case MESH_CELLS:
            return cells;
        case MESH_CELL_CORNERS:
            return cell_corners;
        case MESH_CELL_FACETS:
            return cell_facets;
        case MESH_NONE:
        case MESH_ALL_ELEMENTS:
        case MESH_ALL_SUBELEMENTS:
            geo_assert_not_reached;
        }
        return *(MeshSubElementsStore*)nullptr;
    }
    
    const MeshSubElementsStore& Mesh::get_subelements_by_type(
        MeshElementsFlags what
    ) const {
        switch(what) {
        case MESH_VERTICES:
            return vertices;
        case MESH_EDGES:
            return edges;
        case MESH_FACETS:
            return facets;
        case MESH_FACET_CORNERS:
            return facet_corners;
        case MESH_CELLS:
            return cells;
        case MESH_CELL_CORNERS:
            return cell_corners;
        case MESH_CELL_FACETS:
            return cell_facets;
        case MESH_NONE:
        case MESH_ALL_ELEMENTS:
        case MESH_ALL_SUBELEMENTS:
            geo_assert_not_reached;
        }
        return *(MeshSubElementsStore*)nullptr;
    }
    
    std::string Mesh::subelements_type_to_name(MeshElementsFlags what) {
        std::string result;
        switch(what) {
        case MESH_VERTICES:
            result =  "vertices";
            break;
        case MESH_EDGES:
            result = "edges";
            break;            
        case MESH_FACETS:
            result = "facets";
            break;            
        case MESH_FACET_CORNERS:
            result = "facet_corners";
            break;            
        case MESH_CELLS:
            result = "cells";
            break;            
        case MESH_CELL_CORNERS:
            result = "cell_corners";
            break;            
        case MESH_CELL_FACETS:
            result = "cell_facets";
            break;            
        case MESH_NONE:
        case MESH_ALL_ELEMENTS:
        case MESH_ALL_SUBELEMENTS:
            geo_assert_not_reached;
        }
        return result;
    }
    
    MeshElementsFlags Mesh::name_to_subelements_type(const std::string& name) {
        if(name == "vertices") {
            return MESH_VERTICES;
        } else if(name == "edges") {
            return MESH_EDGES;
        } else if(name == "facets") {
            return MESH_FACETS;
        } else if(name == "facet_corners") {
            return MESH_FACET_CORNERS;
        } else if(name == "cells") {
            return MESH_CELLS;
        } else if(name == "cell_corners") {
            return MESH_CELL_CORNERS;
        } else if(name == "cell_facets") {
            return MESH_CELL_FACETS;
        }
        return MESH_NONE;
    }
    
    /**************************************************************************/
}

namespace {

    using namespace GEO;
    
    /**
     * \brief Gets the names of all scalar attributes from an AttributeManager
     * \param[in] attributes a const reference to the attribute manager
     * \param[in] prefix a const rerefenre to a string to be prepended to
     *  all attribute names
     * \return a ';'-separated list of all the scalar attributes
     */
    std::string get_scalar_attributes_impl(
        const AttributesManager& attributes,
        const std::string& prefix
    ) {
        std::string result;
        vector<std::string> attribute_names;
        attributes.list_attribute_names(attribute_names);

        for(index_t i=0; i<attribute_names.size(); ++i) {
            const AttributeStore* store = attributes.
                find_attribute_store(attribute_names[i]);
            if(ReadOnlyScalarAttributeAdapter::can_be_bound_to(store)) {
                index_t dim =
                    ReadOnlyScalarAttributeAdapter::nb_scalar_elements_per_item(
                        store
                    );
                if(dim == 1) {
                    if(result != "") {
                        result += ";";
                    }
                    result += prefix + "." + attribute_names[i];
                } else {
                    for(index_t j=0; j<dim; ++j) {
                        if(result != "") {
                            result += ";";
                        }
                        result +=
                            prefix + "." + attribute_names[i] +
                            "[" + String::to_string(j) + "]";
                    }
                }
            }
        }
        return result;
    }

    /**
     * \brief Gets the names of all attributes from an AttributeManager
     * \param[in] attributes a const reference to the attribute manager
     * \param[in] prefix a const rerefenre to a string to be prepended to
     *  all attribute names
     * \return a ';'-separated list of all the attributes
     */
    std::string get_attributes_impl(
        const AttributesManager& attributes,
        const std::string& prefix
    ) {
        std::string result;
        vector<std::string> attribute_names;
        attributes.list_attribute_names(attribute_names);

        for(index_t i=0; i<attribute_names.size(); ++i) {
	    if(result != "") {
		result += ";";
	    }
	    result += prefix + "." + attribute_names[i];
        }
        return result;
    }

    /**
     * \brief Gets the names of all vector attributes from an AttributeManager
     * \param[in] attributes a const reference to the attribute manager
     * \param[in] prefix a const rerefenre to a string to be prepended to
     *  all attribute names
     * \param[in] max_dim if non-zero, only return vector attributes with 
     *  dimension lower than max_dim
     * \return a ';'-separated list of all the vector attributes
     */
    std::string get_vector_attributes_impl(
        const AttributesManager& attributes,
        const std::string& prefix,
	index_t max_dim = 0
    ) {
        std::string result;
        vector<std::string> attribute_names;
        attributes.list_attribute_names(attribute_names);

        for(index_t i=0; i<attribute_names.size(); ++i) {
            const AttributeStore* store = attributes.
                find_attribute_store(attribute_names[i]);
	    if(store->dimension() >= 2 && (max_dim == 0 || store->dimension() <= max_dim)) {
		if(result != "") {
		    result += ";";
		}
		result += prefix + "." + attribute_names[i];
	    }
	    if(
		store->elements_type_matches(typeid(vec2).name()) &&
		(max_dim == 0 || 2 <= max_dim)
	    ) {
		if(result != "") {
		    result += ";";
		}
		result += prefix + "." + attribute_names[i];		
	    }
	    if(
		store->elements_type_matches(typeid(vec3).name()) &&
		(max_dim == 0 || 3 <= max_dim)
	    ) {
		if(result != "") {
		    result += ";";
		}
		result += prefix + "." + attribute_names[i];		
	    }
        }
        return result;
    }

    
    /**
     * \brief Appends a string to another one, with ';' delimiters.
     * \details If a is non-empty, a ';' delimiter is inserted.
     * \param[in,out] a a string
     * \param[in] b a string to be appended to a
     */
    static void strappend(std::string& a, const std::string& b) {
        if(b != "") {
            if(a != "") {
                a += ";";
            }
            a += b;
        }
    }
}

namespace GEO {

    std::string Mesh::get_attributes() const {
        std::string result;
        strappend(
            result,get_attributes_impl(vertices.attributes(),"vertices")
        );
        strappend(
            result,get_attributes_impl(edges.attributes(),"edges")
        );
        strappend(
            result,get_attributes_impl(facets.attributes(),"facets")
        );
        strappend(
	    result,get_attributes_impl(
		facet_corners.attributes(),"facet_corners"
	    )
        );
        strappend(
            result,get_attributes_impl(cells.attributes(),"cells")
        );
        strappend(
            result,get_attributes_impl(
                cell_corners.attributes(),"cell_corners"
            )
        );
        strappend(result,get_attributes_impl(
            cell_facets.attributes(),"cell_facets")
        );        
        return result;
    }
    
    std::string Mesh::get_scalar_attributes() const {
        std::string result;
        strappend(
            result,get_scalar_attributes_impl(vertices.attributes(),"vertices")
        );
        strappend(
            result,get_scalar_attributes_impl(edges.attributes(),"edges")
        );
        strappend(
            result,get_scalar_attributes_impl(facets.attributes(),"facets")
        );
        strappend(result,get_scalar_attributes_impl(
                      facet_corners.attributes(),"facet_corners"
                  )
        );
        strappend(
            result,get_scalar_attributes_impl(cells.attributes(),"cells")
        );
        strappend(
            result,get_scalar_attributes_impl(
                cell_corners.attributes(),"cell_corners"
            )
        );
        strappend(result,get_scalar_attributes_impl(
            cell_facets.attributes(),"cell_facets")
        );        
        return result;
    }


    std::string Mesh::get_vector_attributes(index_t max_dim) const {
        std::string result;
        strappend(
            result,get_vector_attributes_impl(vertices.attributes(),"vertices",max_dim)
        );
        strappend(
            result,get_vector_attributes_impl(edges.attributes(),"edges",max_dim)
        );
        strappend(
            result,get_vector_attributes_impl(facets.attributes(),"facets",max_dim)
        );
        strappend(result,get_vector_attributes_impl(
                      facet_corners.attributes(),"facet_corners",max_dim
                  )
        );
        strappend(
            result,get_vector_attributes_impl(cells.attributes(),"cells",max_dim)
        );
        strappend(
            result,get_vector_attributes_impl(
                cell_corners.attributes(),"cell_corners",max_dim
            )
        );
        strappend(result,get_vector_attributes_impl(
	      cell_facets.attributes(),"cell_facets",max_dim)
        );        
        return result;
    }
    
}

