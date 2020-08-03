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
attribute vec4 vertex;
attribute vec3 normal;
attribute vec3 color;
attribute vec2 texCoord;
attribute float metalness;
attribute float roughness;
attribute vec3 tangent;
attribute float alpha;
varying vec3 vert;
varying vec3 vertRaw;
varying vec3 vertNormal;
varying vec3 vertColor;
varying vec2 vertTexCoord;
varying float vertMetalness;
varying float vertRoughness;
varying vec3 cameraPos;
varying vec3 firstLightPos;
varying vec3 secondLightPos;
varying vec3 thirdLightPos;
varying float vertAlpha;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform mat4 viewMatrix;
uniform int normalMapEnabled;
uniform vec3 eyePos;

mat3 transpose(mat3 m) 
{
    return mat3(m[0][0], m[1][0], m[2][0],
                m[0][1], m[1][1], m[2][1],
                m[0][2], m[1][2], m[2][2]);
}

void main()
{
    vert = (modelMatrix * vertex).xyz;
    vertRaw = vert;
    vertNormal = normalize((modelMatrix * vec4(normal, 1.0)).xyz);
    vertColor = color;
    vertAlpha = alpha;
    cameraPos = eyePos;

    firstLightPos = vec3(5.0, 5.0, 5.0);
    secondLightPos = vec3(-5.0, 5.0, 5.0);
    thirdLightPos = vec3(0.0, -5.0, -5.0);

    gl_Position = projectionMatrix * viewMatrix * vec4(vert, 1.0);

    if (normalMapEnabled == 1) {
        vec3 T = normalize(normalMatrix * tangent);
        vec3 N = normalize(normalMatrix * normal);
        T = normalize(T - dot(T, N) * N);
        vec3 B = cross(N, T);
        
        mat3 TBN = transpose(mat3(T, B, N));
        firstLightPos = TBN * firstLightPos;
        secondLightPos = TBN * secondLightPos;
        thirdLightPos = TBN * thirdLightPos;
        cameraPos = TBN * cameraPos;
        vert  = TBN * vert;
    }

    vertTexCoord = texCoord;
    vertMetalness = metalness;
    vertRoughness = roughness;
}