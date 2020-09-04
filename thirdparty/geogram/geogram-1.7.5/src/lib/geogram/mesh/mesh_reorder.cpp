/*
 *  Copyright (c) 2012-2014, Bruno Levy All rights reserved.
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

#include <geogram/mesh/mesh_reorder.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/mesh/index.h>
#include <geogram/delaunay/periodic.h>
#include <geogram/basic/permutation.h>
#include <geogram/basic/process.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/algorithm.h>
#include <geogram/bibliography/bibliography.h>

#include <random>

namespace {

    using namespace GEO;

    /**
     * \brief Splits a sequence into two ordered halves.
     * \details The algorithm shuffles the sequence and
     *  partitions its into two halves with the same number of elements
     *  and such that the elements of the first half are smaller
     *  than the elements of the second half.
     * \param[in] begin an iterator to the first element
     * \param[in] end an iterator one position past the last element
     * \param[in] cmp the comparator object
     * \return an iterator to the middle of the sequence that separates
     *  the two halves
     */
    template <class IT, class CMP>
    inline IT reorder_split(
        IT begin, IT end, CMP cmp
    ) {
        if(begin >= end) {
            return begin;
        }
        IT middle = begin + (end - begin) / 2;
        std::nth_element(begin, middle, end, cmp);
        return middle;
    }

    /************************************************************************/

    /**
     * \brief Used by VertexMesh.
     * \details Exposes an interface compatible with the requirement
     * of Hilbert sort templates for a raw array of vertices.
     */
    class VertexArray {
    public:

        /**
         * \brief Constructs a new VertexArray.
         * \param[in] base address of the points
         * \param[in] stride number of doubles between
         *  two consecutive points
         */
        VertexArray(
            index_t nb_vertices,
            const double* base, index_t stride
        ) :
            base_(base),
            stride_(stride) {
            nb_vertices_ = nb_vertices;
        }

        /**
         * \brief Gets a vertex by its index.
         * \param[in] i the index of the point
         * \return a const pointer to the coordinates of the vertex
         */
        const double* point_ptr(index_t i) const {
            geo_debug_assert(i < nb_vertices_);
            return base_ + i * stride_;
        }

    private:
        const double* base_;
        index_t stride_;
        index_t nb_vertices_;
    };


    /**
     * \brief Exposes an interface compatible with the requirement
     * of Hilbert sort templates for a raw array of vertices.
     */
    class VertexMesh {
    public:
        /**
         * \brief Constructs a new VertexMesh.
         * \param[in] base address of the points
         * \param[in] stride number of doubles between
         *  two consecutive points
         */
        VertexMesh(
            index_t nb_vertices,
            const double* base, index_t stride
        ) : vertices(nb_vertices, base, stride) {
        }
        VertexArray vertices;
    };
    
    /************************************************************************/

    /**
     * \brief The generic comparator class for Hilbert vertex
     *  ordering.
     * \tparam COORD the coordinate to compare
     * \tparam UP    if true, use direct order, else use reverse order
     * \tparam MESH  the class that represents meshes
     */
    template <int COORD, bool UP, class MESH>
    struct Hilbert_vcmp {
    };

    /**
     * \brief Specialization (UP=true) of the generic comparator class
     *  for Hilbert vertex ordering.
     * \see Hilbert_vcmp
     * \tparam COORD the coordinate to compare
     * \tparam MESH  the class that represents meshes
     */
    template <int COORD, class MESH>
    struct Hilbert_vcmp<COORD, true, MESH> {

        /**
         * \brief Constructs a new Hilbert_vcmp.
         * \param[in] mesh the mesh in which the compared
         *  points reside.
         */
        Hilbert_vcmp(const MESH& mesh) :
            mesh_(mesh) {
        }

        /**
         * \brief Compares two points.
         * \param[in] i1 index of the first point to compare
         * \param[in] i2 index of the second point to compare
         * \return true if point \p i1 is before point \p i2,
         *  false otherwise.
         */
        bool operator() (index_t i1, index_t i2) {
            return
                mesh_.vertices.point_ptr(i1)[COORD] <
                mesh_.vertices.point_ptr(i2)[COORD];
        }

        const MESH& mesh_;
    };

    /**
     * \brief Specialization (UP=false) of the generic comparator class
     *  for Hilbert vertex ordering.
     * \see Hilbert_vcmp
     * \tparam COORD the coordinate to compare
     * \tparam MESH  the class that represents meshes
     */
    template <int COORD, class MESH>
    struct Hilbert_vcmp<COORD, false, MESH> {

        /**
         * \brief Constructs a new Hilbert_vcmp.
         * \param[in] mesh the mesh in which the compared
         *  points reside.
         */
        Hilbert_vcmp(const MESH& mesh) :
            mesh_(mesh) {
        }

        /**
         * \brief Compares two points.
         * \param[in] i1 index of the first point to compare
         * \param[in] i2 index of the second point to compare
         * \return true if point \p i1 is before point \p i2,
         *  false otherwise.
         */
        bool operator() (index_t i1, index_t i2) {
            return
                mesh_.vertices.point_ptr(i1)[COORD] >
                mesh_.vertices.point_ptr(i2)[COORD];
        }

        const MESH& mesh_;
    };

    /************************************************************************/

    /**
     * \brief Comparator class for Morton vertex
     *  ordering.
     * \tparam COORD the coordinate to compare
     * \tparam UP ignored in Morton order
     * \tparam MESH  the class that represents meshes
     */
    template <int COORD, bool UP, class MESH>
    struct Morton_vcmp {

        /**
         * \brief Constructs a new Morton_vcmp.
         * \param[in] mesh the mesh in which the compared
         *  points reside.
         */
        Morton_vcmp(const MESH& mesh) :
            mesh_(mesh) {
        }

        /**
         * \brief Compares two points.
         * \param[in] i1 index of the first point to compare
         * \param[in] i2 index of the second point to compare
         * \return true if point \p i1 is before point \p i2,
         *  false otherwise.
         */
        bool operator() (index_t i1, index_t i2) {
            return
                mesh_.vertices.point_ptr(i1)[COORD] <
                mesh_.vertices.point_ptr(i2)[COORD];
        }

        const MESH& mesh_;
    };

    /************************************************************************/

