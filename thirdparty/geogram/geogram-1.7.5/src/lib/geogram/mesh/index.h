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

#ifndef GEOGRAM_MESH_INDEX
#define GEOGRAM_MESH_INDEX

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/argused.h>
#include <geogram/basic/algorithm.h>
#include <iostream>

/**
 * \file geogram/mesh/index.h
 * \brief Classes for managing tuples of indices
 */

namespace GEO {

    /************************************************************************/

    /**
     * \brief A couple of two indices.
     * \details Can be used as a key in associative data structures
     *  (std::map, std::set). The indices are defined by \p IndexType,
     *  generally signed or unsigned integers.
     * \tparam IndexType type of the indices
     */
    template <class IndexType>
    struct basic_bindex {

        /**
         * \brief The array of 2 indices
         */
        IndexType indices[2];

        /**
         * \brief This type is used to overload basic_bindex constructors with
         * versions that keep the order of the stored indices.
         */
        enum KeepOrderType {
            /** Value to pass to basic_bindex ordered constructor */
            KEEP_ORDER
        };

        /**
         * \brief Creates an uninitialized basic_bindex.
         */
        basic_bindex() {
        }

        /**
         * \brief Creates a basic_bindex from two integers.
         * \details The integers are reordered.
         * \param[in] i first integer
         * \param[in] j second integer
         */
        basic_bindex(
            IndexType i,
            IndexType j
        ) {
            if(i < j) {
                indices[0] = i;
                indices[1] = j;
            } else {
                indices[0] = j;
                indices[1] = i;
            }
        }

        /**
         * \brief Creates a basic_bindex from two integers and 
	 *  keeps their order.
         * \details The integers are not sorted.
         * \param[in] i first integer
         * \param[in] j second integer
         * \param[in] order argument of type #KeepOrderType used to select the
         *  right constructor. Use \c basic_bindex::#KEEP_ORDER 
	 *  for this argument.
         */
        basic_bindex(
            IndexType i,
            IndexType j,
            KeepOrderType order
        ) {
            geo_argused(order);
            indices[0] = i;
            indices[1] = j;
        }

        /**
         * \brief Compares two basic_bindex.
         * \param[in] rhs the basic_bindex to compares this basic_bindex with.
         * \return true if \p rhs is smaller than this basic_bindex according
         *  to the lexicographic order, false otherwise.
         */
        bool operator< (const basic_bindex<IndexType>& rhs) const {
            if(indices[0] < rhs.indices[0]) {
                return true;
            }
            if(indices[0] > rhs.indices[0]) {
                return false;
            }
            if(indices[1] < rhs.indices[1]) {
                return true;
            }
            return false;
        }

        /**
         * \brief Compares two basic_bindex.
         * \param[in] rhs the basic_bindex to compare this basic_bindex with.
         * \return true of all indices of this basic_bindex correspond to
         *  the indices in \p rhs at the same positions, false otherwise.
         */
        bool operator== (const basic_bindex<IndexType>& rhs) const {
            return
                (indices[0] == rhs.indices[0]) &&
                (indices[1] == rhs.indices[1]);
        }

        /**
         * \brief Compares two basic_bindex.
         * \param[in] rhs the basic_bindex to compare this basic_bindex with.
         * \return true if one of the indices in \p rhs differs from
         *  the index in this basic_bindex at the same position, 
	 *  false otherwise.
         */
        bool operator!= (const basic_bindex<IndexType>& rhs) const {
            return
                (indices[0] != rhs.indices[0]) ||
                (indices[1] != rhs.indices[1]);
        }

        /**
         * \brief Constructs a basic_bindex from another one.
         * \param[in] rhs the basic_bindex this basic_bindex 
	 *  should be copied from
         */
        basic_bindex(const basic_bindex<IndexType>& rhs) {
            indices[0] = rhs.indices[0];
            indices[1] = rhs.indices[1];
        }

        /**
         * \brief Assigns a basic_bindex to this one.
         * \param[in] rhs the basic_bindex this basic_bindex 
	 *  should be assigned from
         * \return a reference to this basic_bindex
         */
        basic_bindex<IndexType>& operator= (
	    const basic_bindex<IndexType>& rhs
	) {
            indices[0] = rhs.indices[0];
            indices[1] = rhs.indices[1];
            return *this;
        }

        /**
         * \brief Computes the inverse of a basic_bindex
         * \details The inverse of a basic_bindex has the same indices but
         *  in reverse order.
         * \param[in] b the basic_bindex
         * \return a basic_bindex with the same indices as \p b but in reverse
         * order.
         */
        static basic_bindex inverse(const basic_bindex<IndexType>& b) {
            return basic_bindex(b.indices[1], b.indices[0], KEEP_ORDER);
        }
    };

