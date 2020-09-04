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

#include <geogram_gfx/GLUP/GLUP.h>
#include <geogram_gfx/GLUP/GLUP_context_GLSL.h>
#include <geogram_gfx/GLUP/GLUP_context_ES.h>
#include <geogram_gfx/basic/GLSL.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/file_system.h>
#include <geogram/basic/command_line.h>

#ifdef GEO_OS_EMSCRIPTEN
#include <emscripten.h>
#pragma GCC diagnostic ignored "-Wdollar-in-identifier-extension"
#endif

#ifdef GEO_OS_ANDROID
#  pragma GCC diagnostic ignored "-Wpointer-bool-conversion"
#endif


/*****************************************************************************/

namespace {
    using namespace GEO;
    void downgrade_message() {
	static bool done = false;
	if(!done) {
	    done = true;
	    Logger::out("GLUP") << "Something happened, trying to fix (by downgrading)" << std::endl;
	    Logger::out("GLUP") << "If this does not work, try the following options:" << std::endl;
	    Logger::out("GLUP") << " (1) make sure your OpenGL driver is up to date" << std::endl;
	    Logger::out("GLUP") << " (2) create a file named \'"
				<< CmdLine::get_config_file_name() << "\' in " << std::endl;
	    Logger::out("GLUP") << "     your home directory (" << FileSystem::home_directory() << ")" << std::endl;
	    Logger::out("GLUP") << "     with: " << std::endl;
	    Logger::out("GLUP") << "       gfx:GLUP_profile=GLUPES2" << std::endl;
	}
    }
}

/*****************************************************************************/

namespace GLUP {
    using namespace GEO;
    
    extern GLUP_API Context* current_context_;
    Context* current_context_ = nullptr;
    
    static std::set<Context*> all_contexts_;
    static bool initialized_ = false;
    static void cleanup() {
#ifdef GEO_OS_ANDROID
	// Note: removal of context from all_contexts_
	// is done in glupDeleteContext() (not in Context
	// destructor), so we can direcly iterate on all_contexts_
	// and delete.
	// TODO: check that GLUP contexts are really destroyed *before*
	// the OpenGL context. (yes it is, because if I output
	// something to the logger here, it appears in the app's temrinal).
	for(auto ctxt: all_contexts_) {
	    delete ctxt;
	}
	all_contexts_.clear();
	return;
#endif	
	
	// Note: remaining contexts are not deallocated here because:
	// (1) there should be no remaining context (it is Application's
	//   job to deallocate them).
	// (2) when cleanup() is called, Application's delete_window() was
	//   called before, as well as glfwDeleteWindow() / glfwTerminate()
	//   so the OpenGL context is destroyed (and it is not legal to destroy
	//   it before the OpenGL objects in the GLUP context)
	if(all_contexts_.size() != 0) {
	    Logger::warn("GLUP") << "Some GLUP contexts were not deallocated"
				 << std::endl;
	    Logger::warn("GLUP") << "App\'s GL_terminate() probably forgot to"
				 << std::endl;
	    Logger::warn("GLUP") << "call SimpleApplication\'s GL_terminate()"
				 << std::endl;
	    Logger::warn("GLUP") << "(needs to be at then end of the function)"
				 << std::endl;
	}
    }
    
}

/*****************************************************************************/

const char* glupUniformStateDeclaration() {
    GEO_CHECK_GL();     
    return GLUP::current_context_->uniform_state_declaration();
}

GLUPuint glupCompileShader(GLUPenum target, const char* source) {
    GEO_CHECK_GL();
    GLUP::current_context_->setup_shaders_source_for_primitive(GLUP_TRIANGLES);
    // First param: ignored.
    // Second param: all toggle states are unknown.
    GLUP::current_context_->setup_shaders_source_for_toggles(0,~0);
    GEO_CHECK_GL();
    GLUPuint result = GLSL::compile_shader_with_includes(
	target, source, GLUP::current_context_
    );
    GEO_CHECK_GL();
    return result;
}

namespace {
    /**
     * \brief Converts a comment //stage GL_VERTEX_SHADER into
     *  the associated GLUPenum value.
     * \param[in,out] comment_str on entry, a pointer to the comment.
     *  on exit, the next line. A '\0' terminator is inserted in
     *  the string (replaces the '\n' that ends the line).
     * \return the target or 0 if an error was encountered.
     */
    static GLUPenum stage_to_target(char*& comment_str) {
	char* p1 = comment_str + 8;
	char* p2 = strchr(p1, '\n');
	if(p2 == nullptr) {
	    Logger::err("GLSL")
		<< "Missing CR in //stage GL_xxxxx declaration"
		<< std::endl;
	    return 0;
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
	}
	*comment_str = '\0';
	comment_str = p2+1;
	return stage;
    }
}

GLUPuint glupCompileProgram(const char* source_in) {
    std::string source(source_in);
    std::vector<const char*> sources;
    std::vector<GLUPenum> targets;
    std::vector<GLuint> shaders;

    char* p = const_cast<char*>(source.c_str());

    bool has_vertex_shader = false;
    for(
	p = strstr(p,"//stage");
	(p != nullptr) && (*p != '\0');
	p = strstr(p,"//stage ")
    ) {
	GLUPenum target = stage_to_target(p);
	if(target == 0) {
	    return 0;
	}
	sources.push_back(p);
	targets.push_back(target);
	has_vertex_shader = has_vertex_shader || (target == GL_VERTEX_SHADER);
    }

    // Use default vertex shader if no vertex shader was specified.
    if(!has_vertex_shader) {
	if(!strcmp(glupCurrentProfileName(),"GLUPES2")) {
	    targets.push_back(GL_VERTEX_SHADER);	    
	    sources.push_back("//import <GLUPES/vertex_shader.h>\n");
	} else if(
	    !strcmp(glupCurrentProfileName(),"GLUP150") ||
	    !strcmp(glupCurrentProfileName(),"GLUP440")	    
	) {
	    targets.push_back(GL_VERTEX_SHADER);	    	    
	    sources.push_back("//import <GLUPGLSL/vertex_shader.h>\n");	    
	}
    }
    
    GLuint program = 0;
    try {
	for(index_t i=0; i<index_t(sources.size()); ++i) {
	    GLUPuint shader = glupCompileShader(targets[i], sources[i]);
	    if(shader == 0) {
#ifdef GEO_OS_EMSCRIPTEN
		return 0;
#else
		throw(GLSL::GLSLCompileError());
#endif		
	    }
	    shaders.push_back(shader);
	}
	
    	program = glCreateProgram();
	
	for(index_t i=0; i<index_t(shaders.size()); ++i) {
	    glAttachShader(program, shaders[i]);
	}
	
        GEO_CHECK_GL();    	
        glBindAttribLocation(program, GLUP::GLUP_VERTEX_ATTRIBUTE, "vertex_in");
        glBindAttribLocation(program, GLUP::GLUP_COLOR_ATTRIBUTE, "color_in");
        glBindAttribLocation(
	    program, GLUP::GLUP_TEX_COORD_ATTRIBUTE, "tex_coord_in"
	);
        glBindAttribLocation(program, GLUP::GLUP_NORMAL_ATTRIBUTE, "normal_in");
	
        GEO_CHECK_GL();    		
	GLSL::link_program(program);
        GEO_CHECK_GL();    			
	GLUP::current_context_->bind_uniform_state(program);
        GEO_CHECK_GL();
    } catch(...) {
	if(program != 0) {
	    Logger::err("GLSL") << "Could not compile program"
				<< std::endl;
	    glDeleteProgram(program);
	    program = 0;
	}
    }

    // Shaders are reference-counted, now they are attached
    // to the program.
    for(index_t i=0; i<index_t(shaders.size()); ++i) {
	glDeleteShader(shaders[i]);
    }
    
    return program;
}

