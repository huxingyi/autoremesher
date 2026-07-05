// https://www.shadertoy.com/view/4scXzn
/*

	Winding Menger Tunnel
	---------------------

	I got bored and decided to wrap a Menger object around a curvy tunnel, then I got even more bored and 
	incorporated some tubing and some curved screens... I have no idea what they're for either. :)

	Anyway, if you put aside the cheesy, Quake-2-style graphics, it's nothing more than a couple of 
	interwoven fractal objects perturbed sinusoidally about the "XY" plane. In code:

	pos.xy -= sinPath(pos.z);
	dist = FractalObjects(pos);

	Obviously, the camera has to follow the path as well, but that's basically it. You can ignore everything 
	else, which is just less-than-adequate window dressing. I've been on a bit of an oldschool demo trip 
	lately, which probably explains the simplistic lighting style.

	Other tunnel related examples worth looking at:

	// Awesome example. Makes the lighting effort in this one look lazy... which it is. :)
    Castle Tunnel - Hamneggs
    https://www.shadertoy.com/view/Xs3Xzn

    // Love this. It inspired me to interweave the metal tubing in this particular shader.
    Metro Tunnel - fb39ca4
    https://www.shadertoy.com/view/ldsGRS

    // Like all of dr2's stuff, it has a higher level of difficulty. :)
    Gotthard Tunnel - dr2
    https://www.shadertoy.com/view/MlSXRR 

*/


// Used to identify individual scene objects. In this case, there are only three: The metal framework, the walls,
// and the lights.
float objID = 0.; // Metal = 1., Walls = 2., Screens = 3..

// Simple hash function.
float hash( float n ){ return fract(cos(n)*45758.5453); }


// Tri-Planar blending function. Based on an old Nvidia writeup:
// GPU Gems 3 - Ryan Geiss: http://http.developer.nvidia.com/GPUGems3/gpugems3_ch01.html
vec3 tex3D( sampler2D tex, in vec3 p, in vec3 n ){
   /*
    n = max(abs(n), 0.001); // n = max((abs(n) - 0.2)*7., 0.001); // n = max(abs(n), 0.001), etc.
    n /= (n.x + n.y + n.z); 
	return (texture2D(tex, p.yz)*n.x + texture2D(tex, p.zx)*n.y + texture2D(tex, p.xy)*n.z).xyz;
    */
    return vec3(0.0, 0.0, 0.0); // [BL] removed texturing to make it "embeddable" in geoshade.
}

// Common formula for rounded squares, for all intended purposes.
float lengthN(in vec2 p, in float n){ p = pow(abs(p), vec2(n)); return pow(p.x + p.y, 1.0/n); }

// 2D path displacement.
vec2 path(in float x){
    
    //return vec2(0); // Trivial, straight path.
    //return vec2(cos(x*0.25)*1.8 + cos(x*0.15)*2., 0); // Perturbing "X" only.
    return vec2(cos(x*0.25)*1.8 + cos(x*0.15)*1.5, sin(x*0.25)*1.2 + sin(x*0.15)); // Perturbing "X" and "Y."
    
    
}

// Camera path. Arranged to coincide with the frequency of the tunnel.
vec3 camPath(float t){
  
    return vec3(path(t), t);
    
}

// Smooth minimum function. There are countless articles, but IQ explains it best here:
// http://iquilezles.org/www/articles/smin/smin.htm
float sminP( float a, float b, float s ){

    float h = clamp( 0.5+0.5*(b-a)/s, 0.0, 1.0 );
    return mix( b, a, h ) - s*h*(1.0-h);
}

// I have a "Menger Sponge Variation" example somewhere, if you'd like to look into this.
float Menger(in vec3 q){
    
    float s = 4.;
    // Layer one. The ".05" on the end varies the hole size.
 	vec3 p = abs(fract(q/s)*s - s*.5);
 	float d = min(max(p.x, p.y), min(max(p.y, p.z), max(p.x, p.z))) - s/3.;// + .05;
    
    s /= 2.;
    // Layer two.
    p = abs(fract(q/s)*s - s*.5);
 	d = max(d, min(max(p.x, p.y), min(max(p.y, p.z), max(p.x, p.z))) - s/3.);//+ .05
   
    s /= 3.;
    // Layer three. 3D space is divided by two, instead of three, to give some variance.
    p = abs(fract(q/s)*s - s*.5);
 	d = max(d, min(max(p.x, p.y), min(max(p.y, p.z), max(p.x, p.z))) - s/3.); //- .015 
    
    
    //float floor = max(abs(q.x) - 2., abs(q.y) - 1.5);//abs(q.x) - 2.;//
    //return q.y + .8;
    return min(d, q.y + .8);
 
}

