//import <fullscreen/current_profile/fragment_shader_preamble.h>
//import <GLUP/defs.h>

glup_in vec2 tex_coord;  

uniform sampler2D source_tex;
uniform sampler2D depth_tex;

uniform float blur_width;
uniform bool vertical;

const float PI = 3.14159265;
const float threshold = 0.005;

#ifdef GL_ES
uniform vec2 source_tex_size;
#define width float(source_tex_size.x);
#define height float(source_tex_size.y);
#else
float width  = float(textureSize(source_tex,0).x);
float height = float(textureSize(source_tex,0).y);
#endif

// 1D Gaussian distribution, s is standard deviation
float gaussian(in float x, in float s) {
    return exp(-x * x / (2.0 * s * s)) / (s * sqrt(2.0 * PI));
}

float get_z_coeff(in vec2 pos) {
    float zCoef = glup_texture(depth_tex, pos).r;
    zCoef = 3.0 * (zCoef - 0.1) ;
    return zCoef;
}

float get_z_dist(in vec2 center_pos, in vec2 other_pos) {
    return abs(get_z_coeff(center_pos) - get_z_coeff(other_pos));
}

void compute_blur() {
    int n = int(floor(3.0 * blur_width) - 1.0);
    float sum = 0.0;
    int i;
    
    vec2 cur_pix_coords;
    vec4 cur_pix_tex;
    vec4 final_pix_tex = vec4(0.0);
    
    // Calculate the sum of weights for the blur
#ifdef GL_ES
    for(int i = -5; i <= 5; i++) {    
#else    
    for(int i = -n; i <= n; i++) {
#endif	
        float x_offset, y_offset;
        if (vertical) {
            x_offset = 0.0;
            y_offset = float(i);
        } else {
            x_offset = float(i);
            y_offset = 0.0;
        }
        
        x_offset = x_offset / width;
        y_offset = y_offset / height;
        
        cur_pix_coords = vec2(x_offset, y_offset) + tex_coord;
        
        if(get_z_dist(tex_coord, cur_pix_coords) <= threshold) {
            float weight = gaussian(float(i), blur_width);
            sum += weight;
        }
    }
    
    // Calculate the blurred color
#ifdef GL_ES
    for(int i = -5; i <= 5; i++) {    
#else    
    for(int i = -n; i <= n; i++) {
#endif	
        float x_offset, y_offset;
        if (vertical) {
            x_offset = 0.0;
            y_offset = float(i);
        } else {
            x_offset = float(i);
            y_offset = 0.0;
        }
        
        x_offset = x_offset / width;
        y_offset = y_offset / height;
        
        cur_pix_coords = vec2(x_offset, y_offset) + tex_coord;
        
        if(get_z_dist(tex_coord, cur_pix_coords) <= threshold) {
            cur_pix_tex = glup_texture(source_tex, cur_pix_coords);
            float weight = gaussian(float(i), blur_width) / sum;
            final_pix_tex += cur_pix_tex * weight;
        }
    }
    glup_FragColor.rgb = final_pix_tex.rgb;
}

void main() {
    compute_blur();
}

