//import <GLUP/current_profile/geometry_shader_preamble.h>
//import <GLUPGLSL/state.h>
//import <GLUP/stdglup.h>
//import <GLUP/current_profile/toggles.h>
//import <GLUP/current_profile/primitive.h>


#if defined(GLUP_TESS_GATHER)

in GTVertexData {                                    
    vec4 vertex_clip_space[glup_nb_vertices_per_GL_v];            
    vec4 color[glup_nb_vertices_per_GL_v];                  
    vec4 tex_coord[glup_nb_vertices_per_GL_v];
#ifndef GLUP_TESS_MULTI_VERTEX
    bool discard_me;
#endif
} VertexIn[];                                         

#elif defined(GLUP_VERTEX_GATHER)

in GVertexData {                                    
    vec4 other_vertex_clip_space[glup_nb_vertices_per_GL_v-1];            
    vec4 color[glup_nb_vertices_per_GL_v];                  
    vec4 tex_coord[glup_nb_vertices_per_GL_v];
} VertexIn[];                                         

#else

in VertexData {                            
    vec4 color;                            
    vec4 tex_coord;
#if GLUP_PRIMITIVE_DIMENSION==2    
    vec3 normal;
#endif    
} VertexIn[];

#endif

out VertexData {                            
    vec4 vertex_clip_space;                       
    vec4 color;                             
    vec4 tex_coord;
#if GLUP_PRIMITIVE_DIMENSION==2        
    vec3 normal;
#endif    
    vec4 mesh_tex_coord;
} VertexOut;                              

#ifndef GLUP_NO_GL_CLIPPING    
out float gl_ClipDistance[];
#endif

//****** Data abstraction **************

#if defined(GLUP_VERTEX_GATHER) || defined(GLUP_TESS_GATHER)

#   if defined(GLUP_VERTEX_GATHER)
vec4 vertex_clip_space_in(in int i) {                 
    int i0 = i / glup_nb_vertices_per_GL_v;        
    int i1 = i % glup_nb_vertices_per_GL_v;        
    return (i1==0) ? gl_in[i0].gl_Position :
        VertexIn[i0].other_vertex_clip_space[i1-1]; 
}                                          
#   else
vec4 vertex_clip_space_in(in int i) {            
    int i0 = i / glup_nb_vertices_per_GL_v;       
    int i1 = i % glup_nb_vertices_per_GL_v;       
    return VertexIn[i0].vertex_clip_space[i1];   
}
#   endif

vec4 color_in(in int i) {                  
    int i0 = i / glup_nb_vertices_per_GL_v;       
    int i1 = i % glup_nb_vertices_per_GL_v;       
    return VertexIn[i0].color[i1];         
}                                          
                                                   
vec4 tex_coord_in(in int i) {                
    int i0 = i / glup_nb_vertices_per_GL_v;       
    int i1 = i % glup_nb_vertices_per_GL_v;       
    return VertexIn[i0].tex_coord[i1];     
}                                          

bool prim_is_discarded() {
#if defined(GLUP_TESS_GATHER) && !defined(GLUP_TESS_MULTI_VERTEX)
    return VertexIn[0].discard_me;
#endif    
    return false;                   
}                                  

#else

vec4 vertex_clip_space_in(in int i) {                 
    return gl_in[i].gl_Position;           
}                                          

vec4 color_in(in int i) {                  
    return VertexIn[i].color;              
}                                          
                                           
vec4 tex_coord_in(in int i) {              
    return VertexIn[i].tex_coord;          
}                                          

#if GLUP_PRIMITIVE_DIMENSION==2    
vec3 normal_in(in int i) {              
    return VertexIn[i].normal;
}                                          
#endif

bool prim_is_discarded() {                 
    return false;                   
}                                  

#endif

// **** Utilities ****************************

vec4 vertex_clip_space[glup_primitive_nb_vertices];