void glupBindUniformState(GLUPuint program) {
    GEO_CHECK_GL(); 
    GLUP::current_context_->bind_uniform_state(program);
    GEO_CHECK_GL();
}


#if !defined(GEO_OS_EMSCRIPTEN) && \
    !defined(GEO_OS_APPLE) && \
    !defined(GEO_OS_ANDROID)

/**
 * \brief Tests whether tessellation shaders are supported by OpenGL.
 * \details Some drivers may declare to be OpenGL 4.5 compliant whereas
 *  they do not have tesselation shader (for instance, I have an old
 *  NVidia quadro that does that...)
 * \retval true if tessellation shaders are supported
 * \retval false otherwise
 */
static bool supports_tessellation_shader() {
    GEO_CHECK_GL();

#ifndef GEO_GL_150
    return false;
#else    
    bool result = true;

    // Note: I experienced crashes with glPatchParameterfv() with
    // the OpenGL es profile, so I'm deactivating it if detected.
    if(GEO::CmdLine::get_arg("gfx:GL_profile") == "ES") {
        GEO::Logger::out("GLUP")
            << "Deactivating tesselation shader under OpenGL ES profile"
            << std::endl;
        return false;
    }
    
    GLuint s_handle = glCreateShader(GL_TESS_CONTROL_SHADER);
    result = result && (s_handle != 0);
    if (s_handle != 0) {
        glDeleteShader(s_handle);
    }

    // Clear OpenGL error flag.
    while(glGetError() != GL_NO_ERROR) {
    }

    return result;
#endif    
}

#endif

GLUPcontext glupCreateContext() {

    
    if(!GLUP::initialized_) {
        GLUP::initialized_ = true;
        atexit(GLUP::cleanup);
    }

    
    GEO_CHECK_GL();
    
    std::string GLUP_profile = GEO::CmdLine::get_arg("gfx:GLUP_profile");
    GLUP::Context* result = nullptr;

    if(GLUP_profile == "auto") {
      
#if defined(GEO_OS_EMSCRIPTEN) || defined(GEO_OS_APPLE) || defined(GEO_OS_ANDROID)
	GLUP_profile = "GLUPES2";
//	GLUP_profile = "GLUP150"; // Does something but bugged / not fully functional.
#else
      GEO_CHECK_GL();      
      double GLSL_version = GEO::GLSL::supported_language_version();
      GEO_CHECK_GL();            
      if (GLSL_version >= 4.4) {
	  GEO_CHECK_GL();      	  
	  if (!supports_tessellation_shader()) {
	      GEO::Logger::out("GLUP")
		<< "GLSL version >= 4.4 but tessellation unsupported"
		<< std::endl;
	      downgrade_message();
	      GEO::Logger::out("GLUP") << "Downgrading to GLUP 150..."
				       << std::endl;
	      GLSL_version = 1.5;
	  }
	  GEO_CHECK_GL();      	  
      }
      
      if(GLSL_version >= 4.4) {
	  GLUP_profile = "GLUP440";	
      } else if(GLSL_version >= 1.5) {
	  GLUP_profile = "GLUP150";	
      } else {
	  GLUP_profile = "GLUPES2";	
      }
      
#endif        
    }

    
    GEO::Logger::out("GLUP") << "Using " << GLUP_profile << " profile"
                        << std::endl;

#ifdef GEO_GL_440    
    if(GLUP_profile == "GLUP440") {
        try {
	    result = new GLUP::Context_GLSL440;
	    result->setup();	    
	} catch(...) {
	    GEO::Logger::warn("GLUP")
	        << "Caught an exception in GLUP440, downgrading to GLUP150"
	        << std::endl;
	    downgrade_message();	    
	    GLUP_profile = "GLUP150";
	    delete result;
	    result = nullptr;
	}
    }
#endif

#ifdef GEO_GL_150    
    if(GLUP_profile == "GLUP150") {
        try {
            result = new GLUP::Context_GLSL150;
	    result->setup();	    	    
        } catch(...) {
	    GEO::Logger::warn("GLUP")
	        << "Caught an exception in GLUP150, downgrading to GLUPES2"
	        << std::endl;
	    downgrade_message();	    
	    GLUP_profile = "GLUPES2";
	    delete result;
	    result = nullptr;
	}
    }
#endif
    
#ifdef GEO_GL_ES2    
    if(GLUP_profile == "GLUPES2") {
        try {      
	    result = new GLUP::Context_ES2;
	    result->setup();	    	    
        } catch(...) {
	    GEO::Logger::warn("GLUP")
	        << "Caught an exception in GLUPES2"
	        << std::endl;
	    downgrade_message();	    
	    delete result;
	    result = nullptr;
	}
    }
#endif

    if(result == nullptr) {
         GEO::Logger::err("GLUP") << "Could not create context"
	                          << std::endl;
    } else {
        GLUP::all_contexts_.insert(result);
    }
    
    return result;
}

void glupDeleteContext(GLUPcontext context_in) {
    GEO_CHECK_GL();
    
    GLUP::Context* context =
        reinterpret_cast<GLUP::Context*>(context_in);

    auto it = GLUP::all_contexts_.find(context);
    geo_assert(it != GLUP::all_contexts_.end());
    GLUP::all_contexts_.erase(it);
    
    if(GLUP::current_context_ == context) {
        GLUP::current_context_ = nullptr;
    }
    delete context;
}


GLUPcontext glupCurrentContext() {
    // Note: we cannot check GL state if OpenGL
    // is not initialized.
    if(GLUP::current_context_ != nullptr) {
	GEO_CHECK_GL();
    }
    return GLUP::current_context_;
}

const char* glupCurrentProfileName() {
    GEO_CHECK_GL();    
    return GLUP::current_context_->profile_name();
}

void glupMakeCurrent(GLUPcontext context) {
    GEO_CHECK_GL();
    GLUP::current_context_ = reinterpret_cast<GLUP::Context*>(context);
}

GLUPboolean glupPrimitiveSupportsArrayMode(GLUPprimitive prim) {
    GEO_CHECK_GL();            
    return GLUP::current_context_->primitive_supports_array_mode(prim) ?
        GL_TRUE : GL_FALSE ;
}

/****************** Enable / Disable ***************************/

namespace GLUP {
    extern bool vertex_array_emulate;
}

