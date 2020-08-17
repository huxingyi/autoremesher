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

#ifndef GEOGRAM_GFX_BASIC_GL
#define GEOGRAM_GFX_BASIC_GL

#ifdef GEO_DEBUG
#define GEO_DEBUG_GL
#endif

#include <geogram_gfx/basic/common.h>
#include <geogram_gfx/api/defs.h>


#if defined(GEO_OS_EMSCRIPTEN)
#  define GLFW_INCLUDE_ES2 
#  include <GLFW/glfw3.h>
#  define GL_GLEXT_PROTOTYPES
#  include <GLES2/gl2ext.h>
#  define GL_INVALID_INDEX GLuint(-1)
   typedef double GLdouble;
#  define glGenVertexArrays glGenVertexArraysOES
#  define glBindVertexArray glBindVertexArrayOES
#  define glDeleteVertexArrays glDeleteVertexArraysOES
#  define GEO_GL_ES2
#  define GEO_GL_NO_DOUBLES
#elif defined(GEO_OS_ANDROID)
#  include <GLES3/gl3.h>
#  include <GLES3/gl31.h>
#  include <GLES3/gl32.h>
#  define GEO_GL_TEXTURE_3D
#  define GEO_GL_ES2
#  define GEO_GL_150
#  define GEO_GL_NO_DOUBLES
#else
#  include <geogram_gfx/third_party/glad/glad.h>
#  define GEO_GL_TEXTURE_3D
#  define GEO_GL_150
#  define GEO_GL_440
#  define GEO_GL_ES2
#endif

#include <geogram_gfx/GLUP/GLUP.h>
#include <geogram/basic/geometry.h>

// Some defines missing in Emscripten GL headers
#if defined(GEO_OS_EMSCRIPTEN) || defined(GEO_OS_ANDROID)
#   ifndef GL_RGB8
#   define GL_RGB8 0x8051
#   endif

#   ifndef GL_RGBA8
#   define GL_RGBA8 0x8058
#   endif

#   ifndef GL_R16F
#   define GL_R16F 0x822D
#   endif

#   ifndef GL_R32F
#   define GL_R32F 0x822E
#   endif

#   ifndef GL_RED
#   define GL_RED 0x1903
#   endif

#   ifndef GL_R8
#   define GL_R8 0x8229
#   endif

#   ifndef GL_R16
#   define GL_R16 0x822A
#   endif

#   ifndef GL_DEPTH_COMPONENT24
#   define GL_DEPTH_COMPONENT24 0x81A6
#   endif

#endif

/**
 * \file geogram_gfx/basic/GL.h
 * \brief Some utility functions for OpenGL graphics.
 */

namespace GEO {

    namespace GL {
        /**
         * \brief Initializes some GL functions and objects.
         * \details Called by GEO::Graphics::initialize()
         */  
        void GEOGRAM_GFX_API initialize();

        /**
         * \brief Terminates GL functions and objects.
         * \details Called by GEO::Graphics::terminate()
         */  
        void GEOGRAM_GFX_API terminate();
    }


    /**
     * \brief Sends a vertex to OpenGL.
     * \param[in] v a const reference to the vertex to be sent.
     */
    inline void glupVertex(const vec2& v) {
        glupVertex2dv(v.data());
    }
    
    /**
     * \brief Sends a vertex to OpenGL.
     * \param[in] v a const reference to the vertex to be sent.
     */
    inline void glupVertex(const vec3& v) {
        glupVertex3dv(v.data());
    }

    /**
     * \brief Sends a vertex to OpenGL.
     * \param[in] v a const reference to the vertex to be sent, in
     *  homogeneous coordinates (4d).
     */
    inline void glupVertex(const vec4& v) {
        glupVertex4dv(v.data());
    }

    /**
     * \brief Sends a RGB color to OpenGL.
     * \param[in] v a const reference to the color to be sent.
     */
    inline void glupColor(const vec3& v) {
        glupColor3dv(v.data());
    }

