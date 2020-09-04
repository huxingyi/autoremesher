
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

#ifndef GEOGRAM_VORONOI_GENERIC_RVD_CELL
#define GEOGRAM_VORONOI_GENERIC_RVD_CELL

#include <geogram/basic/common.h>
#include <geogram/voronoi/generic_RVD_vertex.h>
#include <geogram/basic/argused.h>
#include <geogram/basic/attributes.h>
#include <iosfwd>
#include <stack>

/**
 * \file geogram/voronoi/generic_RVD_cell.h
 * \brief Internal representation of polyhedra for GEO::GenericVoronoiDiagram.
 * \note This file contains functions and classes used by the 
 *  internal implementation of GEO::GenericVoronoiDiagram. 
 *  They are not meant to be used directly by client code.
 *  Users who whant similar functionalities may use GEO::ConvexCell instead.
 */

namespace GEOGen {

    using GEO::Mesh;
    
    /**
     * \brief Computes the intersection between a set of halfspaces.
     * \note This is an internal implementation class used by
     *  GEO::RestrictedVoronoiDiagram. It is not meant to be 
     *  used directly by client code.
     */
    class GEOGRAM_API ConvexCell {

        /** \brief This class type */
        typedef ConvexCell thisclass;

    public:

        static const index_t NO_TRIANGLE = index_t(-1);
        static const index_t NO_VERTEX = index_t(-1);
        static const index_t END_OF_LIST = index_t(-1);

        /**
         * \brief Represents the current state of a triangle.
         */
        enum TriangleStatus {
            TRI_IS_FREE = 0,
            TRI_IS_CONFLICT = 1,
            TRI_IS_USED = 2
        };

        /**
         * \brief Represents a vertex of this ConvexCell in dual form.
         * \details Each vertex of a ConvexCell is represented
         * combinatorially as a Triangle, in dual form
         * (in primal parlance, each vertex is
         * of degree 3, and can be represented by a triangle).
         * A Triangle knows its tree vertices, its tree adjacent triangles
         * and the geometric point it corresponds to.
         */

        struct Triangle {

            /**
             * \brief Creates a new triangle.
             * \param[in] v0 index of the first vertex
             * \param[in] v1 index of the second vertex
             * \param[in] v2 index of the third vertex
             * \param[in] f0 index of the triangle opposite to \p v0
             * \param[in] f1 index of the triangle opposite to \p v1
             * \param[in] f2 index of the triangle opposite to \p v2
             */
            Triangle(
                index_t v0, index_t v1, index_t v2,
                index_t f0, index_t f1, index_t f2
            ) :
                next_(END_OF_LIST),
                status_(TRI_IS_FREE),
                id_(-1) {
                v[0] = v0;
                v[1] = v1;
                v[2] = v2;
                t[0] = f0;
                t[1] = f1;
                t[2] = f2;
            }

            /**
             * \brief Creates a new uninitialized Triangle.
             */
            Triangle() :
                next_(END_OF_LIST),
                status_(TRI_IS_FREE),
                id_(-1) {
                v[0] = NO_VERTEX;
                v[1] = NO_VERTEX;
                v[2] = NO_VERTEX;
                t[0] = NO_TRIANGLE;
                t[1] = NO_TRIANGLE;
                t[2] = NO_TRIANGLE;
            }

            GEOGen::Vertex dual_;
            index_t v[3];   // The 3 vertices of this triangle
            index_t t[3];   // The 3 triangles adjacent to this triangle
            index_t next_;  // Linked list management.
            TriangleStatus status_;
            // TRI_IS_FREE,TRI_IS_USED or TRI_IS_CONFLICT.
            signed_index_t id_;
        };

        /**
         * \brief Represents a facet of this ConvexCell in dual form.
         * \details Each facet of a ConvexCell is represented
         * combinatorially as a Vertex (dual form).
         * Each vertex v knows an incident triangle (v.t).
         */
        class Vertex {
        public:
            /**
             * \brief Creates a new uninitialized Vertex.
             */
            Vertex() :
                t(-1),
                id_(-1) {
            }

            signed_index_t t;  // One triangle incident to this vertex
            signed_index_t id_;
        };

        /**
         * \brief ConvexCell constructor.
         * \param[in] dim dimension of the ConvexCell, e.g. 3 for 3d
         */
        ConvexCell(coord_index_t dim) :
            first_free_(END_OF_LIST),
            v_to_t_dirty_(false),
            intersections_(dim),
            symbolic_is_surface_(false),
            cell_id_(-1) {
        }

        /**
	 * \brief Copies a ConvexCell.
	 * \details The allocated vertices are shared with \p rhs, thus
	 *  \p rhs should not be deleted before this ConvexCell.
	 * \param[in] rhs a const reference to the ConvexCell to be
	 *  copied.
	 */
	void copy(const ConvexCell& rhs);    
	    
        /**
         * \brief Gets the dimension of this ConvexCell.
         * \return the dimension of this ConvexCell, e.g. 3 for 3d
         */
        coord_index_t dimension() const {
            return intersections_.dimension();
        }

        /**
         * \brief Clears this ConvexCell.
         */
        void clear() {
            first_free_ = END_OF_LIST;
            triangles_.resize(0);
            vertices_.resize(0);
            v_to_t_dirty_ = false;
            intersections_.clear();
        }

        /**
         * \brief Specifies that symbolic information is
         *  relative to surfacic mesh (rather than volumetric mesh).
         * \details Affects the behavior of side_exact().
         * \param[in] x true if symbolic information is relative
         *  to surfacic mesh (triangles), false if symbolic information
         *  is relative to volumetric mesh (tetrahedra).
         */
        void set_symbolic_is_surface(bool x) {
            symbolic_is_surface_ = x;
        }