#ifndef GEOGRAM_PSM
    
    /**
     * \brief Base class for facets ordering.
     * \tparam COORD the coordinate to compare
     * \tparam MESH  the class that represents meshes
     */
    template <int COORD, class MESH>
    class Base_fcmp {
    public:
        /**
         * \brief Constructs a new Base_vcmp.
         * \param[in] mesh the mesh in which the compared
         *  facets reside.
         */
        Base_fcmp(const MESH& mesh) :
            mesh_(mesh) {
        }

        /**
         * \brief Computes the compared coordinate from a facet index.
         * \param[in] f the index of the facet
         * \return the coordinate at the center of facet \p f
         */
        double center(index_t f) const {
            double result = 0.0;
	    double s = 1.0 / double(mesh_.facets.nb_vertices(f));
            for(index_t c: mesh_.facets.corners(f)) {
                result += s*mesh_.vertices.point_ptr(
                    mesh_.facet_corners.vertex(c)
		)[COORD]; 
            }
            return result;
        }

    private:
        const MESH& mesh_;
    };

    /**
     * \brief The generic comparator class for Hilbert facet
     *  ordering.
     * \tparam COORD the coordinate to compare
     * \tparam UP    if true, use direct order, else use reverse order
     * \tparam MESH  the class that represents meshes
     */
    template <int COORD, bool UP, class MESH>
    struct Hilbert_fcmp {
    };

    /**
     * \brief Specialization (UP=true) of the generic comparator class
     *  for Hilbert vertex ordering.
     * \see Hilbert_vcmp
     * \tparam COORD the coordinate to compare
     * \tparam MESH  the class that represents meshes
     */
    template <int COORD, class MESH>
    class Hilbert_fcmp<COORD, true, MESH> : public Base_fcmp<COORD, MESH> {
    public:
        /**
         * \brief Constructs a new Hilbert_fcmp.
         * \param[in] mesh the mesh in which the compared
         *  facets reside.
         */
        Hilbert_fcmp(const MESH& mesh) :
            Base_fcmp<COORD, MESH>(mesh) {
        }

        /**
         * \brief Compares two facets.
         * \param[in] f1 index of the first facet to compare
         * \param[in] f2 index of the second facet to compare
         * \return true if facet \p f1 is before facet \p f2,
         *  false otherwise.
         */
        bool operator() (index_t f1, index_t f2) {
            return this->center(f1) < this->center(f2);
        }
    };

    /**
     * \brief Specialization (UP=false) of the generic comparator class
     *  for Hilbert vertex ordering.
     * \see Hilbert_vcmp
     * \tparam COORD the coordinate to compare
     * \tparam MESH  the class that represents meshes
     */
    template <int COORD, class MESH>
    class Hilbert_fcmp<COORD, false, MESH> : public Base_fcmp<COORD, MESH> {
    public:
        /**
         * \brief Constructs a new Hilbert_fcmp.
         * \param[in] mesh the mesh in which the compared
         *  facets reside.
         */
        Hilbert_fcmp(const MESH& mesh) :
            Base_fcmp<COORD, MESH>(mesh) {
        }

        /**
         * \brief Compares two facets.
         * \param[in] f1 index of the first facet to compare
         * \param[in] f2 index of the second facet to compare
         * \return true if facet \p f1 is before facet \p f2,
         *  false otherwise.
         */
        bool operator() (index_t f1, index_t f2) {
            return this->center(f1) > this->center(f2);
        }
    };

    /**
     * \brief Comparator class for Morton facet
     *  ordering.
     * \tparam COORD the coordinate to compare
     * \tparam UP ignored in Morton order
     * \tparam MESH  the class that represents meshes
     */
    template <int COORD, bool UP, class MESH>
    class Morton_fcmp : public Base_fcmp<COORD, MESH> {
    public:
        /**
         * \brief Constructs a new Morton_fcmp.
         * \param[in] mesh the mesh in which the compared
         *  facets reside.
         */
        Morton_fcmp(const MESH& mesh) :
            Base_fcmp<COORD, MESH>(mesh) {
        }

        /**
         * \brief Compares two facets.
         * \param[in] f1 index of the first facet to compare
         * \param[in] f2 index of the second facet to compare
         * \return true if facet \p f1 is before facet \p f2,
         *  false otherwise.
         */
        bool operator() (index_t f1, index_t f2) {
            return this->center(f1) < this->center(f2);
        }
    };

    /************************************************************************/

    /**
     * \brief Base class for tetrahedra ordering.
     * \tparam COORD the coordinate to compare
     * \tparam MESH  the class that represents meshes
     */
    template <int COORD, class MESH>
    class Base_tcmp {
    public:
        /**
         * \brief Constructs a new Base_tcmp.
         * \param[in] mesh the mesh in which the compared
         *  tetrahedra reside.
         */
        Base_tcmp(const MESH& mesh) :
            mesh_(mesh) {
        }

        /**
         * \brief Computes the compared coordinate from a tetra index.
         * \param[in] t the index of the tetra
         * \return the coordinate at the center of tetra \p f
         */
        double center(index_t t) const {
            double result = 0.0;
            for(index_t lv = 0; lv < 4; ++lv) {
                result += mesh_.vertices.point_ptr(
                    mesh_.cells.vertex(t, lv)
                )[COORD];
            }
            return result;
        }

    private:
        const MESH& mesh_;
    };

    /**
     * \brief The generic comparator class for Hilbert tetra
     *  ordering.
     * \tparam COORD the coordinate to compare
     * \tparam UP    if true, use direct order, else use reverse order
     * \tparam MESH  the class that represents meshes
     */
    template <int COORD, bool UP, class MESH>
    struct Hilbert_tcmp {
    };

    /**
     * \brief Specialization (UP=true) of the generic comparator class
     *  for Hilbert tetra ordering.
     * \see Hilbert_tcmp
     * \tparam COORD the coordinate to compare
     * \tparam MESH  the class that represents meshes
     */
    template <int COORD, class MESH>
    class Hilbert_tcmp<COORD, true, MESH> : public Base_tcmp<COORD, MESH> {
    public:
        /**
         * \brief Constructs a new Hilbert_tcmp.
         * \param[in] mesh the mesh in which the compared
         *  tetrahedra reside.
         */
        Hilbert_tcmp(const MESH& mesh) :
            Base_tcmp<COORD, MESH>(mesh) {
        }

        /**
         * \brief Compares two tetrahedra.
         * \param[in] t1 index of the first tetra to compare
         * \param[in] t2 index of the second tetra to compare
         * \return true if tetra \p t1 is before tetra \p t2,
         *  false otherwise.
         */
        bool operator() (index_t t1, index_t t2) {
            return this->center(t1) < this->center(t2);
        }
    };

    /**
     * \brief Specialization (UP=false) of the generic comparator class
     *  for Hilbert tetra ordering.
     * \see Hilbert_tcmp
     * \tparam COORD the coordinate to compare
     * \tparam MESH  the class that represents meshes
     */
    template <int COORD, class MESH>
    class Hilbert_tcmp<COORD, false, MESH> : public Base_tcmp<COORD, MESH> {
    public:
        /**
         * \brief Constructs a new Hilbert_tcmp.
         * \param[in] mesh the mesh in which the compared
         *  tetrahedra reside.
         */
        Hilbert_tcmp(const MESH& mesh) :
            Base_tcmp<COORD, MESH>(mesh) {
        }

        /**
         * \brief Compares two tetrahedra.
         * \param[in] t1 index of the first tetra to compare
         * \param[in] t2 index of the second tetra to compare
         * \return true if tetra \p t1 is before tetra \p t2,
         *  false otherwise.
         */
        bool operator() (index_t t1, index_t t2) {
            return this->center(t1) > this->center(t2);
        }
    };

    /**
     * \brief Comparator class for Morton tetra
     *  ordering.
     * \tparam COORD the coordinate to compare
     * \tparam UP ignored in Morton order
     * \tparam MESH  the class that represents meshes
     */
    template <int COORD, bool UP, class MESH>
    class Morton_tcmp : public Base_tcmp<COORD, MESH> {
    public:
        /**
         * \brief Constructs a new Morton_tcmp.
         * \param[in] mesh the mesh in which the compared
         *  tetrahedra reside.
         */
        Morton_tcmp(const MESH& mesh) :
            Base_tcmp<COORD, MESH>(mesh) {
        }

        /**
         * \brief Compares two tetrahedra.
         * \param[in] t1 index of the first tetra to compare
         * \param[in] t2 index of the second tetra to compare
         * \return true if tetra \p t1 is before tetra \p t2,
         *  false otherwise.
         */
        bool operator() (index_t t1, index_t t2) {
            return this->center(t1) < this->center(t2);
        }
    };

    /************************************************************************/
    
    /**
     * \brief Base class for cells ordering.
     * \tparam COORD the coordinate to compare
     * \tparam MESH  the class that represents meshes
     */
    template <int COORD, class MESH>
    class Base_ccmp {
    public:
        /**
         * \brief Constructs a new Base_ccmp.
         * \param[in] mesh the mesh in which the compared
         *  cells reside.
         */
        Base_ccmp(const MESH& mesh) :
            mesh_(mesh) {
        }

        /**
         * \brief Computes the compared coordinate from a cell index.
         * \param[in] c the index of the cell
         * \return the coordinate at the center of cell \p c
         */
        double center(index_t c) const {
            double result = 0.0;
            for(index_t lv = 0; lv < mesh_.cells.nb_vertices(c); ++lv) {
                result += mesh_.vertices.point_ptr(
                    mesh_.cells.vertex(c, lv)
                )[COORD];
            }
            return result / double(mesh_.cells.nb_vertices(c));
        }

    private:
        const MESH& mesh_;
    };

    /**
     * \brief The generic comparator class for Hilbert cell
     *  ordering.
     * \tparam COORD the coordinate to compare
     * \tparam UP    if true, use direct order, else use reverse order
     * \tparam MESH  the class that represents meshes
     */
    template <int COORD, bool UP, class MESH>
    struct Hilbert_ccmp {
    };

    /**
     * \brief Specialization (UP=true) of the generic comparator class
     *  for Hilbert cell ordering.
     * \see Hilbert_ccmp
     * \tparam COORD the coordinate to compare
     * \tparam MESH  the class that represents meshes
     */
    template <int COORD, class MESH>
    class Hilbert_ccmp<COORD, true, MESH> : public Base_ccmp<COORD, MESH> {
    public:
        /**
         * \brief Constructs a new Hilbert_ccmp.
         * \param[in] mesh the mesh in which the compared
         *  cells reside.
         */
        Hilbert_ccmp(const MESH& mesh) :
            Base_ccmp<COORD, MESH>(mesh) {
        }

        /**
         * \brief Compares two cells
         * \param[in] c1 index of the first cell to compare
         * \param[in] c2 index of the second cell to compare
         * \return true if cell \p c1 is before cell \p c2,
         *  false otherwise.
         */
        bool operator() (index_t c1, index_t c2) {
            return this->center(c1) < this->center(c2);
        }
    };

    /**
     * \brief Specialization (UP=false) of the generic comparator class
     *  for Hilbert cell ordering.
     * \see Hilbert_ccmp
     * \tparam COORD the coordinate to compare
     * \tparam MESH  the class that represents meshes
     */
    template <int COORD, class MESH>
    class Hilbert_ccmp<COORD, false, MESH> : public Base_ccmp<COORD, MESH> {
    public:
        /**
         * \brief Constructs a new Hilbert_ccmp.
         * \param[in] mesh the mesh in which the compared
         *  tetrahedra reside.
         */
        Hilbert_ccmp(const MESH& mesh) :
            Base_ccmp<COORD, MESH>(mesh) {
        }

        /**
         * \brief Compares two cells.
         * \param[in] c1 index of the first cell to compare
         * \param[in] c2 index of the second cell to compare
         * \return true if cell \p c1 is before cell \p c2,
         *  false otherwise.
         */
        bool operator() (index_t c1, index_t c2) {
            return this->center(c1) > this->center(c2);
        }
    };

    /**
     * \brief Comparator class for Morton cell
     *  ordering.
     * \tparam COORD the coordinate to compare
     * \tparam UP ignored in Morton order
     * \tparam MESH  the class that represents meshes
     */
    template <int COORD, bool UP, class MESH>
    class Morton_ccmp : public Base_ccmp<COORD, MESH> {
    public:
        /**
         * \brief Constructs a new Morton_ccmp.
         * \param[in] mesh the mesh in which the compared
         *  cells reside.
         */
        Morton_ccmp(const MESH& mesh) :
            Base_ccmp<COORD, MESH>(mesh) {
        }

        /**
         * \brief Compares two tetrahedra.
         * \param[in] c1 index of the first cell to compare
         * \param[in] c2 index of the second cell to compare
         * \return true if cell \p c1 is before cell \p c2,
         *  false otherwise.
         */
        bool operator() (index_t c1, index_t c2) {
            return this->center(c1) < this->center(c2);
        }
    };