    /**
     * \brief Sends a RGBA color to OpenGL.
     * \param[in] v a const reference to the color to be sent.
     */
    inline void glupColor(const vec4& v) {
        glupColor4dv(v.data());
    }


    /**
     * \brief Sends 2d texture coordinates to OpenGL.
     * \param[in] v a const reference to the texture coordinates to be sent.
     */
    inline void glupTexCoord(const vec2& v) {
        glupTexCoord2dv(v.data());
    }

    /**
     * \brief Sends 3d texture coordinates to OpenGL.
     * \param[in] v a const reference to the texture coordinates to be sent.
     */
    inline void glupTexCoord(const vec3& v) {
        glupTexCoord3dv(v.data());
    }

    /**
     * \brief Sends 4d texture coordinates to OpenGL.
     * \param[in] v a const reference to the texture coordinates to be sent.
     */
    inline void glupTexCoord(const vec4& v) {
        glupTexCoord4dv(v.data());
    }

    /**
     * \brief Applies a translation.
     * \param[in] v the translation vector.
     */
    inline void glupTranslate(const vec3& v) {
	glupTranslated(v.x, v.y, v.z);
    }

    /**
     * \brief Maps texture coordinates from a specified interval to
     *   the unit interval.
     * \details This changes the GLUP texture matrix. GLUP matrix mode
     *   is reset to GLUP_MODELVIEW_MATRIX on exit.
     * \param[in] minval minimum value, to be mapped to 0
     * \param[in] maxval maximum value, to be mapped to 1
     * \param[in] mult multiplicator, applied after the mapping
     */
    void GEOGRAM_GFX_API glupMapTexCoords1d(
        double minval, double maxval, index_t mult=1
    );
    
    /**
     * \brief Multiplies the current GLUP matrix
     *   with another one.
     * \param[in] m a const reference to the matrix.
     * \note m is transposed before being sent to GLUP
     *  because Geogram uses the convention with column
     *  vectors and GLUP the convention with row vectors
     *  to represent the transformed points.
     */
    void GEOGRAM_GFX_API glupMultMatrix(const mat4& m);

    /**
     * \brief Replaces the current GLUP matrix
     *   with a user defined one.
     * \param[in] m a const reference to the matrix.
     * \note m is transposed before being sent to OpenGL
     *  because Geogram uses the convention with column
     *  vectors and GLUP the convention with row vectors
     *  to represent the transformed points.
     */
    void GEOGRAM_GFX_API glupLoadMatrix(const mat4& m);    

    /**
     * \brief Gets the size (in bytes) of the OpenGL buffer 
     *  bound to a specified target.
     * \param[in] target buffer object target 
     *   (GL_ARRAY_BUFFER, GL_INDEX_BUFFER ...)
     * \return the size in bytes of the buffer object bound 
     *  to \p target.
     */
    GLint64 GEOGRAM_GFX_API get_size_of_bound_buffer_object(GLenum target);

    /**
     * \brief Updates the content of an OpenGL buffer object, 
     *   and resizes it if need be.
     * \param[in,out] buffer_id OpenGL opaque id of the buffer object. 
     *   0 means uninitialized.
     *   may be changed on exit if the buffer needed to be created or
     *   destroyed.
     * \param[in] target buffer object target 
     *   (GL_ARRAY_BUFFER, GL_INDEX_BUFFER ...)
     * \param[in] new_size of the buffer data, in bytes
     * \param[in] data pointer to the data to be copied into the buffer, 
     *  of length new_size
     */
    void GEOGRAM_GFX_API update_buffer_object(
        GLuint& buffer_id, GLenum target, size_t new_size, const void* data
    );