        /**
         * \brief Assigns a mesh tetrahedron to this ConvexCell
         * \details The tetrahedron from the initial mesh is converted into
         *  the internal geometric/symbolic representation.
         * \param[in] mesh the mesh from which the tetrahedron is copied
         * \param[in] t the index of the tetrahedron in \p mesh
         * \param[in] symbolic if true, symbolic information is copied
         * \param[in] vertex_weight if bound, an attribute that gives
         *  the weight of each vertex in \p mesh.
         */
        void initialize_from_mesh_tetrahedron(
            const Mesh* mesh, index_t t, bool symbolic,
            const GEO::Attribute<double>& vertex_weight
        );


        /**
         * \brief Copies a Mesh into a ConvexCell
         * \details The surface mesh in \p mesh represents the boundary
         *  of the ConvexCell.
         * \param[in] mesh a pointer to the input Mesh
         * \param[in] symbolic if true, symbolic information is copied
         */
        void initialize_from_surface_mesh(
            Mesh* mesh, bool symbolic
        );


        /**
         * \brief Copies a ConvexCell into a Mesh
         * \details On exit, the output mesh is a surfacic
         *  mesh with the boundary of the convex cell.
         * \param[out] mesh a pointer to the target mesh
         * \param[in] copy_symbolic_info if true, symbolic
         *   information is copied. An attribute "id" is attached
         *   to the facets. The value of id[f] is either 1 + the index of
         *   the Voronoi vertex that generated with \p i the bisector that
         *   created the facet, or -1-g if the facet was an original facet
         *   of mesh \p mesh, where g is the index of the original 
	 *   facet in \p mesh.
         */
        void convert_to_mesh(Mesh* mesh, bool copy_symbolic_info = false);
        
        /**
         * \brief Clips this ConvexCell with a plane.
         * \details The plane is specified as a bisector
         *    in a Delaunay triangulation.
         * \param[in] mesh input mesh, used by exact predicates
         * \param[in] delaunay the Delaunay triangulation
         * \param[in] i index of the first extremity of the bisector
         * \param[in] j index of the second extremity of the bisector
         * \param[in] exact if true, uses exact predicates (implies symbolic)
         * \param[in] symbolic if true, computes symbolic information
         * \return the index of the newly created vertex that corresponds to
         *  the dual of the new face or -1 if the input cell was completely
         *  on the negative side (removed everything)
         * \tparam DIM dimension (specified as a template parameter for
         *   efficiency reasons).
         * \pre DIM == dimension()
         */
        template <index_t DIM>
        signed_index_t clip_by_plane(
            const Mesh* mesh, const Delaunay* delaunay,
            index_t i, index_t j,
            bool exact, bool symbolic
        ) {
            index_t new_v = create_vertex();
            set_vertex_id(index_t(new_v), signed_index_t(j)+1);
            index_t conflict_begin, conflict_end;

            // Phase I: Determine the conflict zone and chain the triangles
            // Note: they are not immediately deleted, since we need the
            // geometric information in the triangles to compute the new
            // intersections.
            get_conflict_list<DIM>(
                mesh, delaunay, i, j, exact, conflict_begin, conflict_end
            );

            // Special case: the clipping plane did not clip anything.
            if(conflict_begin == END_OF_LIST) {
                return signed_index_t(new_v);
            }

            // Phase II: Find a triangle on the border of the conflict zone
            // (by traversing the conflict list).
	    index_t first_conflict_t;
	    index_t first_conflict_e;
            bool found_h = find_triangle_on_border(
                conflict_begin, conflict_end,
		first_conflict_t, first_conflict_e
            );

            // The clipping plane removed everything !
	    // (note: cannot be empty conflict list, since this
	    //  case was detected by previous test at the end of
	    //  phase I)
            if(!found_h) {
                clear();
                return -1;
            }

            // Phase III: Triangulate hole.
            triangulate_hole<DIM>(
                delaunay, i, j, symbolic,
		first_conflict_t, first_conflict_e,
		new_v
            );

            // Phase IV: Merge the conflict zone into the free list.
            merge_into_free_list(conflict_begin, conflict_end);
            return signed_index_t(new_v);
        }

        /**
         * \brief Gets the maximum valid triangle index plus one.
         * \details May be greater than nb_t() if this ConvexCell has
         *   some free (unused) triangles.
         */
        index_t max_t() const {
            return triangles_.size();
        }

        /**
         * \brief Gets the number of used triangles.
         */
        index_t nb_t() const {
            index_t result = 0;
            for(index_t t = 0; t < max_t(); t++) {
                if(triangle_is_used(t)) {
                    result++;
                }
            }
            return result;
        }

        /**
         * \brief Gets the maximum valid vertex index plus one.
         */
        index_t max_v() const {
            return vertices_.size();
        }

        /**
         * \brief Tests whether a given triangle is free.
         */
        bool triangle_is_free(index_t t) const {
            geo_debug_assert(t != NO_TRIANGLE);
            geo_debug_assert(t < max_t());
            return triangles_[t].status_ == TRI_IS_FREE;
        }

        /**
         * \brief Tests whether a given triangle is valid.
         * \details A "valid" triangle is a triangle from which
         * we can query information (vertices, adjacent triangles,
         * embedding), therefore it is a "used" or "conflict" triangle.
         * \param[in] t index of the triangle
         * \pre t < max_t()
         */
        bool triangle_is_valid(index_t t) const {
            return !triangle_is_free(t);
        }

        /**
         * \brief Tests whether a given triangle is used.
         * \param[in] t index of the triangle
         * \pre t < max_t()
         */
        bool triangle_is_used(index_t t) const {
            geo_debug_assert(t != NO_TRIANGLE);
            geo_debug_assert(t < max_t());
            return triangles_[t].status_ == TRI_IS_USED;
        }

