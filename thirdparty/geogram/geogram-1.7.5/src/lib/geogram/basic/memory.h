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

#ifndef GEOGRAM_BASIC_MEMORY
#define GEOGRAM_BASIC_MEMORY

#include <geogram/basic/common.h>
#include <geogram/basic/assert.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/argused.h>
#include <geogram/basic/atomics.h>
#include <vector>
#include <string.h>
#include <stdlib.h>

#ifdef GEO_OS_WINDOWS

#include <windows.h>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#else

#include <unistd.h>

#endif

/**
 * \file geogram/basic/memory.h
 * \brief Types and functions for memory manipulation
 */

namespace GEO {

    /**
     * \brief Utilities for memory management.
     */
    namespace Memory {
        /** \brief Unsigned byte type */
        typedef unsigned char byte;

        /** \brief Unsigned 8 bits integer */
        typedef unsigned char word8;

        /** \brief Unsigned 16 bits integer */
        typedef unsigned short word16;

        /** \brief Unsigned 32 bits integer */
        typedef unsigned int word32;

        /** \brief Pointer to unsigned byte(s) */
        typedef byte* pointer;

	/** \brief Generic function pointer */
	typedef void (*function_pointer)();
	
        /**
         * \brief Clears a memory block
         * \details Clears (set to zero) the first \p size bytes of array \p
         * addr.
         * \param[in] addr an array of bytes
         * \param[in] size the number of bytes to clear
         */
        inline void clear(void* addr, size_t size) {
            ::memset(addr, 0, size);
        }

        /**
         * \brief Copies a memory block
         * \details Copies the first \p size bytes of array \p from to array
         * \p to. Note that this function has unpredictable results if the
         * memory areas pointed to by \p to and \p from overlap.
         * \param[in] to the destination array of bytes
         * \param[in] from the array of bytes to copy
         * \param[in] size the number of bytes to copy
         */
        inline void copy(void* to, const void* from, size_t size) {
            ::memcpy(to, from, size);
        }

	/**
	 * \brief Converts a function pointer to a generic pointer.
	 * \details In C++ it is not legal to convert between function pointers
	 *  and generic pointers using casts. Such conversion may be required when
	 *  retrieving symbols in dynamically linked libraries, or when interfacing
	 *  with scripting languages.
	 * \param[in] fptr the function pointer
	 * \return a generic pointer with the same address as \p fptr
	 */
	inline pointer function_pointer_to_generic_pointer(function_pointer fptr) {
	    // I know this is ugly, but I did not find a simpler warning-free
	    // way that is portable between all compilers.
	    pointer result = nullptr;
	    ::memcpy(&result, &fptr, sizeof(pointer));
	    return result;
	}

	/**
	 * \brief Converts a generic pointer to a function pointer.
	 * \details In C++ it is not legal to convert between function pointers
	 *  and generic pointers using casts. Such conversion may be required when
	 *  retrieving symbols in dynamically linked libraries, or when interfacing
	 *  with scripting languages.
	 * \param[in] ptr the generic pointer
	 * \return a function pointer with the same address as \p ptr
	 */
	inline function_pointer generic_pointer_to_function_pointer(pointer ptr) {
	    // I know this is ugly, but I did not find a simpler warning-free
	    // way that is portable between all compilers.
	    function_pointer result = nullptr;
	    ::memcpy(&result, &ptr, sizeof(pointer));
	    return result;
	}

	/**
	 * \brief Converts a generic pointer to a function pointer.
	 * \details In C++ it is not legal to convert between function pointers
	 *  and generic pointers using casts. Such conversion may be required when
	 *  retrieving symbols in dynamically linked libraries, or when interfacing
	 *  with scripting languages.
	 * \param[in] ptr the generic pointer
	 * \return a function pointer with the same address as \p ptr
	 */
	inline function_pointer generic_pointer_to_function_pointer(void* ptr) {
	    // I know this is ugly, but I did not find a simpler warning-free
	    // way that is portable between all compilers.
	    function_pointer result = nullptr;
	    ::memcpy(&result, &ptr, sizeof(pointer));
	    return result;
	}
	