void get_vertices() {
    for(int v=0; v<glup_primitive_nb_vertices; ++v) {
        vertex_clip_space[v] = vertex_clip_space_in(v);
    }
    if(GLUP.cells_shrink != 0.0) {                                  
        vec4 g = vec4(0.0, 0.0, 0.0, 0.0);                          
        for(int i=0; i<glup_primitive_nb_vertices; ++i) {
            g += vertex_clip_space[i];                                     
        }                                                           
        g /= float(glup_primitive_nb_vertices);
        float s = GLUP.cells_shrink;                                
        for(int i=0; i<glup_primitive_nb_vertices; ++i) {
            vertex_clip_space[i] = mix(vertex_clip_space[i], g, s);
        }                                                           
    }                                                               
}

float clip_distance(in vec4 V, in bool do_clip) {
    return do_clip?dot(V,GLUP.clip_clip_plane):1.0;               
}                                                                  

bool cell_is_clipped() {                                           
    if(prim_is_discarded()) {                                        
        return true;                                                  
    }                                                                
    if(                                                              
        (glup_primitive_dimension != 3) ||                            
        !glupIsEnabled(GLUP_CLIPPING) ||                                        
        (GLUP.clipping_mode==GLUP_CLIP_STANDARD)  ||                    
	(GLUP.clipping_mode==GLUP_CLIP_SLICE_CELLS)                      
    ) {                                                              
        return false;                                                 
    }                                                                
    int count = 0;                                                   
    for(int i=0; i<glup_primitive_nb_vertices; ++i) {
        count += int(clip_distance(vertex_clip_space_in(i),true) >= 0.0);
    }
    if(
	(GLUP.clipping_mode==GLUP_CLIP_WHOLE_CELLS) && (count == 0)
    ) {    
        return true;                                                   
    }
    if(                                                              
        (GLUP.clipping_mode==GLUP_CLIP_STRADDLING_CELLS) &&
	((count==0) || (count==glup_primitive_nb_vertices))
    ) {                                                              
        return true;                                                   
    }

    // Should not happen ??? (first test would have returned)
    // But if I do not say that, straddling cells
    // do not work ??? (WTF?) GLSL compiler bug ?
    if(GLUP.clipping_mode==GLUP_CLIP_SLICE_CELLS) {
	return true;
    }

    return false;                                                    
}                                                                  

void emit_vertex(in int i, in vec4 mesh_tex_coord, in bool do_clip) {
#ifndef GLUP_NO_GL_CLIPPING        
    if(glupIsEnabled(GLUP_CLIPPING)) {
        gl_ClipDistance[0] = clip_distance(vertex_clip_space_in(i),do_clip);
    }
#endif    
    gl_Position = vertex_clip_space[i];
    VertexOut.vertex_clip_space = gl_Position;
    if(glupIsEnabled(GLUP_VERTEX_COLORS)) {
        VertexOut.color = color_in(i);                             
    }                                                               
    if(glupIsEnabled(GLUP_TEXTURING)) {                                       
        VertexOut.tex_coord = tex_coord_in(i);                     
    }
#if GLUP_PRIMITIVE_DIMENSION==2        
    if(
	glupIsEnabled(GLUP_LIGHTING) &&
	glupIsEnabled(GLUP_VERTEX_NORMALS)) {
	VertexOut.normal = normal_in(i);
    }
#endif    
    if(glupIsEnabled(GLUP_DRAW_MESH)) {
        VertexOut.mesh_tex_coord = mesh_tex_coord;
    }
    EmitVertex();                                                   
}

void draw_triangle(in int i1, in int i2, in int i3, in bool do_clip) {
    emit_vertex(i1, vec4(1.0, 0.0, 0.0, 0.0), do_clip);
    emit_vertex(i2, vec4(0.0, 1.0, 0.0, 0.0), do_clip);
    emit_vertex(i3, vec4(0.0, 0.0, 1.0, 0.0), do_clip);
    EndPrimitive();
}

void draw_quad(in int i1, in int i2, in int i3, in int i4, in bool do_clip) {
    emit_vertex(i1, vec4(0.0, 1.0, 0.0, 1.0), do_clip);
    emit_vertex(i2, vec4(1.0, 0.0, 0.0, 1.0), do_clip);
    emit_vertex(i3, vec4(0.0, 1.0, 1.0, 0.0), do_clip);
    emit_vertex(i4, vec4(1.0, 0.0, 1.0, 0.0), do_clip);
    EndPrimitive();    
}
 