        /**
         * \brief Tests whether a given triangle belongs to
         *  the conflict zone.
         * \param[in] t index of the triangle
         * \pre t < max_t()
         */
        bool triangle_is_conflict(index_t t) const {
            geo_debug_assert(t != NO_TRIANGLE);
            geo_debug_assert(t < max_t());
            return triangles_[t].status_ == TRI_IS_CONFLICT;
        }

        /**
         * \brief Gets the index of a triangle vertex.
         * \param[in] t the triangle index
         * \param[in] iv local vertex index (0,1 or 2) in \p t
         * \return the index of triangle's \p iv th vertex
         * \pre t < max_t()
         */
        index_t triangle_vertex(index_t t, index_t iv) const {
            geo_debug_assert(iv < 3);
            geo_debug_assert(triangle_is_valid(t));
            return triangles_[t].v[iv];
        }

        /**
         * \brief Gets the index of a triangle adjacent to another one.
         * \param[in] t the triangle index
         * \param[in] e local edge index (0,1 or 2) in \p t
         * \return the index of the triangle adjacent to \p t on edge \p e
         */
        index_t triangle_adjacent(index_t t, index_t e) const {
            geo_debug_assert(e < 3);
            geo_debug_assert(triangle_is_valid(t));
            return triangles_[t].t[e];
        }

        /**
         * \brief Sets a vertex of a triangle.
         * \param[in] t the triangle index
         * \param[in] iv local vertex index (0,1 or 2) in \p t
         * \param[in] v global vertex index
         */
        void set_triangle_vertex(index_t t, index_t iv, index_t v) {
            geo_debug_assert(t != NO_TRIANGLE);
            geo_debug_assert(t < max_t());
            geo_debug_assert(iv < 3);
            geo_debug_assert(v < max_v());
            triangles_[t].v[iv] = v;
        }

        /**
         * \brief Sets a triangle adjacency.
         * \param[in] t the triangle index
         * \param[in] e local edge index (0,1 or 2)
         * \param[in] t2 global triangle index
         */
        void set_triangle_adjacent(
            index_t t, index_t e, index_t t2
        ) {
            geo_debug_assert(t != NO_TRIANGLE);
            geo_debug_assert(t < max_t());
            geo_debug_assert(e < 3);
            geo_debug_assert(t2 < max_t());
            triangles_[t].t[e] = t2;
        }

        /**
         * \brief Finds the local index of a triangle vertex
         * \param[in] t the triangle
         * \param[in] v global vertex index
         * \return local vertex index (0,1 or 2) of \p v in \p t
         * \pre \p t is incident to \p v
         */
        index_t find_triangle_vertex(index_t t, index_t v) const {
            geo_debug_assert(t != NO_TRIANGLE);
            geo_debug_assert(t < max_t());
            geo_debug_assert(v < max_v());

            // The following expression is 10% faster than using
            // if() statements (multiply by boolean result of test).
            // Thank to Laurent Alonso for this idea.
            index_t result = index_t(
                (triangles_[t].v[1] == v) | ((triangles_[t].v[2] == v) * 2)
            );
            geo_debug_assert(triangles_[t].v[result] == v);
            return result;
        }

        /**
         * \brief Finds the edge along which two triangles are adjacent
         * \param[in] t1 first triangle
         * \param[in] t2 second triangle
         * \return the local edge index (0,1 or 2) in \p t1 along which
         *    \p t2 is adjacent to \p t1
         * \pre \p t1 and \p t2 are adjacent
         */
        index_t triangle_adjacent_index(
            index_t t1, index_t t2
        ) const {
            geo_debug_assert(t1 != NO_TRIANGLE);
            geo_debug_assert(t1 < max_t());
            geo_debug_assert(t2 != NO_TRIANGLE);
            geo_debug_assert(t2 < max_t());

            // The following expression is 10% faster than using
            // if() statements (multiply by boolean result of test).
            // Thank to Laurent Alonso for this idea.
            index_t result = index_t(
                (triangles_[t1].t[1] == t2) | ((triangles_[t1].t[2] == t2) * 2)
            );

            geo_debug_assert(triangles_[t1].t[result] == t2);

            return result;
        }

        /**
         * \brief Gets one of the triangles incident to a vertex.
         * \details The information is cached and reconstructed
         *   whenever the v_to_t_dirty_ flag is positioned.
         * \param[in] v index of the vertex
         * \return the index of a triangle incident to \p v
         */
         signed_index_t vertex_triangle(index_t v) const {
            if(v_to_t_dirty_) {
                const_cast<ConvexCell*>(this)->init_v_to_t();
            }
            geo_debug_assert(v != NO_VERTEX);
            geo_debug_assert(v < max_v());
            return vertices_[v].t;
        }

        /**
         * \brief Stores in a vertex the index of one
         *  the triangles incident to it.
         * \param[in] v index of the vertex
         * \param[in] t index of one of the triangles incident to \p v
         */
        void set_vertex_triangle(index_t v, index_t t) {
            geo_debug_assert(v != NO_VERTEX);
            geo_debug_assert(index_t(v) < max_v());
            vertices_[v].t = signed_index_t(t);
        }

        /**
         * \brief Gets the dual vertex that corresponds to a triangle.
         * \details Each triangle corresponds to a vertex of the ConvexCell
         *   (combinatorics are stored in dual form).
         * \param[in] t index of the triangle
         * \return a const reference to the GEOGen::Vertex that corresponds
         *  to the triangle, with both geometrical and combinatorial
         *  representations
         */
        const GEOGen::Vertex& triangle_dual(index_t t) const {
            geo_debug_assert(triangle_is_valid(t));
            return triangles_[t].dual_;
        }

