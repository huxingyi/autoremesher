/*
 *  Copyright (c) 2012-2016, Bruno Levy
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

#ifndef GEOGRAM_GFX_GLUP_GLUP_MARCHING_CELLS
#define GEOGRAM_GFX_GLUP_GLUP_MARCHING_CELLS

#include <geogram_gfx/GLUP/GLUP.h>
#include <geogram_gfx/basic/common.h>
#include <geogram/basic/logger.h>
#include <geogram/mesh/mesh.h> // For cell descriptors / marching cells.

/**
 * \file geogram_gfx/GLUP/GLUP_marching_cells.h
 * \brief Implementation of the marching cells algorithms.
 * \details Used to implement GLUP_CLIP_SLICE_CELLS clipping mode.
 */

namespace GLUP {
    using namespace GEO;

    /**********************************************************************/
    
    /**
     * \brief Implements the MarchingCells algorithm.
     * \details MarchingCell compute the intersection between
     *  a cell and a plane, using only combinatorial information.
     *  It uses the static tables from the Mesh class, so that cell
     *  numberings are coherent between storage and graphics.
     */
    class MarchingCell {
    public:

        /**
         * \brief MarchingCell constructor
         * \param[in] prim the GLUP volumetric primitive, should be one
         *  of GLUP_TETRAHEDRA, GLUP_HEXAHEDRA, GLUP_PRISMS, GLUP_PYRAMIDS.
         */
        MarchingCell(GLUPprimitive prim);

        /**
         * \brief MarchingCell destructor.
         */
        ~MarchingCell();

        /**
         * \brief Gets the number of vertices.
         * \return the number of vertices in a cell
         */
        index_t nb_vertices() const {
            return nb_vertices_;
        }

        /**
         * \brief Gets the number of edges.
         * \return the number of edges in a cell
         */
        index_t nb_edges() const {
            return nb_edges_;
        }


        /**
         * \brief Gets the number of configurations.
         * \return the number of configurations
         */
        index_t nb_configs() const {
            return nb_configs_;
        }
        
        /**
         * \brief Gets a vertex by edge index and local vertex index.
         * \param[in] e the index of the edge
         * \param[in] lv the local vertex index in the edge, one of 0,1
         * \return the vertex index 
         */
        index_t edge_vertex(index_t e, index_t lv) const {
            geo_debug_assert(e < nb_edges());
            geo_debug_assert(lv < 2);
            return edge_[e*2+lv];
        }

        /**
         * \brief Gets the number of intersected edges in a configuration.
         * \param[in] config the vertex configuration bitcode. The
         *  bit corresponding to vertex v is set if v is on the positive
         *  side of the intersection plane.
         * \return the number of intersected edges in configuration \p config
         */
        index_t config_size(index_t config) const {
            geo_debug_assert(config < nb_configs());
            return config_size_[config];
        }

        /**
         * \brief Gets the maximum configuration size.
         * \return the largest number of vertices in an intersection polygon
         */
        index_t max_config_size() const {
            return max_config_size_;
        }

        /**
         * \brief Gets the list of intersected edges in a configuration.
         * \param[in] config the vertex configuration bitcode. The
         *  bit corresponding to vertex v is set if v is on the positive
         *  side of the intersection plane.
         * \return a pointer to the array of edge indices that correspond to
         *  this configuration, of size config_size()
         */
        const index_t* config_edges(index_t config) const {
            geo_debug_assert(config < nb_configs());
            return config_ + config * nb_edges_;
        }

        /**
         * \brief Gets the GLSL declaration of marching cell uniform state.
         * \return a pointer to GLSL source code that declares this
         *  marching cell's uniform state.
         */
        const char* GLSL_uniform_state_declaration() const {
            return GLSL_uniform_state_declaration_.c_str();
        }

        /**
         * \brief Gets the GLSL declaration of the function that
         *  computes the intersections.
         * \return a pointer to GLSL source code.
         */
        const char* GLSL_compute_intersections() const {
            return GLSL_compute_intersections_.c_str();
        }

        
        /**
         * \brief Gets the binding point of the uniform buffer that
         *  contains the tables for the marching cell.
         * \return the uniform binding point
         */
        GLuint uniform_binding_point() const {
            return uniform_binding_point_;
        }

        /**
         * \brief Creates a Uniform Buffer Object that contains
         *  the tables for the marching cell.
         * \details Used by GLUP150 and GLUP440 profiles that support
         *  UBOs. This MarchingCells keeps ownership of the created
         *  UBO (it is destroyed by the destructor of this MarchingCells).
         */
        GLuint create_UBO();

        /**
         * \brief Creates a Vertex Buffer Object with the indices
         *  for all configurations.
         * \details Used by GLUPES2 that does not support UBOs.
         *  This MarchingCells keeps ownership of the created
         *  VBO (it is destroyed by the destructor of this MarchingCells).
         * \note NOT USED YET.
         */
        GLuint create_elements_VBO();
        
        /**
         * \brief Binds the uniform state marching cell variables
         *  to a given program.
         */
        void bind_uniform_state(GLuint program);
        
