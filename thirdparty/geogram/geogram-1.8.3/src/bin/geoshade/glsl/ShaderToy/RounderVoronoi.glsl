// https://www.shadertoy.com/view/ld3yRn

// ST_MODE = "ShaderToy mode". Do not undefine. It is here for PolyCube compatibility.
#define ST_MODE

// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// by Tomasz Dobrowolski' 2018

// Use mouse (X/Y) to control two smoothness parameters.

// PolyCube edition:
// http://polycu.be/edit/?h=X3sd5D

// This is continuation of a quest to formulate
// smooth and continuous across whole domain
// distance function to edges of random Voronoi cells.

// I just had a very simple idea today (2018-02-03)
// how to make this function continuous and smooth
// at non-zero distance at the same time, and it works!
// See "voronoi_rounder" function for details.

// My previous shader with analysis of discontinuities
// in Shane's attempt.
// https://www.shadertoy.com/view/MdSfzD

// Shane's original shader.
// https://www.shadertoy.com/view/lsSfz1

// My previous attempt to optimize distance to edges.
// https://www.shadertoy.com/view/llG3zy

// Smooth cell noise function by TinyTexel.
// https://www.shadertoy.com/view/MdByzD

// Play with some options (1 = enable, 0 = disable).
#define DOMAIN_DEFORM 0
#define ANIMATE 1

// How far cells can go off center during animation (must be <= .5)
#define ANIMATE_D .45

// Points cannot be closer than sqrt(EPSILON)
#define EPSILON .00001

#ifdef ST_MODE
#define template_time iTime
#endif

vec2 hash2(vec2 p)
{
    #if 1
       // Dave Hoskin's hash as in https://www.shadertoy.com/view/4djSRW
       vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
       p3 += dot(p3, p3.yzx+19.19);
       vec2 o = fract(vec2((p3.x + p3.y)*p3.z, (p3.x+p3.z)*p3.y));
    #else
       // Texture-based
       vec2 o = texture( iChannel0, (p+0.5)/256.0, -100.0 ).xy;
    #endif
    #if ANIMATE
       o = 0.5 + ANIMATE_D*sin( template_time*.5 + o*6.2831853 );
    #endif
   return o;
}

// Commutative smin function taken
// from Alex Evans aka Statix talk 
// http://media.lolrus.mediamolecule.com/AlexEvans_SIGGRAPH-2015.pdf
// credited to Dave Smith @media molecule
float smin(float a, float b, float r)
{
#if 0
   // Preventing division by zero.
   float f = max(0.,1.-abs(b-a)/max(1e-32,r));
#else
   float f = max(0.,1.-abs(b-a)/r);
#endif
   return min(a,b) - r*.25*f*f;
}

// Smooth abs.
// This one is equivalent to -smin(x, -x, r) - r*.25
float sabs(float x, float r)
{
   float f = max(0.,1.-abs(x + x)/r);
   return abs(x) + r*.25*(f*f - 1.);
}

// This is bullet-proof version of finding closest point
// in 4x4 area around query point "q".
// In fact 12 cells (4x4 without corners) would be enough,
// but it's less elegant to implement.
// We pass n=|q|, f=q-n, as an optimization.
float closest( in vec2 n, in vec2 f, out vec2 mr, out vec2 mg )
{
    // take half-cell position
    vec2 h = step(.5,f) - 2.;
    vec2 n2 = n + h;
    vec2 f2 = f - h;

    float md = 8.0;

    //----------------------------------
    // first pass: regular voronoi
    //----------------------------------
    for( int j=0; j<=3; j++ )
    for( int i=0; i<=3; i++ )
    {
        vec2 g = vec2(float(i),float(j));
        vec2 o = hash2( n2 + g );
        vec2 r = g + o - f2;
        float d = dot(r,r);

        if( d<md )
        {
            md = d;
            mr = r;
            mg = g;
        }
    }
    mg += h; // return cell position relative to "n"
    
    return md;
}

