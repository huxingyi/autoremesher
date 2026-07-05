//import <fullscreen/current_profile/fragment_shader_preamble.h>
//import <GLUP/defs.h>

glup_in vec2 tex_coord;  

uniform sampler2D blur_texture;
uniform sampler2D depth_texture;

uniform bool  do_positive_shadows; // = false;
uniform float shadows_gamma      ; // = 0.8; 
uniform float shadows_intensity  ; // = 1.0; 
uniform bool  shadows_halo       ; // = false;

float equalize_shadow(float x) {
   return pow(x, shadows_gamma);
}

float unsharp_masking(vec2 uv) {
    float orig_depth  = glup_texture(depth_texture,uv).x;
    if(!shadows_halo && orig_depth == 1.0) { orig_depth = 0.5; }
    float smoothed_depth = glup_texture(blur_texture,uv).x;
    float result = smoothed_depth-orig_depth;
    if(result < 0.0) {
        result = -equalize_shadow(-result);
    } else {
        if(do_positive_shadows) {
            result = equalize_shadow(result);
        } else {
            result = 0.0;
        }
    }
    return -100.0 * shadows_intensity * result;
}

void compute_unsharp_masking() {
    float shadow = unsharp_masking(tex_coord);
    if(!do_positive_shadows || shadow > 0.0) {
        glup_FragColor.rgb = vec3(0.0, 0.0, 0.0) ;
        glup_FragColor.a = shadow;
    } else {
        glup_FragColor.rgb = vec3(1.0, 1.0, 1.0) ;
        glup_FragColor.a = -shadow;
    }
}

void main() {
    compute_unsharp_masking();
}

