// https://www.shadertoy.com/view/MdcXzn

// Author : Sebastien Berube
// Created : March 2015
// Modified : Jan 2016
// 
// Composition made from a repeated hexagon prism pattern.
// Hexagon prism distance function had to be modified to smooth out vertical edges.
//
// Sources:
// Inigo Quilez
// http://iquilezles.org/www/articles/distfunctions/distfunctions.htm
// http://iquilezles.org/www/articles/raymarchingdf/raymarchingdf.htm
// For those interested in the origin of sphere tracing:
// Sphere Tracing: A Geometric Method for the Antialiased Ray Tracing of Implicit Surfaces (1994)
// http://citeseer.ist.psu.edu/viewdoc/summary?doi=10.1.1.48.3825
// Spline
// http://www.lighthouse3d.com/tutorials/maths/catmull-rom-spline/
//
// License : Creative Commons Non-commercial (NC) license
//

//----------------------
// Constants
const float PI = 3.14159;
const float SCALE = 1.0;
const float MAX_DIST = 1000.0;
const float FLOOR_HEIGHT  = 0.0;
const float X_REPEAT_DIST = 0.90*SCALE;
const float Z_REPEAT_DIST = 1.05*SCALE;
const float PRIM_HEIGHT    = 1.0;
const float HEX_HALF_WIDTH = 0.26*SCALE;
const float GEOMETRY_DISPLACEMENT = 1.00;
float g_time;

struct AnimationChannels
{
    float material_roughness;   //[0-1 range]
    float geometry_width;       //[0-1 range]
    float geometry_scale;       //[0-1 range]
    float geometry_displacement;//[0-1 range]
	float geometry_smoothness;  //[0-1 range]
    vec3 camPos;                //[IR range]
    vec3 camLookAt;             //[IR range]
};
AnimationChannels g_animationChannels;

//Material ID enum
const int MATERIALID_NONE      = 0;
const int MATERIALID_FLOOR     = 1;
const int MATERIALID_SKY       = 2;
const int MATERIALID_PLASTIC   = 3;
const int MATERIALID_METAL     = 4;

//Debug flag enum
const int DEBUG_RAYLEN  = 0;
const int DEBUG_GEODIST = 1;
const int DEBUG_NORMAL  = 2;
const int DEBUG_MATID   = 3;

float fDEBUG = 0.1;

//Defines
#define saturate(x) clamp(x,0.0,1.0)
//----------------------
// Camera
struct Cam { vec3 R; vec3 U; vec3 D; vec3 o; float lens; float zoom; }; //Right, Up, Direction, origin
Cam    CAM_lookAt(vec3 target, float pitchAngleRad, float dist, float theta);
Cam    CAM_mouseLookAt(vec3 at, float dst);
Cam    CAM_animate(vec2 uv, float fTime);
vec3   CAM_getRay(Cam cam, vec2 uv);

//----------------------
// Post Process
vec3 POST_ProcessFX(vec3 c, vec2 uv);

//----------------------
// Analytic Intersections
float RAYINTERSEC_plane(vec3 o, vec3 d, vec3 po, vec3 pn)
{
    return dot(po-o,pn)/dot(d,pn); 
}

struct repeatInfo
{
    vec3 smpl; //object-space, cyclic
    vec3 anchor; //world space
};
    
#define normalized_wave(a) (0.5*a+0.5)
repeatInfo DF_repeatHex(vec3 p)
{
    //Repetition
    float xRepeatDist = X_REPEAT_DIST;
    float zRepeatDist = Z_REPEAT_DIST*0.5;
    float latticeX = (fract(p.x/xRepeatDist+0.5)-0.5)*xRepeatDist;
    float latticeY = (fract(p.z/zRepeatDist+0.5)-0.5)*zRepeatDist;
    vec2 anchorPosXZ = p.xz-vec2(latticeX,latticeY);
    p.x = latticeX; //Cyclic coords.
    p.z = latticeY;
    
    //Variation
    float period = fract(g_time/30.)*3.0;
    float theta = period*2.0*PI;
    float overallAmplitude = normalized_wave(-cos(theta)); //Overall amplitude modulation
    float waveAmplitude = g_animationChannels.geometry_displacement
                         *normalized_wave(sin(anchorPosXZ.x+anchorPosXZ.y+theta*4.0));
    float primHeight = FLOOR_HEIGHT+overallAmplitude*waveAmplitude;
     
    repeatInfo outData;
    outData.anchor = vec3(anchorPosXZ[0], primHeight, anchorPosXZ[1]);
    outData.smpl = p;
    
    return outData;
}

