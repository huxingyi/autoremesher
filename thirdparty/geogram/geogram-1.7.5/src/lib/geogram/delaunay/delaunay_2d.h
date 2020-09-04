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

#ifndef GEOGRAM_DELAUNAY_DELAUNAY_2D
#define GEOGRAM_DELAUNAY_DELAUNAY_2D

#include <geogram/basic/common.h>
#include <geogram/delaunay/delaunay.h>
#include <geogram/numerics/predicates.h>
#include <geogram/basic/geometry.h>

#include <stack>

/**
 * \file geogram/delaunay/delaunay_2d.h
 * \brief Implementation of Delaunay in 2d.
 */

namespace GEO {

    /**
     * \brief Implementation of Delaunay in 2d.
     * \details This package uses concepts inspired by 
     *  two triangulation softwares, CGAL and tetgen,
     *  described in the following references. This package follows the
     *  idea used in CGAL of traversing the cavity from inside, since
     *  it traverses less triangles than when traversing from outside.
     *  - Jean-Daniel Boissonnat, Olivier Devillers, Monique Teillaud, 
     *   and Mariette Yvinec. Triangulations in CGAL. 
     *   In Proc. 16th Annu. ACM Sympos. Comput. Geom., pages 11-18, 2000.
     *  - Hang Si, Constrained Delaunay trianglesl mesh generation and 
     *   refinement. Finite elements in Analysis and Design, 
     *   46 (1-2):33--46, 2010.
     *
     *  Note that the algorithm here does not support vertex deletion nor
     *  degenerate input with all coplanar or all colinear points (use CGAL
     *  instead if you have these requirements).
     *
     *  The core algorithm used in this code, CGAL and tetgen was
     *  independently and simultaneously discovered by Bowyer and Watson:
     *  - Adrian Bowyer, "Computing Dirichlet tessellations", 
     *   Comput. J., vol. 24, no 2, 1981, p. 162-166 
     *  - David F. Watson, "Computing the n-dimensional Delaunay tessellation 
     *   with application to Voronoi polytopes", Comput. J., vol. 24, 
     *   no 2, 1981, p. 167-172
     *
     *  The spatial reordering method, that dramatically increases the 
     *  performances, also used in this code, CGAL and tetgen was introduced
     *  in the following references. The second one is a smart implementation
     *  based on the std::nth_element() function of the STL, that inspired
     *  the compute_BRIO_ordering() function of this package.
     *  - Nina Amenta, Sunghee Choi and Gunter Rote, "Incremental constructions
     *   con brio", ACM Symposium on Computational Geometry 2003.
     *  - Christophe Delage and Olivier Devillers. Spatial Sorting. 
     *   In CGAL User and Reference Manual. CGAL Editorial Board, 
     *   3.9 edition, 2011
     *
     *  The locate() function is based on the following two references. 
     *  The first one randomizes the choice of the next triangle.
     *  The second one uses an inexact locate() function to initialize 
     *  the exact one (it is called "structural filtering"). The first
     *  idea is used in both CGAL and tetgen, and the second one is used
     *  in CGAL.
     *  - Walking in a triangulation, O Devillers, S Pion, M Teillaud
     *    17th Annual Symposium on Computational geometry, 106-114
     *  - Stefan Funke , Kurt Mehlhorn and Stefan Naher, "Structural filtering,
     *    a paradigm for efficient and exact geometric programs", 
     *    Comput. Geom., 1999
     */
    class GEOGRAM_API Delaunay2d : public Delaunay {
    public:
        /**
         * \brief Constructs a new Delaunay2d.
         * \param[in] dimension dimension of the triangulation (2 or 3).
         * If dimension = 3, this creates a regular triangulation
         *  (dual of a power diagram). In this case:
         *  - the input points are 3d points, were the third coordinate
         *   of point \f$ i \f$ is \f$ \sqrt{W - w_i} \f$ where \f$ W \f$ is
         *   the maximum of the  weights of all the points and \d$ w_i \$ is
         *   the weight associated with vertex \f$ i \f$.
         *  - the constructed combinatorics is a triangulated surface (2d and
         *   not 3d although dimension() returns 3). This triangulated surface
         *   corresponds to the regular triangulation of the weighted points.
         */
        Delaunay2d(coord_index_t dimension = 2);

