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

#include <geogram/delaunay/delaunay_2d.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/geometry_nd.h>
#include <geogram/basic/process.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/stopwatch.h>
#include <geogram/basic/matrix.h>
#include <geogram/basic/permutation.h>
#include <geogram/mesh/mesh_reorder.h>
#include <geogram/bibliography/bibliography.h>

#include <stack>
#include <algorithm>

// TODO: optimizations:
// - convex hull traversal for nearest_vertex()

namespace {
    using namespace GEO;

    /**
     * \brief Computes the (approximate) orientation predicate in 2d.
     * \details Computes the sign of the (approximate) signed volume of
     *  the triangle p0, p1, p2
     * \param[in] p0 first vertex of the triangle
     * \param[in] p1 second vertex of the triangle
     * \param[in] p2 third vertex of the triangle
     * \retval POSITIVE if the triangle is oriented positively
     * \retval ZERO if the triangle is flat
     * \retval NEGATIVE if the triangle is oriented negatively
     * \todo check whether orientation is inverted as compared to 
     *   Shewchuk's version.
     */
    inline Sign orient_2d_inexact(
        const double* p0, const double* p1,
        const double* p2
    ) {
        double a11 = p1[0] - p0[0] ;
        double a12 = p1[1] - p0[1] ;
        
        double a21 = p2[0] - p0[0] ;
        double a22 = p2[1] - p0[1] ;
        
        double Delta = det2x2(
            a11,a12,
            a21,a22
        );

        return geo_sgn(Delta);
    }
}

namespace GEO {

    // triangle edge vertex is such that the triangle
    // formed with:
    //  vertex lv
    //  triangle_edge_vertex[lv][0]
    //  triangle_edge_vertex[lv][1]
    // has the same orientation as the original triangle for
    // any vertex lv.

    char Delaunay2d::triangle_edge_vertex_[3][2] = {
        {1,2},
        {2,0},
        {0,1}
    };

    Delaunay2d::Delaunay2d(coord_index_t dimension) :
        Delaunay(dimension)
    {
	geo_cite_with_info(
	    "DBLP:journals/cj/Bowyer81",
	    "One of the two initial references to the algorithm, "
	    "discovered independently and simultaneously by Bowyer and Watson."
        );
	geo_cite_with_info(
	    "journals/cj/Watson81",
	    "One of the two initial references to the algorithm, "
	    "discovered independently and simultaneously by Bowyer and Watson."
	);
	geo_cite_with_info(
	    "DBLP:conf/compgeom/AmentaCR03",
	    "Using spatial sorting has a dramatic impact on the performances."
	);
	geo_cite_with_info(
	    "DBLP:journals/comgeo/FunkeMN05",
	    "Initializing \\verb|locate()| with a non-exact version "
	    " (structural filtering) gains (a bit of) performance."
	);
	geo_cite_with_info(
	    "DBLP:journals/comgeo/BoissonnatDPTY02",
	    "The idea of traversing the cavity from inside "
	    " used in GEOGRAM is inspired by the implementation of "
	    " \\verb|Delaunay_triangulation_3| in CGAL."
	);
	geo_cite_with_info(
	    "DBLP:conf/imr/Si06",
	    "The triangulation data structure used in GEOGRAM is inspired "
	    "by Tetgen."
	);
	geo_cite_with_info(
	    "DBLP:journals/ijfcs/DevillersPT02",
	    "Analysis of the different versions of the line walk algorithm "
	    " used by \\verb|locate()|."
	);
	
        if(dimension != 2 && dimension != 3) {
            throw InvalidDimension(dimension, "Delaunay2d", "2 or 3");
        }
        first_free_ = END_OF_LIST;
        weighted_ = (dimension == 3);
        // In weighted mode, vertices are 3d but combinatorics is 2d.
        if(weighted_) {
            cell_size_ = 3;
            cell_v_stride_ = 3;
            cell_neigh_stride_ = 3;
        }
        cur_stamp_ = 0;
        debug_mode_ = CmdLine::get_arg_bool("dbg:delaunay");
        verbose_debug_mode_ = CmdLine::get_arg_bool("dbg:delaunay_verbose");
        debug_mode_ = (debug_mode_ || verbose_debug_mode_);
        benchmark_mode_ = CmdLine::get_arg_bool("dbg:delaunay_benchmark");
    }

    Delaunay2d::~Delaunay2d() {
    }