        /**
         * \brief Default memory alignment for efficient vector operations
         * \details The memory alignment is given in bytes. Here is a list of
         * commonly used alignment values for various architectures:
         * - SSE: 16
         * - AVX: 32
         * - AVX-512: 64
         */
#define GEO_MEMORY_ALIGNMENT 64

        /**
         * \brief Defines the memory alignment of points in a vector
         * \details PointAlignment defines the memory alignment of points of
         * dimension \p DIM when they are stored contiguously in an array. The
         * alignment value is contained in the static data member \c value.
         * The PointAlignment template defines a default alignment of 1.
         * PointAlignment template specializations define specific values for
         * the most commonly used point dimensions.
         * \tparam DIM the dimension of the point.
         */
        template <int DIM>
        struct PointAlignment {
            /**
             * \brief Alignment value in bytes
             * \details The default value is 1 byte.
             */
            static const size_t value = 1;
        };

        /**
         * \brief PointAlignment specialization for points of dimension 2
         * \see PointAlignment
         */
        template <>
        struct PointAlignment<2> {
            static const size_t value = 16;
        };

        /**
         * \brief PointAlignment specialization for points of dimension 3
         * \see PointAlignment
         */
        template <>
        struct PointAlignment<3> {
            static const size_t value = 8;
        };

        /**
         * \brief PointAlignment specialization for points of dimension 4
         * \see PointAlignment
         */
        template <>
        struct PointAlignment<4> {
            static const size_t value = 32;
        };

        /**
         * \brief PointAlignment specialization for points of dimension 6
         * \see PointAlignment
         */
        template <>
        struct PointAlignment<6> {
            static const size_t value = 16;
        };

        /**
         * \brief PointAlignment specialization for points of dimension 8
         * \see PointAlignment
         */
        template <>
        struct PointAlignment<8> {
            static const size_t value = 64;
        };

        /**
         * \brief Gets a point alignment
         * \details This gives the alignment of a point of dimension \p dim
         * within an array of points aligned on GEO_MEMORY_ALIGNMENT bytes
         * \param[in] dim the dimension of the point
         * \see GEO::Memory::PointAlignment
         * \see GEO_MEMORY_ALIGNMENT
         */
#define geo_dim_alignment(dim) GEO::Memory::PointAlignment<dim>::value

        /**
         * \brief Allocates aligned memory.
         * \details The address of the allocated block will be a multiple of
         * \p alignment. Aligned memory blocks are required by vector
         * processing instructions (SSE, AVX...)
         * \param[in] size size of the block to allocate
         * \param[in] alignment memory alignment (must be a power of 2)
         * \note Memory alignment is not supported under Android.
         */
        inline void* aligned_malloc(
            size_t size, size_t alignment = GEO_MEMORY_ALIGNMENT
        ) {
#if   defined(GEO_OS_ANDROID)
            // Alignment not supported under Android.
            geo_argused(alignment);
            return malloc(size);
#elif defined(GEO_COMPILER_INTEL)
            return _mm_malloc(size, alignment);
#elif defined(GEO_COMPILER_GCC) || defined(GEO_COMPILER_CLANG)
            void* result;
            return posix_memalign(&result, alignment, size) == 0
                   ? result : nullptr;
#elif defined(GEO_COMPILER_MSVC)
            return _aligned_malloc(size, alignment);
#else
            geo_argused(alignment);
            return malloc(size);
#endif
        }

        /**
         * \brief Deallocates aligned memory
         * \details Deallocates the block of memory pointed to by \p p. Note
         * \p p that must have been previously allocated by aligned_malloc()
         * \see aligned_malloc()
         * \note Memory alignment is not supported under Android.
         */
        inline void aligned_free(void* p) {
#if   defined(GEO_OS_ANDROID)
            // Alignment not supported under Android.
            free(p);
#elif defined(GEO_COMPILER_INTEL)
            _mm_free(p);
#elif defined(GEO_COMPILER_GCC_FAMILY) 
            free(p);
#elif defined(GEO_COMPILER_MSVC)
            _aligned_free(p);
#else
            free(p);
#endif
        }

