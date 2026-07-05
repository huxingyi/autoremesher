struct VSUniformState {              
    mat3 normal_matrix;             
    mat4 modelviewprojection_matrix; 
    mat4 modelview_matrix;
    mat4 projection_matrix;
    mat4 texture_matrix;
    mat4 inverse_modelview_matrix;
    mat4 inverse_projection_matrix;
    mat4 inverse_modelviewprojection_matrix;
    vec4  world_clip_plane;          
    float point_size;
    vec4  viewport;
};

uniform VSUniformState GLUP_VS;     

