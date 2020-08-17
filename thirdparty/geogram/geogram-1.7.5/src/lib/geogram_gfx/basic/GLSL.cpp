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

#include <geogram_gfx/basic/GLSL.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/command_line.h>
#include <cstdarg>
#include <cstdio>

#ifdef __clang__ 
#  pragma GCC diagnostic ignored "-Wpointer-bool-conversion"
#endif

#ifdef GEO_OS_EMSCRIPTEN
#  define GEO_THROW_GLSL_ERROR
#else
#  define GEO_THROW_GLSL_ERROR throw GLSL::GLSLCompileError();
#endif

namespace {

    using namespace GEO;
    
    /**
     * \brief Loads the content of an ASCII file in a buffer.
     * \details Memory ownership is transfered
     *   to the caller. Memory should be deallocated with
     *   delete[].
     * \param[in] filename the name of the file
     * \return a pointer to a buffer that contains the
     *   contents of the file. 
     */
     char* load_ASCII_file(const char* filename) {
        FILE* f = fopen(filename, "rt") ;
        if(!f) {
            Logger::err("GLSL")
                << "Could not open file: \'"
                << filename << "\'" << std::endl;
            return nullptr ;
        }
        /* 
         * An easy way of determining the length of a file:
         * Go to the end of the file, and ask where we are.
         */
        fseek(f, 0, SEEK_END) ;
        size_t size = size_t(ftell(f)) ;

        /* Let's go back to the beginning of the file */
        fseek(f, 0, SEEK_SET) ;
        
        char* result = new char[size+1] ;
        size_t read_size = fread(result, 1, size, f);
        if(read_size != size) {
            Logger::warn("GLSL")
                << "Could not read completely file \'"
                << filename << "\'" << std::endl;
        }
        result[size] = '\0' ;
        fclose(f) ;
        return result ;
    }

    /**
     * \brief Links a GLSL program and displays errors if any.
     * \details If errors where encountered, program is deleted
     *  and reset to zero.
     * \param[in,out] program the handle to the GLSL program
     */
    void link_program_and_check_status(GLuint& program) {
        glLinkProgram(program);
        GLint link_status;
        glGetProgramiv(program, GL_LINK_STATUS, &link_status);
        if(!link_status) {
            GLchar linker_message[4096];
            glGetProgramInfoLog(
                program, sizeof(linker_message), nullptr, linker_message
            );
            Logger::err("GLSL") << "linker status :"
                                << link_status << std::endl;
            Logger::err("GLSL") << "linker message:"
                                << linker_message << std::endl;
            if(!CmdLine::get_arg_bool("dbg:gfx")) {            
                glDeleteProgram(program);
                program = 0;
            }
        }
        if(CmdLine::get_arg_bool("dbg:gfx")) {
            GLSL::introspect_program(program);
        }
    }

    /**
     * \brief Dumps a shader source assembled from multiple strings
     *   and displays line numbers.
     * \details This function can be used to debug shaders that do
     *   not compile, it makes error tracking easier.
     * \param[in] sources a pointer to an array of strings
     * \param[in] nb_sources the number of strings 
     */
    void dump_program_source_with_line_numbers(
        const char** sources, index_t nb_sources
    ) {
        std::string all_sources;
        for(index_t i=0; i<nb_sources; ++i) {
            all_sources += sources[i];
        }
        std::vector<std::string> lines;
        String::split_string(all_sources, '\n', lines);
        bool prev_is_include = false;
        for(index_t i=0; i<lines.size(); ++i) {
            std::string line = lines[i];
            if(line.find("//import") != std::string::npos) {
                if(prev_is_include) {
                    line = "";
                } else {
                    line = "//   [...skipped //import directives...]";
                }
                prev_is_include = true;
            } else {
                prev_is_include = false;
            }
            if(line.length() > 0 && line[line.length()-1] == '\n') {
                line[line.length()-1] = ' ';
            }
            std::string line_number = String::to_string(i+1);
            while(line_number.length() < 4) {
                line_number += ' ';
            }
            Logger::out("GLSL") << line_number << "  " << line << std::endl;
        }
    }
}

