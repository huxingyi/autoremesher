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

#ifndef GEOGRAM_BASIC_COUNTED
#define GEOGRAM_BASIC_COUNTED

#include <geogram/basic/common.h>
#include <geogram/basic/smart_pointer.h>
#include <geogram/basic/assert.h>

/**
 * \file geogram/basic/counted.h
 * \brief Base class of reference-counted objects,
 *  to be used with smart pointers
 */

namespace GEO {

    /**
     * \brief Base class for reference-counted objects
     * \details Reference counted objects implement shared ownership with a
     * simple mechanism: objects willing to share ownership on a Counted object
     * must call ref() on this object and call unref() when they no longer
     * need it. The object is destroyed when no more objects hold a reference
     * on it (when the last holder calls unref()).
     *
     * Objects can benefit reference counted sharing simply by deriving from
     * Counted and implementing the virtual destructor.
     *
     * Reference acquisition and release can be done manually by explicitly
     * calling ref() or unref() on the reference counted objects, or can be
     * done automatically by using SmartPointer<T>.
     * \see SmartPointer
     */
    class GEOGRAM_API Counted {
    public:
        /**
         * \brief Increments the reference count
         * \details This function must be called to share ownership on this
         * object. Calling ref() will prevent this object from being deleted
         * when someone else releases ownership.
         */
        void ref() const {
            ++nb_refs_;
        }

        /**
         * \brief Decrements the reference count
         * \details This function must be called to release ownership on this
         * object when it's no longer needed. Whwen the reference count
         * reaches the value of 0 (zero), the object is simply deleted.
         */
        void unref() const {
            --nb_refs_;
            geo_debug_assert(nb_refs_ >= 0);
            if(nb_refs_ == 0) {
                delete this;
            }
        }

        /**
         * \brief Check if the object is shared
         * \details An object is considered as shared if at least 2 client
         * objects have called ref() on this object.
         * \return \c true if the object is shared, \c false otherwise
         */
        bool is_shared() const {
            return nb_refs_ > 1;
        }

        /**
	 * \brief Gets the number of references that point to this object.
	 * \return the number of references.
	 */
        int nb_refs() const {
	    return nb_refs_;
	}
    
        /**
         * \brief Increments the reference count
         * \details This calls ref() on object \p counted if it is not null.
         * \param[in] counted reference object to reference.
         */
        static void ref(const Counted* counted) {
            if(counted != nullptr) {
                counted->ref();
            }
        }

        /**
         * \brief Decrements the reference count
         * \details This calls unref() on object \p counted if it is not null.
         * \param[in] counted reference object to dereference.
         */
        static void unref(const Counted* counted) {
            if(counted != nullptr) {
                counted->unref();
            }
        }

    protected:
        /**
         * \brief Creates a reference counted object
         * \details This initializes the reference count to 0 (zero).
         */
        Counted() :
            nb_refs_(0) {
        }

        /**
         * \brief Destroys a reference counted object
         * \details The destructor is never called directly but indirectly
         * through unref(). If the reference counter is not null when the
         * destructor is called the program dies with an assertion failure.
         */
        virtual ~Counted();

    private:
        /** Forbid copy constructor */
        Counted(const Counted&);
        /** Forbid assignment operator */
        Counted& operator= (const Counted&);

        mutable int nb_refs_;
    };
}

#endif

