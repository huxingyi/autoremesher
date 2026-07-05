//import <GLUP/current_profile/vertex_shader_preamble.h>
//import <GLUPES/vertex_shader_state.h>
//import <GLUP/stdglup.h>
//import <GLUP/current_profile/toggles.h>
//import <GLUP/current_profile/primitive.h>

glup_in vec4 vertex_in;                                 
glup_in vec4 color_in;                                  
glup_in vec4 tex_coord_in;                              
glup_in highp float vertex_id_in;                       
glup_out vec4 color;                                    
glup_out vec4 tex_coord;                                
glup_out float clip_dist;
glup_flat glup_out float depth_radius;
glup_flat glup_out glup_id primitive_id;                           
                                                              
void main() {                                         
    if(glupIsEnabled(GLUP_CLIPPING)) {                     
        clip_dist = dot(vertex_in, GLUP_VS.world_clip_plane);
    }                                                  
    if(glupIsEnabled(GLUP_PICKING)) {                      
#ifdef GLUP_ES_100
        primitive_id = float(int(vertex_id_in + 0.5)) + 0.5;
#else                                                 
        primitive_id = int(vertex_id_in + 0.5);              
#endif                                                
    }
    if(glupIsEnabled(GLUP_VERTEX_COLORS)) {                
        color = color_in;                              
    }                                                  
    if(glupIsEnabled(GLUP_TEXTURING)) {                    
        tex_coord = GLUP_VS.texture_matrix * tex_coord_in; 
    }
    if(glup_primitive == GLUP_POINTS) {
        gl_PointSize = GLUP_VS.point_size;
    }
    gl_Position = GLUP_VS.modelviewprojection_matrix * vertex_in;
    // TODO (depth radius corresponds to maximum difference of depth,
    // at the center of the displayed GL_POINT).
    depth_radius = 0.001;
}


