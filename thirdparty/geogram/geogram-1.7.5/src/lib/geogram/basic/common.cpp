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

#include <geogram/basic/common.h>
#include <geogram/basic/process.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/progress.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/file_system.h>
#include <geogram/basic/stopwatch.h>
#include <geogram/numerics/multi_precision.h>
#include <geogram/numerics/predicates.h>
#include <geogram/delaunay/delaunay.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/version.h>
#include <geogram/bibliography/bibliography.h>

#include <geogram/image/image.h>
#include <geogram/image/image_library.h>
#include <geogram/image/image_serializer_stb.h>
#include <geogram/image/image_serializer_xpm.h>
#include <geogram/image/image_serializer_pgm.h>

#include <sstream>
#include <iomanip>

#ifdef GEO_OS_EMSCRIPTEN
#include <emscripten.h>
#endif

namespace GEO {

    void initialize(int flags) {
	static bool initialized = false;

	if(initialized) {
	    return;
	}
	
        // When locale is set to non-us countries,
        // this may cause some problems when reading
        // floating-point numbers (some locale expect
        // a decimal ',' instead of a '.').
        // This restores the default behavior for
        // reading floating-point numbers.
#ifdef GEO_OS_UNIX
        setenv("LC_NUMERIC","POSIX",1);
#endif

#ifndef GEOGRAM_PSM						
        Environment* env = Environment::instance();
        env->set_value("version", VORPALINE_VERSION);
        env->set_value("release_date", VORPALINE_BUILD_DATE);
        env->set_value("SVN revision", VORPALINE_SVN_REVISION);        
#endif
	FileSystem::initialize();
        Logger::initialize();
        Process::initialize(flags);
        Progress::initialize();
        CmdLine::initialize();
        PCK::initialize();
        Delaunay::initialize();

#ifndef GEOGRAM_PSM		
	Biblio::initialize();
#endif
        atexit(GEO::terminate);

#ifndef GEOGRAM_PSM	
        mesh_io_initialize();
#endif
	
        // Clear last system error
        errno = 0;

#ifndef GEOGRAM_PSM		
        // Register attribute types that can be saved into files.
        geo_register_attribute_type<Numeric::uint8>("bool");                
        geo_register_attribute_type<char>("char");        
        geo_register_attribute_type<int>("int");
        geo_register_attribute_type<unsigned int>("unsigned int");	
        geo_register_attribute_type<index_t>("index_t");
        geo_register_attribute_type<signed_index_t>("signed_index_t");	
        geo_register_attribute_type<float>("float");
        geo_register_attribute_type<double>("double");

        geo_register_attribute_type<vec2>("vec2");
        geo_register_attribute_type<vec3>("vec3");
#endif
	
#ifdef GEO_OS_EMSCRIPTEN
        
        // This mounts the local file system when an emscripten-compiled
        // program runs in node.js.
        // Current working directory is mounted in /working,
        // and root directory is mounted in /root
        
        EM_ASM(
            if(typeof module !== 'undefined' && this.module !== module) {
                FS.mkdir('/working');
                FS.mkdir('/root');            
                FS.mount(NODEFS, { root: '.' }, '/working');
                FS.mount(NODEFS, { root: '/' }, '/root');
            }
        );
#endif

#ifndef GEOGRAM_PSM
        ImageLibrary::initialize() ;

        geo_declare_image_serializer<ImageSerializerSTBReadWrite>("png");
        geo_declare_image_serializer<ImageSerializerSTBReadWrite>("jpg");
        geo_declare_image_serializer<ImageSerializerSTBReadWrite>("jpeg");
        geo_declare_image_serializer<ImageSerializerSTBReadWrite>("tga");
        geo_declare_image_serializer<ImageSerializerSTBReadWrite>("bmp");
	
        geo_declare_image_serializer<ImageSerializer_xpm>("xpm") ;
        geo_declare_image_serializer<ImageSerializer_pgm>("pgm") ;		
#endif
	
	initialized = true;
    }

    void terminate() {
        if(
            CmdLine::arg_is_declared("sys:stats") &&
            CmdLine::get_arg_bool("sys:stats") 
        ) {
            Logger::div("System Statistics");
            PCK::show_stats();
            Process::show_stats();
        }

        PCK::terminate();

#ifndef GEOGRAM_PSM
        ImageLibrary::terminate() ;
	Biblio::terminate();
#endif
	
        Progress::terminate();
        Process::terminate();
        CmdLine::terminate();
        Logger::terminate();
	FileSystem::terminate();
        Environment::terminate();
    }
}