        /**
         * \brief Gets the dual vertex that corresponds to a triangle.
         * \details Each triangle corresponds to a vertex of the ConvexCell
         *   (combinatorics are stored in dual form).
         * \param[in] t index of the triangle
         * \return a reference to the GEOGen::Vertex that corresponds
         *  to the triangle, with both geometrical and
         *  combinatorial representations
         */
        GEOGen::Vertex& triangle_dual(index_t t) {
            geo_debug_assert(triangle_is_valid(t));
            return triangles_[t].dual_;
        }

        /**
         * \brief Gets the id of this ConvexCell.
         * \details The id can be used to maintain the
         * correspondence with a tetrahedron in the Mesh.
         * \return the id of this ConvexCell. Can be a
         *  negative number.
         */
        signed_index_t cell_id() const {
            return cell_id_;
        }

        /**
         * \brief Sets the id of this ConvexCell.
         * \details The id can be used to maintain the
         * correspondence with a tetrahedron in the Mesh.
         * \param[in] i the id of this ConvexCell. Can be
         *  a negative number.
         */
        void set_cell_id(signed_index_t i) {
            cell_id_ = i;
        }

        /**
         * \brief Gets the id of a triangle.
         * \details Each triangle of a ConvexCell has an id, that can
         * be used to maintain the correspondence with a vertex in the Mesh.
         * \return The id of triangle \p t.
         */
        signed_index_t triangle_id(index_t t) const {
            geo_debug_assert(triangle_is_valid(t));
            return triangles_[t].id_;
        }

        /**
         * \brief Sets the id of a triangle.
         * \details Each triangle of a ConvexCell has an id, that can
         * be used to maintain the correspondence with a vertex in the Mesh.
         */
        void set_triangle_id(index_t t, signed_index_t id) {
            geo_debug_assert(triangle_is_valid(t));
            triangles_[t].id_ = id;
        }

        /**
         * \brief Gets the id of a vertex.
         * \details Each vertex of a ConvexCell has an id, that can
         * be used to maintain the correspondence with a facet in the Mesh.
         */
        signed_index_t vertex_id(index_t v) const {
            geo_debug_assert(v != NO_VERTEX);
            geo_debug_assert(v < max_v());
            return vertices_[v].id_;
        }

        /**
         * \brief Sets the id of a vertex.
         * \details Each vertex of a ConvexCell has an id, that can
         * be used to maintain the correspondence with a facet in the Mesh.
         */
        void set_vertex_id(index_t v, signed_index_t id) {
            geo_debug_assert(v != NO_VERTEX);
            geo_debug_assert(v < max_v());
            vertices_[v].id_ = id;
        }
	
        /**
         * \brief A Corner corresponds to a vertex seen from a triangle.
         * \details Corner has helper functions that facilitate traversing
         * the vertices of a facet in dual form.
         */
        class Corner {
        public:
            /**
             * \brief Creates an uninitialized Corner.
             */
            Corner() :
                t(NO_TRIANGLE),
                v(3) {
            }

            /**
             * \brief Creates a Corner from a triangle index and
             *    local vertex index.
             * \param[in] t_in index of the triangle
             * \param[in] v_in local index (0,1 or 2) in triangle \p t_in.
             */
            Corner(index_t t_in, index_t v_in) :
                t(t_in),
                v(v_in) {
            }

            /**
             * \brief Compares two corners.
             * \param[in] rhs the right hand side
             * \retval true if this Corner and \p rhs correspond to the same
             *  triangle and the same vertex
             * \retval false otherwise
             */
            bool operator== (const Corner& rhs) const {
                return t == rhs.t && v == rhs.v;
            }

            /**
             * \brief Compares two corners.
             * \param[in] rhs the right hand side
             * \retval true if this Corner and \p rhs correspond to different
             *   triangles or different vertices
             * \retval false otherwise
             */
            bool operator!= (const Corner& rhs) const {
                return t != rhs.t || v != rhs.v;
            }

            index_t t;
            index_t v;
        };

        /**
         * \brief Replaces a corner by the next corner obtained by turing around
         * the vertex.
         * \param[in,out] c the corner
         */
        void move_to_next_around_vertex(Corner& c) const {
            index_t t2 = triangle_adjacent(c.t, plus1mod3(c.v));
            index_t v = triangle_vertex(c.t, c.v);
            c.v = find_triangle_vertex(t2, v);
            c.t = t2;
        }

        /**
         * \brief Updates the cache that stores for each vertex a triangle
         *   incident to it.
         */
        void init_v_to_t() {
            v_to_t_dirty_ = false;
            for(index_t v = 0; v < max_v(); v++) {
                set_vertex_triangle(v, NO_TRIANGLE);
            }
            for(index_t t = 0; t < max_t(); t++) {
                if(triangle_is_used(t)) {
                    for(index_t iv = 0; iv < 3; iv++) {
                        set_vertex_triangle(triangle_vertex(t, iv), t);
                    }
                }
            }
        }

        /**
         * \brief Creates a new uninitialized triangle.
         * \details The created triangle is marked as used.
         * \return The index of the new triangle.
         */
        index_t create_triangle() {
            if(first_free_ == END_OF_LIST) {
                grow();
            }
            index_t result = first_free_;
            first_free_ = next_triangle(first_free_);
            mark_as_used(result);
            set_triangle_id(result, -1);
            return result;
        }

        /**
         * \brief Creates a new triangle with specified vertices.
         * \details The created triangle is marked as used. Adjacent
         *   triangles are left uninitialized.
         * \param[in] v0 index of first vertex
         * \param[in] v1 index of second vertex
         * \param[in] v2 index of third vertex
         * \return the index of the new triangle
         */
        index_t create_triangle(index_t v0, index_t v1, index_t v2) {
            index_t t = create_triangle();
            triangles_[t].v[0] = v0;
            triangles_[t].v[1] = v1;
            triangles_[t].v[2] = v2;
            return t;
        }

