/*
 *  Copyright (c) 2012-2015, Bruno Levy
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

#include <geogram_gfx/basic/GL.h>
#include <geogram_gfx/basic/GLSL.h>
#include <geogram_gfx/GLUP/GLUP.h>
#include <geogram_gfx/GLUP/GLUP_context.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/command_line.h>

namespace {
    using namespace GEO;
 
    /** 
     * \brief Converts a mat4 into a matrix for OpenGL.
     * \param[in] m a const reference to the matrix
     * \return a const pointer to the converted matrix,
     *  stored as a static array of 16 doubles.
     * \note The matrix is transposed before being sent to OpenGL
     *  because Geogram uses the convention with column
     *  vectors and OpenGL the convention with row vectors
     *  to represent the transformed points.
     */
    const GLdouble* convert_matrix(
        const mat4& m
    ) {
        static double result[16] ;
        index_t k = 0 ;
        for(index_t i=0; i<4; i++) {
            for(index_t j=0; j<4; j++) {
                result[k] = m(i,j) ;
                k++ ;
            }
        }
        return result ;
    }

    GLuint quad_VAO = 0;
    GLuint quad_vertices_VBO = 0;
    GLuint quad_tex_coords_VBO = 0;
    GLuint quad_program = 0;
    GLuint quad_program_BW = 0;

    /**
     * \brief Creates the VAO, VBOs and program used to draw
     *  a single textured quad.
     * \details We often have to do that, for instance for blitting
     *  a FrameBufferObject onto the screen (or onto another FrameBufferObject)
     */
    void create_quad_VAO_and_program() {
        //   All that stuff just to draw a single textured square,
        // the new OpenGL is really painful !!! 
        // (one could use glBegin()/glVertex()/glEnd() instead, but
        // this would not work with pure Core OpenGL profile and
        // neither with OpenGL ES !!

       
        // Will be drawn with a triangle strip (supported in both
        // OpenGL and OpenGL ES)

        static GLfloat coords[4][2] = {
            {-1.0f, -1.0f},
            { 1.0f, -1.0f},
            {-1.0f,  1.0f},        
            { 1.0f,  1.0f}
        };

        static GLfloat tex_coords[4][2] = {
            { 0.0f,  0.0f},
            { 1.0f,  0.0f},
            { 0.0f,  1.0f},        
            { 1.0f,  1.0f}
        };

        update_buffer_object(
            quad_vertices_VBO, GL_ARRAY_BUFFER,
            sizeof(GL_FLOAT)*2*4, coords
        );
        update_buffer_object(
            quad_tex_coords_VBO, GL_ARRAY_BUFFER,
            sizeof(GL_FLOAT)*2*4, tex_coords
        );
        glupGenVertexArrays(1, &quad_VAO);
        glupBindVertexArray(quad_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, quad_vertices_VBO); 
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0,        // Attribute 0
            2,        // nb coordinates per vertex
            GL_FLOAT, // input coordinates representation
            GL_FALSE, // do not normalize
            0,        // offset between two consecutive vertices (0 = packed)
            nullptr   // addr. relative to bound VBO 
        );

        glBindBuffer(GL_ARRAY_BUFFER, quad_tex_coords_VBO);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1,        // Attribute 1
            2,        // nb coordinates per vertex
            GL_FLOAT, // input coordinates representation
            GL_FALSE, // do not normalize
            0,        // offset between two consecutive vertices (0 = packed)
            nullptr   // addr. relative to bound VBO 
        );
        
        glupBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

