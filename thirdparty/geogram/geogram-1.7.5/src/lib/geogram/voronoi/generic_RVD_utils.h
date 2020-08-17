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

#ifndef GEOGRAM_VORONOI_GENERIC_RVD_UTILS
#define GEOGRAM_VORONOI_GENERIC_RVD_UTILS

#include <geogram/basic/common.h>
#include <geogram/voronoi/generic_RVD_vertex.h>
#include <geogram/voronoi/generic_RVD_polygon.h>
#include <geogram/voronoi/generic_RVD_cell.h>
#include <geogram/basic/memory.h>
#include <stack>
#include <vector>

/**
 * \file geogram/voronoi/generic_RVD_utils.h
 * \brief Some utilities for implementing surfacic
 *  and volumetric restricted Voronoi diagrams.
 */

namespace GEOGen {

    /**
     * \brief A stack implemented in a GEO::vector.
     * \details Used by the Android version of
     *   GEOGen::RestrictedVoronoiDiagram. The
     *   std::stack class has some problems with
     *   multithread memory protection issues (it seems that
     *   a SMP-safe global lock on memory is missing
     *   in Android libraries).
     */
    template <class T>
    class VectorStack {
    public:
        /**
         * \brief Pushes a new item onto the stack.
         */
        void push(const T& x) {
            rep_.push_back(x);
        }

        /**
         * \brief Pops the top of the stack.
         * \pre !empty()
         */
        void pop() {
            rep_.pop_back();
        }

        /**
         * \brief Gets the item on the top.
         * \return a const reference to the item on the top
         * \pre !empty()
         */
        const T& top() const {
            return *rep_.rbegin();
        }

        /**
         * \brief Tests whether the stack is empty.
         */
        bool empty() const {
            return rep_.size() == 0;
        }

    private:
        GEO::vector<T> rep_;
    };

    /************************************************************************/

    /**
     * \brief A (facet,seed) pair.
     * \details Used by GEOGen::RestrictedVoronoiDiagram
     * for propagating over the facet graph and the
     * Delaunay 1-skeleton.
     */
    struct FacetSeed {

        /**
         * \brief Creates a new FacetSeed
         * \param[in] f_in index of the facet
         * \param[in] seed_in index of the seed
         */
        FacetSeed(index_t f_in, index_t seed_in) :
            f(f_in),
            seed(seed_in) {
        }

        /**
         * \brief Creates a new uninitialized FacetSeed
         * \details F and seed contain random values.
         */
        FacetSeed() {
        }

        /**
         * \brief Compares two facet seeds using lexicographic order.
         * \details Makes it possible to use FacetSeed as keys for
         * std::set and std::map.
         */
        bool operator< (const FacetSeed& rhs) const {
            if(f < rhs.f) {
                return true;
            }
            if(f > rhs.f) {
                return false;
            }
            return seed < rhs.seed;
        }

        index_t f;
        index_t seed;
    };

    /**
     * \brief A (tetrahedron,seed) pair.
     * \details Used by GEOGen::RestrictedVoronoiDiagram
     * for propagating over the tetrahedra graph and the
     * Delaunay 1-skeleton.
     */
    typedef FacetSeed TetSeed;
    /************************************************************************/

#ifdef GEO_OS_ANDROID
    // VectorStack uses AlignedAllocator, that is protected
    // by a global lock under Android (needed because it
    // seems that malloc() is not SMP-thread-safe under Android).

    /**
     * \brief A stack of FacetSeed.
     * \details Used by GEOGen::RestrictedVoronoiDiagram.
     */
    typedef VectorStack<FacetSeed> FacetSeedStack;

    /**
     * \brief A stack of TetSeed.
     * \details Used by GEOGen::RestrictedVoronoiDiagram.
     */
    typedef VectorStack<TetSeed> TetSeedStack;

    /**
     * \brief A stack of seed indices (index_t).
     * \details Used by GEOGen::RestrictedVoronoiDiagram.
     */
    typedef VectorStack<index_t> SeedStack;
#else

    /**
     * \brief A stack of FacetSeed.
     * \details Used by GEOGen::RestrictedVoronoiDiagram.
     */
    typedef std::stack<FacetSeed> FacetSeedStack;

    /**
     * \brief A stack of TetSeed.
     * \details Used by GEOGen::RestrictedVoronoiDiagram.
     */
    typedef std::stack<TetSeed> TetSeedStack;

    /**
     * \brief A stack of seed indices (index_t).
     * \details Used by GEOGen::RestrictedVoronoiDiagram.
     */
    typedef std::stack<index_t> SeedStack;
#endif

    /************************************************************************/

    /**
     * \brief Stores associations between (facet,seed) pairs and the index of
     *  a connected component.
     *
     * \details Used by GEOGen::RestrictedVoronoiDiagram.
     *  The implementation uses an array of (key,value) vectors,
     *  with dynamic reallocation and linear search.
     *  Experimentally, this significantly reduces the memory
     *  footprint and execution time as compared to
     *  std::table<FacetSeed,index_t>.
     */
    class FacetSeedMarking {
    public:
        /**
         * \brief Creates a new FacetSeedMarking
         * \param[in] nb_seeds total number of seeds
         */
        FacetSeedMarking(index_t /* nb_facets*/, index_t nb_seeds) {
            set_size(nb_seeds);
        }