    void Delaunay2d::set_vertices(
        index_t nb_vertices, const double* vertices
    ) {
        Stopwatch* W = nullptr;
        if(benchmark_mode_) {
            W = new Stopwatch("DelInternal");
        }
        cur_stamp_ = 0;
        if(weighted_) {
            heights_.resize(nb_vertices);
            for(index_t i = 0; i < nb_vertices; ++i) {
                // Client code uses 3d embedding with ti = sqrt(W - wi)
                //   where W = max(wi)
                // We recompute the standard "shifted" lifting on
                // the paraboloid from it.
                // (we use wi - W, everything is shifted by W, but
                // we do not care since the power diagram is invariant
                // by a translation of all weights).
                double w = -geo_sqr(vertices[3 * i + 2]);
                heights_[i] = -w +
                    geo_sqr(vertices[3 * i]) +
                    geo_sqr(vertices[3 * i + 1]); 
            }
        }

        Delaunay::set_vertices(nb_vertices, vertices);

        index_t expected_triangles = nb_vertices * 2;

        cell_to_v_store_.reserve(expected_triangles * 3);
        cell_to_cell_store_.reserve(expected_triangles * 3);
        cell_next_.reserve(expected_triangles);

        cell_to_v_store_.resize(0);
        cell_to_cell_store_.resize(0);
        cell_next_.resize(0);
        first_free_ = END_OF_LIST;

        //   Sort the vertices spatially. This makes localisation
        // faster.
        if(do_reorder_) {
            compute_BRIO_order(
                nb_vertices, vertex_ptr(0), reorder_, 2, dimension_
            );
	} else {
            reorder_.resize(nb_vertices);
            for(index_t i = 0; i < nb_vertices; ++i) {
                reorder_[i] = i;
            }
        }

        double sorting_time = 0;
        if(benchmark_mode_) {
            sorting_time = W->elapsed_time();
            Logger::out("DelInternal1") << "BRIO sorting:"
                                       << sorting_time
                                       << std::endl;
        } 

        // The indices of the vertices of the first tetrahedron.
        index_t v0, v1, v2;
        if(!create_first_triangle(v0, v1, v2)) {
            Logger::warn("Delaunay2d") << "All the points are colinear"
                << std::endl;
            return;
        }

        index_t hint = NO_TRIANGLE;
        // Insert all the vertices incrementally.
        for(index_t i = 0; i < nb_vertices; ++i) {
            index_t v = reorder_[i];
            // Do not re-insert the first four vertices.
            if(v != v0 && v != v1 && v != v2) {
                index_t new_hint = insert(v, hint);
                if(new_hint != NO_TRIANGLE) {
                    hint = new_hint;
                }
            }
        }

        if(benchmark_mode_) {
            Logger::out("DelInternal2") << "Core insertion algo:"
                                       << W->elapsed_time() - sorting_time
                                       << std::endl;
        }
        delete W;

        if(debug_mode_) {
            check_combinatorics(verbose_debug_mode_);
            check_geometry(verbose_debug_mode_);
        }

        //   Compress cell_to_v_store_ and cell_to_cell_store_
        // (remove free and virtual tetrahedra).
        //   Since cell_next_ is not used at this point,
        // we reuse it for storing the conversion array that
        // maps old tet indices to new tet indices
        // Note: tet_is_real() uses the previous value of 
        // cell_next(), but we are processing indices
        // in increasing order and since old2new[t] is always
        // smaller or equal to t, we never overwrite a value
        // before needing it.
        
        vector<index_t>& old2new = cell_next_;
        index_t nb_triangles = 0;
        index_t nb_triangles_to_delete = 0;
        
        {
            for(index_t t = 0; t < max_t(); ++t) {
                if(
                    (keep_infinite_ && !triangle_is_free(t)) ||
                    triangle_is_real(t)
                ) {
                    if(t != nb_triangles) {
                        Memory::copy(
                            &cell_to_v_store_[nb_triangles * 3],
                            &cell_to_v_store_[t * 3],
                            3 * sizeof(signed_index_t)
                        );
                        Memory::copy(
                            &cell_to_cell_store_[nb_triangles * 3],
                            &cell_to_cell_store_[t * 3],
                            3 * sizeof(signed_index_t)
                        );
                    }
                    old2new[t] = nb_triangles;
                    ++nb_triangles;
                } else {
                    old2new[t] = index_t(-1);
                    ++nb_triangles_to_delete;
                }
            }
            cell_to_v_store_.resize(3 * nb_triangles);
            cell_to_cell_store_.resize(3 * nb_triangles);
            for(index_t i = 0; i < 3 * nb_triangles; ++i) {
                signed_index_t t = cell_to_cell_store_[i];
                geo_debug_assert(t >= 0);
                t = signed_index_t(old2new[t]);
                // Note: t can be equal to -1 when a real tet is
                // adjacent to a virtual one (and this is how the
                // rest of Vorpaline expects to see tets on the
                // border).
                cell_to_cell_store_[i] = t;
            }
        }

        // In "keep_infinite" mode, we reorder the cells in such
        // a way that finite cells have indices [0..nb_finite_cells_-1]
        // and infinite cells have indices [nb_finite_cells_ .. nb_cells_-1]
        
        if(keep_infinite_) {
            nb_finite_cells_ = 0;
            index_t finite_ptr = 0;
            index_t infinite_ptr = nb_triangles - 1;
            for(;;) {
                while(triangle_is_finite(finite_ptr)) {
                    old2new[finite_ptr] = finite_ptr;
                    ++finite_ptr;
                    ++nb_finite_cells_;
                }
                while(!triangle_is_finite(infinite_ptr)) {
                    old2new[infinite_ptr] = infinite_ptr;
                    --infinite_ptr;
                }
                if(finite_ptr > infinite_ptr) {
                    break;
                }
                old2new[finite_ptr] = infinite_ptr;
                old2new[infinite_ptr] = finite_ptr;
                ++nb_finite_cells_;
                for(index_t lf=0; lf<3; ++lf) {
		    std::swap(
                        cell_to_cell_store_[3*finite_ptr + lf],
                        cell_to_cell_store_[3*infinite_ptr + lf]
                    );
                }
                for(index_t lv=0; lv<3; ++lv) {
		    std::swap(
                        cell_to_v_store_[3*finite_ptr + lv],
                        cell_to_v_store_[3*infinite_ptr + lv]
                    );
                }
                ++finite_ptr;
                --infinite_ptr;
            }
            for(index_t i = 0; i < 3 * nb_triangles; ++i) {
                signed_index_t t = cell_to_cell_store_[i];
                geo_debug_assert(t >= 0);
                t = signed_index_t(old2new[t]);
                geo_debug_assert(t >= 0);
                cell_to_cell_store_[i] = t;
            }
        }

        if(benchmark_mode_) {
            if(keep_infinite_) {
                Logger::out("DelCompress") 
                    << "Removed " << nb_triangles_to_delete 
                    << " triangles (free list)" << std::endl;
            } else {
                Logger::out("DelCompress") 
                    << "Removed " << nb_triangles_to_delete 
                    << " triangles (free list and infinite)" << std::endl;
            }
        }
        
        set_arrays(
            nb_triangles,
            cell_to_v_store_.data(), cell_to_cell_store_.data()
        );
    }

