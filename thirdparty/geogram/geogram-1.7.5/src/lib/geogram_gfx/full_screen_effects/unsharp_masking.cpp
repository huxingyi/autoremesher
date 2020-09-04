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


#include <geogram_gfx/full_screen_effects/unsharp_masking.h>
#include <geogram_gfx/basic/GLSL.h>
#include <geogram/basic/logger.h>
#include <geogram/bibliography/bibliography.h>

// Requirements for Android compile are the same
// as for Emscripten.
// TODO: something cleaner.
#ifdef GEO_OS_ANDROID
#define GEO_OS_EMSCRIPTEN
#endif

namespace GEO {

    UnsharpMaskingImpl::UnsharpMaskingImpl() {
        unsharp_masking_program_ = 0;
        blur_program_ = 0;
        positive_shadows_ = true;
        contrast_ = 50;
        intensity_ = 50;
        halos_ = true;
        blur_width_ = 1;
	geo_cite("DBLP:journals/tog/LuftCD06");
    }
    
    UnsharpMaskingImpl::~UnsharpMaskingImpl() {
        if (unsharp_masking_program_ != 0) { 
            glDeleteProgram(unsharp_masking_program_);       
        }
        if (blur_program_ != 0) { 
            glDeleteProgram(blur_program_);       
        }
    } 

    double UnsharpMaskingImpl::required_GLSL_version() const {
#ifdef GEO_OS_EMSCRIPTEN
	return 1.0;
#else	
        return 1.3;
#endif	
    }
    
    void UnsharpMaskingImpl::initialize(index_t w, index_t h) {
        FullScreenEffectImpl::initialize(w,h);
        if(!OK()) {
            return;
        }

#if defined(GEO_OS_EMSCRIPTEN) || defined(GEO_OS_ANDROID)
	// GL_R16F gives a different result (stripes) on Emscripten
	// Note: does not work on Android for now.
#  if defined(GEO_OS_EMSCRIPTEN) && !defined(GEO_WEBGL2)
	const GLint internal_format = GL_RGBA;	
#  else	
	const GLint internal_format = GL_R32F;
#  endif	
#else
	// Note: I do not know what GL_R16 corresponds to internally,
	// it is different from GL_R16F and GL_R16I and GL_R16UI
	// (different behavior when I use them).
        const GLint internal_format = GL_R16; 
#endif
	
        if(!blur_1_.initialize(
	       width(), height(), false, internal_format
	   )
	) {
            Logger::err("UnsharpM")
                << "blur_1_ FBO is not initialized" << std::endl;
        }
        if(!blur_2_.initialize(
	       width(), height(), false, internal_format
	   )
	) {
            Logger::err("UnsharpM")
                << "blur_2_ FBO is not initialized" << std::endl;
        }

	// Shader sources are embedded in source code,
	// Initial sourcecode is in:
	// geogram_gfx/GLUP/shaders/fullscreen
	unsharp_masking_program_ = GLSL::compile_program_with_includes_no_link(
	    this,
	    "//stage GL_VERTEX_SHADER\n"
	    "//import <fullscreen/vertex_shader.h>\n",
	    "//stage GL_FRAGMENT_SHADER\n"
	    "//import <fullscreen/unsharp_masking_fragment_shader.h>\n"
	);
	
        
        glBindAttribLocation(unsharp_masking_program_, 0, "vertex_in");
        glBindAttribLocation(unsharp_masking_program_, 1, "tex_coord_in");

        GLSL::link_program(unsharp_masking_program_);
        
        GLSL::set_program_uniform_by_name(
            unsharp_masking_program_, "blur_texture", 0
        );
        GLSL::set_program_uniform_by_name(
            unsharp_masking_program_, "depth_texture", 1
        );

	// Shader sources are embedded in source code,
	// Initial sourcecode is in:
	// geogram_gfx/GLUP/shaders/fullscreen
	blur_program_ = GLSL::compile_program_with_includes_no_link(
	    this,
	    "//stage GL_VERTEX_SHADER\n"
	    "//import <fullscreen/vertex_shader.h>\n",
	    "//stage GL_FRAGMENT_SHADER\n"
	    "//import <fullscreen/blur_fragment_shader.h>\n"                
	);

        glBindAttribLocation(blur_program_, 0, "vertex_in");
        glBindAttribLocation(blur_program_, 1, "tex_coord_in");

        GLSL::link_program(blur_program_);
        
        GLSL::set_program_uniform_by_name(
            blur_program_, "source_tex", 0
        );
        GLSL::set_program_uniform_by_name(
            blur_program_, "blur_width", 2.0f
        );
	if(ES_profile_) {
	    GLSL::set_program_uniform_by_name(
		blur_program_, "source_tex_size",
		float(blur_1_.width), float(blur_1_.height)
	    );
	}
        update();
    }

