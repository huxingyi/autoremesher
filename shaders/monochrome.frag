#version 110
varying vec3 pointPosition;
varying vec3 pointColor;
varying float pointAlpha;

void main()
{
    gl_FragColor = vec4(pointColor, pointAlpha);
}