namespace GEO {

    /***********************************************************************/
    
    namespace GLSL {
            
        void initialize() {
        }
            
        void terminate() {
        }
            
        /*************************************************************/

        const char* GLSLCompileError::what() const GEO_NOEXCEPT {
            return "GLSL Compile Error";
        }
            
        /*************************************************************/

        

        /**
         * \brief Parses in a string what looks like a version number.
         * \param[in] version_string a const reference to the string
         * \return the parsed version number, as a double precision floating
         *  point number.
         */
        static double find_version_number(const std::string& version_string) {
            // The way the driver exposes the version of GLSL may differ,
            // in some drivers the number comes in first position, in some
            // others it comes in last position, therefore we take the first
            // word that contains a valid number.
            std::vector<std::string> version_words;
            String::split_string(
                version_string, ' ', version_words
            );
            double version = 0.0;
            for(index_t i=0; i<version_words.size(); ++i) {
                version = atof(version_words[i].c_str());
                if(version != 0.0) {
                    break;
                }
            }
            // Some drivers expose version 4.4 as 4.4 and some others
            // as 440 !!
            if(version > 100.0) {
                version /= 100.0;
            }
            return version;
        }

        // If some drivers do not implement glGetString(GLSL_VERSION),
        // then we can determine the GLSL version from the OpenGL version,
        // may be more reliable...
        //
        //GLSL Version      OpenGL Version
        //1.10              2.0
        //1.20              2.1
        //1.30              3.0
        //1.40              3.1
        //1.50              3.2
        //3.30              3.3
        //4.00              4.0
        //4.10              4.1
        //4.20              4.2
        //4.30              4.3
        //4.40              4.4
        //4.50              4.5
        