void glupEnable(GLUPtoggle toggle) {
    GEO_CHECK_GL();
    GLUP::current_context_->uniform_state().toggle[toggle].set(GL_TRUE);
}

void glupDisable(GLUPtoggle toggle) {
    GEO_CHECK_GL();                
    GLUP::current_context_->uniform_state().toggle[toggle].set(GL_FALSE);
}

GLUPboolean glupIsEnabled(GLUPtoggle toggle) {
    GEO_CHECK_GL();                
    return GLUP::current_context_->uniform_state().toggle[toggle].get();
}

/********************** Texturing ******************************/

void glupTextureType(GLUPtextureType type) {
    GEO_CHECK_GL();                
    GLUP::current_context_->uniform_state().texture_type.set(type);
}

GLUPtextureType glupGetTextureType() {
    GEO_CHECK_GL();                
    return GLUPtextureType(
        GLUP::current_context_->uniform_state().texture_type.get()
    );
}

void glupTextureMode(GLUPtextureMode mode) {
    GEO_CHECK_GL();
    GLUP::current_context_->uniform_state().texture_mode.set(mode);    
}

GLUPtextureMode glupGetTextureMode() {
    GEO_CHECK_GL();                
    return GLUPtextureMode(
        GLUP::current_context_->uniform_state().texture_mode.get()
    );
}

/****************** Drawing state ******************************/

void glupSetColor4fv(GLUPcolor color, const GLUPfloat* rgba) {
    GEO_CHECK_GL();                
    if(color == GLUP_FRONT_AND_BACK_COLOR) {
        glupSetColor4fv(GLUP_FRONT_COLOR, rgba);
        glupSetColor4fv(GLUP_BACK_COLOR, rgba);        
    } else {
        GLUP::current_context_->uniform_state().color[color].set(rgba);
    }
}

void glupGetColor4fv(GLUPcolor color, float* rgba) {
    GEO_CHECK_GL();                
    geo_assert(color != GLUP_FRONT_AND_BACK_COLOR);
    GLUP::current_context_->uniform_state().color[color].get(rgba);
}

void glupSetColor3fv(GLUPcolor color, const GLUPfloat* rgba) {
    GEO_CHECK_GL();                
    glupSetColor4f(color, rgba[0], rgba[1], rgba[2], 1.0);
}

void glupSetColor4f(
    GLUPcolor color, GLUPfloat r, GLUPfloat g, GLUPfloat b, GLUPfloat a
) {
    GEO_CHECK_GL();                
    if(color == GLUP_FRONT_AND_BACK_COLOR) {
        glupSetColor4f(GLUP_FRONT_COLOR, r, g, b, a);
        glupSetColor4f(GLUP_BACK_COLOR, r, g, b, a);
    } else {
        GLUPfloat* ptr =
            GLUP::current_context_->uniform_state().color[color].get_pointer();
        ptr[0] = r;
        ptr[1] = g;
        ptr[2] = b;
        ptr[3] = a;
    }
}

void glupSetColor3f(GLUPcolor color, GLUPfloat r, GLUPfloat g, GLUPfloat b) {
    GEO_CHECK_GL();                
    glupSetColor4f(color, r, g, b, 1.0f);
}

void glupSetColor4dv(GLUPcolor color, const GLUPdouble* rgba) {
    GEO_CHECK_GL();                
    glupSetColor4f(
        color,
        GLUPfloat(rgba[0]),
        GLUPfloat(rgba[1]),
        GLUPfloat(rgba[2]),
        GLUPfloat(rgba[3])
    );
}

void glupSetColor3dv(GLUPcolor color, const GLUPdouble* rgba) {
    GEO_CHECK_GL();                
    glupSetColor4f(
        color,
        GLUPfloat(rgba[0]),
        GLUPfloat(rgba[1]),
        GLUPfloat(rgba[2]),
        1.0f
    );
}

void glupSetColor4d(
    GLUPcolor color, GLUPdouble r, GLUPdouble g, GLUPdouble b, GLUPdouble a
) {
    GEO_CHECK_GL();                
    glupSetColor4f(
        color,
        GLUPfloat(r),
        GLUPfloat(g),
        GLUPfloat(b),
        GLUPfloat(a)
    );
}

void glupSetColor3d(
    GLUPcolor color, GLUPdouble r, GLUPdouble g, GLUPdouble b
) {
    GEO_CHECK_GL();                
    glupSetColor4f(
        color,
        GLUPfloat(r),
        GLUPfloat(g),
        GLUPfloat(b),
        1.0f
    );
}

void glupLightVector3f(GLUPfloat x, GLUPfloat y, GLUPfloat z) {
    GEO_CHECK_GL();                
    GLUPfloat* ptr =
        GLUP::current_context_->uniform_state().light_vector.get_pointer();
    ptr[0] = x;
    ptr[1] = y;
    ptr[2] = z;
    GLUP::current_context_->flag_lighting_as_dirty();
}

void glupLightVector3fv(GLUPfloat* xyz) {
    GEO_CHECK_GL();                
    GLUP::current_context_->uniform_state().light_vector.set(xyz);
    GLUP::current_context_->flag_lighting_as_dirty();
}

void glupGetLightVector3fv(GLUPfloat* xyz) {
    GEO_CHECK_GL();                
    GLUPfloat* ptr =
        GLUP::current_context_->uniform_state().light_vector.get_pointer();
    xyz[0] = ptr[0];
    xyz[1] = ptr[1];
    xyz[2] = ptr[2];
}

void glupSetPointSize(GLUPfloat size) {
    GEO_CHECK_GL();                
    GLUP::current_context_->uniform_state().point_size.set(size);
}

GLUPfloat glupGetPointSize() {
    GEO_CHECK_GL();                
    return GLUP::current_context_->uniform_state().point_size.get();
}

void glupSetMeshWidth(GLUPint width) {
    GEO_CHECK_GL();                
    GLUP::current_context_->uniform_state().mesh_width.set(GLfloat(width));
}

GLUPint glupGetMeshWidth() {
    GEO_CHECK_GL();                
    return GLUPint(GLUP::current_context_->uniform_state().mesh_width.get());
}

void glupSetCellsShrink(GLUPfloat x) {
    GEO_CHECK_GL();                
    x = std::min(x, 1.0f);
    x = std::max(x, 0.0f);
    GLUP::current_context_->uniform_state().cells_shrink.set(x);
}

GLUPfloat glupGetCellsShrink() {
    GEO_CHECK_GL();                
    return GLUP::current_context_->uniform_state().cells_shrink.get();    
}

void glupSetAlphaThreshold(GLUPfloat x) {
    GEO_CHECK_GL();
    GLUP::current_context_->uniform_state().alpha_threshold.set(x);    
}

GLUPfloat glupGetAlphaThreshold() {
    GEO_CHECK_GL();
    return GLUP::current_context_->uniform_state().alpha_threshold.get();
}

void glupSetSpecular(GLUPfloat x) {
    GEO_CHECK_GL();
    GLUP::current_context_->uniform_state().specular.set(x);    
}

GLUPfloat glupGetSpecular() {
    GEO_CHECK_GL();
    return GLUP::current_context_->uniform_state().specular.get();
}