	/**
	 * \copydoc Delaunay::set_vertices()
	 */
        virtual void set_vertices(
            index_t nb_vertices, const double* vertices
        );

	/**
	 * \copydoc Delaunay::nearest_vertex()
	 */
        virtual index_t nearest_vertex(const double* p) const;


    protected:

        /**
         * \brief Symbolic constant for uninitialized hint.
         * \details Locate functions can be accelerated by
         *  specifying a hint. This constant indicates that
         *  no hint is given.
         */
        static const index_t NO_TRIANGLE = index_t(-1);

        /**
         * \brief Finds in the pointset a set of three non-colinear
         *  points.
         * \details This function is used to initiate the incremental
         *  Delaunay construction.
         * \param[out] iv0 index of the first vertex
         * \param[out] iv1 index of the second vertex
         * \param[out] iv2 index of the third vertex
         * \retval true if a set of three non-colinear points was found
         * \retval false if all the points are colinear
         */
        bool create_first_triangle(
            index_t& iv0, index_t& iv1, index_t& iv2
        );

        /**
         * \brief Finds the triangle that contains a point.
         * \details If the point is on an edge or vertex,
         *  the function returns one of the triangles incident
         *  to that edge or vertex.
         * \param[in] p a pointer to the coordinates of the point
         * \param[in] thread_safe if true, a global spinlock is
         *  used to protect the calls to random(), this is necessary
         *  if multiple threads use locate() simultaneously
         * \param[out] orient a pointer to an array of three Sign%s
         *  or nullptr. If non-nullptr, returns the orientation with respect
         *  to the three edges of the triangle that contains \p p.
         * \return the index of a triangle that contains \p p.
         *  If the point is outside the convex hull of
         *  the inserted so-far points, then the returned triangle
         *  is a virtual one (first vertex is the "vertex at infinity"
         *  of index -1) or NO_TRIANGLE if the virtual triangles
         *  were previously removed.
         */
         index_t locate(
            const double* p, index_t hint = NO_TRIANGLE,
            bool thread_safe = false,
            Sign* orient = nullptr
         ) const;
         
        /**
         * \brief Finds the triangle that (approximately) 
         *  contains a point using inexact predicates.
         * \details The result of this function can be used as a hint
         *  for locate(). It accelerates locate as compared to calling
         *  it directly. This technique is referred to as "structural
         *  filtering".
         * \param[in] p a pointer to the coordinates of the point
         * \param[in] max_iter maximum number of traversed tets
         * \return the index of a triangle that (approximately) 
         *  contains \p p.
         *  If the point is outside the convex hull of
         *  the inserted so-far points, then the returned triangle
         *  is a virtual one (first vertex is the "vertex at infinity"
         *  of index -1) or NO_TRIANGLE if the virtual triangles
         *  were previously removed.
         */
         index_t locate_inexact(
             const double* p, index_t hint, index_t max_iter
         ) const;

        /**
         * \brief Inserts a point in the triangulation.
         * \param[in] v the index of the point to be inserted
         * \param[in] hint the index of a triangle as near as
         *  possible to \p v, or -1 if unspecified
         * \return the index of one of the triangles incident to
         *  point \p v
         */
         index_t insert(index_t v, index_t hint = NO_TRIANGLE);

        /**
         * \brief Determines the list of triangles in conflict
         *  with a given point.
         * \param[in] v the index of the point to be inserted
         * \param[in] t the index of a triangle that contains
         *  \p p, as returned by locate()
         * \param[in] orient an array of three signs indicating
         *  the orientation of \p p with respect to the three
         *  edges of \p t, as returned by locate()
         * \param[out] t_bndry a triangle adjacent to the
         *  boundary of the conflict zone
         * \param[out] e_bndry the edge along which t_bndry is
         *  adjacent to the boundary of the conflict zone
         * \param[out] first the index of the first triangle in conflict
         * \param[out] last the index of the last triangle in conflict
         *  The other triangles are linked, and can be traversed 
         *  from \p first by using triangle_next() until \p last or END_OF_LIST 
         *  is reached.
         *  The conflict zone can be empty under two circumstances:
         *  - the vertex \p v already exists in the triangulation
         *  - the triangulation is weighted and \p v is not visible
         *  in either cases, both \p first and \p last contain END_OF_LIST
         */
         void find_conflict_zone(
             index_t v, 
             index_t t, const Sign* orient,
             index_t& t_bndry, index_t& e_bndry,
             index_t& first, index_t& last
         );
         
