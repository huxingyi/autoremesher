// https://www.shadertoy.com/view/3tXXRn

// Spherical Fibonnacci points, as described by Benjamin Keinert, Matthias Innmann, 
// Michael Sanger and Marc Stamminger in their paper (below)

//=================================================================================================
// http://lgdv.cs.fau.de/uploads/publications/spherical_fibonacci_mapping_opt.pdf
//=================================================================================================
const float PI  = 3.14159265359;
const float PHI = 1.61803398875;

// Originally from https://www.shadertoy.com/view/lllXz4
// Modified by fizzer to put out the vector q.
vec2 inverseSF( vec3 p, float n, out vec3 outq ) 
{
    float m = 1.0 - 1.0/n;
    
    float phi = min(atan(p.y, p.x), PI), cosTheta = p.z;
    
    float k  = max(2.0, floor( log(n * PI * sqrt(5.0) * (1.0 - cosTheta*cosTheta))/ log(PHI+1.0)));
    float Fk = pow(PHI, k)/sqrt(5.0);
    vec2  F  = vec2( round(Fk), round(Fk * PHI) ); // k, k+1

    vec2 ka = 2.0*F/n;
    vec2 kb = 2.0*PI*( fract((F+1.0)*PHI) - (PHI-1.0) );    
    
    mat2 iB = mat2( ka.y, -ka.x, 
                    kb.y, -kb.x ) / (ka.y*kb.x - ka.x*kb.y);
    
    vec2 c = floor( iB * vec2(phi, cosTheta - m));
    float d = 8.0;
    float j = 0.0;
    for( int s=0; s<4; s++ ) 
    {
        vec2 uv = vec2( float(s-2*(s/2)), float(s/2) );
        
        float i = round(dot(F, uv + c));
        
        float phi = 2.0*PI*fract(i*PHI);
        float cosTheta = m - 2.0*i/n;
        float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
        
        vec3 q = vec3( cos(phi)*sinTheta, sin(phi)*sinTheta, cosTheta );
        float squaredDistance = dot(q-p, q-p);
        if (squaredDistance < d) 
        {
            outq = q;
            d = squaredDistance;
            j = i;
        }
    }
    return vec2( j, sqrt(d) );
}

vec2 intersectSphere(vec3 ro, vec3 rd, vec3 org, float rad)
{
   float a = dot(rd, rd);
   float b = 2. * dot(rd, ro - org);
   float c = dot(ro - org, ro - org) - rad * rad;
   float desc = b * b - 4. * a * c;
   if (desc < 0.)
      return vec2(1, 0);

   return vec2((-b - sqrt(desc)) / (2. * a), (-b + sqrt(desc)) / (2. * a));
}

// polynomial smooth min (k = 0.1);
// from iq: https://www.iquilezles.org/www/articles/smin/smin.htm
float smin( float a, float b, float k )
{
    float h = clamp( 0.5+0.5*(b-a)/k, 0.0, 1.0 );
    return mix( b, a, h ) - k*h*(1.0-h);
}

//float smin(float a,float b,float k){ return -log2(exp2(-k*a)+exp2(-k*b))/k;}//from iq
float smax(float a,float b,float k){ return -smin(-a,-b,k);}

mat3 rotX(float a)
{
    return mat3(1., 0., 0.,
                0., cos(a), sin(a),
                0., -sin(a), cos(a));
}

mat3 rotY(float a)
{
    return mat3(cos(a), 0., sin(a),
                0., 1., 0.,
                -sin(a), 0., cos(a));
}

mat3 rotZ(float a)
{
    return mat3(cos(a), sin(a), 0.,
                -sin(a), cos(a), 0.,
                0., 0., 1.);
}

// Anti-aliasing samples count sqrt
#define AA 2

float time;

// Rotation matrix for spherical layer.
mat3 rot(float r)
{
    float t = time - r * 2.;
    float s = .5 + .5 * r;
    return rotX(cos(t / 1.5) * s) * rotY(sin(t / 3.) * s);
}

// Scene SDF
float dist(vec3 p)
{
    const float precis = 35.0;

    vec3 op = p;
    
    p = rot(length(p)) * p;
    
    // Rotational velocity estimation.
    float diff = distance(p, rot(length(op) - 1e-2) * op);

    // A scaling factor based on the rotational velocity to
    // simulate a 'bunching up' of the tentacles when they are spinning fast.
    float k = max(1e-3, 1. + diff * 1.);
    
    p *= k;
    op *= k;
    
    vec3 q;
    vec2 sf = inverseSF( normalize(p), precis, q ); 
    
    q *= k;
    
    float d = length(p);
    
    // Alternating tentacle lengths based on spiral point ID.
    float r3 = (mod(sf.x, 3.) < 1.) ? 1. : 1.45;
    float r2 = r3 / k;

    d = smax(sf.y - diff * 2.2 - .04 / dot(p, p), d - r3, 32. / 200.);
        
    // Add spheres at the ends of the tentacles using the unrotated
    // sample space, to keep them spherical.
    q = transpose(rot(r2 + .05)) * q;
    d = smin(d,  length(op - q * r2) - .1, 32. / 200.);
    
    return min(d * .6 / k, .2);
}

