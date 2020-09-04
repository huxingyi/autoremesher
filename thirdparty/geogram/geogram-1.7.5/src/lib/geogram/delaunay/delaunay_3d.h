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

#ifndef GEOGRAM_DELAUNAY_DELAUNAY_3D
#define GEOGRAM_DELAUNAY_DELAUNAY_3D

#include <geogram/basic/common.h>
#include <geogram/delaunay/delaunay.h>
#include <geogram/delaunay/cavity.h>
#include <geogram/numerics/predicates.h>
#include <geogram/basic/geometry.h>

#include <stack>

/**
 * \file geogram/delaunay/delaunay_3d.h
 * \brief Implementation of Delaunay in 3d.
 */

namespace GEO {

    /**
     * \brief Implementation of Delaunay in 3d.
     * \details This package uses concepts inspired by 
     *  two triangulation softwares, CGAL and tetgen,
     *  described in the following references. This package follows the
     *  idea used in CGAL of traversing the cavity from inside, since
     *  it traverses less tetrahedra than when traversing from outside.
     *  - Jean-Daniel Boissonnat, Olivier Devillers, Monique Teillaud, 
     *   and Mariette Yvinec. Triangulations in CGAL. 
     *   In Proc. 16th Annu. ACM Sympos. Comput. Geom., pages 11-18, 2000.
     *  - Hang Si, Constrained Delaunay tetrahedral mesh generation and 
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
     *  The first one randomizes the choice of the next tetrahedron.
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
    class GEOGRAM_API Delaunay3d : public Delaunay {
    public:
        /**
         * \brief Constructs a new Delaunay3d.
         * \param[in] dimension dimension of the triangulation (3 or 4).
         * If dimension = 4, this creates a regular triangulation
         *  (dual of a power diagram). In this case:
         *  - the input points are 4d points, were the fourth coordinate
         *   of point \f$ i \f$ is \f$ \sqrt{W - w_i} \f$ where \f$ W \f$ is
         *   the maximum of the  weights of all the points and \d$ w_i \$ is
         *   the weight associated with vertex \f$ i \f$.
         *  - the constructed combinatorics is a tetrahedralized volume (3d and
         *   not 4d although dimension() returns 4). This tetrahedralized volume
         *   corresponds to the regular triangulation of the weighted points.
         */
        Delaunay3d(coord_index_t dimension = 3);

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
        static const index_t NO_TETRAHEDRON = index_t(-1);

        /**
         * \brief Finds in the pointset a set of four non-coplanar
         *  points.
         * \details This function is used to initiate the incremental
         *  Delaunay construction.
         * \param[out] iv0 index of the first vertex
         * \param[out] iv1 index of the second vertex
         * \param[out] iv2 index of the third vertex
         * \param[out] iv3 index of the fourth vertex
         * \retval true if a set of four non-coplanar points was found
         * \retval false if all the points are coplanar
         */
        bool create_first_tetrahedron(
            index_t& iv0, index_t& iv1, index_t& iv2, index_t& iv3
        );

        /**
         * \brief Finds the tetrahedron that contains a point.
         * \details If the point is on a face, edge or vertex,
         *  the function returns one of the tetrahedra incident
         *  to that face, edge or vertex.
         * \param[in] p a pointer to the coordinates of the point
         * \param[in] thread_safe if true, a global spinlock is
         *  used to protect the calls to random(), this is necessary
         *  if multiple threads use locate() simultaneously
         * \param[out] orient a pointer to an array of four Sign%s
         *  or nullptr. If non-nullptr, returns the orientation with respect
         *  to the four facets of the tetrahedron that contains \p p.
         * \return the index of a tetrahedron that contains \p p.
         *  If the point is outside the convex hull of
         *  the inserted so-far points, then the returned tetrahedron
         *  is a virtual one (first vertex is the "vertex at infinity"
         *  of index -1) or NO_TETRAHEDRON if the virtual tetrahedra 
         *  were previously removed.
         */
         index_t locate(
            const double* p, index_t hint = NO_TETRAHEDRON,
            bool thread_safe = false,
            Sign* orient = nullptr
         ) const;
         
        /**
         * \brief Finds the tetrahedron that (approximately) 
         *  contains a point using inexact predicates.
         * \details The result of this function can be used as a hint
         *  for locate(). It accelerates locate as compared to calling
         *  it directly. This technique is referred to as "structural
         *  filtering".
         * \param[in] p a pointer to the coordinates of the point
         * \param[in] max_iter maximum number of traversed tets
         * \return the index of a tetrahedron that (approximately) 
         *  contains \p p.
         *  If the point is outside the convex hull of
         *  the inserted so-far points, then the returned tetrahedron
         *  is a virtual one (first vertex is the "vertex at infinity"
         *  of index -1) or NO_TETRAHEDRON if the virtual tetrahedra 
         *  were previously removed.
         */
         index_t locate_inexact(
             const double* p, index_t hint, index_t max_iter
         ) const;

        /**
         * \brief Inserts a point in the triangulation.
         * \param[in] v the index of the point to be inserted
         * \param[in] hint the index of a tetrahedron as near as
         *  possible to \p v, or -1 if unspecified
         * \return the index of one of the tetrahedra incident to
         *  point \p v
         */
         index_t insert(index_t v, index_t hint = NO_TETRAHEDRON);

