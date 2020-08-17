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

#ifndef GEOGRAM_GFX_BASIC_GLSL
#define GEOGRAM_GFX_BASIC_GLSL

#include <geogram_gfx/basic/common.h>
#include <geogram/basic/numeric.h>

/**
 * \file geogram_gfx/basic/GLSL.h
 * \brief Utilities for manipulating GLSL shaders.
 */

namespace GEO {

    namespace GLSL {

        /**
         * \brief Initializes some GLSL functions and objects.
         * \details Called by GEO::Graphics::initialize()
         */  
        void GEOGRAM_GFX_API initialize();

        /**
         * \brief Terminates GLSL functions and objects.
         * \details Called by GEO::Graphics::terminate()
         */  
        void GEOGRAM_GFX_API terminate();
        
        /**
         * \brief Exception thrown when a GLSL shader fails to
         *  compiled.
         * \details Can occur when OpenGL driver or hardware
         *  does not support some features.
         */
        struct GEOGRAM_GFX_API GLSLCompileError : std::exception {

            /**
             * \brief Gets the string identifying the exception
             */
            virtual const char* what() const GEO_NOEXCEPT;
        };


        /**
         * \brief Gets the supported GLSL language version.
         * \details The supported GLSL version is determined
         *  from hardware/driver capabilities and user-defined
         *  parameters.
         */
        double GEOGRAM_GFX_API supported_language_version();

        /************************************************************/

        /**
         * \brief A GLSL source.
         * \details Can be a pointer to a static string in constant memory
         *  or a dynamically created string. 
         */
        class Source {
        public:
            /**
             * \brief Source constructor.
             */
            Source() : text_(nullptr) {
            }

            /**
             * \brief Source constructor from a char pointer.
             * \param[in] src a pointer to the source text. Can be a constant
             *  string. It is not copied.
             */
            Source(const char* src) : text_(src) {
            }

            /**
             * \brief Source constructor from a string.
             * \param[in] src a const reference to a string. It is copied
             *  into this Source.
             */
            Source(const std::string& src) : text_string_(src) {
                text_ = text_string_.c_str();
            }

            /**
             * \brief Source copy constructor.
             * \param[in] rhs a const reference to the Source to be copied
             */
            Source(const Source& rhs) {
                copy(rhs);
            }

            /**
             * \brief Source assignment operator.
             * \param[in] rhs a const reference to the Source to be copied.
             * \return this Source after assignment
             */
            Source& operator=(const Source& rhs) {
                if(&rhs != this) {
                    copy(rhs);
                }
                return *this;
            }

            /**
             * \brief Gets the text.
             * \return a const pointer to the text of this source.
             */
            const char* text() {
                return text_;
            }
            
        protected:
            /**
             * \brief Copies a Source.
             * \param[in] rhs a const reference to the Source to be copied.
             */
            void copy(const Source& rhs) {
                if(rhs.text_string_ != "") {
                    text_string_ = rhs.text_string_;
                    text_ = text_string_.c_str();
                } else {
                    text_ = rhs.text_;
                }
            }
            
        private:
            const char* text_;
            std::string text_string_;
        };

        /**
         * \brief A class that can register functions to the GLSL
         *  pseudo file system.
         * \details The GLSL pseudo file system manages the
         *  include directives.
         */
        class GEOGRAM_GFX_API PseudoFileProvider {
        public:
            /**
             * \brief PseudoFileProvider destructor.
             */
            virtual ~PseudoFileProvider();
        };

        /**
         * \brief A pointer to a function registered as a pseudo file
         *  in the GLSL pseudo file system.
         * \details The GLSL pseudo file system manages the
         *  include directives. A PseudoFile is a pointer to a function
         *  that fills-in the file contents. The file contents is a vector
         *  of Source objects, that can be either constant string litterals
         *  or dynamically created strings.
         */
        typedef void (*PseudoFile)(
            PseudoFileProvider* provider, std::vector<Source>& sources
        );


        /**
         * \brief Registers a file in the GLSL pseudo file system.
         * \details The file can then be included in a GLSL source
         *  with the //include <name> directive.
         * \param[in] name the name of the pseudo file
         * \param[in] source the GLSL source of the file
         */
        void GEOGRAM_GFX_API register_GLSL_include_file(
            const std::string& name, const char* source
        );