#define zclamp(a) max(a,0.0) //Clamp negative values at zero
float DF_RoundedHex( vec3 p, float width, float height)
{
    //Modified version (smooth edges) of the exagon prism found here:
    //http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
    float smoothRadius = g_animationChannels.geometry_smoothness*0.2;
    width -= smoothRadius*2.0;
    
    //Hexagon prism constructed using X,Y,Z symmetry.
    //Only quadrant 1 needs to be solved, but the joining diagonal to quadrant IV is also
    //required for distance blending (see db).
    p = abs(p);
    
    //Hexagonal edge distances :
    //Note : [.8666,0.5] = [sin(PI/3,cos(PI/3)] -> Hexagon edges rotation coeff (60 degrees).
    float da = (p.x*0.866025+p.z*0.5)-width; //quadrant I diagonal edge distance
    float db = (p.x*0.866025-p.z*0.5)-width; //quadrant IV diagonal edge distance (needed for blending)
    float dc = p.z-width; //upper distance
    
    vec3 d = zclamp(vec3(da,db,dc));
    //Note: this is not an euclidian length, therefore this operation slightly distorts our distance field.
    //Yet, it is harmless to convergence, and does the smoothing job quite well.
    float dw = length(d)-smoothRadius; //hexagonal part smoothness (blending at 60 deg)
    float dh = p.y-height;
    
    //Now that we have xz distance(dw) and y distance (dh), we can compute the distance 
    //for the given isovalue (the smoothing radius).
    //Note : internal distance (maxX,maxY,maxZ) is also used to genereate internal signed dist,
    //       helping convergence when overstepping (very frequent with domain repetition).
    float externalDistance = length(zclamp(vec2(dh,dw)))-smoothRadius; //Smoothed, unsigned
	float internalDistance = max(max(da,dc),dh); //Sharp, signed.
    return min(externalDistance,internalDistance);
}

struct DF_out
{
    float d;
    int matID;
    vec3 objectPos;
};
    
//The distance field composition.
//::DF_composition
DF_out DF_composition( in vec3 pos )
{
    //Explanation:
    //http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
    DF_out oFloor;
    DF_out oHexA;
    DF_out oHexB;
    
    oHexA.matID = MATERIALID_PLASTIC;
    repeatInfo infoA = DF_repeatHex(pos-vec3(0));
	oHexA.objectPos = infoA.anchor;
    oHexA.d = DF_RoundedHex( infoA.smpl-vec3(0,infoA.anchor.y,0),
	                         g_animationChannels.geometry_width*HEX_HALF_WIDTH, PRIM_HEIGHT );
    
    oHexB.matID = MATERIALID_PLASTIC;
    repeatInfo infoB = DF_repeatHex(pos-vec3(X_REPEAT_DIST*0.5,0, Z_REPEAT_DIST*0.25));
	oHexB.objectPos = infoB.anchor;
    oHexB.d = DF_RoundedHex( infoB.smpl-vec3(0,infoB.anchor.y,0),
	                         g_animationChannels.geometry_width*HEX_HALF_WIDTH, PRIM_HEIGHT );
    
    if(oHexA.d<oHexB.d)
        return oHexA;
    else
        return oHexB;
}

//The distance field gradient
vec3 DF_gradient( in vec3 p )
{
    //The field gradient is the distance derivative along each axis.
    //The surface normal follows the direction where this variation is strongest.
	const float d = 0.001;
	vec3 grad = vec3(DF_composition(p+vec3(d,0,0)).d-DF_composition(p-vec3(d,0,0)).d,
                     DF_composition(p+vec3(0,d,0)).d-DF_composition(p-vec3(0,d,0)).d,
                     DF_composition(p+vec3(0,0,d)).d-DF_composition(p-vec3(0,0,d)).d);
	return grad/(2.0*d);
}

#define OVERSTEP_COMPENSATION 1

