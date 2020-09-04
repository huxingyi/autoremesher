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

#ifndef GEOGRAM_BASIC_SMART_POINTER
#define GEOGRAM_BASIC_SMART_POINTER

#include <geogram/basic/common.h>
#include <geogram/basic/assert.h>
#include <geogram/basic/memory.h>

/**
 * \file geogram/basic/smart_pointer.h
 * \brief Pointers with automatic reference counting
 */

namespace GEO {

    /************************************************************************/

    /**
     * \brief A smart pointer with reference-counted copy semantics.
     *
     * \tparam T the type of pointers stored in the SmartPointer.
     * \details
     * SmartPointer%s have the ability of taking ownership of a pointer of
     * type \p T and share that ownership: once they take ownership, the group
     * of owners of a pointer become responsible for its deletion when the
     * last one of them releases that ownership.
     *
     * The object pointed to must implement the two following static
     * functions:
     * - T::ref(T*): to increment the reference count
     * - T::unref(T*): to decrement the reference count.
     *
     * More specifically, SmartPointer can be used with classes inheriting the
     * Counted class.
     * \see Counted
     */
    template <class T>
    class SmartPointer {
    public:
        /**
         * \brief Creates an empty smart pointer
         */
        SmartPointer() :
            pointer_(nullptr) {
        }

        /**
         * \brief Creates a smart pointer that owns a pointer
         * \details This calls T::ref() on the pointer \p ptr to take
         * ownership on it.
         * \param[in] ptr source pointer convertible to T*
         */
        SmartPointer(T* ptr) :
            pointer_(ptr) {
            T::ref(pointer_);
        }

        /**
         * \brief Create a copy of a smart pointer
         * \details This calls T::ref() on the pointer help by \p rhs to take
         * ownership on it.
         * \param[in] rhs the smart pointer to copy
         */
        SmartPointer(const SmartPointer<T>& rhs) :
            pointer_(rhs) {
            T::ref(pointer_);
        }

        /**
         * \brief Deletes a smart pointer
         * \details This calls T::unref() to release ownership on the help
         * pointer. If this smart pointer is the last one owning the pointer,
         * the pointer is deleted.
         */
        ~SmartPointer() {
            T::unref(pointer_);
        }

        /**
         * \brief Assignment from a pointer
         * \details Releases ownership on the stored pointer as if reset()
         * were called and takes ownership on \p ptr.
         * \param[in] ptr a pointer convertible to T*
         * \return this smart pointer
         */
        SmartPointer<T>& operator= (T* ptr) {
            if(ptr != pointer_) {
                T::unref(pointer_);
                pointer_ = ptr;
                T::ref(pointer_);
            }
            return *this;
        }

        /**
         * \brief Assignment from a smart pointer
         * \details Releases ownership on the stored pointer as if reset()
         * were called and takes ownership on the pointer stored in \p rhs.
         * \param[in] rhs the smart pointer to copy
         * \return this smart pointer
         */
        SmartPointer<T>& operator= (const SmartPointer<T>& rhs) {
            T* rhs_p = rhs.get();
            if(rhs_p != pointer_) {
                T::unref(pointer_);
                pointer_ = rhs_p;
                T::ref(pointer_);
            }
            return *this;
        }

        /**
         * \brief Resets pointer
         * \details Releases ownership on the help pointer and resets it to
         * null. The smart pointer becomes as if it were default-constructed.
         * \note P.reset() is equivalent to assigning a nullptr pointer: p = nullptr
         */
        void reset() {
            T::unref(pointer_);
            pointer_ = nullptr;
        }

        /**
         * \brief Dereferences object member
         * \details Returns the stored pointer in order to access one of its
         * members. This member function shall not be called if the stored
         * pointer is a null pointer.
         * \return the stored pointer if not null, or aborts otherwise.
         */
        T* operator-> () const {
            geo_assert(pointer_ != nullptr);
            return pointer_;
        }

        /**
         * \brief Dereferences object
         * \details Returns the stored pointer in order to dereference it.
         * This member function shall not be called if the stored pointer is a
         * null pointer.
         * \return the stored pointer if not null, or aborts otherwise.
         */
        T& operator* () const {
            geo_assert(pointer_ != nullptr);
            return *pointer_;
        }

        /**
         * \brief Conversion operator
         * \return the stored pointer
         */
        operator T* () const {
            return pointer_;
        }

        /**
         * \brief Get pointer
         * \return the stored pointer
         */
        T* get() const {
            return pointer_;
        }

        /**
         * \brief Check if stored pointer is null
         * \return \c true if the stored pointer is null, \c false otherwise.
         */
        bool is_null() const {
            return pointer_ == nullptr;
        }

    private:
        T* pointer_;
    };

    /**
     * \brief Equal operator
     * \param[in] lhs the first pointer to compare
     * \param[in] rhs the second pointer to compare
     * \return \c true if the pointer stored in \p lhs is equal to the pointer
     * stored in \p rhs.
     * \relates SmartPointer
     */
    template <class T1, class T2>
    inline bool operator== (const SmartPointer<T1>& lhs, const SmartPointer<T2>& rhs) {
        return lhs.get() == rhs.get();
    }

    /**
     * \brief Not equal operator
     * \param[in] lhs the first pointer to compare
     * \param[in] rhs the second pointer to compare
     * \return \c true if the pointer stored in \p lhs is not equal to the
     * pointer stored in \p rhs.
     * \relates SmartPointer
     */
    template <class T1, class T2>
    inline bool operator!= (const SmartPointer<T1>& lhs, const SmartPointer<T2>& rhs) {
        return lhs.get() != rhs.get();
    }

    /**
     * \brief Less than operator
     * \param[in] lhs the first pointer to compare
     * \param[in] rhs the second pointer to compare
     * \return \c true if the pointer stored in \p lhs is less than the
     * pointer stored in \p rhs.
     * \relates SmartPointer
     */
    template <class T1, class T2>
    inline bool operator< (const SmartPointer<T1>& lhs, const SmartPointer<T2>& rhs) {
        return lhs.get() < rhs.get();
    }

    /**
     * \brief Less or equal operator
     * \param[in] lhs the first pointer to compare
     * \param[in] rhs the second pointer to compare
     * \return \c true if the pointer stored in \p lhs is less than or equal
     * to the pointer stored in \p rhs.
     * \relates SmartPointer
     */
    template <class T1, class T2>
    inline bool operator<= (const SmartPointer<T1>& lhs, const SmartPointer<T2>& rhs) {
        return lhs.get() <= rhs.get();
    }

    /**
     * \brief Greater than operator
     * \param[in] lhs the first pointer to compare
     * \param[in] rhs the second pointer to compare
     * \return \c true if the pointer stored in \p lhs is greater than the
     * pointer stored in \p rhs.
     * \relates SmartPointer
     */
    template <class T1, class T2>
    inline bool operator> (const SmartPointer<T1>& lhs, const SmartPointer<T2>& rhs) {
        return lhs.get() > rhs.get();
    }

    /**
     * \brief Greater or equal operator
     * \param[in] lhs the first pointer to compare
     * \param[in] rhs the second pointer to compare
     * \return \c true if the pointer stored in \p lhs is greater than or
     * equal to the pointer stored in \p rhs.
     * \relates SmartPointer
     */
    template <class T1, class T2>
    inline bool operator>= (const SmartPointer<T1>& lhs, const SmartPointer<T2>& rhs) {
        return lhs.get() >= rhs.get();
    }
}

#endif

