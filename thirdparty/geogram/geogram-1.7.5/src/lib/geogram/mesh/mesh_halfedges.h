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

#ifndef GEOGRAM_MESH_MESH_HALFEDGES
#define GEOGRAM_MESH_MESH_HALFEDGES

#include <geogram/basic/common.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_geometry.h>
#include <iostream>

/**
 * \file geogram/mesh/mesh_halfedges.h
 * \brief Classes and function for virtually seeing a mesh as a set of halfedges
 */

namespace GEO {

    /**
     * \brief Exposes a half-edge like API for
     * traversing a Mesh.
     */
    class GEOGRAM_API MeshHalfedges {
    public:
        /**
         * \brief Stores a reference to a mesh corner and facet, and
         *  provides a halfedge-like API.
         */
        struct Halfedge {

            static const index_t NO_FACET  = index_t(-1);
            static const index_t NO_CORNER = index_t(-1);

            /**
             * \brief Constructs a new uninitialized Halfedge.
             */
            Halfedge() :
                facet(NO_FACET),
                corner(NO_CORNER) {
            }

            /**
             * \brief Constructs a new Halfedge from a facet and corner index.
             * \param[in] f the facet index
             * \param[in] c the corner index
             */
            Halfedge(index_t f, index_t c) :
                facet(f),
                corner(c) {
            }

            /**
             * \brief Clears this Halfedge.
             */
            void clear() {
                facet = NO_FACET;
                corner = NO_CORNER;
            }

            /**
             * \brief Tests whether this Halfedge is initialized.
             * \return true if this Halfedge is uninitialized, false otherwise
             */
            bool is_nil() const {
                return (facet == NO_FACET) && (corner == NO_CORNER);
            }

            /**
             * \brief Tests whether this Halfedge is the same as another one
             * \param[in] rhs the comparand
             * \return true if this Halfedge and \p rhs refer to the same
             *  facet and corner, false otherwise.
             */
            bool operator== (const Halfedge& rhs) const {
                return facet == rhs.facet && corner == rhs.corner;
            }

            /**
             * \brief Tests whether this Halfedge is different from another one
             * \param[in] rhs the comparand
             * \return true if this Halfedge and \p rhs refer to a different
             *  facet or corner, false otherwise.
             */
            bool operator!= (const Halfedge& rhs) const {
                return !(rhs == *this);
            }

            index_t facet;
            index_t corner;

        };

        /**
         * \brief Creates a new MeshHalfedges
         * \param[in] mesh the Mesh
         */
        MeshHalfedges(Mesh& mesh) : mesh_(mesh) {
        }

        /**
         * \brief Gets the mesh.
         * \return a reference to the mesh.
         */
        Mesh& mesh() {
            return mesh_;
        }

        /**
         * \brief Gets the mesh.
         * \return a const reference to the mesh.
         */
        const Mesh& mesh() const {
            return mesh_;
        }

        /**
         * \brief Sets whether facet regions determine borders.
         * \param[in] x if set, then an halfedge incident to two facets
         *  with different facet regions is considered to be a
         *  border
         */
        void set_use_facet_region(bool x) {
            if(x) {
                if(!facet_region_.is_bound()) {
                    facet_region_.bind(mesh_.facets.attributes(),"region");
                }
            } else {
                if(facet_region_.is_bound()) {
                    facet_region_.unbind();
                }
            }
        }

	/**
	 * \brief Sets a facet attribute name that determines borders.
	 * \param[in] attribute_name the name of the facet attribute to
	 *  be used to determine borders.
	 */
	void set_use_facet_region(const std::string& attribute_name) {
	    if(facet_region_.is_bound()) {
		facet_region_.unbind();
	    }
	    facet_region_.bind(mesh_.facets.attributes(),attribute_name);
	}

	
        /**
         * \brief Tests whether a Halfedge is valid.
         * \param[in] H the Halfedge to be tested
         * \return true if \p H refers to a halfedge that
         *  exists in the mesh, false otherwise
         * \note It only tests whether H.corner and H.facet are valid
         *  indices in the mesh, but does not test whether H.corner exists
         *  in H.facet.
         */
        bool halfedge_is_valid(const Halfedge& H) const {
            return
                H.facet != Halfedge::NO_FACET && 
                H.corner != Halfedge::NO_CORNER &&
                H.facet < mesh_.facets.nb() &&
                H.corner < mesh_.facet_corners.nb()
            ;
        }

