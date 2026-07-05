#version 110
/*
 *  Copyright (c) 2016-2020 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:

 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.

 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
uniform sampler2D textureId;
uniform int textureEnabled;
uniform sampler2D normalMapId;
uniform int normalMapEnabled;
uniform sampler2D metalnessRoughnessAoMapId;
uniform int metalnessMapEnabled;
uniform int roughnessMapEnabled;
uniform int aoMapEnabled;
uniform vec3 eyePosition;
uniform int mousePickEnabled;
uniform vec3 mousePickTargetPosition;
uniform float mousePickRadius;
varying vec3 pointPosition;
varying vec3 pointNormal;
varying vec3 pointColor;
varying vec2 pointTexCoord;
varying float pointAlpha;
varying float pointMetalness;
varying float pointRoughness;
varying mat3 pointTBN;

// Defined as a constant for a fixed "Top-Right" light source
// X: 10.0 (Right), Y: 15.0 (High Up), Z: 10.0 (Slightly in front of objects)
const vec3 LIGHT_POS = vec3(10.0, 15.0, 10.0);

// Soft cool gray for shadows
const vec3 SHADOW_TINT = vec3(0.82, 0.81, 0.85);

void main()
{
    vec3 color = pointColor;
    float alpha = pointAlpha;
    if (1 == textureEnabled) {
        vec4 textColor = texture2D(textureId, pointTexCoord);
        color = textColor.rgb;
        alpha = textColor.a;
    }

    if (1 == mousePickEnabled) {
        float distanceWithMouse = distance(mousePickTargetPosition, pointPosition);
        if (distanceWithMouse >= mousePickRadius * 0.94 && distanceWithMouse <= mousePickRadius) {
            color = color + vec3(0.99, 0.4, 0.13);
        }
    }

    vec3 normal = pointNormal;
    if (1 == normalMapEnabled) {
        normal = texture2D(normalMapId, pointTexCoord).rgb;
        normal = pointTBN * normalize(normal * 2.0 - 1.0);
    }

    vec3 lightDir = normalize(LIGHT_POS - pointPosition);

    // Soft "Half-Lambert" Diffuse
    // This maps dot product from [-1, 1] to [0.5, 1.0]
    // This ensures the "dark side" is never pitch black
    float diff = dot(normal, lightDir) * 0.25 + 0.75;

    // --- Hemispherical Component ---
    // Objects are slightly darker on their underside to simulate contact with the floor
    float hemi = smoothstep(-0.2, 1.0, normal.y);
    vec3 ambient = mix(SHADOW_TINT, vec3(1.0), hemi);

    // Combine light contribution
    vec3 lighting = ambient * diff;

    color = color * lighting;

    // Apply a light gamma correction for the "washed out" architectural feel
    gl_FragColor = vec4(pow(color, vec3(1.0 / 1.1)), alpha);
}