        /**
         * \brief Tests whether a given facet,seed couple is marked.
         */
        bool is_marked(index_t facet, index_t seed) const {
            return find_index(seed, facet) != -1;
        }

        /**
         * \brief Tests whether a fiven FacetSeed is marked.
         */
        bool is_marked(const FacetSeed& fs) const {
            return is_marked(fs.f, fs.seed);
        }

        /**
         * \brief Gets the index of the connected component associated
         *   with a given FacetSeed.
         */
        signed_index_t get_connected_component(const FacetSeed& fs) const {
            return find_value(fs.seed, fs.f);
        }

        /**
         * \brief Marks a FacetSeed and sets the associated
         *  connected component index.
         */
        void mark(const FacetSeed& fs, index_t conn_comp) {
            insert(fs.seed, fs.f, conn_comp);
        }

        /**
         * \brief FacetSeedMarking destructor
         */
        ~FacetSeedMarking() {
            for(index_t i = 0; i < nb_arrays(); ++i) {
                free(keys_[i]);
            }
            for(index_t i = 0; i < nb_arrays(); ++i) {
                free(values_[i]);
            }
        }

    protected:
        /**
         * \brief Gets the number of arrays used internally.
         */
        index_t nb_arrays() const {
            return index_t(keys_.size());
        }

        /**
         * \brief Sets the number of arrays to be used.
         * \param[in] nb_arrays number of arrays
         */
        void set_size(index_t nb_arrays) {
            keys_.assign(nb_arrays, nullptr);
            values_.assign(nb_arrays, nullptr);
            size_.assign(nb_arrays, 0);
        }

        /**
         * \brief Gets the size of one of the arrays.
         * \param[in] array index of the array
         * \pre array < nb_arrays()
         */
        index_t array_size(index_t array) const {
            return size_[array];
        }

        /**
         * \brief Gets the capacity of one of the arrays.
         * \details It corresponds with the power of two immediately
         *  greater than size. Unlike in std::vector, capacity is implicitly
         * retrieved from size (this saves one integer
         * per seed).
         * \param[in] array index of the array
         * \pre array < nb_arrays()
         */
        index_t array_capacity(index_t array) const {
            index_t size = array_size(array);
            if(size == 0) {
                return 0;
            }
            index_t result = 1;
            index_t mask = 1;
            for(index_t i = 0; i < 32; i++) {
                mask = mask << 1;
                if((size & mask) != 0) {
                    result = mask;
                }
            }
            // If size is not already a power of two,
            if(result != size) {
                result = result << 1;
            }
            return result;
        }

        /**
         * \brief Finds the index of one of the keys in one of the arrays.
         * \param[in] array index of the array
         * \param[in] key the query key
         * \return the index of \p key in \p array or -1 if not found
         */
        signed_index_t find_index(index_t array, index_t key) const {
            index_t* K = keys_[array];
            for(index_t i = 0; i < array_size(array); ++i) {
                if(K[i] == key) {
                    return signed_index_t(i);
                }
            }
            return -1;
        }

        /**
         * \brief Finds the value associated with a key in one
         * of the arrays.
         * \param[in] array index of the array
         * \param[in] key the query key
         * \return the value associated with \p key in \p array
         *  or -1 if not found.
         */
        signed_index_t find_value(index_t array, index_t key) const {
            signed_index_t i = find_index(array, key);
            if(i == -1) {
                return -1;
            }
            return signed_index_t(values_[array][i]);
        }

        /**
         * \brief Inserts a (key,value) pair into one of the arrays.
         * \param[in] array index of the array
         * \param[in] key the key
         * \param[in] value the value to be associated with \p key
         */
        void insert(index_t array, index_t key, index_t value) {
            signed_index_t si = find_index(array, key);
            if(si == -1) {
                // If not found, append at the end of array
                index_t i = size_[array];
                if(i == array_capacity(array)) {
                    // If capacity is reached, grow storage
                    index_t new_nb = index_t(2*i);
                    if(new_nb == 0) {
                        new_nb = 1;
                    }
                    keys_[array] = reinterpret_cast<index_t*>(
                        realloc(keys_[array], sizeof(index_t) * new_nb)
                    );
                    values_[array] = reinterpret_cast<index_t*>(
                        realloc(values_[array], sizeof(index_t) * new_nb)
                    );
                }
                size_[array] = i + 1;
                si = signed_index_t(i);
            }
            keys_[array][si] = key;
            values_[array][si] = value;
        }

    private:
        std::vector<index_t*> keys_;
        std::vector<index_t*> values_;
        std::vector<index_t> size_;
    };

    /************************************************************************/

    /**
     * \brief Stores associations between (tet,seed) pairs and the index of
     *  a connected component.
     */    
    typedef FacetSeedMarking TetSeedMarking;

    /************************************************************************/    
}

#endif