         /**
          * \brief This function is used to implement find_conflict_zone.
          * \details This function detects the neighbors of \p t that are
          *  in the conflict zone and calls itself recursively on them.
          * \param[in] p the point to be inserted
          * \param[in] t index of a triangle in the fonflict zone
          * \param[out] t_bndry a triangle adjacent to the
          *  boundary of the conflict zone
          * \param[out] e_bndry the edge along which t_bndry is
          *  adjacent to the boundary of the conflict zone
          * \param[out] first the index of the first triangle in conflict
          * \param[out] last the index of the last triangle in conflict
          * \pre The triangle \p t was already marked as 
          *  conflict (triangle_is_in_list(t))
          */
         void find_conflict_zone_iterative(
             const double* p, index_t t,
             index_t& t_bndry, index_t& e_bndry,
             index_t& first, index_t& last
         );

         
         /**
          * \brief Creates a star of triangles filling the conflict
          *  zone.
          * \details For each triangle edge on the border of the
          *  conflict zone, a new triangle is created, resting on
          *  the edge and incident to vertex \p v. The function is 
          *  called recursively until the entire conflict zone is filled.
          * \param[in] v the index of the point to be inserted
          * \param[in] t_bndry index of a triangle on the border
          *  of the conflict zone.
          * \param[in] e_bndry index of the facet along which \p t_bndry
          *  is incident to the border of the conflict zone
          * \return the index of one the newly created triangles
          */
         index_t stellate_conflict_zone(
             index_t v, 
             index_t t_bndry, index_t e_bndry
         );
         
         // _________ Combinatorics - new and delete _________________________

         /**
         * \brief Maximum valid index for a triangle.
         * \details This includes not only real triangles,
         *  but also the virtual ones on the border, the conflict
         *  list and the free list.
         * \return the maximum valid index for a triangle.
         */
        index_t max_t() const {
            return cell_to_v_store_.size() / 3;
        }


        /**
         * \brief Default symbolic value of the cell_next_ field
         *  that indicates that a triangle is not
         *  in a linked list.
         * \details This is the default value. Note that it suffices
         *  that NOT_IN_LIST_BIT is set for a triangle
         *  to be not in any list.
         * A triangle can be:
         *  - in a list (cell_next_[t] & NOT_IN_LIST_BIT == 0)
         *  - not in a list and not marked 
         *    (cell_next_[t] & NOT_IN_LIST_BIT != 0) && 
         *    (cell_next_[t] != cur_stamp_)
         *  - not in a list and marked
         *    (cell_next_[t] == cur_stamp_)
         */
        static const index_t NOT_IN_LIST  = index_t(~0);

        /**
         * \brief If cell_next_[t] & NOT_IN_LIST_BIT != 0,
         *  then t is not in a linked list.
         * \details The other bits of cell_next_[t] are used
         *  to store the stamp (i.e. index of the current point
         *  being inserted). The stamp is used for marking triangles
         *  that were detected as non-conflict when inserting a point.
         * A triangle can be:
         *  - in a list (cell_next_[t] & NOT_IN_LIST_BIT == 0)
         *  - not in a list and not marked 
         *    (cell_next_[t] & NOT_IN_LIST_BIT != 0) && 
         *    (cell_next_[t] != cur_stamp_)
         *  - not in a list and marked
         *    (cell_next_[t] == cur_stamp_)
         */
        static const index_t NOT_IN_LIST_BIT = index_t(1u << 31);

        /**
         * \brief Symbolic value of the cell_next_ field
         *  that indicates the end of list in a linked
         *  list of triangles.
         */
        static const index_t END_OF_LIST = ~(NOT_IN_LIST_BIT);


        /**
         * \brief Tests whether a triangle belongs to a linked
         *  list.
         * \details Triangles can be linked, it is used to manage
         *  both the free list that recycles deleted triangles,
         *  the conflict region and the list of newly created
         *  triangles. In addition, a triangle that is not
         *  in a list can be marked. The same space is used for
         *  marking and chaining triangles in lists.
         *  A triangle can be in the following states:
         *  - in list
         *  - not in list and marked
         *  - not in list and not marked
         * \param[in] t the index of the triangle
         * \retval true if triangle \p t belongs to a linked list
         * \retval false otherwise
         */
        bool triangle_is_in_list(index_t t) const {
            geo_debug_assert(t < max_t());
            return (cell_next_[t] & NOT_IN_LIST_BIT) == 0;
        }

