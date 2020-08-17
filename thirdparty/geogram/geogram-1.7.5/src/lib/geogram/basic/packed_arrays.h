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

#ifndef GEOGRAM_BASIC_PACKED_ARRAYS
#define GEOGRAM_BASIC_PACKED_ARRAYS

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/assert.h>
#include <geogram/basic/process.h>

/**
 * \file geogram/basic/packed_arrays.h
 * \brief Efficient storage for array of arrays
 */

namespace GEO {

    /**
     * \brief Efficient storage for array of arrays.
     * \details
     * PackedArray is not a two-dimensional array, it is an array of values
     * where each value is itself an array of \c GEO::index_t.
     * PackedArrays provides functions for efficiently setting and getting
     * arrays by \b value (set_array() and get_array()).
     *
     * PackedArrays tries to be as memory efficient as possible:
     * - each sub-array is configured with a default capacity allowing to
     *   allocate a single fixed size memory area for all sub-arrays
     * - sub-arrays can be resized individually using an overflow area by
     *   calling resize_array().
     *
     * The idea behind PackedArrays is to be able to efficiently operate in a
     * multi-threaded environment while limiting the locking time of the
     * sub-arrays.
     *
     * A classical read operation mode would be:
     * - lock a sub-array
     * - read the sub-array by value
     * - unlock the sub-array
     * - access sub-array elements
     *
     * A classical write operation mode would be:
     * - prepare or modify sub-array elements
     * - lock a sub-array
     * - write sub-the array by value
     * - unlock the sub-array
     *
     * PackedArrays use SpinLockArray to lock/unlock sub-arrays in a
     * multi-threaded environment.
     */
    class GEOGRAM_API PackedArrays {
    public:
        /**
         * \brief Creates a new empty packed array
         */
        PackedArrays();

        /**
         * \brief Deletes a packed array
         */
        ~PackedArrays();

        /**
         * \brief Checks the thread-safety mode
         * \retval true if the array is configured to be thread-safe
         * \retval false otherwise
         */
        bool thread_safe() const {
            return thread_safe_;
        }

        /**
         * \brief Sets the thread-safety mode
         * \param[in] flag configures the array to be thread-safe if \c true.
         */
        void set_thread_safe(bool flag);

        /**
         * \brief Initializes a packed array
         * \details This allocates storage for storing \p nb_arrays with a
         * capacity of \p Z1_block_size elements of type \c
         * GEO::index_t. If parameter static_mode is \c true, the
         * overflow area is disabled and arrays are constrained to a maximum
         * of \p Z1_block_size elements.
         * \param[in] nb_arrays number of arrays to allocate
         * \param[in] Z1_block_size default capacity of the arrays (in number
         * of elements)
         * \param[in] static_mode specifies whether the array is static mode
         */
        void init(
            index_t nb_arrays,
            index_t Z1_block_size,
            bool static_mode = false
        );

        /**
         * \brief Clears the packed array
         * \details This frees allocated storage and resets the array as being
         * default-constructed. Call function init() with different parameters
         * to use it again.
         */
        void clear();

        /**
         * \brief Get the number of arrays
         * \return the number of arrays in this packed array
         */
        index_t nb_arrays() const {
            return nb_arrays_;
        }

        /**
         * \brief Gets the size of a sub-array
         * \details Returns the number of actual elements in the sub-array at
         * index \p array_index. This corresponds to the number of elements in
         * the last sub-array set by set_array(). This differs from the
         * default sub-array capacity used to preallocate the storage for the
         * packed-array.
         * \param[in] array_index index of the sub-array
         * \return the actual size of the sub-array elements
         */
        index_t array_size(index_t array_index) const {
            geo_debug_assert(array_index < nb_arrays_);
            return Z1_[array_index * Z1_stride_];
        }

        /**
         * \brief Gets a sub-array as a vector
         * \details
         * Copies the elements of the sub-array at index \p array_index to the
         * vector \p array.
         *
         * The operation is atomic if parameter \p lock is \c true (the
         * default) and the array is configured in thread-safety mode.
         *
         * \param[in] array_index the index of the sub-array
         * \param[out] array the output vector
         * \param[in] lock specifies whether the operation is atomic
         */
        void get_array(
            index_t array_index, vector<index_t>& array, bool lock = true
        ) const {
            if(lock) {
                lock_array(array_index);
            }
            array.resize(array_size(array_index));
            if(array.size() != 0) {
                get_array(array_index, &array[0], false);
            }
            if(lock) {
                unlock_array(array_index);
            }
        }