        /**
         * \brief Determines the list of tetrahedra in conflict
         *  with a given point.
         * \param[in] v the index of the point to be inserted
         * \param[in] t the index of a tetrahedron that contains
         *  \p p, as returned by locate()
         * \param[in] orient an array of four signs indicating
         *  the orientation of \p p with respect to the four
         *  faces of \p t, as returned by locate()
         * \param[out] t_bndry a tetrahedron adjacent to the
         *  boundary of the conflict zone
         * \param[out] f_bndry the facet along which t_bndry is
         *  adjacent to the boundary of the conflict zone
         * \param[out] first the index of the first tetrahedron in conflict
         * \param[out] last the index of the last tetrahedron in conflict
         *  The other tetrahedra are linked, and can be traversed 
         *  from \p first by using tet_next() until \p last or END_OF_LIST 
         *  is reached.
         *  The conflict zone can be empty under two circumstances:
         *  - the vertex \p v already exists in the triangulation
         *  - the triangulation is weighted and \p v is not visible
         *  in either cases, both \p first and \p last contain END_OF_LIST
         */
         void find_conflict_zone(
             index_t v, 
             index_t t, const Sign* orient,
             index_t& t_bndry, index_t& f_bndry,
             index_t& first, index_t& last
         );
         
         /**
          * \brief This function is used to implement find_conflict_zone.
          * \details This function detects the neighbors of \p t that are
          *  in the conflict zone and calls itself recursively on them.
          * \param[in] p the point to be inserted
          * \param[in] t index of a tetrahedron in the fonflict zone
          * \param[out] t_bndry a tetrahedron adjacent to the
          *  boundary of the conflict zone
          * \param[out] f_bndry the facet along which t_bndry is
          *  adjacent to the boundary of the conflict zone
          * \param[out] first the index of the first tetrahedron in conflict
          * \param[out] last the index of the last tetrahedron in conflict
          * \pre The tetrahedron \p t was alredy marked as 
          *  conflict (tet_is_in_list(t))
          */
         void find_conflict_zone_iterative(
             const double* p, index_t t,
             index_t& t_bndry, index_t& f_bndry,
             index_t& first, index_t& last
         );

	 /**
	  * \brief Creates a star of tetrahedra filling the conflict 
	  *  zone.
          * \param[in] v the index of the point to be inserted
	  * \details This function is used when the Cavity computed 
	  *  when traversing the conflict zone is OK, that is to say
	  *  when its array sizes were not exceeded.
          * \return the index of one the newly created tetrahedron
	  */
	 index_t stellate_cavity(index_t v);
	 
         
         /**
          * \brief Creates a star of tetrahedra filling the conflict
          *  zone.
          * \details For each tetrahedron facet on the border of the
          *  conflict zone, a new tetrahedron is created, resting on
          *  the facet and incident to vertex \p v. The function is 
          *  called recursively until the entire conflict zone is filled.
          * \param[in] v the index of the point to be inserted
          * \param[in] t_bndry index of a tetrahedron on the border
          *  of the conflict zone.
          * \param[in] f_bndry index of the facet along which \p t_bndry
          *  is incident to the border of the conflict zone
          * \param[in] prev_f the facet of \p t_bndry connected to the
          *  tetrahedron that \p t_bndry was reached from, or index_t(-1)
          *  if it is the first tetrahedron.
          * \return the index of one the newly created tetrahedron
          */
         index_t stellate_conflict_zone_iterative(
             index_t v, 
             index_t t_bndry, index_t f_bndry, 
             index_t prev_f=index_t(-1)
         );
         
         /**
          * \brief Finds the neighbor of a tetrahedron on the border of the 
          *  conflict zone.
          * \details This function is used by stellate_conflict_zone_iterative()
          * \param[in] t1 a tetrahedron on the border of the conflict zone
          * \param[in] t1fborder the local facet index of \p t1 along which it
          *  is on the border of the conflict zone
          * \param[in] t1ft2 the local facet index of \p t1 that will be 
          *  traversed
          * \param[out] t2 a tetrahedron on the border of the conflict zone,
          *  with an edge common to facets \p t1fborder and \p t1ft2 of 
          *  tetrahedron \p t1
          * \param[out] t2fborder the local facet index of \p t2 along which it
          *  is on the border of the conflict zone
          * \param[out] t2ft1 the local index of the facet of \p t2 that has a
          *  common edge with facets \p t1fborder and \p t1ft2 of tetrahedron
          *  \p t1
          * \retval true if \p t2 is a newly created tetrahedron
          * \retval false if \p t2 is an old tetrahedron in conflict
          */
         bool get_neighbor_along_conflict_zone_border(
             index_t t1,
             index_t t1fborder,
             index_t t1ft2,
             index_t& t2,
             index_t& t2fborder,
             index_t& t2ft1
         ) const {
             
             // Note: this function is a bit long for an inline function,
             // but I observed a (modest) performance gain doing so.
             
             //   Find two vertices that are both on facets new_f and f1
             //  (the edge around which we are turning)
             //  This uses duality as follows:
             //  Primal form (not used here): 
             //    halfedge_facet_[v1][v2] returns a facet that is incident
             //    to both v1 and v2.
             //  Dual form (used here):
             //    halfedge_facet_[f1][f2] returns a vertex that both 
             //    f1 and f2 are incident to.
             signed_index_t ev1 = 
                 tet_vertex(t1, index_t(halfedge_facet_[t1ft2][t1fborder]));
             signed_index_t ev2 = 
                 tet_vertex(t1, index_t(halfedge_facet_[t1fborder][t1ft2]));

             //   Turn around edge [ev1,ev2] inside the conflict zone
             // until we reach again the boundary of the conflict zone.
             // Traversing inside the conflict zone is faster (as compared
             // to outside) since it traverses a smaller number of tets.
             index_t cur_t = t1;
             index_t cur_f = t1ft2;
             index_t next_t = index_t(tet_adjacent(cur_t,cur_f));
             while(tet_is_in_list(next_t)) {
                 geo_debug_assert(next_t != t1);
                 cur_t = next_t;
                 cur_f = get_facet_by_halfedge(cur_t,ev1,ev2);
                 next_t = index_t(tet_adjacent(cur_t, cur_f));
             }
             
             //  At this point, cur_t is in conflict zone and
             // next_t is outside the conflict zone.
             index_t f12,f21;
             get_facets_by_halfedge(next_t, ev1, ev2, f12, f21);
             t2 = index_t(tet_adjacent(next_t,f21));
             signed_index_t v_neigh_opposite = tet_vertex(next_t,f12);
             t2ft1 = find_tet_vertex(t2, v_neigh_opposite);
             t2fborder = cur_f;
        
             //  Test whether the found neighboring tet was created
             //  (then return true) or is an old tet in conflict
             //  (then return false).
             return(t2 != cur_t);
         }
         
