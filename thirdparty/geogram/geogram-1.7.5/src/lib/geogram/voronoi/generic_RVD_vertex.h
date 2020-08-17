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

#ifndef GEOGRAM_VORONOI_GENERIC_RVD_VERTEX
#define GEOGRAM_VORONOI_GENERIC_RVD_VERTEX

#include <geogram/basic/common.h>
#include <geogram/mesh/mesh.h>
#include <geogram/delaunay/delaunay_nn.h>
#include <geogram/basic/assert.h>
#include <geogram/basic/process.h>
#include <geogram/basic/attributes.h>

/**
 * \file geogram/voronoi/generic_RVD_vertex.h
 * \brief Types and utilities for manipulating vertices in geometric
 *  and symbolic forms in restricted Voronoi diagrams.
 * \note This file contains functions and classes used by the 
 *  internal implementation of GEO::GenericVoronoiDiagram. 
 *  Except some special uses, e.g. subclassing GEO::IntegrationSimplex, 
 *  they are not meant to be used directly by client code.
 */

namespace GEOGen {

    using GEO::Delaunay;      /**< \brief type for nD Delaunay triangulation */
    using GEO::index_t;   /**< \brief type for indices (vertex and facet id) */
    using GEO::signed_index_t;  /**< \brief type for indices (can be <0)     */
    using GEO::coord_index_t;   /**< \brief type for coordinate indices      */
    using GEO::Sign;  /**< \brief type for signs (POSITIVE,ZERO or NEGATIVE) */

    using GEO::Mesh;
    
    /**
     * \brief Small_set is similar to std::set, but with fixed
     * maximum size (and no dynamic memory allocation).
     * \details Used by GenericVoronoiDiagram to store vertices equations
     * (represented as plane indices triplets).
     * \note This is an internal implementation class, not meant to be
     *  used by client code.
     */
    template <class T, index_t DIM>
    class small_set {

        /** \brief This class type */
        typedef small_set<T, DIM> thisclass;

    public:
        /** \brief A random access iterator to elements  */
        typedef T* iterator;

        /** \brief A random access iterator to const elements */
        typedef const T* const_iterator;

        /** \brief Reference to element */
        typedef T& reference;

        /** \brief Type of the elements */
        typedef T value_type;

        /**
         * \brief Constructs an empty small_set.
         */
        small_set() :
            size_(0) {
        }

        /**
         * \brief Gets the number of element in this small_set.
         */
        index_t size() const {
            return size_;
        }

        /**
         * \brief Gets the maximum number of elements that can be
         * stored in this small_set.
         */
        index_t capacity() const {
            return (index_t) DIM;
        }

        /**
         * \brief Gets an iterator to the first element.
         */
        iterator begin() {
            return data_;
        }

        /**
         * \brief Gets an iterator one position past the last element.
         */
        iterator end() {
            return data_ + size_;
        }

        /**
         * \brief Gets an iterator one position past the last element
         *  that can be stored.
         */
        iterator end_of_storage() {
            return data_ + DIM;
        }

        /**
         * \brief Gets a const iterator to the first element..
         */
        const_iterator begin() const {
            return data_;
        }

        /**
         * \brief Gets a const iterator one position past the last element.
         */
        const_iterator end() const {
            return data_ + size_;
        }

        /**
         * \brief Gets a const iterator one position past the last element
         *  that can be stored
         */
        const_iterator end_of_storage() const {
            return data_ + DIM;
        }

        /**
         * \brief Insert a new element.
         * \param[in] x a const reference to the element to be inserted
         * \return an iterator to the inserted element
         * \note Throws an assertion failure if maximum capacity is reached
         */
        iterator insert(const T& x) {
            return insert(x, find_i(x));
        }