//o = ray origin, d = direction, t = distance travelled along ray, starting from origin
float RAYMARCH_isosurface( vec3 o, vec3 d, float isoSurfaceValue)
{
    //Learned from Inigo Quilez DF ray marching :
    //http://www.iquilezles.org/www/articles/raymarchingdf/raymarchingdf.htm
    //Original articles (interesting read) :
    //Sphere Tracing: A Geometric Method for the Antialiased Ray Tracing of Implicit Surfaces (1989)
    //http://mathinfo.univ-reims.fr/IMG/pdf/hart94sphere.pdf
    //John C. Hart Sphere Tracing: A Geometric Method for the Antialiased Ray Tracing of Implicit Surfaces (1994)
    //http://citeseer.ist.psu.edu/viewdoc/summary?doi=10.1.1.48.3825 p. 5.75-5.85
    
    const float tolerance = 0.0001;
    float t = 0.0;
    float dist = MAX_DIST;
    #if OVERSTEP_COMPENSATION
    for( int i=0; i<30; i++ )
    {
        dist = DF_composition( o+d*t ).d;
        dist -= isoSurfaceValue;
        
        if( abs(dist)<tolerance*100.0 ) break;
        t += dist;
    }
    
    t -= Z_REPEAT_DIST/2.0;
    
    for( int i=0; i<30; i++ )
    {
        dist = DF_composition( o+d*t ).d;
        dist -= isoSurfaceValue;
        
        if( abs(dist)<tolerance ) break;
        
        t += min(dist,Z_REPEAT_DIST/5.0);
    }
    #else
    for( int i=0; i<70; i++ )
    {
        dist = DF_composition( o+d*t ).d;
        dist -= isoSurfaceValue;
        
        if( abs(dist)<tolerance ) break;
        t += dist;
    }
    #endif
    
    return t;
}

#define saturate(x) clamp(x,0.0,1.0)
float RAYMARCH_DFSS( vec3 o, vec3 L, float coneWidth )
{
    //Variation of the Distance Field Soft Shadow from : https://www.shadertoy.com/view/Xds3zN
    //Initialize the minimum aperture (angle tan) allowable with this distance-field technique
    //(45deg: sin/cos = 1:1)
    float minAperture = 1.0; 
    float t = 0.0; //initial travel distance, from geometry surface (usually, pretty close)
    float dist = 10.0;
    for( int i=0; i<7; i++ )
    {
        vec3 p = o+L*t; //Sample position = ray origin + ray direction * travel distance
        float dist = DF_composition( p ).d;
        dist = min(dist,t);
        float curAperture = dist/t; //Aperture ~= cone angle tangent (sin=dist/cos=travelDist)
        minAperture = min(minAperture,curAperture);
        //Step size : limit range (0.02-0.42)
        t += 0.02+min(dist,0.4);
    }
    
    //The cone width controls shadow transition. The narrower, the sharper the shadow.
    return saturate(minAperture/coneWidth); //Should never exceed [0-1]. 0 = shadow, 1 = fully lit.
}

float RAYMARCH_DFAO( vec3 o, vec3 N, float isoSurfaceValue)
{
    //Variation of DFAO from : https://www.shadertoy.com/view/Xds3zN
    //Interesting reads:
    //https://docs.unrealengine.com/latest/INT/Engine/Rendering/LightingAndShadows/DistanceFieldAmbientOcclusion/index.html#howdoesitwork?
    //Implementation notes:
    //-Doubling step size at each iteration
    //-Allowing negative distance field values to contribute, making cracks much darker
    //-Not reducing effect with distance (specific to this application)
    float MaxOcclusion = 0.0;
    float TotalOcclusion = 0.0;
    const int nSAMPLES = 4;
    float stepSize = 0.11/float(nSAMPLES);
    for( int i=0; i<nSAMPLES; i++ )
    {
        float t = 0.01 + stepSize;
        //Double distance each iteration (only valid for small sample count, e.g. 4)
        stepSize = stepSize*2.0;
        float dist = DF_composition( o+N*t ).d-isoSurfaceValue;
        //Occlusion factor inferred from the difference between the 
        //distance covered along the ray, and the distance from other surrounding geometry.
        float occlusion = zclamp(t-dist);
        TotalOcclusion += occlusion;//Not reducing contribution on each iteration
        MaxOcclusion += t;
    }
    
    //Here, TotalOcclusion can actually exceed MaxOcclusion, where the rays
    //get inside the shape and grab negative occlusion values. It does look good
    //that way IMHO (much darker in the cracks), therefore the maximum occlusion is bumped
    //25% to allow those cracks to get darker.
    return saturate(1.0-TotalOcclusion/(MaxOcclusion*1.25));
}

