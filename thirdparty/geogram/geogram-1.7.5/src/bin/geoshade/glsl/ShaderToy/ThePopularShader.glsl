// https://www.shadertoy.com/view/XdB3Dw
#define USE_IQ_SMIN 0

float time;

vec2 leg0[3];
vec2 leg1[3];

vec2 arm0[3];
vec2 arm1[3];

float wlen=15.0;
float bob;
float wc_scale=0.5;
float scroll;
float scene_scale=15.0;

// Finds the entry and exit points of a 2D ray with a circle of radius 1
// centered at the origin.
vec2 intersectCircle(vec2 ro, vec2 rd)
{
	float a = dot(rd, rd);
	float b = 2.0 * dot(rd, ro);
	float ds = b * b - 4.0 * a * (dot(ro, ro) - 1.0);
	
	if(ds < 0.0)
		return vec2(1e3);
	
	return ((-b - sqrt(ds) * vec2(-1.0, 1.0))) / (2.0 * a);
}

mat3 rotateXMat(float a)
{
	return mat3(1.0, 0.0, 0.0, 0.0, cos(a), -sin(a), 0.0, sin(a), cos(a));
}

mat3 rotateYMat(float a)
{
	return mat3(cos(a), 0.0, -sin(a), 0.0, 1.0, 0.0, sin(a), 0.0, cos(a));
}

// Adapted from https://www.shadertoy.com/view/ldlGR7   
vec2 solve( vec2 p, float l1, float l2, float side )
{
	vec2 q = p*( 0.5 + 0.5*(l1*l1-l2*l2)/dot(p,p) );
	
	float s = l1*l1/dot(q,q) - 1.0;
	
	if( s<0.0 ) return vec2(-100.0);
	
	return q + q.yx*vec2(-1.0,1.0)*side*sqrt( s );
}

// Returns a pyramid-like periodic signal.
float pyramid(float x)
{
	x = fract(x);
	return min(x * 2.0, (1.0 - x) * 2.0);
}

// Returns a semicircular periodic signal.
float circ(float x)
{
	x = fract(x) * 2.0 - 1.0;
	return sqrt(1.0 - x * x);
}

#if USE_IQ_SMIN
float smin(float a,float b,float k){ return -log(exp(-k*a)+exp(-k*b))/k;}//from iq
#else
// http://www.johndcook.com/blog/2010/01/20/how-to-compute-the-soft-maximum/
float smin(in float a, in float b, in float k) { return a - log(1.0+exp(k*(a-b))) / k; }
#endif

float mp(float x)
{
	float y=0.3;
	return clamp((pyramid(x)-0.5)*2.0-0.4,-y,y);
}

float mosaic(vec3 p)
{
	// Disabled because it causes a compilation failure due to time-out or size limit.
	return 0.0;//max(mp(p.y*10.0),mp(p.z*10.0))*0.01;
}
/*
mat3 transpose(mat3 m)
{
	return mat3(vec3(m[0].x,m[1].x,m[2].x),
				vec3(m[0].y,m[1].y,m[2].y),
				vec3(m[0].z,m[1].z,m[2].z));
}*/

float capsuleDist(vec3 p,vec3 o,vec3 d,float h0,float h1,float r0,float r1)
{
	vec3 u=cross(d,vec3(1.0,0.0,0.0));
	vec3 v=cross(u,d);
	u=cross(v,d);
	mat3 m=transpose(mat3(normalize(u),normalize(v),normalize(d)));
	d=normalize(d);
	float t=clamp(dot(p-o,d),h0,h1);
	vec3 np=o+t*d;
	return distance(np,p)-mix(r0,r1,t)+mosaic(m*(p-o));
}

float boxDist(vec3 p,vec3 s,float r)
{
	return length(max(vec3(0.0),abs(p)-s))-r+mosaic(p);
}

float sphereDist(vec3 p,vec3 o,float r)
{
	return distance(p,o)-r+mosaic(p-o);
}