        /**
         * \brief Inserts a new element at a specified location..
         * \param[in] x a const reference to the element to be inserted
         * \param[in] where an iterator to the location where \p x should be
         *  inserted
         * \return an iterator to the inserted element (\p = where)
         * \note Throws an assertion failure if maximum capacity is reached
         */
        iterator insert(const T& x, iterator where) {
            if(where == end()) {
                *where = x;
                grow();
                return where;
            }
            if(*where == x) {
                return where;
            }
            grow();
            if(where == end() - 1) {
                *where = x;
                return where;
            }
            for(iterator i = end() - 1; i != where; i--) {
                geo_debug_assert(i != begin());
                *i = *(i - 1);
            }
            *where = x;
#ifdef GEO_DEBUG
            for(iterator i = begin(); i != end() - 1; ++i) {
                geo_debug_assert(*i < *(i + 1));
            }
#endif
            return where;
        }

        /**
         * \brief Clears this small_set.
         */
        void clear() {
            size_ = 0;
        }

        /**
         * \brief Finds an element by value.
         * \param[in] x a const reference to the value of the element
         * \return an iterator to the element or end() if not found
         */
        iterator find(const T& x) {
            iterator result = find_i(x);
            if(*result != x) {
                result = end();
            }
            return result;
        }

        /**
         * \brief Finds an element by value.
         * \param[in] x a const reference to the value of the element
         * \return a const iterator to the element or end() if not found
         */
        const_iterator find(const T& x) const {
            const_iterator result = find_i(x);
            if(*result != x) {
                result = end();
            }
            return result;
        }

        /**
         * \brief Appends an element to the end of the list.
         * \param[in] x a const reference to the value of the element
         * \pre \p x is greater than all the stored elements
         */
        void push_back(const T& x) {
#ifdef GEO_DEBUG
            for(iterator i = begin(); i != end(); ++i) {
                geo_debug_assert(*i < x);
            }
#endif
            *end() = x;
            grow();
        }

        /**
         * \brief Displays the stored elements.
         */
        void print(std::ostream& out) const {
            out << "[ ";
            for(const_iterator it = begin(); it != end(); ++it) {
                out << *it << " ";
            }
            out << "]";
        }

        /**
         * \brief Direct access to an element.
         * \param[in] i index of the element
         * \return a reference to the element
         */
        T& operator[] (signed_index_t i) {
            geo_debug_assert(i >= 0);
            geo_debug_assert(begin() + i < end());
            return begin()[i];
        }

        /**
         * \brief Direct access to an element.
         * \param[in] i index of the element
         * \return a const reference to the element
         */
        const T& operator[] (signed_index_t i) const {
            geo_debug_assert(i >= 0);
            geo_debug_assert(begin() + i < end());
            return begin()[i];
        }

    protected:
        /**
         * \brief Increases the size of this small_set.
         * \details Cannot grow past the maximum size.
         */
        void grow() {
            geo_debug_assert(end() != end_of_storage());
            size_++;
        }

        // Note: maybe we should start from end() instead of begin()
        // since negative indices are inserted first.

        /**
         * \brief Finds where an element is or where it should be inserted
         *  from its value.
         * \param[in] x a const reference to the value of the element
         * \return an iterator to the location where the element should be
         *  found or inserted
         */
        iterator find_i(const T& x) {
            iterator result = begin();
            while(result != end() && *result < x) {
                result++;
            }
            return result;
        }

        /**
         * \brief Finds where an element should be located from its value.
         * \param[in] x a const reference to the value of the element
         * \return a const iterator to the location where the element should be
         *  found.
         */
        const_iterator find_i(const T& x) const {
            const_iterator result = begin();
            while(result != end() && *result < x) {
                result++;
            }
            return result;
        }

    protected:
        T data_[DIM];
        index_t size_;
    };

    /**
     * \brief Displays the contents of a small_set to a std::ostream.
     */
    template <class T, index_t DIM>
    inline std::ostream& operator<< (
        std::ostream& out,
        const small_set<T, DIM>& S) {
        S.print(out);
        return out;
    }

