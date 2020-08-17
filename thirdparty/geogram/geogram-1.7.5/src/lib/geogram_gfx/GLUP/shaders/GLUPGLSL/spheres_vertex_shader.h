//import <GLUP/current_profile/vertex_shader_preamble.h>
//import <GLUPGLSL/state.h>
//import <GLUP/stdglup.h>
//import <GLUP/current_profile/toggles.h>
//import <GLUP/current_profile/primitive.h>

in vec4 vertex_in;                         
in vec4 color_in;                          
in vec4 tex_coord_in;                      

out VertexData {                           
    vec4 color;                             
    vec4 tex_coord;
    vec3 center_world_space;
    float radius;
} VertexOut;

// Reference:
// http://reality.cs.ucl.ac.uk/projects/quadrics/pbg06.pdf
// Ported from Dmitry / Sam code

vec4 row(in mat4 M, in int i) {
    return vec4(M[0][i], M[1][i], M[2][i], M[3][i]);
}

void main(void) {
    
    if(glupIsEnabled(GLUP_VERTEX_COLORS)) {                           
        VertexOut.color = color_in;                          
    }
    
    if(glupIsEnabled(GLUP_TEXTURING)) {                               
        if(glupIsEnabled(GLUP_INDIRECT_TEXTURING)) {                   
            VertexOut.tex_coord = tex_coord_in;              
        } else {                                             
            VertexOut.tex_coord = GLUP.texture_matrix * tex_coord_in; 
        }                                                    
    }                                                       
    
    float R = vertex_in.w;
    gl_Position =
	GLUP.modelviewprojection_matrix*vec4(vertex_in.xyz,1.0);

    // TODO: optimize: directly compute r1,r2,r4
    mat4 T = mat4(
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	vertex_in.x/R, vertex_in.y/R, vertex_in.z/R, 1.0/R
    );

    mat4 PMT = GLUP.modelviewprojection_matrix * T;
    vec4 r1 = row(PMT,0);
    vec4 r2 = row(PMT,1);
    vec4 r4 = row(PMT,3);
    
    float r1Dr4T = dot(r1.xyz,r4.xyz)-r1.w*r4.w;
    float r1Dr1T = dot(r1.xyz,r1.xyz)-r1.w*r1.w;
    float r4Dr4T = dot(r4.xyz,r4.xyz)-r4.w*r4.w;
    float r2Dr2T = dot(r2.xyz,r2.xyz)-r2.w*r2.w;
    float r2Dr4T = dot(r2.xyz,r4.xyz)-r2.w*r4.w;

    float discriminant_x = r1Dr4T*r1Dr4T-r4Dr4T*r1Dr1T;
    float discriminant_y = r2Dr4T*r2Dr4T-r4Dr4T*r2Dr2T;
    float screen = max(GLUP.viewport[2], GLUP.viewport[3]);
    
    gl_PointSize = sqrt(max(discriminant_x,discriminant_y)) * screen/(-r4Dr4T);

    VertexOut.center_world_space = vertex_in.xyz;
    VertexOut.radius = R;
}


