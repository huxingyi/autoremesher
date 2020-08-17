//import <GLUP/current_profile/fragment_shader_preamble.h>
//import <GLUPGLSL/state.h>
//import <GLUP/stdglup.h>
//import <GLUP/current_profile/toggles.h>
//import <GLUP/current_profile/primitive.h>
//import <GLUP/fragment_shader_utils.h>

#ifndef GLUP_NO_GL_CLIPPING        
in float gl_ClipDistance[];
#endif

#ifdef GL_ARB_conservative_depth
layout (depth_less) out float gl_FragDepth;   
#endif

in VertexData {                            
    vec4 color;                             
    vec4 tex_coord;
    float depth_radius;
} FragmentIn;                              

void main() {

#ifdef GLUP_GL_ES
#ifndef GLUP_NO_GL_CLIPPING            
    if(glupIsEnabled(GLUP_CLIPPING) && (gl_ClipDistance[0] < 0.0)) {
        discard;                                                
    }
#endif    
#endif

    vec2 V = 2.0*(gl_PointCoord - vec2(0.5, 0.5));             
    float one_minus_r2 = 1.0 - dot(V,V);                       
    if(one_minus_r2 < 0.0) {                                   
        discard;                                                
    }                                                          

    vec3 N = vec3(V.x, -V.y, sqrt(one_minus_r2));

    glup_FragDepth = gl_FragCoord.z - FragmentIn.depth_radius * N.z;
    
    if(glupIsEnabled(GLUP_PICKING)) {
        glup_FragColor = glup_picking(gl_PrimitiveID);
	return;
    }
    
    vec4 result;
    if(glupIsEnabled(GLUP_VERTEX_COLORS)) {
	result = FragmentIn.color;
    } else {
	result = GLUP.front_color;
    }
    if(glupIsEnabled(GLUP_TEXTURING)) {
	result = glup_texturing(result, FragmentIn.tex_coord);
    }
    if(glupIsEnabled(GLUP_LIGHTING)) {
	result = glup_lighting(result, N);
    }
    glup_FragColor = result;
    glup_alpha_discard();
}                                                                  