    /**
     * \brief A basic_bindex made of 2 unsigned integers
     * \relates basic_index
     */
    typedef basic_bindex<index_t> bindex;

    /**
     * \brief A basic_bindex made of 2 signed integers
     * \relates basic_index
     */
    typedef basic_bindex<signed_index_t> signed_bindex;

    /**
     * \brief Writes a basic_bindex to a stream
     * \details Displays all the indices of the basic_bindex \p B.
     * \param[in] out the output stream
     * \param[in] B the basic_bindex to write
     * \tparam IndexType type of the indices
     * \return a reference to the output stream \p out
     * \relates basic_bindex
     */
    template <class IndexType>
    inline std::ostream& operator<< (
	std::ostream& out, const basic_bindex<IndexType>& B
    ) {
        return out << B.indices[0] << " " << B.indices[1];
    }

    /************************************************************************/

    /**
     * \brief A triple of three indices.
     * \details Can be used as a key in associative data structures
     *  (std::map, std::set). The indices are defined by \p IndexType,
     *  generally signed or unsigned integers.
     * \tparam IndexType type of the indices
     */
    template <class IndexType>
    struct basic_trindex {

        /**
         * \brief The array of 3 indices
         */
        IndexType indices[3];

        /**
         * \brief This type is used to overload basic_trindex constructors with
         * versions that keep the order of the stored indices.
         */
        enum KeepOrderType {
            /** Value to pass to basic_trindex ordered constructor */
            KEEP_ORDER
        };

        /**
         * \brief Creates an uninitialized basic_trindex.
         */
        basic_trindex() {
        }

        /**
         * \brief Creates a basic_trindex from three integers.
         * \details The integers are reordered.
         * \param[in] i first integer
         * \param[in] j second integer
         * \param[in] k third integer
         */
        basic_trindex(
            IndexType i,
            IndexType j,
            IndexType k
        ) {
            indices[0] = i;
            indices[1] = j;
            indices[2] = k;
	    GEO::sort_3(indices);
        }

        /**
         * \brief Creates a basic_trindex from three integers and 
	 *  keeps their order.
         * \details The integers are not sorted.
         * \param[in] i first integer
         * \param[in] j second integer
         * \param[in] k third integer
         * \param[in] order argument of type #KeepOrderType used to select the
         *  right constructor. Use \c basic_trindex::#KEEP_ORDER 
	 *  for this argument.
         */
        basic_trindex(
            IndexType i,
            IndexType j,
            IndexType k,
            KeepOrderType order
        ) {
            geo_argused(order);
            indices[0] = i;
            indices[1] = j;
            indices[2] = k;
        }

        /**
         * \brief Compares two basic_trindex.
         * \param[in] rhs the basic_trindex to compares this basic_trindex with.
         * \return true if \p rhs is smaller than this basic_trindex according
         *  to the lexicographic order, false otherwise.
         */
        bool operator< (const basic_trindex<IndexType>& rhs) const {
            for(index_t i = 0; i < 3; i++) {
                if(indices[i] < rhs.indices[i]) {
                    return true;
                }
                if(indices[i] > rhs.indices[i]) {
                    return false;
                }
            }
            return false;
        }

        /**
         * \brief Compares two basic_trindex.
         * \param[in] rhs the basic_trindex to compare this basic_trindex with.
         * \return true of all indices of this basic_trindex correspond to
         *  the indices in \p rhs at the same positions, false otherwise.
         */
        bool operator== (const basic_trindex<IndexType>& rhs) const {
            return
                (indices[0] == rhs.indices[0]) &&
                (indices[1] == rhs.indices[1]) &&
                (indices[2] == rhs.indices[2]);
        }

        /**
         * \brief Compares two basic_trindex.
         * \param[in] rhs the basic_trindex to compare this basic_trindex with.
         * \return true if one of the indices in \p rhs differs from
         *  the index in this basic_trindex at the same position, 
	 *  false otherwise.
         */
        bool operator!= (const basic_trindex<IndexType>& rhs) const {
            return
                (indices[0] != rhs.indices[0]) ||
                (indices[1] != rhs.indices[1]) ||
                (indices[2] != rhs.indices[2]);
        }

        /**
         * \brief Constructs a basic_trindex from another one.
         * \param[in] rhs the basic_trindex this basic_trindex 
	 *  should be copied from
         */
        basic_trindex(const basic_trindex<IndexType>& rhs) {
            indices[0] = rhs.indices[0];
            indices[1] = rhs.indices[1];
            indices[2] = rhs.indices[2];
        }