        /**
         * \brief Gets the index of a successor of a triangle.
         * \details Triangles can be linked, it is used to manage
         *  both the free list that recycles deleted triangles.
         * \param[in] t the index of the triangle
         * \retval END_OF_LIST if the end of the list is reached
         * \retval the index of the successor of
         *   triangle \t otherwise
         * \pre triangle_is_in_list(t)
         */
        index_t triangle_next(index_t t) const {
            geo_debug_assert(t < max_t());
            geo_debug_assert(triangle_is_in_list(t));
            return cell_next_[t];
        }

        /**
         * \brief Adds a triangle to a linked list.
         * \details Triangles can be linked, it is used to manage
         *  the free list that recycles deleted triangles.
         * \param[in] t the index of the triangle
         * \param[in,out] first first item of the list or END_OF_LIST if
         *  the list is empty
         * \param[in,out] last last item of the list or END_OF_LIST if
         *  the list is empty
         */
        void add_triangle_to_list(index_t t, index_t& first, index_t& last) {
            geo_debug_assert(t < max_t());
            geo_debug_assert(!triangle_is_in_list(t));
            if(last == END_OF_LIST) {
                geo_debug_assert(first == END_OF_LIST);
                first = last = t;
                cell_next_[t] = END_OF_LIST;
            } else {
                cell_next_[t] = first;
                first = t;
            }
        }

        /**
         * \brief Removes a triangle from the linked list it
         *  belongs to.
         * \details Triangles can be linked, it is used to manage
         *  both the free list that recycles deleted triangles and
         *  the list of triangles in conflict with the inserted 
         *  point.
         * \param[in] t the index of the triangle
         */
        void remove_triangle_from_list(index_t t) {
            geo_debug_assert(t < max_t());
            geo_debug_assert(triangle_is_in_list(t));
            cell_next_[t] = NOT_IN_LIST;
        }

        /**
         * \brief Symbolic value for a vertex of a
         *  triangle that indicates a virtual triangle.
         * \details The three other vertices then correspond to a
         *  facet on the convex hull of the points.
         */
        static const signed_index_t VERTEX_AT_INFINITY = -1;

        /**
         * \brief Tests whether a given triangle
         *   is a finite one.
         * \details Infinite triangles are the ones
         *   that are incident to the infinite vertex
         *   (index -1)
         * \param[in] t the index of the triangle
         * \retval true if \p t is finite
         * \retval false otherwise
         */
        bool triangle_is_finite(index_t t) const {
            return 
                cell_to_v_store_[3 * t]     >= 0 &&
                cell_to_v_store_[3 * t + 1] >= 0 &&
                cell_to_v_store_[3 * t + 2] >= 0 ;
        }
        
        /**
         * \brief Tests whether a triangle is
         *  a real one.
         * \details Real triangles are incident to
         *  three user-specified vertices (there are also
         *  virtual triangles that are incident to the
         *  vertex at infinity, with index -1)
         * \param[in] t index of the triangle
         * \retval true if triangle \p t is a real one
         * \retval false otherwise
         */
        bool triangle_is_real(index_t t) const {
            return !triangle_is_free(t) && triangle_is_finite(t);
        }

        /**
         * \brief Tests whether a triangle is
         *  a virtual one.
         * \details Virtual triangles are triangles
         *  incident to the vertex at infinity.
         * \param[in] t index of the triangle
         * \retval true if triangle \p t is virtual
         * \retval false otherwise
         */
        bool triangle_is_virtual(index_t t) const {
            return
            !triangle_is_free(t) && (
		cell_to_v_store_[3 * t] == VERTEX_AT_INFINITY ||
		cell_to_v_store_[3 * t + 1] == VERTEX_AT_INFINITY ||
		cell_to_v_store_[3 * t + 2] == VERTEX_AT_INFINITY
	    );
        }

