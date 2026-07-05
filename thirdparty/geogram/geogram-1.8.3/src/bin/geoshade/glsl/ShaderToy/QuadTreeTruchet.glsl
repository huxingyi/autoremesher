// https://www.shadertoy.com/view/4t3BW4
/*

	Quadtree Truchet
	----------------

    A multiscale, multitile, overlapped, weaved Truchet pattern -- However, since
	that description is a little verbose, I figured that a quadtree Truchet was as
	good a description as any. :) The mild weave effect is provided via the
	"INCLUDE_LINE_TILES" define.

	In order to produce a varied looking Truchet pattern, there are a couple of
	simple things you can try: One is to use more than one tile, and the other is
	to stitch weaved tiles together to produce a cool under-over effect. There are
    a few examples on Shadertoy of each, which are easy enough to find -- Just do
	a search for "Truchet" and look for the multitile and weaved examples.

    Lesser known variations include using Truchet tiles that overlap one another,
    and stitching together multiscaled tiles -- usually on something like a quadtree
    grid. This example uses elements of all of the aforementioned.

	In the past, I've combined two non-overlapping tile scales, but had never
    considered taking it beyond that... until I came across Christopher Carlson's
	article, "Multi-Scale Truchet Patterns." If you follow the link below and refer
	to the construction process, you'll see that the idea behind it is almost
	rundimentary. As a consequence, I figured that it'd take me five minutes to put
	the ideas into pixel shader form. Unfortunately, they say the dumber you are,
	the more overconfident you'll be, and to cut a long story short... It took me
	longer than five minutes. :D

	The code below is somewhat obfuscated and strewn with defines - The defines are
	my fault, since I wanted to provide a few rendering options. However, the
	remaining complication boils down to the necessity to render overlapping tiles
	on a repeat quadtree grid in an environment that doesn't allow random pixel
	access. The only example along those lines I could find on here was IQ's
	hierachical Voronoi demonstration, which is pretty cool, but I needed to render
	things in a way that involved less nesting, which complicated things.

	Either way, the idea is pretty simple: Construct a grid, randomly render some
	Truchet tiles, subdivide the remaining squares into four, randomly render some
    more tiles in reverse color order, then continue ad infinitum. By the way, I
	constructed this on the fly using the best method I could think of at the time.
	However, if anyone out there has a more elegant solution, feel free to post it. :)
	
	Naturally, the idea can be extended to 3D. Three levels with this particular
	setup might be a little slow. However, two levels using a non overlapping tile
	is definitely doable, so I intend to produce an example along those lines in the
	near future.


	Based on the following:

	Multi-Scale Truchet Patterns  - Christopher Carlson
	https://christophercarlson.com/portfolio/multi-scale-truchet-patterns/
    Linking paper containing more detail:
    http://archive.bridgesmathart.org/2018/bridges2018-39.pdf

	Quadtree Related:

	// Considers overlap.
	https://www.shadertoy.com/view/Xll3zX
	Voronoi - hierarchical - IQ

    // No overlap, but I really like this one.
    SDF Raymarch Quadtree - Paniq
	https://www.shadertoy.com/view/MlffW8

	// Multilevel, and nice and simple.
	quadtree - 4 - FabriceNeyret2
	https://www.shadertoy.com/view/ltlyRH

*/


// DEFINES: Feel free to try them out.

// Default colored setting. Not applicable when using the stacked tiles option.
// When turned off, the color is white.
#define SPECTRUM_COLORED

// Showing the different tile layers stacked on top of one another. Aesthetically, I prefer
// this more, because it has a raised look about it. However, you can't make out the general
// pattern as well, so it's off by default.
//#define STACKED_TILES

// Pink -- Less bland than white, and has a velvety feel... Gets overridden by the spectrum
// color option, so only works when "SPECTRUM_COLORED" is commented out.
//#define PINK

// This option produces art deco looking patterns, which are probably more interesting, but
// I wanted the default pattern to be more simplistic.
//#define INCLUDE_LINE_TILES