        /**
         * \def geo_decl_aligned(var)
         * \brief Specifies that a given variable should be memory-aligned.
         * \details
         *  It helps the compiler vectorizing loops,
         *  i.e. generating SSE/AVX/... code.
         * \param[in] var a variable in the current scope
         * \par Example
         * \code
         * geo_decl_aligned(double x);
         * \endcode
         * \note Memory alignment is not supported under Android.
         */
#if   defined(GEO_OS_ANDROID)
#define geo_decl_aligned(var) var
#elif defined(GEO_COMPILER_INTEL)
#define geo_decl_aligned(var) __declspec(aligned(GEO_MEMORY_ALIGNMENT)) var
#elif defined(GEO_COMPILER_GCC_FAMILY)
#define geo_decl_aligned(var) var __attribute__((aligned(GEO_MEMORY_ALIGNMENT)))
#elif defined(GEO_COMPILER_MSVC)
#define geo_decl_aligned(var) __declspec(align(GEO_MEMORY_ALIGNMENT)) var
#elif defined(GEO_COMPILER_EMSCRIPTEN)
#define geo_decl_aligned(var) var        
#endif

        /**
         * \def geo_assume_aligned(var, alignment)
         * \brief Informs the compiler that a given pointer is memory-aligned.
         * \details
         *  It helps the compiler vectorizing loops, i.e.
         *  generating SSE/AVX/... code.
         * \param[in] var a pointer variable in the current scope
         * \param[in] alignment the memory alignment (must be a power of 2)
         * \par Example
         * \code
         * double* p = ...;
         * geo_assume_aligned(p,alignment);
         * \endcode
         * \note Memory alignment is not supported under Android.
         */
#if   defined(GEO_OS_ANDROID)
#define geo_assume_aligned(var, alignment)
#elif defined(GEO_COMPILER_INTEL)
#define geo_assume_aligned(var, alignment) \
    __assume_aligned(var, alignment)
#elif defined(GEO_COMPILER_CLANG)
#define geo_assume_aligned(var, alignment)
        // GCC __builtin_assume_aligned is not yet supported by clang-3.3
#elif defined(GEO_COMPILER_GCC)
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 7
#define geo_assume_aligned(var, alignment) \
        *(void**) (&var) = __builtin_assume_aligned(var, alignment)
        // the GCC way of specifying that a pointer is aligned returns
        // the aligned pointer (I can't figure out why). It needs to be
        // affected otherwise it is not taken into account (verified by
        // looking at the output of gcc -S)
#else
#define geo_assume_aligned(var, alignment)        
#endif        
#elif defined(GEO_COMPILER_MSVC) 
#define geo_assume_aligned(var, alignment)
        // TODO: I do not know how to do that with MSVC
#elif defined(GEO_COMPILER_EMSCRIPTEN)        
#define geo_assume_aligned(var, alignment)
#elif defined(GEO_COMPILER_MINGW)        
#define geo_assume_aligned(var, alignment)
#endif

        /**
         * \def geo_restrict
         * \brief Informs the compiler that a given pointer has no aliasing
         * \details
         *  No aliasing means that no other pointer points to the same area of
         *  memory.
         * \code
         * double* geo_restrict p = ...;
         * \endcode
         */
#if   defined(GEO_COMPILER_INTEL)
#define geo_restrict __restrict
#elif defined(GEO_COMPILER_GCC_FAMILY)
#define geo_restrict __restrict__
#elif defined(GEO_COMPILER_MSVC)
#define geo_restrict __restrict
#elif defined(GEO_COMPILER_EMSCRIPTEN)
#define geo_restrict 
#endif

        /**
         * \brief Checks whether a pointer is aligned.
         * \param[in] p the pointer to check
         * \param[in] alignment memory alignment (must be a power of 2)
         * \retval true if \p is aligned on \p alignment bytes
         * \retval false otherwise
         */
        inline bool is_aligned(
            void* p, size_t alignment = GEO_MEMORY_ALIGNMENT
        ) {
            return (reinterpret_cast<size_t>(p) & (alignment - 1)) == 0;
        }

