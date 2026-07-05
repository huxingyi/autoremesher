//import <GLUP/current_profile/vertex_shader_preamble.h>
//import <GLUPGLSL/state.h>
//import <GLUP/stdglup.h>
//import <GLUP/current_profile/toggles.h>
//import <GLUP/current_profile/primitive.h>

in vec4 vertex_in;                         
in vec4 color_in;                          
in vec4 tex_coord_in;                      

out VertexData {                           
    vec4 color;                             
    vec4 tex_coord;
    float depth_radius;
} VertexOut;

void main() {
#ifndef GLUP_NO_GL_CLIPPING            
    if(glupIsEnabled(GLUP_CLIPPING)) {                               
        gl_ClipDistance[0] = dot(                           
            vertex_in, GLUP.world_clip_plane               
        );                                                  
    } else {                                                
        gl_ClipDistance[0] = 0.0;                            
    }
#endif    
    if(glupIsEnabled(GLUP_VERTEX_COLORS)) {                           
        VertexOut.color = color_in;                          
    }                                                       
    if(glupIsEnabled(GLUP_TEXTURING)) {                               
        if(glupIsEnabled(GLUP_INDIRECT_TEXTURING)) {                   
            VertexOut.tex_coord = tex_coord_in;              
        } else {                                             
            VertexOut.tex_coord = GLUP.texture_matrix * tex_coord_in; 
        }                                                    
    }                                                       
    gl_PointSize = GLUP.point_size;                         
    gl_Position = GLUP.modelviewprojection_matrix*vertex_in;

    // TODO (depth radius corresponds to maximum difference of depth,
    // at the center of the displayed GL_POINT).
    VertexOut.depth_radius = 0.001;
}