/****************** Picking ******************************/

void glupPickingMode(GLUPpickingMode mode) {
    GEO_CHECK_GL();                
    GLUP::current_context_->uniform_state().picking_mode.set(mode);
}

GLUPpickingMode glupGetPickingMode() {
    GEO_CHECK_GL();                
    return GLUPpickingMode(
        GLUP::current_context_->uniform_state().picking_mode.get()
    );    
}

void glupPickingId(GLUPuint64 id) {
    // TODO: uint64
    GEO_CHECK_GL();                
    GLUP::current_context_->uniform_state().picking_id.set(GLint(id));
}

GLUPuint64 glupGetPickingId() {
    // TODO: uint64
    GEO_CHECK_GL();                
    return GLUPuint64(
        GLUP::current_context_->uniform_state().picking_id.get()
    );
}

void glupBasePickingId(GLUPuint64 id) {
    // TODO: uint64
    GEO_CHECK_GL();                
    GLUP::current_context_->uniform_state().base_picking_id.set(GLint(id));    
}

GLUPuint64 glupGetBasePickingId() {
    // TODO: uint64
    GEO_CHECK_GL();                
    return GLUPuint64(
        GLUP::current_context_->uniform_state().base_picking_id.get()
    );
}

/****************** Clipping ******************************/

void glupClipMode(GLUPclipMode mode) {
    GEO_CHECK_GL();                
    GLUP::current_context_->uniform_state().clipping_mode.set(mode);
}

GLUPclipMode glupGetClipMode() {
    GEO_CHECK_GL();                
    return GLUPclipMode(
        GLUP::current_context_->uniform_state().clipping_mode.get()
    );
}

void glupClipPlane(const GLUPdouble* eqn_in) {
    GEO_CHECK_GL();
    
    const GLfloat* modelview =
        GLUP::current_context_->get_matrix(GLUP_MODELVIEW_MATRIX);
    GLfloat modelview_invert[16];
    if(!GLUP::invert_matrix(modelview_invert,modelview)) {
        GEO::Logger::warn("GLUP") << "Singular ModelView matrix"
                             << std::endl;
        GLUP::show_matrix(modelview);
    }
    GLfloat* state_world_clip_plane =
        GLUP::current_context_->uniform_state().world_clip_plane.get_pointer();
    GLfloat* state_clip_plane =
        GLUP::current_context_->uniform_state().clip_plane.get_pointer();
    for(GEO::index_t i=0; i<4; ++i) {
        state_world_clip_plane[i] = float(eqn_in[i]);
    }
    GLUP::mult_matrix_vector(
        state_clip_plane,modelview_invert,state_world_clip_plane
    );
    // TODO? clip_clip_plane update ?
}

void glupGetClipPlane(GLUPdouble* eqn) {
    GEO_CHECK_GL();
    
    const GLfloat* ptr =
        GLUP::current_context_->uniform_state().clip_plane.get_pointer();
    eqn[0] = GLdouble(ptr[0]);
    eqn[1] = GLdouble(ptr[1]);
    eqn[2] = GLdouble(ptr[2]);
    eqn[3] = GLdouble(ptr[3]);    
}

/******************* Matrices ***************************/


void glupMatrixMode(GLUPmatrix matrix) {
    GEO_CHECK_GL();                
    GLUP::current_context_->set_matrix_mode(matrix);
}

GLUPmatrix glupGetMatrixMode() {
    GEO_CHECK_GL();                
    return GLUP::current_context_->get_matrix_mode();
}

void glupPushMatrix() {
    GEO_CHECK_GL();            
    GLUP::current_context_->push_matrix();
}

void glupPopMatrix() {
    GEO_CHECK_GL();                
    GLUP::current_context_->pop_matrix();    
}

void glupGetMatrixdv(GLUPmatrix matrix, GLUPdouble* ptr) {
    GEO_CHECK_GL();                
    for(GEO::index_t i=0; i<16; ++i) {
        ptr[i] = GLUPdouble(
            GLUP::current_context_->get_matrix(matrix)[i]
        );
    }
}

void glupGetMatrixfv(GLUPmatrix matrix, GLUPfloat* ptr) {
    GEO_CHECK_GL();                
    for(GEO::index_t i=0; i<16; ++i) {
        GLUP::copy_vector(
            ptr, GLUP::current_context_->get_matrix(matrix), 16
        );
    }
}

void glupLoadIdentity() {
    GEO_CHECK_GL();                
    GLUP::current_context_->load_identity();
}

void glupLoadMatrixf(const GLUPfloat* M) {
    GEO_CHECK_GL();                
    GLUP::current_context_->load_matrix(M);
}

void glupLoadMatrixd(const GLUPdouble* M) {
    GEO_CHECK_GL();                
    GLfloat Mf[16];
    for(GEO::index_t i=0; i<16; ++i) {
        Mf[i] = GLfloat(M[i]);
    }
    glupLoadMatrixf(Mf);
}    

void glupMultMatrixf(const GLUPfloat* M) {
    GEO_CHECK_GL();                
    GLUP::current_context_->mult_matrix(M);
}

void glupMultMatrixd(const GLUPdouble* M) {
    GEO_CHECK_GL();                
    GLfloat Mf[16];
    for(GEO::index_t i=0; i<16; ++i) {
        Mf[i] = GLfloat(M[i]);
    }
    glupMultMatrixf(Mf);
}    

void glupTranslatef(GLUPfloat x, GLUPfloat y, GLUPfloat z) {
    GEO_CHECK_GL();
    
    GLfloat M[16];

    M[4*0+0] = 1.0f;
    M[4*0+1] = 0.0f;
    M[4*0+2] = 0.0f;
    M[4*0+3] = x;

    M[4*1+0] = 0.0f;
    M[4*1+1] = 1.0f;
    M[4*1+2] = 0.0f;
    M[4*1+3] = y;

    M[4*2+0] = 0.0f;
    M[4*2+1] = 0.0f;
    M[4*2+2] = 1.0f;
    M[4*2+3] = z;

    M[4*3+0] = 0.0f;
    M[4*3+1] = 0.0f;
    M[4*3+2] = 0.0f;
    M[4*3+3] = 1.0f;

    GLUP::transpose_matrix(M);
    
    glupMultMatrixf(M);
}

void glupTranslated(GLUPdouble x, GLUPdouble y, GLUPdouble z) {
    GEO_CHECK_GL();
    
    glupTranslatef(GLfloat(x), GLfloat(y), GLfloat(z));
}

void glupScalef(GLUPfloat sx, GLUPfloat sy, GLUPfloat sz) {
    GEO_CHECK_GL();
    
    GLfloat M[16];

    M[4*0+0] = sx;
    M[4*0+1] = 0.0f;
    M[4*0+2] = 0.0f;
    M[4*0+3] = 0.0f;

    M[4*1+0] = 0.0f;
    M[4*1+1] = sy;
    M[4*1+2] = 0.0f;
    M[4*1+3] = 0.0f;

    M[4*2+0] = 0.0f;
    M[4*2+1] = 0.0f;
    M[4*2+2] = sz;
    M[4*2+3] = 0.0f;

    M[4*3+0] = 0.0f;
    M[4*3+1] = 0.0f;
    M[4*3+2] = 0.0f;
    M[4*3+3] = 1.0f;

    glupMultMatrixf(M);
}