    index_t Delaunay2d::nearest_vertex(const double* p) const {

        // TODO: For the moment, we fallback to the (unefficient)
        // baseclass implementation when in weighted mode.
        if(weighted_) {
            return Delaunay::nearest_vertex(p);
        }

        // Find a triangle (real or virtual) that contains p
        index_t t = locate(p, NO_TRIANGLE, thread_safe());

        //   If p is outside the convex hull of the inserted points,
        // a special traversal is required (not implemented yet).
        // TODO: implement convex hull boundary traversal
        // (for now we fallback to linear search implemented
        //  in baseclass)
        if(t == NO_TRIANGLE || triangle_is_virtual(t)) {
            return Delaunay::nearest_vertex(p);
        }

        double sq_dist = 1e30;
        index_t result = NO_TRIANGLE;

        // Find the nearest vertex among t's vertices
        for(index_t lv = 0; lv < 3; ++lv) {
            signed_index_t v = triangle_vertex(t, lv);
            // If the tetrahedron is virtual, then the first vertex
            // is the vertex at infinity and is skipped.
            if(v < 0) {
                continue;
            }
            double cur_sq_dist = Geom::distance2(p, vertex_ptr(index_t(v)), 2);
            if(cur_sq_dist < sq_dist) {
                sq_dist = cur_sq_dist;
                result = index_t(v);
            }
        }
        return result;
    }