float sceneDist(vec3 p)
{
	float d=1e3;
	
	p+=vec3(0.0,0.07,0.0)*scene_scale;
	p=rotateYMat(3.1415926*0.5)*p;
	
	p.z+=cos(p.y*2.0+time)*0.1;
	float tm=fract(time*wc_scale*2.0-0.1);
	p.x-=(smoothstep(0.0,0.3,tm)-smoothstep(0.4,1.0,tm))*smoothstep(0.5,2.0,p.y)*0.2+scroll;
	
	// Leg 0
	{
		float g=0.08;
		vec3 o=vec3(0.0,0.0,0.2);
		float d0=capsuleDist(p+o,vec3(leg0[0],0.0),vec3(leg0[1]-leg0[0],0.0),0.0,1.0-g,0.1,0.1);
		float d1=capsuleDist(p+o,vec3(leg0[1],0.0),vec3(leg0[2]-leg0[1],0.0),g,1.0,0.1,0.2);
		d=min(d,smin(d0,d1,15.0));
	}
	
	// Leg 1
	{
		float g=0.08;
		vec3 o=vec3(0.0,0.0,-0.2);
		float d0=capsuleDist(p+o,vec3(leg1[0],0.0),vec3(leg1[1]-leg1[0],0.0),0.0,1.0-g,0.1,0.1);
		float d1=capsuleDist(p+o,vec3(leg1[1],0.0),vec3(leg1[2]-leg1[1],0.0),g,1.0,0.1,0.2);
		d=min(d,smin(d0,d1,15.0));
	}
	
	p.y-=bob;
	
	// Arm 0
	{
		float g=0.08;
		vec3 o=vec3(0.0,0.0,0.4);
		mat3 m=rotateXMat(-0.3)*rotateYMat((cos((time*wc_scale+0.5)*3.1415926*2.0)-0.6)*0.5);
		float d0=capsuleDist(p+o,vec3(arm0[0],0.0),m*vec3(arm0[1]-arm0[0],0.0),0.0,0.7-g,0.03,0.03);
		float d1=capsuleDist(p+o,vec3(arm0[0],0.0)+m*vec3(arm0[1]-arm0[0],0.0),m*vec3(arm0[2]-arm0[1],0.0),g,0.7,0.03,0.06);
		d=min(d,smin(d0,d1,15.0));
	}
	
	// Arm 1
	{
		float g=0.08;
		vec3 o=vec3(0.0,0.0,-0.4);
		mat3 m=rotateXMat(0.3)*rotateYMat(-(cos(time*wc_scale*3.1415926*2.0)-0.6)*0.5);
		float d0=capsuleDist(p+o,vec3(arm1[0],0.0),m*vec3(arm1[1]-arm1[0],0.0),0.0,0.7-g,0.03,0.03);
		float d1=capsuleDist(p+o,vec3(arm1[0],0.0)+m*vec3(arm1[1]-arm1[0],0.0),m*vec3(arm1[2]-arm1[1],0.0),g,0.7,0.03,0.06);
		d=min(d,smin(d0,d1,15.0));
	}
	
	// Torso   
	d=smin(d,boxDist(p+vec3(0.0,-0.7,0.0),vec3(0.05,0.7,0.15),0.1),15.0);
	d=smin(d,boxDist(p+vec3(-0.1,-1.1,0.0),vec3(0.05,0.2,0.15)*0.1,0.1),5.0);
	
	// Head
	d=smin(d,sphereDist(p,vec3(0.0,1.825,0.0),0.2),15.0);
	
	
	return d;
}

vec3 sceneNorm(vec3 p)
{
	p*=scene_scale;
	float c=sceneDist(p);
	float e=1e-3;
	return normalize(vec3(sceneDist(p+vec3(e,0,0))-c,
						  sceneDist(p+vec3(0,e,0))-c,
						  sceneDist(p+vec3(0,0,e))-c));
}

