//import <GLUP/current_profile/fragment_shader_preamble.h>
//import <GLUP/defs.h>

#if defined(GLUP150) || defined (GLUP440)
  in VertexData {
     vec4 color;
     vec4 tex_coord;
     vec3 normal;
  } FragmentIn;
  vec4 tex_coord = FragmentIn.tex_coord;
#elif defined(GLUPES2)               
  glup_in vec4 tex_coord; 
#endif

