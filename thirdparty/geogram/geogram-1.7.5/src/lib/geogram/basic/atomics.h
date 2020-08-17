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

#ifndef GEOGRAM_BASIC_ATOMICS
#define GEOGRAM_BASIC_ATOMICS

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>

/**
 * \file geogram/basic/atomics.h
 * \brief Functions for atomic operations
 */

#ifdef GEO_OS_LINUX
#  if defined(GEO_OS_EMSCRIPTEN) 
#    define GEO_USE_DUMMY_ATOMICS
#  elif defined(GEO_OS_RASPBERRY)
#    define GEO_USE_ARM32_ATOMICS
#  elif defined(GEO_OS_ANDROID)
#    define GEO_USE_ANDROID_ATOMICS
#  else
#    define GEO_USE_X86_ATOMICS
#  endif
#endif

#if defined(GEO_USE_DUMMY_ATOMICS)

inline void geo_pause() {
}

inline char atomic_bittestandset_x86(volatile unsigned int*, unsigned int) {
    return 0;
}

inline char atomic_bittestandreset_x86(volatile unsigned int*, unsigned int) {
    return 0;
}

#elif defined(GEO_USE_ANDROID_ATOMICS)

/** A mutex for Android */
typedef GEO::Numeric::uint32 android_mutex_t;

inline void lock_mutex_android(volatile android_mutex_t* lock) {
    while(__sync_lock_test_and_set(lock, 1) != 0);
}

inline void unlock_mutex_android(volatile android_mutex_t* lock) {
    __sync_lock_release(lock);
}

inline unsigned int atomic_bitset_android(volatile unsigned int* ptr, unsigned int bit) {
    return __sync_fetch_and_or(ptr, 1u << bit) & (1u << bit);
}

inline unsigned int atomic_bitreset_android(volatile unsigned int* ptr, unsigned int bit) {
    return __sync_fetch_and_and(ptr, ~(1u << bit)) & (1u << bit);
}

inline void memory_barrier_android() {
    // Full memory barrier.
    __sync_synchronize();
}

inline void wait_for_event_android() {
    /* TODO */    
}

inline void send_event_android() {
    /* TODO */    
}

#elif defined(GEO_USE_ARM32_ATOMICS)

/** A mutex for ARM processors */
typedef GEO::Numeric::uint32 arm32_mutex_t;

/**
 * \brief Acquires a lock (ARM only)
 * \param[in,out] lock the mutex to lock
 */
inline void lock_mutex_arm32(volatile arm32_mutex_t* lock) {
    arm_mutex_t tmp;
    __asm__ __volatile__ (
        "1:     ldrex   %0, [%1]     \n" // read lock
        "       cmp     %0, #0       \n" // check if zero
        "       wfene                \n" // wait for event if non-zero
        "       strexeq %0, %2, [%1] \n" // attempt to store new value
        "       cmpeq   %0, #0       \n" // test if store succeeded
        "       bne     1b           \n" // retry if not
        "       dmb                  \n" // memory barrier
        : "=&r" (tmp)
        : "r" (lock), "r" (1)
        : "cc", "memory");
}

/**
 * \brief Releases a lock (ARM only)
 * \param[in,out] lock the mutex to unlock
 */
inline void unlock_mutex_arm32(volatile arm32_mutex_t* lock) {
    __asm__ __volatile__ (
        "       dmb              \n" // ensure all previous access are observed
        "       str     %1, [%0] \n" // clear the lock
        "       dsb              \n" // ensure completion of clear lock ...
        "       sev              \n" // ... before sending the event
        :
        : "r" (lock), "r" (0)
        : "cc", "memory");
}

/**
 * \brief Atomically tests and sets a bit (ARM only)
 * \details Sets the bit \p bit of the *\p ptr.
 * The function is atomic and acts as a read-write memory barrier.
 * \param[in] ptr a pointer to an unsigned integer
 * \param[in] bit index of the bit to set in *\p ptr
 * \retval a non-zero integer if the bit was previously set
 * \retval 0 if the bit was previously not set
 */
inline unsigned int atomic_bitset_arm32(volatile unsigned int* ptr, unsigned int bit) {
    unsigned int tmp;
    unsigned int result;
    unsigned int OK;
    __asm__ __volatile__ (
        "1:     ldrex   %1, [%5]           \n" // result = *ptr
        "       orr     %0, %1, %6, LSL %4 \n" // tmp = result OR (1 << bit)
        "       strex   %2, %0, [%5]       \n" // *ptr = tmp, status in OK
        "       teq     %2, #0             \n" // if !OK then
        "       bne     1b                 \n" //    goto 1:
        "       and     %1, %1, %6, LSL %4 \n" // result = result AND (1 << bit)
        : "=&r" (tmp), "=&r" (result), "=&r" (OK), "+m" (*ptr)
        : "r" (bit), "r" (ptr), "r" (1)
        : "cc"
    );
    return result;
}

/**
 * \brief Atomically tests and resets a bit (ARM only)
 * \details Resets the bit \p bit of *\p ptr.
 * The function is atomic and acts as a read-write memory barrier.
 * \param[in] ptr a pointer to an unsigned integer
 * \param[in] bit index of the bit to reset in *\p ptr
 * \retval a non-zero integer if the bit was previously reset
 * \retval 0 if the bit was previously not reset
 */
