// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
///
/// @file Platform.h

#ifndef OPENVDB_PLATFORM_HAS_BEEN_INCLUDED
#define OPENVDB_PLATFORM_HAS_BEEN_INCLUDED

#include "PlatformConfig.h"

#define PRAGMA(x) _Pragma(#x)

/// @name Utilities
/// @{
/// @cond OPENVDB_VERSION_INTERNAL
#define OPENVDB_PREPROC_STRINGIFY_(x) #x
/// @endcond
/// @brief Return @a x as a string literal.  If @a x is a macro,
/// return its value as a string literal.
/// @hideinitializer
#define OPENVDB_PREPROC_STRINGIFY(x) OPENVDB_PREPROC_STRINGIFY_(x)

/// @cond OPENVDB_VERSION_INTERNAL
#define OPENVDB_PREPROC_CONCAT_(x, y) x ## y
/// @endcond
/// @brief Form a new token by concatenating two existing tokens.
/// If either token is a macro, concatenate its value.
/// @hideinitializer
#define OPENVDB_PREPROC_CONCAT(x, y) OPENVDB_PREPROC_CONCAT_(x, y)
/// @}


/// Use OPENVDB_DEPRECATED to mark functions as deprecated.
/// It should be placed right before the signature of the function,
/// e.g., "OPENVDB_DEPRECATED void functionName();".
#ifdef OPENVDB_DEPRECATED
#undef OPENVDB_DEPRECATED
#endif
#ifdef _MSC_VER
    #define OPENVDB_DEPRECATED  __declspec(deprecated)
#else
    #define OPENVDB_DEPRECATED  __attribute__ ((deprecated))
#endif

/// Macro for determining if GCC version is >= than X.Y
#if defined(__GNUC__)
    #define OPENVDB_CHECK_GCC(MAJOR, MINOR) \
        (__GNUC__ > MAJOR || (__GNUC__ == MAJOR && __GNUC_MINOR__ >= MINOR))
#else
    #define OPENVDB_CHECK_GCC(MAJOR, MINOR) 0
#endif

/// OpenVDB now requires C++11
#define OPENVDB_HAS_CXX11 1

/// For compilers that need templated function specializations to have
/// storage qualifiers, we need to declare the specializations as static inline.
/// Otherwise, we'll get linker errors about multiply defined symbols.
#if defined(__GNUC__) && OPENVDB_CHECK_GCC(4, 4)
    #define OPENVDB_STATIC_SPECIALIZATION
#else
    #define OPENVDB_STATIC_SPECIALIZATION static
#endif


/// SIMD Intrinsic Headers
#if defined(OPENVDB_USE_SSE42) || defined(OPENVDB_USE_AVX)
    #if defined(_WIN32)
        #include <intrin.h>
    #elif defined(__GNUC__)
        #if defined(__x86_64__) || defined(__i386__)
            #include <x86intrin.h>
        #elif defined(__ARM_NEON__)
            #include <arm_neon.h>
        #endif
    #endif
#endif


/// Bracket code with OPENVDB_NO_UNREACHABLE_CODE_WARNING_BEGIN/_END,
/// as in the following example, to inhibit ICC remarks about unreachable code:
/// @code
/// template<typename NodeType>
/// void processNode(NodeType& node)
/// {
///     OPENVDB_NO_UNREACHABLE_CODE_WARNING_BEGIN
///     if (NodeType::LEVEL == 0) return; // ignore leaf nodes
///     int i = 0;
///     ...
///     OPENVDB_NO_UNREACHABLE_CODE_WARNING_END
/// }
/// @endcode
/// In the above, <tt>NodeType::LEVEL == 0</tt> is a compile-time constant expression,
/// so for some template instantiations, the line below it is unreachable.
#if defined(__INTEL_COMPILER)
    // Disable ICC remarks 111 ("statement is unreachable"), 128 ("loop is not reachable"),
    // 185 ("dynamic initialization in unreachable code"), and 280 ("selector expression
    // is constant").
    #define OPENVDB_NO_UNREACHABLE_CODE_WARNING_BEGIN \
        _Pragma("warning (push)") \
        _Pragma("warning (disable:111)") \
        _Pragma("warning (disable:128)") \
        _Pragma("warning (disable:185)") \
        _Pragma("warning (disable:280)")
    #define OPENVDB_NO_UNREACHABLE_CODE_WARNING_END \
        _Pragma("warning (pop)")
#elif defined(__clang__)
    #define OPENVDB_NO_UNREACHABLE_CODE_WARNING_BEGIN \
        PRAGMA(clang diagnostic push) \
        PRAGMA(clang diagnostic ignored "-Wunreachable-code")
    #define OPENVDB_NO_UNREACHABLE_CODE_WARNING_END \
        PRAGMA(clang diagnostic pop)
#else
    #define OPENVDB_NO_UNREACHABLE_CODE_WARNING_BEGIN
    #define OPENVDB_NO_UNREACHABLE_CODE_WARNING_END
#endif


