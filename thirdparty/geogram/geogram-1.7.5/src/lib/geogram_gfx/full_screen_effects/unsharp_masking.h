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
 
#ifndef H_OGF_RENDERER_GL_CONTEXT_UNSHARP_MASKING_GLSL_H
#define H_OGF_RENDERER_GL_CONTEXT_UNSHARP_MASKING_GLSL_H

#include <geogram_gfx/basic/common.h>
#include <geogram_gfx/full_screen_effects/full_screen_effect.h>
#include <geogram_gfx/basic/frame_buffer_object.h>

/**
 * \file geogram_gfx/full_screen_effects/unsharp_masking.h
 * \brief Implementation of UnsharpMasking full screen effect.
 */

namespace GEO {

    /**
     * \brief Implementation of UnsharpMasking full screen effect.
     * \details Reference: Image Enhancement by Unsharp Masking the 
     *  Depth Buffer, by Luft, Colditz and Deussen, ACM SIGGRAPH conference
     *  proceedings 2006.
     */
    class GEOGRAM_GFX_API UnsharpMaskingImpl : public FullScreenEffectImpl {
    public:
        /**
         * \brief UnsharpMaskingImpl constructor.
         */
        UnsharpMaskingImpl();

        /**
         * \brief UnsharpMaskingImpl destructor.
         */
        virtual ~UnsharpMaskingImpl();

        /**
         * \copydoc FullScreenEffectImpl::required_GLSL_version()
         */
        virtual double required_GLSL_version() const;
        
        /**
         * \copydoc FullScreenEffectImpl::pre_render()
         */
        virtual void pre_render(index_t w, index_t h);

        /**
         * \copydoc FullScreenEffectImpl::post_render()
         */
        virtual void post_render();
        
        /**
         * \copydoc FullScreenEffectImpl::update()
         */
        virtual void update();

        /**
         * \brief Gets the intensity.
         * \return the intensity, as an integer.
         *    A value of 50 (default) corresponds to average intensity.
         */
        index_t get_intensity() const {
            return intensity_;
        }

        /**
         * \brief Sets the intensity.
         * \param[in] x the intensity, as an integer.
         *    A value of 50 (default) corresponds to average intensity.
         */
        void set_intensity(index_t x) {
            intensity_ = x;
        }

        /**
         * \brief Gets the contrast.
         * \return the contrast, as an integer.
         *    A value of 50 (default) corresponds to average contrast.
         */
        index_t get_contrast() const {
            return contrast_;
        }

        /**
         * \brief Sets the contrast.
         *    A value of 50 (default) corresponds to average contrast.
         */
        void set_contrast(index_t x) {
            contrast_ = x;
        }

        /**
         * \brief Gets the size of the blurring kernel.
         * \return the size of the blurring kernel, in pixels.
         */
        index_t get_blur_width() const {
            return blur_width_;
        }

        /**
         * \brief Sets the size of the blurring kernel.
         * \param[in] x the size of the blurring kernel, in pixels.
         */
        void set_blur_width(index_t x) {
            blur_width_ = x;
        }

        /**
         * \brief Tests whether halos should be drawn.
         * \retval true if halos should be drawn
         * \retval false otherwise
         */
        bool get_halos() const {
            return halos_;
        }

        /**
         * \brief Specifies whether halos should be used.
         * \param[in] x true if halos should be drawn, false otherwise
         */
        void set_halos(bool x) {
            halos_ = x;
        }

        /**
         * \brief Tests whether positive_shadows should be drawn.
         * \details Positive shadows are white shadows that appear on zones
         *  opposite to dark shadows. It further enhances the perception of
         *  the shape.
         * \retval true if positive_shadows should be drawn
         * \retval false otherwise
         */
        bool get_positive_shadows() const {
            return positive_shadows_;
        }

        /**
         * \brief Specifies whether positive_shadows should be used.
         * \details Positive shadows are white shadows that appear on zones
         *  opposite to dark shadows. It further enhances the perception of
         *  the shape.
         * \param[in] x true if positive_shadows should be drawn, 
         *  false otherwise
         */
        void set_positive_shadows(bool x) {
            positive_shadows_ = x;
        }

    protected:
        /**
         * \copydoc FullScreenEffectImpl::initialize()
         */
        virtual void initialize(index_t w, index_t h);

        /**
         * \copydoc FullScreenEffectImpl::resize()
         */
        virtual void resize(index_t width, index_t height);

        /**
         * \brief Applies a Gaussian blur to the depth buffer.
         * \details The input is taken from the depth buffer of
	 *  draw_FBO_. The result is in blur_1_. 
         *  The function uses blur_2_ as a work variable. It does two passes 
         *  of 1D blurring (horizontal and vertical, one from the depth buffer 
	 *  to blur_2_ and the other from blur_2_ to blur_1_.
         */
        void blur();

        /**
         * \brief Displays the final result.
         * \details It composites the final result with the
         *   previously rendered frame.
         */
        void display_final_texture();

    private:
        index_t intensity_;
        index_t contrast_;
        index_t blur_width_;
        bool halos_;
        bool positive_shadows_;
        
        FrameBufferObject blur_1_;
        FrameBufferObject blur_2_;
        
        GLuint unsharp_masking_program_;
        GLuint blur_program_;
    } ;

    /**
     * \brief An automatic reference-counted pointer to an UnsharpMaskingImpl.
     */
    typedef SmartPointer<UnsharpMaskingImpl> UnsharpMaskingImpl_var ;
    
}

#endif
