
#ifndef EXPLORAGRAM_API_DEFS
#define EXPLORAGRAM_API_DEFS

#include <geogram/api/defs.h>

/**
 * \file exploragram/api/defs.h
 * \brief Basic definitions for the exploragram API
 */

/**
 * \brief Linkage declaration for exploragram symbols.
 */
#ifdef exploragram_EXPORTS
#define EXPLORAGRAM_API GEO_EXPORT
#else
#define EXPLORAGRAM_API GEO_IMPORT
#endif

/**
 * \brief A place-holder linkage declaration to indicate
 *  that the symbol should not be exported by Windows DLLs.
 * \details For instance, classes that inherit templates from
 *  the STL should not be exported, else it generates multiply
 *  defined symbols.
 */
#define NO_EXPLORAGRAM_API


#endif