struct TraceData
{
    float rayLen;  //Ray travel distance
    vec3  rayDir;  //Ray direction
    float geoDist; //Distance to geometry (error on final position)
    vec3  normal;  //Geometry normal
    vec3  objectPos; //Object position (center)
    int   matID;     //Material ID
};
    
TraceData new_TraceData()
{
    TraceData td;
    td.rayLen = 0.;
    td.rayDir = vec3(0);
    td.geoDist = 0.;
    td.normal = vec3(0);
    td.objectPos = vec3(0);
    td.matID = MATERIALID_NONE;
    return td;
}

vec3 PBR_HDRremap(vec3 c)
{
    float fHDR = smoothstep(2.900,3.0,c.x+c.y+c.z);
    return mix(c,1.3*vec3(4.5,3.5,3.0),fHDR);
}

//http://refractiveindex.info/?shelf=3d&book=liquids&page=water
const float F_DIELECTRIC_PLASTIC = 1.49; //@550nm
const float F_DIELECTRIC_WATER   = 1.33; //@550nm
const float F_DIELECTRIC_DIAMOND = 2.42; //@550nm

//ior = index of refraction
//n = refraction index
vec3 PBR_Fresnel_Schlick_Dielectric(vec3 n, float VdotH)
{
	//<Source : https://en.wikipedia.org/wiki/Schlick%27s_approximation>
	vec3 F0 = abs ((1.0 - n) / (1.0 + n));
	return F0 + (1.-F0) * pow( 1. - VdotH, 5.);
    //</Source : https://en.wikipedia.org/wiki/Schlick%27s_approximation>
}

vec3 PBR_ABL_Equation(vec3 V, vec3 L, vec3 N, float roughness, float metallic, vec3 ior_n, vec3 ior_k)
{
    roughness = max(roughness,0.01);
    
	vec3 H = normalize(L+V);
	float NdotH = dot(N,H);//Nn.H;
	float NdotL = dot(N,L);//Nn.Ln;
	float VdotH = dot(V,H);//Vn.H;
    float NdotV = dot(N,V);//Nn.Vn;
    
    //Distribution term
    //This D value is an approximation of the probability for a given light to bounce into the viewing vector direction.
	//It is not necessarily 100% mathematically/physically correct : this is still just a function which has a curve that decently
    //matches the physical distribution.
    //<Source: https://de45xmedrsdbp.cloudfront.net/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf p.3/59>
    float PI = 3.14159;
    float alpha2 = roughness * roughness;
    float NoH2 = NdotH * NdotH;
    float den = NoH2*(alpha2-1.0)+1.0;
    float D = (NdotH>0.)?alpha2/(PI*den*den):0.0;
	//</https://de45xmedrsdbp.cloudfront.net/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf p.3/59>
    
    //Fresnel term
    vec3 F = PBR_Fresnel_Schlick_Dielectric(ior_n, VdotH);
    
    //Geometric term
    //<Source: https://de45xmedrsdbp.cloudfront.net/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf p.3/59>
    float Gk = (roughness+1.)*(roughness+1.)/8.; //<-Disney's modification for ABL
    float Gl = max(NdotL,0.)/(NdotL*(1.0-Gk)+Gk);
    float Gv = max(NdotV,0.)/(NdotV*(1.0-Gk)+Gk);
    float G = Gl*Gv;
    //</https://de45xmedrsdbp.cloudfront.net/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf p.3/59>
    
    //The PBR equation seen pretty much everywhere:
    //<Source : https://seblagarde.wordpress.com/2015/07/14/siggraph-2014-moving-frostbite-to-physically-based-rendering/ p.14>
    //<Source : http://www.codinglabs.net/article_physically_based_rendering_cook_torrance.aspx>
    float softTr = 0.2; // Valid range : [0.001-0.25]. Will reduce reflexivity on edges if too high.
    //Personal addition : This parameter softens up the transition at grazing angles (otherwise too sharp IMHO).
    vec3 Rs = D*F*G / (4.*NdotV*NdotL*(1.0-softTr)+softTr);
    //<Source : http://www.codinglabs.net/article_physically_based_rendering_cook_torrance.aspx>
    
	return Rs;
}