    /**
     * \brief Updates the content of an OpenGL buffer object in streaming
     *   mode.
     * \details Streaming mode means that there will be many updates of
     *   the contents of the same buffer object. stream_buffer_object()
     *   does the same thing as update_buffer_object(), but may
     *   be faster than update_buffer_object() in this situation.
     * \param[in,out] buffer_id OpenGL opaque id of the buffer object. 
     *   0 means uninitialized.
     *   may be changed on exit if the buffer needed to be created or
     *   destroyed.
     * \param[in] target buffer object target 
     *   (GL_ARRAY_BUFFER, GL_INDEX_BUFFER ...)
     * \param[in] new_size of the buffer data, in bytes
     * \param[in] data pointer to the data to be copied into the buffer, 
     *  of length new_size
     */
    void GEOGRAM_GFX_API stream_buffer_object(
        GLuint& buffer_id, GLenum target, size_t new_size, const void* data
    );
    

    /**
     * \brief Updates the content of an OpenGL buffer object, 
     *   and resizes it if need be, or tests whether it has the
     *   size it should have.
     * \param[in,out] buffer_id OpenGL opaque id of the buffer object. 
     *   0 means uninitialized.
     *   may be changed on exit if the buffer needed to be created or
     *   destroyed.
     * \param[in] target buffer object target 
     *   (GL_ARRAY_BUFFER, GL_INDEX_BUFFER ...)
     * \param[in] new_size of the buffer data, in bytes
     * \param[in] data pointer to the data to be copied into the buffer, 
     *  of length new_size
     * \param[in] update 
     *  - if true, the buffer will be updated, and resized if need be. 
     *  - if false, the size of the buffer will be tested, and an error 
     *    message will be displayed in the logger if it does not match
     *    the specified size (and update will be forced).
     */
    void GEOGRAM_GFX_API update_or_check_buffer_object(
        GLuint& buffer_id, GLenum target, size_t new_size, const void* data,
        bool update
    );
    
    /**
     * \brief Tests for OpenGL errors and displays a message if
     *  OpenGL errors were encountered.
     * \param[in] file current sourcefile, as given by __FILE__
     * \param[in] line current line, as given by __LINE__
     * \param[in] warning_only if true, then errors are reported as warnings.
     */
    void GEOGRAM_GFX_API check_gl(
	const char* file, int line, bool warning_only=false
    );

    /**
     * \brief Clears all error flags set by previous OpenGL calls.
     * \details This function shoud be called to ensure that subsequent 
     *  calls to check_gl() will not report any error. This is necessary
     *  to workaround some buggy or incomplete implementations of OpenGL.
     *  In debug mode, error are always reported.
     * \param[in] file current sourcefile, as given by __FILE__
     * \param[in] line current line, as given by __LINE__
     */
    void GEOGRAM_GFX_API clear_gl_error_flags(const char* file, int line);
    
    /**
     * \brief Draws a textured quad.
     * \param[in] BW if set, copy the red channel to the output red, green
     *  blue channels (and set alpha to 1.0).
     * \details The textured quad spans the [-1,1]x[-1,1] square with
     *  texture coordinates in [0,1]x[0,1]. If no program is currently
     *  bound, then a default one is used, and it uses the texture bound
     *  to unit 0 of GL_TEXTURE_2D. If a program is bound, then it is used.
     *  Vertices coordinates are sent to vertex attribute 0 and texture 
     *  coordinates to vertex attribute 1.
     */
    void GEOGRAM_GFX_API draw_unit_textured_quad(bool BW=false);
    
    /**
     * \brief Tests for OpenGL errors. 
     * \details If an OpenGL error was flagged, display it together
     *  with current file and line number.
     */
#ifdef GEO_DEBUG_GL    
#   define GEO_CHECK_GL() ::GEO::check_gl(__FILE__,__LINE__)
#else
#   define GEO_CHECK_GL()
#endif    

    /***********************************************************/

    void GEOGRAM_GFX_API glTexImage2Dxpm(char const* const* xpm_data);
    
}

#endif

