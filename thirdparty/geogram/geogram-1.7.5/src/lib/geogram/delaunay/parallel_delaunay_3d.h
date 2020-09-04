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

#ifndef GEOGRAM_PARALLEL_DELAUNAY_DELAUNAY_3D
#define GEOGRAM_PARALLEL_DELAUNAY_DELAUNAY_3D

#ifdef GEOGRAM_WITH_PDEL

#include <geogram/basic/common.h>
#include <geogram/delaunay/delaunay.h>
#include <geogram/basic/process.h>

/**
 * \file geogram/delaunay/parallel_delaunay_3d.h
 * \brief Multithreaded implementation of Delaunay in 3d.
 */

namespace GEO {

     typedef Numeric::uint8 thread_index_t;
     
    /**
     * \brief Multithreaded implementation of Delaunay in 3d.
     * \details This class is based on ideas and a prototype
     *   implementation by Alain Filbois. This class also uses 
     *  concepts inspired by two triangulation softwares, CGAL and tetgen,
     *  described in the following references. This package follows the
     *  idea used in CGAL of traversing the cavity from inside, since
     *  it traverses less tetrahedra than when traversing from outside.
     *  - Jean-Daniel Boissonnat, Olivier Devillers, Monique Teillaud, 
     *   and Mariette Yvinec. Triangulations in CGAL. 
     *   In Proc. 16th Annu. ACM Sympos. Comput. Geom., pages 11–18, 2000.
     *  - Hang Si, Constrained Delaunay tetrahedral mesh generation and 
     *   refinement. Finite elements in Analysis and Design, 
     *   46 (1-2):33--46, 2010.
     *
     *  Note that the algorithm here does not support vertex deletion nor
     *  degenerate input with all coplanar or all colinear points (use CGAL
     *  instead if you have these requirements).
     *
     *  The core algorithm used in both this code, CGAL and tetgen was
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
     *   17th Annual Symposium on Computational geometry, 106-114
     *  - Stefan Funke , Kurt Mehlhorn and Stefan Naher, "Structural filtering,
     *  a paradigm for efficient and exact geometric programs", 1999
     */
    class GEOGRAM_API ParallelDelaunay3d : public Delaunay {
    public:
        /**
         * \brief Constructs a new ParallelDelaunay3d.
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
        ParallelDelaunay3d(coord_index_t dimension = 3);

	/**
	 * \copydoc Delaunay::set_vertices
	 */
        virtual void set_vertices(
            index_t nb_vertices, const double* vertices
        );

	/**
	 * \copydoc Delaunay::nearest_vertex()
	 */
        virtual index_t nearest_vertex(const double* p) const;

	/**
	 * \copydoc Delaunay::set_BRIO_levels()
	 */
        virtual void set_BRIO_levels(const vector<index_t>& levels);

    private:
        vector<signed_index_t> cell_to_v_store_;
        vector<signed_index_t> cell_to_cell_store_;
        vector<index_t> cell_next_;
        vector<thread_index_t> cell_thread_;
        ThreadGroup threads_;
        bool weighted_; // true for regular triangulation.
        vector<double> heights_; // only used in weighted mode.        
        vector<index_t> reorder_;
        vector<index_t> levels_;

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
        
        
        friend class Delaunay3dThread;
    };
    

}

#endif

#endif
