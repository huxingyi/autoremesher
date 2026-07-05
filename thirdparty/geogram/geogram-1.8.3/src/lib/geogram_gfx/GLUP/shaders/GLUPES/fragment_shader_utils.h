//import <GLUP/fragment_shader_utils.h>

float cell_edge_factor(in vec2 bary) {                                
    return edge_factor1(1.0-(1.0 - bary.x)*(1.0 - bary.y));         
}                                                                  

float edge_factor(in vec4 bary) {
    float e1,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12;
    float r1,r2,r3;
    if(glup_primitive == GLUP_TRIANGLES) {
        return edge_factor3(bary.xyz);    
    } else if(glup_primitive == GLUP_QUADS) {
        return edge_factor4(bary);
    } else if(glup_primitive == GLUP_TETRAHEDRA) {
        e1 = cell_edge_factor(bary.xy);                       
        e2 = cell_edge_factor(bary.xz);                       
        e3 = cell_edge_factor(bary.xw);                       
        e4 = cell_edge_factor(bary.yz);                       
        e5 = cell_edge_factor(bary.yw);                       
        e6 = cell_edge_factor(bary.zw);                   
        return min3(min(e1,e2),min(e3,e4),min(e5,e6));             
    } else if(glup_primitive == GLUP_HEXAHEDRA) {
        vec3 u = bary.xyz;                          
        vec3 U = vec3(1.0, 1.0, 1.0) - u;           
        e1 = cell_edge_factor(vec2(u.x,u.y)); 
        e2 = cell_edge_factor(vec2(u.x,u.z)); 
        e3 = cell_edge_factor(vec2(u.x,U.y)); 
        e4 = cell_edge_factor(vec2(u.x,U.z)); 
        e5 = cell_edge_factor(vec2(U.x,u.y)); 
        e6 = cell_edge_factor(vec2(U.x,u.z)); 
        e7 = cell_edge_factor(vec2(U.x,U.y)); 
        e8 = cell_edge_factor(vec2(U.x,U.z)); 
        e9  = cell_edge_factor(vec2(u.y,u.z)); 
        e10 = cell_edge_factor(vec2(u.y,U.z)); 
        e11 = cell_edge_factor(vec2(U.y,u.z)); 
        e12 = cell_edge_factor(vec2(U.y,U.z)); 
        r1 = min4(e1,e2,e3,e4);                
        r2 = min4(e5,e6,e7,e8);                
        r3 = min4(e9,e10,e11,e12);             
        return min3(r1,r2,r3);                       
    } else if(glup_primitive == GLUP_PRISMS) {
        vec4 bary2 = vec4(bary.x, bary.y, bary.z, 1.0 - bary.w);    
        e1 = cell_edge_factor(bary.xw);                       
        e2 = cell_edge_factor(bary.yw);                       
        e3 = cell_edge_factor(bary.zw);                       
        e4 = cell_edge_factor(bary2.xw);                      
        e5 = cell_edge_factor(bary2.yw);                      
        e6 = cell_edge_factor(bary2.zw);                      
        e7 = cell_edge_factor(bary.xy);                       
        e8 = cell_edge_factor(bary.yz);                       
        e9 = cell_edge_factor(bary.zx);                       
        return min(min3(e7,e8,e9),                                  
                   min3(min(e1,e2),
                        min(e3,e4),min(e5,e6))           
               );                                                   
    } 
    return 1.0;
}

vec4 glup_draw_mesh(in vec4 color, in vec4 mesh_tex_coord) {
#ifdef GLUP_NO_MESH_TEX_COORDS
    return color;
#else    
    return mix(
        GLUP.mesh_color, color, edge_factor(mesh_tex_coord)                  
    );
#endif    
}

vec4 glup_shading(
    in vec4 color,
    in vec4 tex_coord,
    in vec3 normal,
    in highp int primitive_id,
    in vec4 mesh_tex_coord
) {
    if(glupIsEnabled(GLUP_PICKING)) {
        return glup_picking(primitive_id);
    }
    vec4 result;
    if(glupIsEnabled(GLUP_VERTEX_COLORS)) {
        result = color;
    } else {
        result = gl_FrontFacing ? GLUP.front_color : GLUP.back_color;
    }
    if(glupIsEnabled(GLUP_TEXTURING) && !glupIsEnabled(GLUP_NORMAL_MAPPING)) {
        result = glup_texturing(result, tex_coord);
    }
    if(glupIsEnabled(GLUP_LIGHTING)) {
        result = glup_lighting(result, normal);
    }
    if(
        glupIsEnabled(GLUP_DRAW_MESH) && (
            (glup_primitive_dimension == 2) ||
            !glupIsEnabled(GLUP_CLIPPING) ||
            GLUP.clipping_mode != GLUP_CLIP_SLICE_CELLS
        )    
    ) {
        result = glup_draw_mesh(result, mesh_tex_coord);
    }                                                     
    return result;
}