    protected:

        /**
         * \brief Computes the intersection polygon for a configuration.
         * \param[in] config the vertex configuration bitcode. The
         *  bit corresponding to vertex v is set if v is on the positive
         *  side of the intersection plane.
         */
        void compute_config(index_t config);
        
        /**
         * \brief Moves from a given halfedge to the next halfege.
         * \details The halfedge is refered to as a facet index and a
         *  local vertex index within the facet.
         * \param[in,out] f the index of the facet
         * \param[in,out] lv the local index of the vertex in the facet.
         */
        void move_to_next(index_t& f, index_t& lv) {
            lv = (lv+1) % desc_->nb_vertices_in_facet[f];
        }

        /**
         * \brief Gets the origin vertex of a halfedge.
         * \details The halfedge is refered to as a facet index and a
         *  local vertex index within the facet.
         * \param[in] f the index of the facet
         * \param[in] lv the local index of the vertex in the facet.
         * \return the index of the origin vertex of the halfedge
         */
        index_t origin_vertex(index_t f, index_t lv) {
            return desc_->facet_vertex[f][lv];
        }

        /**
         * \brief Gets the destination vertex of a halfedge.
         * \details The halfedge is refered to as a facet index and a
         *  local vertex index within the facet.
         * \param[in] f the index of the facet
         * \param[in] lv the local index of the vertex in the facet.
         * \return the index of the destination vertex of the halfedge
         */
        index_t destination_vertex(index_t f, index_t lv) {
            move_to_next(f, lv);
            return origin_vertex(f, lv);
        }

        /**
         * \brief Gets the edge index that corresponds to a given halfedge.
         * \details The halfedge is refered to as a facet index and a
         *  local vertex index within the facet.
         * \param[in] f the index of the facet
         * \param[in] lv the local index of the vertex in the facet.
         * \return the index of the edge.
         */
        index_t edge(index_t f, index_t lv) {
            index_t v1 = origin_vertex(f, lv);
            index_t v2 = destination_vertex(f, lv);
            index_t result = vv_to_e_[v1*desc_->nb_vertices+v2];
            geo_debug_assert(result != index_t(-1));
            return result;
        }

        /**
         * \brief Moves from a given halfedge to the opposite halfege.
         * \details The halfedge is refered to as a facet index and a
         *  local vertex index within the facet.
         * \param[in,out] f the index of the facet
         * \param[in,out] lv the local index of the vertex in the facet.
         */
        void move_to_opposite(index_t& f, index_t& lv);

        /**
         * \brief Tests whether a given edge is intersected.
         * \details The halfedge is refered to as a facet index and a
         *  local vertex index within the facet.
         * \param[in] f the index of the facet
         * \param[in] lv the local index of the vertex in the facet.
         * \param[in] config the vertex configuration bitcode. The
         *  bit corresponding to vertex v is set if v is on the positive
         *  side of the intersection plane.
         * \retval true if the edge is intersected
         * \retval false otherwise
         */
        bool edge_is_intersected(index_t f, index_t lv, index_t config) {
            index_t v1 = origin_vertex(f, lv);
            index_t v2 = destination_vertex(f, lv);
            bool v1_in = ((config & 1u<<v1) != 0);
            bool v2_in = ((config & 1u<<v2) != 0);
            return (v1_in != v2_in);
        }

        /**
         * \brief Tests whether a vertex configuration bitcode is
         *  ambiguous.
         * \details A configuration is ambiguous if it results in several
         *  intersection polygons.
         * \param[in] config the vertex configuration bitcode. The
         *  bit corresponding to vertex v is set if v is on the positive
         *  side of the intersection plane.
         * \retval true if the configuration is ambiguous
         * \retval false otherwise
         */
        bool config_is_ambiguous(index_t config);
        
        /**
         * \brief Gets the first intersected halfedge given a 
         *  vertex configuration.
         * \param[out] f the facet of the first intersected halfedge
         * \param[out] lv the local vertex index of the first intersected
         *  halfedge
         * \param[in] config the vertex configuration bitcode. The
         *  bit corresponding to vertex v is set if v is on the positive
         *  side of the intersection plane.
         * \retval true if there was an intersection
         * \retval false otherwise
         */
        bool get_first_edge(index_t& f, index_t& lv, index_t config);

    private:
        /**
         * \brief Forbids copy.
         */
        MarchingCell(const MarchingCell& rhs);
        
        /**
         * \brief Forbids copy.
         */
        MarchingCell& operator=(const MarchingCell& rhs);
        
    private:
        const CellDescriptor* desc_;
        index_t vv_to_e_[64];
        index_t nb_vertices_;
        index_t nb_configs_;
        index_t* config_size_;
        index_t max_config_size_;
        index_t* config_;
        index_t nb_edges_;
        index_t* edge_;
        std::string GLSL_uniform_state_declaration_;
        std::string GLSL_compute_intersections_;
        GLuint uniform_binding_point_;
        GLuint UBO_;
        GLuint elements_VBO_;
    };

    /**********************************************************************/
    
}

#endif
