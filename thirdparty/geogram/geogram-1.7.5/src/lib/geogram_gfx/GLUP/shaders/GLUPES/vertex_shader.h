//import <GLUP/current_profile/vertex_shader_preamble.h>
//import <GLUP/stdglup.h>
//import <GLUP/current_profile/toggles.h>
//import <GLUP/current_profile/primitive.h>
//import <GLUPES/vertex_shader_state.h>

   glup_in vec4 vertex_in;                                 
   glup_in vec4 color_in;                                  
   glup_in vec4 tex_coord_in;
   glup_in highp float vertex_id_in;                       
   glup_out vec3 vertex_view_space;                        
   glup_out float clip_dist;                               
   glup_out vec4 color;                                    
   glup_out vec4 tex_coord;
   glup_out vec4 mesh_tex_coord;
   glup_flat glup_out glup_id primitive_id;

#if GLUP_PRIMITIVE_DIMENSION==2
   glup_in vec4 normal_in;
   glup_out vec3 normal;
#endif
        
void main() {                                         
    
    if(glupIsEnabled(GLUP_CLIPPING)) {                     
        clip_dist = dot(                                
            vertex_in, GLUP_VS.world_clip_plane         
        );                                              
    }                                                  

    if(glupIsEnabled(GLUP_PICKING)) {
#ifdef GLUP_ES_100
        // Note: we need to add 0.5, else there are some precision
        // issues, and the integer mod() operation creates random
        // values...
        primitive_id = float(
            int(vertex_id_in+0.5)/glup_primitive_nb_vertices
        )+0.5;
#else                                                   
        primitive_id = int(vertex_id_in + 0.5)/glup_primitive_nb_vertices; 
#endif                                                  
    }                                                    
        
    if(glupIsEnabled(GLUP_LIGHTING)) {                             
        vertex_view_space = (GLUP_VS.modelview_matrix * vertex_in).xyz;
#if GLUP_PRIMITIVE_DIMENSION==2	
	if(glupIsEnabled(GLUP_VERTEX_NORMALS)) {
	    normal = GLUP_VS.normal_matrix*normal_in.xyz;
	}
#endif
    }                                                  
    
    if(glupIsEnabled(GLUP_VERTEX_COLORS)) {                
        color = color_in;                               
    }                                                  
        
    if(glupIsEnabled(GLUP_TEXTURING)) {
        if(glupIsEnabled(GLUP_INDIRECT_TEXTURING)) {                        
            tex_coord = tex_coord_in;                   
        } else {                                                  
            tex_coord = GLUP_VS.texture_matrix * tex_coord_in;      
        }                                                         
    }                                                  

    if(glupIsEnabled(GLUP_DRAW_MESH)) {
        mesh_tex_coord = get_mesh_tex_coord(int(vertex_id_in + 0.5));
    }                                                  
    
    gl_Position = GLUP_VS.modelviewprojection_matrix * vertex_in;  
}                                                     
