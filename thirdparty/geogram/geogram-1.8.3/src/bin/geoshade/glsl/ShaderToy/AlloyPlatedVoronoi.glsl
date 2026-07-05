// https://www.shadertoy.com/view/XdfyWM

/*

	Alloy Plated Voronoi
	--------------------

    This was originally a rigid scrolling example, but I thought the movement looked more 
	interesting. Obviously, hard, fluid alloy doesn't make a lot of physical sense, so if the 
	situation really messes with your sense of correctness, uncomment the "RIGID_SCOLL" 
	define... or just pretend it's futuristic alien alloy from another dimension. :D

    From a technical perspective, it's a couple of smooth Voronoi layers (based on IQ's
	article below), split into fractional contours then bump mapped. Hardly cutting edge,
    and	not all that exciting. One of the things that did require some effort was producing 
	the succinct smooth borders with enough quality to allow for pronounced bump mapping
	with minimal artifacts. The results are sufficient, but there are probably more efficent 
	ways to achieve the same.

	I was going for a tarnished silver kind of look. Not sure whether it worked, but it looks
	shiny, so that's good enough for me. :) When I first started taking an interest in graphics, 
	my idea of producing a metallic looking object was to render it flat grey, then add some 
	ambient lighting... Yes, I was "that" naive. :D

	These days, I apply a little more science, but a lot of it is still made up. Specular 
	lighting and fake reflections help add to the illusion, but I find that	ramping the diffuse 
	light to a few powers tends to work best. Also, to give the light more angularity, I 
	accentuated the bump mapping more than I usually would.

	
	

	Similar Examples:

	Voronoi - smooth - iq
	https://www.shadertoy.com/view/ldB3zc

    // Great accompanying article.
    http://www.iquilezles.org/www/articles/smoothvoronoi/smoothvoronoi.htm


    // A cleaner, more simplistic rendering.
	Smooth Voronoi Contours - Shane
    https://www.shadertoy.com/view/4sdXDX


*/

//#define RIGID_SCROLL

// Gradient factor. See the "func" function. It's a fudge factor used to make the contour lines appear 
// unison in width. It's a mildly expensive calulation, and I'm reusing it, so it's global. I'll try to 
// localize it later.
float gF; 

//float sFract(float x, float sm){ float fx = fract(x); return fx - smoothstep(fwidth(x)*sm, 0., 1. - fx); }
//float sFract(float x, float sm){ float fx = fract(x); return min(fx, fx*(1. - fx)/gF/sm); }//fwidth(x)

// Based on Ollj's smooth "fract" formula.
float sFract(float x, float sm){
    
    // Extra smoothing factor. "1" is the norm.
    const float sf = .5; 
    
    // The hardware "fwidth" is cheap, but you could take the expensive route and
    // calculate it by hand if more quality was required.
    vec2 u = vec2(x, fwidth(x)*sf*sm);
    
    // Ollj's original formula with a transcendental term omitted.
    u.x = fract(u.x);
    u += (1. - 2.*u)*step(u.y, u.x);
    return clamp(1. - u.x/u.y, 0., 1.); // Cos term ommitted.
}



float sFloor(float x){ return x - sFract(x, 1.); } // Only works for nonnegative "x," which is fine here.


// Standard hue rotation formula with a bit of streamlining. 
vec3 rotHue(vec3 p, float a){

    vec2 cs = sin(vec2(1.570796, 0) + a);

    mat3 hr = mat3(0.299,  0.587,  0.114,  0.299,  0.587,  0.114,  0.299,  0.587,  0.114) +
        	  mat3(0.701, -0.587, -0.114, -0.299,  0.413, -0.114, -0.300, -0.588,  0.886) * cs.x +
        	  mat3(0.168,  0.330, -0.497, -0.328,  0.035,  0.292,  1.250, -1.050, -0.203) * cs.y;
							 
    return clamp(p*hr, 0., 1.);
}


// Standard 2x2 hash algorithm.
vec2 hash22(vec2 p) {
    
    // Faster, but probaly doesn't disperse things as nicely as other methods.
    float n = sin(dot(p, vec2(41, 289)));
    p = fract(vec2(2097152, 262144)*n);
    #ifdef RIGID_SCROLL
    return p - .5;
    #else
    return cos(p*6.283 + iTime)*.5;
    //return abs(fract(p+ iTime*.25)-.5)*2. - .5; // Snooker.
    //return abs(cos(p*6.283 + iTime))*.5; // Bounce.
    #endif

}


// vec3 to vec3 hash algorithm.
vec3 hash33(vec3 p) { 

    // Faster, but doesn't disperse things quite as nicely as the block below it. However, when framerate
    // is an issue, and it often is, this is the one to use. Basically, it's a tweaked amalgamation I put
    // together, based on a couple of other random algorithms I've seen around... so use it with caution,
    // because I make a tonne of mistakes. :)
    float n = sin(dot(p, vec3(7, 157, 113)));    
    return fract(vec3(2097152, 262144, 32768)*n)*2. - 1.; // return fract(vec3(64, 8, 1)*32768.0*n)*2.-1.; 

    // I'll assume the following came from IQ.
    //p = vec3( dot(p, vec3(127.1, 311.7, 74.7)), dot(p, vec3(269.5, 183.3, 246.1)), dot(p, vec3(113.5, 271.9, 124.6)));
    //return (fract(sin(p)*43758.5453)*2. - 1.);

}