void glupScaled(GLUPdouble sx, GLUPdouble sy, GLUPdouble sz) {
    GEO_CHECK_GL();
    
    glupScalef(GLfloat(sx), GLfloat(sy), GLfloat(sz));    
}

void glupRotatef(
    GLUPfloat angle, GLUPfloat x, GLUPfloat y, GLUPfloat z
) {
    GEO_CHECK_GL();
    
    GLUPfloat l = 1.0f / ::sqrtf(x*x+y*y+z*z);
    x *= l;
    y *= l;
    z *= l;
    GLUPfloat s = ::sinf(angle * GLUPfloat(M_PI) / 180.0f);
    GLUPfloat c = ::cosf(angle * GLUPfloat(M_PI) / 180.0f);
    GLUPfloat M[16];

    M[4*0+0] = x*x*(1.0f-c)+c;
    M[4*0+1] = x*y*(1.0f-c)-z*s;
    M[4*0+2] = x*z*(1.0f-c)+y*s;
    M[4*0+3] = 0.0f;

    M[4*1+0] = y*x*(1.0f-c)+z*s;
    M[4*1+1] = y*y*(1.0f-c)+c;
    M[4*1+2] = y*z*(1.0f-c)-x*s;
    M[4*1+3] = 0.0f;

    M[4*2+0] = z*x*(1.0f-c)-y*s;
    M[4*2+1] = z*y*(1.0f-c)+x*s;
    M[4*2+2] = z*z*(1.0f-c)+c;
    M[4*2+3] = 0.0f;

    M[4*3+0] = 0.0f;
    M[4*3+1] = 0.0f;
    M[4*3+2] = 0.0f;
    M[4*3+3] = 1.0f;

    GLUP::transpose_matrix(M);
    
    glupMultMatrixf(M);
}

void glupRotated(
    GLUPdouble angle, GLUPdouble x, GLUPdouble y, GLUPdouble z
) {
    GEO_CHECK_GL();
    
    glupRotatef(GLfloat(angle), GLfloat(x), GLfloat(y), GLfloat(z));
}


void glupOrtho(
    GLUPdouble left, GLUPdouble right,
    GLUPdouble bottom, GLUPdouble top,
    GLUPdouble nearVal, GLUPdouble farVal
) {
    GEO_CHECK_GL();
    
    GLfloat M[16];

    GLdouble tx = -(right+left)/(right-left);
    GLdouble ty = -(top+bottom)/(top-bottom);
    GLdouble tz = -(farVal+nearVal)/(farVal-nearVal);
    
    M[4*0+0] = GLfloat(2.0 / (right-left));
    M[4*0+1] = 0.0f;
    M[4*0+2] = 0.0f;
    M[4*0+3] = GLfloat(tx);

    M[4*1+0] = 0.0f;
    M[4*1+1] = GLfloat(2.0 / (top-bottom));
    M[4*1+2] = 0.0f;
    M[4*1+3] = GLfloat(ty);

    M[4*2+0] = 0.0f;
    M[4*2+1] = 0.0f;
    M[4*2+2] = GLfloat(-2.0 / (farVal - nearVal));
    M[4*2+3] = GLfloat(tz);

    M[4*3+0] = 0.0f;
    M[4*3+1] = 0.0f;
    M[4*3+2] = 0.0f;
    M[4*3+3] = 1.0f;
    
    GLUP::transpose_matrix(M);
    glupMultMatrixf(M);
}

void glupOrtho2D(
    GLUPdouble left, GLUPdouble right, GLUPdouble bottom, GLUPdouble top
) {
    GEO_CHECK_GL();
    
    glupOrtho(left, right, bottom, top, -1.0, 1.0);
}

void glupFrustum(
    GLUPdouble left, GLUPdouble right,
    GLUPdouble bottom, GLUPdouble top,
    GLUPdouble nearVal, GLUPdouble farVal
) {
    GEO_CHECK_GL();
    
    GLfloat M[16];

    GLdouble A = (right + left) / (right - left);
    GLdouble B = (top + bottom) / (top - bottom);
    GLdouble C = -(farVal + nearVal) / (farVal - nearVal);
    GLdouble D = -2.0*farVal*nearVal / (farVal - nearVal);
    
    M[4*0+0] = GLfloat(2.0 * nearVal / (right - left));
    M[4*0+1] = 0.0f;
    M[4*0+2] = GLfloat(A);
    M[4*0+3] = 0.0f;

    M[4*1+0] = 0.0f;
    M[4*1+1] = GLfloat(2.0 * nearVal / (top - bottom));
    M[4*1+2] = GLfloat(B);
    M[4*1+3] = 0.0f;

    M[4*2+0] = 0.0f;
    M[4*2+1] = 0.0f;
    M[4*2+2] = GLfloat(C);
    M[4*2+3] = GLfloat(D);

    M[4*3+0] =  0.0f;
    M[4*3+1] =  0.0f;
    M[4*3+2] = -1.0f;
    M[4*3+3] =  0.0f;

    GLUP::transpose_matrix(M);
    glupMultMatrixf(M);
} 

void glupPerspective(
    GLUPdouble fovy, GLUPdouble aspect,
    GLUPdouble zNear, GLUPdouble zFar
) {
    GEO_CHECK_GL();
    
    GLfloat M[16];
    
    double f = 1.0 / tan(fovy * M_PI / 180.0);

    M[4*0+0] = GLfloat(f / aspect);
    M[4*0+1] = 0.0f;
    M[4*0+2] = 0.0f;
    M[4*0+3] = 0.0f;

    M[4*1+0] = 0.0f;
    M[4*1+1] = GLfloat(f);
    M[4*1+2] = 0.0f;
    M[4*1+3] = 0.0f;

    M[4*2+0] = 0.0f;
    M[4*2+1] = 0.0f;
    M[4*2+2] = GLfloat((zFar+zNear)/(zNear-zFar));
    M[4*2+3] = GLfloat(2.0*zFar*zNear/(zNear-zFar));

    M[4*3+0] =  0.0f;
    M[4*3+1] =  0.0f;
    M[4*3+2] = -1.0f;
    M[4*3+3] =  0.0f;

    GLUP::transpose_matrix(M);    
    glupMultMatrixf(M);    
}