    /**
     * \brief Computes the intersection between two small_set%s.
     * \param[in] S1 the first set
     * \param[in] S2 the second set
     * \param[out] I where to store the intersection
     */
    template <class T, index_t DIM1, index_t DIM2, index_t DIM3>
    inline void sets_intersect(
        const small_set<T, DIM1>& S1,
        const small_set<T, DIM2>& S2,
        small_set<T, DIM3>& I
    ) {
        I.clear();
        auto i1 = S1.begin();
        auto i2 = S2.begin();
        while(i1 < S1.end() && i2 < S2.end()) {
            if(*i1 < *i2) {
                ++i1;
            }
            else if(*i2 < *i1) {
                ++i2;
            }
            else {
                I.push_back(*i1);
                ++i1;
                ++i2;
            }
        }
    }

    /**
     * \brief A set of three integers that encodes the
     *  equation of a vertex in GenericVoronoiDiagram.
     *
     * \details
     * - Each positive entry i denotes the bisector of the segment that connects
     *   the center vertex to the i-th vertex (note that the center vertex
     *   needs to be stored elsewhere, but is known when a RVD is used,
     *   since we know which dual cell we are processing).
     *
     * - Each negative entry i denotes the i-th face in the boundary TriMesh.
     *   Note: indexing starts with 1 (resp. -1), 0 is kept for error codes.
     *
     * - There is some additional information for the following
     *   two configurations:
     *   - boundary vertex: (nb_boundary_facets = 3)
     *     the index of the boundary vertex is returned
     *     by get_boundary_vertex()
     *   - intersection between boundary edge and bisector:
     *     (nb_boundary_facets = 2)
     *     the indices v1,v2 of the extremities of the boundary edges
     *     are obtained by get_boundary_edge(v1,v2)
     *
     *  Doing so avoids recomputing vertices that we already know
     *  (and avoids numerical problems when the boundary surface has
     *  coplanar (or nearly coplanar) facets).
     *  It also allows using exact predicates (not implemented yet).
     *
     * \note This is an internal implementation class, not meant to be
     *  used by client code.
     */
    class SymbolicVertex : public small_set<GEO::signed_index_t, 3> {

        /** \brief This class type */
        typedef SymbolicVertex thisclass;

        /** \brief The base class of this class */
        typedef small_set<GEO::signed_index_t, 3> baseclass;

    public:
        /**
         * \brief Creates an uninitialized SymbolicVertex.
         */
        SymbolicVertex() :
            v1_(0),
            v2_(0) {
        }
	    
        /**
         * \brief Adds a bisector to the symbolic representation.
         */
        void add_bisector(index_t i) {
            baseclass::insert(signed_index_t(i) + 1);
        }

        /**
         * \brief Adds a boundary facet to the symbolic representation.
         */
        void add_boundary_facet(index_t i) {
            baseclass::insert(-signed_index_t(i) - 1);
        }

        /**
         * \brief Gets the number of boundary facets in the
         *  symbolic representation.
         */
        index_t nb_boundary_facets() const {
            index_t result = 0;
            for(auto it = baseclass::begin();
                it != baseclass::end() && *it < 0; ++it) {
                result++;
            }
            return result;
        }

        /**
         * \brief Gets the number of bisectors in the symbolic representation.
         */
        index_t nb_bisectors() const {
            index_t result = 0;
            for(auto it = baseclass::end() - 1;
                it != baseclass::begin() - 1 && *it > 0; --it) {
                result++;
            }
            return result;
        }

        /**
         * \brief Casts a signed_index_t into an (unsigned) index_t.
         * \details In debug mode, throws an assertion failure
         *  exception whenever \p x is negative.
         */
        static index_t to_unsigned_int(signed_index_t x) {
            geo_debug_assert(x >= 0);
            return (index_t) (x);
        }

        /**
         * \brief Gets a bisector
         * \param[in] i local index of the bisector
         * \return the index of the Delaunay vertex that corresponds to
         *    the second extremity of the bisector
         * \pre i < nb_bisectors()
         */
        index_t bisector(signed_index_t i) const {
            geo_debug_assert(i < (signed_index_t) nb_bisectors());
            return to_unsigned_int((baseclass::end()[-1 - i]) - 1);
        }