#endif
    
    /************************************************************************/
    
    /**
     * \brief Generic class for sorting arbitrary elements in
     *  Hilbert and Morton orders in 3d.
     * \details The implementation is inspired by:
     *  - Christophe Delage and Olivier Devillers. Spatial Sorting. 
     *   In CGAL User and Reference Manual. CGAL Editorial Board, 
     *   3.9 edition, 2011
     * \tparam CMP the comparator class for ordering the elements. CMP
     *  is itself a template parameterized by~:
     *    - COORD the coordinate along which elements should be
     *      sorted
     *    - UP a boolean that indicates whether direct or reverse
     *      order should be used
     *    - MESH the class that represents meshes
     * \tparam MESH  the class that represents meshes
     */
    template <template <int COORD, bool UP, class MESH> class CMP, class MESH>
    struct HilbertSort3d {

        /**
         * \brief Low-level recursive spatial sorting function
         * \details This function is recursive
         * \param[in] M the mesh in which the elements reside
         * \param[in] begin an iterator that points to the
         *  first element of the sequence
         * \param[in] end an iterator that points one position past the
         *  last element of the sequence
         * \param[in] limit subsequences smaller than limit are left unsorted
         * \tparam COORDX the first coordinate, can be 0,1 or 2. The second
         *  and third coordinates are COORDX+1 modulo 3 and COORDX+2 modulo 3
         *  respectively
         * \tparam UPX whether ordering along the first coordinate
         *  is direct or inverse
         * \tparam UPY whether ordering along the second coordinate
         *  is direct or inverse
         * \tparam UPZ whether ordering along the third coordinate
         *  is direct or inverse
         */
        template <int COORDX, bool UPX, bool UPY, bool UPZ, class IT>
        static void sort(
            const MESH& M, IT begin, IT end, index_t limit = 1
        ) {
            const int COORDY = (COORDX + 1) % 3, COORDZ = (COORDY + 1) % 3;
            if(end - begin <= signed_index_t(limit)) {
                return;
            }
            IT m0 = begin, m8 = end;
            IT m4 = reorder_split(m0, m8, CMP<COORDX, UPX, MESH>(M));
            IT m2 = reorder_split(m0, m4, CMP<COORDY, UPY, MESH>(M));
            IT m1 = reorder_split(m0, m2, CMP<COORDZ, UPZ, MESH>(M));
            IT m3 = reorder_split(m2, m4, CMP<COORDZ, !UPZ, MESH>(M));
            IT m6 = reorder_split(m4, m8, CMP<COORDY, !UPY, MESH>(M));
            IT m5 = reorder_split(m4, m6, CMP<COORDZ, UPZ, MESH>(M));
            IT m7 = reorder_split(m6, m8, CMP<COORDZ, !UPZ, MESH>(M));
            sort<COORDZ, UPZ, UPX, UPY>(M, m0, m1);
            sort<COORDY, UPY, UPZ, UPX>(M, m1, m2);
            sort<COORDY, UPY, UPZ, UPX>(M, m2, m3);
            sort<COORDX, UPX, !UPY, !UPZ>(M, m3, m4);
            sort<COORDX, UPX, !UPY, !UPZ>(M, m4, m5);
            sort<COORDY, !UPY, UPZ, !UPX>(M, m5, m6);
            sort<COORDY, !UPY, UPZ, !UPX>(M, m6, m7);
            sort<COORDZ, !UPZ, !UPX, UPY>(M, m7, m8);
        }

        /**
         * \brief Sorts a sequence of elements spatially.
         * \details This function does an indirect sort, 
         *  in the sense that a sequence 
         *  of indices that refer to the elements is sorted. 
         *  This function uses a multithreaded implementation.
         * \param[in] M the mesh in which the elements to sort reside
         * \param[in] b an iterator to the first index to be sorted
         * \param[in] e an iterator one position past the last index 
         *  to be sorted
         * \param[in] limit subsequences smaller than limit are left unsorted
         */
        HilbertSort3d(
            const MESH& M,
            vector<index_t>::iterator b,
            vector<index_t>::iterator e,
            index_t limit = 1
        ) :
            M_(M)
        {
            geo_debug_assert(e >= b);
	    geo_cite_with_info(
		"WEB:SpatialSorting",
		"The implementation of spatial sort in GEOGRAM is inspired by "
		"the idea of using \\verb|std::nth_element()| and the recursive"
                " template in the spatial sort package of CGAL"
	    );

            // If the sequence is smaller than the limit, skip it
            if(index_t(e - b) <= limit) {
                return;
            }

            // If the sequence is smaller than 1024, use sequential sorting
            if(index_t(e - b) < 1024) {
                sort<0, false, false, false>(M_, b, e);
                return;
            }

            // Parallel sorting (2 then 4 then 8 sorts in parallel)

// Unfortunately we cannot access consts for template arguments in lambdas in all
// compilers (gcc is OK but not MSVC) so I'm using (ugly) macros here...
	    
#          define COORDX 0
#          define COORDY 1
#          define COORDZ 2
#          define UPX false
#          define UPY false
#          define UPZ false
	    
            m0_ = b;
            m8_ = e;
            m4_ = reorder_split(m0_, m8_, CMP<COORDX, UPX, MESH>(M));

	    
	    parallel(
		[this]() { m2_ = reorder_split(m0_, m4_, CMP<COORDY,  UPY, MESH>(M_)); },
		[this]() { m6_ = reorder_split(m4_, m8_, CMP<COORDY, !UPY, MESH>(M_)); }
	    );

	    parallel(
		[this]() { m1_ = reorder_split(m0_, m2_, CMP<COORDZ,  UPZ, MESH>(M_)); },
		[this]() { m3_ = reorder_split(m2_, m4_, CMP<COORDZ, !UPZ, MESH>(M_)); },
		[this]() { m5_ = reorder_split(m4_, m6_, CMP<COORDZ,  UPZ, MESH>(M_)); },
		[this]() { m7_ = reorder_split(m6_, m8_, CMP<COORDZ, !UPZ, MESH>(M_)); }
	    );

	    parallel(
		[this]() { sort<COORDZ,  UPZ,  UPX,  UPY>(M_, m0_, m1_); },
		[this]() { sort<COORDY,  UPY,  UPZ,  UPX>(M_, m1_, m2_); },
		[this]() { sort<COORDY,  UPY,  UPZ,  UPX>(M_, m2_, m3_); },
		[this]() { sort<COORDX,  UPX, !UPY, !UPZ>(M_, m3_, m4_); },
		[this]() { sort<COORDX,  UPX, !UPY, !UPZ>(M_, m4_, m5_); },
		[this]() { sort<COORDY, !UPY,  UPZ, !UPX>(M_, m5_, m6_); },
		[this]() { sort<COORDY, !UPY,  UPZ, !UPX>(M_, m6_, m7_); },
		[this]() { sort<COORDZ, !UPZ, !UPX,  UPY>(M_, m7_, m8_); }
	    );

#          undef COORDX
#          undef COORDY
#          undef COORDZ
#          undef UPX
#          undef UPY
#          undef UPZ	    
        }

    private:
        const MESH& M_;
        vector<index_t>::iterator
            m0_, m1_, m2_, m3_, m4_, m5_, m6_, m7_, m8_;
    };

    /************************************************************************/

    /**
     * \brief Generic class for sorting arbitrary elements in
     *  Hilbert and Morton orders in 3d.
     * \details The implementation is inspired by:
     *  - Christophe Delage and Olivier Devillers. Spatial Sorting. 
     *   In CGAL User and Reference Manual. CGAL Editorial Board, 
     *   3.9 edition, 2011
     * \tparam CMP the comparator class for ordering the elements. CMP
     *  is itself a template parameterized by~:
     *    - COORD the coordinate along which elements should be
     *      sorted
     *    - UP a boolean that indicates whether direct or reverse
     *      order should be used
     *    - MESH the class that represents meshes
     * \tparam MESH  the class that represents meshes
     */
    template <template <int COORD, bool UP, class MESH> class CMP, class MESH>
    struct HilbertSort2d {

        /**
         * \brief Low-level recursive spatial sorting function
         * \details This function is recursive
         * \param[in] M the mesh in which the elements reside
         * \param[in] begin an iterator that points to the
         *  first element of the sequence
         * \param[in] end an iterator that points one position past the
         *  last element of the sequence
         * \param[in] limit subsequences smaller than limit are left unsorted
         * \tparam COORDX the first coordinate, can be 0,1 or 2. The second
         *  coordinate is COORDX+1 modulo 2. 
         * \tparam UPX whether ordering along the first coordinate
         *  is direct or inverse
         * \tparam UPY whether ordering along the second coordinate
         *  is direct or inverse
         */
        template <int COORDX, bool UPX, bool UPY, class IT>
        static void sort(
            const MESH& M, IT begin, IT end, index_t limit = 1
        ) {
            const int COORDY = (COORDX + 1) % 2;
            if(end - begin <= signed_index_t(limit)) {
                return;
            }
	    IT m0 = begin, m4 = end;

	    IT m2 = reorder_split (m0, m4, CMP<COORDX,  UPX, MESH>(M));
	    IT m1 = reorder_split (m0, m2, CMP<COORDY,  UPY, MESH>(M));
	    IT m3 = reorder_split (m2, m4, CMP<COORDY, !UPY, MESH>(M));

	    sort<COORDY, UPY, UPX> (M, m0, m1);
	    sort<COORDX, UPX, UPY> (M, m1, m2);
	    sort<COORDX, UPX, UPY> (M, m2, m3);
	    sort<COORDY,!UPY,!UPX> (M, m3, m4);
        }

        /**
         * \brief Sorts a sequence of elements spatially.
         * \details This function does an indirect sort, 
         *  in the sense that a sequence 
         *  of indices that refer to the elements is sorted. 
         *  This function uses a multithreaded implementation.
         * \param[in] M the mesh in which the elements to sort reside
         * \param[in] b an iterator to the first index to be sorted
         * \param[in] e an iterator one position past the last index 
         *  to be sorted
         * \param[in] limit subsequences smaller than limit are left unsorted
         */
        HilbertSort2d(
            const MESH& M,
            vector<index_t>::iterator b,
            vector<index_t>::iterator e,
            index_t limit = 1
        ) :
            M_(M)
        {
            geo_debug_assert(e > b);
	    geo_cite_with_info(
		"WEB:SpatialSorting",
		"The implementation of spatial sort in GEOGRAM is inspired by "
		"the idea of using \\verb|std::nth_element()| and the recursive"
                " template in the spatial sort package of CGAL"
	    );

            // If the sequence is smaller than the limit, skip it
            if(index_t(e - b) <= limit) {
                return;
            }
	    sort<0, false, false>(M_, b, e);
        }
    private:
        const MESH& M_;
    };

    /************************************************************************/

