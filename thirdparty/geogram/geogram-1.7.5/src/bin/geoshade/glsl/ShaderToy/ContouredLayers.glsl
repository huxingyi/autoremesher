// https://www.shadertoy.com/view/3lj3zt

/*

	Contoured Layers
	----------------

	Constructing some concise contoured layers, then applying various edge and shading 
	effects to produce some faux depth. Technically, that's what is happening here, but
	this example was mainly put together as a means to demonstrate various layering
    effects, like strokes, highlights, shadows, etc. No 3D was harmed during the making
	of this example. :)

	I love those contoured noise-based paper layer images that various graphic artists
	from places like Adobe distribute with their applications. Most consist of some
	antialised noise layers rendered in a flat tone with drop shadows for each. The 
	fancier ones sometimes include highlighted edges, etc, which is what I've put 
	together here. None of it is difficult to produce, provided you're happy with
	concept of smoothing layers at a particular threshold with respect to the field
	derivative.

	I put in a few diferent aesthetic options to try, just to show how much something
	like a simple palette change, drop shadow, etc, can effect the overall feel. 
	Anyway, feel free to play around with the defines below. At some stage, I might
    render some icons and allow the various options to be manipulated via the mouse,
	but for now, this will suffice.

	I also have a few raymarched 3D versions that I'll put up at a later date.	


*/

// Dropping down a blurry dark layer to give a fake ambient occlusion effect.
// It's subtle, but gives things a bit more depth. However, turning it off gives
// a crisper look. I guess it depends what you're after.
#define FAKE_AO

// Controur strokes are great for that hand drawn look, or just to give some definition
// to geometry. This one is dark, but it can be any color.
#define STROKE

// Highlights, to give the impression that light is hitting the surface.
#define HILIGHT

// Shadows: There aren't too many times when I wouldn't want shadows, but I can think
// of a few. If expense if a problem, you can fake it with a thicker AO layer, but it's
// not quite the same.
#define SHADOW

// Include the metallic texture. I overuse this particular texture, but it's the only
// one on Shadertoy with a fine enough grade on it. I'm hoping more subtle textures
// will get added at some stage. :)
// #define TEXTURED

// Running a cheap hatch-like algorithm over the layers for a bit of extra texture.
#define HATCH

// Very subtle paper grain. It's pretty simple, and I think it came from one of
// Flockaroo's examples.
#define PAPER_GRAIN

// Palette: It's amazing how something as simple as color choice can effect the feel
// of an image.
// Settings: Greyscale: 0, Red: 1, Blue: 2, Earth: 3, Pink and grey: 4.
#define PALETTE 3




// Standard 2D rotation formula.
mat2 rot2(in float a){ float c = cos(a), s = sin(a); return mat2(c, -s, s, c); }

// vec3 to float hash.
float hash21( vec2 p ){ 
     
    return fract(sin(dot(p, vec2(41, 289)))*45758.5453); 
   
    //p.x = fract(sin(dot(p, vec2(1, 113)))*45758.5453);
    //return sin(p.x*6.2831853 + iTime); 
}

// vec2 to vec2 hash.
vec2 hash22(vec2 p) { 

    // Faster, but doesn't disperse things quite as nicely. However, when framerate
    // is an issue, and it often is, this is a good one to use. Basically, it's a tweaked 
    // amalgamation I put together, based on a couple of other random algorithms I've 
    // seen around... so use it with caution, because I make a tonne of mistakes. :)
    float n = sin(dot(p, vec2(1, 113)));
    //return fract(vec2(262144, 32768)*n)*2. - 1.; 
    
    // Animated.
    p = fract(vec2(262144, 32768)*n); 
    // Note the ".45," insted of ".5" that you'd expect to see. When edging, it can open 
    // up the cells ever so slightly for a more even spread. In fact, lower numbers work 
    // even better, but then the random movement would become too restricted. Zero would 
    // give you square cells.
    return sin( p*6.2831853 + iTime); 
    
}



// Cheap and nasty 2D smooth noise function with inbuilt hash function -- based on IQ's 
// original. Very trimmed down. In fact, I probably went a little overboard. I think it 
// might also degrade with large time values, but that's not an issue here.
float n2D(vec2 p) {

	vec2 i = floor(p); p -= i; p *= p*(3. - p*2.);  
    
	return dot(mat2(fract(sin(vec4(0, 1, 113, 114) + dot(i, vec2(1, 113)))*43758.5453))*
                vec2(1. - p.y, p.y), vec2(1. - p.x, p.x) );

}


