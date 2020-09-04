/*
 *  Copyright (c) 2004-2010, Bruno Levy
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
 *     levy@loria.fr
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 */

#ifndef OPENNL_PRIVATE_H
#define OPENNL_PRIVATE_H

#include "nl.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#ifndef NDEBUG
#define NL_DEBUG
#endif

/**
 * \file geogram/NL/nl_private.h
 * \brief Some macros and functions used internally by OpenNL.
 */

#if defined(__APPLE__) && defined(__MACH__)
/**
 * \brief Defined if compiled on a Mac OS/X platform.
 */
#define NL_OS_APPLE
#endif

#if defined(__linux__) || defined(__ANDROID__) || defined(NL_OS_APPLE)
/**
 * \brief Defined if compiled on a Unix-like platform.
 */
#define NL_OS_UNIX
#endif


#if defined(WIN32) || defined(_WIN64)
/**
 * \brief Defined if compiled on a Windows platform.
 */
#define NL_OS_WINDOWS
#endif

/**
 * \brief Suppresses unsused argument warnings
 * \details Some callbacks do not necessary use all their
 *  arguments.
 * \param[in] x the argument to be tagged as used
 */
#define nl_arg_used(x) (void)x

/**
 * \name Assertion checks
 * @{ 
 */

#if defined(__clang__) || defined(__GNUC__)
#define NL_NORETURN __attribute__((noreturn))
#else
#define NL_NORETURN 
#endif

#if defined(_MSC_VER)
#define NL_NORETURN_DECL __declspec(noreturn) 
#else
#define NL_NORETURN_DECL 
#endif

/**
 * \brief Displays an error message and aborts the program when
 *  an assertion failed.
 * \details Called by nl_assert() whenever the assertion failed
 * \param[in] cond the textual representation of the condition
 * \param[in] file the source filename
 * \param[in] line the line number
 */
NL_NORETURN_DECL void nl_assertion_failed(
    const char* cond, const char* file, int line
) NL_NORETURN;

/**
 * \brief Displays an error message and aborts the program
 *  when a range assertion failed.
 * \details Called by nl_range_assert() whenever the assertion failed
 * \param[in] x the variable
 * \param[in] min_val the minimum value
 * \param[in] max_val the maximum value
 * \param[in] file the source filename
 * \param[in] line the line number
 */
NL_NORETURN_DECL void nl_range_assertion_failed(
    double x, double min_val, double max_val, const char* file, int line
) NL_NORETURN;

/**
 * \brief Displays an error message and aborts the program
 *  when the execution flow reached a point it should not
 *  have reached.
 * \details called by nl_assert_not_reached
 * \param[in] file the source filename
 * \param[in] line the line number
 */
NL_NORETURN_DECL void nl_should_not_have_reached(
    const char* file, int line
) NL_NORETURN;

/**
 * \brief Tests an assertion and aborts the program if the test fails
 * \param[in] x the condition to be tested
 */
#define nl_assert(x) {                                          \
    if(!(x)) {                                                  \
        nl_assertion_failed(#x,__FILE__, __LINE__) ;            \
    }                                                           \
} 

/**
 * \brief Tests a range assertion and aborts the program if the test fails
 * \param[in] x the variable to be tested
 * \param[in] min_val the minimum admissible value for the variable
 * \param[in] max_val the maximum admissible value for the variable
 */
#define nl_range_assert(x,min_val,max_val) {                         \
    if(((int)(x) < (int)(min_val)) || ((int)(x) > (int)(max_val))) { \
        nl_range_assertion_failed(x, min_val, max_val,               \
            __FILE__, __LINE__                                       \
        ) ;                                                          \
    }                                                                \
}

/**
 * \brief Triggers an assertion failure when the execution flow
 *  reaches a specific location in the code.
 */
#define nl_assert_not_reached {                                 \
    nl_should_not_have_reached(__FILE__, __LINE__) ;            \
}

#ifdef NL_DEBUG
    #define nl_debug_assert(x) nl_assert(x)
    #define nl_debug_range_assert(x,min_val,max_val)            \
                               nl_range_assert(x,min_val,max_val)
#else
    #define nl_debug_assert(x) 
    #define nl_debug_range_assert(x,min_val,max_val) 
#endif

#ifdef NL_PARANOID
    #define nl_parano_assert(x) nl_assert(x)
    #define nl_parano_range_assert(x,min_val,max_val)           \
                               nl_range_assert(x,min_val,max_val)
#else
    #define nl_parano_assert(x) 
    #define nl_parano_range_assert(x,min_val,max_val) 
#endif

/**
 * @}
 * \name Error reporting
 * @{ 
 */

/**
 * \brief Displays an error message
 * \param[in] function name of the function that triggered the error
 * \param[in] message error message
 */
void nlError(const char* function, const char* message) ;