        /**
         * \brief Gets a boundary facet
         * \param[in] i local index of the boundary facet
         * \return the index of the mesh facet
         * \pre i < nb_boundary_facets()
         */
        index_t boundary_facet(signed_index_t i) const {
            geo_debug_assert(i < (signed_index_t) nb_boundary_facets());
            return to_unsigned_int(-(baseclass::begin()[i]) - 1);
        }

        /**
         * \brief Tests whether a bisector is present in the
         *  symbolic representation this vertex.
         * \param[in] i global index of the bisector
         */
        bool has_bisector(index_t i) const {
            return baseclass::find(signed_index_t(i) + 1) != baseclass::end();
        }

        /**
         * \brief Tests whether a boundary facet is present in the
         *  symbolic representation of this vertex.
         * \param[in] i global index of the boundary facet
         */
        bool has_boundary_facet(index_t i) const {
            return baseclass::find(-signed_index_t(i) - 1) != baseclass::end();
        }

        /**
         * \brief Gets the global index of the boundary vertex that corresponds
         * to this vertex.
         * \pre nb_boundary_facets() == 3
         */
        index_t get_boundary_vertex() const {
            geo_debug_assert(nb_boundary_facets() == 3);
            geo_debug_assert(v1_ != 0);
            return v1_ - 1;
        }

        /**
         * \brief Gets the global indices of the boundary vertices that
         *  define the boundary edge on which this vertex is located.
         * \param[out] v1 index of the first extremity of the boundary edge
         * \param[out] v2 index of the second extremity of the boundary edge
         * \pre nb_boundary_facets() == 2
         */
        void get_boundary_edge(index_t& v1, index_t& v2) const {
            geo_debug_assert(nb_boundary_facets() == 2);
            geo_debug_assert(v1_ != 0);
            geo_debug_assert(v2_ != 0);
            v1 = v1_ - 1;
            v2 = v2_ - 1;
        }

        /**
         * \brief Sets the boundary vertex on which this vertex is located.
         * \param[in] v global index of the boundary vertex
         */
        void set_boundary_vertex(index_t v) {
            v1_ = v + 1;
            v2_ = 0;
        }

        /**
         * \brief Sets the boundary edge on which this vertex is located.
         * \param[in] v1 global index of the first boundary vertex
         * \param[in] v2 global index of the second boundary vertex
         */
        void set_boundary_edge(index_t v1, index_t v2) {
            v1_ = v1 + 1;
            v2_ = v2 + 1;
        }

        /**
         * \brief Copies a boundary edge from the symbolic representation
         *  of another vertex.
         */
        void copy_boundary_edge_from(const thisclass& rhs) {
            geo_debug_assert(rhs.nb_boundary_facets() == 2);
            geo_debug_assert(rhs.nb_bisectors() == 1);
            geo_debug_assert(rhs.v1_ > 0);
            geo_debug_assert(rhs.v2_ > 0);
            v1_ = rhs.v1_;
            v2_ = rhs.v2_;
        }

