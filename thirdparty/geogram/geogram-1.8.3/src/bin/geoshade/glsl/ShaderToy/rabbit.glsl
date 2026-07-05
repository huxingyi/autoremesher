// https://www.shadertoy.com/view/XlccWH

float time;

// polynomial smooth min (k = 0.1) (from IQ)
float smin( float a, float b, float k )
{
    float h = clamp( 0.5+0.5*(b-a)/k, 0.0, 1.0 );
    return mix( b, a, h ) - k*h*(1.0-h);
}


float smax(float a,float b, float k)
{
    return -smin(-a,-b,k);
}

mat2 rotmat(float a)
{
    return mat2(cos(a),sin(a),-sin(a),cos(a));
}

float cylinder(vec3 p,vec3 dir,float h,float r)
{
    float t=dot(p,dir);
    float d=distance(p,dir*t);
    return length(max(abs(vec2(d,t))-vec2(r,h),vec2(0)));
    d=max(d,-t);
    d=max(d,t-h);
    return d;
}

float pupdist=1e4;

float rabdist(vec3 p)
{
    float an=.5*.5*2.*6. +iMouse.x/iResolution.x*6.;
    p.xz=mat2(cos(an),sin(an),sin(an),-cos(an))*p.xz;
    
    float time2=time*2.4;

    p.y+=2.3;
    p.xy*=rotmat(cos(time2+1.)*.04);
    p.y-=2.3;

    vec3 op=p;

    vec3 p2=p;
    p2.xy*=rotmat(cos(time2)*.1);

    vec3 p3=p;
    p3.xy*=rotmat(cos(time2-.0-length(p)/2.)*.13);

    float d=1e4;
    p.x=abs(p.x);
    p2.x=abs(p2.x);
    p3.x=abs(p3.x);

    d=smin(length(p2-vec3(-.75,0.,-.1))-.4,length(p2-vec3(.75,0.,-.1))-.5,2.);
    d=smin(d,length(p2-vec3(0,0.4,-.1))-.9,1.6);
    d+=.1;  
    // ears 1
    d=smin(d,distance(vec3(.7,clamp(p3.y,0.,2.2),0.),p3.xyz)-.4,.14);   
    d=smax(d,-(length(p3-vec3(.7,1.7,-0.5))-.5),.2);

    d=smin(d,distance(vec3(0.,clamp(p.y,-1.6,-1.1),0.),p.xyz)-.6,.04);   
    d=smin(d,distance(vec3(.3,clamp(p.y,-2.6,-1.5),0.),p.xyz)-.3,.1);   
    d=smin(d,distance(vec3(0.,-1.5,-.2),p)-.5+cos(time*3.)*.03,.4);   

    // ears 2
    d=smin(d,distance(vec3(1.1,2.3,-.1),p3)-.2,.8);   

    // tail
    d=smin(d,distance(vec3(0,-1.7,.6),p)-.3,.1);   

    vec3 q=vec3(0.35,.4,-1);

    if(mod(time-1.,4.)>.04)
    {
        d=smax(d,-(cylinder(p2-q,normalize(q-p2),.3,.1)-.0001),.05);
        d=smin(d,(length(p2-q*.9)-.2),.24);

        // eye pupils
        if(op.x>0.)
            pupdist=(length(p2-vec3(.39,.32,-1.))-.2);
        else
            pupdist=(length(p2-vec3(.28,.32,-1.02))-.2);

        d=smin(d,pupdist,.005);
    }

    // nose
    d=smin(d,(length(p2-vec3(0,.1,-1.02))-.2),.02);

    float d3=smax(-(length(p-vec3(-.05,-.29,-1.02))-.1),-(length(p-vec3(.05,-.29,-1.02))-.1),.1);

    float d2=max(p2.z,distance(p2,vec3(clamp(p2.x,0.,.3),-.2,clamp(p2.z,-2.,2.)))+.01);

    float time4=time/8.;
    float gg=smoothstep(0.,1.,clamp((min(fract(time4),1.-fract(time4))-.25)*64.,0.,1.));
    d=smax(d,mix(-d2,d3,gg),.1);

    // tooth
    d=min(d,(length(p-vec3(.0,-.2,-1.02))-.08));

    p.y+=.2;
    p.xy*=rotmat(.4+cos(time2*2.)*.02);

    // arms
    float armd=smin(distance(vec3(.2,clamp(p.y,-1.8,-0.),0.),p.xyz)-.2,
                    distance(p,vec3(0.2,-1.7,0))-.2,.2);

    d=smin(d,armd,.05);   

    return d;
}

float floordist(vec3 p)
{
    return p.y+2.85;
}

float f(vec3 p)
{
    return min(rabdist(p),floordist(p));
}

float sceneDist(vec3 p) { return f(p); }

vec3 sceneNorm(vec3 p)
{
    vec3 e=vec3(1e-2,0,0);
    float d = sceneDist(p);
    return normalize(vec3(sceneDist(p + e.xyy) - d, sceneDist(p + e.yxy) - d,
                          sceneDist(p + e.yyx) - d));
}


// from simon green and others
float ambientOcclusion(vec3 p, vec3 n)
{
    const int steps = 18;
    const float delta = 1.5;

    float a = 0.0;
    float weight = .5;
    for(int i=1; i<=steps; i++) {
        float d = (float(i) / float(steps)) * delta; 
        a += weight*(d - sceneDist(p + n*d));
        weight *= 0.6;
    }
    return clamp(1.0 - a, 0.0, 1.0);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = (fragCoord/iResolution.xy * 2. - 1.) * .75;
    uv.x *= iResolution.x / iResolution.y;
    vec2 vt=uv;
    float an;
    time=iTime;

    vec3 ro=vec3(0.,-.1,-8.+iMouse.y/iResolution.y*2.);
    vec3 rd=normalize(vec3(uv,1.8));


    float s=20.;

    float t=2.,d=0.;
    for(int i=0;i<120;++i)
    {
        d=f(ro+rd*t);
        if(d<1e-4)break;
        if(t>50.)break;
        t+=d;
    }

    float d2=f(ro+rd*t+normalize(vec3(1,2,-2))*5e-2);
    float l=.5+.5*(d2-d)/5e-2;

    vec3 rp=(ro+rd*t);

    vec3 n=sceneNorm(rp);

    vec3 col=vec3(1);

    col*=mix(.1,1.,smoothstep(0.0,.01,pupdist));

    vec3 bcol=vec3(1,.4,.18);

    if(floordist(rp)<rabdist(rp))
        col=max(mix(bcol+(.1-length(vt.xy)/3.),vec3(1),.1),0.);

    col*=l;

    col+=pow(clamp(-n.y,0.,1.),2.)*bcol/1.5;
    if(n.y<.9999)col+=pow(clamp(-rp.y-1.8,0.,1.),4.)*vec3(1,.4,.18)/3.;

    if(n.y>.99999)
    {
        col*=pow(ambientOcclusion(rp,n),1.);
        col*=mix(.7,1.,smoothstep(0.,2.,length(rp.xz)));
    }
    else
    {
     //   vec3 r=reflect(rd,n);
     //   col += texture(iChannel0, r).rgb*.2*pow(clamp(0.,1.,1.-dot(-rd,n)),2.);
     //   col*=pow(ambientOcclusion(rp,n),2.);
    }
        
    fragColor.rgb=max(col,0.);
    fragColor.rgb=sqrt(fragColor.rgb+.01);
}