         // _________ Combinatorics - new and delete _________________________

         /**
         * \brief Maximum valid index for a tetrahedron.
         * \details This includes not only real tetrahedra,
         *  but also the virtual ones on the border, the conflict
         *  list and the free list.
         * \return the maximum valid index for a tetrahedron
         */
        index_t max_t() const {
            return cell_to_v_store_.size() / 4;
        }


        /**
         * \brief Default symbolic value of the cell_next_ field
         *  that indicates that a tetrahedron is not
         *  in a linked list.
         * \details This is the default value. Note that it suffices
         *  that NOT_IN_LIST_BIT is set for a tetrahedron
         *  to be not in any list.
         * A tetrahedron can be:
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
         *  being inserted). The stamp is used for marking tetrahedra
         *  that were detected as non-conflict when inserting a point.
         * A tetrahedron can be:
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
         *  list of tetrahedra.
         */
        static const index_t END_OF_LIST = ~(NOT_IN_LIST_BIT);


        /**
         * \brief Tests whether a tetrahedron belongs to a linked
         *  list.
         * \details Tetrahedra can be linked, it is used to manage
         *  both the free list that recycles deleted tetrahedra,
         *  the conflict region and the list of newly created
         *  tetrahedra. In addition, a tetrahedron that is not
         *  in a list can be marked. The same space is used for
         *  marking and chaining tetrahedra in lists.
         *  A tetrahedron can be in the following states:
         *  - in list
         *  - not in list and marked
         *  - not in list and not marked
         * \param[in] t the index of the tetrahedron
         * \retval true if tetrahedron \p t belongs to a linked list
         * \retval false otherwise
         */
        bool tet_is_in_list(index_t t) const {
            geo_debug_assert(t < max_t());
            return (cell_next_[t] & NOT_IN_LIST_BIT) == 0;
        }

        /**
         * \brief Gets the index of a successor of a tetrahedron.
         * \details Tetrahedra can be linked, it is used to manage
         *  both the free list that recycles deleted tetrahedra.
         * \param[in] t the index of the tetrahedron
         * \retval END_OF_LIST if the end of the list is reached
         * \retval the index of the successor of
         *   tetrahedron \t otherwise
         * \pre tet_is_in_list(t)
         */
        index_t tet_next(index_t t) const {
            geo_debug_assert(t < max_t());
            geo_debug_assert(tet_is_in_list(t));
            return cell_next_[t];
        }

