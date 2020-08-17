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

#ifndef GEOGRAM_BASIC_PERMUTATION
#define GEOGRAM_BASIC_PERMUTATION

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/memory.h>

/**
 * \file geogram/basic/permutation.h
 * \brief Functions to manipulate permutations
 */

namespace GEO {

    /**
     * \brief Utilities for manipulating permutations
     */
    namespace Permutation {

        /**
         * \brief Checks whether a vector is a valid permutation.
         * \details The vector \p permutation is a valid permutation if there
         * is a bijection between the range [0..N-1] and the range
         * [permutation[0]..permutation[N-1]] where N is the number of
         * elements in the vector. An empty vector is considered as a valid
         * permutation.
         * \param[in] permutation a vector of integers
         * \retval true if \p permutation is a valid permutation
         * \retval false otherwise
         */
        inline bool is_valid(
            const vector<index_t>& permutation
        ) {
            std::vector<bool> visited(permutation.size(), false);
            for(index_t i = 0; i < permutation.size(); i++) {
                if(permutation[i] >= permutation.size()) {
                    return false;
                }
                if(visited[i]) {
                    return false;
                }
                visited[i] = true;
            }
            return true;
        }

        /**
         * \brief Marks a permutation element as visited
         * \details Sets a \e visited mark for element at index \p i.
         * in \p permutation. The element must \b not have been already marked.
         * The mark can be checked with is_marked() and removed with unmark().
         * \param[in,out] permutation a valid permutation
         * \param[in] i the index of the element in \p permutation
         * \note Used internally by apply().
         * Implementation note: marking an element modifies the value in such
         * a way that the initial value of the element can be restored.
         */
        inline void mark(
            vector<index_t>& permutation,
            index_t i
        ) {
            geo_debug_assert(i < permutation.size());
            geo_debug_assert(signed_index_t(permutation[i]) >= 0);
            permutation[i] = index_t(-signed_index_t(permutation[i]) - 1);
        }

        /**
         * \brief Checks if a permutation element has been visited
         * \param[in] permutation a valid permutation
         * \param[in] i the index of the element in \p permutation
         * \note Used internally by apply()
         */
        inline bool is_marked(
            const vector<index_t>& permutation,
            index_t i
        ) {
            geo_debug_assert(i < permutation.size());
            return signed_index_t(permutation[i]) < 0;
        }

        /**
         * \brief Unmarks a permutation element
         * \details This restores the initial value of element at index \p i
         * in \p permutation
         * \param[in,out] permutation a valid permutation
         * \param[in] i the index of the element in \p permutation
         * \note Used internally by apply()
         */
        inline void unmark(
            vector<index_t>& permutation,
            index_t i
        ) {
            geo_debug_assert(is_marked(permutation, i));
            permutation[i] = index_t(-signed_index_t(permutation[i]) - 1);
        }

        /**
         * \brief Applies a permutation in-place.
         * Permutes the first \p N elements of size \p elemsize in array \p
         * data using permutation \p permutation where \p N is the number
         * of elements in \p permutation. The result of the permutation is
         * left in \p data.
         * The array \p data must contain at least \c permutation.size()
         * elements otherwise memory corruption will happen.
         *
         * Applying permutation \p permutation is equivalent to:
         * \code
         * for(i=0; i<permutation.size(); i++) {
         *    data2[i] = data[permutation[i]]
         * }
         * data = data2 ;
         * \endcode
         * \param[in,out] data an array of \c permutation.size() elements to
         * permute
         * \param[in] permutation_in the permutation.
         *  It is temporarily changed during execution of the
         *  function, but identical to the input on exit.
         * \param[in] elemsize size of the vector elements
         */
        inline void apply(
            void* data, const vector<index_t>& permutation_in,
            index_t elemsize
        ) {
            Memory::pointer pdata = (Memory::pointer) (data);
            vector<index_t>& permutation =
                const_cast<vector<index_t>&>(permutation_in);
            geo_debug_assert(is_valid(permutation));
            Memory::byte* temp = static_cast<Memory::byte*>(alloca(elemsize));
            for(index_t k = 0; k < permutation.size(); k++) {
                if(is_marked(permutation, k)) {
                    continue;
                }
                index_t i = k;
                index_t j = permutation[k];
                Memory::copy(temp, pdata + i * elemsize, elemsize);
                mark(permutation, k);
                while(j != k) {
                    Memory::copy(
                        pdata + i * elemsize, pdata + j * elemsize, elemsize
                    );
                    index_t nj = permutation[j];
                    mark(permutation, j);
                    i = j;
                    j = nj;
                }
                Memory::copy(pdata + i * elemsize, temp, elemsize);
            }
            for(index_t k = 0; k < permutation.size(); k++) {
                unmark(permutation, k);
            }
        }