// I have a "Steel Lattice" example somewhere, if you'd like to look into this. There's not
// much to it, though.
float tubing(in vec3 p){
 
    // SECTION 1
    //
    // Repeat field entity one, which is just some tubes repeated in all directions every 
    // two units, then combined with a smooth minimum function. Otherwise known as a lattice.
    p = fract(p/2.)*2. - 1.;
    float x1 = sminP(length(p.xy),sminP(length(p.yz),length(p.xz), 0.25), 0.25)-0.5; // EQN 1
    //float x1 = sqrt(min(dot(p.xy, p.xy),min(dot(p.yz, p.yz),dot(p.xz, p.xz))))-0.5; // EQN 2
    //p = abs(p); float x1 = min(max(p.x, p.y),min(max(p.y, p.z),max(p.x, p.z)))-0.5; // EQN 3

    // SECTION 2
    //
    // Repeat field entity two, which is just an abstract object repeated every half unit. 
    p = abs(fract(p*2.)*.5 - .25);
    //float x2 = min(p.x,min(p.y,p.z)); // EQN 1
    //float x2 = min(max(p.x, p.y),min(max(p.y, p.z),max(p.x, p.z)))-0.15; //-0.175, etc. // EQN 2
    float x2 = min(p.x, min(p.y,p.z))-.025; // EQN 1
    
    // SECTION 3
    //
    // Combining the two entities above.
    return max(abs(x1), abs(x2)) - .0175;
    
}

// Creating the scene geometry. This is the process:
//
// Use the sinusoidal path function to perturb the original position. Create the Menger object
// using the perturbed postion. Do the same for the tubing and again with the screens.
// Return the minimum of the objects, and also use the relative minimums to return the object's
// individual ID. That's basically it.
float map(in vec3 p){
   

	// Partial anti-warping solution, based on Gaz's "Square Sin Curve" shader below:
    // https://www.shadertoy.com/view/MscGzf
    //
    // As you could imagine, tunnels (columns) get a little warped when you bend them. Countering 
    // that by taking the curvature into account helps quite a bit. Unfortunately, it slows things 
    // down, so isn't being used here, which is a shame, because I like it a lot more. Anyway, if 
    // you can spare the cycles, it gives the tunnel's "X" and "Y" (width and height) dimensions a 
    // little more consistency.
    //vec2 g = (path(p.z + 0.01) - path(p.z - 0.01))/0.02;
    //g = cos(atan(g));
    
    
    // "Windy Tunnels 101" - Use "Z" to perturb the "XY" plane. If you're not sure how it'd done,
    // I have a few tunnel examples where I explain the process.
    p.xy -= path(p.z);
    
    //p.xy *= g; // See the anti-warping explanation above.


    // A bit of tubing, using a combination of repeat objects.
    float tube = tubing(p);
    
    
    // Again a little expensive, but it's a surprisingly effective way to bump the tunnel walls.
    // This is a variation, but you can thank "aeikick" for this little snippet. :)
    //vec3 u = p;
    //p.x -= sign(u.x)*(texture2D(iChannel0, u.yz/8.).x - .0)*.03;//-.2;
	//p.y -= sign(u.y)*(texture2D(iChannel0, u.xz/8.).x - .0)*.03;  

    
    // The walls. I have another Menger example, if you'd like to look into that more closely.
    float walls = Menger(p);
    // Simpler alternatives.
    //float walls = 1. - max(abs(p.x), abs(p.y));
    //float walls = 1.25 - lengthN(p.xy, 4.0);
    
    // The curved screens. Kind of worth the effort, but not really. Fine details always overcomplicate 
    // things, not to mention, halve the frame rate. :) Anyway, it's basically repeated square box-related 
    // stuff... Add this, take that, etc. Fiddly, hacky, not all that interesting, and probably not the
    // best way to do it. Chipping away at a cylinder might raymarch better. 
    //
    p += vec3(sign(p.x)*(-.11 + (sin(p.z*3.14159*2. + 1.57/1.))*.05), 0., 0.); // Screen curve, and repositioning.
    vec3 q = abs(mod(p + vec3(.0, .5, 0.), vec3(1., 1., 2.)) - vec3(.5, .5, 1.)); // Repeat space.
    float screen = max(max(q.y, q.z) - .22, q.x-.05); // Box.
    screen = max(screen, max(abs(p.x) - .5, abs(p.y) - .22)); // Chopping off anything outside the tunnel... Kind of.
    
    // Object ID: Equivalent to: if(tube<walls)objID=2; else objID = 1.; //etc.
    //
    // By the way, if you need to identify multiple objects, you're better off doing it in a seperate pass, 
    // after the raymarching function. Having multiple "if" statements in a distance field equation can slow 
    // things down considerably. Alternatively, there's the "vec2 objA = vec2(objectADist, objAID)" option 
    // that many are fond of. It seems to be slower on my machines, but seems to work well enough.
    objID = 1. + step(tube, walls) + step(screen, tube)*step(screen, walls)*2.;
   

    // Returning the minimum of the three objects.
    return min(min(tube, walls), screen);
    
/*    
    //Two object combinations. Spoils the illusion, but helps visualize things.

    //objID = 2. + step(screen, tube);
    //return min(tube, screen); 
        
        
    //objID = 1. + step(tube, walls);
    //return min(tube, walls);        
        
    objID = 1. + step(screen, walls)*2.;
    return min(screen, walls);    
*/     
    
    
}



