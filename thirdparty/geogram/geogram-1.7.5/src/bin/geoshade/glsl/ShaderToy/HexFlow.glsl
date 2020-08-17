// https://www.shadertoy.com/view/llSyDh

/*

	Hexagonal Maze Flow
	-------------------

	I've been playing around with hexagonal grids lately. It's possible to make all kinds of
	interesting things with them. I'll eventually tire of hexagons and get back to what I'm 
	supposed to be doing, but for now, here's a maze with a flowing polkadot snake-like
	pattern running through it... I'm not really sure what it is either. :)

	It has an impossible geometry feel to it, and is rendered in an oldschool kind of game
	style. It was pretty easy to produce: Obtain some hexagonal grid cell coordinates and 
	corresponding ID, render some lines, shapes, and a few arcs for the Truchet component,
	apply some shading, etc.
	
	It was more of an exercise in applying layers in the right order than anything else. One 
	of the things that might be of interest is the flowing Truchet pattern. Producing a 
	flowing hexagonal Truchet isn't much different to producing a square one. In fact, it's
	easier - in the sense that you don't have to reverse directions from cell to cell. 

	However, one of the downsides is that it's not immediately obvious how to apply UV 
	coordinates - I think BigWings mentioned this, and I concur. :) I got around the problem
	by ensuring that the texture pattern had the appropriate symmetry and applying hybrid 
	polar coordinates. By that, I mean, I wrapped the polar angles across the arc boundaries
	and used that for one coordinate, then used the Truchet distance field itself for the 
	other...

    Roughly speaking, it's not much different to a circle: In that situation, you use the 
	familiar polar coordinates (r = length(u), a = atan(u.y, u.x)). The only difference in 
	this case is that you use a modified radial coordinate (r = min(min(r1, r2), r3)) - or 
	to put it another way, you use the distance field value... It's a bit difficult to 
	explain, but easy to perform. When I get time, I'm going to produce a more robust Truchet 
	texturing example.
	

	Related references:

	// You can't do a hexagonal grid example without referencing this. :) Very stylish.
	Hexagons - distance - iq
	https://www.shadertoy.com/view/Xd2GR3
	

	// Simpler hexagonal grid example that attempts to explain the grid setup used to produce 
	// the pattern here.
	//
	Minimal Hexagonal Grid - Shane
	https://www.shadertoy.com/view/Xljczw

*/

// Interlaced variation - Interesting, but patched together in a hurry.
#define INTERLACING

// A quick hack to get rid of the winding overlay - in order to show the maze only.
//#define MAZE_ONLY



// Helper vector. If you're doing anything that involves regular triangles or hexagons, the
// 30-60-90 triangle will be involved in some way, which has sides of 1, sqrt(3) and 2.
const vec2 s = vec2(1, 1.7320508);

// Standard vec2 to float hash - Based on IQ's original.
float hash21(vec2 p){ return fract(sin(dot(p, vec2(141.173, 289.927)))*43758.5453); }


// Standard 2D rotation formula.
mat2 r2(in float a){ float c = cos(a), s = sin(a); return mat2(c, -s, s, c); }


// The 2D hexagonal isosuface function: If you were to render a horizontal line and one that
// slopes at 60 degrees, mirror, then combine them, you'd arrive at the following.
float hex(in vec2 p){
    
    p = abs(p);
    
    // Below is equivalent to:
    //return max(p.x*.5 + p.y*.866025, p.x); 

    return max(dot(p, s*.5), p.x); // Hexagon.
    
}