        static double GLSL_version_from_OpenGL_version() {
            const char* opengl_ver_str = (const char*)glGetString(GL_VERSION);
            if(opengl_ver_str == nullptr) {
                Logger::warn("GLSL")
                    << "glGetString(GL_VERSION)"
                    << " did not answer, falling back to VanillaGL"
                    << std::endl;
               return 0.0;
            }
            double OpenGL_version = find_version_number(opengl_ver_str);
            
            Logger::out("GLSL")
                << "Determining GLSL version from OpenGL version"
                << std::endl;
            
            Logger::out("GLSL")
                << "OpenGL version = " << OpenGL_version
                << std::endl;

            double GLSL_version = 0.0;
            if(OpenGL_version >= 3.3) {
                GLSL_version = OpenGL_version;
            } else if(OpenGL_version == 2.0) {
                GLSL_version = 1.1;
            } else if(OpenGL_version == 2.1) {
                GLSL_version = 1.2;                
            } else if(OpenGL_version == 3.0) {
                GLSL_version = 1.3;                                
            } else if(OpenGL_version == 3.1) {
                GLSL_version = 1.4;
            } else if(OpenGL_version == 3.2) {
                GLSL_version = 1.5;                
            }

            if(GLSL_version == 0.0) {
                Logger::warn("GLSL") << "Could not determine GLSL version"
                                     << std::endl;
            } else {
                Logger::out("GLSL") << "GLSL version = "
                                     << GLSL_version
                                     << std::endl;
            }
            return GLSL_version;
        }

        
        double supported_language_version() {

            double GLSL_version = CmdLine::get_arg_double("gfx:GLSL_version");
            
            if(GLSL_version != 0.0) {
                Logger::out("GLSL") << "forced to version "
                                    << GLSL_version 
                                    << " (gfx:GLSL_version)" << std::endl;
                return GLSL_version;
            }

            const char* shading_language_ver_str = nullptr;

#ifdef GEO_GL_150
#ifndef GEO_OS_APPLE
            // glGetStringi() is the new way of querying OpenGL implementation
            if(glGetStringi) {
                shading_language_ver_str = (const char*)glGetStringi(
                    GL_SHADING_LANGUAGE_VERSION, 0
                );
		// Intel driver has glGetStringi() but it does not seem
		// to be implemented (triggers OpenGL errors). We make
		// them silent. We use glGetString() below.
		clear_gl_error_flags(__FILE__, __LINE__);
            }
#endif            
#endif            
            if(shading_language_ver_str == nullptr) {
                // Some buggy drivers do not implement glGetStringi(),
                // so I try also glGetString() (without the "i")
                shading_language_ver_str =
                    (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
            }

            // If the driver does not implement glGetString neither
            // glGetStringi with GL_SHADING_LANGUAGE_VERSION, then try
            // to deduce it from OpenGL version.
            if(shading_language_ver_str == nullptr) {
                return GLSL_version_from_OpenGL_version();
            }
           
            const char* vendor = (const char*)glGetString(GL_VENDOR);
            
            Logger::out("GLSL") << "vendor = " << vendor << std::endl;
            Logger::out("GLSL") << "version string = "
                                << shading_language_ver_str << std::endl;


            GLSL_version = find_version_number(shading_language_ver_str);
            Logger::out("GLSL") << "version = " << GLSL_version
                                << std::endl;
            return GLSL_version;
        }
            

        PseudoFileProvider::~PseudoFileProvider() {
        }
        
        
        GLuint compile_shader(
            GLenum target, const char** sources, index_t nb_sources
        ) {
            GLuint s_handle = glCreateShader(target);
            if(s_handle == 0) {
                Logger::err("GLSL") << "Could not create shader for target"
                                    << std::endl;
                switch(target) {
                case GL_VERTEX_SHADER:
                    Logger::err("GLSL") << " (target = GL_VERTEX_SHADER)"
                                        << std::endl;
                    break;
                case GL_FRAGMENT_SHADER:
                    Logger::err("GLSL")
                        << " (target = GL_FRAGMENT_SHADER)"
                        << std::endl;
                    break;
#ifdef GEO_GL_150
                case GL_COMPUTE_SHADER:
                    Logger::err("GLSL") << " (target = GL_COMPUTE_SHADER)"
                                        << std::endl;
                    break;
                case GL_TESS_CONTROL_SHADER:
                    Logger::err("GLSL") << " (target = GL_TESS_CONTROL_SHADER)"
                                        << std::endl;
                    break;
                case GL_TESS_EVALUATION_SHADER:
                    Logger::err("GLSL")
                        << " (target = GL_TESS_EVALUATION_SHADER)"
                        << std::endl;
                    break;
                case GL_GEOMETRY_SHADER:
                    Logger::err("GLSL")
                        << " (target = GL_GEOMETRY_SHADER)"
                        << std::endl;
                    break;
#endif                    
                default:
                    Logger::err("GLSL")
                        << " (unknown target)"
                        << std::endl;
                    break;
                }
		GEO_THROW_GLSL_ERROR;
            }
            glShaderSource(s_handle, (GLsizei)nb_sources, sources, nullptr);
            glCompileShader(s_handle);
            GLint compile_status;
            glGetShaderiv(s_handle, GL_COMPILE_STATUS, &compile_status);
            if(!compile_status) {
                GLchar compiler_message[4096];
                glGetShaderInfoLog(
                    s_handle, sizeof(compiler_message), nullptr,
		    compiler_message
                );

                Logger::out("GLSL") << "Error in program:"
                                    << std::endl;

		if(CmdLine::get_arg_bool("gfx:GL_debug")) {
		    dump_program_source_with_line_numbers(sources, nb_sources);
		}

                Logger::err("GLSL")
                    << "compiler status :"
                    << compile_status << std::endl;
                Logger::err("GLSL")
                    << "compiler message:" << '\n'
                    << compiler_message << std::endl;
		
                glDeleteShader(s_handle);
                s_handle = 0;
		GEO_THROW_GLSL_ERROR;		
            }
            return s_handle;
        }

        GLuint compile_shader(
            GLenum target,
            const char* source1,
            const char* source2,
            const char* source3,
            const char* source4,
            const char* source5,
            const char* source6,
            const char* source7,
            const char* source8,
            const char* source9,
            const char* source10,
            const char* source11,
            const char* source12,
            const char* source13,
            const char* source14,
            const char* source15,
            const char* source16,
            const char* source17,
            const char* source18,
            const char* source19,
            const char* source20
        ) {
            vector<const char*> sources;
            geo_assert(source1 != nullptr);
            if(source1 != nullptr) {
                sources.push_back(source1);
            }
            if(source2 != nullptr) {
                sources.push_back(source2);
            }
            if(source3 != nullptr) {
                sources.push_back(source3);
            }
            if(source4 != nullptr) {
                sources.push_back(source4);
            }
            if(source5 != nullptr) {
                sources.push_back(source5);
            }
            if(source6 != nullptr) {
                sources.push_back(source6);
            }
            if(source7 != nullptr) {
                sources.push_back(source7);
            }
            if(source8 != nullptr) {
                sources.push_back(source8);
            }
            if(source9 != nullptr) {
                sources.push_back(source9);
            }
            if(source10 != nullptr) {
                sources.push_back(source10);
            }
            if(source11 != nullptr) {
                sources.push_back(source11);
            }
            if(source12 != nullptr) {
                sources.push_back(source12);
            }
            if(source13 != nullptr) {
                sources.push_back(source13);
            }
            if(source14 != nullptr) {
                sources.push_back(source14);
            }
            if(source15 != nullptr) {
                sources.push_back(source15);
            }
            if(source16 != nullptr) {
                sources.push_back(source16);
            }
            if(source17 != nullptr) {
                sources.push_back(source17);
            }
            if(source18 != nullptr) {
                sources.push_back(source18);
            }
            if(source19 != nullptr) {
                sources.push_back(source19);
            }
            if(source20 != nullptr) {
                sources.push_back(source20);
            }

            if(CmdLine::get_arg_bool("dbg:gfx")) {
                std::ofstream out("last_shader.glsl");
                
                for(index_t i=0; i<sources.size(); ++i) {
                    out << sources[i];
                }
                
                Logger::out("GLSL") << "===== Shader source ===="
                                    << std::endl;

                dump_program_source_with_line_numbers(
                    &sources[0], sources.size()
                );
            }
            
            return compile_shader(target, &sources[0], sources.size());
        }


        void link_program(GLuint program) {
            link_program_and_check_status(program);
            if(program == 0) {
		GEO_THROW_GLSL_ERROR;				
            }
        }

        GLuint create_program_from_shaders_no_link(GLuint shader1, ...) {
            va_list args;            
            GLuint program = glCreateProgram();
            va_start(args,shader1);
            GLuint shader = shader1;
            while(shader != 0) {
                glAttachShader(program, shader);
                shader = va_arg(args, GLuint);
            }
            va_end(args);
            return program;
        }
        
        GLuint create_program_from_shaders(GLuint shader1, ...) {
            va_list args;            
            GLuint program = glCreateProgram();
            va_start(args,shader1);
            GLuint shader = shader1;
            while(shader != 0) {
                glAttachShader(program, shader);
                shader = va_arg(args, GLuint);
            }
            va_end(args);
            link_program_and_check_status(program);
            return program;
        }

        /*****************************************************************/

        GLuint create_program_from_string_no_link(
            const char* string_in, bool copy_string
        ) {
            GLuint program = glCreateProgram();
            
            // string will be temporarily modified (to insert '\0' markers)
            // but will be restored to its original state right after.
            char* string = const_cast<char*>(string_in);
            if(copy_string) {
                string = strdup(string_in);
            }
            
            char* src = string;
            
            bool err_flag = false;
            
            for(;;) {
                char* begin = strstr(src, "#BEGIN(");
                char* end = strstr(src, "#END(");
                
                if(begin == nullptr && end == nullptr) {
                    break;
                }
                
                if(begin == nullptr) {
                    Logger::err("GLSL") << "missing #BEGIN() statement"
                                        << std::endl;
                    err_flag = true;
                    break;
                }
                
                if(end == nullptr) {
                    Logger::err("GLSL") << "missing #END() statement"
                                        << std::endl;
                    err_flag = true;
                    break;
                }

                    
                if(begin > end) {
                    Logger::err("GLSL") << "#END() before #BEGIN()"
                                        << std::endl;
                    err_flag = true;
                    break;
                }

                char* begin_opening_brace = begin + strlen("#BEGIN");
                char* end_opening_brace = end + strlen("#END");
                
                char* begin_closing_brace = strchr(begin_opening_brace,')');
                char* end_closing_brace = strchr(end_opening_brace, ')');
                
                if(begin_closing_brace == nullptr) {
                    Logger::err("GLSL") << "#BEGIN: missing closing brace"
                                        << std::endl;
                    err_flag = true;
                    break;
                }

                if(end_closing_brace == nullptr) {
                    Logger::err("GLSL") << "#END: missing closing brace"
                                        << std::endl;
                    err_flag = true;
                    break;
                }
                    
                std::string begin_kw(
                    begin_opening_brace+1,
                    size_t((begin_closing_brace - begin_opening_brace) - 1)
                    );
                std::string end_kw(
                    end_opening_brace+1,
                    size_t((end_closing_brace - end_opening_brace) - 1)
                    );
                if(end_kw != begin_kw) {
                    Logger::err("GLSL")
                        << "Mismatch: #BEGIN(" << begin_kw
                        << ") / #END(" << end_kw << ")"
                        << std::endl;
                    err_flag = true;
                    break;
                }
                
                // Replace '#END(...)' with string end marker
                *end = '\0';
                
                GLenum shader_type = GLenum(0);
                if(begin_kw == "GL_VERTEX_SHADER") {
                    shader_type = GL_VERTEX_SHADER;
                } else if(begin_kw == "GL_FRAGMENT_SHADER") {
                    shader_type = GL_FRAGMENT_SHADER;
                }
#ifdef GEO_GL_150
                  else if(begin_kw == "GL_GEOMETRY_SHADER") {
                    shader_type = GL_GEOMETRY_SHADER;
                } else if(begin_kw == "GL_TESS_CONTROL_SHADER") {
                    shader_type = GL_TESS_CONTROL_SHADER;
                } else if(begin_kw == "GL_TESS_EVALUATION_SHADER") {
                    shader_type = GL_TESS_EVALUATION_SHADER;
                }
#endif
                  else {
                    Logger::err("GLSL") << begin_kw
                                        << ": No such shader type"
                                        << std::endl;
                    err_flag = true;
                    break;
                }
                
                src = begin_closing_brace+1;
                GLuint shader = 0;
                try {
                    shader = compile_shader(shader_type, src, nullptr);
                } catch(...) {
                    err_flag = true;
                    break;
                }
                glAttachShader(program, shader);
                
                // Restore '#END(...)' statement
                // ('#' was replaced with string end marker).
                *end = '#';
                
                src = end_closing_brace + 1;
            }
            
            if(copy_string) {
                free(string);
            }
            
            if(err_flag) {
                glDeleteProgram(program);
                return 0;
            }
            return program;
        }

        /*****************************************************************/

        GLuint create_program_from_file_no_link(const std::string& filename) {
            char* buffer = load_ASCII_file(filename.c_str());
            if(buffer == nullptr) {
                return 0;
            }
            GLuint result = 0;
#ifdef GEO_OS_EMSCRIPTEN
	    result = create_program_from_string_no_link(buffer,false);	    
#else	    
            try {
                // last argument to false:
                //  no need to copy the buffer, we know it
                // is not a string litteral.
                result = create_program_from_string_no_link(buffer,false);
            } catch(...) {
                delete[] buffer;
                throw;
            }
#endif	    
            return result;
        }

        /*****************************************************************/

        GLint GEOGRAM_GFX_API get_uniform_variable_offset(
            GLuint program, const char* varname
        ) {
#ifndef GEO_GL_150
            geo_argused(program);
            geo_argused(varname);
            return -1;
#else
            GLuint index = GL_INVALID_INDEX;
            glGetUniformIndices(program, 1, &varname, &index);
            if(index == GL_INVALID_INDEX) {
                Logger::err("GLUP")
                    << varname 
                    << ":did not find uniform state variable"
                    << std::endl;
		GEO_THROW_GLSL_ERROR;		
            }
            geo_assert(index != GL_INVALID_INDEX);
            GLint offset = -1;
            glGetActiveUniformsiv(
                program, 1, &index, GL_UNIFORM_OFFSET, &offset
            );
            geo_assert(offset != -1);
            return offset;
#endif            
        }

	size_t get_uniform_variable_array_stride(
            GLuint program, const char* varname
	) {
#ifndef GEO_GL_150
            geo_argused(program);
            geo_argused(varname);
            return size_t(-1);
#else
            GLuint index = GL_INVALID_INDEX;
            glGetUniformIndices(program, 1, &varname, &index);
            if(index == GL_INVALID_INDEX) {
                Logger::err("GLUP")
                    << varname 
                    << ":did not find uniform state variable"
                    << std::endl;
		GEO_THROW_GLSL_ERROR;		
            }
            geo_assert(index != GL_INVALID_INDEX);
            GLint stride = -1;
            glGetActiveUniformsiv(
                program, 1, &index, GL_UNIFORM_ARRAY_STRIDE, &stride
            );
            geo_assert(stride != -1);
            return size_t(stride);
#endif            
	}
	
        void introspect_program(GLuint program) {
            Logger::out("GLSL") << "Program " << program << " introspection:"
                                << std::endl;
            if(!glIsProgram(program)) {
                Logger::out("GLSL") << "  not a program !"
                                    << std::endl;
                return;
            }

            {
                GLint link_status;
                glGetProgramiv(program, GL_LINK_STATUS, &link_status);
                Logger::out("GLSL") << "  link status=" << link_status
                                    << std::endl;
            }

            {
                GLint active_attributes;
                glGetProgramiv(
                    program, GL_ACTIVE_ATTRIBUTES, &active_attributes
                );
                Logger::out("GLSL")
                    << "  active attributes=" << active_attributes
                    << std::endl;
                for(GLuint i=0; i<GLuint(active_attributes); ++i) {
                    GLsizei length;
                    GLint size;
                    GLenum type;
                    GLchar name[1024];
                    glGetActiveAttrib(
                        program, i, GLsizei(1024), &length, &size, &type, name
                    );
                    Logger::out("GLSL") << "    Attribute " << i << " : "
                                        << name
                                        << std::endl;
                }
            }

            {
                GLint active_uniforms;
                glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &active_uniforms);
                Logger::out("GLSL") << "  active uniforms=" << active_uniforms
                                    << std::endl;
                for(GLuint i=0; i<GLuint(active_uniforms); ++i) {
                    GLsizei length;
                    GLint size;
                    GLenum type;
                    GLchar name[1024];
                    glGetActiveUniform(
                        program, i, GLsizei(1024), &length, &size, &type, name
                    );
                    Logger::out("GLSL") << "    Uniform " << i << " : "
                                        << name
                                        << std::endl;
                }
            }

#ifdef GEO_GL_150            
            {
                GLint active_uniform_blocks;
                glGetProgramiv(
                    program, GL_ACTIVE_UNIFORM_BLOCKS, &active_uniform_blocks
                );
                Logger::out("GLSL") << "  active uniform blocks="
                                    << active_uniform_blocks
                                    << std::endl;
            }
#endif
            
        }
    }
}

   /***************** GLSL pseudo file system ****************************/