float robot(vec3 ro,vec3 rd)
{
	float t=0.0;
	float tm;
	
	tm=time*wc_scale;
	
	leg0[0]=vec2(0.0,bob);
	leg0[2]=vec2(pyramid(tm)-0.3,-1.8+0.3*circ(tm*2.0)*step(fract(tm),0.5));
	leg0[1]=(leg0[0]+solve(leg0[2]-leg0[0],1.0,1.0,1.0));
	
	arm1[0]=vec2(0.0,1.4);
	arm1[2]=vec2(pyramid(tm)-0.3,0.1+pow(pyramid(tm),2.0)*0.7);
	arm1[1]=(arm1[0]+solve(arm1[2]-arm1[0],0.7,0.7,-1.0));
	
	tm+=0.5;
	
	leg1[0]=vec2(0.0,bob);
	leg1[2]=vec2(pyramid(tm)-0.3,-1.8+0.3*circ(tm*2.0)*step(fract(tm),0.5));
	leg1[1]=(leg1[0]+solve(leg1[2]-leg1[0],1.0,1.0,1.0));
	
	arm0[0]=vec2(0.0,1.4);
	arm0[2]=vec2(pyramid(tm)-0.3,0.1+pow(pyramid(tm),2.0)*0.7);
	arm0[1]=(arm0[0]+solve(arm0[2]-arm0[0],0.7,0.7,-1.0));
	
	float rt=1e4;
	
	ro*=scene_scale;
	rd*=scene_scale;
	
	for(int i=0;i<15;i+=1)
	{
		vec3 rp=ro+rd*t;
		
		float d=sceneDist(rp);
		
		if(d<1e-2)
		{
			rt=t;
		}
		
		t+=d/scene_scale;
	}
	
	
	return rt;
}


vec2 unitSquareInterval(vec2 ro, vec2 rd)
{	
	vec2 slabs0 = (vec2(+1.0) - ro) / rd;	
	vec2 slabs1 = (vec2(-1.0) - ro) / rd;
	
	vec2 mins = min(slabs0, slabs1);
	vec2 maxs = max(slabs0, slabs1);
	
	return vec2(max(mins.x, mins.y),
				min(maxs.x, maxs.y));
}

vec3 squaresColours(vec2 p)
{
	p+=vec2(time*0.2);
	
	vec3 orange=vec3(1.0,0.4,0.1)*2.0;
	vec3 purple=vec3(1.0,0.2,0.5)*0.8;
	
	float l=pow(0.5+0.5*cos(p.x*7.0+cos(p.y)*8.0)*sin(p.y*2.0),4.0)*2.0;
	vec3 c=pow(l*(mix(orange,purple,0.5+0.5*cos(p.x*40.0+sin(p.y*10.0)*3.0))+
				  mix(orange,purple,0.5+0.5*cos(p.x*20.0+sin(p.y*3.0)*3.0))),vec3(1.2))*0.7;
	
	c+=vec3(1.0,0.8,0.4)*pow(0.5+0.5*cos(p.x*20.0)*sin(p.y*12.0),20.0)*2.0;
	
	c+=vec3(0.1,0.5+0.5*cos(p*20.0))*vec3(0.05,0.1,0.4).bgr*0.7;
	
	return c;
}

vec3 squaresTex(vec2 p,float border)
{
	float sm=0.02;
	vec2 res=vec2(8.0);
	vec2 ip=floor(p*res)/res;
	vec2 fp=fract(p*res);
	float m=1.0-max(smoothstep(border-sm,border,abs(fp.x-0.5)),smoothstep(border-sm,border,abs(fp.y-0.5)));
	m+=1.0-smoothstep(0.0,0.56,distance(fp,vec2(0.5)));
	return m*squaresColours(ip);
}