        /**
         * \brief Creates a new triangles with specified vertices and
         *  adjacent triangles.
         * \details The created triangle is marked as used.
         * \param[in] v0 index of first vertex
         * \param[in] v1 index of second vertex
         * \param[in] v2 index of third vertex
         * \param[in] t0 index of adjacent triangle opposite to \p v0
         * \param[in] t1 index of adjacent triangle opposite to \p v1
         * \param[in] t2 index of adjacent triangle opposite to \p v2
         * \return the index of the new triangle
         */
        index_t create_triangle(
            index_t v0, index_t v1, index_t v2,
            index_t t0, index_t t1, index_t t2
        ) {
            index_t t = create_triangle();
            triangles_[t].v[0] = v0;
            triangles_[t].v[1] = v1;
            triangles_[t].v[2] = v2;
            triangles_[t].t[0] = t0;
            triangles_[t].t[1] = t1;
            triangles_[t].t[2] = t2;
            return t;
        }

        /**
         * \brief Sets the vertices and adjacent triangles of a triangle.
         * \param[in] t index of the triangle
         * \param[in] v0 index of first vertex
         * \param[in] v1 index of second vertex
         * \param[in] v2 index of third vertex
         * \param[in] t0 index of adjacent triangle opposite to \p v0
         * \param[in] t1 index of adjacent triangle opposite to \p v1
         * \param[in] t2 index of adjacent triangle opposite to \p v2
         */
        void set_triangle(
            index_t t,
            index_t v0, index_t v1, index_t v2,
            index_t t0, index_t t1, index_t t2
        ) {
            geo_debug_assert(t != NO_TRIANGLE);
            geo_debug_assert(t < max_t());
            triangles_[t].v[0] = v0;
            triangles_[t].v[1] = v1;
            triangles_[t].v[2] = v2;
            triangles_[t].t[0] = t0;
            triangles_[t].t[1] = t1;
            triangles_[t].t[2] = t2;
        }

        /**
         * \brief Creates a new triangles with specified vertices,
         *  adjacent triangles and geometric location at the dual
         *  vertex.
         * \details The vertex is shared with caller.
         * The created triangle is marked as used.
         * \param[in] p geometric location at the dual vertex, shared with
         *   caller. Caller remains responsible for memory management.
         * \param[in] w the weight associated with point \p p
         * \param[in] v0 index of first vertex
         * \param[in] v1 index of second vertex
         * \param[in] v2 index of third vertex
         * \param[in] t0 index of adjacent triangle opposite to \p v0
         * \param[in] t1 index of adjacent triangle opposite to \p v1
         * \param[in] t2 index of adjacent triangle opposite to \p v2
         * \return the index of the new triangle
         */
        index_t create_triangle(
            const double* p,
            double w,
            index_t v0, index_t v1, index_t v2,
            index_t t0, index_t t1, index_t t2
        ) {
            index_t t = create_triangle(v0, v1, v2, t0, t1, t2);
            triangle_dual(t).set_point(p);
            triangle_dual(t).set_weight(w);
            return t;
        }
        
        /**
         * \brief Creates a new triangles with specified vertices,
         *  adjacent triangles and geometric location at the dual
         *  vertex.
         * \details The vertex is copied into local storage.
         * The created triangle is marked as used.
         * \param[in] p geometric location at the dual vertex. Vertex
         *   is copied into local storage.
         * \param[in] v0 index of first vertex
         * \param[in] v1 index of second vertex
         * \param[in] v2 index of third vertex
         * \param[in] t0 index of adjacent triangle opposite to \p v0
         * \param[in] t1 index of adjacent triangle opposite to \p v1
         * \param[in] t2 index of adjacent triangle opposite to \p v2
         * \return the index of the new triangle
         */
        index_t create_triangle_copy(
            const double* p,
            index_t v0, index_t v1, index_t v2,
            index_t t0, index_t t1, index_t t2
        ) {
            index_t t = create_triangle(v0, v1, v2, t0, t1, t2);
            double* np = intersections_.new_item();
            for(coord_index_t c = 0; c < dimension(); ++c) {
                np[c] = p[c];
            }
            triangle_dual(t).set_point(np);
            return t;
        }

        /**
         * \brief Creates a new vertex.
         * \return the index of the new vertex
         */
        index_t create_vertex() {
            v_to_t_dirty_ = true;
            vertices_.push_back(Vertex());
            return vertices_.size() - 1;
        }

