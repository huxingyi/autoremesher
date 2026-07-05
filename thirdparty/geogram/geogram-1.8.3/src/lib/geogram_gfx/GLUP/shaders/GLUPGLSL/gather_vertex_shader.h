//import <GLUP/current_profile/vertex_shader_preamble.h>
//import <GLUPGLSL/state.h>
//import <GLUP/stdglup.h>
//import <GLUP/current_profile/toggles.h>
//import <GLUP/current_profile/primitive.h>

in vec4 vertex_in[glup_nb_vertices_per_GL_v];              
in vec4 color_in[glup_nb_vertices_per_GL_v];               
in vec4 tex_coord_in[glup_nb_vertices_per_GL_v];           

out GVertexData {                                    
    vec4 other_vertex_clip_space[glup_nb_vertices_per_GL_v-1];         
    vec4 color[glup_nb_vertices_per_GL_v];                  
    vec4 tex_coord[glup_nb_vertices_per_GL_v];
} VertexOut;                                         


void main(void) {
    for(int i=0; i<glup_nb_vertices_per_GL_v; ++i) {         
        if(glupIsEnabled(GLUP_VERTEX_COLORS)) {
            VertexOut.color[i] = color_in[i];
        }
        if(glupIsEnabled(GLUP_TEXTURING)) {                                     
            if(glupIsEnabled(GLUP_INDIRECT_TEXTURING)) {
                VertexOut.tex_coord[i] = tex_coord_in[i];                   
            } else {                                                  
                VertexOut.tex_coord[i] = GLUP.texture_matrix * tex_coord_in[i];
            }                                                         
        }
    }

    for(int i=1; i<glup_nb_vertices_per_GL_v; ++i) {         
        VertexOut.other_vertex_clip_space[i-1] =
            GLUP.modelviewprojection_matrix * vertex_in[i];
    }                                               
    
    gl_Position = GLUP.modelviewprojection_matrix * vertex_in[0];
}                                                                 