#if defined(GEO_OS_EMSCRIPTEN) || defined(GEO_OS_ANDROID)
        static const char* vshader_source =
            "#version 100                               \n"            
            "attribute vec2 vertex_in;                  \n"
            "attribute vec2 tex_coord_in;               \n"
            "varying vec2 tex_coord;                    \n"
            "void main() {                              \n"
            "  tex_coord = tex_coord_in;                \n"
            "  gl_Position = vec4(vertex_in, 0.0, 1.0); \n"
            "}                                          \n"
            ;

        static const char* fshader_source =
            "#version 100                               \n"
	    "precision mediump float;                   \n"        	    
            "varying vec2 tex_coord;                    \n"
            "uniform sampler2D tex;                     \n"
            "void main() {                              \n"
            "   gl_FragColor = texture2D(               \n"
            "       tex, tex_coord                      \n"
            "   );                                      \n"
            "}                                          \n"
            ;

        static const char* fshader_BW_source =
            "#version 100                               \n"
	    "precision mediump float;                   \n"        	    
            "varying vec2 tex_coord;                    \n"
            "uniform sampler2D tex;                     \n"
            "void main() {                              \n"
            "   gl_FragColor = vec4(texture2D(          \n"
            "       tex, tex_coord                      \n"
            "   ).xxx,1.0);                             \n"
            "}                                          \n"
            ;
	
#else

        static const char* vshader_source =
#   ifdef GEO_OS_APPLE
            "#version 330                               \n"	    
#   else	    
            "#version 130                               \n"
#   endif	    
            "in vec2 vertex_in;                         \n"
            "in vec2 tex_coord_in;                      \n"
            "out vec2 tex_coord;                        \n"
            "void main() {                              \n"
            "  tex_coord = tex_coord_in;                \n"
            "  gl_Position = vec4(vertex_in, 0.0, 1.0); \n"
            "}                                          \n"
            ;

        static const char* fshader_source =
#   ifdef GEO_OS_APPLE
            "#version 330                               \n"	    
#   else	    
            "#version 130                               \n"
#   endif	    
            "out vec4 frag_color ;                      \n"
            "in vec2 tex_coord;                         \n"
            "uniform sampler2D tex;                     \n"
            "void main() {                              \n"
            "   frag_color = texture(                   \n"
            "       tex, tex_coord                      \n"
            "   );                                      \n"
            "}                                          \n"
            ;

        static const char* fshader_BW_source =
#   ifdef GEO_OS_APPLE
            "#version 330                               \n"	    
#   else	    
            "#version 130                               \n"
#   endif	    
            "out vec4 frag_color ;                      \n"
            "in vec2 tex_coord;                         \n"
            "uniform sampler2D tex;                     \n"
            "void main() {                              \n"
            "   frag_color = vec4(texture(              \n"
            "       tex, tex_coord                      \n"
            "   ).xxx,1.0);                             \n"
            "}                                          \n"
            ;
#endif
	
        GLuint vshader = GLSL::compile_shader(
            GL_VERTEX_SHADER, vshader_source, nullptr
        );
        
        GLuint fshader = GLSL::compile_shader(
            GL_FRAGMENT_SHADER, fshader_source, nullptr
        );

        GLuint fshader_BW = GLSL::compile_shader(
            GL_FRAGMENT_SHADER, fshader_BW_source, nullptr
        );
        
	
        quad_program = GLSL::create_program_from_shaders_no_link(
            vshader, fshader, nullptr
        );

        quad_program_BW = GLSL::create_program_from_shaders_no_link(
            vshader, fshader_BW, nullptr
        );
	
        glBindAttribLocation(quad_program, 0, "vertex_in");
        glBindAttribLocation(quad_program, 1, "tex_coord_in");
        GLSL::link_program(quad_program);


        glBindAttribLocation(quad_program_BW, 0, "vertex_in");
        glBindAttribLocation(quad_program_BW, 1, "tex_coord_in");
        GLSL::link_program(quad_program_BW);
	
        GLSL::set_program_uniform_by_name(quad_program_BW, "tex", 0);
        
        glDeleteShader(vshader);
        glDeleteShader(fshader);
        glDeleteShader(fshader_BW);	
    }
}

namespace GEO {

    namespace GL {
        
        void initialize() {
        }