// A special field which is only applied when extracing surface normals.
float bump(vec3 p)
{
   return 0.0;    
}

vec3 getNormal(vec3 p)
{
    // Here the epsilon used for scene normal extraction is larger than the detailed
    // bump epsilon. The larger geometry epsilon results in a smoothing effect which helps
    // to blend the bases of the tentacles together.
    
    const vec2 eps = vec2(1e-1, 0);
    const vec2 eps2 = vec2(1e-3, 0);
    return normalize(vec3(dist(p + eps.xyy) + bump(p + eps2.xyy) - dist(p - eps.xyy) - bump(p - eps2.xyy),
                          dist(p + eps.yxy) + bump(p + eps2.yxy) - dist(p - eps.yxy) - bump(p - eps2.yxy),
                          dist(p + eps.yyx) + bump(p + eps2.yyx) - dist(p - eps.yyx) - bump(p - eps2.yyx)));
}

// Pyramid waveform
float tri(float x)
{
    return min(fract(x) * 2., 2. - 2. * fract(x));
}

vec3 render(vec2 fragCoord)
{
    // Jittered time sample for motionblur
    time = iTime + texelFetch(iChannel1, ivec2(fragCoord * 2.) & 1023, 0).r * .025;
    
    vec3 fragColor = vec3(0);
    
    vec2 uv = fragCoord / iResolution.xy * 2. - 1.;
    uv.x *= iResolution.x / iResolution.y;

    vec3 ro = vec3(0, 0, 3), rd = normalize(vec3(uv, -1.8));

    ro.y += sin(time / 4.) * .03;
    ro.x += sin(time / 5.) * .03;
    
    // Clip to a bounding sphere
    vec2 spheret = intersectSphere(ro, rd, vec3(0), 1.6);

    // Background colour
    vec3 bg = vec3(.75) * mix(vec3(.5, .5, 1.), vec3(1), .6) * (1. - smoothstep(0., 7., length(uv)));

    if(spheret.x > spheret.y)
        return bg;

    float t = spheret.x;
    float maxt = spheret.y;

    // Raymarch
    for(int i = 0; i < 80; ++i)
    {
        float d = dist(ro + rd * t);
        if(abs(d) < 1e-4 || t > maxt)
            break;
        t += d;
    }

    if(t > maxt)
    {
        fragColor.rgb = bg;
    }
    else
    {
        vec3 rp = ro + rd * t;
        vec3 n = getNormal(rp);
        vec3 r = reflect(rd, n);
        float l = length(rp);
        float fr = clamp(1. - dot(n, -rd), 0., 1.);

    	// Apply some fake shadowing to the specular highlight and the backlight,
        // by simulating a spherical occluder for the 'body' at the center of the object.
        float bodyR = .5;
        float cone = cos(atan(bodyR / l));

        float specshad = 1. - smoothstep(-.1, .1, dot(r, normalize(vec3(0) - rp)) - cone) * 1.;
        float specshad2 = 1. - smoothstep(-.3, .3, dot(r, normalize(vec3(0) - rp)) - cone) * 1.;

        // Backlight / fake SSS
        fragColor.rgb = bg * mix(vec3(.2, .5, 1.) / 2., vec3(1., .9, .8).bgr, specshad2 * pow(fr, .8));
        
        // Fake AO from center of body
        fragColor.rgb *= vec3(pow(mix(.5 + .5 * dot(n, normalize(vec3(0) - rp)), 1., smoothstep(0., 1.5, l)), .5));
        
        // Slight AO / diffuse bleeding
        fragColor.rgb *= mix(vec3(.75,1.,.75), vec3(1), smoothstep(0.1, .8, l));

        vec3 c = fragColor.rgb;
        
        // Blue / green alternating pattern
        fragColor.rgb = mix(c.bbb * vec3(.5,1.,.5), fragColor.rgb, smoothstep(.3, .7, tri(l * 4.)));
        
        // Yellow tips
        fragColor.rgb = mix(fragColor.rgb, c.bbb * vec3(1,1,.5), smoothstep(1.4, 1.5, l));
        
        // Yellow tips self-illumination
        fragColor.rgb += vec3(1,1,.4) * smoothstep(1.4, 1.5, l) * .11;

        // Specular highlight
        fragColor.rgb += specshad * .9 * smoothstep(.4, .7, dot(r, normalize(vec3(1)))) * fr;

        // Mist
        fragColor.rgb += vec3(mix(vec3(.5, .5, 1.), vec3(0), exp(-t / 25.)));
    }

    return fragColor;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    fragColor.rgb = vec3(0);
    
    // Anti-aliasing sample loop
    for(int y = 0; y < AA; ++y)
    	for(int x = 0; x < AA; ++x)
        {
			fragColor.rgb += clamp(render(fragCoord + vec2(x, y) / float(AA)), 0., 1.);
        }
    
    fragColor.rgb /= float(AA * AA);
    
    // Contrast
    fragColor.rgb = (fragColor.rgb * 1.2 - .05);
    
    // Clamp, gamma, dither
    fragColor.rgb = pow(clamp(fragColor.rgb, 0., 1.), vec3(1. / 2.2)) + texelFetch(iChannel1, ivec2(fragCoord) & 1023, 0).gba / 200.;
}
