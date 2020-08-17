

#ifdef GLUP_ES_100
// converts an integer into an ivec4. The components of
// the ivec4 contains the 4 bytes of the integer.
ivec4 int_to_ivec4(in int x) {
    // TODO: we still have unsigned-signed conversion
    // problems. Picking id in state should be unsigned.
    if(x < 0) {
        return ivec4(255,255,255,255);
    }
    int w = x;                        
    int R = glup_mod(w, 256);         
    w /= 256;                         
    int G = glup_mod(w, 256);         
    w /= 256;                         
    int B = glup_mod(w, 256);                 
    w /= 256;                         
    int A = glup_mod(w, 256);         
    return ivec4(R,G,B,A);            
}                                    
#else
ivec4 int_to_ivec4(in int x) {
    return ivec4(
         x        & 255,
        (x >>  8) & 255,
        (x >> 16) & 255,
        (x >> 24) & 255
    );
}
#endif

// adds two 32 bits integers V1 and V2 represented
// as ivec4.
ivec4 ivec4_add(in ivec4 V1, in ivec4 V2) {
    int R = V1.r + V2.r;                    
    int carry = R/256;                      
    R -= carry*256;                         
    int G = V1.g + V2.g + carry;            
    carry = G/256;                          
    G -= carry*256;                         
    int B = V1.b + V2.b + carry;            
    carry = B/256;                          
    B -= carry*256;                         
    int A = V1.a + V2.a + carry;            
    return ivec4(R,G,B,A);                  
}                                          

// converts a 32 bit represented by an ivec4
// into a color.
vec4 ivec4_to_vec4(in ivec4 V) { 
    return vec4(                  
        float(V.r)/255.0,          
        float(V.g)/255.0,          
        float(V.b)/255.0,          
        float(V.a)/255.0                   
    );                            
}                                

// converts a 32 bit represented by an int
// into a color.
highp vec4 int_to_vec4(in int x) {        
    return ivec4_to_vec4(int_to_ivec4(x)); 
}                                         

float min3(in float x, in float y, in float z) {                            
    return min(min(x,y),z);                                         
}                                                                  

float min4(in float x, in float y, in float z, in float w) {                   
    return min(min(x,y),min(z,w));                                  
}                                                                  
        
float edge_factor1(in float bary) {                                   
    float d = fwidth(bary);                                         
    float a = smoothstep(0.0, d*GLUP.mesh_width, bary);             
    return a;                                                       
}                                                                  

float edge_factor3(in vec3 bary) {                                    
    vec3 d = fwidth(bary);                                          
    vec3 a = smoothstep(                                            
        vec3(0.0, 0.0, 0.0), d*GLUP.mesh_width, bary                 
    );                                                              
    return min3(a.x, a.y ,a.z);                                     
}                                                                  

float edge_factor4(in vec4 bary) {                                    
    vec4 d = fwidth(bary);                                          
    vec4 a = smoothstep(                                            
        vec4(0.0, 0.0, 0.0, 0.0), d*GLUP.mesh_width, bary            
    );                                                              
    return min4(a.x, a.y, a.z, a.w);                                
}                                                                  

vec4 glup_picking(in highp int primitive_id) {
    vec4 result;
    if(GLUP.picking_mode == GLUP_PICK_PRIMITIVE) {       
        ivec4 A4 = int_to_ivec4(primitive_id);
        // TODO: this one will not work if GPU has lowprec fshaders
        ivec4 B4 = int_to_ivec4(GLUP.base_picking_id);    
        result = ivec4_to_vec4(ivec4_add(A4,B4)); 
    } else {                                             
        result = int_to_vec4(GLUP.picking_id);    
    }
    return result;
}

vec4 glup_texturing(in vec4 color, in vec4 tex_coord) {
    vec4 result = color;
    vec4 tex_color;                                       
    if(GLUP.texture_type == GLUP_TEXTURE_1D) {            
        tex_color = glup_texture(                        
            texture1Dsampler, tex_coord.xy               
        );                                               
    } else if(GLUP.texture_type == GLUP_TEXTURE_2D) {
        tex_color = glup_texture(                        
            texture2Dsampler, tex_coord.xy              
        );                                               
    }
#ifdef GLUP_NO_TEXTURE_3D
    else {
	tex_color = vec4(1.0, 0.0, 0.0, 1.0);
    }
#else
    else if(GLUP.texture_type == GLUP_TEXTURE_3D) {            
        tex_color = texture(                                     
            texture3Dsampler, tex_coord.xyz           
        );                                                       
    }
#endif
    if(glupIsEnabled(GLUP_INDIRECT_TEXTURING)) {                           
        tex_color = GLUP.texture_matrix * tex_color;             
        tex_color = glup_texture(texture1Dsampler, tex_color.xy);     
    } 
    if(GLUP.texture_mode == GLUP_TEXTURE_REPLACE) {       
        result = tex_color;                      
    } else if(GLUP.texture_mode==GLUP_TEXTURE_MODULATE) { 
        result *= tex_color;                     
    } else {                                              
        result += tex_color;                     
    }
    return result;
}

vec4 glup_lighting(in vec4 color, in vec3 normal) {
    vec4 result = color;
    float diff = dot(normal,GLUP.light_vector);                   
    if(diff > 0.0) {
        result.rgb = diff*result.rgb + vec3(0.2, 0.2, 0.2);
	if(GLUP.specular > 0.0) {
	    float spec = dot(normal,GLUP.light_half_vector);
	    spec = pow(spec, 30.0);
	    result.rgb += GLUP.specular*spec*vec3(1.0, 1.0, 1.0);
	}
    } else {                                                        
        result.rgb = vec3(0.2, 0.2, 0.2);
    }
    return result;
}


void glup_alpha_discard() {
    if(glupIsEnabled(GLUP_ALPHA_DISCARD)) {
	if(glup_FragColor.a < GLUP.alpha_threshold) {
	    discard;
	}
    }
}