        void terminate() {
            if(quad_program != 0) {
                glDeleteProgram(quad_program);
                quad_program = 0;
            }
            if(quad_program_BW != 0) {
                glDeleteProgram(quad_program_BW);
                quad_program_BW = 0;
            }
            if(quad_VAO != 0) {
                glupDeleteVertexArrays(1, &quad_VAO);
                quad_VAO = 0;
            }
            if(quad_vertices_VBO != 0) {
                glDeleteBuffers(1, &quad_vertices_VBO);
                quad_vertices_VBO = 0;
            }
            if(quad_tex_coords_VBO != 0) {
                glDeleteBuffers(1, &quad_tex_coords_VBO);
                quad_tex_coords_VBO = 0;
            }
        }
    }

    void glupMapTexCoords1d(double minval, double maxval, index_t mult) {
        glupMatrixMode(GLUP_TEXTURE_MATRIX);
        GLUPdouble M[16];
        Memory::clear(M, sizeof(GLUPdouble)*16);
        double d = maxval - minval;
        if(::fabs(d) < 1e-10) {
            d = 0.0;
        } else {
            d = 1.0 / d;
        }
        d *= double(std::max(mult, 1u));
        M[0] =  d;
        M[12] = -d*minval;
        M[15] = 1.0;
        glupLoadMatrixd(M);
        glupMatrixMode(GLUP_MODELVIEW_MATRIX);
    }
    
    void glupLoadMatrix(const mat4& m) {
        glupLoadMatrixd(convert_matrix(m));
    }

    void glupMultMatrix(const mat4& m) {
        glupMultMatrixd(convert_matrix(m));
    }
    

    GLint64 get_size_of_bound_buffer_object(GLenum target) {

#ifdef GLUP_DEBUG
        {
            GLint buffer = 0;
            switch(target) {
            case GL_ARRAY_BUFFER:
                glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &buffer);
                break;
            case GL_ELEMENT_ARRAY_BUFFER:
                glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &buffer);
                break;
            default:
                geo_assert_not_reached;
                break;
            }
            geo_assert(buffer != 0);
        }
#endif        
        
        GLint64 result=0;
        
#ifdef GEO_GL_440
        
        static bool init = false;
        static bool use_glGetBufferParameteri64v = false;

        // Note: there is a version of glGetBufferParameteriv that uses
        // 64 bit parameters. Since array data larger than 4Gb will be
        // common place, it is this version that should be used. However,
        // it is not supported by the Intel driver (therefore we fallback
        // to the standard 32 bits version if such a driver is detected).
	// It is not implemented by Gallium either... Oh well, for now
	// I deactivate it if the driver is not NVIDIA.
	
        if(!init) {
            init = true;
            const char* vendor = (const char*)glGetString(GL_VENDOR);
            use_glGetBufferParameteri64v = (
                strlen(vendor) >= 6 && !strncmp(vendor, "NVIDIA", 6) &&
		(glGetBufferParameteri64v != nullptr)
            );
            // Does not seem to be implemented under OpenGL ES
            if(CmdLine::get_arg("gfx:GL_profile") == "ES") {
                use_glGetBufferParameteri64v = false;
            }
            if(use_glGetBufferParameteri64v) {
                Logger::out("GLSL")
                    << "using glGetBufferParameteri64v"
                    << std::endl;
            }
        }
        if(use_glGetBufferParameteri64v) {
            glGetBufferParameteri64v(target,GL_BUFFER_SIZE,&result);
        } else
