//import <fullscreen/current_profile/vertex_shader_preamble.h>
//import <GLUP/stdglup.h>

glup_in vec2 vertex_in; 
glup_in vec2 tex_coord_in;
glup_out vec2 tex_coord;                         

void main() {
    tex_coord = tex_coord_in;      
    gl_Position = vec4(vertex_in, 0.0, 1.0); 
}

