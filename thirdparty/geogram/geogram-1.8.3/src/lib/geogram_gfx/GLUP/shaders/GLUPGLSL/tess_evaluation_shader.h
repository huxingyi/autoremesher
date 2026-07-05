//import <GLUP/current_profile/tess_evaluation_shader_preamble.h>
//import <GLUPGLSL/state.h>
//import <GLUP/stdglup.h>
//import <GLUP/current_profile/toggles.h>
//import <GLUP/current_profile/primitive.h>

layout(isolines) in;

in VertexData {                                                    
    vec4 color;                                             
    vec4 tex_coord;
} VertexIn[];                                              

out GTVertexData {                                                 
    vec4 vertex_clip_space[glup_nb_vertices_per_GL_v];                          
    vec4 color[glup_nb_vertices_per_GL_v];                                
    vec4 tex_coord[glup_nb_vertices_per_GL_v];
#ifndef GLUP_TESS_MULTI_VERTEX    
    bool discard_me;
#endif    
} VertexOut;

#ifdef GLUP_TESS_MULTI_VERTEX

void main() {
    int i0 = int(gl_TessCoord.x + 0.5);                                   
    for(int i1=0; i1<glup_nb_vertices_per_GL_v; ++i1) {                    
        int i = i0*glup_nb_vertices_per_GL_v + i1;                        
        VertexOut.vertex_clip_space[i1] = gl_in[i].gl_Position;
    }                                                               
    if(glupIsEnabled(GLUP_VERTEX_COLORS)) {                                   
        for(int i1=0; i1<glup_nb_vertices_per_GL_v; ++i1) {                  
            int i = i0*glup_nb_vertices_per_GL_v + i1;                        
            VertexOut.color[i1] = VertexIn[i].color;                   
        }                                                             
    }                                                               
    if(glupIsEnabled(GLUP_TEXTURING)) {                                       
        for(int i1=0; i1<glup_nb_vertices_per_GL_v; ++i1) {                  
            int i = i0*glup_nb_vertices_per_GL_v + i1;                        
            VertexOut.tex_coord[i1] = VertexIn[i].tex_coord;           
        }                                                             
    }
}                                                                  
#else

void main() {                                                      
    VertexOut.discard_me = (gl_TessCoord.x > 0.5);
    if(VertexOut.discard_me) {                                      
        return;                                                    
    } 
    for(int i=0; i<glup_primitive_nb_vertices; ++i) {
        VertexOut.vertex_clip_space[i] = gl_in[i].gl_Position;
    }                                                               
    if(glupIsEnabled(GLUP_VERTEX_COLORS)) {                                   
        for(int i=0; i<glup_primitive_nb_vertices; ++i) {       
            VertexOut.color[i] = VertexIn[i].color;                    
        }                                                             
    }                                                               
    if(glupIsEnabled(GLUP_TEXTURING)) {                                       
        for(int i=0; i<glup_primitive_nb_vertices; ++i) {      
            VertexOut.tex_coord[i] = VertexIn[i].tex_coord;            
        }                                                             
    }
}                                                                  

#endif


