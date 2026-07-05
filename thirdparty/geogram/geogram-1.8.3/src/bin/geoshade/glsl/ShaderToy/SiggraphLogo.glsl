// https://www.shadertoy.com/view/4sl3zn

// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

vec2 disp( in vec3 p )
{
	return vec2( pow( 0.5 + 0.5*cos( 1.0*iTime ), 2.0 ),
                 pow( 0.5 + 0.5*cos( 25.0*p.x  + 1.5*iTime)*
					            sin( 25.0*p.y  + 2.0*iTime )*
					            sin( 25.0*p.z  + 1.0*iTime ), 3.0) );
}

float obj( in vec3 p )
{
	vec3 ax = vec3(-2.0,2.0,1.0)/3.0;
	vec3 ce = vec3(0.0,-0.2,-0.2);

	float d1 = dot(p,ax) - 0.1;
    float d2 = length(p) - 1.0;
	float d3 = length( p-ce - ax*dot(p-ce,ax)) - 1.0;

	return max( max( d1, d2 ), -d3 );
}

vec2 map( in vec3 p )
{
	float d1 = obj( p );
	float d2 = obj( p*vec3(-1.0,-1.0,1.0) );

    vec2        res = vec2( d1, 0.0 );
	if( d2<d1 ) res = vec2( d2, 1.0 );

	vec2 di = disp( p );
	res.x -= 0.04*di.x*di.y;

	return res;
}

vec2 intersect( in vec3 ro, in vec3 rd )
{
	float t = 0.0;
	vec2 h = vec2( -1.0 );
    for( int i=0; i<32; i++ )
    {
        h = map(ro+rd*t);
		t += h.x;
	}
	
	if( h.x<0.1 ) return vec2(t,h.y);

	return vec2(-1.0);
}

vec3 calcNormal( in vec3 pos )
{
    vec3 eps = vec3(0.02,0.0,0.0);

	return normalize( vec3(
           map(pos+eps.xyy).x - map(pos-eps.xyy).x,
           map(pos+eps.yxy).x - map(pos-eps.yxy).x,
           map(pos+eps.yyx).x - map(pos-eps.yyx).x ) );
}

float softshadow( in vec3 ro, in vec3 rd, float mint, float k )
{
    float res = 1.0;
    float t = mint;
    for( int i=0; i<16; i++ )
    {
        float h = map(ro + rd*t).x;
        res = min( res, k*h/t );
        t += h;
    }
    return clamp(res,0.0,1.0);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 p = -1.0 + 2.0 * fragCoord.xy / iResolution.xy;
    p.x *= iResolution.x/iResolution.y;

    vec2 m = iMouse.xy/iResolution.xy;
	if( iMouse.z<0.0 ) m = vec2(0.0);
	
    // camera
	float an = -6.2*m.x + 0.2*sin(0.5*iTime) + 6.5;
    vec3 ro = 1.5*normalize(vec3(sin(an),-6.0*m.y, cos(an)));

    vec3 ww = normalize( vec3(0.0,0.0,0.0) - ro );
    vec3 uu = normalize( cross(ww,vec3(0.0,1.0,0.0) ) );
    vec3 vv = normalize( cross(uu,ww));
    vec3 rd = normalize( p.x*uu + p.y*vv + 1.0*ww );

    vec3 col = vec3(1.0);

	// raymarch
    vec2 tmat = intersect(ro,rd);
    if( tmat.y>-0.5 )
    {
        // geometry
        vec3 pos = ro + tmat.x*rd;
        vec3 nor = calcNormal(pos);
        vec3 ref = reflect(rd,nor);
		vec3 lig = normalize(vec3(-0.6,0.5,0.2));
		vec2 dis = disp( pos );
     
        // lights
        float con = 1.0;
        float amb = 0.5 + 0.5*nor.y;
        float dif = max(dot(nor,lig),0.0);
        float bac = max(0.2 + 0.8*dot(nor,vec3(-lig.x,lig.y,-lig.z)),0.0);
        float rim = pow(1.0+dot(nor,rd),8.0);
        float spe = pow(clamp(dot(lig,ref),0.0,1.0),8.0);
        float occ = mix( 1.0, 0.9 + 3.0*dis.y, dis.x );

		// shadow
		float sh = softshadow( pos, lig, 0.01, 8.0 );
		dif *= sh;
		spe *= sh;
		rim *= sh;
		
        col  = 0.10*con*vec3(1.0)*occ;
        col += 1.00*dif*vec3(1.0,0.8,0.6);
        col += 0.40*bac*vec3(1.0)*occ;
        col += 0.25*amb*vec3(0.6,0.8,1.0)*occ;

        // material
		col *= mix( vec3(0.7,0.1,0.1), vec3(0.0,0.2,1.0), tmat.y );
		
		// speculars
        col += 0.50*spe*vec3(1.0);
		col += 1.00*rim*vec3(1.0);
			
        // gamma
        col = sqrt(col);
    }


    fragColor = vec4( col,1.0 );
}