// A continuous and smooth at non-zero distance
// distance function to Voronoi edges.
// by Tomasz Dobrowolski' 2018
// Extending it to 3d (and more) is trivial.
// x = input coordinate
// s = smooth min cutoff parameter (smoothness inside cell)
// e = smooth abs cutoff parameter (smoothness between cells)
vec3 voronoi_rounder( in vec2 x, in float s, in float e )
{
#if DOMAIN_DEFORM
	x += sin(x.yx*10.)*.07;
#endif

    vec2 n = floor(x);
    vec2 f = fract(x);

    vec2 mr, mg;
    float md = closest(n,f,mr,mg);

    //----------------------------------
    // second pass: distance to edges
    //----------------------------------
    md = 8.0;
    for( int j=-2; j<=2; j++ )
    for( int i=-2; i<=2; i++ )
    {
        vec2 g = mg + vec2(float(i),float(j));
        vec2 o = hash2( n + g );
        vec2 r = g + o - f;

        if( dot(mr-r,mr-r)>EPSILON ) // skip the same cell
        {
            float d = dot( 0.5*(mr+r), normalize(r-mr) );

            // The whole trick to get continuous function
            // across whole domain and smooth at non-zero distance
            // is to use smooth minimum (as usual)
            // and multiple smoothness factor by distance,
            // so it becomes minimum at zero distance.
            // Simple as that!
            // If you keep smoothness factor constant (i.e. multiple by "s" only),
            // the distance function becomes discontinuous
            // (see https://www.shadertoy.com/view/MdSfzD).
            md = smin(d, md, s*d);
        }
    }

    // Totally empirical compensation for
    // smoothing scaling side-effect.
    md *= .5 + s;

    // At the end do some smooth abs
    // on the distance value.
    // This is really optional, since distance function
    // is already continuous, but we can get extra
    // smoothness from it.
    md = sabs(md, e);

    return vec3( md, mr );
}

#ifdef ST_MODE
vec3 plot( vec2 p, float ss )
{
#else
vec3 plot( vec2 p )
{
    float ss = template_tr.z;
#endif
#ifdef ST_MODE
    float s = clamp(iMouse.x/iResolution.x,.0,1.)*.95+.05;
    float e = max(.01,iMouse.y/iResolution.y)*.5;
    if (length(iMouse.xy) < .5) {
		s = .5;
        e = .005;
    }
#else
    const float s = .5;
    const float e = .01;
#endif
    vec3 c = voronoi_rounder(p, s, e);

    const float pd = 40.;
#if 0
    vec3 norm = normalize(vec3(dFdx(c.x),dFdy(c.x),ss*1.5));
    float fw1 = fwidth(c.x);
#else
    const vec2 eps = vec2(.001,0);
    float fdx = (voronoi_rounder(p + eps.xy, s, e).x - c.x)/eps.x;
    float fdy = (voronoi_rounder(p + eps.yx, s, e).x - c.x)/eps.x;
    vec3 norm = normalize(vec3(fdx, fdy, 1.5));
    float fw1 = (abs(fdx) + abs(fdy))*ss;
#endif
    float fw2 = fw1*(pd/3.141592653589793*2.7);

    const float f0 = sin(.7/pd);
    float f = sin(c.x*pd-.7);
    float od = abs(f);
    vec3 ldir = normalize(vec3(-.2,-.3,.6));
    float dd = dot(norm,ldir);
    float rd = pow(max(0.,reflect(-ldir,norm).z),16.);
    float ld = dd*dd*.7+.35;
    float c0 = c.x*.7-step(0.,f)*.05+.6;
    float c1 = c.x*.7+.33;
    vec3 col = mix(vec3(c0*ld+rd*.23), vec3(c1*ld), smoothstep(fw2,0.,od)*.7);
    col = mix(col, vec3(.1,.15,.1), smoothstep(f0+fw1,f0,c.x)*.5);
    col = sqrt(col)*1.5-.53; // some final grading
    return col;
}

#ifdef ST_MODE
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    float sc = step(0.5, iResolution.y)*2. + 3.; // scale differently for fullscreen
    float ss =  sc / iResolution.y; // size of 1 pixel
    vec2 uv = (fragCoord.xy - iResolution.xy*.5) * ss;
    fragColor = vec4(plot(uv, ss), 1.);
}
#endif
