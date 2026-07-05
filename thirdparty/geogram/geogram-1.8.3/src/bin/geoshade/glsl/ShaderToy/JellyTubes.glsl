// https://www.shadertoy.com/view/tsBSzc

#define AA 2 // Square root of the number of anti-aliasing samples to make.

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

vec2 intersectCylinder(vec2 ro, vec2 rd, vec2 org, float rad)
{
    return intersectSphere(vec3(ro, 0), vec3(rd, 0), vec3(org, 0), rad);
}

vec3 nn = vec3(0);
float u = 0.;

vec3 tr2(vec3 o, vec3 r, vec2 t)
{
    o += r * (1e-4 + .5 - o.y) / r.y;
    for(int i = 0; i < 38; ++i)
    {
        vec3 c = vec3(floor(o.x), 0, floor(o.z));

        vec3 ofs = vec3(cos(c.z) * .1, 0, cos(c.x) * .1);      

        float rad = .3+cos(c.x) * .1;
        ofs.x = cos(t.y * 8. + c.z * 2.5) * rad / 4.;

        float h = -(cos(c.x + 3. + c.z * 65.) * .5 + .5) * 11.8;

        u = h;

        float ht = (h - o.y) / r.y;
        float ft = (-2.2 - o.y) / r.y;

        vec2 cyl = intersectCylinder(o.xz, r.xz, (c + ofs).xz + .5, rad);
        vec2 sph = intersectSphere(o, r, (c + ofs) + .5 + vec3(0, -.5 + h, 0), rad);

        cyl.x = max(cyl.x, ht);

        if(sph.x < cyl.x && sph.y > 0. && sph.x < sph.y)
        {
            nn = normalize(o + r * sph.x - ((c + ofs) + .5 + vec3(0, -.5 + h, 0)));
            return o + r * sph.x;
        }

        if((cyl.x < sph.y || sph.x >= sph.y) && cyl.y > 0. && cyl.x < cyl.y)
        {
            nn.xz = o.xz + r.xz * cyl.x - ((c + ofs).xz + .5);
            nn.y = 0.;
            nn = normalize(nn);
            return o + r * cyl.x;
        }

        c.xz = ((c.xz + max(sign(r.xz), 0.)) - o.xz) / r.xz;
        float t = (dot(c.xz, step(c.xz, c.zx)) + 1e-4);
        //      if(ft<t){nn=vec3(0,1,0);return o+r*ft;}
        o += t * r;
    }
    return o;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord / iResolution.xy * 2. - 1.;
    uv.y *= iResolution.y / iResolution.x;

    vec3 acc = vec3(0);
    float wsum = 0.;

    float time = iTime;

    for(int y = 0; y < AA; ++y)
        for(int x = 0; x < AA; ++x)
        {
            vec2 t = uv.xy + vec2(x, y) / float(AA) * vec2(2.2) / iResolution.x;

            vec3 o = vec3(1.4 + time / 3., 2.5, 0), r = normalize(vec3(t.xy + vec2(0, -1), -3.5));

            {
                float ang = -3.14159 / 5.;
                r.xz *= mat2(cos(ang), sin(ang), -sin(ang), cos(ang));
            }

            vec3 rp;
            fragColor.rgb = vec3(0);

            rp = tr2(o, r, t);
            u += o.y * 75.;
            u += rp.y / 4.;

            {
                vec3 rd = r;
                vec3 n = normalize(nn);
                vec3 r = reflect(rd, n);
                float fresnel = pow(clamp(1. - dot(n, -rd), 0., 1.), 2.);

                float spec = step(max(abs(3. + r.x * o.y / r.y) - 4., abs(+r.z * o.y / r.y)), 1.4) * step(0., r.y);
                spec += step(max(abs(-6.6 + r.x * o.y / r.y) - 2., abs(+r.z * o.y / r.y)), 1.4) * step(0., r.y) / 3.;

                spec += step(abs(r.x-1.),.4)/7.;

                fragColor.rgb += (vec3(cos(u), cos(u * 2.), cos(u * 3.)) * .5 + .5) * mix(.9, 1., fresnel);

                vec3 rp2 = rp * 10.;
                fragColor.rgb = mix(fragColor.rgb, vec3(1), .1 - .1 * smoothstep(.0, .05, length(rp2 - (floor(rp2) + cos(time + floor(rp2.zxy) * 9.) * .4 + .5)) - .1));

                vec3 rp3 = rp * 3.;
                fragColor.rgb *= mix(.7, 1., smoothstep(.0, .01, length(rp3 - (floor(rp3) + cos(floor(rp3.zxy) * 9.) * .25 + .5)) - .2));

                fragColor.rgb *= rp.y / 10. + 1.5;

                fragColor.rgb += 1.5 * spec * fresnel + fresnel / 15.;
            }

            acc += clamp(fragColor.rgb, 0., 1.);
            wsum += 1.;
        }
    fragColor.rgb = pow(acc / wsum, vec3(1. / 2.2));
}