        /**
         * \brief Gets a sub-array
         * \details
         * Copies the elements of the sub-array at index \p array_index to the
         * array \p array. The caller must make sure that \p array has enough
         * space for storing \c array_size(array_index) elements.
         *
         * The operation is atomic if parameter \p lock is \c true (the
         * default) and the array is configured in thread-safety mode.
         *
         * \param[in] array_index the index of the sub-array
         * \param[out] array the output array of at least \p array_size
         *  elements.
         * \param[in] lock specifies whether the operation is atomic
         */
        void get_array(
            index_t array_index, index_t* array, bool lock = true
        ) const;

        /**
         * \brief Sets a sub-array
         * \details
         * Copies the first \p array_size elements of the array \p
         * array_elements to the sub-array at index \p array_index. If \p
         * array_size is different than the sub-array size, the sub-array is
         * resized (see resize_array()).
         *
         * If the packed array is in static mode, setting a sub-array with a
         * size greater than the original sub-array capacity makes the
         * function abort().
         *
         * The \p lock is \c true (the default) and the array is configured in
         * thread-safety mode.
         *
         * \param[in] array_index the index of the sub-array
         * \param[in] array_size the number of elements to copy
         * \param[in] array_elements an array of at least \p array_size elements
         * \param[in] lock specifies whether the operation is atomic
         */
        void set_array(
            index_t array_index,
            index_t array_size, const index_t* array_elements,
            bool lock = true
        );

        /**
         * \brief Sets a sub-array from a vector
         * \details
         * Copies all the elements in vector \p array to the sub-array at
         * index \p array_index. If the number of elements in \p vector is
         * different than the sub-array size, the sub-array is resized (see
         * resize_array()).
         *
         * If the packed array is in static mode, setting a sub-array with a
         * size greater than the original sub-array capacity makes the
         * function abort().
         *
         * The operation is atomic if parameter \p lock is \c true (the
         * default) and the array is configured in thread-safety mode.
         *
         * \param[in] array_index the index of the sub-array
         * \param[in] array an array of at least \p array_size elements
         * \param[in] lock specifies whether the operation is atomic
         */
        void set_array(
            index_t array_index,
            const vector<index_t>& array,
            bool lock = true
        ) {
            if(array.size() == 0) {
                set_array(array_index, 0, nullptr, lock);
            } else {
                set_array(
                    array_index, index_t(array.size()), &array[0], lock
                );
            }
        }

        /**
         * \brief Resizes a sub-array
         * \details
         * Reallocates the sub-array at index \p array_index to contain \p
         * array_size elements:
         * - if \p array_size is greater than the original sub-array capacity,
         *   space is allocated in the overflow area to contain the extra
         *   elements,
         * - if \p array_size is smaller than the original sub-array capacity,
         *   any space allocated for the sub-array in the overflow area is
         *   released.
         *
         * The operation is atomic if parameter \p lock is \c true (the
         * default) and the array is configured in thread-safety mode.
         *
         * \param[in] array_index the index of the sub-array
         * \param[in] array_size the new size of the sub-array
         * \param[in] lock specifies whether the operation is atomic
         */
        void resize_array(
            index_t array_index, index_t array_size, bool lock
        );

        /**
         * \brief Locks a sub-array
         * \details This gives the calling thread exclusive access to the
         * sub-array at index \p array_index. Other threads calling this
         * function for the same sub-array will block until the blocker thread
         * calls unlock_array().
         * \param[in] array_index the index of the sub-array
         */
        void lock_array(index_t array_index) const {
            if(thread_safe_) {
                Z1_spinlocks_.acquire_spinlock(array_index);
            }
        }

        /**
         * \brief Unlocks a sub-array
         * \details Releases the lock on the sub-array at index \p
         * array_index. This gives access to the sub-array to any waiting
         * thread.
         * \param[in] array_index the index of the sub-array
         */
        void unlock_array(index_t array_index) const {
            if(thread_safe_) {
                Z1_spinlocks_.release_spinlock(array_index);
            }
        }

        /**
         * \brief Displays array statistics
         * \details This prints statistics about memory occupation in the
         * main allocation area plus statistics about extra space allocated in
         * the overflow area.
         */
        void show_stats();

    protected:
        /**
         * \brief Checks if the array is in static mode
         * \retval true if the array is in static mode
         * \retval false otherwise
         */
        bool static_mode() const {
            return ZV_ == nullptr;
        }

    private:
        /** Forbid copy constructor */
        PackedArrays(const PackedArrays& rhs);

        /** Forbid assignment operator */
        PackedArrays& operator= (const PackedArrays& rhs);

    private:
        index_t nb_arrays_;
        index_t Z1_block_size_;
        index_t Z1_stride_;
        index_t* Z1_;
        index_t** ZV_;
        bool thread_safe_;
        mutable Process::SpinLockArray Z1_spinlocks_;
    };
}

#endif

