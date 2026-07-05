#version 330
in vec3 lineColor; 
in float lineAlpha;

out vec4 fragColor;

void main()
{
    fragColor = vec4(lineColor, lineAlpha);
}