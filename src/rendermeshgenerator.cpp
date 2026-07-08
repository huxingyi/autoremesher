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
#include "rendermeshgenerator.h"
#include <AutoRemesher/AutoRemesher>
#include <cmath>
#include <limits>

void RenderMeshGenerator::process()
{
    generate();

    emit finished();
}

void RenderMeshGenerator::calculateNormalizedFactors(const std::vector<AutoRemesher::Vector3>& vertices, AutoRemesher::Vector3* origin, double* maxLength)
{
    if (vertices.empty()) {
        *origin = AutoRemesher::Vector3();
        *maxLength = 1.0;
        return;
    }

    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();
    double minZ = std::numeric_limits<double>::max();
    double maxZ = std::numeric_limits<double>::lowest();
    for (const auto& v : vertices) {
        if (v.x() < minX)
            minX = v.x();
        if (v.x() > maxX)
            maxX = v.x();
        if (v.y() < minY)
            minY = v.y();
        if (v.y() > maxY)
            maxY = v.y();
        if (v.z() < minZ)
            minZ = v.z();
        if (v.z() > maxZ)
            maxZ = v.z();
    }
    AutoRemesher::Vector3 length = {
        (maxX - minX) * 0.5,
        (maxY - minY) * 0.5,
        (maxZ - minZ) * 0.5,
    };
    *maxLength = length[0];
    if (length[1] > *maxLength)
        *maxLength = length[1];
    if (length[2] > *maxLength)
        *maxLength = length[2];
    if (*maxLength <= 0.0 || !std::isfinite(*maxLength))
        *maxLength = 1.0;
    *origin = {
        (maxX + minX) * 0.5,
        (maxY + minY) * 0.5,
        (maxZ + minZ) * 0.5,
    };
}

void RenderMeshGenerator::normalizeVertices()
{
    AutoRemesher::Vector3 origin;
    double maxLength = 1.0;
    calculateNormalizedFactors(*m_vertices, &origin, &maxLength);
    for (auto& v : *m_vertices) {
        v = (v - origin) / maxLength;
    }
}