namespace GEO {
    namespace GLSL {
        namespace {

            /**
             * \brief Gets all pseudo file names included by a GLSL source.
             * \param[in] source the GLSL source
             * \param[out] includes a vector of string with all included pseudo
             *  file names.
             */
            void get_includes(
                const char* source, std::vector<std::string>& includes
            ) {
                includes.clear();
                const char* cur = source;
                while(cur != nullptr) {
                    cur = strstr(cur, "//import");
                    if(cur == nullptr) {
                        return;
                    }
                    cur += 8;
                    while(*cur == ' ') {
                        ++cur;
                        if(*cur == '\0') {
                            return;
                        }
                    }
                    if(*cur != '<') {
                        continue;
                    }
                    const char* next = strchr(cur, '>');
                    if(next != nullptr) {
                        includes.push_back(
                            std::string(cur+1, size_t(next-cur-1))
                        );
                    }
                    cur = next;
                }
            }

            /**
             * \brief A representation of a GLSL file in the pseudo file
             *  system.
             */
            struct File {

                /**
                 * \brief File default constructor.
                 */
                File() {
                    text = nullptr;
                    pseudo_file = nullptr;
                }
                
                /** 
                 * \brief The name of the file in the pseudo file system. 
                 */
                std::string name;

                /** 
                 * \brief The content of the file, or nullptr if it is a pseudo
                 *  file.
                 */
                const char* text;