#define saturate(x) clamp(x,0.0,1.0)
vec3 MAT_Plastic(TraceData traceData, vec3 cDiff, vec3 N, vec3 V, vec3 L0, vec3 L1, float dfao, float dfss0, float dfss1)
{
    vec3 col = vec3(0);
    
    float fRoughness = g_animationChannels.material_roughness;
    
    //Ambient directional contribution (3x):
    //           color*directionalContribution(<normal,ambientDir>)
    //This give a basic "ambient" shading, which varies with normal angle
    vec3 cAmb  = vec3(0.26,0.24,0.23)*vec3(0.5+0.5*dot(traceData.normal,vec3(+0.08,1,+0.1)))
               + vec3(0.25,0.25,0.30)*vec3(0.5+0.5*dot(traceData.normal,vec3(-0.28,1,-0.17)))
               + vec3(0.19,0.25,0.30)*vec3(0.5+0.5*dot(traceData.normal,vec3(+0.28,1,-0.27)));
    //2 x PBR lights
    vec3 CL0  = PBR_HDRremap(vec3(1))*PBR_ABL_Equation(V,L0,traceData.normal, fRoughness, 0., vec3(F_DIELECTRIC_PLASTIC), vec3(0));
    vec3 CL1  = PBR_HDRremap(vec3(1))*PBR_ABL_Equation(V,L1,traceData.normal, fRoughness, 0., vec3(F_DIELECTRIC_PLASTIC), vec3(0));
    
    col = cAmb*dfao;
    col *= saturate(0.30+fRoughness*0.5+0.2*(dfss0+dfss1));
    col += (dfss0+fRoughness*0.25)*CL0;
    col += (dfss1+fRoughness*0.25)*CL1;
    
    return col*0.75;
}

float SAMPLER_trilinear(vec3 p)
{
    //Noise layering trick from Inigo Quilez.
    //See this for more explanation: https://www.shadertoy.com/view/Ms3SRr
    const float TEXTURE_RES = 256.0; //Noise texture resolution
    p *= TEXTURE_RES;   //Computation in pixel space (1 unit = 1 pixel)
    vec3 pixCoord = floor(p);//Pixel coord, integer [0,1,2,3...256...]
    vec3 t = p-pixCoord;     //Pixel interpolation position, linear range [0-1] (fractional part)
    t = (3.0 - 2.0 * t) * t * t; //interpolant easing function : linear->cubic
    vec2 layer_translation = -pixCoord.y*vec2(37.0,17.0)/TEXTURE_RES; //noise volume stacking trick : g layer = r layer shifted by (37x17 pixels -> this is no keypad smashing, but the actual translation embedded in the noise texture).
//    vec2 layer1_layer2 = texture(iChannel0,layer_translation+(pixCoord.xz+t.xz+0.5)/TEXTURE_RES,-100.0).xy; //Note : +0.5 to fall right on pixel center
    vec2 layer1_layer2 = vec2(0.0, 0.0);
    return mix( layer1_layer2.x, layer1_layer2.y, t.y ); //Layer interpolation (trilinear/volumetric)
}

float MAT_remap_angle_probability(float x_01)
{
    //cos(jitter) is used to alter probabilty distribution : 
    //it remaps an evenly distributed function into another 
    //one where closer angles are more probable, and wider
    //angles are less probable.
    return (1.0-cos(x_01*PI/2.0));
}

vec3 MAT_addFog(float travelDist, in vec3 color, in vec3 p, in vec3 c_atmosphere)
{
    float a = 0.08;
    float NORMALIZATION_TERM = log((1.+a)/a);
    float da = travelDist/50.0;
    da = log((da+a)/a)/NORMALIZATION_TERM;
    vec3 FinalColor = mix(color,c_atmosphere,saturate(da));
    return FinalColor;
}

