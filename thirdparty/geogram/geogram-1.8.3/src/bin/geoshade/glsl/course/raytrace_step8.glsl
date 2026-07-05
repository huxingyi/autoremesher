const float FARAWAY=1e30;
const float EPSILON=1e-5;

// The viewing parameters
// Gathers all parameters needed to launch primary rays
struct Camera {
    vec3 Obs;    // The position of the observer
    vec3 View;   // Unit vector, points to the target
    vec3 Up;     // Unit vector, vertical direction
    vec3 Horiz;  // Unit vector, horizontal direction
    float H;     // Screen height in pixels
    float W;     // Screen width in pixel
    float z;     // offset of the screen along View
};

// \brief A ray, in parametric form. 
struct Ray {
    vec3 Origin;
    vec3 Dir;
};

// \brief Initializes a Camera
// \param[in] Obs the position of the observer
// \param[in] Target the point that will be in the center
// \param[in] aperture the aperture angle in degrees
// \return the initialized Camera
Camera camera(in vec3 Obs, in vec3 Target, in float aperture) {
   Camera C;
   C.Obs = Obs;
   C.View = normalize(Target - Obs);
   C.Horiz = normalize(cross(vec3(0.0, 0.0, 1.0), C.View));
   C.Up = cross(C.View, C.Horiz);
   C.W = float(iResolution.x);
   C.H = float(iResolution.y);
   C.z = (C.H/2.0) / tan((aperture * 3.1415 / 180.0) / 2.0);
   return C;
}

// \brief Launches a primary Ray
// \param[in] C the Camera
// \param[in] XY the pixel coordinates in [0,W-1] x [0,H-1]
Ray launch(in Camera C, in vec2 XY) {
   return Ray(
      C.Obs,
      C.z*C.View+(XY.x-C.W/2.0)*C.Horiz+(XY.y-C.H/2.0)*C.Up 
   );
}

// \brief A sphere, defined by its center and radius
struct Sphere {
   vec3 Center;
   float R;
};

// \brief Material gathers all shading properties
struct Material {
    vec3 Kd;       // diffuse color
    vec3 Ke;       // emissive color
    vec3 Kr;       // reflective material
    float checker; // checkerboard size
    vec3 Ks;       // specular
    float s;       // specular factor
    vec3 Kt;       // transmission
    float n;       // refraction index
};

// \brief Shorthand for the zero vector
const vec3 zero3 = vec3(0.0, 0.0, 0.0);

// \brief Creates a diffuse material
// \param[in] Kd the diffuse color
// \return the created Material
Material diffuse(in vec3 Kd) {
   return Material(Kd, zero3, zero3, 0.0, zero3, 0.0, zero3, 0.0);
}

// \brief Creates a light (emissive) material
// \param[in] Ke the color of the light
// \return the created Material
Material light(in vec3 Ke) {
   return Material(zero3, Ke, zero3, 0.0, zero3, 0.0, zero3, 0.0);
}

// \brief Creates a mirror material
// \param[in] Kd the diffuse color
// \param[in] Kr the reflection coefficient
// \return the created Material
Material mirror(in vec3 Kd, in vec3 Kr) {
   return Material(
     Kd, zero3, Kr, 0.0, vec3(1.0, 1.0, 1.0), 30.0, zero3, 0.0
   );
}

Material transparent(in vec3 Kd, in vec3 Kt) {
   return Material(
     Kd, zero3, zero3, 0.0, vec3(1.0, 1.0, 1.0), 30.0, Kt, 0.9
   );
}



// \brief Creates a shiny material
// \param[in] Kd the diffuse color
// \param[in] Ks the specular coefficient
// \return the created Material
Material shiny(in vec3 Kd, in vec3 Ks) {
   return Material(Kd, zero3, zero3, 0.0, Ks, 30.0, zero3, 0.0);
}

// \brief Creates a checkerboard material
// \param[in] Kd the diffuse color
// \param[in] sz size of the checkers
// \return the created Material
Material checkerboard(in vec3 Kd, in float sz) {
   return Material(Kd, zero3, zero3, sz, zero3, 0.0, zero3, 0.0);
}