    index_t Delaunay2d::locate_inexact(
        const double* p, index_t hint, index_t max_iter
    ) const {

        // If no hint specified, find a tetrahedron randomly
        while(hint == NO_TRIANGLE) {
            hint = index_t(Numeric::random_int32()) % max_t();
            if(triangle_is_free(hint)) {
                hint = NO_TRIANGLE;
            }
        }

	geo_debug_assert(!triangle_is_free(hint));
	geo_debug_assert(!triangle_is_in_list(hint));
	
        //  Always start from a real tet. If the tet is virtual,
        // find its real neighbor (always opposite to the
        // infinite vertex)
        if(triangle_is_virtual(hint)) {
            for(index_t lf = 0; lf < 3; ++lf) {
                if(triangle_vertex(hint, lf) == VERTEX_AT_INFINITY) {
                    hint = index_t(triangle_adjacent(hint, lf));
                    geo_debug_assert(hint != NO_TRIANGLE);
                    break;
                }
            }
        }

        index_t t = hint;
        index_t t_pred = NO_TRIANGLE;

    still_walking:
        {
            const double* pv[3];
            pv[0] = vertex_ptr(finite_triangle_vertex(t,0));
            pv[1] = vertex_ptr(finite_triangle_vertex(t,1));
            pv[2] = vertex_ptr(finite_triangle_vertex(t,2));
            
            for(index_t le = 0; le < 3; ++le) {
                
                signed_index_t s_t_next = triangle_adjacent(t,le);

                //  If the opposite tet is -1, then it means that
                // we are trying to locate() (e.g. called from
                // nearest_vertex) within a tetrahedralization 
                // from which the infinite tets were removed.
                if(s_t_next == -1) {
                    return NO_TRIANGLE;
                }

                index_t t_next = index_t(s_t_next);

                //   If the candidate next tetrahedron is the
                // one we came from, then we know already that
                // the orientation is positive, thus we examine
                // the next candidate (or exit the loop if they
                // are exhausted).
                if(t_next == t_pred) {
                    continue ; 
                }

                //   To test the orientation of p w.r.t. the facet f of
                // t, we replace vertex number f with p in t (same
                // convention as in CGAL).
                const double* pv_bkp = pv[le];
                pv[le] = p;
                Sign ori = orient_2d_inexact(pv[0], pv[1], pv[2]);

                //   If the orientation is not negative, then we cannot
                // walk towards t_next, and examine the next candidate
                // (or exit the loop if they are exhausted).
                if(ori != NEGATIVE) {
                    pv[le] = pv_bkp;
                    continue;
                }

                //  If the opposite tet is a virtual tet, then
                // the point has a positive orientation relative
                // to the facet on the border of the convex hull,
                // thus t_next is a tet in conflict and we are
                // done.
                if(triangle_is_virtual(t_next)) {
                    return t_next;
                }

                //   If we reach this point, then t_next is a valid
                // successor, thus we are still walking.
                t_pred = t;
                t = t_next;
                if(--max_iter != 0) {
                    goto still_walking;
                }
            }
        } 

        //   If we reach this point, we did not find a valid successor
        // for walking (a face for which p has negative orientation), 
        // thus we reached the tet for which p has all positive 
        // face orientations (i.e. the tet that contains p).

        return t;
    }


