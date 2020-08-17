//import <fullscreen/current_profile/fragment_shader_preamble.h>
//import <GLUP/defs.h>

glup_in  vec2 tex_coord;  

// Max radius to be used around pixel to estimate occlusion
uniform float max_radius;

// Multiplicative factor between two consecutive steps.
uniform float step_mul;

uniform sampler2D depth_texture;
uniform sampler2D random_texture;

const float PI = 3.14159265;

uniform mat4  mat_proj_inv;
uniform float shadows_gamma; 
uniform float shadows_intensity; 
uniform float depth_cueing; 
uniform float nb_directions;

#ifdef GL_ES

uniform vec2 depth_tex_size;
uniform vec2 random_tex_size;

#define width float(depth_tex_size.x)
#define height float(depth_tex_size.y)

#define r_width float(random_tex_size.x)
#define r_height float(random_tex_size.y)

#else

// Size of depth buffer
float width  = float(textureSize(depth_texture,0).x);
float height = float(textureSize(depth_texture,0).y);

// Size of random pattern
float r_width  = float(textureSize(random_texture,0).x);
float r_height = float(textureSize(random_texture,0).y);

#endif

//-------------------------------------------------
// Map 2D coordinate into world space.
//-------------------------------------------------

vec3 get_obj_coords(in vec2 where, in sampler2D texture) {
    vec4 p = vec4(where, glup_texture(texture, where).x, 1.); 
    // Map [0,1] to [-1,1]
    p.xyz = p.xyz * 2. - 1.;
    p = mat_proj_inv * p;
    if (p.w != 0.) { 
        p.xyz /= p.w; 
    }
    return vec3(p);
} 

//-------------------------------------------------
// Computes depth coordinate in world space.
//-------------------------------------------------

float get_obj_z(in vec2 where, in sampler2D texture) {
    float depth = glup_texture(texture, where).x;
    // For background points, return infinite value.
    if (depth >= 1.) { 
        return -10000.; 
    }
    // Map from range 0 to 1 to range -1 to 1 
    depth = depth * 2. - 1.;
    float z = (depth * mat_proj_inv[2][2] + mat_proj_inv[3][2]) / 
        (depth * mat_proj_inv[2][3] + mat_proj_inv[3][3]); 
    return z; 
} 

//-------------------------------------------------
// Tests whether a point falls outside a texture
//-------------------------------------------------

bool outside(in vec2 point) {
    return ( 
        point.x > 1. || point.x < 0.|| 
        point.y > 1. || point.y < 0.
    );
}

//-------------------------------------------------
// Horizon point
//-------------------------------------------------     

vec2 horizon_point(in vec2 from, in vec2 dir) {
    vec2 result;
    float horizon_delta = -100000.0;
    float from_z = get_obj_z(from, depth_texture);
    float step = (1.0 / width);
    float r = 2.0 * step;
    vec2 cur_point = from + r * dir;

#ifdef GL_ES
    for(int i=0; i<100; ++i) {
	if(!outside(cur_point)) {
#else    
    while (!outside(cur_point)) {
#endif	
        float z;
	z = get_obj_z(cur_point, depth_texture);        
                
        float delta_z = (z - from_z) / r;
        if( delta_z > horizon_delta) {
            horizon_delta = delta_z;
            result = cur_point;
        }
	if(r > max_radius) {
	    break; 
	}
        r += step;
        step *= step_mul;
        cur_point = from + r * dir;
    }

#ifdef GL_ES
    }
#endif	
    
    return result;
}

//-----------------------------------------------------------------------------
// Computes the angle between the horizon and the specified vector
//   in the specified direction.
// The vector needs to be normalized
//-----------------------------------------------------------------------------

float horizon_angle(in vec2 from, in vec3 from3D, in vec2 dir, in vec3 normal) {
    vec3 horizon =
    get_obj_coords(horizon_point(from, dir), depth_texture) - from3D;
    return acos ( dot(normal, horizon) / length(horizon) );
}

//------------------------------------------------------------------------------
// Generates an image-space noise.
//------------------------------------------------------------------------------

float my_noise() {
    float x = tex_coord.x * width  / r_width;
    float y = tex_coord.y * height / r_height;
    return glup_texture(random_texture, vec2(x,y)).x;
}

float ambient_occlusion(in vec2 from) {       
    float angle_step = 2.0 * PI / (nb_directions);
    float cur_angle = my_noise() * 2. * PI ; 
    float occlusion_factor = 0.0;
    vec3 from3D = get_obj_coords(from, depth_texture);
#ifdef GL_ES
    for (int i=0; i < 7; i++) {	    
#else
    for (int i=0; i < nb_directions; i++) {
#endif	
        vec2 dir = vec2(cos(cur_angle), sin(cur_angle));
        float h_angle = horizon_angle(from, from3D, dir, vec3(0., 0., 1.));
        cur_angle += angle_step;
        occlusion_factor += h_angle;
    }
    return occlusion_factor / (float(nb_directions) * (PI / 2.0));
}

float compute_depth_cueing(vec2 uv) {
    if(depth_cueing == 0.0) { 
        return 0.0; 
    }       
    float depth = glup_texture(depth_texture,tex_coord).x;
    return depth < 1.0 ? depth_cueing * depth : 0.0;
}

void main() {
    float g = 1.0;
    if(glup_texture(depth_texture, tex_coord).x < 1.0) {
        g = ambient_occlusion(tex_coord);
        g = shadows_intensity *
        pow(g, shadows_gamma) - compute_depth_cueing(tex_coord);
    }
    glup_FragColor.rgb = vec3(0.0,0.0,0.0);
    glup_FragColor.a   = 1.0 - g;
}