// \brief An Object, with a shape (Sphere) and a Material
struct Object {
   Sphere sphere;
   Material material;
};

// \brief The scene is stored in a global array
Object scene[23];

// \brief Initializes the scene
void init_scene() {

   scene[0] = Object(
      Sphere(vec3(0.0, 0.0, 0.0),0.5), 
  //    transparent(vec3(0.2, 0.5, 0.2), vec3(0.3, 0.7, 1.0))
        mirror(vec3(0.2, 0.5, 0.2), vec3(0.9, 0.9, 0.9))
   );

   scene[1] = Object(
      Sphere(vec3(0.0, 0.0, -10000.0),9999.5),
      checkerboard(vec3(1.0, 0.2, 0.5), 0.1)
   );

   scene[2] = Object(
      Sphere(vec3(1.0, 0.0, 1.0),0.02),
      light(vec3(1.0, 1.0, 1.0)) 
   );

   for(int i=0; i<20; ++i) {
     float beta = float(iFrame)/30.0 + float(i)*6.28/19.0;
     float s = sin(beta);
     float c = cos(beta); 

     scene[i+3] = Object(
        Sphere(vec3(0.7*s, 0.7*c, 0.0),0.1), 
        (((i >> 2) & 1) == 0) ?
        mirror(vec3(0.2, 0.5, 0.2), vec3(0.9, 0.9, 0.9))
        :
        transparent(vec3(0.2, 0.5, 0.2), vec3(0.3, 0.3, 1.0))
     );
   }
}

// \brief Computes a Ray Sphere intersection
// \param[in] R the Ray
// \param[in] S the Sphere
// \param[out] t the intersection parameter
// \retval true if there was an intersection
// \retval false otherwise
bool intersect_sphere(in Ray R, in Sphere S, out float t) {
   vec3 CO = R.Origin - S.Center;
   float a = dot(R.Dir, R.Dir);
   float b = 2.0*dot(R.Dir, CO);
   float c = dot(CO, CO) - S.R*S.R;
   float delta = b*b - 4.0*a*c;
   if(delta < 0.0) {
      return false;
   }
   t = (-b-sqrt(delta)) / (2.0*a);
   return true;
}

bool intersect_sphere_2(in Ray R, in Sphere S, out float t) {
   vec3 CO = R.Origin - S.Center;
   float a = dot(R.Dir, R.Dir);
   float b = 2.0*dot(R.Dir, CO);
   float c = dot(CO, CO) - S.R*S.R;
   float delta = b*b - 4.0*a*c;
   if(delta < 0.0) {
      return false;
   }
   t = (-b+sqrt(delta)) / (2.0*a);
   return true;
}



// \brief Computes a reflected Ray
// \param[in] I the incident Ray
// \param[in] P the point at which the reflection occurs
// \param[in] N the normal at P
// \return the reflected Ray
Ray reflect_ray(in Ray I, in vec3 P, in vec3 N) {
    return Ray(
      P,
      -2.0*dot(N,I.Dir)*N + I.Dir
   );
}

Ray refract_ray(in Ray I, in vec3 P, in vec3 N, in float n) {
   return  Ray(
     P,
     refract(normalize(I.Dir), N, n)
   );
}

// \brief Tests whether a Ray is in shadow
// \param[in] R a Ray that connects a point to a lightsource
// \retval true if the point is in shadow w.r.t. the lightsource
// \retval false otherwise
bool shadow(in Ray R) {
   for(int i=0; i<scene.length(); ++i) {
        float t;
        if(
          scene[i].material.Ke == vec3(0.0, 0.0, 0.0) &&
          intersect_sphere(R, scene[i].sphere, t) &&
          t > EPSILON && t < 1.0
        ) {
          return true;
        }
    }
    return false;
}

