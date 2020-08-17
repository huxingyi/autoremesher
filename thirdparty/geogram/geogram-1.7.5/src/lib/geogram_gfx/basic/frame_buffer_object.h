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

#ifndef H_GEOGRAM_GFX_BASIC_FBO_H
#define H_GEOGRAM_GFX_BASIC_FBO_H

#include <geogram_gfx/basic/common.h>

/**
 * \file geogram_gfx/basic/frame_buffer_object.h
 * \brief Helper class for manipulating OpenGL frame buffer objects.
 */

namespace GEO {

    /** 
     * \brief An OpenGL frame buffer object.
     */
    class GEOGRAM_GFX_API FrameBufferObject {
    public:
        /**
         * \brief FrameBufferObject constructor.
         * \details Creates an uninitialized FrameBufferObject.
         */
        FrameBufferObject();

        /**
         * \brief FrameBufferObject destructor.
         * \details Releases all the allocated OpenGL resources.
         */
        ~FrameBufferObject();

        /**
         * \brief Initializes the FrameBufferObject.
         * \param[in] width_in the width (in pixels)
         * \param[in] height_in the height (in picels)
         * \param[in] with_depth_buffer if true, a depth buffer is also created
         * \param[in] internal_storage the OpenGL internal storage
         * \param[in] mipmaps if true, the created textures have mipmaps
         */
        bool initialize(
            index_t width_in,
	    index_t height_in, 
            bool with_depth_buffer,
            GLint internal_storage,
            bool mipmaps = false
        );
	
        /**
         * \brief Resizes the FrameBuferObject
         * \param[in] new_width the new width, in pixels
         * \param[in] new_height the new height, in pixels
         */
        void resize(index_t new_width, index_t new_height);

        /**
         * \brief Binds this frame buffer as the input 2D texture.
         */
        void bind_as_texture();

        /**
         * \brief Binds the depth buffer of this frame buffer as the 
         *  input 2D texture.
         * \pre initialize() was called with with_depth_buffer=true
         */
        void bind_depth_buffer_as_texture();

        /**
         * \brief Binds this framebuffer as the output of OpenGL rendering.
	 * \details This memorizes the currently bound framebuffer.
         */
        void bind_as_framebuffer();


	/**
	 * \brief Tests whether this framebuffer is bound as a framebuffer.
	 * \retval true if this framebuffer is bound, i.e. used for OpenGL
	 *   output.
	 * \retval false otherwise
	 */
	bool is_bound_as_framebuffer() const;
	
        /**
         * \brief Unbind this framebuffer.
         * \details This removes all the bindings (both as texture and 
         *  as target of OpenGL rendering). If the framebuffer was bound
         *  as target of OpenGL rendering, this also restores the previously
         *  bound framebuffer.
         */
        void unbind();


	/**
	 * \brief Tests whether this FrameBufferObject is initialized.
	 * \retval true if this FrameBufferObject is initialized.
	 * \retval false otherwise.
	 */
	bool initialized() {
	    return (frame_buffer_id != 0);
	}
	
        /**
         * \brief The id of the frame buffer.
         */
        GLuint frame_buffer_id;

        /**
         * \brief The id of the texture used for the depth buffer.
         */
        GLuint depth_buffer_id;
	
        /**
         * \brief The id of the texture used for the color buffer.
         */
        GLuint offscreen_id;

        /**
         * \brief The width of this frame buffer, in pixels.
         */
        index_t width;

        /**
         * \brief The height of this frame buffer, in pixels.
         */
        index_t height;

        /**
         * \brief The OpenGL internal storage for the color buffer.
         */
        GLint internal_storage;

        /**
         * \brief The default frame buffer object associated with
         *  the Opengl context.
         */
        GLuint previous_frame_buffer_id;
    };

}

#endif