        /**
         * \brief Returns the smallest aligned memory address from p.
         */
        inline void* align(void* p) {
            size_t offset = (
                GEO_MEMORY_ALIGNMENT -
                (reinterpret_cast<size_t>(p) & (GEO_MEMORY_ALIGNMENT - 1))
            ) & (GEO_MEMORY_ALIGNMENT - 1);
            return reinterpret_cast<char*>(p) + offset;
        }

        /**
         * \brief Allocates aligned memory on the stack
         * \brief Allocates \p size bytes on the stack. The returned address
         * is guaranteed to be aligned on \c GEO_MEMORY_ALIGNMENT bytes. To
         * guarantee the memory alignment, the function may allocate more than
         * \p size, but not more than <tt>GEO_MEMORY_ALIGNMENT - 1</tt>.
         * \param[in] size Number of bytes to allocate.
         * \return An aligned pointer to a memory block of \p size bytes.
         */
#define geo_aligned_alloca(size) \
    GEO::Memory::align(alloca(size + GEO_MEMORY_ALIGNMENT - 1))

        /**
         * \brief An allocator that performs aligned memory allocations
         * \details
         * The allocator can be used as a template argument for STL
         * containers. It is required for efficient vectorization of the code
         * using vector processing units (SSE,AVX or AVX-512).
         */
        template <class T, int ALIGN = GEO_MEMORY_ALIGNMENT>
        class aligned_allocator {
        public:
            /** \brief Element type */
            typedef T value_type;

            /** \brief Pointer to element */
            typedef T* pointer;

            /** \brief Reference to element */
            typedef T& reference;

            /** \brief Pointer to constant element */
            typedef const T* const_pointer;

            /** \brief Reference to constant element */
            typedef const T& const_reference;

            /** \brief Quantities of elements */
            typedef ::std::size_t size_type;

            /** \brief Difference between two pointers */
            typedef ::std::ptrdiff_t difference_type;

            /**
             * \brief Defines the same allocator for other types
             * \tparam U type of the elements to allocate
             */
            template <class U>
            struct rebind {
                /** Equivalent allocator type to allocate elements of type \p U*/
                typedef aligned_allocator<U> other;
            };

            /**
             * \brief Gets the address of an object
             * \param[in] x a reference to an object of type T
             * \return a pointer to \p x
             */
            pointer address(reference x) {
                return &x;
            }

            /**
             * \brief Gets the address of a object
             * \param[in] x a const reference to an object of type T
             * \return a const_pointer to \p x
             */
            const_pointer address(const_reference x) {
                return &x;
            }

            /**
             * \brief Allocates a block of storage
             * \details Attempts to allocate a block of storage with a size
             * large enough to contain \p n elements of member type 
             * \c value_type (an alias of the allocator's template parameter), 
             * and returns a pointer to the first element. 
             * The storage is aligned on ALIGN bytes, but they are \b not 
             * constructed.
             * \param[in] nb_elt number of elements to allocate
             * \param[in] hint Either 0 or a valer 0 or a value previously
             * obtained by another call to allocate and not yet freed with
             * deallocate. When it is not 0, this value may be used as a hint
             * to improve performance by allocating the new block near the one
             * specified. The address of an adjacent element is often a good
             * choice.
             * \return A pointer to the initial element in the block of storage
             */
            pointer allocate(
                size_type nb_elt, ::std::allocator<void>::const_pointer hint = nullptr
            ) {
                geo_argused(hint);
                pointer result = static_cast<pointer>(
                    aligned_malloc(sizeof(T) * nb_elt, ALIGN)
                );
                return result;
            }