        /**
         * \brief Adds a tetrahedron to a linked list.
         * \details Tetrahedra can be linked, it is used to manage
         *  the free list that recycles deleted tetrahedra.
         * \param[in] t the index of the tetrahedron
         * \param[in,out] first first item of the list or END_OF_LIST if
         *  the list is empty
         * \param[in,out] last last item of the list or END_OF_LIST if
         *  the list is empty
         */
        void add_tet_to_list(index_t t, index_t& first, index_t& last) {
            geo_debug_assert(t < max_t());
            geo_debug_assert(!tet_is_in_list(t));
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
         * \brief Removes a tetrahedron from the linked list it
         *  belongs to.
         * \details Tetrahedra can be linked, it is used to manage
         *  both the free list that recycles deleted tetrahedra and
         *  the list of tetrahedra in conflict with the inserted 
         *  point.
         * \param[in] t the index of the tetrahedron
         */
        void remove_tet_from_list(index_t t) {
            geo_debug_assert(t < max_t());
            geo_debug_assert(tet_is_in_list(t));
            cell_next_[t] = NOT_IN_LIST;
        }

        /**
         * \brief Symbolic value for a vertex of a
         *  tetrahedron that indicates a virtual tetrahedron.
         * \details The three other vertices then correspond to a
         *  facet on the convex hull of the points.
         */
        static const signed_index_t VERTEX_AT_INFINITY = -1;

        /**
         * \brief Tests whether a given tetrahedron
         *   is a finite one.
         * \details Infinite tetrahedra are the ones
         *   that are incident to the infinite vertex
         *   (index -1)
         * \param[in] t the index of the tetrahedron
         * \retval true if \p t is finite
         * \retval false otherwise
         */
        bool tet_is_finite(index_t t) const {
            return 
                cell_to_v_store_[4 * t]     >= 0 &&
                cell_to_v_store_[4 * t + 1] >= 0 &&
                cell_to_v_store_[4 * t + 2] >= 0 &&
                cell_to_v_store_[4 * t + 3] >= 0;
        }
        
        /**
         * \brief Tests whether a tetrahedron is
         *  a real one.
         * \details Real tetrahedra are incident to
         *  four user-specified vertices (there are also
         *  virtual tetrahedra that are incident to the
         *  vertex at infinity, with index -1)
         * \param[in] t index of the tetrahedron
         * \retval true if tetrahedron \p t is a real one
         * \retval false otherwise
         */
        bool tet_is_real(index_t t) const {
            return !tet_is_free(t) && tet_is_finite(t);
        }

        /**
         * \brief Tests whether a tetrahedron is
         *  a virtual one.
         * \details Virtual tetrahedra are tetrahedra
         *  incident to the vertex at infinity.
         * \param[in] t index of the tetrahedron
         * \retval true if tetrahedron \p t is virtual
         * \retval false otherwise
         */
        bool tet_is_virtual(index_t t) const {
            return
                !tet_is_free(t) && (
                cell_to_v_store_[4 * t] == VERTEX_AT_INFINITY ||
                cell_to_v_store_[4 * t + 1] == VERTEX_AT_INFINITY ||
                cell_to_v_store_[4 * t + 2] == VERTEX_AT_INFINITY ||
                cell_to_v_store_[4 * t + 3] == VERTEX_AT_INFINITY) ;
        }

        /**
         * \brief Tests whether a tetrahedron is
         *  in the free list.
         * \details Deleted tetrahedra are recycled
         *  in a free list.
         * \param[in] t index of the tetrahedron
         * \retval true if tetrahedron \p t is in
         * the free list
         * \retval false otherwise
         */
        bool tet_is_free(index_t t) const {
            return tet_is_in_list(t);
        }

        /**
         * \brief Creates a new tetrahedron.
         * \details Uses either a tetrahedron recycled
         *  from the free list, or creates a new one by
         *  expanding the two indices arrays.
         * \return the index of the newly created tetrahedron
         */
        index_t new_tetrahedron() {
            index_t result;
            if(first_free_ == END_OF_LIST) {
                cell_to_v_store_.resize(cell_to_v_store_.size() + 4, -1);
                cell_to_cell_store_.resize(cell_to_cell_store_.size() + 4, -1);
                // index_t(NOT_IN_LIST) is necessary, else with
                // NOT_IN_LIST alone the compiler tries to generate a
                // reference to NOT_IN_LIST resulting in a link error.
                cell_next_.push_back(index_t(NOT_IN_LIST));
                result = max_t() - 1;
            } else {
                result = first_free_;
                first_free_ = tet_next(first_free_);
                remove_tet_from_list(result);
            }

            cell_to_cell_store_[4 * result] = -1;
            cell_to_cell_store_[4 * result + 1] = -1;
            cell_to_cell_store_[4 * result + 2] = -1;
            cell_to_cell_store_[4 * result + 3] = -1;

            return result;
        }

        /**
         * \brief Creates a new tetrahedron.
         * \details Sets the vertices. Adjacent tetrahedra index are
         *  left uninitialized. Uses either a tetrahedron recycled
         *  from the free list, or creates a new one by
         *  expanding the two indices arrays.
         * \param[in] v1 index of the first vertex
         * \param[in] v2 index of the second vertex
         * \param[in] v3 index of the third vertex
         * \param[in] v4 index of the fourth vertex
         * \return the index of the newly created tetrahedron
         */
        index_t new_tetrahedron(
            signed_index_t v1, signed_index_t v2, 
            signed_index_t v3, signed_index_t v4
        ) {
            index_t result = new_tetrahedron();
            cell_to_v_store_[4 * result] = v1;
            cell_to_v_store_[4 * result + 1] = v2;
            cell_to_v_store_[4 * result + 2] = v3;
            cell_to_v_store_[4 * result + 3] = v4;
            return result;
        }

        /**
         * \brief Generates a unique stamp for marking tets.
         * \details Storage is shared for list-chaining and stamp-marking 
         * (both are mutually exclusive), therefore the stamp has
         * the NOT_IN_LIST_BIT set.
         * \param[in] stamp the unique stamp for marking tets
         */
        void set_tet_mark_stamp(index_t stamp) {
            cur_stamp_ = (stamp | NOT_IN_LIST_BIT);
        }

        /**
         * \brief Tests whether a tetrahedron is marked.
         * \details A tetrahedron is marked whenever it is
         *  detected as non-conflict. The index of the
         *  point being inserted is used as a time-stamp
         *  for marking tetrahedra. The same space is used
         *  for marking and for chaining the conflict list.
         *  A tetrahedron can be in the following states:
         *  - in list
         *  - not in list and marked
         *  - not in list and not marked
         * \param[in] t index of the tetrahedron
         * \retval true if tetrahedron \p t is marked
         * \retval false otherwise
         */
        bool tet_is_marked(index_t t) const {
            return cell_next_[t] == cur_stamp_;
        }

        /**
         * \brief Marks a tetrahedron.
         * \details A tetrahedron is marked whenever it is
         *  detected as non-conflict. The same space is used
         *  for marking and for chaining the conflict list.
         *  The index of the point being inserted is used as a 
         *  time-stamp for marking tetrahedra.
         *  A tetrahedron can be in the following states:
         *  - in list
         *  - not in list and marked
         *  - not in list and not marked
         * \param[in] t index of the tetrahedron to be marked
         */
        void mark_tet(index_t t) {
            cell_next_[t] = cur_stamp_;
        }

        // _________ Combinatorics ___________________________________

        /**
         * \brief Returns the local index of a vertex by 
         *   facet and by local vertex index in the facet.
         * \details
         * tet facet vertex is such that the tetrahedron
         * formed with:
         * - vertex lv
         * - tet_facet_vertex(lv,0)
         * - tet_facet_vertex(lv,1)
         * - tet_facet_vertex(lv,2)
         * has the same orientation as the original tetrahedron for
         * any vertex lv.
         * \param[in] f local facet index, in (0,1,2,3)
         * \param[in] v local vertex index, in (0,1,2)
         * \return the local tetrahedron vertex index of 
         *  vertex \p v in facet \p f
         */
        static index_t tet_facet_vertex(index_t f, index_t v) {
            geo_debug_assert(f < 4);
            geo_debug_assert(v < 3);
            return index_t(tet_facet_vertex_[f][v]);
        }

        /**
         * \brief Gets the index of a vertex of a tetrahedron
         * \param[in] t index of the tetrahedron
         * \param[in] lv local vertex (0,1,2 or 3) index in \p t
         * \return the global index of the \p lv%th vertex of tetrahedron \p t
         *  or -1 if the vertex is at infinity
         */
        signed_index_t tet_vertex(index_t t, index_t lv) const {
            geo_debug_assert(t < max_t());
            geo_debug_assert(lv < 4);
            return cell_to_v_store_[4 * t + lv];
        }

        /**
         * \brief Finds the index of the vertex in a tetrahedron.
         * \param[in] t the tetrahedron
         * \param[in] v the vertex
         * \return iv such that tet_vertex(t,v)==iv
         * \pre \p t is incident to \p v
         */
        index_t find_tet_vertex(index_t t, signed_index_t v) const {
            geo_debug_assert(t < max_t());
            //   Find local index of v in tetrahedron t vertices.
            const signed_index_t* T = &(cell_to_v_store_[4 * t]);
            return find_4(T,v);
        }


        /**
         * \brief Gets the index of a vertex of a tetrahedron
         * \param[in] t index of the tetrahedron
         * \param[in] lv local vertex (0,1,2 or 3) index in \p t
         * \return the global index of the \p lv%th vertex of tetrahedron \p t
         * \pre Vertex \p lv of tetrahedron \p t is not at infinity
         */
         index_t finite_tet_vertex(index_t t, index_t lv) const {
            geo_debug_assert(t < max_t());
            geo_debug_assert(lv < 4);
            geo_debug_assert(cell_to_v_store_[4 * t + lv] != -1);
            return index_t(cell_to_v_store_[4 * t + lv]);
        }

        /**
         * \brief Sets a tetrahedron-to-vertex adjacency.
         * \param[in] t index of the tetrahedron
         * \param[in] lv local vertex index (0,1,2 or 3) in \p t
         * \param[in] v global index of the vertex
         */
        void set_tet_vertex(index_t t, index_t lv, signed_index_t v) {
            geo_debug_assert(t < max_t());
            geo_debug_assert(lv < 4);
            cell_to_v_store_[4 * t + lv] = v;
        }

        /**
         * \brief Gets the index of a tetrahedron adjacent to another one.
         * \param[in] t index of the tetrahedron
         * \param[in] lf local facet (0,1,2 or 3) index in \p t
         * \return the tetrahedron adjacent to \p t accorss facet \p lf
         */
        signed_index_t tet_adjacent(index_t t, index_t lf) const {
            geo_debug_assert(t < max_t());
            geo_debug_assert(lf < 4);
            signed_index_t result = cell_to_cell_store_[4 * t + lf];
            return result;
        }

        /**
         * \brief Sets a tetrahedron-to-tetrahedron adjacency.
         * \param[in] t1 index of the first tetrahedron
         * \param[in] lf1 local facet index (0,1,2 or 3) in t1
         * \param[in] t2 index of the tetrahedron
         *  adjacent to \p t1 accros \p lf1
         */
        void set_tet_adjacent(index_t t1, index_t lf1, index_t t2) {
            geo_debug_assert(t1 < max_t());
            geo_debug_assert(t2 < max_t());
            geo_debug_assert(lf1 < 4);
            cell_to_cell_store_[4 * t1 + lf1] = signed_index_t(t2);
        }
        
        /**
         * \brief Finds the index of the facet accros which t1 is 
         *  adjacent to t2_in.
         * \param[in] t1 first tetrahedron
         * \param[in] t2_in second tetrahedron
         * \return f such that tet_adjacent(t1,f)==t2_in
         * \pre \p t1 and \p t2_in are adjacent
         */
        index_t find_tet_adjacent(
            index_t t1, index_t t2_in
        ) const {
            geo_debug_assert(t1 < max_t());
            geo_debug_assert(t2_in < max_t());
            geo_debug_assert(t1 != t2_in);

            signed_index_t t2 = signed_index_t(t2_in);

            // Find local index of t2 in tetrahedron t1 adajcent tets.
            const signed_index_t* T = &(cell_to_cell_store_[4 * t1]);
            index_t result = find_4(T,t2);

            // Sanity check: make sure that t1 is adjacent to t2
            // only once!
            geo_debug_assert(tet_adjacent(t1,(result+1)%4) != t2);
            geo_debug_assert(tet_adjacent(t1,(result+2)%4) != t2);
            geo_debug_assert(tet_adjacent(t1,(result+3)%4) != t2);
            return result;
        }



        /**
         * \brief Sets the vertices and adjacent tetrahedra of
         *  a tetrahedron.
         * \param[in] t index of the tetrahedron
         * \param[in] v0 index of the first vertex
         * \param[in] v1 index of the second vertex
         * \param[in] v2 index of the third vertex
         * \param[in] v3 index of the fourth vertex
         * \param[in] a0 index of the adjacent tetrahedron opposite to \p v0
         * \param[in] a1 index of the adjacent tetrahedron opposite to \p v1
         * \param[in] a2 index of the adjacent tetrahedron opposite to \p v2
         * \param[in] a3 index of the adjacent tetrahedron opposite to \p v3
         */
        void set_tet(
            index_t t,
            signed_index_t v0, signed_index_t v1,
            signed_index_t v2, signed_index_t v3,
            index_t a0, index_t a1, index_t a2, index_t a3
        ) {
            geo_debug_assert(t < max_t());
            cell_to_v_store_[4 * t] = v0;
            cell_to_v_store_[4 * t + 1] = v1;
            cell_to_v_store_[4 * t + 2] = v2;
            cell_to_v_store_[4 * t + 3] = v3;
            cell_to_cell_store_[4 * t] = signed_index_t(a0);
            cell_to_cell_store_[4 * t + 1] = signed_index_t(a1);
            cell_to_cell_store_[4 * t + 2] = signed_index_t(a2);
            cell_to_cell_store_[4 * t + 3] = signed_index_t(a3);
        }

        // _________ Combinatorics - traversals ______________________________

        /**
         *  Gets the local facet index incident to an
         * oriented halfedge.
         * \param[in] t index of the tetrahedron
         * \param[in] v1 global index of the first extremity
         * \param[in] v2 global index of the second extremity
         * \return the local index of the facet incident to
         *  the oriented edge \p v1, \p v2.
         */
        index_t get_facet_by_halfedge(
            index_t t, signed_index_t v1, signed_index_t v2
        ) const {
            geo_debug_assert(t < max_t());
            geo_debug_assert(v1 != v2);
            //   Find local index of v1 and v2 in tetrahedron t
            const signed_index_t* T = &(cell_to_v_store_[4 * t]);
            index_t lv1 = find_4(T,v1);
            index_t lv2 = find_4(T,v2);
            geo_debug_assert(lv1 != lv2);
            return index_t(halfedge_facet_[lv1][lv2]);
        }


        /**
         *  Gets the local facet indices incident to an
         * oriented halfedge.
         * \param[in] t index of the tetrahedron
         * \param[in] v1 global index of the first extremity
         * \param[in] v2 global index of the second extremity
         * \param[out] f12 the local index of the facet 
         *  indicent to the halfedge [v1,v2]
         * \param[out] f21 the local index of the facet 
         *  indicent to the halfedge [v2,v1]
         */
        void get_facets_by_halfedge(
            index_t t, signed_index_t v1, signed_index_t v2,
            index_t& f12, index_t& f21
        ) const {
            geo_debug_assert(t < max_t());
            geo_debug_assert(v1 != v2);

            //   Find local index of v1 and v2 in tetrahedron t
            // The following expression is 10% faster than using
            // if() statements (multiply by boolean result of test).
            // Thank to Laurent Alonso for this idea.
            const signed_index_t* T = &(cell_to_v_store_[4 * t]);

            signed_index_t lv1 = 
                (T[1] == v1) | ((T[2] == v1) * 2) | ((T[3] == v1) * 3);

            signed_index_t lv2 = 
                (T[1] == v2) | ((T[2] == v2) * 2) | ((T[3] == v2) * 3);

            geo_debug_assert(lv1 != 0 || T[0] == v1);
            geo_debug_assert(lv2 != 0 || T[0] == v2);
            geo_debug_assert(lv1 >= 0);
            geo_debug_assert(lv2 >= 0);
            geo_debug_assert(lv1 != lv2);

            f12 = index_t(halfedge_facet_[lv1][lv2]);
            f21 = index_t(halfedge_facet_[lv2][lv1]);
        }


        /**
         * \brief Gets the next tetrahedron around an oriented edge of
         *  a tetrahedron.
         * \param[in,out] t the tetrahedron
         * \param[in] v1 global index of the first extremity of the edge
         * \param[in] v2 global index of the second extremity of the edge
         * \return the next tetrahedron from \p t around the oriented edge
         *   (\p v1 \p v2).
         */
        index_t next_around_halfedge(
            index_t& t, signed_index_t v1, signed_index_t v2
        ) const {
            return (index_t)tet_adjacent(
                t, get_facet_by_halfedge(t, v1, v2)
            );
        }

        // _________ Predicates _____________________________________________

        /**
         * \brief Tests whether a given tetrahedron is in conflict with
         *  a given 3d point.
         * \details A real tetrahedron is in conflict with a point whenever
         *  the point is contained by its circumscribed sphere, and a
         *  virtual tetrahedron is in conflict with a point whenever the
         *  tetrahedron formed by its real face and with the point has
         *  positive orientation.
         * \param[in] t the index of the tetrahedron
         * \param[in] p a pointer to the coordinates of the point
         * \retval true if point \p p is in conflict with tetrahedron \p t
         * \retval false otherwise
         */
        bool tet_is_conflict(index_t t, const double* p) const {

            // Lookup tetrahedron vertices
            const double* pv[4];
            for(index_t i=0; i<4; ++i) {
                signed_index_t v = tet_vertex(t,i);
                pv[i] = (v == -1) ? nullptr : vertex_ptr(index_t(v));
            }

            // Check for virtual tetrahedra (then in_sphere()
            // is replaced with orient3d())
            for(index_t lf = 0; lf < 4; ++lf) {

                if(pv[lf] == nullptr) {

                    // Facet of a virtual tetrahedron opposite to
                    // infinite vertex corresponds to
                    // the triangle on the convex hull of the points.
                    // Orientation is obtained by replacing vertex lf
                    // with p.
                    pv[lf] = p;
                    Sign sign = PCK::orient_3d(pv[0],pv[1],pv[2],pv[3]);

                    if(sign > 0) {
                        return true;
                    }

                    if(sign < 0) {
                        return false;
                    }

                    // If sign is zero, we check the real tetrahedron
                    // adjacent to the facet on the convex hull.
                    geo_debug_assert(tet_adjacent(t, lf) >= 0);
                    index_t t2 = index_t(tet_adjacent(t, lf));
                    geo_debug_assert(!tet_is_virtual(t2));

                    //  If t2 is already chained in the conflict list,
                    // then it is conflict
                    if(tet_is_in_list(t2)) {
                        return true;
                    }

                    //  If t2 is marked, then it is not in conflict.
                    if(tet_is_marked(t2)) {
                        return false;
                    }

                    return tet_is_conflict(t2, p);
                }
            }

            //   If the tetrahedron is a finite one, it is in conflict
            // if its circumscribed sphere contains the point (this is
            // the standard case).

            if(weighted_) {
                double h0 = heights_[finite_tet_vertex(t, 0)];
                double h1 = heights_[finite_tet_vertex(t, 1)];
                double h2 = heights_[finite_tet_vertex(t, 2)];
                double h3 = heights_[finite_tet_vertex(t, 3)];
                index_t pindex = index_t(
                    (p - vertex_ptr(0)) / int(vertex_stride_)
                );
                double h = heights_[pindex];
                return (PCK::orient_3dlifted_SOS(
                            pv[0],pv[1],pv[2],pv[3],p,h0,h1,h2,h3,h
                       ) > 0) ;
            }

            return (PCK::in_sphere_3d_SOS(pv[0], pv[1], pv[2], pv[3], p) > 0);
        }

    protected:

        /**
         * \brief Finds the index of an integer in an array of four integers.
         * \param[in] T a const pointer to an array of four integers
         * \param[in] v the integer to retrieve in \p T
         * \return the index (0,1,2 or 3) of \p v in \p T
         * \pre The four entries of \p T are different and one of them is
         *  equal to \p v.
         */
        static index_t find_4(const signed_index_t* T, signed_index_t v) {
            // The following expression is 10% faster than using
            // if() statements. This uses the C++ norm, that 
            // ensures that the 'true' boolean value converted to 
            // an int is always 1. With most compilers, this avoids 
            // generating branching instructions.
            // Thank to Laurent Alonso for this idea.
            // Note: Laurent also has this version:
            //    (T[0] != v)+(T[2]==v)+2*(T[3]==v)
            // that avoids a *3 multiply, but it is not faster in
            // practice.
            index_t result = index_t(
                (T[1] == v) | ((T[2] == v) * 2) | ((T[3] == v) * 3)
            );
            // Sanity check, important if it was T[0], not explicitly
            // tested (detects input that does not meet the precondition).
            geo_debug_assert(T[result] == v);
            return result; 
        }

        /**
         * \brief Delaunay3d destructor
         */
        virtual ~Delaunay3d();

        /**
         * \brief For debugging purposes, displays a tetrahedron.
         * \param[in] t index of the tetrahedron to display.
         */
        void show_tet(index_t t) const;

        /**
         * \brief For debugging purposes, displays a tetrahedron adjacency.
         * \param[in] t index of the tetrahedron to display.
         * \param[in] lf local index (0,1,2 or 3) of the tetrahedron
         *  facet adjacenty to display.
         */
        void show_tet_adjacent(index_t t, index_t lf) const;

        /**
         * \brief For debugging purposes, displays a tetrahedron.
         * \param[in] first index of the first tetrahedron in the list
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
         * \brief Gives the indexing of tetrahedron facet
         *  vertices.
         * \details tet_facet_vertex[lf][lv] gives the
         *  local vertex index (in 0,1,2,3) from a
         *  local facet index lf (in 0,1,2,3) and a
         *  local vertex index within the facet (in 0,1,2).
         */
        static char tet_facet_vertex_[4][3];

        /**
         * \brief Gives a local facet index by
         *  halfedge extremities local indices.
         */
        static char halfedge_facet_[4][4];

        /**
         * \brief Used by the (de-recursified)
         *  find_conflict_zone_iterative() function.
         */
        std::stack<index_t> S_;

        /**
         * \brief Used to represent the stack in the
         *  (de-recursified) stellate_conflict_zone_iterative()
         *  function.
         */
        class StellateConflictStack {
        public:

            /**
             * \brief Pushes a new frame onto the stack.
             * \param[in] t1 index of a tetrahedron on the border of
             *  the conflict zone
             * \param[in] t1fbord index of the facet of \p t1 that is
             *  on the border of the conflict zone
             * \param[in] t1fprev index of the facet of \p t1 that we
             *  come from, or index_t(-1) if \p t1 is the first tetrahedron
             */
            void push(index_t t1, index_t t1fbord, index_t t1fprev) {
                store_.resize(store_.size()+1);
                top().t1 = t1;
                top().t1fbord = Numeric::uint8(t1fbord);
                top().t1fprev = Numeric::uint8(t1fprev);
            }

            /**
             * \brief Saves local variables into the current stack frame.
             * \param[in] new_t the index of the newly created tetrahedron
             * \param[in] t1ft2 the facet of t1 that is adjacent to t2
             * \param[in] t2ft1 the facet of t2 that is adjacent to t1
             */
            void save_locals(index_t new_t, index_t t1ft2, index_t t2ft1) {
                geo_debug_assert(!empty());
                top().new_t = new_t;
                top().t1ft2 = Numeric::uint8(t1ft2);
                top().t2ft1 = Numeric::uint8(t2ft1);
            }

            /**
             * \brief Gets the parameters from the current stack frame.
             * \param[out] t1 index of a tetrahedron on the border of
             *  the conflict zone
             * \param[out] t1fbord index of the facet of \p t1 that is
             *  on the border of the conflict zone
             * \param[out] t1fprev index of the facet of \p t1 that we
             *  come from, or index_t(-1) if \p t1 is the first tetrahedron
             */
            void get_parameters(
                index_t& t1, index_t& t1fbord, index_t& t1fprev
            ) const {
                geo_debug_assert(!empty());
                t1      = top().t1;
                t1fbord = index_t(top().t1fbord);
                t1fprev = index_t(top().t1fprev);
            }


            /**
             * \brief Gets the local variables from the current stack frame.
             * \param[out] new_t the index of the newly created tetrahedron
             * \param[out] t1ft2 the facet of t1 that is adjacent to t2
             * \param[out] t2ft1 the facet of t2 that is adjacent to t1
             */
            void get_locals(
                index_t& new_t, index_t& t1ft2, index_t& t2ft1
            ) const {
                geo_debug_assert(!empty());
                new_t = top().new_t;
                t1ft2 = index_t(top().t1ft2);
                t2ft1 = index_t(top().t2ft1);
            }
            
            /**
             * \brief Pops a stack frame.
             */
            void pop() {
                geo_debug_assert(!empty());
                store_.pop_back();
            }
            
            /**
             * \brief Tests whether the stack is empty.
             * \retval true if the stack is empty
             * \retval false otherwise
             */
            bool empty() const {
                return store_.empty();
            }
            
        private:

            /**
             * \brief The parameters and local
             *  variables stored in a stack frame.
             */
            struct Frame {
                // Parameters
                index_t t1;
                index_t new_t;                
                Numeric::uint8 t1fbord ;
                
                // Local variables
                Numeric::uint8 t1fprev ;
                Numeric::uint8 t1ft2   ;
                Numeric::uint8 t2ft1   ;
            };

            /**
             * \brief Gets the top of the stack.
             * \return a modifiable reference to the Frame on
             *  the top of the stack
             * \pre !empty()
             */
            Frame& top() {
                geo_debug_assert(!empty());
                return *store_.rbegin();
            }

            /**
             * \brief Gets the top of the stack.
             * \return a const reference to the Frame on
             *  the top of the stack
             * \pre !empty()
             */
            const Frame& top() const {
                geo_debug_assert(!empty());
                return *store_.rbegin();
            }
            
            std::vector<Frame> store_;
        };

        /**
         * \brief Used by the (de-recursified)
         *   stellate_conflict_zone_iterative() function.
         */
        StellateConflictStack S2_;

	Cavity cavity_;
    };

