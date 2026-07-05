//import <GLUP/current_profile/vertex_shader_preamble.h>
//import <GLUPGLSL/state.h>
//import <GLUP/stdglup.h>
//import <GLUP/current_profile/toggles.h>
//import <GLUP/current_profile/primitive.h>

in vec4 vertex_in;                         
in vec4 color_in;                          
in vec4 tex_coord_in;

#if GLUP_PRIMITIVE_DIMENSION==2
in vec4 normal_in;
#endif

out VertexData {
#ifdef GLUP_GL_ES
      vec4 vertex_clip_space;
#endif    
    vec4 color;                             
    vec4 tex_coord;
#if GLUP_PRIMITIVE_DIMENSION==2    
    vec3 normal;
#  ifdef GLUP_GL_ES
      vec4 mesh_tex_coord;
#  endif    
#endif
} VertexOut;                               


void main(void) {
    if(glupIsEnabled(GLUP_VERTEX_COLORS)) {                                 
        VertexOut.color = color_in;                                
    }                                                             
    if(glupIsEnabled(GLUP_TEXTURING)) {                                     
        if(glupIsEnabled(GLUP_INDIRECT_TEXTURING)) {                        
            VertexOut.tex_coord = tex_coord_in;                   
        } else {                                                  
            VertexOut.tex_coord =                                 
                GLUP.texture_matrix * tex_coord_in;      
        }                                                         
    }
    
#if GLUP_PRIMITIVE_DIMENSION==1

#ifndef GLUP_NO_GL_CLIPPING	
    if(glupIsEnabled(GLUP_CLIPPING)) {
        gl_ClipDistance[0] = dot(                           
            vertex_in, GLUP.world_clip_plane               
        );
    } else {                                                
        gl_ClipDistance[0] = 0.0;                            
    }                                                       
#endif
    
#elif GLUP_PRIMITIVE_DIMENSION==2
    
    if(
	glupIsEnabled(GLUP_LIGHTING) &&
	glupIsEnabled(GLUP_VERTEX_NORMALS)
    ) {
	VertexOut.normal = GLUP.normal_matrix * normal_in.xyz;
    }
#endif    
    gl_Position = GLUP.modelviewprojection_matrix * vertex_in;
#if GLUP_PRIMITIVE_DIMENSION==2
#  ifdef GLUP_GL_ES
    VertexOut.vertex_clip_space = gl_Position;
    if(glupIsEnabled(GLUP_DRAW_MESH)) {
	// Note: does not work with glDrawElements because gl_VertexID is
	// the element index !
	int lv = gl_VertexID % glup_primitive_nb_vertices;
	VertexOut.mesh_tex_coord = vec4(
	    float(lv == 0),
	    float(lv == 1),
	    float(lv == 2),
	    float(lv == 3)
	);
    }
#  endif    
#endif
}                                                                 