#ifndef GEOGRAM_PSM
    
    /**
     * \brief Sorts the vertices of a mesh according to the Hilbert ordering.
     * \details The function does not change the mesh, it computes instead
     *  the permutation. The permutation can then be reused to order other
     *  arrays that may depend on the order of the vertices in the mesh (i.e.
     *  attributes).
     * \param[in] M the mesh where the vertices to be sorted reside
     * \param[out] sorted_indices the permutation to be applied
       to the vertices
     */
    void hilbert_vsort_3d(
        const Mesh& M, vector<index_t>& sorted_indices
    ) {
        sorted_indices.resize(M.vertices.nb());
        for(index_t i: M.vertices) {
            sorted_indices[i] = i;
        }
        HilbertSort3d<Hilbert_vcmp, Mesh>(
            M, sorted_indices.begin(), sorted_indices.end()
        );
    }

    /**
     * \brief Sorts the facets of a mesh according to the Hilbert ordering.
     * \details The function does not change the mesh, it computes instead
     *  the permutation. The permutation can then be reused to order other
     *  arrays that may depend on the order of the facets in the mesh (i.e.
     *  attributes).
     * \param[in] M the mesh where the facets to be sorted reside
     * \param[out] sorted_indices the permutation to be
     *  applied to the facets
     */
    void hilbert_fsort_3d(
        const Mesh& M, vector<index_t>& sorted_indices
    ) {
        sorted_indices.resize(M.facets.nb());
        for(index_t i: M.facets) {
            sorted_indices[i] = i;
        }
        HilbertSort3d<Hilbert_fcmp, Mesh>(
            M, sorted_indices.begin(), sorted_indices.end()
        );
    }

    /**
     * \brief Sorts the cells of a mesh according to the Hilbert ordering.
     * \details The function does not change the mesh, it computes instead
     *  the permutation. The permutation can then be reused to order other
     *  arrays that may depend on the order of the tets in the mesh (i.e.
     *  attributes).
     * \param[in] M the mesh where the cells to be sorted reside
     * \param[out] sorted_indices the permutation to be applied to the tets
     */
    void hilbert_csort_3d(
        const Mesh& M, vector<index_t>& sorted_indices
    ) {
        sorted_indices.resize(M.cells.nb());
        for(index_t i: M.cells) {
            sorted_indices[i] = i;
        }
        if(M.cells.are_simplices()) {
            HilbertSort3d<Hilbert_tcmp, Mesh>(
                M, sorted_indices.begin(), sorted_indices.end()
            );
        } else {
            HilbertSort3d<Hilbert_ccmp, Mesh>(
                M, sorted_indices.begin(), sorted_indices.end()
            );
        }
    }

    /**
     * \brief Sorts the vertices of a mesh according to the Morton ordering.
     * \details The function does not change the mesh, it computes instead
     *  the permutation. The permutation can then be reused to order other
     *  arrays that may depend on the order of the vertices in the mesh (i.e.
     *  attributes).
     * \param[in] M the mesh where the vertices to be sorted reside
     * \param[out] sorted_indices the permutation to be applied to the vertices
     */
    void morton_vsort_3d(
        const Mesh& M, vector<index_t>& sorted_indices
    ) {
        sorted_indices.resize(M.vertices.nb());
        for(index_t i: M.vertices) {
            sorted_indices[i] = i;
        }
        HilbertSort3d<Morton_vcmp, Mesh>(
            M, sorted_indices.begin(), sorted_indices.end()
        );
    }

    /**
     * \brief Sorts the facets of a mesh according to the Morton ordering.
     * \details The function does not change the mesh, it computes instead
     *  the permutation. The permutation can then be reused to order other
     *  arrays that may depend on the order of the facets in the mesh (i.e.
     *  attributes).
     * \param[in] M the mesh where the facets to be sorted reside
     * \param[out] sorted_indices the permutation to be applied to the facets
     */
    void morton_fsort_3d(
        const Mesh& M, vector<index_t>& sorted_indices
    ) {
        sorted_indices.resize(M.facets.nb());
        for(index_t i: M.facets) {
            sorted_indices[i] = i;
        }
        HilbertSort3d<Morton_fcmp, Mesh>(
            M, sorted_indices.begin(), sorted_indices.end()
        );
    }

    /**
     * \brief Sorts the cells of a mesh according to the Morton ordering.
     * \details The function does not change the mesh, it computes instead
     *  the permutation. The permutation can then be reused to order other
     *  arrays that may depend on the order of the tets in the mesh (i.e.
     *  attributes).
     * \param[in] M the mesh where the tets to be sorted reside
     * \param[out] sorted_indices the permutation to be applied to the tets
     */
    void morton_csort_3d(
        const Mesh& M, vector<index_t>& sorted_indices
    ) {
        sorted_indices.resize(M.cells.nb());
        for(index_t i: M.cells) {
            sorted_indices[i] = i;
        }
        if(M.cells.are_simplices()) {
            HilbertSort3d<Morton_tcmp, Mesh>(
                M, sorted_indices.begin(), sorted_indices.end()
            );
        } else {
            HilbertSort3d<Morton_ccmp, Mesh>(
                M, sorted_indices.begin(), sorted_indices.end()
            );
        }
    }