        /**
         * \brief Tests whether a triangle is
         *  in the free list.
         * \details Deleted triangles are recycled
         *  in a free list.
         * \param[in] t index of the triangle
         * \retval true if triangle \p t is in
         * the free list
         * \retval false otherwise
         */
        bool triangle_is_free(index_t t) const {
            return triangle_is_in_list(t);
        }

        /**
         * \brief Creates a new triangle.
         * \details Uses either a triangle recycled
         *  from the free list, or creates a new one by
         *  expanding the two indices arrays.
         * \return the index of the newly created triangle
         */
        index_t new_triangle() {
            index_t result;
            if(first_free_ == END_OF_LIST) {
                cell_to_v_store_.resize(cell_to_v_store_.size() + 3, -1);
                cell_to_cell_store_.resize(cell_to_cell_store_.size() + 3, -1);
                // index_t(NOT_IN_LIST) is necessary, else with
                // NOT_IN_LIST alone the compiler tries to generate a
                // reference to NOT_IN_LIST resulting in a link error.
                cell_next_.push_back(index_t(NOT_IN_LIST));
                result = max_t() - 1;
            } else {
                result = first_free_;
                first_free_ = triangle_next(first_free_);
                remove_triangle_from_list(result);
            }

            cell_to_cell_store_[3 * result] = -1;
            cell_to_cell_store_[3 * result + 1] = -1;
            cell_to_cell_store_[3 * result + 2] = -1;

            return result;
        }

        /**
         * \brief Creates a new triangle.
         * \details Sets the vertices. Adjacent triangles index are
         *  left uninitialized. Uses either a triangle recycled
         *  from the free list, or creates a new one by
         *  expanding the two indices arrays.
         * \param[in] v1 index of the first vertex
         * \param[in] v2 index of the second vertex
         * \param[in] v3 index of the third vertex
         * \return the index of the newly created triangle
         */
        index_t new_triangle(
            signed_index_t v1, signed_index_t v2, 
            signed_index_t v3
        ) {
            index_t result = new_triangle();
            cell_to_v_store_[3 * result] = v1;
            cell_to_v_store_[3 * result + 1] = v2;
            cell_to_v_store_[3 * result + 2] = v3;
            return result;
        }

        /**
         * \brief Generates a unique stamp for marking tets.
         * \details Storage is shared for list-chaining and stamp-marking 
         * (both are mutually exclusive), therefore the stamp has
         * the NOT_IN_LIST_BIT set.
         * \param[in] stamp the unique stamp for marking tets
         */
        void set_triangle_mark_stamp(index_t stamp) {
            cur_stamp_ = (stamp | NOT_IN_LIST_BIT);
        }

        /**
         * \brief Tests whether a triangle is marked.
         * \details A triangle is marked whenever it is
         *  detected as non-conflict. The index of the
         *  point being inserted is used as a time-stamp
         *  for marking triangles. The same space is used
         *  for marking and for chaining the conflict list.
         *  A triangle can be in the following states:
         *  - in list
         *  - not in list and marked
         *  - not in list and not marked
         * \param[in] t index of the triangle
         * \retval true if triangle \p t is marked
         * \retval false otherwise
         */
        bool triangle_is_marked(index_t t) const {
            return cell_next_[t] == cur_stamp_;
        }

        /**
         * \brief Marks a triangle.
         * \details A triangle is marked whenever it is
         *  detected as non-conflict. The same space is used
         *  for marking and for chaining the conflict list.
         *  The index of the point being inserted is used as a 
         *  time-stamp for marking triangles.
         *  A triangle can be in the following states:
         *  - in list
         *  - not in list and marked
         *  - not in list and not marked
         * \param[in] t index of the triangle to be marked
         */
        void mark_triangle(index_t t) {
            cell_next_[t] = cur_stamp_;
        }

        // _________ Combinatorics ___________________________________

        /**
         * \brief Returns the local index of a vertex by 
         *   edge and by local vertex index in the edge.
         * \details
         * tri edge vertex is such that the triangle
         * formed with:
         * - vertex lv
         * - triangle_edge_vertex(lv,0)
         * - triangle_edge_vertex(lv,1)
         * has the same orientation as the original triangle for
         * any vertex lv.
         * \param[in] e local facet index, in (0,1,2)
         * \param[in] v local vertex index, in (0,1)
         * \return the local triangle vertex index of 
         *  vertex \p v in edge \p f
         */
        static index_t triangle_edge_vertex(index_t e, index_t v) {
            geo_debug_assert(e < 3);
            geo_debug_assert(v < 2);
            return index_t(triangle_edge_vertex_[e][v]);
        }