// This function returns the hexagonal grid coordinate for the grid cell, and the corresponding 
// hexagon cell ID - in the form of the central hexagonal point. That's basically all you need to 
// produce a hexagonal grid.
//
// When working with 2D, I guess it's not that important to streamline this particular function.
// However, if you need to raymarch a hexagonal grid, the number of operations tend to matter.
// This one has minimal setup, one "floor" call, a couple of "dot" calls, a ternary operator, etc.
// To use it to raymarch, you'd have to double up on everything - in order to deal with 
// overlapping fields from neighboring cells, so the fewer operations the better.
vec4 getHex(vec2 p){
    
    // The hexagon centers: Two sets of repeat hexagons are required to fill in the space, and
    // the two sets are stored in a "vec4" in order to group some calculations together. The hexagon
    // center we'll eventually use will depend upon which is closest to the current point. Since 
    // the central hexagon point is unique, it doubles as the unique hexagon ID.
    vec4 hC = floor(vec4(p, p - vec2(.5, 1))/s.xyxy) + .5;
    
    // Centering the coordinates with the hexagon centers above.
    vec4 h = vec4(p - hC.xy*s, p - (hC.zw + .5)*s);
    
    // Nearest hexagon center (with respect to p) to the current point. In other words, when
    // "h.xy" is zero, we're at the center. We're also returning the corresponding hexagon ID -
    // in the form of the hexagonal central point. Note that a random constant has been added to 
    // "hC.zw" to further distinguish it from "hC.xy."
    //
    // On a side note, I sometimes compare hex distances, but I noticed that Iomateron compared
    // the Euclidian version, which seems neater, so I've adopted that.
    return dot(h.xy, h.xy)<dot(h.zw, h.zw) ? vec4(h.xy, hC.xy) : vec4(h.zw, hC.zw + vec2(.5, 1));
    
}

// Dot pattern.
float dots(in vec2 p){
    
	p = abs(fract(p) - .5);
    
    return length(p); // Circles.
    
    //return (p.x + p.y)/1.5 + .035; // Diamonds.
    
    //return max(p.x, p.y) + .03; // Squares.
    
    //return max(p.x*.866025 + p.y*.5, p.y) + .01; // Hexagons.
    
    //return min((p.x + p.y)*.7071, max(p.x, p.y)) + .08; // Stars.
    
    
}


// Distance field for the arcs. I think it's called poloidal rotation, or something like that.
float dfPol(vec2 p){
     
    return length(p); // Circular arc.
    
    // There's no rule that says the arcs have to be rounded. Here's a hexagonal one.
    //return hex(p);
    
    // Dodecahedron.
    //return max(hex(p), hex(r2(3.14159/6.)*p));
    
    // Triangle.
    //return max(abs(p.x)*.866025 - p.y, p.y);
    
}


// Truchet pattern distance field.
float df(vec2 p, float dir){
     
    // Weird UV coordinates. The first entry is the Truchet distance field itself,
    // and the second is the polar angle of the arc pixel. The extra ".1" is just a bit
    // of mutational scaling, or something... I can't actually remember why it's there. :)
    vec2 uv = vec2(p.x + .1, p.y);//*vec2(1, 1); // Scaling.
    
    // A checkered dot pattern. At present the pattern needs to have flip symmetry about
    // the center, but I'm pretty sure regular textures could be applied with a few
    // minor changes. Due to the triangular nature of the Truchet pattern, factors of "3"
    // were necessary, but factors of "1.5" seemed to work too. Hence the "4.5."
    return min(dots(uv*4.5), dots(uv*4.5 + .5)) - .3;
    
}


// Polar coordinate of the arc pixel.
float getPolarCoord(vec2 q, float dir){
    
    // The actual animation. You perform that before polar conversion.
    q = r2(iTime*dir)*q;
    
    // Polar angle.
    const float aNum = 1.;
    float a = atan(q.y, q.x);
   
    // Wrapping the polar angle.
    return mod(a/3.14159, 2./aNum) - 1./aNum;
   
    
}