                /**
                 * \brief A pointer to the function that generates the file
                 *  contents if it is a pseudo file, or nullptr if it is a 
                 *  regular file.
                 */
                PseudoFile  pseudo_file;

                /**
                 * \brief All the included files (directly or indirectly)
                 *  in the order they should be grouped to form the source.
                 */
                std::vector<File*> depends;
            };

            typedef std::map<std::string, File> FileSystem;

            FileSystem file_system_;



            /**
             * \brief Gets the dependencies of a given file.
             */
            void get_depends(File& F) {
                // Does nothing for pseudo files.
                if(F.text == nullptr) {
                    return;
                }

                std::vector<std::string> include_names;
                get_includes(F.text, include_names);
                std::vector<File*> includes(include_names.size());
            
                for(size_t i=0; i<include_names.size(); ++i) {
                    FileSystem::iterator it = file_system_.find(
                        include_names[i]
                    );
                    if(it == file_system_.end()) {
                        Logger::err("GLSL")
                            << F.name << " : include file "
                            << include_names[i]
                            << " not found in GLSL pseudo file system"
                            << std::endl;
                        geo_assert_not_reached;
                    }
                    includes[i] = &(it->second);
                }
                std::set<File*> included;
                for(size_t inc=0; inc<includes.size(); ++inc) {
                    File* include = includes[inc];
                    for(size_t dep=0; dep<include->depends.size(); ++dep) {
                        File* depend = include->depends[dep];
                        if(included.find(depend) == included.end()) {
                            included.insert(depend);
                            F.depends.push_back(depend);
                        }
                    }
                    if(included.find(include) != included.end()) {
                        Logger::err("GLSL")
                            << F.name << " : include file "
                            << include_names[inc]
                            << " circularly included"
                            << std::endl;
                        geo_assert_not_reached;                    
                    }
                    included.insert(include);
                    F.depends.push_back(include);
                }
            }
        }
    }
}