        /**
         * \brief Registers a pseudo file in the GLSL pseudo file system.
         * \details The pseudo file can then be included in a GLSL source
         *  with the //include <name> directive. Each time it is included,
         *  it is generated by calling the specified function.
         * \param[in] name the name of the pseudo file
         * \param[in] file a pointer to a member function of an object
         *  derived from a PseudoFileProvider object that returns a string.
         */
        void GEOGRAM_GFX_API register_GLSL_include_file(
            const std::string& name, PseudoFile file
        );


        /**
         * \brief Gets a GLSL include file by file name.
         * \details It needs to be a real file, registered as
         *  a pointer to static text data (not a PseudoFile).
         * \param[in] name the name of the file in the pseudo
         *  file system.
         * \return a const pointer to the contents of the file.
         */
        const char* get_GLSL_include_file(
            const std::string& name
        );
        
        /**
         * \brief Compiles a shader for a specific target.
         * \details This version of compile_shader() supports the 
         *  include directive through the GLSL pseudo file system. 
         *  Errors are detected and displayed to std::err.
         * \param[in] target the OpenGL shader target 
         *   (one of GL_COMPUTE_SHADER,
         *   GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, 
         *   GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER)
         * \param[in] source an ASCII string that contain 
         *   the source of the shader 
         * \param[in] provider a pointer to an object that implements the PseudoFileProvider
         *   interface (typically a GLUP Context) 
         * \return the OpenGL opaque Id of the created shader object
         * \throw GLSLCompileError
         */
        GLuint GEOGRAM_GFX_API compile_shader_with_includes(
            GLenum target, const char* source, PseudoFileProvider* provider
        );


        /**
         * \brief Compiles a program from shader sources.
         * \param[in] provider a pointer to an object that implements the PseudoFileProvider
         *   interface (typically a GLUP Context)
         * \param[in] shader1 , shader2 , shader3 , shader4 , shader5 , shader6 up to
         *  six shader sources definition. Each shader source definition should begin
         *  with //stage STAGE where STAGE is one of GL_VERTEX_SHADER, GL_FRAGMENT_SHADER,
         *  GL_GEOMETRY_SHADER, GL_TESSELLATION_SHADER, GL_TESS_EVALUATION_SHADER
         * \return the OpenGL opaque Id of the created program object
         * \throw GLSLCompileError
         */
        GLuint GEOGRAM_GFX_API compile_program_with_includes_no_link(
            PseudoFileProvider* provider,
            const char* shader1, const char* shader2 = nullptr, const char* shader3 = nullptr,
            const char* shader4 = nullptr, const char* shader5 = nullptr, const char* shader6 = nullptr
        );

        
        /************************************************************/        
        
        /**
         * \brief Compiles a shader for a specific target.
         * \details One can split the source of the shader into
         *  different strings, one of them being used for library
         *  functions common to different shaders.
         *  It may seem more natural to generate a shader object with library 
         *  functions, but OpenGL documentation does not recommend
         *  to do so (and it did not seem to work). Errors are detected and 
         *  displayed to std::err.
         * \param[in] target the OpenGL shader target 
         *  (one of GL_COMPUTE_SHADER,
         *   GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, 
         *   GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER)
         * \param[in] sources an array of pointer to ASCII strings 
         *   that contain the source of the shader 
         * \param[in] nb_sources number of strings in \p sources
         * \return the OpenGL opaque Id of the created shader object
         * \throw GLSLCompileError
         */
        GLuint GEOGRAM_GFX_API compile_shader(
            GLenum target, const char** sources, index_t nb_sources
        );

