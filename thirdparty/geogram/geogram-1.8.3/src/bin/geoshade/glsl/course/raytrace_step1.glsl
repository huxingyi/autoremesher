// Raytracing tutorial step 1:
// Let us draw a red sphere
//
// Try this: move the "head" up and down, left and right,
// by changing the point you are looking at in mainImage()

// The viewing parameters
//
struct Camera {
    vec3 Obs;    // Position of the "observer"
    vec3 View;   // Viewing vector
    vec3 Up;     // Up direction
    vec3 Horiz;  // Horizontal direction
    float H;     // Image height (in pixels)
    float W;     // Image width (in pixel)
    float z;     // Location of the viewing plane along the View vector
};

// Initializes a camera
// Obs:    location of the observer
// LookAt: a 3D point the observer is looking at
// aperture: camera aperture in degrees
//
Camera camera(in vec3 Obs, in vec3 LookAt, in float aperture) {
   Camera C;
   C.Obs = Obs;
   C.View = normalize(LookAt - Obs);
   C.Horiz = normalize(cross(vec3(0.0, 0.0, 1.0), C.View));
   C.Up = cross(C.View, C.Horiz);
   C.W = float(iResolution.x);
   C.H = float(iResolution.y);
   C.z = (C.H/2.0) / tan((aperture * 3.1415 / 180.0) / 2.0);
   return C;
}


struct Ray {
    vec3 Origin; // Origin of the ray (for now, 
                 //  it will be the observer, more to come...)
    vec3 Dir;    // Direction of the ray (not necessarily normalized)
};


// Launches a ray from the camera
// C the camera
// XY the coordinates of the pixel, in [0..width]x[0..height]
//
Ray launch(in Camera C, in vec2 XY) {
   return Ray(
      C.Obs,
      C.z*C.View+(XY.x-C.W/2.0)*C.Horiz+(XY.y-C.H/2.0)*C.Up 
   );
}


struct Sphere {
   vec3 Center; // Center of the sphere
   float R;     // Radius of the sphere
};

// Tests whether there is an intersection between a ray and
// a sphere
//   in  R: the ray
//   in  S: the sphere
//   out t: when there is an intersection, it is given by R.Origin + t*R.Dir
//   returns true if there is an intersection, false otherwise
//
bool intersect_sphere(in Ray R, in Sphere S, out float t) {
   // How to find the intersection between a ray and a sphere:
   // (1) Equation of the sphere: ||M - S.Center||^2 = S.R^2 
   //      (or dot(M - S.Center, M - S.Center) = S.R*S.R)
   // (2) Equation of the ray:    M = R.Origin + t * R.Dir
   //      (imagine that a photon starts at R.Origin at time t=0,
   //       at time t its location is R.Origin + t * R.Dir)
   // Now inject (2) into (1), expand, collect, you end up with
   // a quadratic equation in t: a t^2 + b t + c = 0, solve for
   // t. There can be 0,1 or 2 solutions (0, 1 or 2 intersections
   // between a straight line and a sphere). Take the nearest one.
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

// This function is called for each pixel of the image
// in fragCoord : the coordinates of the pixel, in [0..width]x[0..height]
// out fragColor: the computed color for the pixel
//
void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
 
   Camera C = camera(
       vec3(2.0, 2.0, 1.5), // You are here
       vec3(0.0, 0.0, 0.0), // You are looking at this point
       50.0                 // Camera aperture = 50 degrees
   );
   Ray R = launch(C, fragCoord);
   Sphere S = Sphere(vec3(1.3, 0.0, 0.0), 0.2);

   // We initialize the color with the background.
   vec3 color = vec3(0.2, 0.2, 0.5);
   
   // If there was an intersection with the sphere,
   // we paint the pixel in green.
   float t;
   if(intersect_sphere(R,S,t)) {
      color = vec3(0.0, 1.0, 0.5);   			       
   }

   // This writes the color to the pixel (the 1.0
   // arg. is for transparency, not used here).
   fragColor = vec4(color, 1.0);
}