// Tetrahedral normal, to save a couple of "map" calls. Courtesy of IQ.
vec3 calcNormal(in vec3 p){

    // Note the slightly increased sampling distance, to alleviate artifacts due to hit point inaccuracies.
    vec2 e = vec2(0.0025, -0.0025); 
    return normalize(e.xyy * map(p + e.xyy) + e.yyx * map(p + e.yyx) + e.yxy * map(p + e.yxy) + e.xxx * map(p + e.xxx));
}

/*
// Standard normal function.
vec3 calcNormal(in vec3 p) {
	const vec2 e = vec2(0.005, 0);
	return normalize(vec3(map(p + e.xyy) - map(p - e.xyy), map(p + e.yxy) - map(p - e.yxy),	map(p + e.yyx) - map(p - e.yyx)));
}
*/

// I keep a collection of occlusion routines... OK, that sounded really nerdy. :)
// Anyway, I like this one. I'm assuming it's based on IQ's original .
float calcAO(in vec3 pos, in vec3 nor)
{
	float sca = 2.0, occ = 0.0;
    for( int i=0; i<5; i++ ){
    
        float hr = 0.01 + float(i)*0.5/4.0;        
        float dd = map(nor * hr + pos);
        occ += (hr - dd)*sca;
        sca *= 0.7;
    }
    return clamp( 1.0 - occ, 0.0, 1.0 );    
}


// Texture bump mapping. Four tri-planar lookups, or 12 texture lookups in total. I tried to 
// make it as concise as possible. Whether that translate to speed, or not, I couldn't say.
vec3 texBump( sampler2D tx, in vec3 p, in vec3 n, float bf){
   
    const vec2 e = vec2(0.002, 0);
    
    // Three gradient vectors rolled into a matrix, constructed with offset greyscale texture values.    
    mat3 m = mat3( tex3D(tx, p - e.xyy, n), tex3D(tx, p - e.yxy, n), tex3D(tx, p - e.yyx, n));
    
    vec3 g = vec3(0.299, 0.587, 0.114)*m; // Converting to greyscale.
    g = (g - dot(tex3D(tx,  p , n), vec3(0.299, 0.587, 0.114)) )/e.x; g -= n*dot(n, g);
                      
    return normalize( n + g*bf ); // Bumped normal. "bf" - bump factor.
	
}

// Standard hue rotation formula.
vec3 rotHue(vec3 p, float a){

    vec2 cs = sin(vec2(1.570796, 0) + a);

    mat3 hr = mat3(0.299,  0.587,  0.114,  0.299,  0.587,  0.114,  0.299,  0.587,  0.114) +
        	  mat3(0.701, -0.587, -0.114, -0.299,  0.413, -0.114, -0.300, -0.588,  0.886) * cs.x +
        	  mat3(0.168,  0.330, -0.497, -0.328,  0.035,  0.292,  1.250, -1.050, -0.203) * cs.y;
							 
    return clamp(p*hr, 0., 1.);
}