/// @brief Bracket code with OPENVDB_NO_DEPRECATION_WARNING_BEGIN/_END,
/// to inhibit warnings about deprecated code.
/// @note Use this sparingly.  Remove references to deprecated code if at all possible.
/// @details Example:
/// @code
/// OPENVDB_DEPRECATED void myDeprecatedFunction() {}
///
/// {
///     OPENVDB_NO_DEPRECATION_WARNING_BEGIN
///     myDeprecatedFunction();
///     OPENVDB_NO_DEPRECATION_WARNING_END
/// }
/// @endcode
#if defined __INTEL_COMPILER
    #define OPENVDB_NO_DEPRECATION_WARNING_BEGIN \
        _Pragma("warning (push)") \
        _Pragma("warning (disable:1478)") \
        PRAGMA(message("NOTE: ignoring deprecation warning at " __FILE__  \
            ":" OPENVDB_PREPROC_STRINGIFY(__LINE__)))
    #define OPENVDB_NO_DEPRECATION_WARNING_END \
        _Pragma("warning (pop)")
#elif defined __clang__
    #define OPENVDB_NO_DEPRECATION_WARNING_BEGIN \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")
        // note: no #pragma message, since Clang treats them as warnings
    #define OPENVDB_NO_DEPRECATION_WARNING_END \
        _Pragma("clang diagnostic pop")
#elif defined __GNUC__
    #define OPENVDB_NO_DEPRECATION_WARNING_BEGIN \
        _Pragma("GCC diagnostic push") \
        _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"") \
        _Pragma("message(\"NOTE: ignoring deprecation warning\")")
    #define OPENVDB_NO_DEPRECATION_WARNING_END \
        _Pragma("GCC diagnostic pop")
#elif defined _MSC_VER
    #define OPENVDB_NO_DEPRECATION_WARNING_BEGIN \
        __pragma(warning(push)) \
        __pragma(warning(disable : 4996)) \
        __pragma(message("NOTE: ignoring deprecation warning at " __FILE__ \
            ":" OPENVDB_PREPROC_STRINGIFY(__LINE__)))
    #define OPENVDB_NO_DEPRECATION_WARNING_END \
        __pragma(warning(pop))
#else
    #define OPENVDB_NO_DEPRECATION_WARNING_BEGIN
    #define OPENVDB_NO_DEPRECATION_WARNING_END
#endif


/// @brief Bracket code with OPENVDB_NO_TYPE_CONVERSION_WARNING_BEGIN/_END,
/// to inhibit warnings about type conversion.
/// @note Use this sparingly.  Use static casts and explicit type conversion if at all possible.
/// @details Example:
/// @code
/// float value = 0.1f;
/// OPENVDB_NO_TYPE_CONVERSION_WARNING_BEGIN
/// int valueAsInt = value;
/// OPENVDB_NO_TYPE_CONVERSION_WARNING_END
/// @endcode
#if defined __INTEL_COMPILER
    #define OPENVDB_NO_TYPE_CONVERSION_WARNING_BEGIN
    #define OPENVDB_NO_TYPE_CONVERSION_WARNING_END
#elif defined __GNUC__
    // -Wfloat-conversion was only introduced in GCC 4.9
    #if OPENVDB_CHECK_GCC(4, 9)
        #define OPENVDB_NO_TYPE_CONVERSION_WARNING_BEGIN \
            _Pragma("GCC diagnostic push") \
            _Pragma("GCC diagnostic ignored \"-Wconversion\"") \
            _Pragma("GCC diagnostic ignored \"-Wfloat-conversion\"")
    #else
        #define OPENVDB_NO_TYPE_CONVERSION_WARNING_BEGIN \
            _Pragma("GCC diagnostic push") \
            _Pragma("GCC diagnostic ignored \"-Wconversion\"")
    #endif
    #define OPENVDB_NO_TYPE_CONVERSION_WARNING_END \
        _Pragma("GCC diagnostic pop")
#else
    #define OPENVDB_NO_TYPE_CONVERSION_WARNING_BEGIN
    #define OPENVDB_NO_TYPE_CONVERSION_WARNING_END
#endif

/// Helper macros for defining library symbol visibility
#ifdef OPENVDB_EXPORT
#undef OPENVDB_EXPORT
#endif
#ifdef OPENVDB_IMPORT
#undef OPENVDB_IMPORT
#endif
#ifdef __GNUC__
    #define OPENVDB_EXPORT __attribute__((visibility("default")))
    #define OPENVDB_IMPORT __attribute__((visibility("default")))
#endif
#ifdef _WIN32
    #ifdef OPENVDB_DLL
        #define OPENVDB_EXPORT __declspec(dllexport)
        #define OPENVDB_IMPORT __declspec(dllimport)
    #else
        #define OPENVDB_EXPORT
        #define OPENVDB_IMPORT
    #endif
#endif

/// All classes and public free standing functions must be explicitly marked
/// as \<lib\>_API to be exported. The \<lib\>_PRIVATE macros are defined when
/// building that particular library.
#ifdef OPENVDB_API
#undef OPENVDB_API
#endif
#ifdef OPENVDB_PRIVATE
    #define OPENVDB_API OPENVDB_EXPORT
#else
    #define OPENVDB_API OPENVDB_IMPORT
#endif
#ifdef OPENVDB_HOUDINI_API
#undef OPENVDB_HOUDINI_API
#endif
#ifdef OPENVDB_HOUDINI_PRIVATE
    #define OPENVDB_HOUDINI_API OPENVDB_EXPORT
#else
    #define OPENVDB_HOUDINI_API OPENVDB_IMPORT
#endif

#endif // OPENVDB_PLATFORM_HAS_BEEN_INCLUDED