//::MAT_apply
vec4 MAT_apply(vec3 pos, TraceData traceData)
{
    vec3 c_atmosphere = mix(vec3(0.87,0.94,1.0),vec3(0.6,0.80,1.0),clamp(3.0*pos.y/length(pos.xz),0.,1.));
    
    if(traceData.matID==MATERIALID_SKY)
    {
        return vec4(c_atmosphere,1.0);
    }
    
    vec4 col = vec4(0);
    vec3 N = traceData.normal;
    vec3 V = normalize(-traceData.rayDir);
    vec3 L0 = normalize(vec3(0.5,1.2,0.3));
    vec3 L1 = normalize(vec3(-L0.x,L0.y,-L0.z+0.5));
    
    //<Jittered AO Samples around Y axis, to reduce artifacts associated with closely repeated geometry>
    float fNoiseAmplitude = 0.4;
    float jitter_01 = SAMPLER_trilinear(pos*10.0+g_time*50.0);
    float t = MAT_remap_angle_probability(jitter_01)*fNoiseAmplitude;
    vec3 Na = vec3(N.xz*mat2(cos(t),sin(t),-sin(t),cos(t)),N.y).xzy; //Rotate(t)
    jitter_01 = SAMPLER_trilinear(5.0+pos*9.11);
    t = MAT_remap_angle_probability(jitter_01)*fNoiseAmplitude;
    vec3 Nb = vec3(N.xz*mat2(cos(t),-sin(t),sin(t),cos(t)),N.y).xzy; //Rotate(-t)
    float dfaoA = RAYMARCH_DFAO( pos, Na, 0.02);
    float dfaoB = RAYMARCH_DFAO( pos, Nb, 0.02);
    float dfaoAveraged = 0.5*(dfaoA+dfaoB);
    //</Jittered AO Samples>
    
    float dfss0 = RAYMARCH_DFSS( pos+L0*0.01, L0, 0.2);
    float dfss1 = RAYMARCH_DFSS( pos+L1*0.01, L1, 0.2);
    
    if(traceData.matID==MATERIALID_PLASTIC)
    {
        col.rgb = MAT_Plastic(traceData, vec3(1), N, V, L0, L1, dfaoAveraged, dfss0, dfss1);
    }
    
    col.rgb = MAT_addFog(traceData.rayLen*0.3, col.rgb, pos, c_atmosphere);
    
    return col;
}

float TRACE_zprime(vec3 o, vec3 d)
{
    float geometryCeiling = FLOOR_HEIGHT+PRIM_HEIGHT
	                       +g_animationChannels.geometry_displacement*GEOMETRY_DISPLACEMENT;
    float t = RAYINTERSEC_plane(o, d, vec3(0,geometryCeiling,0), vec3(0,1,0));
    return (t<0.0)?MAX_DIST:t;
    return t;
}

//o=ray origin, d=ray direction
//::TRACE_geometry
TraceData TRACE_geometry(vec3 o, vec3 d)
{
    //Raymarching (the expensive function)
    TraceData dfTrace;
    float rayLen = RAYMARCH_isosurface(o,d,0.0);
    vec3 dfHitPosition = o+rayLen*d;
    
    //Additional sample, to gather material ID and other info
    //(we want that stuff coompiled out of the raymarching loop, it clutters the code and might slow things down)
    DF_out compInfo = DF_composition( dfHitPosition );
    rayLen += compInfo.d;
    dfHitPosition = o+rayLen*d;
        
    dfTrace.rayLen     = rayLen;
    dfTrace.matID      = compInfo.matID;
    dfTrace.objectPos  = compInfo.objectPos;
    dfTrace.geoDist    = compInfo.d;
    dfTrace.rayDir     = d;
    dfTrace.normal     = normalize(DF_gradient(dfHitPosition));
    
    return dfTrace;
}

vec3 TRACE_debug(TraceData traceData, int elemID)
{
    if(elemID==DEBUG_RAYLEN)  return vec3(log(traceData.rayLen)*0.2);
    if(elemID==DEBUG_GEODIST) return vec3(traceData.geoDist);
    if(elemID==DEBUG_NORMAL)  return traceData.normal;
    if(elemID==DEBUG_MATID)   return traceData.matID==MATERIALID_PLASTIC?vec3(1):
                                     vec3(traceData.matID==MATERIALID_FLOOR?1:0,
                                          traceData.matID==MATERIALID_METAL?1:0,
                                          traceData.matID==MATERIALID_SKY?1:0);
    return vec3(0);
}