        /**
         * \brief Computes the symbolic representation of the intersection
         *  between a segment and a bisector.
         * \details Computes the intersection between
         *  the segment [\p v1, \p v2] and the bisector \p E
         *
         * \return false if there was a problem
         *  (happens sometimes in finite precision mode)
         */
        bool intersect_symbolic(
            const thisclass& v1,
            const thisclass& v2,
            index_t E
        ) {

            // Compute the symbolic representation as the intersection
            // of three planes.
            sets_intersect(v1, v2, *this);
            // this computes the set of planes that contain
            // the edge [v1,v2]

            add_bisector(E);   // the intersection is on E.

            // Compute the symbolic representation as intersection between
            //    bisector and boundary edge
            // (it's redundant and less elegant than the representation
            //  as planes interactions,
            //  but we need this to handle degenerate configurations properly,
            //  and to use exact predicates with original boundary vertices
            //  coordinates).

            if(nb_boundary_facets() == 2) {
                // If *this is on the intersection of two boundary facets,
                // then *this is on
                // a boundary edge, and we need to retrieve the indices of the
                // two extremities of this boundary edge.

                index_t nb1 = v1.nb_boundary_facets();
                index_t nb2 = v2.nb_boundary_facets();
                if(nb1 == 3 && nb2 == 3) {
                    // If v1 and v2 are boundary vertices,
                    // then I is on the boundary
                    // edge that connects v1 and v2
                    set_boundary_edge(
                        v1.get_boundary_vertex(),
                        v2.get_boundary_vertex()
                    );
                } else if(nb1 == 2) {
                    geo_debug_assert(nb_boundary_facets() == 2);
                    // If v1 is on a boundary edge,
                    // then I is on the same boundary edge as v1
                    copy_boundary_edge_from(v1);
                } else if(nb2 == 2) {
                    geo_debug_assert(nb_boundary_facets() == 2);
                    // If v2 is on a boundary edge,
                    // then I is on the same boundary edge as v2
                    copy_boundary_edge_from(v2);
                }
            }

            // Sanity check: problem detected here, we
            // notify the caller that will use a workaround
            // (see clip_by_plane())
            if(baseclass::size() != 3) {
                return false;
            }
            return true;
        }

    private:
        index_t v1_;
        index_t v2_;
    };


    /**
     * \brief An allocator for points that are created
     * from intersections in GenericVoronoiDiagram.
     *
     * \details Implementation is an array of chunk. We do not use
     * std::deque since we want to control the chunk size,
     * and we want to clear it without deallocating
     * memory to avoid many calls to memory allocator (which
     * would probably slow down the Windows version a lot, there
     * seems to be a global multithreading lock on malloc()).
     *
     * In most cases, only the first chunk is used
     * (but some degenerate cases may use more). There seems
     * to be no measurable overhead as compared to a contiguous
     * array in our scenario.
     *
     * \note This is an internal implementation class, not meant to be
     *  used by client code.
     */

    class PointAllocator {
    public:
        /**
         * \brief Creates a new empty PointAllocator.
         * \param[in] dim dimension of the points to be allocated
         */
        PointAllocator(coord_index_t dim) :
            size_(0),
            capacity_(0),
            dimension_(dim) {
        } 

        /**
         * \brief Allocates a new point.
         * \return a pointer to the coordinates of the new point. Memory
         *  ownership remains to this PointAllocator.
         */
        double* new_item() {
            if(size_ == capacity_) {
                grow();
            }
            size_++;
            return item(size_ - 1);
        }

        /**
         * \brief Clears this PointAllocator.
         */
        void clear() {
            size_ = 0;
        }

        /**
         * \brief PointAllocator destructor
         * \details This releases all allocated chunks.
         */
        ~PointAllocator() {
            for(index_t c = 0; c < chunks_.size(); c++) {
                GEO::Memory::aligned_free(chunks_[c]);
            }
        }

        /**
         * \brief Gets the dimension of the points stored in
         *  this PointAllocator.
         */
        coord_index_t dimension() const {
            return dimension_;
        }

    protected:
        /**
         * \brief Constants that determine the size of a chunk.
         */
        enum {
            CHUNK_SHIFT = 8,
            CHUNK_SIZE = 1 << CHUNK_SHIFT,
            CHUNK_MASK = CHUNK_SIZE - 1
        };

        /**
         * \brief Allocates a new chunk of memory.
         */
        void grow() {
            chunks_.push_back(
                reinterpret_cast<double*>(
                    GEO::Memory::aligned_malloc(
                        index_t(CHUNK_SIZE) * dimension_ * sizeof(double)
                    )
                )
            );
            capacity_ += CHUNK_SIZE;
        }

        /**
         * \brief Gets a pointer to one of the allocated points from its index.
         * \param[in] i the index of the point in this PointAllocator
         * \return A pointer to the coordinates of the point
         */
        double* item(index_t i) {
            geo_debug_assert(i < size_);
            return &(chunks_[i >> CHUNK_SHIFT][(i & CHUNK_MASK) * dimension_]);
        }

