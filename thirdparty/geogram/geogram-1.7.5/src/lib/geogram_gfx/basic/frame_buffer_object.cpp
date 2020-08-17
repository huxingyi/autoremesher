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

#include <geogram_gfx/basic/frame_buffer_object.h>
#include <geogram_gfx/basic/GLSL.h>
#include <geogram/basic/logger.h>
#include <string>

#ifdef GEO_OS_EMSCRIPTEN
#include <emscripten.h>
#endif

namespace {
    using namespace GEO;

    /**
     * \brief Allocates the currently bound GL_TEXTURE_2D object.
     * \param[in] width , height size of the texture.
     * \param[in] internalformat internal format for the texture.
     * \details Under WebGL2, if a generic internal format is specified,
     *  then a specific one is chosen (generic internal formats are
     *  not supported under WebGL2).
     */
    void allocate_texture_2D(
	index_t width, index_t height, GLint internalformat
    ) {

      // - glTexImage2d in WebGL2 is very picky on what combination of
      //   internalformat/format/type is accepted, and it needs a full
      //   specification of internalformat (for instance, GL_RGBA8 instead
      //   of GL_RGBA)
      //   See
      //   https://webgl2fundamentals.org/webgl/lessons/webgl-data-textures.html
      //   for the valid combinations
      //   OpenGL ES 3.0: see:
      //   https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glTexImage2D.xhtml
      //	
      // - not all internalformat defines are available in Emscripten, 
      //   added the ones that I need in geogram_gfx/basic/GL.h
      //
      // TODO: initialize the WebGL extension "floating point render target"
      // if needed

	GLenum format=0;
	GLenum type=0;
	
	switch(internalformat) {
	    case GL_RGBA:
		format = GL_RGBA;
#if defined(GEO_OS_EMSCRIPTEN) && !defined(GEO_WEBGL2)
		type = GL_FLOAT;		
#else
		type = GL_UNSIGNED_BYTE;
#endif		
		break;
	    case GL_RGB:
		format = GL_RGB;
		type = GL_UNSIGNED_BYTE;
		break;
	    case GL_RED:
		format = GL_RED;
		type = GL_UNSIGNED_BYTE;
		break;
	    case GL_R16:
		format = GL_RED;
		type = GL_SHORT;
		break;
	    case GL_R32F:
		format = GL_RED;
		type = GL_FLOAT;
		break;
	    case GL_R16F:
		format = GL_RED;
		type = GL_FLOAT;
		break;
	    case GL_DEPTH_COMPONENT:
		format = GL_DEPTH_COMPONENT;
		type = GL_UNSIGNED_INT;
		break;
	}

#if (defined(GEO_OS_EMSCRIPTEN) && defined(GEO_WEBGL2)) || defined(GEO_OS_ANDROID)
	// Replace generic internal format with specific
	// one, as required by WebGL2 and ES3
	if(internalformat == GL_RGBA) {
	    internalformat = GL_RGBA8;
	} else if(internalformat == GL_RGB) {
	    internalformat = GL_RGB8;
	} else if(internalformat == GL_DEPTH_COMPONENT) {
	    internalformat = GL_DEPTH_COMPONENT24;
	} else if(internalformat == GL_RED) {
	    internalformat = GL_R8;
	}
#endif

	// If the following assertions fail, then
	// this means that the internalformat that
	// was given is not taken into account (just
	// add it in the previous switch() and cascading
	// ifs).
	geo_assert(format != 0);
	geo_assert(type != 0);

	glTexImage2D(
	    GL_TEXTURE_2D, 0,
	    internalformat,
	    GLsizei(width), GLsizei(height), 0,
	    format,
	    type,
	    nullptr
	);
    }
}

namespace GEO {

    FrameBufferObject::FrameBufferObject() : 
        frame_buffer_id(0),
        depth_buffer_id(0),
        offscreen_id(0),
        width(0),
        height(0),
        previous_frame_buffer_id(0)
    {
    }

    FrameBufferObject::~FrameBufferObject() {
        if(offscreen_id != 0) {
            glDeleteTextures(1, &offscreen_id);
            offscreen_id = 0;
        }
        if(depth_buffer_id != 0) {
            glDeleteTextures(1, &depth_buffer_id);
            depth_buffer_id = 0;
        }
        if(frame_buffer_id != 0) {
            glBindFramebuffer(GL_FRAMEBUFFER, previous_frame_buffer_id);
            glDeleteFramebuffers(1, &frame_buffer_id);
        }
    }