const int SPLINE_POINT_COUNT = 8;
struct SPLINE_CtrlPts
{
    vec4 p[SPLINE_POINT_COUNT];
};
vec4 SPLINE_PointArray(int i, SPLINE_CtrlPts ctrlPts)
{
    //Just a way to get around the fact global arrays do not support random index access.
    //(only texture/resources)
    if(i==0 || i==SPLINE_POINT_COUNT  ) return ctrlPts.p[0];
    if(i==1 || i==SPLINE_POINT_COUNT+1) return ctrlPts.p[1];
    if(i==2 || i==SPLINE_POINT_COUNT+2) return ctrlPts.p[2];
    if(i==3) return ctrlPts.p[3];
    if(i==4) return ctrlPts.p[4];
    if(i==5) return ctrlPts.p[5];
    if(i==6) return ctrlPts.p[6];
    if(i==7) return ctrlPts.p[7];
    return vec4(0);
}

vec4 SPLINE_catmullRom(float fTime, SPLINE_CtrlPts ctrlPts)
{
    float t = fract(fTime);
    const float n = float(SPLINE_POINT_COUNT);
    
    int idxOffset = int(t*n);
    vec4 p1 = SPLINE_PointArray(idxOffset,ctrlPts);
    vec4 p2 = SPLINE_PointArray(idxOffset+1,ctrlPts);
    vec4 p3 = SPLINE_PointArray(idxOffset+2,ctrlPts);
    vec4 p4 = SPLINE_PointArray(idxOffset+3,ctrlPts);
    
    //For some reason, fract(t) returns garbage on my machine with small values of t.
    //return fract(n*t);
    //Using this below yields the same results, minus the glitches.
    t *= n;
    t = (t-float(int(t)));
    
    //A classic catmull-rom
    //e.g.
    //http://steve.hollasch.net/cgindex/curves/catmull-rom.html
    //http://www.lighthouse3d.com/tutorials/maths/catmull-rom-spline/
    vec4 val = 0.5 * ((-p1 + 3.*p2 -3.*p3 + p4)*t*t*t
               + (2.*p1 -5.*p2 + 4.*p3 - p4)*t*t
               + (-p1+p3)*t
               + 2.*p2);
    return val;
}

void ANIM_main(float fTime)
{
    float t1 = 0.010*fTime;
    float t2 = 0.010*fTime+0.03;
    
    SPLINE_CtrlPts cameraPosKeyFrames; //100 sec cycle.
    //                    DATA: PosX,PosY,PosZ,Tilt
    cameraPosKeyFrames.p[1] = vec4(10.0,2.70,05.0,1.90); //t=00.0s
    cameraPosKeyFrames.p[2] = vec4(16.0,3.30,08.5,1.00); //t=12.5s
    cameraPosKeyFrames.p[3] = vec4(20.0,6.80,05.0,2.97); //t=25.0s
    cameraPosKeyFrames.p[4] = vec4(40.0,3.40,17.5,0.82); //t=37.5s
    cameraPosKeyFrames.p[5] = vec4(30.0,3.10,27.5,1.97); //t=50.0s
    cameraPosKeyFrames.p[6] = vec4(25.0,3.20,22.5,1.93); //t=62.5s
    cameraPosKeyFrames.p[7] = vec4(15.0,3.00,24.5,1.95); //t=75.0s
    cameraPosKeyFrames.p[0] = vec4(05.0,2.80,12.5,1.20); //t=87.5s
    vec4 cameraPos = SPLINE_catmullRom(t1,cameraPosKeyFrames);
    vec4 cameraDir = normalize(SPLINE_catmullRom(t2,cameraPosKeyFrames)-cameraPos);
        
    SPLINE_CtrlPts geometryKeyFrames; //25 sec cycle.
    //                      DATA: round,width,roughness,displacement
	geometryKeyFrames.p[1] = vec4(0.070,1.000,0.30,1.000); //t=00.0s
    geometryKeyFrames.p[2] = vec4(0.090,0.900,0.50,0.900); //t=01.25s
    geometryKeyFrames.p[3] = vec4(0.080,1.000,0.20,1.000); //t=02.50s
    geometryKeyFrames.p[4] = vec4(0.150,0.970,0.50,0.990); //t=03.75s
    geometryKeyFrames.p[5] = vec4(0.090,0.820,0.50,0.820); //t=05.00s
    geometryKeyFrames.p[6] = vec4(0.110,0.970,0.50,0.990); //t=06.25s
    geometryKeyFrames.p[7] = vec4(0.050,0.930,0.50,0.930); //t=07.50s
    geometryKeyFrames.p[0] = vec4(0.120,0.950,0.50,0.980); //t=08.75s
    vec4 geoPose = SPLINE_catmullRom(t1*25.0,geometryKeyFrames);
    
    g_animationChannels.camPos    = cameraPos.xyz;
    g_animationChannels.camLookAt = cameraPos.xyz+cameraDir.xyz-vec3(0,cameraPos.w,0);
    g_animationChannels.geometry_smoothness = geoPose[0];
    g_animationChannels.material_roughness = 0.45;
    g_animationChannels.geometry_width = geoPose[1];
    g_animationChannels.geometry_displacement = GEOMETRY_DISPLACEMENT;
}