        /**
         * \brief Gets the index of a vertex of a triangle
         * \param[in] t index of the triangle
         * \param[in] lv local vertex (0,1,2) index in \p t
         * \return the global index of the \p lv%th vertex of triangle \p t
         *  or -1 if the vertex is at infinity
         */
        signed_index_t triangle_vertex(index_t t, index_t lv) const {
            geo_debug_assert(t < max_t());
            geo_debug_assert(lv < 3);
            return cell_to_v_store_[3 * t + lv];
        }

        /**
         * \brief Finds the index of the vertex in a triangle.
         * \param[in] t the triangle
         * \param[in] v the vertex
         * \return iv such that triangle_vertex(t,v)==iv
         * \pre \p t is incident to \p v
         */
        index_t find_triangle_vertex(index_t t, signed_index_t v) const {
            geo_debug_assert(t < max_t());
            //   Find local index of v in triangle t vertices.
            const signed_index_t* T = &(cell_to_v_store_[3 * t]);
            return find_3(T,v);
        }


        /**
         * \brief Gets the index of a vertex of a triangle
         * \param[in] t index of the triangle
         * \param[in] lv local vertex (0,1,2) index in \p t
         * \return the global index of the \p lv%th vertex of triangle \p t
         * \pre Vertex \p lv of triangle \p t is not at infinity
         */
         index_t finite_triangle_vertex(index_t t, index_t lv) const {
            geo_debug_assert(t < max_t());
            geo_debug_assert(lv < 3);
            geo_debug_assert(cell_to_v_store_[3 * t + lv] != -1);
            return index_t(cell_to_v_store_[3 * t + lv]);
        }

        /**
         * \brief Sets a triangle-to-vertex adjacency.
         * \param[in] t index of the triangle
         * \param[in] lv local vertex index (0,1,2) in \p t
         * \param[in] v global index of the vertex
         */
        void set_triangle_vertex(index_t t, index_t lv, signed_index_t v) {
            geo_debug_assert(t < max_t());
            geo_debug_assert(lv < 3);
            cell_to_v_store_[3 * t + lv] = v;
        }

        /**
         * \brief Gets the index of a triangle adjacent to another one.
         * \param[in] t index of the triangle
         * \param[in] le local edge (0,1,2) index in \p t
         * \return the triangle adjacent to \p t accros edge \p le
         */
        signed_index_t triangle_adjacent(index_t t, index_t le) const {
            geo_debug_assert(t < max_t());
            geo_debug_assert(le < 3);
            signed_index_t result = cell_to_cell_store_[3 * t + le];
            return result;
        }

        /**
         * \brief Sets a triangle-to-triangle adjacency.
         * \param[in] t1 index of the first triangle
         * \param[in] le1 local facet index (0,1,2) in t1
         * \param[in] t2 index of the triangle
         *  adjacent to \p t1 accros \p lf1
         */
        void set_triangle_adjacent(index_t t1, index_t le1, index_t t2) {
            geo_debug_assert(t1 < max_t());
            geo_debug_assert(t2 < max_t());
            geo_debug_assert(le1 < 3);
	    geo_debug_assert(t1 != t2);
            cell_to_cell_store_[3 * t1 + le1] = signed_index_t(t2);
        }
        
        /**
         * \brief Finds the index of the edge accros which t1 is 
         *  adjacent to t2_in.
         * \param[in] t1 first triangle
         * \param[in] t2_in second triangle
         * \return e such that triangle_adjacent(t1,e)==t2_in
         * \pre \p t1 and \p t2_in are adjacent
         */
        index_t find_triangle_adjacent(
            index_t t1, index_t t2_in
        ) const {
            geo_debug_assert(t1 < max_t());
            geo_debug_assert(t2_in < max_t());
            geo_debug_assert(t1 != t2_in);

            signed_index_t t2 = signed_index_t(t2_in);

            // Find local index of t2 in triangle t1 adajcent tets.
            const signed_index_t* T = &(cell_to_cell_store_[3 * t1]);
            index_t result = find_3(T,t2);

            // Sanity check: make sure that t1 is adjacent to t2
            // only once!
            geo_debug_assert(triangle_adjacent(t1,(result+1)%3) != t2);
            geo_debug_assert(triangle_adjacent(t1,(result+2)%3) != t2);
            return result;
        }