        /**
         * \brief Triangulates the conflict zone.
         * \details Creates the triangles radiating from \p new_v and
         * attached to the border of the conflict zone, indicated by \p t and \p e.
         * \param[in] delaunay the Delaunay triangulation
         * \param[in] i index of the first extremity of
         *     the bisector in \p delaunay.
         * \param[in] j index of the first extremity of
         *     the bisector in \p delaunay.
         * \param[in] symbolic if true, symbolic representation of the
         *    vertices is generated.
         * \param[in] t1 a triangle adjacent to the border of the conflict zone from
	 *    inside.
	 * \param[in] t1ebord the edge along which t is adjacent to the border of the conflict
	 *    zone.
         * \param[in] v_in index of the new vertex
	 * \return one of the created triangles
         */
        template <index_t DIM>
	index_t triangulate_hole(
            const Delaunay* delaunay,
            index_t i, index_t j, bool symbolic,
	    index_t t1, index_t t1ebord,
            index_t v_in
        ) {
	    index_t t = t1;
	    index_t e = t1ebord;
	    index_t t_adj = triangle_adjacent(t,e);
	    geo_debug_assert(t_adj != index_t(-1));

	    geo_debug_assert(triangle_is_conflict(t));
	    geo_debug_assert(!triangle_is_conflict(t_adj));

	    index_t new_t_first = index_t(-1);
	    index_t new_t_prev  = index_t(-1);

	    do {
		
		index_t v1 = triangle_vertex(t, plus1mod3(e));
		index_t v2 = triangle_vertex(t, minus1mod3(e));	    
		
		// Create new triangle
		index_t new_t = create_triangle(v_in, v1, v2);

                triangle_dual(new_t).intersect_geom<DIM>(
                    intersections_,
                    triangle_dual(t),
                    triangle_dual(triangle_adjacent(t, e)),
                    delaunay->vertex_ptr(i), delaunay->vertex_ptr(j)
                );
		
                if(symbolic) {
                    triangle_dual(new_t).sym().intersect_symbolic(
                        triangle_dual(t).sym(),
                        triangle_dual(triangle_adjacent(t, e)).sym(),
                        j
                    );
                }

		//   Connect new triangle to triangle on the other
		// side of the conflict zone.
		set_triangle_adjacent(new_t, 0, t_adj);
		index_t adj_e = triangle_adjacent_index(t_adj, t);
		set_triangle_adjacent(t_adj, adj_e, new_t);
		
	    
		// Move to next triangle
		e = plus1mod3(e);
		t_adj = index_t(triangle_adjacent(t,e));
		while(triangle_is_conflict(t_adj)) {
		    t = t_adj;
		    e = minus1mod3(find_triangle_vertex(t,v2));		
		    t_adj = index_t(triangle_adjacent(t,e));
		    geo_debug_assert(t_adj != index_t(-1));		
		}
		
		if(new_t_prev == index_t(-1)) {
		    new_t_first = new_t;
		} else {
		    set_triangle_adjacent(new_t_prev, 1, new_t);
		    set_triangle_adjacent(new_t, 2, new_t_prev);
		}
		
		new_t_prev = new_t;
		
	    } while((t != t1) || (e != t1ebord));

	    // Connect last triangle to first triangle
	    set_triangle_adjacent(new_t_prev, 1, new_t_first);
	    set_triangle_adjacent(new_t_first, 2, new_t_prev);
	    
	    return new_t_prev;
	}
	
        /**
         * \brief Determines the conflict zone.
         * \details The conflict zone corresponds to the set of triangles
         *   that have their dual vertices on the negative side of a
         *   bisector.
         * \param[in] mesh the input mesh
         * \param[in] delaunay the Delaunay triangulation
         * \param[in] i index of the first extremity
         *       of the bisector in \p delaunay
         * \param[in] j index of the second extremity
         *       of the bisector in \p delaunay
         * \param[in] exact if true, exact predicates are used
         * \param[out] conflict_begin
         *    index of the first triangle in conflict list
         * \param[out] conflict_end one position past index of the
         *    last triangle in conflict list
         */
        template <index_t DIM>
        void get_conflict_list(
            const Mesh* mesh, const Delaunay* delaunay,
            index_t i, index_t j, bool exact,
            index_t& conflict_begin, index_t& conflict_end
        ) {
            conflict_begin = END_OF_LIST;
            conflict_end = END_OF_LIST;
            if(exact) {
                // In exact mode, we classify each vertex
                // using the exact predicate. Note that
                // "climbing/walking" from a random vertex
                // would be more efficient, but it would require a
                // "comparison" exact predicate (with 8 different
                // versions according to the configuration of the
                // two vertices to be compared)
                for(index_t t = 0; t < max_t(); t++) {
                    if(triangle_is_used(t)) {
                        Sign s = side<DIM>(
                            mesh, delaunay,
                            triangle_dual(t),
                            i, j,
                            exact
                        );
                        if(s == GEO::NEGATIVE) {
                            append_triangle_to_conflict_list(
                                t, conflict_begin, conflict_end
                            );
                        }
                    }
                }
            } else {
                // In non-exact mode, we first detect the
                // vertex that is furthest away along the
                // normal vector of the clipping bisector,
                // and then we propagate using a flood-fill
                // algorithm. This strategy ensures that
                // the conflict zone remains connected, even
                // in the presence of numerical errors. Clearly
                // it does not ensure validity, but in practice
                // it improves resistance to degeneracies.
                index_t furthest_t =
                    find_furthest_point_linear_scan<DIM>(
                        delaunay, i, j
                    );
                propagate_conflict_list<DIM>(
                    mesh, delaunay, furthest_t,
                    i, j, exact,
                    conflict_begin, conflict_end
                );
            }
        }

        /**
         * \brief Finds the index of the vertex furthest away
         *   on the negative side of a bisector.
         * \param[in] delaunay the Delaunay triangulation
         * \param[in] i index of the first extremity of the bisector
         *    in \p delaunay
         * \param[in] j index of the second extremity of the bisector
         *    in \p delaunay
         * \return the index of the vertex furthest away on \p j%'s side,
         *   or -1 if all the vertices are on \p i%'s side.
         */
        template <index_t DIM>
        index_t find_furthest_point_linear_scan(
            const Delaunay* delaunay, index_t i, index_t j
        ) const {
            index_t result = NO_TRIANGLE;
            double furthest_dist = 0.0;
            for(index_t t = 0; t < max_t(); ++t) {
                if(triangle_is_used(t)) {
                    double d = signed_bisector_distance<DIM>(
                        delaunay, i, j, triangle_dual(t).point()
                    );
                    if(d < furthest_dist) {
                        result = t;
                        furthest_dist = d;
                    }
                }
            }
            return (furthest_dist < 0) ? result : NO_TRIANGLE;
        }

