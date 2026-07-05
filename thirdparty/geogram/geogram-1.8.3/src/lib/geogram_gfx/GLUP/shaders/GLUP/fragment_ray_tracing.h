
struct Ray {
    vec3 O; // Origin
    vec3 V; // Direction vector
};

// Notes: GLUP.viewport = [x0,y0,width,height]
// clip-space coordinates are in [-1,1] (not [0,1]) !

// Computes the ray that passes through the current fragment
// The ray is in world space.
Ray glup_primary_ray() {
    vec4 near = vec4(
	2.0 * ( (gl_FragCoord.x - GLUP.viewport[0]) / GLUP.viewport[2] - 0.5),
	2.0 * ( (gl_FragCoord.y - GLUP.viewport[1]) / GLUP.viewport[3] - 0.5),
        0.0,
        1.0
    );
    near = GLUP.inverse_modelviewprojection_matrix * near ;
    vec4 far = near + GLUP.inverse_modelviewprojection_matrix[2] ;
    near.xyz /= near.w ;
    far.xyz /= far.w ;
    return Ray(near.xyz, far.xyz-near.xyz) ;
}

// Updates fragment depth from a point in world space
void glup_update_depth(in vec3 M_world_space) {
    vec4 M_clip_space = GLUP.modelviewprojection_matrix * vec4(M_world_space,1.0);
    float z = 0.5*(1.0 + M_clip_space.z/M_clip_space.w);
    glup_FragDepth = (1.0-z)*gl_DepthRange.near + z*gl_DepthRange.far;
}