/**
 * \brief Displays a warning message
 * \param[in] function name of the function that triggered the error
 * \param[in] message warning message
 */
void nlWarning(const char* function, const char* message) ;

/**
 * @}
 * \name OS
 * @{ 
 */

/**
 * \brief Gets the current time in seconds
 * \return the current time in seconds (starting from a given reference time)
 */
NLdouble nlCurrentTime(void);

/**
 * \brief Type for manipulating DLL/shared object/dylib handles.
 */
typedef void* NLdll;


/**
 * \brief Flag for nlOpenDLL(), resolve all symbols when opening the DLL.
 * \see nlOpenDLL()
 */
#define NL_LINK_NOW    1

/**
 * \brief Flag for nlOpenDLL(), resolve symbols only when they are called.
 * \see nlOpenDLL()
 */
#define NL_LINK_LAZY   2

/**
 * \brief Flag for nlOpenDLL(), add all loaded symbols to global namespace.
 */
#define NL_LINK_GLOBAL 4

/**
 * \brief Flag for nlOpenDLL(), do not display messages.
 */
#define NL_LINK_QUIET  8

/**
 * \brief Flag for nlOpenDLL(), use fallback geogram numerical library if
 *  library is not found in the system.
 */
#define NL_LINK_USE_FALLBACK 16

/**
 * \brief Dynamically links a DLL/shared object/dylib to the current process.
 * \param[in] filename the file name fo the DLL/shared object/dylib.
 * \param[in] flags an or-combination of NL_LINK_NOW, NL_LINK_LAZY, NL_LINK_GLOBAL,
 *  NL_LINK_QUIET.
 * \return a handle to the DLL/shared object/dylib or NULL if it could not
 *  be found.
 */
NLdll nlOpenDLL(const char* filename, NLenum flags);

/**
 * \brief Closes a DLL/shared object/dylib.
 * \param[in] handle a handle to a DLL/shared object/dylib that was previously
 *  obtained by nlOpenDLL()
 */
void nlCloseDLL(NLdll handle);

/**
 * \brief Finds a function in a DLL/shared object/dylib.
 * \param[in] handle a handle to a DLL/shared object/dylib that was previously
 *  obtained by nlOpenDLL()
 * \param[in] funcname the name of the function
 * \return a pointer to the function or NULL if no such function was found
 */
NLfunc nlFindFunction(NLdll handle, const char* funcname);

/******************************************************************************/
/* classic macros */

#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y)) 
#endif

#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y)) 
#endif


/**
 * @}
 * \name Memory management
 * @{ 
 */

/**
 * \brief Allocates a new element
 * \details Memory is zeroed after allocation
 * \param[in] T type of the element to be allocated
 */
#define NL_NEW(T)                (T*)(calloc(1, sizeof(T))) 

/**
 * \brief Allocates a new array of elements
 * \details Memory is zeroed after allocation
 * \param[in] T type of the elements 
 * \param[in] NB number of elements 
 */
#define NL_NEW_ARRAY(T,NB)       (T*)(calloc((size_t)(NB),sizeof(T)))

/**
 * \brief Changes the size of an already allocated array of elements
 * \details Memory is zeroed after allocation
 * \param[in] T type of the elements 
 * \param[in,out] x a pointer to the array to be resized
 * \param[in] NB number of elements 
 */
#define NL_RENEW_ARRAY(T,x,NB)   (T*)(realloc(x,(size_t)(NB)*sizeof(T))) 

/**
 * \brief Deallocates an element
 * \param[in,out] x a pointer to the element to be deallocated
 */
#define NL_DELETE(x)             free(x); x = NULL 

/**
 * \brief Deallocates an array
 * \param[in,out] x a pointer to the first element of the array to 
 *  be deallocated
 */
#define NL_DELETE_ARRAY(x)       free(x); x = NULL

/**
 * \brief Clears an element
 * \param[in] T type of the element to be cleared
 * \param[in,out] x a pointer to the element
 */
#define NL_CLEAR(T, x)           memset(x, 0, sizeof(T)) 

/**
 * \brief Clears an array of elements
 * \param[in] T type of the element to be cleared
 * \param[in,out] x a pointer to the element
 * \param[in] NB number of elements
 */
#define NL_CLEAR_ARRAY(T,x,NB)   memset(x, 0, (size_t)(NB)*sizeof(T)) 

/**
 * @}
 * \name Integer bounds
 * @{ 
 */

/**
 * \brief Maximum unsigned 32 bits integer
 */
#define NL_UINT_MAX 0xffffffff

/**
 * \brief Maximum unsigned 16 bits integer
 */
#define NL_USHORT_MAX 0xffff

/**
 * @}
 * \name Logging and messages
 * @{ 
 */

extern NLprintfFunc nl_printf;

extern NLfprintfFunc nl_fprintf;

/**
 * @}
 */

#endif