GLUPint glupProject(
    GLUPdouble objx, GLUPdouble objy, GLUPdouble objz,
    const GLUPdouble modelMatrix[16],
    const GLUPdouble projMatrix[16],
    const GLUPint viewport[4],
    GLUPdouble* winx, GLUPdouble* winy, GLUPdouble* winz
) {
    GEO_CHECK_GL();
    
    double in[4];
    double out[4];

    in[0]=objx;
    in[1]=objy;
    in[2]=objz;
    in[3]=1.0;

    GLUP::mult_transpose_matrix_vector(out, modelMatrix, in);
    GLUP::mult_transpose_matrix_vector(in, projMatrix, out);

    if (in[3] == 0.0) {
        return(GL_FALSE);
    }
    in[0] /= in[3];
    in[1] /= in[3];
    in[2] /= in[3];

    // Map x, y and z to range 0-1 */
    in[0] = in[0] * 0.5 + 0.5;
    in[1] = in[1] * 0.5 + 0.5;
    in[2] = in[2] * 0.5 + 0.5;

    // Map x,y to viewport 
    in[0] = in[0] * viewport[2] + viewport[0];
    in[1] = in[1] * viewport[3] + viewport[1];

    *winx=in[0];
    *winy=in[1];
    *winz=in[2];
    return(GL_TRUE);
}

GLUPboolean glupUnProject(
    GLUPdouble winx, GLUPdouble winy, GLUPdouble winz,
    const GLUPdouble modelMatrix[16],
    const GLUPdouble projMatrix[16],
    const GLUPint viewport[4],
    GLUPdouble *objx, GLUPdouble *objy, GLUPdouble *objz
) {
    GEO_CHECK_GL();
    
    double modelviewproject[16];
    double modelviewproject_inv[16];
    GLUP::mult_matrices(modelviewproject, modelMatrix, projMatrix);
    if(!GLUP::invert_matrix(modelviewproject_inv, modelviewproject)) {
        return GL_FALSE;
    }

    double in[4];
    in[0] = winx;
    in[1] = winy;
    in[2] = winz;
    in[3] = 1.0;

    // Invert viewport transform
    in[0] = (in[0] - double(viewport[0])) / double(viewport[2]);
    in[1] = (in[1] - double(viewport[1])) / double(viewport[3]);

    // Map to [-1, 1]
    in[0] = in[0] * 2.0 - 1.0;
    in[1] = in[1] * 2.0 - 1.0;
    in[2] = in[2] * 2.0 - 1.0;

    double out[4];
    GLUP::mult_transpose_matrix_vector(out, modelviewproject_inv, in);

    if(out[3] == 0.0) {
        return GL_FALSE;
    }

    *objx = out[0] / out[3];
    *objy = out[1] / out[3];
    *objz = out[2] / out[3];
    
    return GL_TRUE;
}

GLUPboolean glupInvertMatrixfv(
    GLUPfloat Minvert[16],    
    const GLUPfloat M[16]
) {
    GEO_CHECK_GL();
    
    return GLUP::invert_matrix(Minvert, M);
}

GLUPboolean glupInvertMatrixdv(
    GLUPdouble Minvert[16],    
    const GLUPdouble M[16]
) {
    GEO_CHECK_GL();
        
    return GLUP::invert_matrix(Minvert, M);
}



/******************* Drawing ***************************/

void glupDrawArrays(
    GLUPprimitive primitive, GLUPint first, GLUPsizei count
) {
    GEO_CHECK_GL();
    
    GLUP::current_context_->draw_arrays(
        primitive, first, count
    );

    GEO_CHECK_GL();    
}
    
void glupDrawElements(
    GLUPprimitive primitive, GLUPsizei count,
    GLUPenum type, const GLUPvoid* indices
) {
    GEO_CHECK_GL();
    
    GLUP::current_context_->draw_elements(
        primitive, count, type, indices
    );

    GEO_CHECK_GL();    
}

void glupBegin(GLUPprimitive primitive) {
    GEO_CHECK_GL();    
    GLUP::current_context_->begin(primitive);
    GEO_CHECK_GL();    
}

void glupEnd() {
    GEO_CHECK_GL();    
    GLUP::current_context_->end();
    GEO_CHECK_GL();    
}

void glupVertex2fv(const GLUPfloat* xy) {
    GEO_CHECK_GL();    
    GLUP::current_context_->immediate_vertex(xy[0], xy[1]);
}

void glupVertex3fv(const GLUPfloat* xyz) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_vertex(xyz[0], xyz[1], xyz[2]);    
}

void glupVertex4fv(const GLUPfloat* xyzw) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_vertex(
        xyzw[0], xyzw[1], xyzw[2], xyzw[3]
    );        
}

void glupVertex2dv(const GLUPdouble* xy) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_vertex(
        GLfloat(xy[0]),
        GLfloat(xy[1])
    );
}

void glupVertex3dv(const GLUPdouble* xyz) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_vertex(
        GLfloat(xyz[0]),
        GLfloat(xyz[1]),
        GLfloat(xyz[2])        
    );
}

void glupVertex4dv(const GLUPdouble* xyzw) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_vertex(
        GLfloat(xyzw[0]),
        GLfloat(xyzw[1]),
        GLfloat(xyzw[2]),
        GLfloat(xyzw[3])                
    );
}

void glupVertex2f(GLUPfloat x, GLUPfloat y) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_vertex(x,y);    
}        

void glupVertex3f(GLUPfloat x, GLUPfloat y, GLUPfloat z) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_vertex(x,y,z);        
}    

void glupVertex4f(GLUPfloat x, GLUPfloat y, GLUPfloat z, GLUPfloat w) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_vertex(x,y,z,w);            
}

void glupVertex2d(GLUPdouble x, GLUPdouble y) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_vertex(
        GLfloat(x),
        GLfloat(y)
    );
}        

void glupVertex3d(GLUPdouble x, GLUPdouble y, GLUPdouble z) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_vertex(
        GLfloat(x),
        GLfloat(y),
        GLfloat(z)
    );
}    

void glupVertex4d(GLUPdouble x, GLUPdouble y, GLUPdouble z, GLUPdouble w) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_vertex(
        GLfloat(x),
        GLfloat(y),
        GLfloat(z),
        GLfloat(w)                
    );
}

void glupColor3fv(const GLUPfloat* rgb) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_color(rgb[0], rgb[1], rgb[2]);
}

void glupColor4fv(const GLUPfloat* rgba) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_color(rgba[0], rgba[1], rgba[2], rgba[3]);
}

void glupColor3dv(const GLUPdouble* rgb) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_color(
        GLfloat(rgb[0]),
        GLfloat(rgb[1]),
        GLfloat(rgb[2])
    );    
}

void glupColor4dv(const GLUPdouble* rgba) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_color(
        GLfloat(rgba[0]),
        GLfloat(rgba[1]),
        GLfloat(rgba[2]),
        GLfloat(rgba[3])        
    );    
}

void glupColor3f(GLUPfloat r, GLUPfloat g, GLUPfloat b) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_color(r, g, b);    
}    

void glupColor4f(GLUPfloat r, GLUPfloat g, GLUPfloat b, GLUPfloat a) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_color(r, g, b, a);        
}

void glupColor3d(GLUPdouble r, GLUPdouble g, GLUPdouble b) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_color(
        GLfloat(r),
        GLfloat(g),
        GLfloat(b)
    );    
}    

void glupColor4d(GLUPdouble r, GLUPdouble g, GLUPdouble b, GLUPdouble a) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_color(
        GLfloat(r),
        GLfloat(g),
        GLfloat(b),
        GLfloat(a)
    );    
}