        /**
         * \brief Tests whether a Halfedge is on the boder.
         * \details If set_use_facet_region() is set, then
         *  Halfedges incident to two different facet regions are
         *  considered as borders.
         * \param[in] H the Halfedge
         * \return true if \p H is on the border, false otherwise
         */
        bool halfedge_is_border(const Halfedge& H) const {
            geo_debug_assert(halfedge_is_valid(H));
            if(facet_region_.is_bound()) {
                index_t f = H.facet;
                index_t adj_f =
                    mesh_.facet_corners.adjacent_facet(H.corner);
                return
                    adj_f == NO_FACET ||
                    facet_region_[f] != facet_region_[adj_f]
                ;
            } 
            return mesh_.facet_corners.adjacent_facet(H.corner) == NO_FACET;
        }

        /**
         * \brief Replaces a Halfedge with the next one around the facet.
         * \param[in,out] H the Halfedge
         */
        void move_to_next_around_facet(Halfedge& H) const {
            geo_debug_assert(halfedge_is_valid(H));
            H.corner = mesh_.facets.next_corner_around_facet(H.facet, H.corner);
        }

        /**
         * \brief Replaces a Halfedge with the previous one around the facet.
         * \param[in,out] H the Halfedge
         */
        void move_to_prev_around_facet(Halfedge& H) const {
            geo_debug_assert(halfedge_is_valid(H));
            H.corner = mesh_.facets.prev_corner_around_facet(H.facet, H.corner);
        }
        
        /**
         * \brief Replaces a Halfedge with the next one around the vertex.
         * \param[in,out] H the Halfedge
         * \return true if the move was successful, false otherwise. On borders,
         *  the next halfedge around a vertex may not exist.
         */
        bool move_to_next_around_vertex(Halfedge& H) const;

        /**
         * \brief Replaces a Halfedge with the previous one around the vertex.
         * \param[in,out] H the Halfedge
         * \return true if the move was successful, false otherwise. On borders,
         *  the previous halfedge around a vertex may not exist.
         */
        bool move_to_prev_around_vertex(Halfedge& H) const;

        /**
         * \brief Replaces a Halfedge with the next one around the border.
         * \details If set_use_facet_region() is set, then
         *  Halfedges incident to two different facet regions are
         *  considered as borders.
         * \param[in,out] H the Halfedge
         */
        void move_to_next_around_border(Halfedge& H) const;

        /**
         * \brief Replaces a Halfedge with the previous one around the border.
         * \details If set_use_facet_region() is set, then
         *  Halfedges incident to two different facet regions are
         *  considered as borders.
         * \param[in,out] H the Halfedge
         */
        void move_to_prev_around_border(Halfedge& H) const;
        
        /**
         * \brief Replaces a Halfedge with the opposite one in the
         *  adjacent facet.
         * \param[in,out] H the Halfedge
         * \pre !is_on_border(H)
         */
        void move_to_opposite(Halfedge& H) const;

    private:
        Mesh& mesh_;
        Attribute<index_t> facet_region_;
    };

    /**
     * \brief Displays a Halfedge.
     * \param[out] out the stream where to print the Halfedge
     * \param[in] H the Halfedge
     * \return a reference to the stream \p out
     */
    inline std::ostream& operator<< (
        std::ostream& out, const MeshHalfedges::Halfedge& H
    ) {
        return out << '(' << H.facet << ',' << H.corner << ')';
    }

    namespace Geom {

        /**
         * \brief Gets the origin point of a Halfedge
         * \param[in] M the mesh
         * \param[in] H the Halfedge
         * \return a const reference to the origin of \p H
         */
        inline const vec3& halfedge_vertex_from(
            const Mesh& M, const MeshHalfedges::Halfedge& H
        ) {
            return mesh_vertex(M, M.facet_corners.vertex(H.corner));
        }

        /**
         * \brief Gets the arrow extremity point of a Halfedge
         * \param[in] M the mesh
         * \param[in] H the Halfedge
         * \return a const reference to the arrow extremity of \p H
         */
        inline const vec3& halfedge_vertex_to(
            const Mesh& M, const MeshHalfedges::Halfedge& H
        ) {
            index_t c = M.facets.next_corner_around_facet(H.facet, H.corner);
            return mesh_vertex(M, M.facet_corners.vertex(c));
        }

        /**
         * \brief Gets a 3d vector that connects the origin with the arrow
         *  extremity of a Halfedge.
         * \param[in] M the Mesh
         * \param[in] H the Halfedge
         * \return a 3d vector that connects the origin with the arrow
         *  extremity of \p H
         */
        inline vec3 halfedge_vector(
            const Mesh& M, const MeshHalfedges::Halfedge& H
        ) {
            return halfedge_vertex_to(M, H) - halfedge_vertex_from(M, H);
        }

        /**
         * \brief Gets the length of a Halfedge
         * \param[in] M the Mesh
         * \param[in] H the Halfedge
         * \return the 3d length of \p H
         */
        inline double edge_length(
            const Mesh& M, const MeshHalfedges::Halfedge& H
        ) {
            return length(halfedge_vector(M, H));
        }
    }
}

#endif