        /**
         * \brief Sets the vertices and adjacent triangles of
         *  a triangle.
         * \param[in] t index of the triangle
         * \param[in] v0 index of the first vertex
         * \param[in] v1 index of the second vertex
         * \param[in] v2 index of the third vertex
         * \param[in] a0 index of the adjacent triangle opposite to \p v0
         * \param[in] a1 index of the adjacent triangle opposite to \p v1
         * \param[in] a2 index of the adjacent triangle opposite to \p v2
         */
        void set_tet(
            index_t t,
            signed_index_t v0, signed_index_t v1,
            signed_index_t v2, 
            index_t a0, index_t a1, index_t a2
        ) {
            geo_debug_assert(t < max_t());
            cell_to_v_store_[3 * t] = v0;
            cell_to_v_store_[3 * t + 1] = v1;
            cell_to_v_store_[3 * t + 2] = v2;
            cell_to_cell_store_[3 * t] = signed_index_t(a0);
            cell_to_cell_store_[3 * t + 1] = signed_index_t(a1);
            cell_to_cell_store_[3 * t + 2] = signed_index_t(a2);
        }

        // _________ Predicates _____________________________________________

        /**
         * \brief Tests whether a given triangle is in conflict with
         *  a given 3d point.
         * \details A real triangle is in conflict with a point whenever
         *  the point is contained by its circumscribed sphere, and a
         *  virtual triangle is in conflict with a point whenever the
         *  triangle formed by its real face and with the point has
         *  positive orientation.
         * \param[in] t the index of the triangle
         * \param[in] p a pointer to the coordinates of the point
         * \retval true if point \p p is in conflict with triangle \p t
         * \retval false otherwise
         */
        bool triangle_is_conflict(index_t t, const double* p) const {

            // Lookup triangle vertices
            const double* pv[3];
            for(index_t i=0; i<3; ++i) {
                signed_index_t v = triangle_vertex(t,i);
                pv[i] = (v == -1) ? nullptr : vertex_ptr(index_t(v));
            }

            // Check for virtual triangles (then in_circle()
            // is replaced with orient2d())
            for(index_t le = 0; le < 3; ++le) {

                if(pv[le] == nullptr) {

                    // Facet of a virtual triangle opposite to
                    // infinite vertex corresponds to
                    // the triangle on the convex hull of the points.
                    // Orientation is obtained by replacing vertex lf
                    // with p.
                    pv[le] = p;
                    Sign sign = PCK::orient_2d(pv[0],pv[1],pv[2]);

                    if(sign > 0) {
                        return true;
                    }

                    if(sign < 0) {
                        return false;
                    }

                    // If sign is zero, we check the real triangle
                    // adjacent to the facet on the convex hull.
                    geo_debug_assert(triangle_adjacent(t, le) >= 0);
                    index_t t2 = index_t(triangle_adjacent(t, le));
                    geo_debug_assert(!triangle_is_virtual(t2));

                    //  If t2 is already chained in the conflict list,
                    // then it is conflict
                    if(triangle_is_in_list(t2)) {
                        return true;
                    }

                    //  If t2 is marked, then it is not in conflict.
                    if(triangle_is_marked(t2)) {
                        return false;
                    }

                    return triangle_is_conflict(t2, p);
                }
            }

            //   If the triangle is a finite one, it is in conflict
            // if its circumscribed sphere contains the point (this is
            // the standard case).

            if(weighted_) {
                double h0 = heights_[finite_triangle_vertex(t, 0)];
                double h1 = heights_[finite_triangle_vertex(t, 1)];
                double h2 = heights_[finite_triangle_vertex(t, 2)];
                index_t pindex = index_t(
                    (p - vertex_ptr(0)) / int(vertex_stride_)
                );
                double h = heights_[pindex];
                return (PCK::orient_2dlifted_SOS(
                            pv[0],pv[1],pv[2],p,h0,h1,h2,h
                       ) > 0) ;
            }

            return (PCK::in_circle_2d_SOS(pv[0], pv[1], pv[2], p) > 0);
        }

    protected:

