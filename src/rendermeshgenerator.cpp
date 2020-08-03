/*
 *  Copyright (c) 2020 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved. 
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
#include <AutoRemesher/AutoRemesher>
#include "rendermeshgenerator.h"

void RenderMeshGenerator::process()
{
    generate();
    
    emit finished();
}

void RenderMeshGenerator::normalizeVertices()
{
    AutoRemesher::Vector3 origin;
    double maxLength = 1.0;
    AutoRemesher::AutoRemesher::calculateNormalizedFactors(*m_vertices, &origin, &maxLength);
    for (auto &v: *m_vertices) {
        v = (v - origin) / maxLength;
    }
}

void RenderMeshGenerator::generate()
{
    normalizeVertices();
    
    std::vector<AutoRemesher::Vector3> vertexNormals(m_vertices->size());
    for (size_t i = 0; i < m_faces->size(); ++i) {
        const auto &sourceFace = (*m_faces)[i];
        auto normal = AutoRemesher::Vector3::normal((*m_vertices)[sourceFace[0]],
            (*m_vertices)[sourceFace[1]], 
            (*m_vertices)[sourceFace[2]]);
        if (sourceFace.size() == 4) {
            normal += AutoRemesher::Vector3::normal((*m_vertices)[sourceFace[2]],
                (*m_vertices)[sourceFace[3]], 
                (*m_vertices)[sourceFace[0]]);
            normal.normalize();
        }
        for (size_t j = 0; j < sourceFace.size(); ++j)
            vertexNormals[sourceFace[j]] += normal;
    }
    for (auto &it: vertexNormals)
        it.normalize();
    
    int vertexNum = 0;
    int edgeVertexCount = 0;
    for (size_t i = 0; i < m_faces->size(); ++i) {
        const auto &sourceFace = (*m_faces)[i];
        if (sourceFace.size() == 4) {
            vertexNum += 6;
            edgeVertexCount += 12;
        } else if (sourceFace.size() == 3) {
            vertexNum += 3;
            edgeVertexCount += 6;
        }
    }
    PbrShaderVertex *triangleVertices = new PbrShaderVertex[vertexNum];
    PbrShaderVertex *edgeVertices = new PbrShaderVertex[edgeVertexCount];
    
    memset(triangleVertices, 0, sizeof(PbrShaderVertex) * vertexNum);
    memset(edgeVertices, 0, sizeof(PbrShaderVertex) * edgeVertexCount);
    
    vertexNum = 0;
    edgeVertexCount = 0;
    
    auto addTriangle = [&](const std::vector<size_t> &sourceFace, size_t startIndex) {
        for (size_t j = 0; j < 3; ++j) {
            auto &v = triangleVertices[vertexNum++];
            auto index = sourceFace[(startIndex + j) % sourceFace.size()];
            const auto &src = (*m_vertices)[index];
            const auto &normal = vertexNormals[index];
            v.posX = (float)src.x();
            v.posY = (float)src.y();
            v.posZ = (float)src.z();
            v.normX = (float)normal.x();
            v.normY = (float)normal.y();
            v.normZ = (float)normal.z();
            v.colorR = 1.0f;
            v.colorG = 1.0f;
            v.colorB = 1.0f;
            v.roughness = 1.0f;
            v.alpha = 1.0f;
        }
    };
    
    auto addEdge = [&](const std::vector<size_t> &sourceFace, size_t startIndex) {
        for (size_t j = 0; j < 2; ++j) {
            auto &v = edgeVertices[edgeVertexCount++];
            auto index = sourceFace[(startIndex + j) % sourceFace.size()];
            const auto &src = (*m_vertices)[index];
            const auto &normal = vertexNormals[index];
            v.posX = (float)src.x();
            v.posY = (float)src.y();
            v.posZ = (float)src.z();
            v.normX = (float)normal.x();
            v.normY = (float)normal.y();
            v.normZ = (float)normal.z();
            v.colorR = 0.0f;
            v.colorG = 0.0f;
            v.colorB = 0.0f;
            v.roughness = 1.0f;
            v.alpha = 1.0f;
        }
    };
    
    for (size_t i = 0; i < m_faces->size(); ++i) {
        const auto &sourceFace = (*m_faces)[i];
        if (sourceFace.size() == 4) {
            addTriangle(sourceFace, 0);
            addEdge(sourceFace, 0);
            addEdge(sourceFace, 1);

            addTriangle(sourceFace, 2);
            addEdge(sourceFace, 2);
            addEdge(sourceFace, 3);
        } else if (sourceFace.size() == 3) {
            addTriangle(sourceFace, 0);
            addEdge(sourceFace, 0);
            addEdge(sourceFace, 1);
            addEdge(sourceFace, 2);
        }
    }
    
    delete m_renderMesh;
    m_renderMesh = new PbrShaderMesh(triangleVertices, vertexNum, edgeVertices, edgeVertexCount,
        m_vertices, m_faces);
    m_vertices = nullptr;
    m_faces = nullptr;
}
