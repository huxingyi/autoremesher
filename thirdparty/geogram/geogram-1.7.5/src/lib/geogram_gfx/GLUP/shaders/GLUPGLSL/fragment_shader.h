//import <GLUP/current_profile/fragment_shader_preamble.h>
//import <GLUPGLSL/state.h>
//import <GLUP/stdglup.h>
//import <GLUP/current_profile/toggles.h>
//import <GLUP/current_profile/primitive.h>
//import <GLUP/fragment_shader_utils.h>

#ifndef GLUP_NO_GL_CLIPPING	
in float gl_ClipDistance[];                                
#endif

in VertexData {                            
    vec4 vertex_clip_space;                       
    vec4 color;                             
    vec4 tex_coord;
#if GLUP_PRIMITIVE_DIMENSION==2        
    vec3 normal;
#endif    
    vec4 mesh_tex_coord;
} FragmentIn;                              


bool is_triangle(in vec4 mesh_tex_coord) {
    if(
        !glupIsEnabled(GLUP_CLIPPING) ||
        GLUP.clipping_mode != GLUP_CLIP_SLICE_CELLS
    ) {
        switch(glup_primitive) {
        case GLUP_TRIANGLES:
            return true;
        case GLUP_QUADS:
            return false;
        case GLUP_TETRAHEDRA:
            return true;
        case GLUP_HEXAHEDRA:
            return false;
        }
    }
    return (
        (mesh_tex_coord.x +
         mesh_tex_coord.y +
         mesh_tex_coord.z +
         mesh_tex_coord.w) < 1.5
    );
}

float edge_factor(in vec4 mesh_tex_coord) {
    if(is_triangle(mesh_tex_coord)) {
        return edge_factor3(mesh_tex_coord.xyz);    
    } else {
        return edge_factor4(mesh_tex_coord);
    }
}

vec4 glup_draw_mesh(in vec4 color, in vec4 mesh_tex_coord) {
    return mix(
	GLUP.mesh_color, color, edge_factor(mesh_tex_coord)                  
    );
}

void main() {

#ifdef GLUP_GL_ES
#ifndef GLUP_NO_GL_CLIPPING    
    if(glupIsEnabled(GLUP_CLIPPING) && (gl_ClipDistance[0] < 0.0)) {
        discard;                                                
    }
#endif    
#endif
    
    if(glupIsEnabled(GLUP_PICKING)) {
        glup_FragColor = glup_picking(gl_PrimitiveID);        
        return;
    }
    
    vec4 result;
    if(glupIsEnabled(GLUP_VERTEX_COLORS)) {
        result = FragmentIn.color;
    } else {
        result = gl_FrontFacing ? GLUP.front_color : GLUP.back_color;        
    }

    if(glupIsEnabled(GLUP_TEXTURING) && !glupIsEnabled(GLUP_NORMAL_MAPPING)) {
        result = glup_texturing(result, FragmentIn.tex_coord);
    }
    if(glupIsEnabled(GLUP_LIGHTING)) {
	vec3 N;
	if(
	    glupIsEnabled(GLUP_TEXTURING) &&
	    glupIsEnabled(GLUP_NORMAL_MAPPING)
	){
	    N = glup_texturing(vec4(1.0,1.0,1.0,1.0), FragmentIn.tex_coord).xyz;
	    N = N-vec3(0.5,0.5,0.5);
	    N = normalize(GLUP.normal_matrix*N);
	    if(!gl_FrontFacing) {
		N = -N;
	    }
	} else {
#if GLUP_PRIMITIVE_DIMENSION==2    	
	    if(glupIsEnabled(GLUP_VERTEX_NORMALS)) {
		N = normalize(FragmentIn.normal);
		if(!gl_FrontFacing) {
		    N = -N;
		}
	    } else
#endif
	    {
		vec3 U = dFdx(FragmentIn.vertex_clip_space.xyz);
		vec3 V = dFdy(FragmentIn.vertex_clip_space.xyz);
		
		mat3 M = transpose(
		    mat3(
			GLUP.projection_matrix[0].xyz,
			GLUP.projection_matrix[1].xyz,
			GLUP.projection_matrix[2].xyz
		    )
		);

// I do not know why it is reversed, to be checked
// (maybe it is just my test program that does not
// have the same transform orientation, but then I
// do not understand why it gives the same result as
// the desktop version with GLUPES2...)		
#ifdef GLUP_GL_ES
		N = normalize(M*cross(U,V));		
#else		
		N = -normalize(M*cross(U,V));
#endif		
	    }
	}
	result = glup_lighting(result, N);
    }
    if(glupIsEnabled(GLUP_DRAW_MESH)) {
        result = glup_draw_mesh(result, FragmentIn.mesh_tex_coord);
    }                                                     
    glup_FragColor = result;
    glup_alpha_discard();
}