// \brief Computes the lighting
// \param[in] P the intersection point
// \param[in] N the normal to the intersected surface at P
// \param[in] material the material
// \param[in] Ray the incident Ray
// \return the computed color
vec3 lighting(
   in vec3 P, in vec3 N, in Material material, in Ray R
) {

   // If it is a lightsource, then return its color
   // (and we are done) 
   if(material.Ke != vec3(0.0, 0.0, 0.0)) {
      return material.Ke;
   }  

   vec3 result = vec3(0.0, 0.0, 0.0);

   // Compute the influence of all lightsources
   for(int i=0; i<scene.length(); ++i) {
      if(scene[i].material.Ke != vec3(0.0, 0.0, 0.0)) {
         Ray R2 = Ray(P, scene[i].sphere.Center);
         if(!shadow(R2)) {
           vec3 E = scene[i].sphere.Center - P;
  
           // Diffuse lighting
           float lamb = max(0.0, dot(E,N) / length(E));
           vec3 Kd = material.Kd;
           if(material.checker != 0.0 && 
              sin(P.x/material.checker)*
              sin(P.y/material.checker) > 0.0) {
               Kd = vec3(1.0, 1.0, 1.0) - Kd;
           }
           result += lamb * Kd * scene[i].material.Ke;

           // Specular lighting
           if(material.Ks != zero3) {
               vec3 Er = 2.0*dot(N,E)*N - E;
               vec3 View = R.Origin - P;
               float spec=max(dot(Er,View),0.0);
               spec /= sqrt(dot(Er,Er)*dot(View,View));
               spec = pow(spec, material.s);
               result += 
                  spec * material.Ks * scene[i].material.Ke;
           }
         }
      }
   }

   return result;
}

// \brief Computes the nearest intersection along a Ray
// \param[in] R the ray
// \param[out] P the intersection point
// \param[out] N the normal to the intersected surface at P
// \param[out] material the material of the intersected object
bool nearest_intersection(
   in Ray R, 
   out vec3 P, out vec3 N, out Material material
) {
   const float FARAWAY=1e30; 
   float t = FARAWAY;

   for(int i=0; i<scene.length(); ++i) {
       float cur_t;
       if(
          intersect_sphere(R, scene[i].sphere, cur_t) 
          && cur_t < t && cur_t > EPSILON 
       ) {
           t = cur_t;
           P = R.Origin + t*R.Dir;
           N = normalize(P - scene[i].sphere.Center);
           material = scene[i].material;
       } else if( 
          intersect_sphere_2(R, scene[i].sphere, cur_t) 
          && cur_t < t && cur_t > EPSILON       
       ) {
           t = cur_t;
           P = R.Origin + t*R.Dir;
           N = -normalize(P - scene[i].sphere.Center);
           material = scene[i].material;
           material.n = 1.0 / material.n;
           material.Kt = vec3(1.0, 1.0, 1.0) * pow(length(R.Dir) / (4.0*t), 2.0);
       }
   }
   return (t != FARAWAY);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {

   // Yes, it is a bit stupid to call this for each pixel,
   // but well, does not cost that much...
   init_scene();

   float beta = float(iFrame)/150.0;
   float s = sin(beta);
   float c = cos(beta); 

   // Initialize the Camera (and make it orbit around the
   // origin)
   Camera C = camera(
       vec3(2.0*c, 2.0*s, 0.5),
       vec3(0.5, 0.5, 0.5),
       50.0       
   );

   // Lauch the primary ray that corresponds to this pixel
   Ray R = launch(C, fragCoord);
   
   
   fragColor = vec4(0.0, 0.0, 0.0, 1.0);

   vec3 P;  // Current intersected point
   vec3 N;  // Normal to the intersected object at P
   Material material; // Material of the intersected object
 
   // Compute up to 5 ray bounces
   vec3 Kr_cumul = vec3(1.0, 1.0, 1.0);
   for(int k=0; k<5; ++k) {
       if(nearest_intersection(R, P, N, material)) {
          fragColor.rgb += Kr_cumul * lighting(P,N,material,R);
          if(material.Kr != zero3) {
             Kr_cumul *= material.Kr;
             R = reflect_ray(R, P, N);
          } else if(material.Kt != zero3) {
             Kr_cumul *= material.Kt;
             R = refract_ray(R, P, N, material.n);
          } else {
            break;
          }
       } else {
          fragColor.rgb += Kr_cumul * vec3(0.5, 0.5, 1.0);
          break;
       }
   }  
}









