//import <fullscreen/current_profile/fragment_shader_preamble.h>
//import <GLUP/stdglup.h>

glup_in vec2 tex_coord;  

uniform sampler2D source_tex;

uniform float blur_width;
uniform bool vertical;

const float PI = 3.14159265;

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

void compute_blur() {
    int n = int(floor(3.0 * blur_width) - 1.0);
    float sum = 0.0;

#ifdef GL_ES
    for(int i = -5; i <= 5; i++) {    
#else    
    for(int i = -n; i <= n; i++) {
#endif	
        float weight = gaussian(float(i), blur_width);
        sum += weight;
    }

    vec2 cur_pix_coords;
    vec4 cur_pix_tex;
    vec4 final_pix_tex = vec4(0.0);

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
        
        float weight = gaussian(float(i), blur_width) / sum;
        cur_pix_coords = vec2(x_offset, y_offset) + tex_coord;
        cur_pix_tex = glup_texture(source_tex, cur_pix_coords);
        final_pix_tex += cur_pix_tex * weight;
    }
    glup_FragColor.rgb = final_pix_tex.rgb;
}

void main() {
    compute_blur();
}