            /**
             * \brief Releases a block of storage
             * \details Releases a block of storage previously allocated with
             * member allocate()) and not yet released. The elements in the
             * array \b are not destroyed by a call to this member function.
             * \param[in] p Pointer to a block of storage previously allocated
             * with aligned_allocator::allocate.
             * \param[in] nb_elt Number of elements allocated on the call to
             * aligned_allocator::allocate() for this block of storage.
             * \see allocate()
             */
            void deallocate(pointer p, size_type nb_elt) {
                geo_argused(nb_elt);
                aligned_free(p);
            }

            /**
             * \brief Gets the maximum size possible to allocate
             * \return the maximum number of elements, each of member type
             * \c value_type that could potentially be allocated by a call to
             * member allocate().
             */
            size_type max_size() const {
                ::std::allocator<char> a;
                return a.max_size() / sizeof(T);
            }

            /**
             * \brief Constructs an object
             * \details Constructs an element object on the location pointed 
             *  by \p p.
             *  Notice that this does not allocate space for the element. It
             *  should already be available at p (see member allocate() to
             *  allocate space).
             * \param[in] p pointer to a location with enough storage space to
             *  contain an element of type value_type.
             * \param[in] val value to initialize the constructed element to.
             * \see allocate()
             */
            void construct(pointer p, const_reference val) {
                new (static_cast<void*>(p))value_type(val);
            }

            /**
             * \brief Destroys an object
             * \details Destroys in-place the object pointed by p. Notice that
             * this does not deallocate the storage for the element (see
             * member deallocate() to release storage space).
             * \param[in] p pointer to the object to be destroyed.
             * \see deallocate()
             */
            void destroy(pointer p) {
                p->~value_type();
#ifdef GEO_COMPILER_MSVC
                (void) p; // to avoid a "unreferenced variable" warning
#endif
            }

            /**
             * \brief Conversion operator to different aligned_allocator
             * \details Required when compiling under MSVC version <= 2010
             */
            template <class T2, int A2> operator aligned_allocator<T2, A2>() {
                return aligned_allocator<T2,A2>();
            }
        };

        /**
         * \brief Tests whether two aligned_allocator%s are equal.
         * \return Always true.
         */
        template <typename T1, int A1, typename T2, int A2>
        inline bool operator== (
            const aligned_allocator<T1, A1>&, const aligned_allocator<T2, A2>&
        ) {
            return true;
        }

        /**
         * \brief Tests whether two aligned_allocator%s are different.
         * \return Always false.
         */
        template <typename T1, int A1, typename T2, int A2>
        inline bool operator!= (
            const aligned_allocator<T1, A1>&, const aligned_allocator<T2, A2>&
        ) {
            return false;
        }
    }

    /************************************************************************/

    /**
     * \brief Vector with aligned memory allocation
     * \details
     * Class vector is a \c std::vector that uses a memory-aligned allocator
     * Memory-aligned allocation makes it well suited for SSE/AVX/... vector
     * code generation.
     * \see Memory::aligned_allocator
     */
    template <class T>
    class vector : public ::std::vector<T, Memory::aligned_allocator<T> > {
        /** 
         * \brief Shortcut to the base class type 
         */
        typedef ::std::vector<T, Memory::aligned_allocator<T> > baseclass;

    public:
        /**
         * \brief Creates an empty vector
         */
        vector() :
            baseclass() {
        }

        /**
         * \brief Creates a pre-allocated vector
         * \details Constructs a container with \p size elements. 
         *  Each element is default-constructed.
         * \param[in] size Number of elements to allocate
         */
        explicit vector(index_t size) :
            baseclass(size) {
        }

        /**
         * \brief Creates a pre-initialized vector
         * \details Constructs a container with \p size elements. 
         *  Each element is a copy of \p val.
         * \param[in] size Number of elements to allocate
         * \param[in] val Initial value of the elements
         */
        explicit vector(index_t size, const T& val) :
            baseclass(size, val) {
        }

        /**
         * \brief Gets the number of elements
         * \return The actual number of elements in the vector
         */
        index_t size() const {
            //   casts baseclass::size() from size_t (64 bits)
            //   to index_t (32 bits), because all
            //   indices in Vorpaline are supposed to fit in 32 bits (index_t).
            // TODO: geo_debug_assert(baseclass::size() < max index_t)
            return index_t(baseclass::size());
        }