    index_t Delaunay2d::locate(
        const double* p, index_t hint, bool thread_safe,
        Sign* orient
    ) const {

        //   Try improving the hint by using the 
        // inexact locate function. This gains
        // (a little bit) performance (a few 
        // percent in total Delaunay computation
        // time), but it is better than nothing...
        //   Note: there is a maximum number of tets 
        // traversed by locate_inexact()  (2500)
        // since there exists configurations in which
        // locate_inexact() loops forever !

	hint = locate_inexact(p, hint, 2500);

        static Process::spinlock lock = GEOGRAM_SPINLOCK_INIT;

        // We need to have this spinlock because
        // of random() that is not thread-safe
        // (TODO: implement a random() function with
        //  thread local storage)
        if(thread_safe) {
            Process::acquire_spinlock(lock);
        }

        // If no hint specified, find a tetrahedron randomly
        while(hint == NO_TRIANGLE) {
            hint = index_t(Numeric::random_int32()) % max_t();
            if(triangle_is_free(hint)) {
                hint = NO_TRIANGLE;
            }
        }

	geo_debug_assert(!triangle_is_free(hint));
	geo_debug_assert(!triangle_is_in_list(hint));
	
        //  Always start from a real tet. If the tet is virtual,
        // find its real neighbor (always opposite to the
        // infinite vertex)
        if(triangle_is_virtual(hint)) {
            for(index_t le = 0; le < 3; ++le) {
                if(triangle_vertex(hint, le) == VERTEX_AT_INFINITY) {
                    hint = index_t(triangle_adjacent(hint, le));
                    geo_debug_assert(hint != NO_TRIANGLE);
                    break;
                }
            }
        }

	geo_debug_assert(!triangle_is_free(hint));
	geo_debug_assert(!triangle_is_in_list(hint));
	geo_debug_assert(!triangle_is_virtual(hint));	
	
        index_t t = hint;
        index_t t_pred = NO_TRIANGLE;
        Sign orient_local[3];
        if(orient == nullptr) {
            orient = orient_local;
        }


    still_walking:
        {
            const double* pv[3];
            pv[0] = vertex_ptr(finite_triangle_vertex(t,0));
            pv[1] = vertex_ptr(finite_triangle_vertex(t,1));
            pv[2] = vertex_ptr(finite_triangle_vertex(t,2));
            
            // Start from a random facet
            index_t e0 = index_t(Numeric::random_int32()) % 3;
            for(index_t de = 0; de < 3; ++de) {
                index_t le = (e0 + de) % 3;
                
                signed_index_t s_t_next = triangle_adjacent(t,le);

                //  If the opposite triangle is -1, then it means that
                // we are trying to locate() (e.g. called from
                // nearest_vertex) within a tetrahedralization 
                // from which the infinite tets were removed.
                if(s_t_next == -1) {
                    if(thread_safe) {
                        Process::release_spinlock(lock);
                    }
                    return NO_TRIANGLE;
                }

                index_t t_next = index_t(s_t_next);

		geo_debug_assert(!triangle_is_free(t_next));
		geo_debug_assert(!triangle_is_in_list(t_next));

		
                //   If the candidate next tetrahedron is the
                // one we came from, then we know already that
                // the orientation is positive, thus we examine
                // the next candidate (or exit the loop if they
                // are exhausted).
                if(t_next == t_pred) {
                    orient[le] = POSITIVE ;
                    continue ; 
                }

                //   To test the orientation of p w.r.t. the facet f of
                // t, we replace vertex number f with p in t (same
                // convention as in CGAL).
                // This is equivalent to tet_facet_point_orient3d(t,f,p)
                // (but less costly, saves a couple of lookups)
                const double* pv_bkp = pv[le];
                pv[le] = p;
                orient[le] = PCK::orient_2d(pv[0], pv[1], pv[2]);

                //   If the orientation is not negative, then we cannot
                // walk towards t_next, and examine the next candidate
                // (or exit the loop if they are exhausted).
                if(orient[le] != NEGATIVE) {
                    pv[le] = pv_bkp;
                    continue;
                }

                //  If the opposite tet is a virtual tet, then
                // the point has a positive orientation relative
                // to the facet on the border of the convex hull,
                // thus t_next is a tet in conflict and we are
                // done.
                if(triangle_is_virtual(t_next)) {
                    if(thread_safe) {
                        Process::release_spinlock(lock);
                    }
                    for(index_t tle = 0; tle < 3; ++tle) {
                        orient[tle] = POSITIVE;
                    }
                    return t_next;
                }

                //   If we reach this point, then t_next is a valid
                // successor, thus we are still walking.
                t_pred = t;
                t = t_next;
                goto still_walking;
            }
        } 

        //   If we reach this point, we did not find a valid successor
        // for walking (a face for which p has negative orientation), 
        // thus we reached the tet for which p has all positive 
        // face orientations (i.e. the tet that contains p).

        if(thread_safe) {
            Process::release_spinlock(lock);
        }
        return t;
    }