// Cheap, streamlined 3D Simplex noise... of sorts. I cut a few corners, so it's not perfect, but it's
// artifact free and does the job. I gave it a different name, so that it wouldn't be mistaken for
// the real thing.
// 
// Credits: Ken Perlin, the inventor of Simplex noise, of course. Stefan Gustavson's paper - 
// "Simplex Noise Demystified," IQ, other "ShaderToy.com" people, etc.
float tetraNoise(in vec3 p){
    
    // Skewing the cubic grid, then determining the first vertice and fractional position.
    vec3 i = floor(p + dot(p, vec3(0.333333)) );  p -= i - dot(i, vec3(0.166666)) ;
    
    // Breaking the skewed cube into tetrahedra with partitioning planes, then determining which side of the 
    // intersecting planes the skewed point is on. Ie: Determining which tetrahedron the point is in.
    vec3 i1 = step(p.yzx, p), i2 = max(i1, 1.0-i1.zxy); i1 = min(i1, 1.0-i1.zxy);    
    
    // Using the above to calculate the other three vertices. Now we have all four tetrahedral vertices.
    vec3 p1 = p - i1 + 0.166666, p2 = p - i2 + 0.333333, p3 = p - 0.5;
  

    // 3D simplex falloff.
    vec4 v = max(0.5 - vec4(dot(p,p), dot(p1,p1), dot(p2,p2), dot(p3,p3)), 0.0);
    
    // Dotting the fractional position with a random vector generated for each corner -in order to determine 
    // the weighted contribution distribution... Kind of. Just for the record, you can do a non-gradient, value 
    // version that works almost as well.
    vec4 d = vec4(dot(p, hash33(i)), dot(p1, hash33(i + i1)), dot(p2, hash33(i + i2)), dot(p3, hash33(i + 1.)));
    
     
    // Simplex noise... Not really, but close enough. :)
    return clamp(dot(d, v*v*v*8.)*1.732 + .5, 0., 1.); // Not sure if clamping is necessary. Might be overkill.

}



// Smooth Voronoi. I'm not sure who came up with the original, but I think IQ
// was behind this particular algorithm. It's just like the regular Voronoi
// algorithm, but instead of determining the minimum distance, you accumulate
// values - analogous to adding metaball field values. The result is a nice
// smooth pattern. The "falloff" variable is a smoothing factor of sorts.
//
float smoothVoronoi(vec2 p, float falloff) {

    vec2 ip = floor(p); p -= ip;
	
	float d = 1., res = 0.;
	
	for(int i=-1; i<=2; i++) {
		for(int j=-1; j<=2; j++) {
            
			vec2 b = vec2(i, j);
            
			vec2 v = b - p + hash22(ip + b);
            
			d = max(dot(v,v), 1e-8);
			
			res += 1./pow(d, falloff );
            //res += exp( -16.*d ); // Alternate version.
		}
	}

	return pow(1./res, .5/falloff);
    //return clamp((-(1./16.)*log(res) + .1)/1.1, 0., 1.); // Alternate version.
}



// 2D function we'll be creating the fractional contours for. 
float func2D(vec2 p){
    
    #ifdef RIGID_SCROLL
    p += vec2(-.2, 0)*iTime; // Scrolling.
    #endif
    
    return smoothVoronoi(p*2., 4.)*.66 + smoothVoronoi(p*6., 4.)*.34;
    
}

// For fractional countours, something like "fract(func(p)*layers" will work, but the results 
// are aliased, so a smooth "fract" function needs to be applied. For nice, even contour lines,
// the functional curvature needs to be taken into account. Hence the "fxr" and "fyb" samples.
float func(vec2 p){
    
    float f = func2D(p); // Function value.
   
    // Samples in the X and Y directions to even up the contour lines according to curvature.
    vec2 e = vec2(8./iResolution.y, 0);
    float fxr = func2D(p - e.xy);
    float fyb = func2D(p - e.yx);
 
    // Gradient factor. Four samples would be better, but I'm trying to save some calculations.
    // I made "gF" global, for reuse purposes, but I'll try to rectify that later. See the "sFract"
    // function. Press pause, then set "gF" to a constant, like ".04," to see what it does.
    // This example would be a lot faster if "gF" was set to a constant (since the two samples 
    // above wouldn't be necessary), so if you like that aesthetic more, then that's the way to go.
    gF = length(f - vec2(fxr, fyb)); 
    
    const float palNum = 12.; // 12 contour lines.
    return sFract(f*palNum, 4.); // 4 is a smoothing factor. Getting the balance right can be frustrating.   
    
}