#endif
        {
            GLint result32=0;
            glGetBufferParameteriv(target,GL_BUFFER_SIZE,&result32);
            result = GLint64(result32);
        }
        return result;
    }


    void update_buffer_object(
        GLuint& buffer_id, GLenum target, size_t new_size, const void* data
    ) {
        if(new_size == 0) {
            if(buffer_id != 0) {
                glDeleteBuffers(1, &buffer_id);
                buffer_id = 0;
            }
            return;
        }

        GLint64 size = 0;        
        if(buffer_id == 0) {
            glGenBuffers(1, &buffer_id);
            glBindBuffer(target, buffer_id);            
        } else {
            glBindBuffer(target, buffer_id);
            size = get_size_of_bound_buffer_object(target);
        }
        
        if(new_size == size_t(size)) {
            glBufferSubData(target, 0, GLsizeiptr(size), data);
        } else {
            glBufferData(
                target, GLsizeiptr(new_size), data, GL_STATIC_DRAW
            );
        }
    }

    void stream_buffer_object(
        GLuint& buffer_id, GLenum target, size_t new_size, const void* data
    ) {
        if(new_size == 0) {
            if(buffer_id != 0) {
                glDeleteBuffers(1, &buffer_id);
                buffer_id = 0;
            }
            return;
        }

        GLint64 size = 0;        
        if(buffer_id == 0) {
            glGenBuffers(1, &buffer_id);
            glBindBuffer(target, buffer_id);            
        } else {
            glBindBuffer(target, buffer_id);
            size = get_size_of_bound_buffer_object(target);
        }
        
        if(new_size == size_t(size)) {
            //   Binding nullptr makes the GPU-side allocated buffer "orphan",
            // if there was a rendering operation currently using it, then
            // it can safely continue.
            glBufferData(target, GLsizeiptr(size), nullptr, GL_STREAM_DRAW);
            //   And here we bind a fresh new block of GPU-side memory.
            // See https://www.opengl.org/wiki/Buffer_Object_Streaming
            glBufferData(target, GLsizeiptr(size), data, GL_STREAM_DRAW);
        } else {
            glBufferData(
                target, GLsizeiptr(new_size), data, GL_STATIC_DRAW
            );
        }
    }

    
    void update_or_check_buffer_object(
        GLuint& buffer_id, GLenum target, size_t new_size, const void* data,
        bool update
    ) {
        if(update) {
            update_buffer_object(buffer_id, target, new_size, data);
        } else {
            glBindBuffer(target, buffer_id);
            if(new_size != size_t(get_size_of_bound_buffer_object(target))) {
                Logger::warn("OpenGL")
                    << "Buffer Object does not have the expected size."
                    << std::endl;
                Logger::warn("OpenGL")
                    << "An object was probably changed "
                    << "without notifying/updating the graphics."
                    << std::endl;
                Logger::warn("OpenGL")
                    << "Forcing Buffer Object update."
                    << std::endl;
                update_buffer_object(buffer_id, target, new_size, data);
            }
        }
    }

    void check_gl(const char* file, int line, bool warning_only) {
       // TODO: implement some form of gluErrorString(error_code) 		
	
        GLenum error_code = glGetError() ;
        bool has_opengl_errors = false ;
        while(error_code != GL_NO_ERROR) {
            has_opengl_errors = true ;
	    if(warning_only) {
		Logger::warn("OpenGL")
		    << file << ":" << line << " "
		    << "(ignored)"
		    << std::endl;
	    } else {
		Logger::err("OpenGL")
		    << file << ":" << line << " "
		    << std::endl;
	    }
	    error_code = glGetError() ;
	}
        geo_argused(has_opengl_errors);
    }

    void clear_gl_error_flags(const char* file, int line) {
#ifdef GEO_DEBUG
	check_gl(file,line,true);
#else
	geo_argused(file);
	geo_argused(line);
        while(glGetError() != GL_NO_ERROR);
#endif	
    }
    
    void draw_unit_textured_quad(bool BW) {
        if(quad_VAO == 0) {
            create_quad_VAO_and_program();
        }
        GLint current_program = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
        if(current_program == 0) {
            glUseProgram(BW ? quad_program_BW : quad_program);
        }
        glupBindVertexArray(quad_VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);        
        glupBindVertexArray(0);
        if(current_program == 0) {
            glUseProgram(0);
        }
    }

    /****************************************************************/


    static int htoi(char digit) {
	if(digit >= '0' && digit <= '9') {
	    return digit - '0';
	}
	if(digit >= 'a' && digit <= 'f') {
	    return digit - 'a' + 10;
	}
	if(digit >= 'A' && digit <= 'F') {
	    return digit - 'A' + 10;
	}
	fprintf(stderr, "xpm: unknown digit\n");
	return 0;
    }

    /* The colormap. */
    static unsigned char i2r[1024];
    static unsigned char i2g[1024];
    static unsigned char i2b[1024];
    static unsigned char i2a[1024];
    
    /*
     * Converts a two-digit XPM color code into
     *  a color index.
     */
    static int char_to_index[256][256];
    
    void glTexImage2Dxpm(char const* const* xpm_data) {
	int width, height, nb_colors, chars_per_pixel;
	int line = 0;
	int color = 0;
	int key1 = 0, key2 = 0;
	const char* colorcode;
	int x, y;
	unsigned char* rgba;
	unsigned char* pixel;

	sscanf(
	    xpm_data[line], "%6d%6d%6d%6d",
	    &width, &height, &nb_colors, &chars_per_pixel
	);
	line++;
	if(nb_colors > 1024) {
	    fprintf(stderr, "xpm with more than 1024 colors\n");
	    return;
	}
	if(chars_per_pixel != 1 && chars_per_pixel != 2) {
	    fprintf(stderr, "xpm with more than 2 chars per pixel\n");
	    return;
	}
	for(color = 0; color < nb_colors; color++) {
	    int r, g, b;
	    int none ;
        
	    key1 = xpm_data[line][0];
	    key2 = (chars_per_pixel == 2) ? xpm_data[line][1] : 0;
	    colorcode = strstr(xpm_data[line], "c #");
	    none = 0;
	    if(colorcode == nullptr) {
		colorcode = "c #000000";
		if(strstr(xpm_data[line], "None") != nullptr) {
		    none = 1;
		} else {
		    fprintf(
			stderr, "unknown xpm color entry (replaced with black)\n"
		    );
		}
	    }
	    colorcode += 3;

	    if(strlen(colorcode) == 12) {
		r = 16 * htoi(colorcode[0]) + htoi(colorcode[1]);
		g = 16 * htoi(colorcode[4]) + htoi(colorcode[5]);
		b = 16 * htoi(colorcode[8]) + htoi(colorcode[9]);
	    } else {
		r = 16 * htoi(colorcode[0]) + htoi(colorcode[1]);
		g = 16 * htoi(colorcode[2]) + htoi(colorcode[3]);
		b = 16 * htoi(colorcode[4]) + htoi(colorcode[5]);
	    }
	    
	    i2r[color] = (unsigned char) r;
	    i2g[color] = (unsigned char) g;
	    i2b[color] = (unsigned char) b;
	    if(none) {
		i2a[color] = 0;
	    } else {
		i2a[color] = 255;
	    }
	    char_to_index[key1][key2] = color;
	    line++;
	}
	rgba = (unsigned char*) malloc((size_t) (width * height * 4));
	pixel = rgba;
	for(y = 0; y < height; y++) {
	    for(x = 0; x < width; x++) {
		if(chars_per_pixel == 2) {
		    key1 = xpm_data[line][2 * x];
		    key2 = xpm_data[line][2 * x + 1];
		} else {
		    key1 = xpm_data[line][x];
		    key2 = 0;
		}
		color = char_to_index[key1][key2];
		pixel[0] = i2r[color];
		pixel[1] = i2g[color];
		pixel[2] = i2b[color];
		pixel[3] = i2a[color];
		pixel += 4;
	    }
	    line++;
	}
    
	glTexImage2D(
	    GL_TEXTURE_2D, 0,
	    GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba
	);
#ifndef __EMSCRIPTEN__    
	glGenerateMipmap(GL_TEXTURE_2D);
#endif    
	free(rgba);
    }
    
    
}
