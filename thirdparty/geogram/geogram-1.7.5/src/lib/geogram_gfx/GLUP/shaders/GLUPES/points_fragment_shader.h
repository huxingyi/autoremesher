//import <GLUP/current_profile/fragment_shader_preamble.h>
//import <GLUPES/fragment_shader_state.h>
//import <GLUP/stdglup.h>
//import <GLUP/current_profile/toggles.h>
//import <GLUP/current_profile/primitive.h>
//import <GLUPES/fragment_shader_utils.h>

   glup_in vec4 color;                                             
   glup_in vec4 tex_coord;                                         
   glup_in float clip_dist;
   glup_flat glup_in float depth_radius;
   glup_flat glup_in glup_id primitive_id;

void main() {
    
    if(glupIsEnabled(GLUP_CLIPPING) && (clip_dist < 0.0)) {              
        discard;                                                
    }                                                          

    vec2 V = 2.0*(gl_PointCoord - vec2(0.5, 0.5));             
    float one_minus_r2 = 1.0 - dot(V,V);                       
    if(one_minus_r2 < 0.0) {                                   
        discard;                                                
    }                                                          

    vec3 N = vec3(V.x, -V.y, sqrt(one_minus_r2));
    glup_FragDepth = gl_FragCoord.z - depth_radius * N.z;

    if(glupIsEnabled(GLUP_PICKING)) {
        glup_FragColor = glup_picking(int(primitive_id));        
        return;
    }

    vec4 result;
    if(glupIsEnabled(GLUP_VERTEX_COLORS)) {
        result = color;
    } else {
        result = GLUP.front_color;
    }
    if(glupIsEnabled(GLUP_TEXTURING)) {
        result = glup_texturing(result, tex_coord);
    }
    if(glupIsEnabled(GLUP_LIGHTING)) {
        result = glup_lighting(result, N);
    }
    glup_FragColor = result;
    glup_alpha_discard();        
}                                                             