// Based on IQ's gradient noise formula.
float n2D3G( in vec2 p ){
   
    vec2 i = floor(p); p -= i;
    
    vec4 v;
    v.x = dot(hash22(i), p);
    v.y = dot(hash22(i + vec2(1, 0)), p - vec2(1, 0));
    v.z = dot(hash22(i + vec2(0, 1)), p - vec2(0, 1));
    v.w = dot(hash22(i + 1.), p - 1.);

#if 1
    // Quintic interpolation.
    p = p*p*p*(p*(p*6. - 15.) + 10.);
#else
    // Cubic interpolation.
    p = p*p*(3. - 2.*p);
#endif

    return mix(mix(v.x, v.y, p.x), mix(v.z, v.w, p.x), p.y);
    //return v.x + p.x*(v.y - v.x) + p.y*(v.z - v.x) + p.x*p.y*(v.x - v.y - v.z + v.w);
}



// The map function. Just two layers of gradient noise. Way more interesting
// functions are possible, but we're keeping things simple.
float map(vec3 p, float i){
    
    return n2D3G(p.xy*3.)*.66 + n2D3G(p.xy*6.)*.34 + i/10.*1. - .15;
  
}


// 2D derivative function.
vec3 getNormal(in vec3 p, float m, float i) {
	
    vec2 e = vec2(.001, 0);
    
    // Four extra samples. Slightly better, but not really needed here.
	//return (vec3(map(p + e.xyy, i) - map(p - e.xyy, i), map(p + e.yxy, i) - map(p - e.yxy, i),	0.))/e.x*.7071;

    // Three samples, but only two extra sample calculations. 
    return (vec3(m - map(p - e.xyy, i), m - map(p - e.yxy, i),	0.))/e.x*1.4142;
}

// The map layer and its derivative. To produce constant width layer edges, the derivative
// is necessary, so the distance field value and the derivative at the point is returned.
vec4 mapLayer(in vec3 p, float i){
    
    vec4 d;
    
    d.x = map(p, i); // Distance field value.
    
    d.yzw = getNormal(p, d.x, i); // Derivative.
    
    return d;
    
}



// Layer color. Based on the shade, layer number and smoothing factor.
vec3 getCol(vec2 p, float sh, float fi, float sf){
     
    // Color.
    vec3 col;

    
    #if PALETTE == 0
        // Light attenuation palette.
    	col = vec3(1)*(1. - .75/(1. + sh*sh*2.));
    	//col = vec3(sh*sh*.65 + .22);
        //col = vec3(sh*.5 + .2); 
        // Etc.
    #elif PALETTE == 1
        col = pow(min(vec3(1.5, 1, 1)*(sh*.35 + .6), 1.), vec3(1, 3, 16));
        if(fi==0.) col = vec3(.35, .05, .3);
        col = mix(col.xzy, col, sh*.5 + .5);
    #elif PALETTE == 2
        col = pow(min(vec3(1.5, 1, 1)*(sh*.35 + .6), 1.), vec3(1, 3, 16));
        if(fi==0.) col = vec3(.6, .2, .07);
        col = mix(col.xzy, col, sh*.5 + .5).zyx;
    #elif PALETTE == 3
        if(fi==0.) col = vec3(.25, .52, .75);
        if(fi==1.) col = vec3(.8, .8, .6);
        if(fi==2.) col = vec3(.75, .6, .5);
        if(fi==3.) col = vec3(.6, .58, .5);
        if(fi==4.) col = vec3(.5, .72, .4);
        if(fi==5.) col = vec3(.65, .85, .5);
    #else
    	if(mod(fi, 2.)<.5) col = vec3(.25, .15, .2);
    	else col = vec3(1, .15, .4)*.8;
    #endif
    
      
    #ifdef TEXTURED
    vec3 tx = texture(iChannel0, p + hash21(vec2(sh, fi))).xyz; tx *= tx;
    col = min(col*tx*3., 1.);
    #endif

    
    return col;
    
}