#endif    
    
    /**
     * \brief Computes the BRIO order for a set of 3D points.
     * \details Implementation of compute_BRIO_order(). 
     *  It is used to accelerate incremental insertion in Delaunay triangulation
     * \param[in] nb_vertices number of vertices to sort
     * \param[in] vertices pointer to the coordinates of the vertices
     * \param[in] stride number of doubles between two consecutive vertices
     * \param[in,out] sorted_indices indices to sort
     * \param[in] b iterator to the first index to sort
     * \param[in] e iterator one position past the last index to sort
     * \param[in] threshold minimum size of interval to be sorted
     * \param[in] ratio splitting ratio between current interval and
     *  the rest to be sorted
     * \param[in,out] depth iteration depth
     * \param[out] levels if non-null, bounds of each level
     */
    void compute_BRIO_order_recursive(
        index_t nb_vertices, const double* vertices,
        index_t dimension, index_t stride,
        vector<index_t>& sorted_indices,
        vector<index_t>::iterator b,
        vector<index_t>::iterator e,
        index_t threshold,
        double ratio,
        index_t& depth,
        vector<index_t>* levels
    ) {
        geo_debug_assert(e > b);

        vector<index_t>::iterator m = b;
        if(index_t(e - b) > threshold) {
            ++depth;
            m = b + int(double(e - b) * ratio);
            compute_BRIO_order_recursive(
                nb_vertices, vertices,
		dimension, stride,
                sorted_indices, b, m,
                threshold, ratio, depth,
                levels
            );
        }

        VertexMesh M(nb_vertices, vertices, stride);
	if(dimension == 3) {
	    HilbertSort3d<Hilbert_vcmp, VertexMesh>(
		M, m, e
	    );
	} else if(dimension ==2) {
	    HilbertSort2d<Hilbert_vcmp, VertexMesh>(
		M, m, e
	    );
	} else {
	    geo_assert_not_reached;
	}

        if(levels != nullptr) {
            levels->push_back(index_t(e - sorted_indices.begin()));
        }
    }
}

