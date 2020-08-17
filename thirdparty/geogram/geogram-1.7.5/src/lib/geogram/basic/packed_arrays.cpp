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

#include <geogram/basic/packed_arrays.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/string.h>

namespace {

    using namespace GEO;

    /**
     * \brief Computes a percentage of a reference value
     * \param[in] num the value to compute the percentage of
     * \param[in] denom the reference value
     * \return a formatted string that contains the \p num and its percentage
     * of \p denom
     */
    std::string percent_str(index_t num, index_t denom) {
        if(denom == 0) {
            return String::to_string(num);
        }
        double x = double(num) / double(denom) * 100.0;
        return String::to_string(num) + "(" + String::to_string(x) + "%)";
    }
}

namespace GEO {

    PackedArrays::PackedArrays() {
        nb_arrays_ = 0;
        Z1_block_size_ = 0;
        Z1_stride_ = 0;
        Z1_ = nullptr;
        ZV_ = nullptr;
        thread_safe_ = false;
    }

    void PackedArrays::show_stats() {
        index_t nb_items_in_Z1 = 0;
        index_t nb_items_in_ZV = 0;
        index_t nb_arrays_in_ZV = 0;
        index_t nb_items = 0;
        for(index_t i = 0; i < nb_arrays_; i++) {
            index_t sz = array_size(i);
            nb_items += sz;
            if(sz > Z1_block_size_) {
                nb_items_in_ZV += (sz - Z1_block_size_);
                nb_arrays_in_ZV++;
            }
            nb_items_in_Z1 += std::min(sz, Z1_block_size_);
        }

        Logger::out("PArrays")
            << "stats (nb_arrays=" << nb_arrays_
            << ", Z1 block size=" << Z1_block_size_ << ") "
            << (static_mode() ? "static" : "dynamic")
            << std::endl;

        index_t Z1_total = nb_arrays_ * Z1_block_size_;

        Logger::out("PArrays")
            << "Z1 filling:"
            << percent_str(nb_items_in_Z1, Z1_total) << std::endl;

        if(!static_mode()) {
            Logger::out("PArrays")
                << "arrays in ZV:" << percent_str(nb_arrays_in_ZV, nb_arrays_)
                << std::endl;
            Logger::out("PArrays")
                << "items  in Z1:" << percent_str(nb_items_in_Z1, nb_items)
                << std::endl;
            Logger::out("PArrays")
                << "items  in ZV:" << percent_str(nb_items_in_ZV, nb_items)
                << std::endl;
        }
    }

    PackedArrays::~PackedArrays() {
        clear();
    }

    void PackedArrays::clear() {
        if(ZV_ != nullptr) {
            for(index_t i = 0; i < nb_arrays_; i++) {
                free(ZV_[i]);
            }
            free(ZV_);
            ZV_ = nullptr;
        }
        nb_arrays_ = 0;
        Z1_block_size_ = 0;
        Z1_stride_ = 0;
        free(Z1_);
        Z1_ = nullptr;
    }

    void PackedArrays::set_thread_safe(bool x) {
        thread_safe_ = x;
        if(x) {
            Z1_spinlocks_.resize(nb_arrays_);
        } else {
            Z1_spinlocks_.clear();
        }
    }

    void PackedArrays::init(
        index_t nb_arrays,
        index_t Z1_block_size,
        bool static_mode
    ) {
        clear();
        nb_arrays_ = nb_arrays;
        Z1_block_size_ = Z1_block_size;
        Z1_stride_ = Z1_block_size_ + 1;  // +1 for storing array size.
        Z1_ = (index_t*) calloc(
            nb_arrays_, sizeof(index_t) * Z1_stride_
        );
        if(!static_mode) {
            ZV_ = (index_t**) calloc(
                nb_arrays_, sizeof(index_t*)
            );
        }
        if(thread_safe_) {
            Z1_spinlocks_.resize(nb_arrays_);
        }
    }

    void PackedArrays::get_array(
        index_t array_index, index_t* array, bool lock
    ) const {
        geo_debug_assert(array_index < nb_arrays_);
        if(lock) {
            lock_array(array_index);
        }
        const index_t* array_base = Z1_ + array_index * Z1_stride_;
        index_t array_size = *array_base;
        index_t nb = array_size;
        array_base++;
        index_t nb_in_block = std::min(nb, Z1_block_size_);
        Memory::copy(array, array_base, sizeof(index_t) * nb_in_block);
        if(nb > nb_in_block) {
            nb -= nb_in_block;
            array += nb_in_block;
            array_base = ZV_[array_index];
            Memory::copy(array, array_base, sizeof(index_t) * nb);
        }
        if(lock) {
            unlock_array(array_index);
        }
    }

    void PackedArrays::set_array(
        index_t array_index,
        index_t array_size, const index_t* array,
        bool lock
    ) {
        geo_debug_assert(array_index < nb_arrays_);
        if(lock) {
            lock_array(array_index);
        }
        index_t* array_base = Z1_ + array_index * Z1_stride_;
        index_t old_array_size = *array_base;
        array_base++;
        if(array_size != old_array_size) {
            resize_array(array_index, array_size, false);
        }
        index_t nb = array_size;
        index_t nb_in_block = std::min(nb, Z1_block_size_);
        Memory::copy(array_base, array, sizeof(index_t) * nb_in_block);
        if(nb > nb_in_block) {
            nb -= nb_in_block;
            array += nb_in_block;
            array_base = ZV_[array_index];
            Memory::copy(array_base, array, sizeof(index_t) * nb);
        }
        if(lock) {
            unlock_array(array_index);
        }
    }

    void PackedArrays::resize_array(
        index_t array_index, index_t array_size, bool lock
    ) {
        geo_debug_assert(array_index < nb_arrays_);
        if(lock) {
            lock_array(array_index);
        }
        index_t* array_base = Z1_ + array_index * Z1_stride_;
        index_t old_array_size = *array_base;
        if(old_array_size != array_size) {
            *array_base = array_size;
            if(static_mode()) {
                geo_assert(array_size <= Z1_block_size_);
            } else {
                index_t nb_in_ZV =
                    (array_size > Z1_block_size_) ?
                    array_size - Z1_block_size_ : 0;
                ZV_[array_index] = (index_t*) realloc(
                    ZV_[array_index], sizeof(index_t) * nb_in_ZV
                );
            }
        }
        if(lock) {
            unlock_array(array_index);
        }
    }
}

