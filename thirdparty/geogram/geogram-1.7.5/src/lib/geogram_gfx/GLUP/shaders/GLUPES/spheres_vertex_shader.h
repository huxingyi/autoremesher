//import <GLUP/current_profile/vertex_shader_preamble.h>
//import <GLUPES/vertex_shader_state.h>
//import <GLUP/stdglup.h>
//import <GLUP/current_profile/toggles.h>
//import <GLUP/current_profile/primitive.h>

glup_in vec4 vertex_in;                         
glup_in vec4 color_in;                          
glup_in vec4 tex_coord_in;                      
glup_in highp float vertex_id_in;                       

glup_flat glup_out vec4 color;                             
glup_flat glup_out vec4 tex_coord;
glup_flat glup_out vec3 center_world_space;
glup_flat glup_out float radius;
glup_flat glup_out glup_id primitive_id;                           

vec4 row0(in mat4 M) {
    return vec4(M[0][0], M[1][0], M[2][0], M[3][0]);
}

vec4 row1(in mat4 M) {
    return vec4(M[0][1], M[1][1], M[2][1], M[3][1]);
}

vec4 row3(in mat4 M) {
    return vec4(M[0][3], M[1][3], M[2][3], M[3][3]);
}

// Reference:
// http://reality.cs.ucl.ac.uk/projects/quadrics/pbg06.pdf
// Ported from Dmitry / Sam code

void main(void) {

    if(glupIsEnabled(GLUP_PICKING)) {                      
#ifdef GLUP_ES_100
        primitive_id = float(int(vertex_id_in + 0.5)) + 0.5;
#else                                                 
        primitive_id = int(vertex_id_in + 0.5);              
#endif                                                
    }
    
    if(glupIsEnabled(GLUP_VERTEX_COLORS)) {                           
        color = color_in;                          
    }
    
    if(glupIsEnabled(GLUP_TEXTURING)) {                               
        if(glupIsEnabled(GLUP_INDIRECT_TEXTURING)) {                   
            tex_coord = tex_coord_in;              
        } else {                                             
            tex_coord = GLUP_VS.texture_matrix * tex_coord_in; 
        }                                                    
    }                                                       
    
    float R = vertex_in.w;
    gl_Position =
	GLUP_VS.modelviewprojection_matrix*vec4(vertex_in.xyz,1.0);

    // TODO: optimize, directly compute r1,r2,r4
    mat4 T = mat4(
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	vertex_in.x/R, vertex_in.y/R, vertex_in.z/R, 1.0/R
    );

    mat4 PMT = GLUP_VS.modelviewprojection_matrix * T;
    vec4 r1 = row0(PMT);
    vec4 r2 = row1(PMT);
    vec4 r4 = row3(PMT);
    
    float r1Dr4T = dot(r1.xyz,r4.xyz)-r1.w*r4.w;
    float r1Dr1T = dot(r1.xyz,r1.xyz)-r1.w*r1.w;
    float r4Dr4T = dot(r4.xyz,r4.xyz)-r4.w*r4.w;
    float r2Dr2T = dot(r2.xyz,r2.xyz)-r2.w*r2.w;
    float r2Dr4T = dot(r2.xyz,r4.xyz)-r2.w*r4.w;

    float discriminant_x = r1Dr4T*r1Dr4T-r4Dr4T*r1Dr1T;
    float discriminant_y = r2Dr4T*r2Dr4T-r4Dr4T*r2Dr2T;
    float screen = max(GLUP_VS.viewport[2], GLUP_VS.viewport[3]);
    gl_PointSize = sqrt(max(discriminant_x, discriminant_y)) * screen/(-r4Dr4T);

    center_world_space = vertex_in.xyz;
    radius = vertex_in.w;
}


