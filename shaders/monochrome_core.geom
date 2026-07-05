#version 330 core
layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

in vec3 pointColor[];
in float pointAlpha[];

out vec3 lineColor;
out float lineAlpha;

uniform vec2 viewportSize;
const float thickness = 6.0; // Fixed pixel width
const float zBias = 0.00001; // To make the thickness even, and not z-fight with model

void main() {
    // 1. Get Clip Space positions
    vec4 p1 = gl_in[0].gl_Position;
    vec4 p2 = gl_in[1].gl_Position;

    // 2. Convert to Screen Space (Pixels)
    // We multiply by viewportSize to work in actual pixel units
    vec2 s1 = (p1.xy / p1.w) * viewportSize;
    vec2 s2 = (p2.xy / p2.w) * viewportSize;

    // 3. Calculate 2D line direction and the perpendicular normal
    vec2 dir = s2 - s1;
    vec2 normal;
    
    // Safety check: if the line is a single point (facing the camera)
    if (length(dir) < 0.0001) {
        normal = vec2(1.0, 0.0);
    } else {
        normal = normalize(vec2(-dir.y, dir.x));
    }

    // 4. Offset in pixel space
    // Dividing back by viewportSize converts pixels back to NDC units
    vec2 offset = normal * (thickness * 0.5) / viewportSize;

    // --- Vertex 1 & 2 (Start) ---
    lineColor = pointColor[0];
    lineAlpha = pointAlpha[0];
    gl_Position = vec4((p1.xy / p1.w + offset) * p1.w, p1.z, p1.w); gl_Position.z -= zBias; EmitVertex();
    gl_Position = vec4((p1.xy / p1.w - offset) * p1.w, p1.z, p1.w); gl_Position.z -= zBias; EmitVertex();

    // --- Vertex 3 & 4 (End) ---
    lineColor = pointColor[1];
    lineAlpha = pointAlpha[1];
    gl_Position = vec4((p2.xy / p2.w + offset) * p2.w, p2.z, p2.w); gl_Position.z -= zBias; EmitVertex();
    gl_Position = vec4((p2.xy / p2.w - offset) * p2.w, p2.z, p2.w); gl_Position.z -= zBias; EmitVertex();

    EndPrimitive();
}