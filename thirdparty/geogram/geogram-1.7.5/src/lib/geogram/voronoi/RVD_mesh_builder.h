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

#ifndef GEOGRAM_VORONOI_RVD_MESH_BUILDER
#define GEOGRAM_VORONOI_RVD_MESH_BUILDER

#include <geogram/basic/common.h>
#include <geogram/voronoi/generic_RVD.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/index.h>
#include <geogram/basic/attributes.h>
#include <geogram/basic/argused.h>

#include <vector>
#include <map>

/**
 * \file geogram/voronoi/RVD_mesh_builder.h
 * \brief Utilities to build meshes derived from restricted Voronoi diagrams.
 * \note This file contains functions and classes used by the internal 
 *  implementation of GEO::GenericVoronoiDiagram. 
 *  They are not meant to be used directly by client code.
 */

namespace GEO {

    /**
     * \brief RVDVertexMap maps symbolic vertices to unique ids.
     * \details Symbolic vertices are manipulated by
     *  GEOGen::RestrictedVoronoiDiagram. This class is
     *  used for instance in the implementation of
     *  RVDMeshBuilder.
     * \note This is an internal implementation class, not meant to
     *  be used directly, use GEO::RestrictedVoronoiDiagram instead.
     */
    class GEOGRAM_API RVDVertexMap {
    public:
        /**
         * \brief Constructs an empty map
         */
        RVDVertexMap();

        /**
         * \brief Maps the symbolic information of a vertex
         * into a unique identifier.
         * \param[in] center_vertex_id the index of the current Voronoi
         *  seed (provided by action classes in
         *  GEOGen::RestrictedVoronoiDiagram)
         * \param[in] sym the symbolic representation of the vertex
         *  (provided by action classes in
         *  GEOGen::RestrictedVoronoiDiagram)
         * \return a unique identifier for this vertex
         */
        index_t find_or_create_vertex(
            index_t center_vertex_id, const SymbolicVertex& sym
        );

        /**
         * \brief Defines the index of the first created vertex.
         * \param[in] i index of the first vertex that will be created
         *  (default is 0).
         */
        void set_first_vertex_index(index_t i) {
            nb_vertices_ = i;
        }

    protected:
        /**
         * \brief Allocates a new vertex.
         */
        index_t new_vertex() {
            index_t result = nb_vertices_;
            nb_vertices_++;
            return result;
        }

        /**
         * \brief Gets the number of bisectors represented
         *   in a symbolic vertex.
         */
        index_t nb_bisectors(const signed_trindex& sym) const {
            index_t result = 0;
            for(index_t i = 0; i < 3; i++) {
                if(sym.indices[i] >= 0) {
                    result++;
                }
            }
            return result;
        }

    private:
        // Maps (+++)-center vertex id quadruples to unique vertex id
        // +++ encodes a Voronoi vertex
        std::map<quadindex, index_t> ppp_to_id_;

        // Maps (++-)-center vertex id quadruples to unique vertex id
        // ++- encodes the intersection between a Voronoi edge (++) and
        //    a facet of the boundary (-).
        std::map<signed_quadindex, index_t> ppm_to_id_;

        // Maps (+--)-center vertex id quadruples to unique vertex id
        // +-- encodes the intersection between a Voronoi facet (+) and
        //    an edge of the boundary (--).
        std::map<signed_quadindex, index_t> pmm_to_id_;

        // Maps boundary vertex index to unique vertex id.
        vector<signed_index_t> bv_to_id_;
        index_t nb_vertices_;
    };

    /************************************************************************/

    /**
     * \brief Builds a Mesh using the symbolic information
     *  in the vertices computed by a RestrictedVoronoiDiagram.
     * \details The vertices with the same symbolic information are
     *  merged.
     * \note This is an internal implementation class, not meant to
     *  be used directly, use GEO::RestrictedVoronoiDiagram instead.
     */
    class RVDMeshBuilder {
    public:
        /**
         * \brief Constructs a new RVDMeshBuilder
         * \param[out] target where to build the mesh
         *     that represents the Restricted Voronoi Diagram
         * \param[in] reference the input mesh
         */
        RVDMeshBuilder(
            Mesh* target, Mesh* reference,
            Delaunay*
        ) :
            target_(target),
            nb_vertices_(0)
        {
            dim_ = coord_index_t(reference->vertices.dimension());
            current_seed_ = max_index_t();
        }

        /**
         * \brief Starts to build a new surface.
         */
        void begin_surface() {
            target_->clear();
            target_->vertices.set_dimension(dim_);
            facet_region_.bind(target_->facets.attributes(),"region");
        }

        /**
         * \brief Starts a new reference facet.
         * \details Does nothing in this implementation
         *  that does not use the reference facet index.
         * \param[in] ref_facet index of the reference facet (unused)
         */
        void begin_reference_facet(index_t ref_facet) {
            geo_argused(ref_facet);
        }

        /**
         * \brief Starts a new facet of the restricted
         *    Voronoi diagram.
         * \param[in] seed the Voronoi seed that
         *    corresponds to the new facet.
         */
        void begin_facet(index_t seed) {
            current_seed_ = seed;
            facet_vertices_.resize(0);
        }

        /**
         * \brief Adds a vertex to the current facet.
         * \param[in] point coordinates of the vertex
         * \param[in] sym symbolic representation of the vertex
         */
        void add_vertex_to_facet(
            const double* point, const SymbolicVertex& sym
        ) {
            index_t id = vertex_map_.find_or_create_vertex(
                current_seed_, sym
            );
            if(id >= nb_vertices_) {
                index_t v = target_->vertices.create_vertex();
                for(index_t c=0; c<dim_; ++c) {
                    target_->vertices.point_ptr(v)[c] = point[c];
                }
                nb_vertices_ = id + 1;
            }
            facet_vertices_.push_back(id);
        }

        /**
         * \brief Terminates the current facet.
         * \note The reference facet information is
         *  not used by this implementation.
         */
        void end_facet() {
            index_t f = target_->facets.create_polygon(facet_vertices_.size());
            for(index_t lv=0; lv<facet_vertices_.size(); ++lv) {
                target_->facets.set_vertex(f,lv,facet_vertices_[lv]);
            }
            facet_region_[f] = current_seed_;
        }

        /**
         * \brief Terminates the current reference facet.
         * \details Does nothing in this implementation.
         */
        void end_reference_facet() {
        }

        /**
         * \brief Terminates the current surface.
         */
        void end_surface() {
            target_->facets.connect();
            facet_region_.unbind();
        }

        /**
         * \brief Specifies the dimension to be used.
         * \details Not implemented yet, uses the dimension
         *   of the RestrictedVoronoiDiagram.
         */
        void set_dimension(coord_index_t x) {
            geo_argused(x);
            // TODO - Not implemented yet
        }

    private:
        Mesh* target_;
        Attribute<index_t> facet_region_;
        RVDVertexMap vertex_map_;
        coord_index_t dim_;
        index_t current_seed_;
        index_t nb_vertices_;
        vector<index_t> facet_vertices_;
    };

    /************************************************************************/
}

#endif

