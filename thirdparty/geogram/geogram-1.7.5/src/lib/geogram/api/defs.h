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

#ifndef GEOGRAM_API_DEFS
#define GEOGRAM_API_DEFS

/**
 * \file geogram/api/defs.h
 * \brief Basic definitions for the Geogram C API
 */

/*
 * Deactivate warnings about documentation
 * We do that, because CLANG's doxygen parser does not know
 * some doxygen commands that we use (retval, copydoc) and
 * generates many warnings for them...
 */
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command" 
#endif

/**
 * \brief Linkage declaration for geogram symbols.
 */

#if defined(GEO_DYNAMIC_LIBS)
   #if defined(_MSC_VER)
      #define GEO_IMPORT __declspec(dllimport) 
      #define GEO_EXPORT __declspec(dllexport) 
   #elif defined(__GNUC__)
      #define GEO_IMPORT  
      #define GEO_EXPORT __attribute__ ((visibility("default")))
   #else
      #define GEO_IMPORT
      #define GEO_EXPORT
   #endif
#else
   #define GEO_IMPORT
   #define GEO_EXPORT
#endif

#ifdef geogram_EXPORTS
#define GEOGRAM_API GEO_EXPORT
#else
#define GEOGRAM_API GEO_IMPORT
#endif


/**
 * \brief A place-holder linkage declaration to indicate
 *  that the symbol should not be exported by Windows DLLs.
 * \details For instance, classes that inherit templates from
 *  the STL should not be exported, else it generates multiply
 *  defined symbols.
 */
#define NO_GEOGRAM_API

/**
 * \brief Opaque identifier of a mesh.
 * \details Used by the C API.
 */
typedef int GeoMesh;

/**
 * \brief Represents dimension (e.g. 3 for 3d, 4 for 4d ...).
 * \details Used by the C API.
 */
typedef unsigned char geo_coord_index_t;

/* 
 * If GARGANTUA is defined, then geogram is compiled 
 * with 64 bit indices. 
 */
#ifdef GARGANTUA

#include <stdint.h>

/**
 * \brief Represents indices.
 * \details Used by the C API.
 */
typedef uint64_t geo_index_t;

/**
 * \brief Represents possibly negative indices.
 * \details Used by the C API.
 */
typedef int64_t geo_signed_index_t;

#else

/**
 * \brief Represents indices.
 * \details Used by the C API.
 */
typedef unsigned int geo_index_t;

/**
 * \brief Represents possibly negative indices.
 * \details Used by the C API.
 */
typedef int geo_signed_index_t;

#endif

/**
 * \brief Represents floating-point coordinates.
 * \details Used by the C API.
 */
typedef double geo_coord_t;

/**
 * \brief Represents truth values.
 * \details Used by the C API.
 */
typedef int geo_boolean;

/**
 * \brief Thruth values (geo_boolean).
 * \details Used by the C API.
 */
enum {
    GEO_FALSE = 0,
    GEO_TRUE = 1
};

#endif