    void FrameBufferObject::resize(index_t new_width, index_t new_height) {
        if(width != new_width || height != new_height) {
            width  = new_width; 
            height = new_height;

	    GEO_CHECK_GL();
            glBindTexture(GL_TEXTURE_2D, offscreen_id);
	    allocate_texture_2D(width, height, internal_storage);
	    GEO_CHECK_GL();	    
            if(depth_buffer_id != 0) {
                glBindTexture(GL_TEXTURE_2D, depth_buffer_id);
		GEO_CHECK_GL();
		allocate_texture_2D(width, height, GL_DEPTH_COMPONENT);
		GEO_CHECK_GL();				
            }
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    bool FrameBufferObject::initialize(
        index_t width_in, index_t height_in, 
        bool with_depth_buffer, GLint internal_storage_in,
        bool mipmaps
    ) {

	GEO_CHECK_GL();
	
        //  Get the id of the default frame buffer used by the
        // context. It will be 0 if it is a regular OpenGL context,
        // or it can be a bound frame buffer object since Qt5.4
        // QOpenGLWidget uses a frame buffer to implement light weight
        // OpenGL contexts. Note that it may change when the Qt rendering
	// context is resized (see bind_as_framebuffer()).
	glGetIntegerv(
	    GL_FRAMEBUFFER_BINDING, (GLint*)(&previous_frame_buffer_id)
	);
	
        width = width_in;
        height = height_in;
        internal_storage = internal_storage_in;

        // Generate frame buffer object then bind it.
        glGenFramebuffers(1, &frame_buffer_id);
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_id);

	GEO_CHECK_GL();
	
        if(with_depth_buffer) {
            glGenTextures(1, &depth_buffer_id);
        }

	GEO_CHECK_GL();
	
        // Create the texture we will be using to render to.
        glGenTextures(1, &offscreen_id);
        glBindTexture(GL_TEXTURE_2D, offscreen_id);

	GEO_CHECK_GL();

	if(!mipmaps) {
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	GEO_CHECK_GL();
	
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GEO_CHECK_GL();
	allocate_texture_2D(width, height, internal_storage);
	GEO_CHECK_GL();
	
        // Bind the texture to the frame buffer.
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, offscreen_id, 0
        );

	GEO_CHECK_GL();
	
        // Initialize the depth buffer.
        if(with_depth_buffer) {

            glBindTexture(GL_TEXTURE_2D, depth_buffer_id);
	    allocate_texture_2D(width, height, GL_DEPTH_COMPONENT);
	    GEO_CHECK_GL();
	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	    GEO_CHECK_GL();
	
            glFramebufferTexture2D(
                GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,		
                GL_TEXTURE_2D, depth_buffer_id, 0
            );

	    GEO_CHECK_GL();	    
        }

	GEO_CHECK_GL();

//      Heard it said that DrawBuffer and ReadBuffer
//      should be set to NONE before checking frame buffer
//      status, but it seems to be unnecessary (commented-out
//      for now)
//	
//	glDrawBuffer(GL_NONE);
//	glReadBuffer(GL_NONE);
	
        // Make sure we have not errors.
        if(
            glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
            GL_FRAMEBUFFER_COMPLETE
        ) {
//	    glDrawBuffer(GL_BACK);
//	    glReadBuffer(GL_BACK);
            return false;
        }

	GEO_CHECK_GL();
	
//	glDrawBuffer(GL_BACK);
//	glReadBuffer(GL_BACK);
	
	GEO_CHECK_GL();
	
        // Restore previously bound frame buffer object.
        glBindFramebuffer(GL_FRAMEBUFFER, previous_frame_buffer_id);

	GEO_CHECK_GL();
	
        return true;
    }

    void FrameBufferObject::bind_as_texture() {
        glBindTexture(GL_TEXTURE_2D, offscreen_id);
    }

    void FrameBufferObject::bind_depth_buffer_as_texture() {
        glBindTexture(GL_TEXTURE_2D, depth_buffer_id);
    }

    void FrameBufferObject::bind_as_framebuffer() {
	// Current frame buffer ID may have changed,
	// for instance under Qt if rendering area was resized.
	glGetIntegerv(
	    GL_FRAMEBUFFER_BINDING, (GLint*)(&previous_frame_buffer_id)
	);
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_id);
    }

    void FrameBufferObject::unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, previous_frame_buffer_id);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    bool FrameBufferObject::is_bound_as_framebuffer() const {
	GLuint current_frame_buffer_id;
	glGetIntegerv(
	    GL_FRAMEBUFFER_BINDING, (GLint*)(&current_frame_buffer_id)
	);
	return (current_frame_buffer_id == frame_buffer_id);
    }
}