// Simple environment mapping. Pass the reflected vector in and create some
// colored noise with it.
vec3 envMap(vec3 rd){
    
   
    float c = tetraNoise(rd*3.)*.57 + tetraNoise(rd*6.)*.28 + tetraNoise(rd*12.)*.15; // Noise value.
    c = smoothstep(.4, 1., c); // Darken and add contast for more of a spotlight look.
    
    vec3 col = vec3(c*c*c, c*c, c); // Simple, cool coloring.
    //vec3 col = vec3(min(c*1.5, 1.), pow(c, 2.5), pow(c, 12.)); // Warm color.
    
    // Mix in the reverse color to tone it down and return.
    return mix(col, col.zxy, rd*.25 + .25); 
    
}



void mainImage(out vec4 fragColor, in vec2 fragCoord){

    // Screen coordinates.
	vec2 u = (fragCoord.xy - iResolution.xy*.5)/iResolution.y;
    
    // Function value.
    float f = func(u);
    float ssd = func2D(u); // Saving the unpalettized noise value to add a little gradient to the color, etc.
    
    // Four sample values around the original. Used for edging and highlighting. Note the "1.5" factor in a 
    // couple of samples. I was playing around and liked it more that way, but you can take it out if you want.
    vec2 e = vec2(2./450., 0);
    float fxl = func(u + e.xy*1.5);
    float fxr = func(u - e.xy);
    float fyt = func(u + e.yx*1.5);
    float fyb = func(u - e.yx);
  
    // Colorizing the the function value.
    vec3 col = vec3(.5);
    
    // Applying some color and hue rotation based on fractional layer and position. Designed to coincide
    // with the "func" function.
    const float palNum2 = 12.;
    float fi = (1. - clamp(sFloor(ssd*(palNum2))/(palNum2 - 1.), 0., 1.));
    fi = mod(fi, 4./12.);
    
    if(fi>3./12.) {
        // Extra color layers. I found it a bit much.
        //col *= vec3(1, .18, .28);
        //col = rotHue(col, mod(iTime/3. + 3.14159, 6.2831853) + (length(u*vec2(3., 5.))));        
        col *= .25;                   
    }
    else if(fi>2./12.) {
        
        col *= vec3(1, .18, .28);
        col = rotHue(col, mod(iTime/4., 6.2831853) + (length(u*vec2(2.5, 4.5))));// + iTime*.5
    }
    
    // Adding a bit of noise for a bit more authenticity.
    vec3 u3 = vec3(u, f); // Fake height, "ssd," according to function value.
    #ifdef RIGID_SCROLL
    u3.xy += vec2(-.2, 0)*iTime; // Scrolling.
    #endif
    col = min(col*.8 + tetraNoise(u3*128.)*.4, 1.);
    

   
    // Extra highlighting to shine up the edges. Purely for aesthetics. Not based on science. :)
    col += vec3(.5, .7, 1)*(max(f - fyt, 0.) + max(f - fxl, 0.))*1.*ssd*2.;

    
    vec3 rd = normalize(vec3(u, 1)); // Unit direction vector.
    vec3 n = normalize(vec3(0, 0, -1) + vec3(fxr - fxl, fyb - fyt, 0)/e.x/1.4*.01); // Bumped normal.
    vec3 ld = (vec3(.25, .25, -1.) - vec3(u, 0)); // Light direction - Position minus surface point.
    
    float dist = length(ld); // Light distance.
    float atten = 1./(1. + dist*dist*.25); // Light attenuation.
    
    ld /= dist; // Normalizing the light dirction vector.
    
    
    float diff = max(dot(ld, n), 0.); // Diffuse.
    diff = pow(diff, 4.)*.66 + pow(diff, 8.)*.34; // Ramped diffuse - for shininess.
    float spec = pow(max(dot(reflect(ld, n), rd), 0.), 8.); // Specular.
    float fres = pow(clamp(dot(rd, n) + 1., 0., 1.), 3.); // Fresnel.
    
    // Combining the above terms to produce the lit color.
    col = col*(diff*2. + .125) + vec3(1, .7, .3)*spec*2. + vec3(.25, .5, 1)*fres*2.;
    //col += vec3(.5, .7, 1)*diff*diff*.5; // Too much shine. :)


    // Fake environment mapping for that reflective look.
    col += (col*.65 + .35)*envMap(reflect(rd, vec3(0, 0, -1)*.25 + n*.75))*2.; 

    // Attenuating, then adding some brown shadowing for a subtle tarnished look.
    col *= atten*(vec3(f, pow(f, 1.1), pow(f, 1.2))*.85 + .15); 
    
    
    // col *= vec3(1.2, .95, .8); // Bronze, copper... kind of.
  
    
    // Subtle, bluish vignette.
    //u = fragCoord/iResolution.xy;
    //col = mix(vec3(0, .1, 1), col, pow( 16.0*u.x*u.y*(1.0-u.x)*(1.0-u.y) , .125)*.15 + .85);

 	
    // Rough gamma correction.
    fragColor = vec4(sqrt(clamp(col, 0., 1.)), 1);
    
}
