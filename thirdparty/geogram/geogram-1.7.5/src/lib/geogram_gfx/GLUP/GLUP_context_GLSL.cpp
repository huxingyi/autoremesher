/*
 *  Copyright (c) 2012-2016, Bruno Levy
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

#include <geogram_gfx/GLUP/GLUP_context_GLSL.h>
#include <geogram_gfx/basic/GLSL.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/progress.h>
#include <geogram/basic/logger.h>

#ifdef GEO_GL_150

namespace GLUP {
    using namespace GEO;

    Context_GLSL150::Context_GLSL150() {
    }
    
    void Context_GLSL150::setup() {
        Context::setup();
        marching_tet_.create_UBO();        
        marching_hex_.create_UBO();        
        marching_prism_.create_UBO();
        marching_pyramid_.create_UBO();
        marching_connector_.create_UBO();
    }
    
    const char* Context_GLSL150::profile_name() const {
        return "GLUP150";
    }

    /****************** Primitives implementation *************************/
    
    void Context_GLSL150::setup_GLUP_POINTS() {
        if(!use_core_profile_) {
#ifdef GL_POINT_SPRITE	    
            glEnable(GL_POINT_SPRITE);
#endif	    
        }
        set_primitive_info(
            GLUP_POINTS, GL_POINTS,
            GLSL::compile_program_with_includes_no_link(
                this,
                "//stage GL_VERTEX_SHADER\n"
                "//import <GLUPGLSL/points_vertex_shader.h>\n",
                "//stage GL_FRAGMENT_SHADER\n"
                "//import <GLUPGLSL/points_fragment_shader.h>\n"                
            )
        );
    }

    void Context_GLSL150::setup_GLUP_LINES() {
        set_primitive_info(
            GLUP_LINES, GL_LINES,
            GLSL::compile_program_with_includes_no_link(
                this,
                "//stage GL_VERTEX_SHADER\n"
                "//import <GLUPGLSL/vertex_shader.h>\n",
                "//stage GL_FRAGMENT_SHADER\n"
                "//import <GLUPGLSL/lines_fragment_shader.h>\n"                
            )
        );
    }

    void Context_GLSL150::setup_GLUP_TRIANGLES() {
        set_primitive_info(
            GLUP_TRIANGLES, GL_TRIANGLES,
            GLSL::compile_program_with_includes_no_link(
                this,
                "//stage GL_VERTEX_SHADER\n"
                "//import <GLUPGLSL/vertex_shader.h>\n",
                "//stage GL_FRAGMENT_SHADER\n"
                "//import <GLUPGLSL/fragment_shader.h>\n"
#ifndef GEO_OS_ANDROID
		,
                "//stage GL_GEOMETRY_SHADER\n"
                "//import <GLUPGLSL/geometry_shader_preamble.h>\n"
                "void main() {\n"
                "    gl_PrimitiveID = gl_PrimitiveIDIn;\n"
                "    get_vertices();\n"
                "    draw_triangle(0,1,2,true);\n"
                "}\n"
#endif		
            )
       );
    }

    void Context_GLSL150::setup_GLUP_QUADS() {
        set_primitive_info(
            GLUP_QUADS, GL_LINES_ADJACENCY,
            GLSL::compile_program_with_includes_no_link(
                this,
                "//stage GL_VERTEX_SHADER\n"
                "//import <GLUPGLSL/vertex_shader.h>\n",
                "//stage GL_FRAGMENT_SHADER\n"
                "//import <GLUPGLSL/fragment_shader.h>\n"
#ifndef GEO_OS_ANDROID
		,
                "//stage GL_GEOMETRY_SHADER\n"
                "//import <GLUPGLSL/geometry_shader_preamble.h>\n"
                "void main() {\n"
                "    gl_PrimitiveID = gl_PrimitiveIDIn;\n"
                "    get_vertices();\n"
                "    draw_quad(0,1,3,2,true);\n"
                "}\n"
#endif				
            )
        );
    }

    void Context_GLSL150::setup_GLUP_TETRAHEDRA() {
        GLuint program = GLSL::compile_program_with_includes_no_link(
            this,
            "//stage GL_VERTEX_SHADER\n"
            "//import <GLUPGLSL/vertex_shader.h>\n",
            "//stage GL_FRAGMENT_SHADER\n"
            "//import <GLUPGLSL/fragment_shader.h>\n",
            "//stage GL_GEOMETRY_SHADER\n"
            "//import <GLUPGLSL/geometry_shader_preamble.h>\n"
            "//import <GLUPGLSL/marching_cells.h>\n"
            "void main() {\n"
            "if(draw_clipped_cell()) { return; } \n"
            "emit_vertex(2, vec4(1.0, 0.0, 0.0, 0.0), compute_clip_coords());\n"
            "emit_vertex(0, vec4(0.0, 1.0, 0.0, 0.0), compute_clip_coords());\n"
            "emit_vertex(1, vec4(0.0, 0.0, 1.0, 0.0), compute_clip_coords());\n"
            "emit_vertex(3, vec4(1.0, 0.0, 0.0, 0.0), compute_clip_coords());\n"
            "EndPrimitive();\n"
            "emit_vertex(0, vec4(1.0, 0.0, 0.0, 0.0), compute_clip_coords());\n"
            "emit_vertex(2, vec4(0.0, 1.0, 0.0, 0.0), compute_clip_coords());\n"
            "emit_vertex(3, vec4(0.0, 0.0, 1.0, 0.0), compute_clip_coords());\n"
            "emit_vertex(1, vec4(1.0, 0.0, 0.0, 0.0), compute_clip_coords());\n"
            "}\n"
        );
        set_primitive_info(GLUP_TETRAHEDRA, GL_LINES_ADJACENCY,program);
        marching_tet_.bind_uniform_state(program);
    }

    void Context_GLSL150::setup_GLUP_CONNECTORS() {
        GLuint program = GLSL::compile_program_with_includes_no_link(
            this,
            "//stage GL_VERTEX_SHADER\n"
            "//import <GLUPGLSL/vertex_shader.h>\n",
            "//stage GL_FRAGMENT_SHADER\n"
            "//import <GLUPGLSL/fragment_shader.h>\n",
            "//stage GL_GEOMETRY_SHADER\n"
            "//import <GLUPGLSL/geometry_shader_preamble.h>\n"
            "//import <GLUPGLSL/marching_cells.h>\n"
            "void main() {\n"
            "    if(draw_clipped_cell()) { return; } \n"
            "    draw_quad(0,1,3,2,compute_clip_coords());\n"
            "    draw_triangle(2,1,0,compute_clip_coords());\n"
            "    draw_triangle(3,2,0,compute_clip_coords());\n"
            "}\n"
        );
        set_primitive_info(GLUP_CONNECTORS, GL_LINES_ADJACENCY, program);
        marching_connector_.bind_uniform_state(program);
    }
    
    void Context_GLSL150::setup_GLUP_PRISMS() {
        GLuint program = GLSL::compile_program_with_includes_no_link(
            this,
            "//stage GL_VERTEX_SHADER\n"
            "//import <GLUPGLSL/vertex_shader.h>\n",
            "//stage GL_FRAGMENT_SHADER\n"
            "//import <GLUPGLSL/fragment_shader.h>\n",
            "//stage GL_GEOMETRY_SHADER\n"
            "//import <GLUPGLSL/geometry_shader_preamble.h>\n"
            "//import <GLUPGLSL/marching_cells.h>\n"
            "void main() {\n"
            "    if(draw_clipped_cell()) { return; } \n"
            "    draw_triangle(0,1,2,compute_clip_coords());\n"
            "    draw_triangle(5,4,3,compute_clip_coords());\n"
            "    draw_quad(0,3,1,4,compute_clip_coords());\n"
            "    draw_quad(0,2,3,5,compute_clip_coords());\n"
            "    draw_quad(1,4,2,5,compute_clip_coords());\n" 
            "}\n"
        );
        set_primitive_info(GLUP_PRISMS, GL_TRIANGLES_ADJACENCY, program);
        marching_prism_.bind_uniform_state(program);
    }

    void Context_GLSL150::setup_GLUP_HEXAHEDRA() {
        GLuint program = GLSL::compile_program_with_includes_no_link(
            this,
            "//stage GL_VERTEX_SHADER\n"
            "//import <GLUPGLSL/gather_vertex_shader.h>\n",
            "//stage GL_FRAGMENT_SHADER\n"
            "//import <GLUPGLSL/fragment_shader.h>\n",
            "//stage GL_GEOMETRY_SHADER\n"
            "//import <GLUPGLSL/geometry_shader_preamble.h>\n"
            "//import <GLUPGLSL/marching_cells.h>\n"
            "void main() {\n"
            "if(draw_clipped_cell()) { return; } \n"
            "emit_vertex(6,vec4(0.0,1.0,0.0,1.0),compute_clip_coords());\n"
            "emit_vertex(7,vec4(1.0,0.0,0.0,1.0),compute_clip_coords());\n"
            "emit_vertex(4,vec4(0.0,1.0,1.0,0.0),compute_clip_coords());\n"
            "emit_vertex(5,vec4(1.0,0.0,1.0,0.0),compute_clip_coords());\n"
            "emit_vertex(0,vec4(0.0,1.0,0.0,1.0),compute_clip_coords());\n"
            "emit_vertex(1,vec4(1.0,0.0,0.0,1.0),compute_clip_coords());\n"
            "emit_vertex(2,vec4(0.0,1.0,1.0,0.0),compute_clip_coords());\n"
            "emit_vertex(3,vec4(1.0,0.0,1.0,0.0),compute_clip_coords());\n"
            "emit_vertex(6,vec4(0.0,1.0,0.0,1.0),compute_clip_coords());\n"
            "emit_vertex(7,vec4(1.0,0.0,0.0,1.0),compute_clip_coords());\n"
            "EndPrimitive();\n"            
            "draw_quad(4,0,6,2,compute_clip_coords());\n"
            "draw_quad(1,5,3,7,compute_clip_coords());\n"            
            "}\n"
        );
        set_primitive_info_vertex_gather_mode(
            GLUP_HEXAHEDRA, GL_LINES_ADJACENCY, program
        );
        marching_hex_.bind_uniform_state(program);
    }
    
    void Context_GLSL150::setup_GLUP_PYRAMIDS() {
        GLuint program = GLSL::compile_program_with_includes_no_link(
            this,
            "//stage GL_VERTEX_SHADER\n"
            "//import <GLUPGLSL/gather_vertex_shader.h>\n",
            "//stage GL_FRAGMENT_SHADER\n"
            "//import <GLUPGLSL/fragment_shader.h>\n",
            "//stage GL_GEOMETRY_SHADER\n"
            "//import <GLUPGLSL/geometry_shader_preamble.h>\n"
            "//import <GLUPGLSL/marching_cells.h>\n"
            "void main() {\n"
            "    if(draw_clipped_cell()) { return; }\n"
            "    draw_quad(0,1,3,2,compute_clip_coords());\n"
            "    draw_triangle(0,4,1,compute_clip_coords());\n"
            "    draw_triangle(0,3,4,compute_clip_coords());\n"
            "    draw_triangle(2,4,3,compute_clip_coords());\n"
            "    draw_triangle(2,1,4,compute_clip_coords());\n"
            "}\n"
        );
        GEO_CHECK_GL();    	
        set_primitive_info_vertex_gather_mode(
            GLUP_PYRAMIDS, GL_POINTS, program
        );
        GEO_CHECK_GL();    	
        marching_pyramid_.bind_uniform_state(program);
        GEO_CHECK_GL();    	
    }

    void Context_GLSL150::setup_GLUP_SPHERES() {
        set_primitive_info(
            GLUP_SPHERES, GL_POINTS,
            GLSL::compile_program_with_includes_no_link(
                this,
                "//stage GL_VERTEX_SHADER\n"
                "//import <GLUPGLSL/spheres_vertex_shader.h>\n",
                "//stage GL_FRAGMENT_SHADER\n"
                "//import <GLUPGLSL/spheres_fragment_shader.h>\n"
            )
        );
    }
    
    /******************* pseudo-files ******************************/
    
    static void OES_extensions(std::vector<GLSL::Source>& sources) {
        // To be checked: seems that these functionalities are
	// standard with OpenGL ES 3.0 and greater (which we
	// imply because we use here in/out instead of attribute)
#ifndef GEO_OS_ANDROID
        sources.push_back(
            "#ifdef GL_ES\n"
            "  #extension GL_OES_texture_3D : enable \n"
            "  #extension GL_OES_standard_derivatives : enable \n"
            "  #extension GL_OES_geometry_shader : enable \n"
            "  #extension GL_OES_tessellation_shader : enable \n"
            "#endif\n"
        );
#endif	
    }

    void Context_GLSL150::get_vertex_shader_preamble_pseudo_file(
        std::vector<GLSL::Source>& sources
    ) {
#if defined(GEO_OS_ANDROID)
        sources.push_back("#version 320 es\n");
	sources.push_back("precision lowp sampler3D;\n");
	sources.push_back("precision highp float;\n");
	sources.push_back("precision highp int;\n");		
	sources.push_back("#define GLUP_NO_GL_CLIPPING\n");
#elif defined(GEO_OS_APPLE)
        sources.push_back("#version 150\n");
#else	
        sources.push_back("#version 150 core\n");        
#endif
        sources.push_back("#define GLUP_VERTEX_SHADER\n");
        OES_extensions(sources);
    }
   
    void Context_GLSL150::get_fragment_shader_preamble_pseudo_file(
        std::vector<GLSL::Source>& sources
    ) {
#if defined(GEO_OS_ANDROID)
        sources.push_back("#version 320 es\n");
	sources.push_back("precision lowp sampler3D;\n");
	sources.push_back("precision highp float;\n");
	sources.push_back("precision highp int;\n");		
	sources.push_back("#define GLUP_NO_GL_CLIPPING\n");	
#elif defined(GEO_OS_APPLE)
        sources.push_back("#version 150\n");
#else	
        sources.push_back("#version 150 core\n");        
#endif
	
        sources.push_back(
            "#define GLUP_FRAGMENT_SHADER\n"
	    "#ifndef GL_ES\n"
	    "#extension GL_ARB_conservative_depth : enable\n"
	    "#endif\n"
        );
	
        OES_extensions(sources);        
    }

    void Context_GLSL150::get_geometry_shader_layout(
        std::vector<GLSL::Source>& sources                        
    ) {
	switch(primitive_source_) {
        case GLUP_POINTS:
        case GLUP_LINES:
        case GLUP_SPHERES:
            break;
        case GLUP_TRIANGLES:
            sources.push_back(
                "layout(triangles) in;\n"
                "layout(triangle_strip, max_vertices = 3) out;\n"
            );
            break;
        case GLUP_QUADS:
            sources.push_back(
                "layout(lines_adjacency) in;\n"
                "layout(triangle_strip, max_vertices = 4) out;\n"
            );
            break;
        case GLUP_TETRAHEDRA:
            sources.push_back(
                "layout(lines_adjacency) in;\n"
                "layout(triangle_strip, max_vertices = 8) out;\n"
            );
            break;
        case GLUP_HEXAHEDRA:
            sources.push_back(
                "layout(lines_adjacency) in;\n"
                "layout(triangle_strip, max_vertices = 18) out;\n"
            );
            break;
        case GLUP_PRISMS:
            sources.push_back(
                "layout(triangles_adjacency) in;\n"
                "layout(triangle_strip, max_vertices = 24) out;\n"
            );
            break;
        case GLUP_PYRAMIDS:
            sources.push_back(
                "layout(points) in;\n"
                "layout(triangle_strip, max_vertices = 28) out;\n"
            );
            break;
        case GLUP_CONNECTORS:
            sources.push_back(
                "layout(lines_adjacency) in;\n"
                "layout(triangle_strip, max_vertices = 12) out;\n"
            );
            break;
        case GLUP_NB_PRIMITIVES:
            geo_assert_not_reached;
        }
    }
    
    void Context_GLSL150::get_geometry_shader_preamble_pseudo_file(
        std::vector<GLSL::Source>& sources
    ) {
#if defined(GEO_OS_ANDROID)
        sources.push_back("#version 320 es\n");
	sources.push_back("precision lowp sampler3D;\n");
	sources.push_back("precision highp float;\n");
	sources.push_back("precision highp int;\n");		
	sources.push_back("#define GLUP_NO_GL_CLIPPING\n");	
#elif defined(GEO_OS_APPLE)
        sources.push_back("#version 150\n");
#else	
        sources.push_back("#version 150 core\n");        
#endif
        sources.push_back("#define GLUP_GEOMETRY_SHADER\n");
        OES_extensions(sources);

        get_geometry_shader_layout(sources);
    }

    void Context_GLSL150::get_primitive_pseudo_file(
        std::vector<GLSL::Source>& sources
    ) {
        Context::get_primitive_pseudo_file(sources);
        if(primitive_source_ == GLUP_HEXAHEDRA) {
            sources.push_back("#define GLUP_VERTEX_GATHER\n");
            sources.push_back(
                "const int glup_nb_vertices_per_GL_v=2;\n"
            );
        } else if(primitive_source_ == GLUP_PYRAMIDS) {
            sources.push_back("#define GLUP_VERTEX_GATHER\n");
            sources.push_back(
                "const int glup_nb_vertices_per_GL_v=5;\n"
            );
        }
    }
    
    /****** GLUP440 implementation *****************************************/

    Context_GLSL440::Context_GLSL440() {
        use_tessellation_ =
            GEO::CmdLine::get_arg_bool("gfx:GLSL_tesselation");
    }
    
    const char* Context_GLSL440::profile_name() const {
        return "GLUP440";
    }

    void Context_GLSL440::setup_GLUP_HEXAHEDRA() {
        if(!GEO::CmdLine::get_arg_bool("gfx:GLSL_tesselation")) {
            Context_GLSL150::setup_GLUP_HEXAHEDRA();
            return;
        }
        GLuint program = GLSL::compile_program_with_includes_no_link(
            this,
            "//stage GL_VERTEX_SHADER\n"
            "//import <GLUPGLSL/vertex_shader.h>\n",
            "//stage GL_FRAGMENT_SHADER\n"
            "//import <GLUPGLSL/fragment_shader.h>\n",
            "//stage GL_TESS_EVALUATION_SHADER\n"
            "//import <GLUPGLSL/tess_evaluation_shader.h>\n",            
            "//stage GL_GEOMETRY_SHADER\n"
            "//import <GLUPGLSL/geometry_shader_preamble.h>\n"
            "//import <GLUPGLSL/marching_cells.h>\n"
            "void main() {\n"
            "if(draw_clipped_cell()) { return; } \n"
            "emit_vertex(6,vec4(0.0,1.0,0.0,1.0),compute_clip_coords());\n"
            "emit_vertex(7,vec4(1.0,0.0,0.0,1.0),compute_clip_coords());\n"
            "emit_vertex(4,vec4(0.0,1.0,1.0,0.0),compute_clip_coords());\n"
            "emit_vertex(5,vec4(1.0,0.0,1.0,0.0),compute_clip_coords());\n"
            "emit_vertex(0,vec4(0.0,1.0,0.0,1.0),compute_clip_coords());\n"
            "emit_vertex(1,vec4(1.0,0.0,0.0,1.0),compute_clip_coords());\n"
            "emit_vertex(2,vec4(0.0,1.0,1.0,0.0),compute_clip_coords());\n"
            "emit_vertex(3,vec4(1.0,0.0,1.0,0.0),compute_clip_coords());\n"
            "emit_vertex(6,vec4(0.0,1.0,0.0,1.0),compute_clip_coords());\n"
            "emit_vertex(7,vec4(1.0,0.0,0.0,1.0),compute_clip_coords());\n"
            "EndPrimitive();\n"
            "draw_quad(4,0,6,2,compute_clip_coords());\n"
            "draw_quad(1,5,3,7,compute_clip_coords());\n"            
            "}\n"
        );
        set_primitive_info(
            GLUP_HEXAHEDRA, GL_PATCHES, program
        );
        marching_hex_.bind_uniform_state(program);
    }
    
    void Context_GLSL440::setup_GLUP_PYRAMIDS() {
        if(!GEO::CmdLine::get_arg_bool("gfx:GLSL_tesselation")) {
            Context_GLSL150::setup_GLUP_PYRAMIDS();
            return;
        }
        GLuint program = GLSL::compile_program_with_includes_no_link(
            this,
            "//stage GL_VERTEX_SHADER\n"
            "//import <GLUPGLSL/vertex_shader.h>\n",
            "//stage GL_FRAGMENT_SHADER\n"
            "//import <GLUPGLSL/fragment_shader.h>\n",
            "//stage GL_TESS_EVALUATION_SHADER\n"
            "//import <GLUPGLSL/tess_evaluation_shader.h>\n",            
            "//stage GL_GEOMETRY_SHADER\n"
            "//import <GLUPGLSL/geometry_shader_preamble.h>\n"
            "//import <GLUPGLSL/marching_cells.h>\n"
            "void main() {\n"
            "    if(draw_clipped_cell()) { return; } \n"
            "    draw_quad(0,1,3,2,compute_clip_coords());\n"
            "    draw_triangle(0,4,1,compute_clip_coords());\n"
            "    draw_triangle(0,3,4,compute_clip_coords());\n"
            "    draw_triangle(2,4,3,compute_clip_coords());\n"
            "    draw_triangle(2,1,4,compute_clip_coords());\n"
            "}\n"
        );
        set_primitive_info(
            GLUP_PYRAMIDS, GL_PATCHES, program
        );
        marching_pyramid_.bind_uniform_state(program);
    }

    /***********************************************************************/

    
    void Context_GLSL440::get_vertex_shader_preamble_pseudo_file(
        std::vector<GLSL::Source>& sources
    ) {
#ifdef GEO_OS_APPLE
        sources.push_back("#version 440\n");
#else
        sources.push_back("#version 440 core\n");        
#endif
        sources.push_back("#define GLUP_VERTEX_SHADER\n");
        OES_extensions(sources);
    }

    void Context_GLSL440::get_fragment_shader_preamble_pseudo_file(
        std::vector<GLSL::Source>& sources
    ) {
#ifdef GEO_OS_APPLE
        sources.push_back("#version 440\n");
#else
        sources.push_back("#version 440 core\n");        
#endif
        sources.push_back(
            "#define GLUP_FRAGMENT_SHADER\n"            
	    "#extension GL_ARB_conservative_depth : enable\n"
        );
        OES_extensions(sources);        
    }

    void Context_GLSL440::get_geometry_shader_preamble_pseudo_file(
        std::vector<GLSL::Source>& sources
    ) {
#ifdef GEO_OS_APPLE
        sources.push_back("#version 440\n");
#else
        sources.push_back("#version 440 core\n");        
#endif
        sources.push_back("#define GLUP_GEOMETRY_SHADER\n");
        OES_extensions(sources);
        get_geometry_shader_layout(sources);
    }
    
    void Context_GLSL440::get_tess_evaluation_shader_preamble_pseudo_file(
        std::vector<GLSL::Source>& sources
    ) {
#ifdef GEO_OS_APPLE
        sources.push_back("#version 440\n");
#else
        sources.push_back("#version 440 core\n");        
#endif
        sources.push_back("#define GLUP_TESS_EVALUATION_SHADER\n");
        OES_extensions(sources);        
    }

    void Context_GLSL440::get_primitive_pseudo_file(
        std::vector<GLSL::Source>& sources
    ) {
        Context::get_primitive_pseudo_file(sources);
        if(use_tessellation_) {
            if(primitive_source_ == GLUP_HEXAHEDRA) {
                sources.push_back("#define GLUP_TESS_GATHER\n");
                sources.push_back("#define GLUP_TESS_MULTI_VERTEX\n");
                sources.push_back(
                    "const int glup_nb_vertices_per_GL_v=4;\n"
                );
            } else if(primitive_source_ == GLUP_PYRAMIDS) {
                sources.push_back("#define GLUP_TESS_GATHER\n");
                sources.push_back(
                    "const int glup_nb_vertices_per_GL_v=5;\n"
                );
            }
        } else {
            if(primitive_source_ == GLUP_HEXAHEDRA) {
                sources.push_back("#define GLUP_VERTEX_GATHER\n");
                sources.push_back(
                    "const int glup_nb_vertices_per_GL_v=2;\n"
                );
            } else if(primitive_source_ == GLUP_PYRAMIDS) {
                sources.push_back("#define GLUP_VERTEX_GATHER\n");
                sources.push_back(
                    "const int glup_nb_vertices_per_GL_v=5;\n"
                );
            }
        }
    }

    void Context_GLSL440::get_geometry_shader_layout(
        std::vector<GLSL::Source>& sources                        
    ) {
        switch(primitive_source_) {
        case GLUP_POINTS:
        case GLUP_LINES:
        case GLUP_TRIANGLES:
        case GLUP_QUADS:
        case GLUP_TETRAHEDRA:
        case GLUP_PRISMS:
        case GLUP_CONNECTORS:
	case GLUP_SPHERES:
            Context_GLSL150::get_geometry_shader_layout(sources);            
            break;

        case GLUP_HEXAHEDRA:
            sources.push_back(
                "layout(lines) in;\n"
                "layout(triangle_strip, max_vertices = 18) out;\n"
            );
            break;

        case GLUP_PYRAMIDS:
            sources.push_back(
                "layout(lines) in;\n"
                "layout(triangle_strip, max_vertices = 28) out;\n"
            );
            break;
            
        case GLUP_NB_PRIMITIVES:
            geo_assert_not_reached;
        }
    }
}

#endif