vec3 TRACE_main( vec3 o, vec3 dir, vec2 uv)
{ 
    float fRemainingAlpha = 1.0;
    float zStart = TRACE_zprime(o, dir);
    vec3 pt = o+dir*zStart;
    vec3 ptGeo = vec3(0);
    
    TraceData geometryTraceData;
    if(zStart< MAX_DIST)
    {
        geometryTraceData = TRACE_geometry(pt, dir);
        geometryTraceData.rayLen += zStart;
        ptGeo = o+dir*geometryTraceData.rayLen;
    }
    else
    {
        geometryTraceData.rayLen     = MAX_DIST;
    	geometryTraceData.matID      = MATERIALID_SKY;
    	geometryTraceData.objectPos  = pt;
    	geometryTraceData.geoDist    = 0.0;
    	geometryTraceData.rayDir     = dir;
        ptGeo = pt;
    }
    
    //return TRACE_debug(geometryTraceData, DEBUG_RAYLEN);  //OK
    //return TRACE_debug(geometryTraceData, DEBUG_GEODIST); //OK
    //return TRACE_debug(geometryTraceData, DEBUG_NORMAL);  //OK
    //return TRACE_debug(geometryTraceData, DEBUG_MATID);   //OK
    
    vec4 cFinal = MAT_apply(ptGeo,geometryTraceData);
        
    return cFinal.rgb;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    g_time = iTime+2.6; //Time offset for better preview
    vec2 uv = (fragCoord.xy-0.5*iResolution.xy) / iResolution.xx;
    
    float fTime = g_time+2.1;
    ANIM_main(fTime);
    
    Cam cam = CAM_animate(uv,fTime);
    vec3 d = CAM_getRay(cam,uv);
    vec3 c = TRACE_main(cam.o, d, uv);
    
    //No supersampling required for most PostProcessFX.
    c = POST_ProcessFX(c,uv);
    
    fragColor = vec4(c,1.0);
}

vec3 POST_ProcessFX(vec3 c, vec2 uv)
{
    //Vignetting
    float lensRadius = 0.65;
    uv /= lensRadius;
    float sin2 = uv.x*uv.x+uv.y*uv.y;
    float cos2 = 1.0-min(sin2*sin2,1.0);
    float cos4 = cos2*cos2;
    c *= cos4;
    
    //Gamma
    c = pow(c,vec3(0.4545));
    return c;
}

//----------------------
// Camera
//::CAM
Cam CAM_animate(vec2 uv, float fTime)
{
    Cam cam;
    cam.o = g_animationChannels.camPos;
    cam.D = normalize(g_animationChannels.camLookAt-cam.o);
	cam.R = normalize(cross(cam.D,vec3(0,1,0)));
    cam.U = normalize(cross(cam.R,cam.D));
    cam.lens = 1.2+0.3*sin(fTime*0.1);
    cam.zoom = 3.0+sin(fTime*0.1)/cam.lens;
	return cam;
}

vec3 CAM_getRay(Cam cam,vec2 uv)
{
    uv = cam.lens*uv/(cam.lens-length(uv)*length(uv));
    uv *= cam.zoom;
    return normalize(uv.x*cam.R+uv.y*cam.U+cam.D);
}