    void Delaunay2d::find_conflict_zone(
        index_t v, 
        index_t t, const Sign* orient, 
        index_t& t_bndry, index_t& e_bndry,
        index_t& first, index_t& last
    ) {
        first = last = END_OF_LIST;

        //  Generate a unique stamp from current vertex index,
        // used for marking tetrahedra.
        set_triangle_mark_stamp(v);

        // Pointer to the coordinates of the point to be inserted
        const double* p = vertex_ptr(v);

        geo_debug_assert(t != NO_TRIANGLE);

        // Test whether the point already exists in
        // the triangulation. The point already exists
        // if it's located on three faces of the
        // tetrahedron returned by locate().
        int nb_zero = 
            (orient[0] == ZERO) +
            (orient[1] == ZERO) +
            (orient[2] == ZERO) ;

        if(nb_zero >= 2) {
            return; 
        }

        //  Weighted triangulations can have dangling
        // vertices. Such vertices p are characterized by
        // the fact that p is not in conflict with the 
        // tetrahedron returned by locate().
        if(weighted_ && !triangle_is_conflict(t, p)) {
            return;
        }

        // Note: points on edges and on facets are
        // handled by the way triangle_is_in_conflict()
        // is implemented, that naturally inserts
        // the correct tetrahedra in the conflict list.


        // Mark t as conflict
        add_triangle_to_list(t, first, last);

        // A small optimization: if the point to be inserted
        // is on some faces of the located triangle, insert
        // the neighbors accros those edges in the conflict list.
        // It saves a couple of calls to the predicates in this
        // specific case (combinatorics are in general less 
        // expensive than the predicates).
        if(!weighted_ && nb_zero != 0) {
            for(index_t le = 0; le < 3; ++le) {
                if(orient[le] == ZERO) {
                    index_t t2 = index_t(triangle_adjacent(t, le));
                    add_triangle_to_list(t2, first, last);
                }
            }
            for(index_t le = 0; le < 3; ++le) {
                if(orient[le] == ZERO) {
                    index_t t2 = index_t(triangle_adjacent(t, le));
                    find_conflict_zone_iterative(
                        p,t2,t_bndry,e_bndry,first,last
                    );
                }
            }
        }

        // Determine the conflict list by greedy propagation from t.
        find_conflict_zone_iterative(p,t,t_bndry,e_bndry,first,last);
    }
    
    void Delaunay2d::find_conflict_zone_iterative(
        const double* p, index_t t_in,
        index_t& t_bndry, index_t& e_bndry,
        index_t& first, index_t& last
    ) {

        S_.push(t_in);

        while(!S_.empty()) {

            index_t t = S_.top();
            S_.pop();
            
            for(index_t le = 0; le < 3; ++le) {
                index_t t2 = index_t(triangle_adjacent(t, le));

                if(
                    triangle_is_in_list(t2) || // known as conflict
                    triangle_is_marked(t2)     // known as non-conflict
                ) {
                    continue;
                }

                if(triangle_is_conflict(t2, p)) {
                    // Chain t2 in conflict list
                    add_triangle_to_list(t2, first, last);
                    S_.push(t2);
                    continue;
                } 
                
                //   At this point, t is in conflict 
                // and t2 is not in conflict. 
                // We keep a reference to a tet on the boundary
                t_bndry = t;
                e_bndry = le;
                // Mark t2 as visited (but not conflict)
                mark_triangle(t2);
            }
        }
    }