void mainImage(out vec4 fragColor, in vec2 fragCoord){

    // I didn't feel like tayloring the antiasing to suit every resolution, which can get tiring, 
    // so I've put a range on it. Just for the record, I coded this for the 800 by 450 pixel canvas.
    float res = clamp(iResolution.y, 300., 600.); 
    
    // Aspect correct screen coordinates.
	vec2 u = (fragCoord - iResolution.xy*.5)/res;
    
    // Scaling and moving the screen coordinates.
    vec2 sc = u*4. + s.yx*iTime/8.;
    
    // Converting the scaled and translated pixels to a hexagonal grid cell coordinate and
    // a unique coordinate ID. The resultant vector contains everything you need to produce a
    // pretty pattern, so what you do from here is up to you.
    vec4 h = getHex(sc); // + s.yx*iTime/2.
    
    // Obtaining some offset values to do a bit of cubic shading. There are probably better ways
    // to go about it, but it's quick and gets the job done.
    vec4 h2 = getHex(sc - 1./s);
    vec4 h3 = getHex(sc + 1./s);
    
    // Storing the hexagonal coordinates in "p" to save having to write "h.xy" everywhere.
    vec2 p = h.xy;
    
    // The beauty of working with hexagonal centers is that the relative edge distance will simply 
    // be the value of the 2D isofield for a hexagon.
    //
    float eDist = hex(p); // Edge distance.
    float cDist = dot(p, p); // Relative squared distance from the center.

    
    // Using the identifying coordinate - stored in "h.zw," to produce a unique random number
    // for the hexagonal grid cell.
    float rnd = hash21(h.zw);
    //float aRnd = sin(rnd*6.283 + iTime*1.5)*.5 + .5; // Animating the random number.
    
    #ifdef INTERLACING
    // Random vec3 - used for some overlapping.
    //vec3 lRnd = vec3(rnd*14.4 + .81, fract(rnd*21.3 + .97), fract(rnd*7.2 + .63));
    vec3 lRnd = vec3(hash21(h.zw + .23), hash21(h.zw + .96), hash21(h.zw + .47));
    #endif
    
    // It's possible to control the randomness to form some kind of repeat pattern.
    //rnd = mod(h.z + h.w, 2.);
    

    // Redundant here, but I might need it later.
    float dir = 1.;
    

    
    // Storage vector.
    vec2 q;
    
    
    // If the grid cell's random ID is above a threshold, flip the Y-coordinates.
    if(rnd>.5) p.y = -p.y;
        
    

    // Determining the closest of the three arcs to the current point, the keeping a copy
    // of the vector used to produce it. That way, you'll know just to render that particular
    // decorated arc, lines, etc - instead of all three. 
    const float r = 1.;
    const float th = .2; // Arc thickness.
    
    // Arc one.
    q = p - vec2(0, r)/s;
    vec3 da = vec3(q, dfPol(q));
    
    // Arc two. "r2" could be hardcoded, but this is a relatively cheap 2D example.
    q = r2(3.14159*2./3.)*p - vec2(0, r)/s;
    vec3 db = vec3(q, dfPol(q));

     // Arc three. 
    q = r2(3.14159*4./3.)*p - vec2(0, r)/s;
    vec3 dc = vec3(q, dfPol(q));
    
    // Compare distance fields, and return the vector used to produce the closest one.
    vec3 q3 = da.z<db.z && da.z<dc.z? da : db.z<dc.z ? db : dc;
    
    
    // TRUCHET PATTERN
    // Produce the closest arc. The result is the Truchet pattern.
    // Set the circle radius. For the hexagonal version, use .5/2., and for triangles, .7071/2. 
    q3.z -= .57735/2. + th/2.;  
    q3.z = max(q3.z, -th - q3.z); // Chop out the smaller radius. The result is an arc.
    
    // Store the result in "d" - only to save writing "q3.z" everywhere.
    float d = q3.z;
    
    // If you'd like to see the maze by itself.
    #ifdef MAZE_ONLY
    d += 1e5;
    #endif
    
    // Truchet border.
    float dBord = max(d - .015, -d);
    

    
 
    
    // MAZE BORDERS
    // Producing the stright-line arc borders. Basically, we're rendering some hexagonal borders around
    // the arcs. The result is the hexagonal maze surrounding the Truchet pattern.
    q = q3.xy;
    const float lnTh = .05;
    q = abs(q);
    
    float arcBord = hex(q);
    //float arcBord = length(q); // Change arc length to ".57735."
    //float arcBord = max(hex(q), hex(r2(3.14159/6.)*q)); // Change arc length to ".57735."
    
    // Making the hexagonal arc.
    float lnOuter = max(arcBord - .5, -(arcBord - .5 + lnTh)); //.57735
    
    
    #ifdef INTERLACING
    float ln = min(lnOuter, (q.y*.866025 + q.x*.5, q.x) - lnTh);
    #else
    float ln = min(lnOuter, arcBord - lnTh);
    #endif
    float lnBord = ln - .03; // Border lines to the maze border, if that makes any sense. :)
     
    
   
    
    ///////
    // The moving Truchet pattern. The polar coordinates consist of a wrapped angular coordinate,
    // and the distance field itself.
    float a = getPolarCoord(q3.xy, dir);
    float d2 = df(vec2(q3.z, a), dir); 
    
    // Smoothstepped Truchet mask.
    float dMask = smoothstep(0., .015, d);
    ///////
    
    // Producing the background with some subtle gradients.
    vec3 bg =  mix(vec3(0, .4, .6), vec3(0, .3, .7), dot(sin(u*6. - cos(u*3.)), vec2(.4/2.)) + .4); 
    bg = mix(bg, bg.xzy, dot(sin(u*6. - cos(u*3.)), vec2(.4/2.)) + .4);
    bg = mix(bg, bg.zxy, dot(sin(u*3. + cos(u*3.)), vec2(.1/2.)) + .1);
   
    #ifdef INTERLACING
    // Putting in background cube lines for the interlaced version.
    float hLines = smoothstep(0., .02, eDist - .5 + .02);
    bg = mix(bg, vec3(0), smoothstep(0., .02, ln)*dMask*hLines);
    #endif
    
    // Lines over the maze lines. Applying difference logic, depending on whether the 
    // pattern is interlaced or not.
    const float tr = 1.;

    float eDist2 = hex(h2.xy);
    float hLines2 = smoothstep(0., .02, eDist2 - .5 + .02);
    #ifdef INTERLACING
    if(rnd>.5 && lRnd.x<.5) hLines2 *= smoothstep(0., .02, ln);
    if(lRnd.x>.5) hLines2 *= dMask;
    #else
    if(rnd>.5) hLines2 *= smoothstep(0., .02, ln);
    hLines2 *= dMask;
    #endif
    bg = mix(bg, vec3(0), hLines2*tr);
    
    float eDist3 = hex(h3.xy);
    float hLines3 = smoothstep(0., .02, eDist3 - .5 + .02);
    #ifdef INTERLACING
    if(rnd<=.5 && lRnd.x>.5) hLines3 *= smoothstep(0., .02, ln);
    if(lRnd.x>.5) hLines3 *= dMask;
    #else
    if(rnd<=.5) hLines3 *= smoothstep(0., .02, ln);
    hLines3 *= dMask;
    #endif
    bg = mix(bg, vec3(0), hLines3*tr);


    // Using the two off-centered hex coordinates to give the background a bit of highlighting.
    float shade = max(1.25 - dot(h2.xy, h2.xy)*2., 0.);
    shade = min(shade, max(dot(h3.xy, h3.xy)*3. + .25, 0.));
    bg = mix(bg, vec3(0), (1.-shade)*.5); 
    
    // I wanted to change the colors of everything at the last minute. It's pretty hacky, so
    // when I'm feeling less lazy, I'll tidy it up. :)
    vec3 dotCol = bg.zyx*vec3(1.5, .4, .4);
    vec3 bCol = mix(bg.zyx, bg.yyy, .25);
    bg = mix(bg.yyy, bg.zyx, .25);
    

    // Under the random threshold, and we draw the lines under the Truchet pattern.
    #ifdef INTERLACING
    if(lRnd.x>.5){
       bg = mix(bg, vec3(0), (1. - smoothstep(0., .015, lnBord)));
       bg = mix(bg, bCol, (1. - smoothstep(0., .015, ln))); 
       // Center lines.
       bg = mix(bg, vec3(0), smoothstep(0., .02, eDist3 - .5 + .02)*tr);
    }
    #else
    bg = mix(bg, vec3(0), (1. - smoothstep(0., .015, lnBord)));
    bg = mix(bg, bCol, (1. - smoothstep(0., .015, ln)));
    #endif

   
    
    // Apply the Truchet shadow to the background.
    bg = mix(bg, vec3(0), (1. - smoothstep(0., .07, d))*.5);
    
    
    // Place the Truchet field to the background, with some additional shading to give it a 
    // slightly rounded, raised feel.
    vec3 col = mix(bg, vec3(1)*max(-d*3. + .7, 0.), (1. - dMask)*.65);

    
    // Apply the moving dot pattern to the Truchet.
    //dotCol = mix(dotCol, dotCol.xzy, dot(sin(u*3.14159*2. - cos(u.yx*3.14159*2.)*3.14159), vec2(.25)) + .5);
    col = mix(col, vec3(0), (1. - dMask)*(1. - smoothstep(0., .02, d2)));
    col = mix(col, dotCol, (1. - dMask)*(1. - smoothstep(0., .02, d2 + .125)));
    
    // Truchet border.
    col = mix(col, vec3(0), 1. - smoothstep(0., .015, dBord));
    
    #ifdef INTERLACING
    // Over the random threshold, and we draw the lines over the Truchet.
    if(lRnd.x<=.5){
        col = mix(col, vec3(0), (1. - smoothstep(0., .015, lnBord)));
        col = mix(col, bCol, (1. - smoothstep(0., .015, ln)));  
        // Center lines.
        col = mix(col, vec3(0), smoothstep(0., .02, eDist2 - .5 + .02)*tr);
    }
    #endif

        
    
    
    // Using the offset hex values for a bit of fake 3D highlighting.
    //if(rnd>.5) h3.y = -h3.y; // All raised edges. Spoils the mild 3D illusion.
    #ifdef INTERLACING
    float trSn = max(dMask, 1. - smoothstep(0., .015, lnBord))*.75 + .25;
    #else
    float trSn = dMask*.75 + .25;
    #endif
    col = mix(col, vec3(0), trSn*(1. - hex(s/2.+h2.xy)));
    col = mix(col, vec3(0), trSn*(1. - hex(s/2.-h3.xy)));
 
    
    // Using the edge distance to produce some repeat contour lines. Standard stuff.
    //if (rnd>.5) h.xy = -h.yx;
    //float cont = clamp(cos(hex(h.xy)*6.283*12.)*1.5 + 1.25, 0., 1.);
    //col = mix(col, vec3(0), (1. - smoothstep(0., .015, ln))*(smoothstep(0., .015, d))*(1.-cont)*.5);
    
    
    // Very basic hatch line effect.
    float gr = dot(col, vec3(.299, .587, .114));
    float hatch = (gr<.45)? clamp(sin((sc.x - sc.y)*3.14159*40.)*2. + 1.5, 0., 1.) : 1.;
    float hatch2 = (gr<.25)? clamp(sin((sc.x + sc.y)*3.14159*40.)*2. + 1.5, 0., 1.) : 1.;

    col *= min(hatch, hatch2)*.5 + .5;    
    col *= clamp(sin((sc.x - sc.y)*3.14159*80.)*1.5 + .75, 0., 1.)*.25 + 1.;  
    
 
    // Subtle vignette.
    u = fragCoord/iResolution.xy;
    col *= pow(16.*u.x*u.y*(1. - u.x)*(1. - u.y) , .125) + .25;
    // Colored varation.
    //col = mix(pow(min(vec3(1.5, 1, 1)*col, 1.), vec3(1, 3, 16)), col, 
            //pow(16.*u.x*u.y*(1. - u.x)*(1. - u.y) , .25)*.75 + .25);    
    

    
    // Rough gamma correction.    
	fragColor = vec4(sqrt(max(col, 0.)), 1);
    
}