/****************************************************************************/

namespace GEO {

#ifndef GEOGRAM_PSM
    
    void mesh_reorder(Mesh& M, MeshOrder order) {

        geo_assert(M.vertices.dimension() >= 3);

        // Step 1: reorder vertices
        {
            vector<index_t> sorted_indices;
            switch(order) {
                case MESH_ORDER_HILBERT:
                    hilbert_vsort_3d(M, sorted_indices);
                    break;
                case MESH_ORDER_MORTON:
                    morton_vsort_3d(M, sorted_indices);
                    break;
            }
            M.vertices.permute_elements(sorted_indices);
        }

        // Step 2: reorder facets
        if(M.facets.nb() != 0) {
            vector<index_t> sorted_indices;
            switch(order) {
                case MESH_ORDER_HILBERT:
                    hilbert_fsort_3d(M, sorted_indices);
                    break;
                case MESH_ORDER_MORTON:
                    morton_fsort_3d(M, sorted_indices);
                    break;
            }
            M.facets.permute_elements(sorted_indices);
        }

        // Step 3: reorder cells
        if(M.cells.nb() != 0) {
            vector<index_t> sorted_indices;
            switch(order) {
                case MESH_ORDER_HILBERT:
                    hilbert_csort_3d(M, sorted_indices);
                    break;
                case MESH_ORDER_MORTON:
                    morton_csort_3d(M, sorted_indices);
                    break;
            }
            M.cells.permute_elements(sorted_indices);
        }
    }

#endif