        /**
         * \brief Assigns a basic_trindex to this one.
         * \param[in] rhs the basic_trindex this basic_trindex should 
	 *  be assigned from
         * \return a reference to this basic_trindex
         */
        basic_trindex<IndexType>& operator= (
	    const basic_trindex<IndexType>& rhs
	) {
            indices[0] = rhs.indices[0];
            indices[1] = rhs.indices[1];
            indices[2] = rhs.indices[2];
            return *this;
        }

        /**
         * \brief Tests whether a basic_trindex has the same orientation 
	 *  as a triple of integers.
         * \details Two basic_trindex have the same orientation if one of them
         *  is a circular permutation of the other one.
         * \param[in] t the basic_trindex
         * \param[in] i first index
         * \param[in] j second index
         * \param[in] k third index
         * \return true if the indices in \p t are a circular permutation
         *  of (\p i, \p j, \p k), false otherwise.
         */
        static bool same_orientation(
            const basic_trindex<IndexType>& t,
            IndexType i, IndexType j, IndexType k
        ) {
            return
                (t.indices[0] == i && t.indices[1] == j && t.indices[2] == k) ||
                (t.indices[1] == i && t.indices[2] == j && t.indices[0] == k) ||
                (t.indices[2] == i && t.indices[0] == j && t.indices[1] == k);
        }

        /**
         * \brief Tests whether two basic_trindex have the same orientation.
         * \details Two basic_trindex have the same orientation if one of them
         *  is a circular permutation of the other one.
         * \param[in] t1 first basic_trindex
         * \param[in] t2 second basic_trindex
         * \return true if the indices in \p t2 are a circular permutation
         *  of the indices in \p t1, false otherwise.
         */
        static bool same_orientation(
	    const basic_trindex<IndexType>& t1,
	    const basic_trindex<IndexType>& t2
	) {
            return same_orientation(
                t1, t2.indices[0], t2.indices[1], t2.indices[2]
            );
        }

        /**
         * \brief Computes the inverse of a basic_trindex
         * \details The inverse of a basic_trindex has the same indices but
         *  in reverse order.
         * \param[in] t the basic_trindex
         * \return a basic_trindex with the same indices as \p t but in reverse
         * order.
         */
        static basic_trindex inverse(const basic_trindex<IndexType>& t) {
            return basic_trindex(
                t.indices[2], t.indices[1], t.indices[0], KEEP_ORDER
            );
        }
    };

    /**
     * \brief A basic_trindex made of 3 unsigned integers
     * \relates basic_index
     */
    typedef basic_trindex<index_t> trindex;

    /**
     * \brief A basic_trindex made of 3 signed integers
     * \relates basic_index
     */
    typedef basic_trindex<signed_index_t> signed_trindex;

    /**
     * \brief Writes a basic_trindex to a stream
     * \details Displays all the indices of the basic_trindex \p T.
     * \param[in] out the output stream
     * \param[in] T the basic_trindex to write
     * \tparam IndexType type of the indices
     * \return a reference to the output stream \p out
     * \relates basic_trindex
     */
    template <class IndexType>
	inline std::ostream& operator<< (
	    std::ostream& out, const basic_trindex<IndexType>& T
    ) {
        return out
            << T.indices[0] << " " << T.indices[1] << " " << T.indices[2];
    }

    /************************************************************************/

    /**
     * \brief A tuple of four indices.
     * \details Can be used as a key in associative data structures
     *  (std::map, std::set). The indices are defined by \p IndexType,
     *  generally signed or unsigned integers.
     * \tparam IndexType type of the indices
     */
    template <class IndexType>
    struct basic_quadindex {
        /**
         * \brief The array of 4 indices
         */
        IndexType indices[4];

        /**
         * \brief This type is used to overload basic_quadindex 
	 *  constructors with versions that keep the order of the 
	 *  stored indices.
         */
        enum KeepOrderType {
            /** Value to pass to basic_quadindex ordered constructor */
            KEEP_ORDER
        };

        /**
         * \brief Constructs a new uninitialized basic_quadindex
         */
        basic_quadindex() {
        }

        /**
         * \brief Creates a basic_quadindex from four integers.
         * \details The integers are reordered.
         * \param[in] i first integer
         * \param[in] j second integer
         * \param[in] k third integer
         * \param[in] l fourth integer
         */
        basic_quadindex(
            IndexType i,
            IndexType j,
            IndexType k,
            IndexType l
        ) {
            indices[0] = i;
            indices[1] = j;
            indices[2] = k;
            indices[3] = l;
	    GEO::sort_4(indices);
        }

