#version 110
attribute vec4 vertex;
attribute vec3 color;
attribute float alpha;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
varying vec3 pointPosition;
varying vec3 pointColor;
varying float pointAlpha;

void main()
{
    pointPosition = (modelMatrix * vertex).xyz;
    pointColor = color;
    pointAlpha = alpha;

    gl_Position = projectionMatrix * viewMatrix * vec4(pointPosition, 1.0);
}