    void compute_Hilbert_order(
        index_t total_nb_vertices, const double* vertices,
        vector<index_t>& sorted_indices,
        index_t first,
        index_t last,
        index_t dimension, index_t stride
    ) {
        geo_debug_assert(last > first);
        if(last - first <= 1) {
            return;
        }
        VertexMesh M(total_nb_vertices, vertices, stride);
	if(dimension == 3) {
	    HilbertSort3d<Hilbert_vcmp, VertexMesh>(
		M, sorted_indices.begin() + int(first),
		sorted_indices.begin() + int(last)
	    );
	} else if(dimension == 2) {
	    HilbertSort2d<Hilbert_vcmp, VertexMesh>(
		M, sorted_indices.begin() + int(first),
		sorted_indices.begin() + int(last)
	    );
	} else {
	    geo_assert_not_reached;
	}
    }
    
    void compute_BRIO_order(
        index_t nb_vertices, const double* vertices,
        vector<index_t>& sorted_indices,
	index_t dimension,
        index_t stride,
        index_t threshold,
        double ratio,
        vector<index_t>* levels
    ) {
        if(levels != nullptr) {
            levels->clear();
            levels->push_back(0);
        }
        index_t depth = 0;
        sorted_indices.resize(nb_vertices);
        for(index_t i = 0; i < nb_vertices; ++i) {
            sorted_indices[i] = i;
        }

        //The next three lines replace the following commented-out line
        //(random_shuffle is deprecated in C++17, and they call this 
        // progess...)
        //std::random_shuffle(sorted_indices.begin(), sorted_indices.end()); 
        std::random_device rng;
        std::mt19937 urng(rng());
        std::shuffle(sorted_indices.begin(), sorted_indices.end(), urng);

        compute_BRIO_order_recursive(
            nb_vertices, vertices,
	    dimension, stride,
            sorted_indices,
            sorted_indices.begin(), sorted_indices.end(),
            threshold, ratio, depth, levels
        );
    }
}

/**********************************************************************/

namespace {
    using namespace GEO;

