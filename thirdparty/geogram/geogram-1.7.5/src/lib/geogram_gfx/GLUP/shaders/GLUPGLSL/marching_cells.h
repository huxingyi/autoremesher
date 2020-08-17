//import <GLUP/current_profile/marching_cells.h>

int compute_config() {                               
    int result = 0;                                    
    for(int v=0; v<cell_nb_vertices; ++v) {            
        if(dot(vertex_clip_space_in(v),GLUP.clip_clip_plane) > 0.0) {
            result = result | (1 << v);                   
        }                                                
    }                                                   
    return result;                                      
}                                                    

void emit_isect_vertex(in int i, in vec4 mesh_tex_coord) {
#ifndef GLUP_NO_GL_CLIPPING            
    gl_ClipDistance[0] = 1.0;
#endif    
    gl_Position = isect_point_clip_space[i];                     
    if(glupIsEnabled(GLUP_VERTEX_COLORS)) {
        VertexOut.color = isect_color[i];            
    }                                                 
    if(glupIsEnabled(GLUP_TEXTURING)) {                         
        VertexOut.tex_coord = isect_tex_coord[i];    
    }
    if(glupIsEnabled(GLUP_DRAW_MESH)) {
        VertexOut.mesh_tex_coord = mesh_tex_coord;
    }
    if(glupIsEnabled(GLUP_LIGHTING)) {
        VertexOut.vertex_clip_space = gl_Position;
    }
    EmitVertex();                                     
}                                                    

void isect_triangle(
    in int i, in vec4 mtexi,
    in int j, in vec4 mtexj,
    in int k, in vec4 mtexk
) {           
    emit_isect_vertex(i,mtexi);                             
    emit_isect_vertex(j,mtexj);                             
    emit_isect_vertex(k,mtexk);
}                                                    

void draw_marching_cell() {                          
    int config = compute_config();


    if(config_size(config) == 0) {
        return;
    } 
    
    compute_intersections();                          
    int size = config_size(config);

    if(glupIsEnabled(GLUP_DRAW_MESH)) {
        // Single triangle: mesh tex coords are standard tri mesh tex coord.
        if(size == 3) {                                
            isect_triangle(                              
                config_edge(config,0), vec4(1.0, 0.0, 0.0, 0.0),
                config_edge(config,1), vec4(0.0, 1.0, 0.0, 0.0),
                config_edge(config,2), vec4(0.0, 0.0, 1.0, 0.0)
            );                                           
        } else {
            // First triangle: draw mesh only on first wedge
            // (let's pretend it is a wedge of a quad)
            isect_triangle(                              
                config_edge(config,0), vec4(0.0, 1.0, 0.0, 1.0),                
                config_edge(config,1), vec4(1.0, 0.0, 0.0, 1.0),
                config_edge(config,2), vec4(1.0, 0.0, 1.0, 0.0)
            );
            // Middle triangles: draw mesh only on [i,i+1]
            // (let's pretend that vertex i is the quad center)
            for(int i=2; i+2<size; ++i) {                
                isect_triangle(                            
                    config_edge(config,0), vec4(0.5, 0.5, 0.5, 0.5),
                    config_edge(config,i), vec4(0.0, 1.0, 0.0, 1.0),
                    config_edge(config,i+1), vec4(1.0, 0.0, 0.0, 1.0)
                );                                         
            }
            // Last triangle: draw mesh only on last wedge
            // (let's pretend it is a wedge of a quad)            
            isect_triangle(                              
                config_edge(config,0),      vec4(0.0, 1.0, 0.0, 1.0),
                config_edge(config,size-2), vec4(1.0, 0.0, 1.0, 0.0),
                config_edge(config,size-1), vec4(1.0, 0.0, 0.0, 1.0)
            );                                           
        }                                               
    } else {                                          
        for(int i=1; i+1<size; ++i) {                  
            isect_triangle(                              
                config_edge(config,0),   vec4(0.0, 0.0, 0.0, 0.0),
                config_edge(config,i),   vec4(0.0, 0.0, 0.0, 0.0),
                config_edge(config,i+1), vec4(0.0, 0.0, 0.0, 0.0)
            );                                           
        }                                              
    } 
}

bool compute_clip_coords() {
    return (
        glupIsEnabled(GLUP_CLIPPING) &&
        GLUP.clipping_mode==GLUP_CLIP_STANDARD
    );
}

bool draw_clipped_cell() {
    if(cell_is_clipped()) {
        return true;
    }
    gl_PrimitiveID = gl_PrimitiveIDIn;
    get_vertices();
    if(
        glupIsEnabled(GLUP_CLIPPING) &&
        GLUP.clipping_mode == GLUP_CLIP_SLICE_CELLS
    ) {
        draw_marching_cell();
        return true;
    }
    return false;
}