        /**
         * \brief Compiles a shader for a specific target.
         * \details One can split the source of the shader into
         *  different strings, one of them being used for library
         *  functions common to different shaders.
         *  It may seem more natural to generate a shader object with library 
         *  functions, but OpenGL documentation does not recommend
         *  to do so (and it did not seem to work). Errors are detected and 
         *  displayed to std::err.
         * \param[in] target the OpenGL shader target 
         *  (one of GL_COMPUTE_SHADER,
         *   GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, 
         *   GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER)
         * \param[in] source1 , source2 , ... ASCII strings that will be 
         *  concatened to form the source of the shader. It needs to be
         *  terminated by 0.
         * \return the OpenGL opaque Id of the created shader object
         * \throw GLSLCompileError
         * \note Could have been implemented using varargs, but I had
         *  problems with it (crashes that I could not fix), and it is
         *  not recommended anyway (does not have typechecking).
         */
        GLuint GEOGRAM_GFX_API compile_shader(
            GLenum target,
            const char* source1,
            const char* source2,
            const char* source3 = nullptr,
            const char* source4 = nullptr,
            const char* source5 = nullptr,
            const char* source6 = nullptr,
            const char* source7 = nullptr,
            const char* source8 = nullptr,
            const char* source9 = nullptr,
            const char* source10 = nullptr,
            const char* source11 = nullptr,
            const char* source12 = nullptr,
            const char* source13 = nullptr,
            const char* source14 = nullptr,
            const char* source15 = nullptr,
            const char* source16 = nullptr,
            const char* source17 = nullptr,
            const char* source18 = nullptr,
            const char* source19 = nullptr,
            const char* source20 = nullptr            
        );


        /**
         * \brief Links a program.
         * \details Errors are detexted and displayed to the Logger.
         * \param[in] program the program to be linked
         */
        void GEOGRAM_GFX_API link_program(GLuint program);
        
        /**
         * \brief Creates a GLSL program from a zero-terminated 
         *  list of shaders
         * \details Errors are detected and displayed to the Logger.
         * \note link_program() needs to be called after.
         *   If the program has vertex attributes, then 
         *   glBindAttribLocation() needs to be called after
         *   create_program_from_shaders_no_link() and before
         *   link_program().
         * \param[in] shader the first shader of the list
         * \return the OpenGL opaque Id of the created program
         */
        GLuint GEOGRAM_GFX_API create_program_from_shaders_no_link(
            GLuint shader, ...
        );

        /**
         * \brief Creates a GLSL program from a zero-terminated 
         *  list of shaders
         * \details Errors are detected and displayed to the Logger.
         * \note If the program has vertex attributes and needs 
         *  glBindAttribLocation(), then use 
         *  create_program_from_shaders_no_link() instead.
         * \param[in] shader the first shader of the list
         * \return the OpenGL opaque Id of the created program
         */
        GLuint GEOGRAM_GFX_API create_program_from_shaders(GLuint shader, ...);
        
        /**
         * \brief Creates a GLSL program from a string.
         * \details The string may contain several shaders. Each shader
         *   is delimited by begin-end statements: 
         *   #begin(SHADER_TYPE) / #end(SHADER_TYPE)
         *   where SHADER_TYPE is one of GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, 
         *   GL_GEOMETRY_SHADER, GL_TESS_CONTROL_SHADER, 
         *   GL_TESS_EVALUATION_SHADER.
         * \note link_program() needs to be called after.
         * \param[in,out] string the combined shaders that constitute the 
         *  program. 
         * \param[in] copy_string if true, the input string is copied 
         *   internally. The function temporarily modifies the input string 
         *   (and then restores it on exit). This may
         *   be forbidden when input string is a constant char array 
         *   (string litteral in source code). In this case, the input 
         *   string is copied to a temporary buffer.
         * \return the OpenGL opaque Id of the created shader object
         * \throw GLSLCompileError
         */
        GLuint GEOGRAM_GFX_API create_program_from_string_no_link(
            const char* string, bool copy_string = true
        );

        /**
         * \brief Creates a GLSL program from a file.
         * \details The file contains a list of shaders, delimited by
         *   begin-end statements (see setup_program_from_string()).
         * \note link_program() needs to be called after.
         * \param[in] filename the name of the file
         * \throw GLSLCompileError
         */
        GLuint GEOGRAM_GFX_API create_program_from_file_no_link(
            const std::string& filename
        );

        /**
         * \brief Sets a uniform variable in a shader by name.
         * \param[in] shader_id the handle to the GLSL shader
         * \param[in] name the name of the uniform variable,
         *   as specified in the GLSL source of the shader.
         * \param[in] val the value of the parameter
         * \tparam T the type of the parameter. Needs to match
         *  the type of the uniform parameter in the GLSL source.
         */
        template <class T> inline bool set_program_uniform_by_name(
            GLuint shader_id, const char* name, T val
        ) {
            geo_argused(shader_id);
            geo_argused(name);
            geo_argused(val);
            geo_assert_not_reached;
            return false;
        }