void mainImage(out vec4 fragColor, in vec2 fragCoord){
    

    // Aspect correct screen coordinates. Setting a minumum resolution on the
    // fullscreen setting in an attempt to keep things relatively crisp.
    float res = min(iResolution.y, 700.);
	vec2 uv = (fragCoord - iResolution.xy*.5)/res;
    
    // Scaling and translation.
    vec2 p = uv + vec2(0.1, 0.1)*iTime;

    // Resolution based smoothing factor. Later, the contour derivative will
    // be factored in.
    float sf = 1./iResolution.y;
 
    // Initialize to the first layer color.
    vec3 col = getCol(p, 0., 0., sf);
    
    // Previous layer variable.
    float pL = 0.;
    
    
    // Random looking diagonal hatch lines.
    vec2 u2 = p*res/16.;
    float hatch = clamp(sin((u2.x - u2.y)*3.14159*200.)*2. + .5, 0., 1.); // Diagonal lines.
    float hRnd = hash21(floor(u2*6.) + .73);
    if(hRnd>.66) hatch = hRnd; // Slight randomization of the diagonal lines. 
    #ifdef TEXTURED
    hatch = hatch*.75 + .5; // Stronger hatching for the textured version.
    #else
    hatch = hatch*.5 + .75;
    #endif
    
    #ifndef HATCH
    hatch = 1.;
    #endif
    
    // Applying the cross hatch.
    col *= hatch;
    
    // Number of layers.
    int lNum = 5;
    float flNum = 5.;
    
    
    for(int i = 0; i<5; i++){
        
        
        float fi = float(i);
       
        // The map layer value (just some gradient noise), and its derivative.
        vec4 c = mapLayer(vec3(p, 1.), fi);
        // Offset noise layer value with derivative. It's offset so as to coincide with
        // to the directional lighting.
        vec4 cSh = mapLayer(vec3(p - vec2(.03, -.03)*((flNum - fi)/flNum*.5 + .5), 1.), fi);
         
        // Shade.
        float sh = (fi + 1.)/(flNum);
         
        // Layer color.
        vec3 lCol = getCol(p, sh, fi + 1., sf);

        // Some standard direct lighting to apply to the edge layer. It's set in a
        // direction to compliment the shadow layer.
        vec3 ld = normalize(vec3(-1, 1, -.25));
        vec3 n = normalize(vec3(0, 0, -1) + c.yzw);
        float diff = max(dot(ld, n), 0.);
        #ifdef TEXTURED
        diff *= 2.5; // Add a bit more light to the edges for the textured version.
        #else
        diff *= 2.;
        #endif
        
        
        // Applying the diffuse lighting to the edge layer.
        vec3 eCol = lCol*(diff + 1.);
        
        // Apply the layers.
    
        // Smoothing factor, based on the distance field derivative.
    	float sfL = sf*length(c.yzx)*2.;
    	float sfLSh = sf*length(cSh.yzx)*6.;
        
        // Drop shadow.
        #ifdef SHADOW
        #ifdef TEXTURED
        const float shF = .5;
        #else
        const float shF = .35;
        #endif
        col = mix(col, vec3(0), (1. - smoothstep(0., sfLSh, max(cSh.x, pL)))*shF);
        #endif
        
        // Dark blurred layer.
        #ifdef FAKE_AO
		col = mix(col, vec3(0), (1. - smoothstep(0., sfL*3., c.x))*.25);
        #endif
        
        // Dark edge stroke.
        #ifdef STROKE
        col = mix(col, vec3(0), (1. - smoothstep(0., sfL, c.x))*.85);
        #endif
        
        // Hilight and color layer.
        #ifdef HILIGHT
        col = mix(col, eCol*hatch, (1. - smoothstep(0., sfL, c.x + length(c.yzx)*.003)));
        col = mix(col, lCol*hatch, (1. - smoothstep(0., sfL, c.x + length(c.yzx)*.006)));
        #else
        col = mix(col, lCol*hatch, (1. - smoothstep(0., sfL, c.x + length(c.yzx)*.0025)));
        #endif
        
        // Previous layer, to take away from the shadow.
        pL = c.x;
        
    }
    
    
    
    // Mixing in a little extra noisy color for the default greyscale textured setting.
    #ifdef TEXTURED
    #if PALETTE == 0
	col *= mix(vec3(1.8, 1, .7).zyx, vec3(1.8, 1, .7).xzy, n2D(p*2.));
    #endif
    #endif


    
    // Paper.
    #ifdef PAPER_GRAIN
    vec3 rn3 = vec3(n2D((uv*iResolution.y/1. + 1.7)) - n2D(vec2(uv*iResolution.y/1. + 3.4)));
    col *= .93 + .07*rn3.xyz  + .07*dot(rn3, vec3(.299, .587, .114));
    #endif
    
            
    // Subtle vignette.
    uv = fragCoord/iResolution.xy;
    col *= pow(16.*uv.x*uv.y*(1. - uv.x)*(1. - uv.y) , .0625);
    // Colored variation.
    //col = mix(col*vec3(.3, 0, 1), col, pow(16.*uv.x*uv.y*(1. - uv.x)*(1. - uv.y) , .125));

    
    // Rough gamma correction, then output to the screen.
    fragColor = vec4(sqrt(max(col, 0.)), 1);
}