    /************************************************************************/

    /**
     * \brief Regular Delaunay triangulation of weighted points
     * \details
     * - the input points are 4d points, were the fourth coordinate
     *  of point \f$ i \f$ is \f$ \sqrt{W - w_i} \f$ where \f$ W \f$ is
     *  the maximum of the  weights of all the points and \d$ w_i \$ is
     *  the weight associated with vertex \f$ i \f$.
     * - the constructed combinatorics is a tetrahedralized volume (3d and
     *  not 4d although dimension() returns 4). This tetrahedralized volume
     *  corresponds to the regular triangulation of the weighted points.
     */
    class GEOGRAM_API RegularWeightedDelaunay3d : public Delaunay3d {
    public:
        /**
         * \brief Constructs a new Regular Delaunay3d triangulation.
         * \details RegularWeightedDelaunay3d triangulations are only
         * supported for dimension 3. If a different dimension is specified in
         * the constructor, a InvalidDimension exception is thrown.
         * \param[in] dimension dimension of the triangulation
         * \throw InvalidDimension This exception is thrown if dimension is
         * different than 3.
         */
        RegularWeightedDelaunay3d(coord_index_t dimension = 4);

    protected:
        /**
         * \brief RegularWeightedDelaunay3d destructor
         */
        virtual ~RegularWeightedDelaunay3d();
    };
}

#endif