namespace GEO {
    namespace GLSL {

        void register_GLSL_include_file(
            const std::string& name, const char* source
        ) {
            geo_assert(file_system_.find(name) == file_system_.end());
            File& F = file_system_[name];
            F.name = name;
            F.text = source;
            F.pseudo_file = nullptr;
            get_depends(F);
        }

        void register_GLSL_include_file(
            const std::string& name, PseudoFile file
        ) {
            geo_assert(file_system_.find(name) == file_system_.end());
            File& F = file_system_[name];
            F.name = name;
            F.text = nullptr;
            F.pseudo_file = file;
        }

        
        const char* get_GLSL_include_file(
            const std::string& name
        ) {
            FileSystem::iterator it = file_system_.find(name);
            if(it == file_system_.end()) {

                for(FileSystem::iterator jt = file_system_.begin();
                    jt != file_system_.end(); ++jt) {
                    Logger::err("GLSL") << "FileSystem has: " << jt->first
                                        << std::endl;
                }

                
                Logger::err("GLSL")
                    << name 
                    << " : not found in GLSL pseudo file system"
                    << std::endl;
                geo_assert_not_reached;
            }
            if(it->second.text == nullptr) {
                Logger::err("GLSL")
                    << name
                    << " : is a pseudo-file"
                    << std::endl;
                geo_assert_not_reached;
            }
            return it->second.text;
        }
        
        
        GLuint compile_shader_with_includes(
            GLenum target, const char* source, PseudoFileProvider* provider
        ) {
            File F;
            F.text = source;
            get_depends(F);

            std::vector<Source> sources;
            std::vector<const char*> sources_texts;
            
            for(size_t dep=0; dep<F.depends.size(); ++dep) {
                if(F.depends[dep]->pseudo_file != nullptr) {
		    F.depends[dep]->pseudo_file(provider,sources);
                } else {
                    sources.push_back(F.depends[dep]->text);
                }
            }
            sources.push_back(source);

            sources_texts.resize(sources.size());
            for(size_t i=0; i<sources.size(); ++i) {
                sources_texts[i] = sources[i].text();
            }

            return compile_shader(
                target, &sources_texts[0], index_t(sources_texts.size())
            );
        }