    void UnsharpMaskingImpl::pre_render(index_t w, index_t h) {
        FullScreenEffectImpl::pre_render(w,h);
    }

    void UnsharpMaskingImpl::resize(index_t width, index_t height) {
        blur_1_.resize(width, height);
        blur_2_.resize(width, height);
	if(ES_profile_) {
	    GLSL::set_program_uniform_by_name(
		blur_program_, "source_tex_size",
		float(width), float(height)
	    );
	}
        FullScreenEffectImpl::resize(width, height);
    }

    void UnsharpMaskingImpl::blur() {
        glViewport(0, 0, GLsizei(width()), GLsizei(height()));
	
        // Horizontal blur: ZBuffer -> blur_2_
	draw_FBO_.bind_depth_buffer_as_texture();	
        blur_2_.bind_as_framebuffer();
        GLSL::set_program_uniform_by_name(blur_program_, "vertical", false);
        glUseProgram(blur_program_);
        draw_unit_textured_quad();
        blur_2_.unbind();
	draw_FBO_.unbind();
	
        // Vertical blur: blur_2_ -> blur_1_
        blur_1_.bind_as_framebuffer();
        blur_2_.bind_as_texture();
        GLSL::set_program_uniform_by_name(blur_program_, "vertical", true);
        glUseProgram(blur_program_);
        draw_unit_textured_quad();	
        blur_1_.unbind();
	blur_2_.unbind();
	
        glUseProgram(0);
    }

    void UnsharpMaskingImpl::display_final_texture() {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glViewport(0, 0, GLsizei(width()), GLsizei(height()));	

        glUseProgram(unsharp_masking_program_);
        glActiveTexture(GL_TEXTURE1);
	draw_FBO_.bind_depth_buffer_as_texture();
        glActiveTexture(GL_TEXTURE0);
        blur_1_.bind_as_texture();
        draw_unit_textured_quad();
        glUseProgram(0);
	draw_FBO_.unbind();	

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        blur_1_.unbind();
    }

    void UnsharpMaskingImpl::post_render() {
	// Copies the content of draw_FBO_ to the screen.	
        FullScreenEffectImpl::post_render();

	// Computes effect and composites effect with
	// previously rendered image.
        blur();
        display_final_texture();
	reset_alpha();
    }

    void UnsharpMaskingImpl::update() {
        if(!OK()) {
            return;
        }
        if(unsharp_masking_program_ != 0) {
            float fx = 1.0f - float(contrast_)/100.0f;
            geo_clamp(fx, 0.0f, 1.0f);
            GLSL::set_program_uniform_by_name(
                unsharp_masking_program_,
                "shadows_gamma", fx
            );
            GLSL::set_program_uniform_by_name(
                unsharp_masking_program_,
                "shadows_intensity", float(intensity_) / 400.0f
            );
            GLSL::set_program_uniform_by_name(
                unsharp_masking_program_, "shadows_halo", halos_
            );
            GLSL::set_program_uniform_by_name(
                unsharp_masking_program_, "do_positive_shadows",
                positive_shadows_
            );
        }
        if(blur_program_ != 0) {
            GLSL::set_program_uniform_by_name(
                blur_program_, "blur_width", float(blur_width_)
            );
        }
    }
}