    private:
        index_t size_;
        index_t capacity_;
        coord_index_t dimension_;
        std::vector<double*> chunks_;
    };

    /**
     * \brief Flags associated with edges.
     */
    enum {
        ORIGINAL = 1,  /**< Edge belongs to the input surface     */
        INTERSECT = 2  /**< Edge was generated by an intersection */
    };

    /**
     * \brief A set of EdgeFlags
     * \details EdgeFlag%s are combined with bitewise or.
     */
    typedef index_t EdgeFlags;

    /**
     * \brief An individual edge flag
     */
    typedef index_t EdgeFlag;

    /**
     * \brief Internal representation of vertices
     *  in GenericVoronoiDiagram.
     * \details Vertex has both
     *  geometrical and symbolic representations.
     * \note This is an internal implementation class, not meant to be
     *  used by client code (except in some particular case, such as
     *  subclassing GEO::IntegrationSimplex).
     */
    class Vertex {

        /** \brief This class type */
        typedef Vertex thisclass;

    public:
        /**
         * \brief Creates a new Vertex
         * \param[in] p geometric location at the vertex, shared with caller
         * \param[in] w weight
         * \param[in] f facet of the input mesh this Vertex comes from
         * \param[in] sym symbolic representation
         */
        Vertex(
            const double* p, double w, signed_index_t f,
            const SymbolicVertex& sym
        ) :
            point_(p),
            weight_(w),
            f_(f),
            seed_(-1),
            sym_(sym),
            flags_(ORIGINAL) {
        }

        /**
         * \brief Creates a new Vertex
         * \param[in] p geometric location at the vertex, shared with caller
         * \param[in] w weight
         * \param[in] f facet of the input mesh this Vertex comes from
         */
        Vertex(const double* p, double w, signed_index_t f) :
            point_(p),
            weight_(w),
            f_(f),
            seed_(-1),
            flags_(ORIGINAL) {
        }

        /**
         * \brief Creates an uninitialized Vertex.
         */
        Vertex() :
            point_(nullptr),
            weight_(1.0),
            f_(-1),
            seed_(-1),
            flags_(0) {
        }

        /**
         * \brief Gets the geometric location at this Vertex.
         * \return a const pointer to the coordinates
         */
        const double* point() const {
            return point_;
        }

        /**
         * \brief Sets the geometric location at this vertex.
         * \param[in] p the geometric location, shared with caller
         */
        void set_point(const double* p) {
            point_ = p;
        }

        /**
         * \brief Gets Vertex weight.
         * \details Used by non-uniform centroidal
         *  Voronoi tesselation.
         */
        double weight() const {
            return weight_;
        }

        /**
         * \brief Sets the vertex weight.
         * \details Used by non-uniform centroidal
         * Voronoi tesselation..
         */
        void set_weight(double w) {
            weight_ = w;
        }

        /**
         * \brief Gets the adjacent seed.
         * \return the global index of the adjacent seed
         */
        signed_index_t adjacent_seed() const {
            return seed_;
        }

        /**
         * \brief Sets the adjacent seed.
         * \param[in] s the global index of the adjacent seed
         */
        void set_adjacent_seed(signed_index_t s) {
            seed_ = s;
        }

        /** Symbolic representation */

        /**
         * \brief Gets the symbolic representation.
         */
        const SymbolicVertex& sym() const {
            return sym_;
        }

        /**
         * \brief Gets the symbolic representation.
         */
        SymbolicVertex& sym() {
            return sym_;
        }

        /**
         * \brief Gets the adjacent facet.
         * \return the global index of the adjacent facet
         */
        signed_index_t adjacent_facet() const {
            return f_;
        }

        /**
         * \brief Sets the adjacent facet.
         * \param[in] f the global index of the adjacent facet
         */
        void set_adjacent_facet(signed_index_t f) {
            f_ = f;
        }

        /**
         * \brief Implicit conversion that accesses the geometric location.
         * \details With this implicit conversions, we can have template
         * arguments for RestrictedVoronoiDiagram that take
         * const double* as arguments instead of Vertices.
         * \return a const pointer to the coordinates
         */
        operator const double* () const {
            return point_;
        }

