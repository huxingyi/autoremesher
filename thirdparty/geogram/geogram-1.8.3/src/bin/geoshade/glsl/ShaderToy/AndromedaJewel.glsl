// https://www.shadertoy.com/view/4sVfWR

#define pi 3.14159265359
//this noise stuff is from iq thanks
float hash (vec2 v) {
    v = floor(v);
    return fract(67.3249*sin(17.1234*length(v-vec2(34.14,123.))));
}
float noise (vec2 v) {
    vec4 n = vec4(floor(v),ceil(v));
    vec4 h = vec4(hash(n.xy),hash(n.zy),hash(n.xw),hash(n.zw));
    return mix(mix(h.x,h.y,v.x-n.x),mix(h.z,h.w,v.x-n.x),v.y-n.y);
}
mat2 r (float a) {
    float s = sin(a), c = cos(a);
    return mat2(s,-c,c,s);
}
float no (vec2 v) {
    float c = 0.;
    for (int i = 1; i < 10; i++) {
        v = 2.*r(0.2944*pi)*v;
        c += 0.2*noise(v)/(1.+length(sin(0.5*v)));
    }
    return c;
}
// flow stuff is from trirop https://www.shadertoy.com/view/MsScWD very cool
vec2 ff (vec2 v) {
    	float n = 1.+0.5*noise(v);
		return 
	        sign(v.y-0.5)*50.*vec2(n*sin(1.4*v.y),0.)/(v.y+3.5)+
	        .1*vec2(sin(-12.*v.y*n),cos(13.*v.x))+
	        1.8*vec2(cos(-6.*v.y),sin(4.*v.x))+
	        1.2*vec2(sin(-1.4*v.y),cos(1.5*v.x))+
	        2.0*vec2(sin(-.5*v.y),cos(.6*v.x))+
	        0.8*vec2(sin(-.2*v.y),cos(.2*v.x*n))
	     ;}
bool star = false;
vec2 mouse;
void sphere (inout vec3 p, inout vec3 d) {
	float r = .7, dp = dot(d,p), pp = dot(p,p), det = dp*dp+r*r-pp;
    if (det < 0.) star = true;
    float x = -dp+sqrt(det);
    p = (p+d*x);
    d = reflect(normalize(p),d);
}

vec3 surface (vec2 uv) {
	vec3 col = 0.*vec3(7.-abs(uv.y))*no (uv);
    for (int i = 0; i < 45; i++) {
		uv += 0.01*(2.+1.5*sin(.1*iTime))*ff(uv);
    }
    float j = no(0.1*uv*pi);
    vec3 c = sin(j*vec3(1,2,3));
    col += abs(mix(c*c*c,vec3(j),abs(1.-uv.y/7.5)));
    return col;
}
vec3 stars (vec2 v) {
	return vec3(pow(1.35*no(0.1*iTime+5.*mouse+3.*v/dot(v,v)),7.));
}
void mainImage( out vec4 fragColor, in vec2 U )
{
    vec2 uv = (2.*U-iResolution.xy)/iResolution.y;
   	float q = 1.2+0.1*min(17.,iTime);
    vec3 p = vec3(0,0,-q);
    vec3 d = normalize(vec3(uv,4.));
    mouse = iMouse.xy/iResolution.xy;
    p.yz = r(-mouse.y+0.7*pi)*p.yz;
    d.yz = r(-mouse.y+0.7*pi)*d.yz;
    p.zx = r(mouse.x+0.25*pi)*p.zx;
    d.zx = r(mouse.x+0.25*pi)*d.zx;
    vec3 col;
    sphere(p,d);
    if (star) {
        col = stars(uv);
    } else {
    	col = 0.8*surface(8.*vec2(atan(d.z,d.x)+0.01*iTime*min(iTime,17.),acos(d.y)));
        float sh = dot(d,normalize(vec3(1,0,-1)));
        col *= sh+0.4;
    }
    float l = length(uv-vec2(0.2,0))*q;
    col = col+.09*vec3(0.5,0.7,1.)*(uv.x+0.5)*exp(-0.01*l*l*l*l)*q;
    fragColor = vec4(col,1);  
}