        template<> inline bool set_program_uniform_by_name(
            GLuint shader_id, const char* name, bool val
        ) {
            GLint location = glGetUniformLocation(shader_id, name) ;
            if(location < 0) {
                return false ;
            }
            glUseProgram(shader_id);
            glUniform1i(location, val ? 1 : 0) ;
            glUseProgram(0);
            return true;
        }

        template<> inline bool set_program_uniform_by_name(
            GLuint shader_id, const char* name, float val
        ) {
            GLint location = glGetUniformLocation(shader_id, name) ;
            if(location < 0) {
                return false ;
            }
            glUseProgram(shader_id);
            glUniform1f(location, val) ;
            glUseProgram(0);
            return true;
        }

        template<> inline bool set_program_uniform_by_name(
            GLuint shader_id, const char* name, double val
        ) {
            GLint location = glGetUniformLocation(shader_id, name) ;
            if(location < 0) {
                return false ;
            }
            glUseProgram(shader_id);
            glUniform1f(location, float(val)) ;
            glUseProgram(0);
            return true;
        }
        
        template<> inline bool set_program_uniform_by_name(
            GLuint shader_id, const char* name, int val
        ) {
            GLint location = glGetUniformLocation(shader_id, name) ;
            if(location < 0) {
                return false ;
            }
            glUseProgram(shader_id);
            glUniform1i(location, val) ;
            glUseProgram(0);
            return true;
        }


        template<> inline bool set_program_uniform_by_name(
            GLuint shader_id, const char* name, unsigned int val
        ) {
            GLint location = glGetUniformLocation(shader_id, name) ;
            if(location < 0) {
                return false ;
            }
            glUseProgram(shader_id);
#ifdef GEO_GL_150
            glUniform1ui(location, val) ;            
#else            
            glUniform1i(location, GLint(val)) ;            
#endif            
            glUseProgram(0);
            return true;
        }
        
        /**
         * \brief Sets an array of uniform variables in a shader by name.
         * \param[in] shader_id the handle to the GLSL shader
         * \param[in] name the name of the uniform variable,
         *   as specified in the GLSL source of the shader.
         * \param[in] count number of values
         * \param[in] values a pointer to an array of values of size count
         */
        inline bool set_program_uniform_by_name(
            GLuint shader_id, const char* name, index_t count, float* values
        ) {
            GLint location = glGetUniformLocation(shader_id, name) ;
            if(location < 0) {
                return false ;
            }
            glUseProgram(shader_id);
            glUniform1fv(location, GLsizei(count), values) ;
            glUseProgram(0);
            return true;
        }

        inline bool set_program_uniform_by_name(
            GLuint shader_id, const char* name, float x, float y
        ) {
            GLint location = glGetUniformLocation(shader_id, name) ;
            if(location < 0) {
                return false ;
            }
            glUseProgram(shader_id);
            glUniform2f(location, x, y);
            glUseProgram(0);
            return true;
        }
	
        /**
         * \brief Gets the offset of a uniform variable relative
         *  to the uniform block it is declared in.
         * \param[in] program a GLSL program handle
         * \param[in] varname the name of the variable
         * \return the offset of the variable relative to the beginning
         *  of the uniform block it is declared in, in bytes.
         */
        GLint GEOGRAM_GFX_API get_uniform_variable_offset(
            GLuint program, const char* varname
        );

	/**
	 * \brief Queries array stride for a variable in a 
	 *   GLSL program using introspection.
	 * \param[in] program the handle of the program
	 * \param[in] varname a string with the name of the array variable
	 * \return the number of bytes between two consecutive elements of the
	 *  array.
	 */
	size_t GEOGRAM_GFX_API get_uniform_variable_array_stride(
            GLuint program, const char* varname
	);

        /**
         * \brief Outputs to the logger everything that can 
         *  be queried about a program using OpenGL 
         *  introspection APIs.
         */
        void GEOGRAM_GFX_API introspect_program(GLuint program);
        
    }
}

#endif