        /**
         * \brief Clears this Vertex.
         */
        void clear() {
            flags_ = 0;
            f_ = -1;
        }

        /**
         * \brief Sets an EdgeFlag in this Vertex.
         */
        void set_flag(EdgeFlag f) {
            flags_ |= f;
        }

        /**
         * \brief Resets an EdgeFlag in this Vertex.
         */
        void unset_flag(EdgeFlag f) {
            flags_ &= ~f;
        }

        /**
         * \brief Tests an EdgeFlag in this Vertex.
         */
        bool check_flag(EdgeFlag f) const {
            return (flags_ & f) != 0;
        }

        /**
         * \brief Copies adjacent facet and edge flags from another Vertex.
         */
        void copy_edge_from(const Vertex& rhs) {
            set_adjacent_facet(rhs.adjacent_facet());
            flags_ = rhs.flags_;
        }

        /**
         * \brief Computes the intersection between
         *    a segment and a bisector.
         * \details Computes the intersection between
         *  the segment [vq1, vq2] and the bisector
         *  of [p1,p2]..
         * \tparam DIM dimension, specified as a template
         *  argument for efficiency considerations
         */
        template <index_t DIM>
        void intersect_geom(
            PointAllocator& target_intersections,
            const Vertex& vq1, const Vertex& vq2,
            const double* p1, const double* p2
        ) {
            const double* q1 = vq1.point();
            const double* q2 = vq2.point();
            double* Ipoint = target_intersections.new_item();
            set_point(Ipoint);
            double d = 0.0, l1 = 0.0, l2 = 0.0;
            for(coord_index_t c = 0; c < DIM; ++c) {
                double n = p1[c] - p2[c];
                d -= n * (p2[c] + p1[c]);
                l1 += q2[c] * n;
                l2 += q1[c] * n;
            }
            d = 0.5 * d;
            l1 = ::fabs(l1 + d);
            l2 = ::fabs(l2 + d);
            double l12 = l1 + l2;
            if(l12 > 1e-30) {
                l1 /= l12;
                l2 /= l12;
            } else {
                l1 = 0.5;
                l2 = 0.5;
            }
            for(coord_index_t c = 0; c < DIM; ++c) {
                Ipoint[c] = l1 * q1[c] + l2 * q2[c];
            }
            set_weight(l1 * vq1.weight() + l2 * vq2.weight());
        }

        /**
         * \brief Computes the side of this vertex relative
         *  to a bisector.
         * \details This version is not exact.
         * \param[in] p1 first extremity of the bisector
         * \param[in] p2 second extremity of the bisector
         * \return POSITIVE if this vertex is on p1's side,
         *  NEGATIVE if this vertex is on p2's side, and ZERO
         *  if this vertex is on the bisector of [p1,p2].
         */
        template <index_t DIM>
        Sign side_fast(
            const double* p1, const double* p2
        ) const {
            double r = 0.0;
            for(index_t c = 0; c < DIM; ++c) {
                r += GEO::geo_sqr(p2[c] - point()[c]);
                r -= GEO::geo_sqr(p1[c] - point()[c]);
            }
            return GEO::geo_sgn(r);
        }

    private:
        const double* point_;
        double weight_;

        /**
         * The facet adjacent to the edge
         * incident to this vertex.
         */
        signed_index_t f_;

        /**
         * indicates the seed of the bisector that generated the
         * edge that has this vertex and the previous one as
         * extremities (or -1 if border).
         */
        signed_index_t seed_;

        /** The symbolic representation of this vertex. */
        SymbolicVertex sym_;

        /**
         * Indicates the type of edge
         * (virtual, original or intersection).
         */
        EdgeFlags flags_;
    };
}

/*
namespace GEO {
    template <> struct can_be_used_as_attribute<GEOGen::SymbolicVertex> {
	static constexpr auto value = std::integral_constant<bool,true>();
    };
}
*/    


#endif

