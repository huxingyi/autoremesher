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

#include "nl_private.h"

#if (defined (WIN32) || defined(_WIN64))
#include <windows.h>
#  define NL_DLL_EXT ".dll"
#else
#include <sys/types.h>
#include <sys/times.h> 
#endif

#if defined(GEO_DYNAMIC_LIBS) && defined(NL_OS_UNIX)
#include <dlfcn.h>
#ifdef NL_OS_APPLE
#  define NL_DLL_EXT ".dylib"
#else
#  define NL_DLL_EXT ".so"
#endif
#endif

/******************************************************************************/
/* Assertions */


void nl_assertion_failed(const char* cond, const char* file, int line) {
    nl_fprintf(
        stderr, 
        "OpenNL assertion failed: %s, file:%s, line:%d\n",
        cond,file,line
    ) ;
    abort() ;
}

void nl_range_assertion_failed(
    double x, double min_val, double max_val, const char* file, int line
) {
    nl_fprintf(
        stderr, 
        "OpenNL range assertion failed: "
	"%f in [ %f ... %f ], file:%s, line:%d\n",
        x, min_val, max_val, file,line
    ) ;
    abort() ;
}

void nl_should_not_have_reached(const char* file, int line) {
    nl_fprintf(
        stderr, 
        "OpenNL should not have reached this point: file:%s, line:%d\n",
        file,line
    ) ;
    abort() ;
}


/******************************************************************************/
/* Timing */

#ifdef WIN32
NLdouble nlCurrentTime() {
    return (NLdouble)GetTickCount() / 1000.0 ;
}
#else
double nlCurrentTime() {
    clock_t user_clock ;
    struct tms user_tms ;
    user_clock = times(&user_tms) ;
    return (NLdouble)user_clock / 100.0 ;
}
#endif

/******************************************************************************/
/* DLLs/shared objects/dylibs */

#if defined(GEO_DYNAMIC_LIBS) 

#  if defined(NL_OS_UNIX)

NLdll nlOpenDLL(const char* name, NLenum flags_in) {
    void* result = NULL;
    int flags = 0;
    if((flags_in & NL_LINK_NOW) != 0) {
	flags |= RTLD_NOW;
    }
    if((flags_in & NL_LINK_LAZY) != 0) {
	flags |= RTLD_LAZY;
    }
    if((flags_in & NL_LINK_GLOBAL) != 0) {
	flags |= RTLD_GLOBAL;
    }
    if((flags_in & NL_LINK_QUIET) == 0) {
	nl_fprintf(stdout,"Trying to load %s\n", name);
    }
    result = dlopen(name, flags);
    if(result == NULL) {
	if((flags_in & NL_LINK_QUIET) == 0) {	
	    nl_fprintf(stderr,"Did not find %s,\n", name);
	    nl_fprintf(
		stderr,
		"Retrying with libgeogram_num_3rdparty"
		NL_DLL_EXT
		"\n"
	    );
	}
	if((flags_in & NL_LINK_USE_FALLBACK) != 0) {
	    result=dlopen(
		"libgeogram_num_3rdparty" NL_DLL_EXT,
		flags
	    );
	    if(result == NULL) {
		if((flags_in & NL_LINK_QUIET) == 0) {		    
		    nlError("nlOpenDLL/dlopen",dlerror());
		}
	    }
        }
    }
    if((flags_in & NL_LINK_QUIET) == 0 && result != NULL) {
	nl_fprintf(stdout,"Loaded %s\n", name);
    }
    
    return result;
}

void nlCloseDLL(void* handle) {
    dlclose(handle);
}

NLfunc nlFindFunction(void* handle, const char* name) {
    /*
     * It is not legal in modern C to cast a void*
     *  pointer into a function pointer, thus requiring this
     *  (quite dirty) function that uses a union.    
     */
    union {
        void* ptr;
        NLfunc fptr;
    } u;
    u.ptr = dlsym(handle, name);
    return u.fptr;
}

#  elif defined(NL_OS_WINDOWS)

NLdll nlOpenDLL(const char* name, NLenum flags) {
    /* Note: NL_LINK_LAZY and NL_LINK_GLOBAL are ignored. */
    void* result = LoadLibrary(name);
    if(result == NULL && ((flags & NL_LINK_USE_FALLBACK) != 0)) {
	if((flags & NL_LINK_QUIET) == 0) {
	    nl_fprintf(stderr,"Did not find %s,\n", name);
	    nl_fprintf(
		stderr,
		"Retrying with geogram_num_3rdparty"
		NL_DLL_EXT
		"\n");
	}
        result=LoadLibrary("geogram_num_3rdparty" NL_DLL_EXT);
    }
    return result;
}

void nlCloseDLL(void* handle) {
    FreeLibrary((HMODULE)handle);
}

NLfunc nlFindFunction(void* handle, const char* name) {
    return (NLfunc)GetProcAddress((HMODULE)handle, name);
}

#  endif

#else

NLdll nlOpenDLL(const char* name, NLenum flags) {
    nl_arg_used(name);
    nl_arg_used(flags);
#ifdef NL_OS_UNIX
    nlError("nlOpenDLL","Was not compiled with dynamic linking enabled");
    nlError("nlOpenDLL","(see VORPALINE_BUILD_DYNAMIC in CMakeLists.txt)");
#else    
    nlError("nlOpenDLL","Not implemented");
#endif    
    return NULL;
}

void nlCloseDLL(void* handle) {
    nl_arg_used(handle);
    nlError("nlCloseDLL","Not implemented");        
}

NLfunc nlFindFunction(void* handle, const char* name) {
    nl_arg_used(handle);
    nl_arg_used(name);
    nlError("nlFindFunction","Not implemented");            
    return NULL;
}

#endif

/************************************************************************************/
/* Error-reporting functions */

NLprintfFunc nl_printf = printf;
NLfprintfFunc nl_fprintf = fprintf;

void nlError(const char* function, const char* message) {
    nl_fprintf(stderr, "OpenNL error in %s(): %s\n", function, message) ; 
}

void nlWarning(const char* function, const char* message) {
    nl_fprintf(stderr, "OpenNL warning in %s(): %s\n", function, message) ; 
}

void nlPrintfFuncs(NLprintfFunc f1, NLfprintfFunc f2) {
    nl_printf = f1;
    nl_fprintf = f2;
}

/************************************************************************************/