    // Same as in delaunay/periodic.cpp,
    // copied here for now because linker does not find
    // it under Android.
    int Periodic_translation[27][3] = {
	{  0,  0,  0}, //13 -> 0   +   <-- zero displacement is first.
	{ -1, -1, -1}, //0  -> 1   -
	{ -1, -1,  0}, //1  -> 2   -
	{ -1, -1,  1}, //2  -> 3   -
	{ -1,  0, -1}, //3  -> 4   -
	{ -1,  0,  0}, //4  -> 5   - 
	{ -1,  0,  1}, //5  -> 6   -
	{ -1,  1, -1}, //6  -> 7   -
	{ -1,  1,  0}, //7  -> 8   -
	{ -1,  1,  1}, //8  -> 9   - 
	{  0, -1, -1}, //9  -> 10  -
	{  0, -1,  0}, //10 -> 11  -
	{  0, -1,  1}, //11 -> 12  -
	{  0,  0, -1}, //12 -> 13  -
	// (zero displacement was there)
	{  0,  0,  1}, //14 -> 14  +
	{  0,  1, -1}, //15 -> 15  -
	{  0,  1,  0}, //16 -> 16  +
	{  0,  1,  1}, //17 -> 17  +
	{  1, -1, -1}, //18 -> 18  -
	{  1, -1,  0}, //19 -> 19  -
	{  1, -1,  1}, //20 -> 20  -
	{  1,  0, -1}, //21 -> 21  -
	{  1,  0,  0}, //22 -> 22  +
	{  1,  0,  1}, //23 -> 23  +
	{  1,  1, -1}, //24 -> 24  -
	{  1,  1,  0}, //25 -> 25  +
	{  1,  1,  1}  //26 -> 26  +
    };

    
    /**
     * \details Exposes an interface compatible with the requirement
     *   of Hilbert sort templates for a raw array of vertices.
     */
    class PeriodicVertexArray3d {
    public:
        /**
         * \brief Constructs a new PeriodicVertexArray.
	 * \param[in] nb_vertices total number of vertices, including
	 *   the 27 copies.
         * \param[in] base address of the points.
         * \param[in] stride number of doubles between
         *  two consecutive points.
	 * \param[in] period the edge length of the periodic domain.
         */
        PeriodicVertexArray3d(
            index_t nb_vertices,
            const double* base, index_t stride,
	    double period = 1.0
        ) :
            base_(base),
            stride_(stride) {
            nb_vertices_ = nb_vertices;
	    nb_real_vertices_ = nb_vertices_ / 27;
	    geo_debug_assert(nb_vertices % 27 == 0);

	    
	    for(index_t i=0; i<27; ++i) {
		for(index_t j=0; j<3; ++j) {
		    xlat_[i][j] = period * double(Periodic_translation[i][j]);
		}
	    }
        }

        /**
         * \brief Gets a point coordinate by its index and coordinate.
         * \param[in] i the index of the point.
	 * \param[in] coord the coordinate.
         * \return the value of the coordinate.
         */
	double point_coord(index_t i, index_t coord) const {
	    index_t instance = i / nb_real_vertices_;
	    i = i % nb_real_vertices_;
            return (base_ + i * stride_)[coord] + xlat_[instance][coord];
        }

    private:
        const double* base_;
        index_t stride_;
        index_t nb_vertices_;
	index_t nb_real_vertices_;
	double xlat_[27][3];
    };

    /**
     * \brief Exposes an interface compatible with the requirement
     *  of Hilbert sort templates for a raw array of vertices.
     */
    class PeriodicVertexMesh3d {
    public:
        /**
         * \brief Constructs a new VertexMesh.
	 * \param[in] nb_vertices total number of vertices, including
	 *   the 27 copies.
         * \param[in] base address of the points
         * \param[in] stride number of doubles between
         *  two consecutive points
         */
        PeriodicVertexMesh3d(
            index_t nb_vertices,
            const double* base, index_t stride, double period
	) : vertices(nb_vertices, base, stride, period) {
        }
	
        PeriodicVertexArray3d vertices;
    };

    /**
     * \brief Drop-in replacement of Hilbert_vcmp for
     *  periodic vertices.
     * \details Needed because point_ptr() is not defined,
     *  we need to use point_coord() instead.
     */
    template <int COORD, bool UP, class MESH>
    struct Hilbert_vcmp_periodic {
    };

    /**
     * \brief Drop-in replacement of Hilbert_vcmp for
     *  periodic vertices, specialization for UP.
     * \details Needed because point_ptr() is not defined,
     *  we need to use point_coord() instead.
     */
    template <int COORD>
    struct Hilbert_vcmp_periodic<COORD, true, PeriodicVertexMesh3d> {
        Hilbert_vcmp_periodic(const PeriodicVertexMesh3d& mesh) :
            mesh_(mesh) {
        }
        bool operator() (index_t i1, index_t i2) {
            return
                mesh_.vertices.point_coord(i1,COORD) <
                mesh_.vertices.point_coord(i2,COORD);
        }
        const PeriodicVertexMesh3d& mesh_;
    };

    /**
     * \brief Drop-in replacement of Hilbert_vcmp for
     *  periodic vertices, specialization for !UP.
     * \details Needed because point_ptr() is not defined,
     *  we need to use point_coord() instead.
     */
    template <int COORD>
    struct Hilbert_vcmp_periodic<COORD, false, PeriodicVertexMesh3d> {
        Hilbert_vcmp_periodic(const PeriodicVertexMesh3d& mesh) :
            mesh_(mesh) {
        }
        bool operator() (index_t i1, index_t i2) {
            return
                mesh_.vertices.point_coord(i1,COORD) >
                mesh_.vertices.point_coord(i2,COORD);
        }
        const PeriodicVertexMesh3d& mesh_;
    };
    
}

namespace GEO {

    void Hilbert_sort_periodic(
	index_t nb_vertices, const double* vertices,
	vector<index_t>& sorted_indices,
	index_t dimension,
        index_t stride,
	vector<index_t>::iterator b,
	vector<index_t>::iterator e,
	double period
    ) {
	geo_assert(dimension == 3); // Only implemented for 3D.	
	geo_argused(sorted_indices); // Accessed through b and e.

       
        //The next three lines replace the following commented-out line
        //(random_shuffle is deprecated in C++17, and they call this 
        // progress...)
        // std::random_shuffle(b,e);
        std::random_device rng;
        std::mt19937 urng(rng());
        std::shuffle(b,e, urng);

	PeriodicVertexMesh3d M(nb_vertices, vertices, stride, period);
	HilbertSort3d<Hilbert_vcmp_periodic, PeriodicVertexMesh3d>(
	    M, b, e
	);
    }

}