        /**
         * \brief Applies a permutation in-place.
         * Permutes the first \p N elements of vector \p data using
         * permutation \p permutation where \p N is the number of elements in
         * \p permutation. The result of the permutation is left in \p data.
         * The array \p data must contain at least \c permutation.size()
         * elements otherwise the function throws an out_of_range exception.
         *
         * Applying permutation \p permutation is equivalent to:
         * \code
         * for(i=0; i<permutation.size(); i++) {
         *    data2[i] = data[permutation[i]]
         * }
         * data = data2 ;
         * \endcode
         * \param[in,out] data the vector to permute
         * \param[in] permutation_in the permutation.
         *  It is temporarily changed during execution of the
         *  function, but identical to the input on exit.
         */
        template <class T>
        inline void apply(
            vector<T>& data, const vector<index_t>& permutation_in
        ) {
            vector<index_t>& permutation =
                const_cast<vector<index_t>&>(permutation_in);
            geo_debug_assert(is_valid(permutation));
            T temp;
            for(index_t k = 0; k < permutation.size(); k++) {
                if(is_marked(permutation, k)) {
                    continue;
                }
                index_t i = k;
                temp = data[i];
                index_t j = permutation[k];
                mark(permutation, k);
                while(j != k) {
                    data[i] = data[j];
                    index_t nj = permutation[j];
                    mark(permutation, j);
                    i = j;
                    j = nj;
                }
                data[i] = temp;
            }
            for(index_t k = 0; k < permutation.size(); k++) {
                unmark(permutation, k);
            }
        }

        /**
         * \brief Inverts a permutation in-place.
         * \details Inverses the given \p permutation in place, the
         * result of the inversion is left in \p permutation.
         *
         * The inversion is equivalent to:
         * \code
         * vector<index_t> inverse;
         * Permutation::invert(permutation, inverse);
         * permutation = inverse;
         * \endcode
         * \param[in,out] permutation to inverse.
         */
        inline void invert(vector<index_t>& permutation) {
            geo_debug_assert(is_valid(permutation));
            for(index_t k = 0; k < permutation.size(); k++) {
                if(is_marked(permutation, k)) {
                    continue;
                }
                index_t i = k;
                index_t j = permutation[i];
                while(j != k) {
                    index_t temp = permutation[j];
                    permutation[j] = i;
                    mark(permutation, j);
                    i = j;
                    j = temp;
                }
                permutation[j] = i;
                mark(permutation, j);
            }
            for(index_t k = 0; k < permutation.size(); k++) {
                unmark(permutation, k);
            }
        }

        /**
         * \brief Inverts a permutation.
         * \details Computes the inverse of a given \p permutation and 
         *  stores the result in another one.
         * \param[in] permutation the permutation to invert
         * \param[out] invert the computed inverse of \p permutation
         * \note there is also a variant of invert() that computes the
         *  permutation in-place.
         */
        inline void invert(
            const vector<index_t>& permutation, vector<index_t>& invert
        ) {
            geo_debug_assert(is_valid(permutation));
            invert.resize(permutation.size());
            for(index_t i=0; i<permutation.size(); ++i) {
                invert[permutation[i]] = i;
            }
        }

    }
}

#endif

