#version 330
layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 color;
layout(location = 2) in float alpha;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
out vec3 pointPosition;
out vec3 pointColor;
out float pointAlpha;

void main()
{
    pointPosition = (modelMatrix * vertex).xyz;
    pointColor = color;
    pointAlpha = alpha;

    gl_Position = projectionMatrix * viewMatrix * vec4(pointPosition, 1.0);
}