// vec2 to vec2 hash.
vec2 hash22(vec2 p) {

    // Faster, but doesn't disperse things quite as nicely. However, when framerate
    // is an issue, and it often is, this is a good one to use. Basically, it's a tweaked
    // amalgamation I put together, based on a couple of other random algorithms I've
    // seen around... so use it with caution, because I make a tonne of mistakes. :)
    float n = sin(dot(p, vec2(57, 27)));

    return fract(vec2(262144, 32768)*n);

    /*
    // Animated.
    p = fract(vec2(262144, 32768)*n);
    // Note the ".35," insted of ".5" that you'd expect to see. .
    return sin(p*6.2831853 + iTime/2.)*.24;
    */
}

// Standard 2D rotation formula.
mat2 r2(in float a){ float c = cos(a), s = sin(a); return mat2(c, s, -s, c); }

/*
// IQ's 2D unsigned box formula.
float sBox(vec2 p, vec2 b){ return length(max(abs(p) - b, 0.)); }

// IQ's 2D signed box formula.
float sBoxU(vec2 p, vec2 b){

  vec2 d = abs(p) - b;
  return min(max(d.x, d.y), 0.) + length(max(d, 0.));
}
*/

void mainImage(out vec4 fragColor, in vec2 fragCoord){

    // Screen coordinates.
    vec2 uv = (fragCoord - iResolution.xy*.5)/iResolution.y;

    // Scaling, rotation and transalation.
    vec2 oP = uv*5.;
    oP *= r2(sin(iTime/8.)*3.14159/8.);
    oP -= vec2(cos(iTime/8.)*0., -iTime);


    // Distance field values -- One for each color. They're "vec4"s to hold the three
    // layers and an an unused spare. The other is for the grid.
    vec4 d = vec4(1e5), d2 = vec4(1e5), grid = vec4(1e5);

    // Final entry needs to fill in the rest, so you give it a 100% chance of success.
    // I'd rather not say how long it took me to figure that out. :D
    vec2 rndTh[3];
    rndTh[0] = vec2(.5, .35);
    rndTh[1] = vec2(.5, .7);
    rndTh[2] = vec2(.5, 1);


    // The scale dimentions. Gets multiplied by two each iteration.
    float dim = 1.;



    // If you didn't need to worry about overlap, you wouldn't need to consider neighboring
    // cell rendering, which would make this far less complicated - One loop and a break.

    // Three tile levels.
	for(int k=0; k<3; k++){

    	// Base cell ID.
		vec2 ip = floor(oP*dim);


        for(int j=-1; j<=1; j++){
            for(int i=-1; i<=1; i++){

                // The neighboring cell ID.
                vec2 rndIJ = hash22(ip + vec2(i, j));

                // The cell IDs for the previous dimension, or dimensions, as the case may be.
                // Because the tiles overlap, rendering order matters. In this case, the tiles
                // need to laid down from largest (k = 0) to smallest (k = 2). If a large tile
                // has taken up the space, you need to check on the next iterations and skip --
                // so as not to lay smaller tiles over the larger ones.
                //
                // So why not just break from the loop? Unfortunately, there are neighboring
                // cells to check, and the IDs need to be calculated from the perspective of
                // each cell neighbor... Yeah, I'm confused too. You can either take my word
                // for it, or better yet, come up with a more elegant solution. :)
                vec2 rndIJ2 = hash22(floor((ip + vec2(i, j))/2.));
                vec2 rndIJ4 = hash22(floor((ip + vec2(i, j))/4.));
				
                // If the previous large tile has been rendered, continue.
                if(k==1 && rndIJ2.y<rndTh[0].y) continue;
                // If any of the two previous larger have been rendered, continue.
                if(k==2 && (rndIJ2.y<rndTh[1].y || rndIJ4.y<rndTh[0].y)) continue;


                // If the random cell ID at this particular scale is below a certain threshold,
                // render the tile. The code below is a little messy, due to to fact that I wanted
                // to render a few different tile styles without bloating the code too much.
                // This meant a bunch of random coordinate flipping, reflecting, etc. As mentioned,
                // I'll provide a much simpler example later.
				//
                if(rndIJ.y<rndTh[k].y){

                    vec2 p = mod(oP, 1./dim) - .5/dim - vec2(i, j)/dim;


                    // The grid square.
                    float square = max(abs(p.x), abs(p.y)) - .5/dim;
     			
                    // The grid lines.
                    const float lwg = .01;
                    float gr = abs(square) - lwg/2.;
                    grid.x = min(grid.x, gr);

					
                    // TILE COLOR ONE.

                    // Standard Truchet rotation and flipping -- based on a random cell ID.
                    if(rndIJ.x<rndTh[k].x) p.xy = p.yx;
                    if(fract(rndIJ.x*57.543 + .37)<rndTh[k].x) p.x = -p.x;



                    // Rotating by 90 degrees, then reflecting across both axes by the correct
                    // distance to produce four circles on the midway points of the grid boundary
                    // lines... A lot of this stuff is just practice. Do it often enough and
                    // it'll become second nature... sometimes. :)
                    vec2 p2 = abs(vec2(p.y - p.x, p.x + p.y)*.7071) - vec2(.5, .5)*.7071/dim;
                    float c3 = length(p2) - .5/3./dim;

                    float c, c2;

                    // Truchet arc one.
                    c = abs(length(p - vec2(-.5, .5)/dim) - .5/dim) - .5/3./dim;

                    // Truchet arc two.
                    if(fract(rndIJ.x*157.763 + .49)>.35){
                        c2 = abs(length(p - vec2(.5, -.5)/dim) - .5/dim) - .5/3./dim;
                    }
                    else{
                        // Circles at the mid boundary lines -- instead of an arc.
                        // c2 = 1e5; // In some situations, just this would work.
                        c2 = length(p -  vec2(.5, 0)/dim) - .5/3./dim;
                        c2 = min(c2, length(p -  vec2(0, -.5)/dim) - .5/3./dim);
                    }


                    // Randomly overiding some arcs with lines.
                    #ifdef INCLUDE_LINE_TILES
                        if(fract(rndIJ.x*113.467 + .51)<.35){
                        	c = abs(p.x) - .5/3./dim;
                        }
                        if(fract(rndIJ.y*113.467 + .51)<.35){
                        	c2 = abs(p.y) - .5/3./dim;
                        }
                    #endif


					// Truch arcs, lines, or dots -- as the case may be.
                    float truchet = min(c, c2);

                    // Carving out a mild channel around the line to give a faux weave effect.
                    #ifdef INCLUDE_LINE_TILES
                    	float lne = abs(c - .5/6./dim + .5/12./dim) - .5/12./dim;
     					truchet = max(truchet, -lne);
                    #endif

                    // Each tile has two colors. This is the first, and it's rendered on top.
                    c = min(c3, max(square, truchet));
                    d[k] = min(d[k], c); // Tile color one.


                    // TILE COLOR TWO.
                    // Repeat trick, to render four circles at the grid vertices.
                    p = abs(p) - .5/dim;
                    float l = length(p);
                    // Four circles at the grid vertices and the square.
                    c = min(l - 1./3./dim, square);
                    //c = max(c, -truchet);
                    //c = max(c, -c3);
                    d2[k] = min(d2[k], c); // Tile color two.

                    // Rendering some circles at the actual grid vertices. Mouse down to see it.
                    grid.y = min(grid.y, l - .5/9./dim); //.05/(dim*.35 + .65)
                    grid.z = min(grid.z, l);


                }



            }
        }


        dim *= 2.;


    }


    // The scene color. Initiated to grey.
    vec3 col = vec3(.25);


    // Just a simple lined pattern.
    float pat3 = clamp(sin((oP.x - oP.y)*6.283*iResolution.y/24.)*1. + .9, 0., 1.)*.25 + .75;
    // Resolution based falloff... Insert "too may different devices these days" rant here. :D
    float fo = 5./iResolution.y;


    // Tile colors.
    vec3 pCol2 = vec3(.125);
    vec3 pCol1 = vec3(1);

    //The spectrum color option overides the pink option.
    #ifdef SPECTRUM_COLORED
    pCol1 = vec3(.7, 1.4, .4);
    #else
    // Pink version.
        #ifdef PINK
        pCol1 = mix(vec3(1, .1, .2), vec3(1, .1, .5), uv.y*.5 + .5);;
        pCol2 = vec3(.1, .02, .06);
        #endif
    #endif




	#ifdef STACKED_TILES
        // I provided this as an option becaue I thought it might be useful
        // to see the tile layering process.

        float pw = .02;
        d -= pw/2.;
        d2 -= pw/2.;

        // Render each two-colored tile, switching colors on alternating iterations.
    	for (int k=0; k<3; k++){

            col = mix(col, vec3(0), (1. - smoothstep(0., fo*5., d2[k]))*.35);
            col = mix(col, vec3(0), 1. - smoothstep(0., fo, d2[k]));
            col = mix(col, pCol2, 1. - smoothstep(0., fo, d2[k] + pw));

            col = mix(col, vec3(0), (1. - smoothstep(0., fo*5., d[k]))*.35);
            col = mix(col, vec3(0), 1. - smoothstep(0., fo, d[k]));
            col = mix(col, pCol1, 1. - smoothstep(0., fo, d[k] + pw));

            vec3 temp = pCol1; pCol1 = pCol2; pCol2 = temp;
        }

        col *= pat3;

    #else

        // Combining the tile layers into a continuous surface. I'd like to say that
        // I applied years of topological knowledge to arrive at this, but like most
        // things, I threw a bunch of formulas at the screen in frustration until I
        // fluked the solution. :D There was a bit of logic applied though. :)
        d.x = max(d2.x, -d.x);
        d.x = min(max(d.x, -d2.y), d.y);
        d.x = max(min(d.x, d2.z), -d.z);

        // A couple of distance field patterns and a shade.
        float pat = clamp(-sin(d.x*6.283*20.) - .0, 0., 1.);
        float pat2 = clamp(sin(d.x*6.283*16.)*1. + .9, 0., 1.)*.3 + .7;
        float sh = clamp(.75 + d.x*2., 0., 1.);

        #ifdef SPECTRUM_COLORED

            col *= pat;

    		// Render the combined shape.
            d.x = -(d.x + .03);

            col = mix(col, vec3(0), (1. - smoothstep(0., fo*5., d.x)));
            col = mix(col, vec3(0), 1. - smoothstep(0., fo, d.x));
            col = mix(col, vec3(.8, 1.2, .6), 1. - smoothstep(0., fo*2., d.x + .02));
            col = mix(col, vec3(0), 1. - smoothstep(0., fo*2., d.x + .03));
            col = mix(col, vec3(.7, 1.4, .4)*pat2, 1. - smoothstep(0., fo*2., d.x + .05));

            col *= sh;

        #else

            //d.x -= .01;
            col = pCol1;

    		// Render the combined shape.
            col = mix(col, vec3(0), (1. - smoothstep(0., fo*5., d.x))*.35);
            col = mix(col, vec3(0), 1. - smoothstep(0., fo, d.x));
            col = mix(col, pCol2, 1. - smoothstep(0., fo, d.x + .02));


            col *= pat3; // Line decroation.
        #endif

	#endif



    // Mild spotlight.
    col *= max(1.15 - length(uv)*.5, 0.);


    // Click the left mouse button to show the underlying quadtree grid structure. It's
    // helpful to see the cell borders to see the random tile constructions.
    if(iMouse.z>0.){


        vec3 vCol1 = vec3(.8, 1, .7);
        vec3 vCol2 = vec3(1, .7, .4);

        #ifdef PINK
        vCol1 = vCol1.zxy;
        vCol2 = vCol2.zyx;
        #endif

        // Grid lines.
        vec3 bg = col;
        col = mix(col, vec3(0), (1. - smoothstep(0., .02, grid.x - .02))*.7);
        col = mix(col, vCol1 + bg/2., 1. - smoothstep(0., .015, grid.x));

        // Circles on the grid vertices.
        float fo = .5/iResolution.y/grid.z;
        col = mix(col, vec3(0), (1. - smoothstep(0., fo*3., grid.y - .02))*.5);
    	col = mix(col, vec3(0), 1. - smoothstep(0., fo, grid.y - .02));
        col = mix(col, vCol2, 1. - smoothstep(0., fo, grid.y));
        col = mix(col, vec3(0), 1. - smoothstep(0., fo, grid.z - .001));
    }


    // Mix the colors, if the spectrum option is chosen.
    #ifdef SPECTRUM_COLORED
    col = mix(col, col.yxz, uv.y*.75 + .5); //.zxy
    col = mix(col, col.zxy, uv.x*.7 + .5); //.zxy
    #endif


    // Rough gamma correction, and output to the screen.
    fragColor = vec4(sqrt(max(col, 0.)), 1);
}