        /**
         * \brief Gets a vector element
         * \param[in] i index of the element
         * \return A reference to the element at position \p i in the vector.
         */
        T& operator[] (index_t i) {
            geo_debug_assert(i < size());
            return baseclass::operator[] (i);
        }

        /**
         * \brief Gets a vector element
         * \param[in] i index of the element
         * \return A const reference to the element at position 
         *  \p i in the vector.
         */
        const T& operator[] (index_t i) const {
            geo_debug_assert(i < size());
            return baseclass::operator[] (i);
        }

        /**
         * \brief Gets a vector element
         * \param[in] i index of the element
         * \return A reference to the element at position \p i in the vector.
         */
        T& operator[] (signed_index_t i) {
            geo_debug_assert(i >= 0 && index_t(i) < size());
            return baseclass::operator[] (index_t(i));
        }

        /**
         * \brief Gets a vector element
         * \param[in] i index of the element
         * \return A const reference to the element at position \p i 
         *  in the vector.
         */
        const T& operator[] (signed_index_t i) const {
            geo_debug_assert(i >= 0 && index_t(i) < size());
            return baseclass::operator[] (index_t(i));
        }


#ifdef GARGANTUA // If compiled with 64 bits index_t

        /**
         * \brief Gets a vector element
         * \param[in] i index of the element
         * \return A reference to the element at position \p i in the vector.
         */
        T& operator[] (int i) {
            geo_debug_assert(i >= 0 && index_t(i) < size());
            return baseclass::operator[] (index_t(i));
        }

        /**
         * \brief Gets a vector element
         * \param[in] i index of the element
         * \return A const reference to the element at position \p i 
         *  in the vector.
         */
        const T& operator[] (int i) const {
            geo_debug_assert(i >= 0 && index_t(i) < size());
            return baseclass::operator[] (index_t(i));
        }

        /**
         * \brief Gets a vector element
         * \param[in] i index of the element
         * \return A reference to the element at position \p i in the vector.
         */
        T& operator[] (unsigned int i) {
            geo_debug_assert(i >= 0 && index_t(i) < size());
            return baseclass::operator[] (index_t(i));
        }

        /**
         * \brief Gets a vector element
         * \param[in] i index of the element
         * \return A const reference to the element at position \p i 
         *  in the vector.
         */
        const T& operator[] (unsigned int i) const {
            geo_debug_assert(i >= 0 && index_t(i) < size());
            return baseclass::operator[] (index_t(i));
        }
#endif	
	
        /**
         * \brief Gets a pointer to the array of elements
         * \return a pointer to the first element of the vector
         */
        T* data() {
            return size() == 0 ? nullptr : &(*this)[0];
        }

        /**
         * \brief Gets a pointer to the array of elements
         * \return a const pointer to the first element of the vector
         */
        const T* data() const {
            return size() == 0 ? nullptr : &(*this)[0];
        }

    };

    /**
     * \brief Specialization of vector for elements of type bool
     * \details This specialization uses std::vector<bool> directly without
     * memory alignment.
     * \see vector
     */
    template <>
    class vector<bool> : public ::std::vector<bool> {
        typedef ::std::vector<bool> baseclass;

    public:
        /** \copydoc vector::vector() */
        vector() :
            baseclass() {
        }

        /** \copydoc vector::vector(index_t) */
        explicit vector(index_t size) :
            baseclass(size) {
        }

        /** \copydoc vector::vector(index_t,const T&) */
        explicit vector(index_t size, bool val) :
            baseclass(size, val) {
        }

        /** \copydoc vector::size() */
        index_t size() const {
            //   casts baseclass::size() from size_t (64 bits)
            //   to index_t (32 bits), because all
            //   indices in Vorpaline are supposed to fit in 32 bits (index_t).
            // TODO: geo_debug_assert(baseclass::size() < max index_t)
            return index_t(baseclass::size());
        }

        // TODO: operator[] with bounds checking (more complicated
        // than just returning bool&, check implementation in STL).
    };
}

#endif

