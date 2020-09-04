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

#ifndef H_OGF_RENDERER_CONTEXT_FULL_SCREEN_EFFECT_H
#define H_OGF_RENDERER_CONTEXT_FULL_SCREEN_EFFECT_H

#include <geogram_gfx/basic/common.h>
#include <geogram_gfx/basic/GLSL.h>
#include <geogram_gfx/basic/frame_buffer_object.h>
#include <geogram/basic/counted.h>

/**
 * \file geogram_gfx/full_screen_effects/full_screen_effect.h
 * \brief Low-level base class for full screen effects.
 */

namespace GEO {

    /**
     * \brief Implementation of full screen effects.
     * \details This is the low-level class for full screen
     *  effects, that communicates with the RenderingContext
     *  and with OpenGL. In Graphite, typically a full screen 
     *  effect is implemented as a pair of FullScreenEffectImpl / 
     *  FullScreenEffect. 
     */
    class GEOGRAM_GFX_API FullScreenEffectImpl :
	public Counted, public GLSL::PseudoFileProvider {
    public:
        /**
         * \brief FullScreenEffectImpl constructor.
         */
        FullScreenEffectImpl();

	/**
	 * \brief FullScreenEffectImpl destructor.
	 */
	~FullScreenEffectImpl() override;
	

        /**
         * \brief Gets the minimum required GLSL version needed
         *  to execute the shaders in this FullScreenEffectImpl.
         * \details Default implementation returns 1.0.
         * \return the minimum required GLSL version as a double
         *  precision floating point number.
         */
        virtual double required_GLSL_version() const;
        
        /**
         * \brief Callback called at the beginning of each frame.
	 * \param[in] width , height dimension of the rendering context.
         * \details Baseclass implementation redirects rendering to
	 *   draw_FBO_. 
         */
        virtual void pre_render(index_t width, index_t height);

        /**
         * \brief Callback called at the end of each frame.
         * \details Subclasses may overload this function, and
         *   use it to transfered the content of FrameBufferObjects
         *   to the screen. Baseclass implementation copies the contents
	 *   of draw_FBO_ to the screen.
         */
        virtual void post_render();

        /**
         * \brief Callback called whenever parameters are changed
         *  in the GUI.
         * \details Subclasses may overload this function, and use
         *  it to transfer parameters to shader uniforms.
         */
        virtual void update();

        /**
         * \brief Gets the width of the rendering context.
         * \details We cache the size of the rendering context,
         *  it is used for instance to create FrameBufferObjects.
         * \return The width of the rendering context, in pixels.
         */
        index_t width() const {
            return width_;
        }

        /**
         * \brief Gets the height of the rendering context.
         * \details We cache the size of the rendering context,
         *  it is used for instance to create FrameBufferObjects.
         * \return The height of the rendering context, in pixels.
         */
        index_t height() const {
            return height_;
        }

        /**
         * \brief Tests whether this FullScreenEffect can be used.
         * \details Most full screen effects require some hardware support,
         *  this function tests whether the GPU has sufficient functionalities
         *  for implementing this FullScreenEffect.
         * \retval true if it can be used
         * \retval false otherwise
         */
        bool OK() const {
            return OK_;
        }
        
        /**
         * \brief Gets the content of the virtual file
         *  GLUP/current_profile/vertex_shader_preamble.h.
         * \param[in,out] sources where the content of the 
         *  virtual file should be appended
         */
        virtual void get_vertex_shader_preamble_pseudo_file(
            std::vector<GLSL::Source>& sources
        );

        /**
         * \brief Gets the content of the virtual file
         *  GLUP/current_profile/fragment_shader_preamble.h
         * \param[in,out] sources where the content of the 
         *  virtual file should be appended
         */
        virtual void get_fragment_shader_preamble_pseudo_file(
            std::vector<GLSL::Source>& sources
        );

      protected:
	
        /**
         * \brief Callback called the first time this FullScreenEffectImpl
         *  is used.
	 * \param[in] w , h width and height of the rendering context.
         * \details Subclasses may overload this callback. The OpenGL context
         *  is properly bound when this function is called.
         */
        virtual void initialize(index_t w, index_t h);

        /**
         * \brief Callback called whenever the rendering context is resized.
         * \details Subclasses may overload this callback, for instance to
         *   resize FrameBufferObject.
         * \param[in] w , h new width and height of the rendering context, in
         *   pixels.
         */
        virtual void resize(index_t w, index_t h);
        
	/**
	 * \brief Resets alpha plane to 1.0 (opaque)
	 */
	void reset_alpha();
	
     private:
        bool initialized_;
        bool OK_;
        index_t width_;
        index_t height_;

     protected:
	FrameBufferObject draw_FBO_;
	bool ES_profile_;
    };

    /**
     * \brief An automatic reference-counted pointer to a FullScreenEffectImpl.
     */
    typedef SmartPointer<FullScreenEffectImpl> FullScreenEffectImpl_var;
    
}

#endif