// Screen pattern. Simple, but effective. The idea to go with this was inspired by Dmitry Andreev's 
// really cool "pixelScreen" shader, here: https://www.shadertoy.com/view/XdG3Wc
//
// His example is a little fancier, mainly because he's using way more code... The fact that he won 
// Assembly a couple of times might also be a factor. :)
float dotPattern(vec2 p){
    
    // Partition space into multiple squares.
    vec2 fp = abs(fract(p)-0.5)*2.;
    
    // Rounded circle, for the overlay, or vignette, if you prefer.
    fp = pow(fp, vec2(8.));
    float r = max(1. - pow(fp.x + fp.y, 1.), 0.);
    
    // More squarish (Chebyshev) version of the above.
    //fp = pow(fp, vec2(8.));
    //float r = 1. - max(fp.x, fp.y);
    
    // Single value for each square. Used for IDs and a bunch of other things, but in this 
    // case it'll give the square a homogeneous color.
    p = floor(p); 
    
    // The blocky pattern value. Made up, but you could use all kinds of things, like Voronoi, etc. 
    float c = dot(sin(p/4. - cos(p.yx/.2 + iTime/4.)), vec2(.5));

    c = fract(c * 7.0); // Mixing it up, for no particular reason.

    return c*r; // Pixel shade, multiplied by the rounded square vignette. Range: [0, 1].
    
}