    index_t Delaunay2d::stellate_conflict_zone(
        index_t v_in, index_t t1, index_t t1ebord
    ) {

	index_t t = t1;
	index_t e = t1ebord;
	index_t t_adj = index_t(triangle_adjacent(t,e));

	geo_debug_assert(t_adj != index_t(-1));
	
	geo_debug_assert(triangle_is_in_list(t));
	geo_debug_assert(!triangle_is_in_list(t_adj));
	

	index_t new_t_first = index_t(-1);
	index_t new_t_prev  = index_t(-1);
	
	do {

	    signed_index_t v1 = triangle_vertex(t, (e+1)%3);
	    signed_index_t v2 = triangle_vertex(t, (e+2)%3);	    

	    // Create new triangle
	    index_t new_t = new_triangle(signed_index_t(v_in), v1, v2);

	    //   Connect new triangle to triangle on the other
	    // side of the conflict zone.
	    set_triangle_adjacent(new_t, 0, t_adj);
	    index_t adj_e = find_triangle_adjacent(t_adj, t);
	    set_triangle_adjacent(t_adj, adj_e, new_t);
	    
	    
	    // Move to next triangle
	    e = (e + 1)%3;
	    t_adj = index_t(triangle_adjacent(t,e));
	    while(triangle_is_in_list(t_adj)) {
		t = t_adj;
		e = (find_triangle_vertex(t,v2) + 2)%3;		
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

    index_t Delaunay2d::insert(index_t v, index_t hint) {
       index_t t_bndry = NO_TRIANGLE;
       index_t e_bndry = index_t(-1);
       index_t first_conflict = NO_TRIANGLE;
       index_t last_conflict  = NO_TRIANGLE;

       const double* p = vertex_ptr(v);

       Sign orient[3];
       index_t t = locate(p, hint, false, orient);
       find_conflict_zone(
           v,t,orient,t_bndry,e_bndry,first_conflict,last_conflict
       );
       
       // The conflict list can be empty if:
       //  - Vertex v already exists in the triangulation
       //  - The triangulation is weighted and v is not visible
       if(first_conflict == END_OF_LIST) {
           return NO_TRIANGLE;
       }

       index_t new_triangle = stellate_conflict_zone(v,t_bndry,e_bndry);
       
       // Recycle the tetrahedra of the conflict zone.
       cell_next_[last_conflict] = first_free_;
       first_free_ = first_conflict;
       
       // Return one of the newly created triangles
       return new_triangle;
    }

    bool Delaunay2d::create_first_triangle(
        index_t& iv0, index_t& iv1, index_t& iv2
    ) {
        if(nb_vertices() < 3) {
            return false;
        }

        iv0 = 0;

        iv1 = 1;
        while(
            iv1 < nb_vertices() &&
            PCK::points_are_identical_2d(
                vertex_ptr(iv0), vertex_ptr(iv1)
            )
        ) {
            ++iv1;
        }
        if(iv1 == nb_vertices()) {
            return false;
        }

        iv2 = iv1 + 1;
	Sign s = ZERO;
        while(
            iv2 < nb_vertices() &&  
	    (s = PCK::orient_2d(vertex_ptr(iv0), vertex_ptr(iv1), vertex_ptr(iv2))) == ZERO
	) {
            ++iv2;
        }
        if(iv2 == nb_vertices()) {
            return false;
        }
	if(s == NEGATIVE) {
	    std::swap(iv1,iv2);
	}
	    
        // Create the first triangle
        index_t t0 = new_triangle(
            signed_index_t(iv0), 
            signed_index_t(iv1), 
            signed_index_t(iv2)
        );

        // Create the first three virtual triangles surrounding it
        index_t t[3];
        for(index_t e = 0; e < 3; ++e) {
            // In reverse order since it is an adjacent tetrahedron
            signed_index_t v1 = triangle_vertex(t0, triangle_edge_vertex(e,1));
            signed_index_t v2 = triangle_vertex(t0, triangle_edge_vertex(e,0));
            t[e] = new_triangle(VERTEX_AT_INFINITY, v1, v2);
        }

        // Connect the virtual triangles to the real one
        for(index_t e=0; e<3; ++e) {
            set_triangle_adjacent(t[e], 0, t0);
            set_triangle_adjacent(t0, e, t[e]);
        }

        // Interconnect the three virtual triangles along their common
        // edges
        for(index_t e = 0; e < 3; ++e) {
            // In reverse order since it is an adjacent tetrahedron
            index_t lv1 = triangle_edge_vertex(e,1);
            index_t lv2 = triangle_edge_vertex(e,0);
            set_triangle_adjacent(t[e], 1, t[lv1]);
            set_triangle_adjacent(t[e], 2, t[lv2]);
        }

        return true;
    }

    /************************************************************************/

    void Delaunay2d::show_triangle(index_t t) const {
        std::cerr << "tri"
            << (triangle_is_in_list(t) ? '*' : ' ')
            << t
            << ", v=["
            << triangle_vertex(t, 0)
            << ' '
            << triangle_vertex(t, 1)
            << ' '
            << triangle_vertex(t, 2)
            << "]  adj=[";
        show_triangle_adjacent(t, 0);
        show_triangle_adjacent(t, 1);
        show_triangle_adjacent(t, 2);
        std::cerr << "] ";

        for(index_t e = 0; e < 3; ++e) {
            std::cerr << 'e' << e << ':';
            for(index_t v = 0; v < 2; ++v) {
                std::cerr << triangle_vertex(t, triangle_edge_vertex(e,v))
			  << ',';
            }
            std::cerr << ' ';
        }
        std::cerr << std::endl;
    }

    void Delaunay2d::show_triangle_adjacent(index_t t, index_t le) const {
        signed_index_t adj = triangle_adjacent(t, le);
        if(adj != -1) {
            std::cerr << (triangle_is_in_list(index_t(adj)) ? '*' : ' ');
        }
        std::cerr << adj;
        std::cerr << ' ';
    }

    void Delaunay2d::show_list(
        index_t first, const std::string& list_name
    ) const {
        index_t t = first;
        std::cerr << "tri list: " << list_name << std::endl;
        while(t != END_OF_LIST) {
            show_triangle(t);
            t = triangle_next(t);
        }
        std::cerr << "-------------" << std::endl;
    }

    void Delaunay2d::check_combinatorics(bool verbose) const {
        if(verbose) {
            std::cerr << std::endl;
        }
        bool ok = true;
        std::vector<bool> v_has_triangle(nb_vertices(), false);
        for(index_t t = 0; t < max_t(); ++t) {
            if(triangle_is_free(t)) {
/*
                if(verbose) {
                    std::cerr << "-Deleted tri: ";
                    show_tri(t);
                }
*/
            } else {
/*
                if(verbose) {
                    std::cerr << "Checking tri: ";
                    show_tet(t);
                }
*/
                for(index_t le = 0; le < 3; ++le) {
                    if(triangle_adjacent(t, le) == -1) {
                        std::cerr << le << ":Missing adjacent tri"
                            << std::endl;
                        ok = false;
                    } else if(triangle_adjacent(t, le) == signed_index_t(t)) {
                        std::cerr << le << ":Tri is adjacent to itself"
                            << std::endl;
                        ok = false;
                    } else {
                        index_t t2 = index_t(triangle_adjacent(t, le));
                        bool found = false;
                        for(index_t le2 = 0; le2 < 3; ++le2) {
                            if(triangle_adjacent(t2, le2) == signed_index_t(t)) {
                                found = true;
                            }
                        }
                        if(!found) {
                            std::cerr
                                << le << ":Adjacent link is not bidirectional"
                                << std::endl;
                            ok = false;
                        }
                    }
                }
                index_t nb_infinite = 0;
                for(index_t lv = 0; lv < 3; ++lv) {
                    if(triangle_vertex(t, lv) == -1) {
                        ++nb_infinite;
                    }
                }
                if(nb_infinite > 1) {
                    ok = false;
                    std::cerr << "More than one infinite vertex"
                        << std::endl;
                }
            }
            for(index_t lv = 0; lv < 3; ++lv) {
                signed_index_t v = triangle_vertex(t, lv);
                if(v >= 0) {
                    v_has_triangle[index_t(v)] = true;
                }
            }
        }
        for(index_t v = 0; v < nb_vertices(); ++v) {
            if(!v_has_triangle[v]) {
                if(verbose) {
                    std::cerr << "Vertex " << v
                        << " is isolated (duplicated ?)" << std::endl;
                }
            }
        }
        geo_assert(ok);
        if(verbose) {
            std::cerr << std::endl;
        }
        std::cerr << std::endl << "Delaunay Combi OK" << std::endl;
    }

    void Delaunay2d::check_geometry(bool verbose) const {
        bool ok = true;
        for(index_t t = 0; t < max_t(); ++t) {
            if(!triangle_is_free(t)) {
                signed_index_t v0 = triangle_vertex(t, 0);
                signed_index_t v1 = triangle_vertex(t, 1);
                signed_index_t v2 = triangle_vertex(t, 2);
                for(index_t v = 0; v < nb_vertices(); ++v) {
                    signed_index_t sv = signed_index_t(v);
                    if(sv == v0 || sv == v1 || sv == v2) {
                        continue;
                    }
                    if(triangle_is_conflict(t, vertex_ptr(v))) {
                        ok = false;
                        if(verbose) {
                            std::cerr << "Tri " << t <<
                                " is in conflict with vertex " << v
                                    << std::endl;

                            std::cerr << "  offending tri: ";
                            show_triangle(t);
                        }
                    }
                }
            }
        }
        geo_assert(ok);
        std::cerr << std::endl << "Delaunay Geo OK" << std::endl;
    }

    /************************************************************************/

    RegularWeightedDelaunay2d::RegularWeightedDelaunay2d(
        coord_index_t dimension
    ) :
        Delaunay2d(3)
    {
        if(dimension != 3) {
            throw InvalidDimension(dimension, "RegularWeightedDelaunay2d", "3");
        }
    }

    RegularWeightedDelaunay2d::~RegularWeightedDelaunay2d() {
    }
}

