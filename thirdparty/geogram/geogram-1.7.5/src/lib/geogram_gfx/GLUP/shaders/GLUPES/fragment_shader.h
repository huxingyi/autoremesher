//import <GLUP/current_profile/fragment_shader_preamble.h>
//import <GLUPES/fragment_shader_state.h>
//import <GLUP/stdglup.h>
//import <GLUP/current_profile/toggles.h>
//import <GLUP/current_profile/primitive.h>
//import <GLUPES/fragment_shader_utils.h>

   glup_in vec3 vertex_view_space;                                 
   glup_in float clip_dist;                                 
   glup_in vec4 color;                                     
   glup_in vec4 tex_coord;
   glup_in vec4 mesh_tex_coord;
   glup_flat glup_in glup_id primitive_id;

#if GLUP_PRIMITIVE_DIMENSION==2
   glup_in vec3 normal;
#endif


void main() {

    if(glupIsEnabled(GLUP_CLIPPING)) {
        if(glup_primitive_dimension == 2) {
            if(clip_dist < 0.0) {                                       
                discard;                               
            }                                         
        } else if(glup_primitive_dimension == 3) {
            if(
                clip_dist < 0.0 &&
                GLUP.clipping_mode == GLUP_CLIP_STANDARD
            ) {
                discard;
            }
        }
    }

    vec3 N;
    if(glupIsEnabled(GLUP_LIGHTING)) {
	if(
	    glupIsEnabled(GLUP_TEXTURING) &&
	    glupIsEnabled(GLUP_NORMAL_MAPPING)
	) {
	    N = glup_texturing(vec4(1.0,1.0,1.0,1.0), tex_coord).xyz;
	    N = N-vec3(0.5,0.5,0.5);
	    N = normalize(GLUP.normal_matrix*N);
	    if(!gl_FrontFacing) {
		N = -N;
	    }
	} else {
#if GLUP_PRIMITIVE_DIMENSION==2		
	    if(glupIsEnabled(GLUP_VERTEX_NORMALS)) {
		N = normalize(normal);
		if(!gl_FrontFacing) {
		    N = -N;
		}
	    } else
#endif
	    {
		// Note: we still compute view space vertex
		// in vertex shader, because we seemingly do
		// not have sufficient precision to do the same
		// trick as in GLUP_GLSL (i.e. going back to
		// view space from clip space using projection
		// matrix).
		vec3 U = dFdx(vertex_view_space);                     
		vec3 V = dFdy(vertex_view_space);                 
		N = normalize(cross(U,V));
	    }
	}
    }
    
    glup_FragColor = glup_shading(
        color, tex_coord, N, int(primitive_id), mesh_tex_coord
    );
    glup_alpha_discard();
}                                                             