void mainImage( out vec4 fragColor, in vec2 fragCoord ){
   
    
	// Screen coordinates.
	vec2 u = (fragCoord - iResolution.xy*0.5)/iResolution.y;
	
	// Camera Setup.
    vec3 ro = camPath(iTime*1.5); // Camera position, doubling as the ray origin.
    vec3 lk = camPath(iTime*1.5 + .1);  // "Look At" position.
    vec3 lp = camPath(iTime*1.5 + 2.) + vec3(0, 2, 0); // Light position, somewhere near the moving camera.


    // Using the above to produce the unit ray-direction vector.
    float FOV = 1.57; // FOV - Field of view.
    vec3 fwd = normalize(lk-ro);
    vec3 rgt = normalize(vec3(fwd.z, 0., -fwd.x )); 
    vec3 up = cross(fwd, rgt);

    // Unit direction ray.
    vec3 rd = normalize(fwd + FOV*(u.x*rgt + u.y*up));
 
    vec2 a = sin(vec2(1.5707963, 0) - camPath(lk.z).x/12.); 
    mat2 rM = mat2(a, -a.y, a.x);
    rd.xy = rd.xy*rM; // Apparently, "rd.xy *= rM" doesn't work on some setups. Crazy.
    
    // Mouse controls, as per Dave Hoskins's suggestion. A bit hacky, but I'll fix them.    
	vec2 ms = vec2(0);
    if (iMouse.z > 1.0) ms = (2.*iMouse.xy - iResolution.xy)/iResolution.xy;
    a = sin(vec2(1.5707963, 0) - ms.x); 
    rM = mat2(a, -a.y, a.x);
    rd.xz = rd.xz*rM; 
    a = sin(vec2(1.5707963, 0) - ms.y); 
    rM = mat2(a, -a.y, a.x);
    rd.yz = rd.yz*rM;
    

    
    // Raymarching.
    const float FAR = 50.0;
    float t = 0.0, h;
    for(int i = 0; i < 96; i++){
    
        h = map(ro+rd*t);
        // Note the "t*b + a" addition. Basically, we're putting less emphasis on accuracy, as
        // "t" increases. It's a cheap trick that works in most situations... Not all, though.
        if(abs(h)<0.001*(t*.75 + .25) || t>FAR) break;//*(t*.5 + 1.)
        t += h*.75;
        //t += step(.5, t)*h*.25 + h*.5;
        
    }
    
    // Initialize the scene color.
    vec3 col = vec3(0);
    
    // Scene hit, so color the pixel. 
    if(t<FAR){
        
        // This looks a little messy and haphazard, but it's really just some basic lighting, and application
        // of the following material properties: Metal = 1., Walls = 2., Screens = 3..
    
        float ts = 2.;
        // Global object ID. It needs to be saved just after the raymarching equation, since other "map" calls,
        // like normal calculations will give incorrect results. Found that out the hard way. :)
        float saveObjID = objID; 
        
        
        vec3 pos = ro + rd*t; // Scene postion.
        vec3 pOffs = pos - vec3(camPath(pos.z).xy, 0); // Postion, offset by the path. 
        vec3 nor = calcNormal(pos); // Normal.
        
        // Apply some subtle texture bump mapping to the walls and the metal tubing, but not the screen.
        // I should probably get rid of that "if" statement later, but it seems OK for now.
        if(saveObjID<2.5) nor = texBump(iChannel0, pOffs*ts, nor, 0.002 + step(saveObjID, 1.5)*0.012);
        
        
	//	col = tex3D(iChannel0, pOffs*ts, nor); // Texture pixel at the scene postion.
	col = vec3(0.5, 0.5, 0.5);
        col = smoothstep(-.3, .8, col)*vec3(1., .8, .7); // Process the color a little.

        // More fake lighting. This was just a bit of trial-and-error to produce some repetitive,
        // slightly overhead, spotlights in each of the modules. Cylinder in XY, sine repeat
        // in the Z direction... Something like that.
        float spot = max(2. - length(pOffs.xy - vec2(0, 1)), 0.)*(sin((pOffs.z)*3.14159 + 1.57)*.5+.5);
        spot = smoothstep(0.25, 1., spot);
        
        
        
        float occ = calcAO( pos, nor ); // Occlusion.
		vec3  li = normalize( lp - pos ); // Point light.
        float dif = clamp(dot(nor, li), 0.0, 1.0); // Diffuse.
        float spe = pow(max(dot(reflect(-li, nor), -rd), 0.), 8.); // Object specular.
        float spe2 = 0.; // Global specular.

            

        vec3  rCol = vec3(0); // Reflection color. Mostly fake.
        
        // If the metal tubing or the screen is hit, apply the individual properties.
        if(saveObjID>1.5){ 
			
            // Grey out the limestone wall color.
            col = vec3(1)*dot(col*.7+.2, vec3(.299, .587, .114));
            // Add some fake reflection. Not reliable, but it's subtle.
            rCol = tex3D(iChannel0, (pOffs + reflect(rd, nor))*ts, nor);
            col += rCol*.25 + spot*.25;
            spe2 = spe*spe*.25; // Ramp up the global specular a bit.
            
        }         
        
        // If just the screen has been hit, apply some extra properties, then draw the screen image.
        // I could just write "saveObjID == 3.," but I get a little paranoid where floats are concerned. :)
        if(saveObjID>2.5){

            // For the screen image, we're interested in the offset height and depth positions. Ie: pOffs.zy.
            
            // Pixelized dot pattern shade.
            float c = dotPattern(pOffs.zy*36.+.5);
            
            // Applying some color to the shade.
            col = vec3(min(c*1.5, 1.), pow(c, 2.5), pow(c, 12.));
            // Mixing the colors around a little. Made up.
            col = mix(col.zyx, col, sin(dot(pos, vec3(.333))*3.14159*6.)*.34+.66);
			
            // Individual screen ID or sorts.
            float id = hash(dot(floor(pOffs + vec3(.0, .5, .5)), vec3(7, 157, 113)));
            
            // Use the screen ID to give it a different random hue.
            col = rotHue(col, floor(id*12.)/12.*6.283/2.); 
            
            col += rCol*rCol*.5; // Screen reflection.            
            
            dif += .5; // Make the screen appear self illuminating, but increasing the diffuse.
            spe += .25;
            
        }
       
        // Combining everything together to produce the scene color.
        col *= (dif + .25 + spot*.5 + vec3(.25, .3, .5)*spe) + spe2;
        col *= occ; // Applying occlusion.
       
        
    }
    
    // Applying some very slight fog in the distance. This is technically an inside scene...
    // Or is it underground... Who cares, it's just a shader. :)
    col = mix(min(col, 1.), vec3(0), 1.-exp(-t*t/FAR/FAR*15.));//smoothstep(0., FAR-20., t)
    
    // Done.
    fragColor = vec4(col, 1.0);
    
}