        /**
         * \brief Finds the index of an integer in an array of three integers.
         * \param[in] T a const pointer to an array of three integers
         * \param[in] v the integer to retrieve in \p T
         * \return the index (0,1 or 2) of \p v in \p T
         * \pre The three entries of \p T are different and one of them is
         *  equal to \p v.
         */
        static index_t find_3(const signed_index_t* T, signed_index_t v) {
            // The following expression is 10% faster than using
            // if() statements. This uses the C++ norm, that 
            // ensures that the 'true' boolean value converted to 
            // an int is always 1. With most compilers, this avoids 
            // generating branching instructions.
            // Thank to Laurent Alonso for this idea.
            index_t result = index_t( (T[1] == v) | ((T[2] == v) * 2) );
            // Sanity check, important if it was T[0], not explicitly
            // tested (detects input that does not meet the precondition).
            geo_debug_assert(T[result] == v);
            return result; 
        }

        /**
         * \brief Delaunay2d destructor
         */
        virtual ~Delaunay2d();

        /**
         * \brief For debugging purposes, displays a triangle.
         * \param[in] t index of the triangle to display.
         */
        void show_triangle(index_t t) const;

        /**
         * \brief For debugging purposes, displays a triangle adjacency.
         * \param[in] t index of the triangle to display.
         * \param[in] le local index (0,1,2) of the triangle
         *  facet adjacenty to display.
         */
        void show_triangle_adjacent(index_t t, index_t le) const;

        /**
         * \brief For debugging purposes, displays a triangle.
         * \param[in] first index of the first triangle in the list
         * \param[in] list_name name of the list, will be displayed as well
         */
        void show_list(
            index_t first, const std::string& list_name
        ) const;

        /**
         * \brief For debugging purposes, tests some combinatorial properties.
         */
        void check_combinatorics(bool verbose = false) const;

        /**
         * \brief For debugging purposes, test some geometrical properties.
         */
        void check_geometry(bool verbose = false) const;

    private:
        vector<signed_index_t> cell_to_v_store_;
        vector<signed_index_t> cell_to_cell_store_;
        vector<index_t> cell_next_;
        vector<index_t> reorder_;
        index_t cur_stamp_; // used for marking
        index_t first_free_;
        bool weighted_;
        vector<double> heights_; // only used in weighted mode

        /**
         * Performs additional checks (costly !)
         */
         bool debug_mode_;

        /**
         * Displays the result of the additional checks.
         */
         bool verbose_debug_mode_;

        /**
         * Displays the timing of the core algorithm.
         */
         bool benchmark_mode_;

        /**
         * \brief Gives the indexing of triangle edge
         *  vertices.
         * \details triangle_edge_vertex[le][lv] gives the
         *  local vertex index (in 0,1,2) from a
         *  local edge index le (in 0,1,2) and a
         *  local vertex index within the edge (in 0,1).
         */
	 static char triangle_edge_vertex_[3][2];

	 /**
	  * \brief Used by find_conflict_zone_iterative()
	  */
	 std::stack<index_t> S_;
    };

    /************************************************************************/

    /**
     * \brief Regular Delaunay triangulation of weighted points
     * \details
     * - the input points are 2d points, were the third coordinate
     *  of point \f$ i \f$ is \f$ \sqrt{W - w_i} \f$ where \f$ W \f$ is
     *  the maximum of the  weights of all the points and \d$ w_i \$ is
     *  the weight associated with vertex \f$ i \f$.
     * - the constructed combinatorics is a triangulated surface (2d and
     *  not 3d although dimension() returns 3). This triangulated surface
     *  corresponds to the regular triangulation of the weighted points.
     */
    class GEOGRAM_API RegularWeightedDelaunay2d : public Delaunay2d {
    public:
        /**
         * \brief Constructs a new Regular Delaunay2d triangulation.
         * \details RegularWeightedDelaunay2d triangulations are only
         * supported for dimension 3. If a different dimension is specified in
         * the constructor, a InvalidDimension exception is thrown.
         * \param[in] dimension dimension of the triangulation
         * \throw InvalidDimension This exception is thrown if dimension is
         * different from 3.
         */
        RegularWeightedDelaunay2d(coord_index_t dimension = 3);

    protected:
        /**
         * \brief RegularWeightedDelaunay2d destructor
         */
        virtual ~RegularWeightedDelaunay2d();
    };
}

#endif