        /**
         * \brief Evaluates the equation of a bisector at a given point.
         * \details Positive side corresponds to vertex \p i and negative
         * side to vertex \p j.
         * \param[in] delaunay the Delaunay triangulation
         * \param[in] i index of the first extremity of the bisector
         *    in \p delaunay
         * \param[in] j index of the second extremity of the bisector
         *    in \p delaunay
         * \param[in] q the query point
         */
        template <index_t DIM>
        static double signed_bisector_distance(
            const Delaunay* delaunay, index_t i, index_t j, const double* q
        ) {
            const double* pi = delaunay->vertex_ptr(i);
            const double* pj = delaunay->vertex_ptr(j);
            double result = 0;
            for(coord_index_t c = 0; c < DIM; ++c) {
                result += GEO::geo_sqr(q[c] - pj[c]);
                result -= GEO::geo_sqr(q[c] - pi[c]);
            }
            return result;
        }

        /**
         * \brief Computes the conflict list by propagation from
         *   a conflict triangle.
         * \param[in] mesh the input mesh
         * \param[in] delaunay the Delaunay triangulation
         * \param[in] first_t a triangle in the conflict zone
         * \param[in] i index of the first extremity of the bisector
         *    in \p delaunay
         * \param[in] j index of the second extremity of the bisector
         *    in \p delaunay
         * \param[in] exact if true, exact predicates are used
         * \param[out] conflict_begin
         *    index of the first triangle in conflict list
         * \param[out] conflict_end one position past index of the
         *    last triangle in conflict list
         */
        template <index_t DIM>
        void propagate_conflict_list(
            const Mesh* mesh, const Delaunay* delaunay,
            index_t first_t,
            index_t i, index_t j, bool exact,
            index_t& conflict_begin, index_t& conflict_end
        ) {
            conflict_begin = END_OF_LIST;
            conflict_end = END_OF_LIST;

            // Special case, clipping plane does not clip anything
            if(first_t == NO_TRIANGLE) {
                return;
            }

            std::stack<index_t> S;
            S.push(first_t);
            append_triangle_to_conflict_list(
                first_t, conflict_begin, conflict_end
            );
            while(!S.empty()) {
                index_t t = S.top();
                S.pop();
                for(unsigned int e = 0; e < 3; e++) {
                    index_t neigh = index_t(triangle_adjacent(t, e));
                    if(!triangle_is_conflict(neigh)) {
                        if(
                            side<DIM>(
                                mesh, delaunay, triangle_dual(neigh),
                                i, j, exact
                            ) == GEO::NEGATIVE
                        ) {
                            S.push(neigh);
                            append_triangle_to_conflict_list(
                                neigh, conflict_begin, conflict_end
                            );
                        }
                    }
                }
            }
        }

        /**
         * \brief Tests on which side a vertex is relative to
         *   a bisector.
         * \param[in] mesh the input mesh
         * \param[in] delaunay the Delaunay triangulation
         * \param[in] v the query vertex
         * \param[in] i index of the first extremity of the bisector
         *    in \p delaunay
         * \param[in] j index of the second extremity of the bisector
         *    in \p delaunay
         * \param[in] exact if true, exact predicates are used
         * \return POSITIVE if \p v is on vertex \p i%'s side,
         *  NEGATIVE otherwise. ZERO is never returned since
         *  globally coherent symbolic perturbations are used
         *  in exact mode.
         * \tparam DIM dimension, specified as a template
         *  parameter for efficiency considerations.
         */
        template <index_t DIM>
        Sign side(
            const Mesh* mesh, const Delaunay* delaunay,
            const GEOGen::Vertex& v,
            index_t i, index_t j, bool exact
        ) const {
            Sign result = GEO::ZERO;
            if(exact) {
                result = side_exact(
                    mesh, delaunay, v,
                    delaunay->vertex_ptr(i),
                    delaunay->vertex_ptr(j),
                    DIM,
                    symbolic_is_surface_
                );
            } else {
                result = v.side_fast<DIM>(
                    delaunay->vertex_ptr(i),
                    delaunay->vertex_ptr(j)
                );
            }
            return result;
        }

        /**
         * \brief Tests on which side a vertex is relative to
         *   a bisector using exact predicates.
         * \param[in] mesh the input mesh
         * \param[in] delaunay the Delaunay triangulation
         * \param[in] v the query vertex
         * \param[in] pi first extremity of the bisector
         * \param[in] pj second extremity of the bisector
         * \param[in] dim dimension of the points
         * \param[in] symbolic_is_surface if true, then symbolic
         *  information is relative to a surface mesh (facets)
         *  rather than volumetric mesh (tetrahedra).
         * \return POSITIVE if \p v is on vertex \p i%'s side,
         *  NEGATIVE otherwise. ZERO is never returned since
         *  globally coherent symbolic perturbations are used
         *  in exact mode.
         * \note Only dimension=3 is implemented for now
         */
        Sign side_exact(
            const Mesh* mesh, const Delaunay* delaunay,
            const GEOGen::Vertex& v,
            const double* pi, const double* pj,
            coord_index_t dim,
            bool symbolic_is_surface = false
        ) const;

        /**
         * \brief Gets a triangle and an edge on the internal border of the conflict zone.
	 * \details The returned triangle touches the conflict zone from inside.
         * \param[in] conflict_begin first triangle of the conflict zone
         * \param[in] conflict_end one element past the last triangle of
         *   the conflict zone
         * \param[out] t a triangle in the conflict zone adjacent to the border of the
	 *   conflict zone.
	 * \param[out] e the edge along which \p t is adjacent to the border of the
	 *   conflict zone.
         * \return true if a triangle on the border was found, false otherwise.
         */
	bool find_triangle_on_border(
            index_t conflict_begin, index_t conflict_end,
	    index_t& t, index_t& e
	) const {
            GEO::geo_argused(conflict_end);
            t = conflict_begin;
            do {
                for(e = 0; e < 3; ++e) {
                    index_t nt = triangle_adjacent(t, e);
                    if(triangle_is_used(nt)) {
                        return true;
                    }
                }
                t = next_triangle(t);
            } while(t != END_OF_LIST);
            return false;
	}
	