inline unsigned int atomic_bitreset_arm32(volatile unsigned int* ptr, unsigned int bit) {
    unsigned int tmp;
    unsigned int result;
    unsigned int OK;
    __asm__ __volatile__ (
        "1:     ldrex   %1, [%5]           \n" // result = *ptr
        "       bic     %0, %1, %6, LSL %4 \n" // tmp = result AND NOT(1 << bit)
        "       strex   %2, %0, [%5]       \n" // *ptr = tmp, status in OK
        "       teq     %2, #0             \n" // if !OK then
        "       bne     1b                 \n" //    goto 1:
        "       and     %1, %1, %6, LSL %4 \n" // result = result AND (1 << bit)
        : "=&r" (tmp), "=&r" (result), "=&r" (OK), "+m" (*ptr)
        : "r" (bit), "r" (ptr), "r" (1)
        : "cc"
    );
    return result;
}

/**
 * \brief Issues a memory and compiler barrier (ARM only)
 */
inline void memory_barrier_arm32() {
    __asm__ __volatile__ (
        "dmb \n"
        : : : "memory"
    );
}

/**
 * \brief Waits for an event (ARM only)
 */
inline void wait_for_event_arm32() {
    __asm__ __volatile__ (
        "wfe \n"
        : : : 
    );
}

/**
 * \brief Sends an event (ARM only)
 */
inline void send_event_arm32() {
    __asm__ __volatile__ (
        "dsb \n" // ensure completion of store operations
        "sev \n"
        : : : 
    );
}

#elif defined(GEO_USE_X86_ATOMICS)

#  define GEO_USE_X86_PAUSE

#  ifdef GEO_USE_X86_PAUSE

/**
 * \brief Issues a processor pause (INTEL only)
 */
inline void geo_pause() {
    __asm__ __volatile__ (
        "pause;\n"
    );
}

#  else
#    ifdef __ICC
#      define geo_pause _mm_pause
#    else
#      define geo_pause __builtin_ia32_pause
#    endif

#  endif

/**
 * \brief Atomically tests and sets a bit (INTEL only)
 * \details Sets bit \p bit of *\p ptr and returns its previous value.
 * The function is atomic and acts as a read-write memory barrier.
 * \param[in] ptr a pointer to an unsigned integer
 * \param[in] bit index of the bit to set in *\p ptr
 * \return the previous value of bit \p bit
 */
inline char atomic_bittestandset_x86(volatile unsigned int* ptr, unsigned int bit) {
    char out;
#if defined(__x86_64)
    __asm__ __volatile__ (
        "lock; bts %2,%1\n"  // set carry flag if bit %2 (bit) of %1 (ptr) is set
                             //   then set bit %2 of %1
        "sbb %0,%0\n"        // set %0 (out) if carry flag is set
        : "=r" (out), "=m" (*ptr)
        : "Ir" (bit)
        : "memory"
    );
#else
    __asm__ __volatile__ (
        "lock; bts %2,%1\n"  // set carry flag if bit %2 (bit) of %1 (ptr) is set
                             //   then set bit %2 of %1
        "sbb %0,%0\n"        // set %0 (out) if carry flag is set
        : "=q" (out), "=m" (*ptr)
        : "Ir" (bit)
        : "memory"
    );
#endif
    return out;
}

/**
 * \brief Atomically tests and resets a bit (INTEL only)
 * \details Resets bit \p bit of *\p ptr and returns its previous value.
 * The function is atomic and acts as a read-write memory barrier
 * \param[in] ptr a pointer to an unsigned integer
 * \param[in] bit index of the bit to reset in \p ptr
 * \return the previous value of bit \p bit
 */
inline char atomic_bittestandreset_x86(volatile unsigned int* ptr, unsigned int bit) {
    char out;
#if defined(__x86_64)
    __asm__ __volatile__ (
        "lock; btr %2,%1\n"  // set carry flag if bit %2 (bit) of %1 (ptr) is set
                             //   then reset bit %2 of %1
        "sbb %0,%0\n"        // set %0 (out) if carry flag is set
        : "=r" (out), "=m" (*ptr)
        : "Ir" (bit)
        : "memory"
    );
#else
    __asm__ __volatile__ (
        "lock; btr %2,%1\n"  // set carry flag if bit %2 (bit) of %1 (ptr) is set
                             //   then reset bit %2 of %1
        "sbb %0,%0\n"        // set %0 (out) if carry flag is set
        : "=q" (out), "=m" (*ptr)
        : "Ir" (bit)
        : "memory"
    );
#endif
    return out;
}

#elif defined(GEO_OS_APPLE)

#include <libkern/OSAtomic.h>

#elif defined(GEO_OS_WINDOWS)

#include <windows.h>
#include <intrin.h>
#pragma intrinsic(_InterlockedCompareExchange8)
#pragma intrinsic(_InterlockedCompareExchange16)
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_interlockedbittestandset)
#pragma intrinsic(_interlockedbittestandreset)
#pragma intrinsic(_ReadBarrier)
#pragma intrinsic(_WriteBarrier)
#pragma intrinsic(_ReadWriteBarrier)

#  ifdef GEO_COMPILER_MINGW
inline void geo_pause() {
}
#  endif

#endif // GEO_OS_WINDOWS

#endif