vec3 room(vec3 ro,vec3 rd,out vec3 rp,out vec3 n)
{
	vec2 box_size=vec2(1.0,5.0+3.0/8.0);
	
	vec2 cp=vec2(0.0),ct=vec2(1e3);
	
	for(int i=0;i<4;i+=1)
	{
		float cr=0.03;
		vec2 tcp=vec2(2.5/8.0*float(-1),float(i)-2.0+0.5/8.0);
		vec2 tct=intersectCircle((ro.xz-tcp)/cr,rd.xz/cr);
		
		if(tct.y > 0.0 && tct.y<ct.y)
		{
			ct=tct;
			cp=tcp;
		}
	}

	for(int i=0;i<4;i+=1)
	{
		float cr=0.03;
		vec2 tcp=vec2(2.5/8.0*float(+1),float(i)-2.0+0.5/8.0);
		vec2 tct=intersectCircle((ro.xz-tcp)/cr,rd.xz/cr);
		
		if(tct.y > 0.0 && tct.y<ct.y)
		{
			ct=tct;
			cp=tcp;
		}
	}
	
	ct.y=max(0.0,ct.y);
	
	vec3 ci=ro+rd*ct.y;
	vec2 cu=vec2(atan(ci.z-cp.y,ci.x-cp.x)/3.1415926*0.5,(ci.y+0.5/8.0)*4.0);
	
	float wt=max(0.0,unitSquareInterval(ro.xy * box_size, rd.xy * box_size).y);
	float t=min(ct.y,wt);
	
	rp=ro+rd*(t-1e-4);
	
	n.z=0.0;
	if(abs(rp.x*box_size.x)>abs(rp.y*box_size.y))
		n.xy=vec2(rp.x/abs(rp.x),0.0);
	else
		n.xy=vec2(0.0,rp.y/abs(rp.y));
	
	if(ct.y<wt)
	{
		n.y=0.0;
		n.xz=normalize(rp.xz-ci.xz);
	}
	
	float l=1.0-smoothstep(0.0,3.0,abs(rp.z-ro.z));
	
	vec3 wc=mix(squaresTex(rp.zy+vec2(0.0,0.5/8.0),0.5),squaresTex(rp.xz,0.44),step(0.999/box_size.y,abs(rp.y)));
	vec3 cc=squaresTex(cu,0.45)+0.8*vec3(smoothstep(0.83/5.0,0.86/5.0,abs(rp.y)));
	
	return l*mix(cc,wc,step(wt,t));
}

vec3 scene(vec2 p)
{
	mat3 cam = rotateXMat(cos(time * 0.2) * 0.1) * rotateYMat(time * 0.5);
	float lt=mod(time*wc_scale,wlen)/wlen;
	
	vec3 ro = cam*vec3(0.0,-0.15+lt*0.15, 0.15+lt*0.2)+vec3(0.0,0.0,scroll/scene_scale);
	vec3 rd = cam*vec3(p, -1.0);
	
	rd=normalize(rd);
	
	float robot_t=robot(ro,rd);
	
	vec3 n,rp;
	
	vec3 c;
	vec3 c0=room(ro,rd,rp,n);
	
	if(robot_t < distance(ro,rp))
	{
		rp=ro+rd*robot_t;
		n=sceneNorm(rp);
		vec3 r=reflect(rd,n);
		c=vec3(0.5+0.5*n.y)*0.5*vec3(1.0,0.8,0.5);
		vec3 c1=room(rp,r,rp,n);
		c+=c1*0.5;
	}
	else
	{
		vec3 r=reflect(rd,n);
		vec3 c1=room(rp,r,rp,n);
		c=c0+c1*c0*0.4;
	}
		
	vec3 ll=vec3(1.0-(smoothstep(0.0,0.07,lt)-smoothstep(0.93,1.0,lt)));
	
	return ll+c+
		0.6*((sin(p.y)*cos(p.x+time*2.0)*0.5+0.5)*
			 pow(mix(vec3(1.0,0.7,0.1),vec3(1.0,0.2,0.6),0.5+0.5*cos(p.x+sin(time*3.0+p.y*2.0))),vec3(2.0)));
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	time=iTime+1.0;
	bob=cos(time*12.0)*0.05;
	scroll=-15.0+mod(time*wc_scale,wlen)*2.0;
	vec2 uv = fragCoord.xy / iResolution.xy;
	vec2 q=uv;
	vec2 t=uv*2.0-vec2(1.0);
	t.x*=iResolution.x/iResolution.y;
	fragColor.rgb = scene(t.xy) * 1.3;
	
	// vignet
	fragColor.rgb *= 0.5 + 0.5*pow( 16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.1 );
}