        GLuint compile_program_with_includes_no_link(
            PseudoFileProvider* provider,            
            const char* shader1, const char* shader2, const char* shader3,
            const char* shader4, const char* shader5, const char* shader6
        ) {
            std::vector<const char*> sources;

            GLuint program = glCreateProgram();            
            
            if(shader1 != nullptr) {
                sources.push_back(shader1);
            }
            
            if(shader2 != nullptr) {
                sources.push_back(shader2);
            }
            
            if(shader3 != nullptr) {
                sources.push_back(shader3);
            }
            
            if(shader4 != nullptr) {
                sources.push_back(shader4);
            }
            
            if(shader5 != nullptr) {
                sources.push_back(shader5);
            }
            
            if(shader6 != nullptr) {
                sources.push_back(shader6);
            }

            for(size_t i=0; i<sources.size(); ++i) {
                const char* p1 = strstr(sources[i], "//stage ");
                if(p1 == nullptr) {
                    Logger::err("GLSL")
                        << "Missing //stage GL_xxxxx declaration"
                        << std::endl;
		    GEO_THROW_GLSL_ERROR;		    
                }
                p1 += 8;
                const char* p2 = strchr(p1, '\n');
                if(p2 == nullptr) {
                    Logger::err("GLSL")
                        << "Missing CR in //stage GL_xxxxx declaration"
                        << std::endl;
		    GEO_THROW_GLSL_ERROR;		    
                }
                std::string stage_str(p1, size_t(p2-p1));
                GLenum stage = 0;
                if(stage_str == "GL_VERTEX_SHADER") {
                    stage = GL_VERTEX_SHADER;
                } else if(stage_str == "GL_FRAGMENT_SHADER") {
                    stage = GL_FRAGMENT_SHADER;
                }

#ifndef GEO_OS_EMSCRIPTEN
                else if(stage_str == "GL_GEOMETRY_SHADER") {
                    stage = GL_GEOMETRY_SHADER;
                } else if(stage_str == "GL_TESS_CONTROL_SHADER") {
                    stage = GL_TESS_CONTROL_SHADER;
                } else if(stage_str == "GL_TESS_EVALUATION_SHADER") {
                    stage = GL_TESS_EVALUATION_SHADER;
                }
#endif
                else {
                    Logger::err("GLSL") << stage_str << ": unknown stage"
                                        << std::endl;
		    GEO_THROW_GLSL_ERROR;		    
                }

                GLuint shader =
                    compile_shader_with_includes(stage, sources[i], provider);
                
                glAttachShader(program, shader);

                // It is reference-counted by OpenGL
                //   (and it is attached to the program)
                glDeleteShader(shader);
                
            }
            return program;
        }

	
        
    }
}
    

