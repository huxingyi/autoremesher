// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
///
/// @file PlatformConfig.h

#ifndef OPENVDB_PLATFORMCONFIG_HAS_BEEN_INCLUDED
#define OPENVDB_PLATFORMCONFIG_HAS_BEEN_INCLUDED

// Windows specific configuration
#ifdef _WIN32

    // By default, assume we're building OpenVDB as a DLL if we're dynamically
    // linking in the CRT, unless OPENVDB_STATICLIB is defined.
    #if defined(_DLL) && !defined(OPENVDB_STATICLIB) && !defined(OPENVDB_DLL)
        #define OPENVDB_DLL
    #endif

    // By default, assume that we're dynamically linking OpenEXR, unless
    // OPENVDB_OPENEXR_STATICLIB is defined.
    #if !defined(OPENVDB_OPENEXR_STATICLIB) && !defined(OPENEXR_DLL)
        #define OPENEXR_DLL
    #endif

#endif // _WIN32

#endif // OPENVDB_PLATFORMCONFIG_HAS_BEEN_INCLUDED