void glupTexCoord2fv(const GLUPfloat* st) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_tex_coord(st[0], st[1]);    
}

void glupTexCoord3fv(const GLUPfloat* stu) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_tex_coord(stu[0], stu[1], stu[2]);        
}

void glupTexCoord4fv(const GLUPfloat* stuv) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_tex_coord(
        stuv[0], stuv[1], stuv[2], stuv[3]
    );            
}

void glupTexCoord2dv(const GLUPdouble* st) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_tex_coord(
        GLfloat(st[0]),
        GLfloat(st[1])
    );            
}

void glupTexCoord3dv(const GLUPdouble* stu) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_tex_coord(
        GLfloat(stu[0]),
        GLfloat(stu[1]),
        GLfloat(stu[2])
    );            
}

void glupTexCoord4dv(const GLUPdouble* stuv) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_tex_coord(
        GLfloat(stuv[0]),
        GLfloat(stuv[1]),
        GLfloat(stuv[2]),
        GLfloat(stuv[3])
    );            
}

void glupTexCoord1f(GLUPfloat s) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_tex_coord(s);        
}

void glupTexCoord2f(GLUPfloat s, GLUPfloat t) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_tex_coord(s,t);        
}        

void glupTexCoord3f(GLUPfloat s, GLUPfloat t, GLUPfloat u) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_tex_coord(s,t,u);        
}    

void glupTexCoord4f(GLUPfloat s, GLUPfloat t, GLUPfloat u, GLUPfloat v) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_tex_coord(s,t,u,v);    
}

void glupTexCoord1d(GLUPdouble s) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_tex_coord(
        GLfloat(s)
    );        
}            

void glupTexCoord2d(GLUPdouble s, GLUPdouble t) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_tex_coord(
        GLfloat(s),
        GLfloat(t)
    );        
}        

void glupTexCoord3d(GLUPdouble s, GLUPdouble t, GLUPdouble u) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_tex_coord(
        GLfloat(s),
        GLfloat(t),
        GLfloat(u)
    );        
}    

void glupTexCoord4d(GLUPdouble s, GLUPdouble t, GLUPdouble u, GLUPdouble v) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_tex_coord(
        GLfloat(s),
        GLfloat(t),
        GLfloat(u),
        GLfloat(v)
    );        
}


void glupNormal3fv(GLUPfloat* xyz) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_normal(
        xyz[0],xyz[1],xyz[2]
    );
}

void glupNormal3f(GLUPfloat x, GLUPfloat y, GLUPfloat z) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_normal(
        x,y,z
    );
}    

void glupNormal3dv(GLUPdouble* xyz) {
    GEO_CHECK_GL();            
    GLUP::current_context_->immediate_normal(
        GLfloat(xyz[0]),
	GLfloat(xyz[1]),
	GLfloat(xyz[2])
    );
}

void glupNormal3d(GLUPdouble x, GLUPdouble y, GLUPdouble z) {
    GEO_CHECK_GL();        
    GLUP::current_context_->immediate_normal(
        GLfloat(x),
        GLfloat(y),
        GLfloat(z)
    );
}    

void glupUseProgram(GLUPuint program) {
    GEO_CHECK_GL();    
    GLUP::current_context_->set_user_program(program);
}



/****************************************************************************/

namespace GLUP {
    
    static GLUPuint vertex_array_binding = 0;
    static GLint max_vertex_attrib = 0;

    /**
     * \brief If true, then GLUP uses its own implementation
     *  of Vertex Array Object. 
     * \details This is for instance required
     *  when using GLUP with Emscripten, in a brower that does
     *  not have the Vertex Array Object extension (see
     *  Context_GLES.cpp)
     */
    bool vertex_array_emulate = false;
    
    /**
     * \brief Stores the state of a vertex attribute
     *  binding.
     * \details Used to implement emulated vertex array
     *  objects.
     * \see VertexArrayObject.
     */
    class VertexAttribBinding {
        
    public:
        
        /**
         * \brief VertexAttribBinding constructor.
         */
        VertexAttribBinding() :
            enabled(GL_FALSE),
            size(0),
            type(GL_FLOAT),
            normalized(GL_FALSE),
            stride(0),
            pointer(nullptr),
            buffer_binding(0) {
        }

        /**
         * \brief Copies the binding of a vertex attribute
         *  from OpenGL state to this VertexAttribBinding.
         * \param[in] index the index of the attribute
         */
        void copy_from_GL(GLuint index) {

            // From the spec, glGetVertexAttribiv is supposed
            // to return 4 values always, even if we are only
            // interested in the first one, I guess it is needed
            // to read in a buffer with enough space for 4 values.
            GLint buff[4];

            glGetVertexAttribiv(
                index, GL_VERTEX_ATTRIB_ARRAY_ENABLED, buff
            );
            enabled = buff[0];

            //   With some webbrowsers, querying a vertex attrib array
            // that is not enabled returns the default values instead
            // of the actual values.
            if(!enabled) {
                glEnableVertexAttribArray(index);
            }
            
            glGetVertexAttribiv(
                index, GL_VERTEX_ATTRIB_ARRAY_SIZE, buff
            );
            size = buff[0];

            glGetVertexAttribiv(
                index, GL_VERTEX_ATTRIB_ARRAY_TYPE, buff
            );
            type = buff[0];

            glGetVertexAttribiv(
                index, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, buff
            );
            normalized = buff[0];
            
            glGetVertexAttribiv(
                index, GL_VERTEX_ATTRIB_ARRAY_STRIDE, buff
            );
            stride = buff[0];

            glGetVertexAttribPointerv(
                index, GL_VERTEX_ATTRIB_ARRAY_POINTER, &pointer
            );
            
            glGetVertexAttribiv(
                index, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, buff
            );
            buffer_binding = buff[0];

            if(!enabled) {
                glDisableVertexAttribArray(index);
            }
        }

        /**
         * \brief Copies the binding stored in this VertexAttribBinding
         *  into OpenGL.
         * \param[in] index the index of the attribute where the binding
         *  should be copied.
         */
        void copy_to_GL(GLuint index) {
            if(enabled) {
                glEnableVertexAttribArray(index);
            } else {
                glDisableVertexAttribArray(index);            
            }
            glBindBuffer(GL_ARRAY_BUFFER, GLuint(buffer_binding));
            if(buffer_binding != 0 || pointer != nullptr ) {
                glVertexAttribPointer(
                    index, size, GLenum(type),
                    GLboolean(normalized), GLsizei(stride), pointer
                );
            }
        }

        /**
         * \brief Resets all the stored bindings to default
         *  values.
         */
        void reset() {
            enabled=GL_FALSE;
            size=0;
            type=GL_FLOAT;
            normalized=GL_FALSE;
            stride=0;
            pointer=nullptr;
            buffer_binding=0; 
        }
        
    private:
        GLint enabled;
        GLint size;
        GLint type;
        GLint normalized;
        GLint stride;
        GLvoid* pointer;
        GLint buffer_binding;
    };
    