        /**
         * \brief Gets the successor of a triangle.
         * \details Triangles are linked, for instance to represent
         *   the conflict zone.
         */
        index_t next_triangle(index_t t) const {
            geo_debug_assert(t != NO_TRIANGLE);
            geo_debug_assert(t < max_t());
            return triangles_[t].next_;
        }

        /**
         * \brief Sets the successor of a triangle.
         * \details Triangles are linked, for instance to represent
         *   the conflict zone.
         * \param[in] t index of the triangle
         * \param[in] t2 index of the successor
         */
        void set_next_triangle(index_t t, index_t t2) {
            geo_debug_assert(t != NO_TRIANGLE);
            geo_debug_assert(t < max_t());
            triangles_[t].next_ = t2;
        }

        /**
         * \brief Specify that a triangle is free.
         * \details A free triangle can be reused by subsequent
         *  triangle creations.
         */
        void mark_as_free(index_t t) {
            geo_debug_assert(t != NO_TRIANGLE);
            geo_debug_assert(t < max_t());
            triangles_[t].status_ = TRI_IS_FREE;
        }

        /**
         * \brief Specify that a triangle belongs to the conflict zone.
         */
        void mark_as_conflict(index_t t) {
            geo_debug_assert(t != NO_TRIANGLE);
            geo_debug_assert(t < max_t());
            triangles_[t].status_ = TRI_IS_CONFLICT;
        }

        /**
         * \brief Specify that a triangle is used.
         */
        void mark_as_used(index_t t) {
            geo_debug_assert(t != NO_TRIANGLE);
            geo_debug_assert(t < max_t());
            triangles_[t].status_ = TRI_IS_USED;
        }

        /**
         * \brief Appends a triangle to the conflict list.
         * \details The triangle is marked as conflict and
         *    linked to the conflict list.
         * \param[in] t the triangle
         * \param[in,out] conflict_begin first triangle in the conflict list
         * \param[in,out] conflict_end one position past the last triangle
         *   in the conflict list
         */
        void append_triangle_to_conflict_list(
            index_t t, index_t& conflict_begin, index_t& conflict_end
        ) {
            geo_debug_assert(triangle_is_used(t));
            set_next_triangle(t, conflict_begin);
            mark_as_conflict(t);
            conflict_begin = t;
            if(conflict_end == END_OF_LIST) {
                conflict_end = t;
            }
        }

        /**
         * \brief Merges a list of triangles into the free list.
         * \param[in] list_begin first triangle in the list to be freed
         * \param[in] list_end one position past the last triangle of
         *   the list to be freed
         */
        void merge_into_free_list(index_t list_begin, index_t list_end) {
            if(list_begin != END_OF_LIST) {
                geo_debug_assert(list_end != END_OF_LIST);

                index_t cur = list_begin;
                while(cur != list_end) {
                    mark_as_free(cur);
                    cur = next_triangle(cur);
                }
                mark_as_free(list_end);
                set_next_triangle(list_end, first_free_);
                first_free_ = list_begin;
            }
        }

        /**
         * \brief Allocates a new triangle.
         * \details This function is called whenever a triangle needs
         * to be created and the free list is empty.
         */
        void grow() {
            geo_debug_assert(first_free_ == END_OF_LIST);
            first_free_ = triangles_.size();
            triangles_.push_back(Triangle());
        }

        /**
         * \brief Displays the number of free,used,conflict triangles.
         * \details For debugging purposes.
         */
        std::ostream& show_stats(std::ostream& os) const;

        /**
         * \brief Computes a unique global facet id from a mesh
         *  tetrahedron and local facet index.
         * \details If a facet is shared by two tetrahedra t1 and t2,
         *  its global index determined from t1 and from t2 is the same.
         * \param[in] mesh the mesh
         * \param[in] t the index of the tetrahedron
         * \param[in] lf the local facet index (0,1,2 or 3) in tetrahedron \p t
         * \return an index that uniquely identifies the facet 
         *  in the tetrahedron
         */
        static index_t global_facet_id(
            const Mesh* mesh, index_t t, index_t lf
        ) {
            index_t t2 = mesh->cells.tet_adjacent(t, lf);
            if(t2 != GEO::NO_CELL && t2 > t) {
                index_t lf2 = mesh->cells.find_tet_adjacent(
                    t2, t
                );
                geo_debug_assert(lf2 != GEO::NO_FACET);
                return index_t(4 * t2 + lf2);
            }
            return 4 * t + lf;
        }

      private:
        GEO::vector<Triangle> triangles_;
        GEO::vector<Vertex> vertices_;
        index_t first_free_;
        bool v_to_t_dirty_;
        PointAllocator intersections_;
        bool symbolic_is_surface_;
        signed_index_t cell_id_;

        static index_t plus1mod3_[3];
        static index_t minus1mod3_[3];

        /**
         * \brief Gets the modulo-3 successor of an index.
         * \param[in] i the index
         * \return \p i plus 1 modulo 3
         */
        static index_t plus1mod3(index_t i) {
            geo_debug_assert(i < 3);
            return plus1mod3_[i];
        }

        /**
         * \brief Gets the modulo-3 predecessor of an index.
         * \param[in] i the index
         * \return \p i minus 1 modulo 3
         */
        static index_t minus1mod3(index_t i) {
            geo_debug_assert(i < 3);
            return minus1mod3_[i];
        }
    };
}

#endif