        /**
         * \brief Creates a basic_quadindex from four integers and 
	 *  keeps their order.
         * \details The integers are not sorted.
         * \param[in] i first integer
         * \param[in] j second integer
         * \param[in] k third integer
         * \param[in] l fourth integer
         * \param[in] order argument of type #KeepOrderType used to select the
         *  right constructor. Use \c basic_quadindex::#KEEP_ORDER for 
	 *  this argument.
         */
        basic_quadindex(
            IndexType i,
            IndexType j,
            IndexType k,
            IndexType l,
            KeepOrderType order
        ) {
            geo_argused(order);
            indices[0] = i;
            indices[1] = j;
            indices[2] = k;
            indices[3] = l;
        }

        /**
         * \brief Compares two basic_quadindex.
         * \param[in] rhs the basic_quadindex to compares this 
	 *  basic_quadindex with.
         * \return true if \p rhs is smaller than this basic_quadindex according
         *  to the lexicographic order, false otherwise.
         */
        bool operator< (const basic_quadindex<IndexType>& rhs) const {
            for(index_t i = 0; i < 4; i++) {
                if(indices[i] < rhs.indices[i]) {
                    return true;
                }
                if(indices[i] > rhs.indices[i]) {
                    return false;
                }
            }
            return false;
        }

        /**
         * \brief Compares two basic_quadindex.
         * \param[in] rhs the basic_quadindex to compare this 
	 *  basic_quadindex with.
         * \return true of all indices of this basic_quadindex correspond to
         *  the indices in \p rhs at the same positions, false otherwise.
         */
        bool operator== (const basic_quadindex<IndexType>& rhs) const {
            return
                (indices[0] == rhs.indices[0]) &&
                (indices[1] == rhs.indices[1]) &&
                (indices[2] == rhs.indices[2]) &&
                (indices[3] == rhs.indices[3]);
        }

        /**
         * \brief Compares two basic_quadindex.
         * \param[in] rhs the basic_quadindex to compare this 
	 *  basic_quadindex with.
         * \return true if one of the indices in \p rhs differs from
         *  the index in this basic_quadindex at the same position, 
	 *  false otherwise.
         */
        bool operator!= (const basic_quadindex<IndexType>& rhs) const {
            return
                (indices[0] != rhs.indices[0]) ||
                (indices[1] != rhs.indices[1]) ||
                (indices[2] != rhs.indices[2]) ||
                (indices[3] != rhs.indices[3]);
        }

        /**
         * \brief Constructs a basic_quadindex from another one.
         * \param[in] rhs the basic_quadindex this basic_quadindex 
	 *  should be copied from
         */
        basic_quadindex(const basic_quadindex<IndexType>& rhs) {
            indices[0] = rhs.indices[0];
            indices[1] = rhs.indices[1];
            indices[2] = rhs.indices[2];
            indices[3] = rhs.indices[3];
        }

        /**
         * \brief Assigns a basic_quadindex to this one.
         * \param[in] rhs the basic_quadindex this basic_quadindex 
	 *  should be assigned from
         * \return a reference to this basic_quadindex
         */
        basic_quadindex<IndexType>& operator= (
	    const basic_quadindex<IndexType>& rhs
	) {
            indices[0] = rhs.indices[0];
            indices[1] = rhs.indices[1];
            indices[2] = rhs.indices[2];
            indices[3] = rhs.indices[3];
            return *this;
        }
    };

    /**
     * \brief A basic_quadindex made of 4 unsigned integers
     * \relates basic_index
     */
    typedef basic_quadindex<index_t> quadindex;

    /**
     * \brief A basic_quadindex made of 4 signed integers
     * \relates basic_index
     */
    typedef basic_quadindex<signed_index_t> signed_quadindex;

    /**
     * \brief Writes a basic_quadindex to a stream
     * \details Displays all the indices of the basic_quadindex \p Q.
     * \param[in] out the output stream
     * \param[in] Q the basic_quadindex to write
     * \tparam IndexType type of the indices
     * \return a reference to the output stream \p out
     * \relates basic_quadindex
     */
    template <class IndexType>
    inline std::ostream& operator<< (
        std::ostream& out, const basic_quadindex<IndexType>& Q
    ) {
        return out
            << Q.indices[0] << " " << Q.indices[1] << " "
            << Q.indices[2] << " " << Q.indices[3];
    }

    /************************************************************************/
}

#endif