    /**
     * \brief Emulates vertex array objects if not supported
     *  by OpenGL implementation.
     */
    class VertexArrayObject {
    public:
        /**
         * \brief The maximum number of vertex attributes
         *  that we save in a VAO. 
         * \details For GLUP, only 4 are needed. Can be
         *  increased if need be.
         */
        enum {MAX_VERTEX_ATTRIB = 4};

        /**
         * \brief VertexArrayObject constructor.
         */
        VertexArrayObject() :
            element_array_buffer_binding_(0) {
            if(max_vertex_attrib == 0) {
                glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vertex_attrib);
                max_vertex_attrib = std::min(
                    max_vertex_attrib, GLint(MAX_VERTEX_ATTRIB)
                );
            }
        }

        /**
         * \brief Binds this VertexArrayObject.
         * \details This copies the stored element array and vertex attribute
         *  bindings to OpenGL.
         */
        void bind() {
            glBindBuffer(
                GL_ELEMENT_ARRAY_BUFFER, GLuint(element_array_buffer_binding_)
            );
            for(GLint i=0; i<max_vertex_attrib; ++i) {
                attrib_binding_[i].copy_to_GL(GLuint(i));
            }
        }

        /**
         * \brief Unbinds this VertexArrayObject.
         * \details This copies the currently bound element array and 
         *  vertex attribute bindings from OpenGL to this VertexArrayObject.
         */
        void unbind() {
            // Note: In Emscripten, glGetIntegerv() does not suffer from the
            // same bug as glGetVertexAttribiv(
            //   ..., GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, ...
            // ), therefore there is no special case here.
            glGetIntegerv(
                GL_ELEMENT_ARRAY_BUFFER_BINDING,
                &element_array_buffer_binding_
            );
            for(GLint i=0; i<max_vertex_attrib; ++i) {
                attrib_binding_[i].copy_from_GL(GLuint(i));
            }
        }

        /**
         * \brief Resets all the stored bindings to default
         *  values.
         */
        void reset() {
            element_array_buffer_binding_ = 0;
            for(GLint i=0; i<max_vertex_attrib; ++i) {
                attrib_binding_[i].reset();
            }
        }
        
    private:
        VertexAttribBinding attrib_binding_[MAX_VERTEX_ATTRIB];
        GLint element_array_buffer_binding_;
    };

    
    /**
     * \brief Manages the emulated vertex array objects.
     */
    class VertexArrayObjectAllocator {
    public:
        /**
         * \brief VertexArrayObjectAllocator constructor.
         */
        VertexArrayObjectAllocator() {
            // Create the dummy slot 0.
            slots_.push_back(Slot());            
        }

        /**
         * \brief VertexArrayObjectAllocator destructor.
         */
        ~VertexArrayObjectAllocator() {
            for(index_t i=0; i<slots_.size(); ++i) {
                if(slots_[i].VAO != nullptr) {
                    delete slots_[i].VAO;
                    slots_[i].VAO = nullptr;
                }
            }
        }

        /**
         * \brief Creates a new vertex array object.
         * \return the index of the newly created vertex
         *  array object.
         */
        index_t new_VAO() {
            index_t result = 0;
            if(first_free_ != 0) {
                result = first_free_;
                first_free_ = slots_[first_free_].next;
                slots_[result].VAO->reset();
            } else {
                slots_.push_back(Slot());
                result = index_t(slots_.size()-1);
                slots_[result].VAO = new VertexArrayObject();
            }
            return result;
        }

        /**
         * \brief Deletes a vertex array object.
         * \details Vertex array objects are recycled internally.
         * \param[in] VAOindex the index of the vertex
         *  array object to delete.
         */
        void delete_VAO(index_t VAOindex) {
            slots_[VAOindex].next = first_free_;
            first_free_ = VAOindex;
        }

        /**
         * \brief Gets a vertex array object by index.
         * \param[in] VAOindex the index of the vertex array object.
         * \return a pointer to the vertex array object.
         */
        VertexArrayObject* get_VAO(index_t VAOindex) {
            return slots_[VAOindex].VAO;
        }
        
    private:

        /**
         * \brief The information attached to each vertex
         *  array object index.
         */
        struct Slot {
            /**
             * \brief Slot constructor.
             */
            Slot() :
                VAO(nullptr), next(0) {
            }

            /**
             * \brief A pointer to the internal representation.
             */
            VertexArrayObject* VAO;

            /**
             * \brief The index of the next free element, used
             *  to have constant-time allocation and deallocation.
             */
            index_t next;
        };

        vector<Slot> slots_;

        /**
         * \brief The head of the free list.
         */
        GLUPuint first_free_;
    };

    static VertexArrayObjectAllocator VAO_allocator;

}

// TODO1: ArraysOES()/Arrays() switch based on OpenGL profile
// (runtime) rather than GEO_OS_EMSCRIPTEN macro (compile-time)

void glupGenVertexArrays(GLUPsizei n, GLUPuint* arrays) {
    if(GLUP::vertex_array_emulate) {
        for(GLUPsizei i=0; i<n; ++i) {
            arrays[i] = GLUP::VAO_allocator.new_VAO();
        }

    } else {
#ifdef GEO_OS_EMSCRIPTEN
        glGenVertexArraysOES(n, arrays);
#else
	if(!glGenVertexArrays) {
	    GLUP::vertex_array_emulate = true;
	    glupGenVertexArrays(n,arrays);
	    return;
	}
        glGenVertexArrays(n, arrays);
#endif        
    }
}

void glupDeleteVertexArrays(GLUPsizei n, const GLUPuint *arrays) {
    if(GLUP::vertex_array_emulate) {
        for(GLUPsizei i=0; i<n; ++i) {
            GLUP::VAO_allocator.delete_VAO(arrays[i]);
        }
    } else {
#if defined(GEO_OS_EMSCRIPTEN) 
        glDeleteVertexArraysOES(n, arrays);        
#else
	if(!glDeleteVertexArrays) {
	    GLUP::vertex_array_emulate = true;
	    glupDeleteVertexArrays(n,arrays);
	    return;
	}
        glDeleteVertexArrays(n, arrays);
#endif        
    }
}

void glupBindVertexArray(GLUPuint array) {
    if(GLUP::vertex_array_emulate) {
        if(array != GLUP::vertex_array_binding) {
            if(GLUP::vertex_array_binding != 0) {
                GLUP::VAO_allocator.get_VAO(
                    GLUP::vertex_array_binding
                )->unbind();
            }
            GLUP::vertex_array_binding = array;
            if(GLUP::vertex_array_binding != 0) {
                GLUP::VAO_allocator.get_VAO(
                    GLUP::vertex_array_binding
                )->bind();
            }
        }
    } else {
#if defined(GEO_OS_EMSCRIPTEN) 
        glBindVertexArrayOES(array);        
#else
	if(!glBindVertexArray) {
	    GLUP::vertex_array_emulate = true;
	    glupBindVertexArray(array);
	    return;
	}
        glBindVertexArray(array);
#endif
        GLUP::vertex_array_binding = array;        
    }
}

GLUPuint glupGetVertexArrayBinding() {
    return GLUP::vertex_array_binding;
}

/****************************************************************************/