void RenderMeshGenerator::generate()
{
    normalizeVertices();

    auto isRenderableFace = [&](const std::vector<size_t>& face) {
        if (face.size() < 3)
            return false;
        for (const auto& vertexIndex : face) {
            if (vertexIndex >= m_vertices->size())
                return false;
        }
        return true;
    };

    std::vector<AutoRemesher::Vector3> vertexNormals(m_vertices->size());
    std::vector<AutoRemesher::Vector3> faceCenters(m_faces->size());
    std::vector<AutoRemesher::Vector3> faceNormals(m_faces->size());
    for (size_t i = 0; i < m_faces->size(); ++i) {
        const auto& sourceFace = (*m_faces)[i];
        if (!isRenderableFace(sourceFace))
            continue;

        AutoRemesher::Vector3 center;
        for (const auto& it : sourceFace) {
            center += (*m_vertices)[it];
        }
        center /= sourceFace.size();
        faceCenters[i] = center;

        AutoRemesher::Vector3 normal;
        for (size_t corner = 0; corner < sourceFace.size(); ++corner) {
            size_t nextCorner = (corner + 1) % sourceFace.size();
            normal += AutoRemesher::Vector3::normal(center,
                (*m_vertices)[sourceFace[corner]],
                (*m_vertices)[sourceFace[nextCorner]]);
        }
        normal.normalize();

        faceNormals[i] = normal;

        for (size_t j = 0; j < sourceFace.size(); ++j)
            vertexNormals[sourceFace[j]] += normal;
    }
    for (auto& it : vertexNormals)
        it.normalize();

    size_t triangleVertexCount = 0;
    size_t edgeVertexCountSize = 0;
    for (size_t i = 0; i < m_faces->size(); ++i) {
        const auto& sourceFace = (*m_faces)[i];
        if (!isRenderableFace(sourceFace))
            continue;
        if (3 == sourceFace.size())
            triangleVertexCount += 3;
        else
            triangleVertexCount += sourceFace.size() * 3;
        edgeVertexCountSize += sourceFace.size() * 2;
    }
    if (triangleVertexCount > static_cast<size_t>(std::numeric_limits<int>::max()) || edgeVertexCountSize > static_cast<size_t>(std::numeric_limits<int>::max())) {
        delete m_renderMesh;
        m_renderMesh = new ModelShaderMesh(nullptr, 0, nullptr, 0, m_vertices, m_faces);
        m_vertices = nullptr;
        m_faces = nullptr;
        return;
    }
    int vertexNum = static_cast<int>(triangleVertexCount);
    int edgeVertexCount = static_cast<int>(edgeVertexCountSize);
    ModelShaderVertex* triangleVertices = vertexNum > 0 ? new ModelShaderVertex[vertexNum] : nullptr;
    ModelShaderVertex* edgeVertices = edgeVertexCount > 0 ? new ModelShaderVertex[edgeVertexCount] : nullptr;

    if (triangleVertices != nullptr)
        memset(triangleVertices, 0, sizeof(ModelShaderVertex) * vertexNum);
    if (edgeVertices != nullptr)
        memset(edgeVertices, 0, sizeof(ModelShaderVertex) * edgeVertexCount);

    vertexNum = 0;
    edgeVertexCount = 0;

    auto addTriangle = [&](const std::vector<size_t>& sourceFace,
                           size_t startIndex,
                           const AutoRemesher::Vector3& normal,
                           const AutoRemesher::Vector3& center) {
        for (size_t j = 0; j < 2; ++j) {
            auto& v = triangleVertices[vertexNum++];
            auto index = sourceFace[(startIndex + j) % sourceFace.size()];
            const auto& src = (*m_vertices)[index];
            v.posX = (float)src.x();
            v.posY = (float)src.y();
            v.posZ = (float)src.z();
            v.normX = (float)normal.x();
            v.normY = (float)normal.y();
            v.normZ = (float)normal.z();
            v.colorR = 1.0f;
            v.colorG = 0.996f;
            v.colorB = 0.890f;
            v.roughness = 1.0f;
            v.alpha = 1.0f;
        }

        auto& v = triangleVertices[vertexNum++];
        v.posX = (float)center.x();
        v.posY = (float)center.y();
        v.posZ = (float)center.z();
        v.normX = (float)normal.x();
        v.normY = (float)normal.y();
        v.normZ = (float)normal.z();
        v.colorR = 1.0f;
        v.colorG = 0.996f;
        v.colorB = 0.890f;
        v.roughness = 1.0f;
        v.alpha = 1.0f;
    };

    auto addEdge = [&](const std::vector<size_t>& sourceFace, size_t startIndex) {
        for (size_t j = 0; j < 2; ++j) {
            auto& v = edgeVertices[edgeVertexCount++];
            auto index = sourceFace[(startIndex + j) % sourceFace.size()];
            const auto& src = (*m_vertices)[index];
            const auto& normal = vertexNormals[index];
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
        const auto& sourceFace = (*m_faces)[i];
        if (!isRenderableFace(sourceFace))
            continue;
        if (sourceFace.size() == 3) {
            for (size_t j = 0; j < sourceFace.size(); ++j) {
                addEdge(sourceFace, j);
            }
            addTriangle(sourceFace, 0, faceNormals[i], (*m_vertices)[sourceFace[2]]);
            continue;
        }
        for (size_t j = 0; j < sourceFace.size(); ++j) {
            addEdge(sourceFace, j);
            addTriangle(sourceFace, j, faceNormals[i], faceCenters[i]);
        }
    }

    delete m_renderMesh;
    m_renderMesh = new ModelShaderMesh(triangleVertices, vertexNum, edgeVertices, edgeVertexCount,
        m_vertices, m_faces);
    m_vertices = nullptr;
    m_faces = nullptr;
}
