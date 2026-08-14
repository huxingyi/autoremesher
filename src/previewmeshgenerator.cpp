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
#include "previewmeshgenerator.h"
#include <QImage>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

void PreviewMeshGenerator::process()
{
    generate();

    emit finished();
}

static ModelShaderMesh* buildRenderMeshFromTriangles(
    const std::vector<AutoRemesher::Vector3>& vertices,
    const std::vector<std::vector<size_t>>& triangles)
{
    if (vertices.empty() || triangles.empty())
        return new ModelShaderMesh;

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
    AutoRemesher::Vector3 origin = {
        (maxX + minX) * 0.5, (maxY + minY) * 0.5, (maxZ + minZ) * 0.5
    };
    double maxLength = std::max({ maxX - minX, maxY - minY, maxZ - minZ }) * 0.5;
    if (maxLength < 1e-10)
        maxLength = 1.0;

    std::vector<AutoRemesher::Vector3> normalizedVerts(vertices.size());
    for (size_t i = 0; i < vertices.size(); ++i)
        normalizedVerts[i] = (vertices[i] - origin) / maxLength;

    std::vector<AutoRemesher::Vector3> normals(vertices.size());
    for (const auto& tri : triangles) {
        AutoRemesher::Vector3 n = AutoRemesher::Vector3::normal(
            normalizedVerts[tri[0]], normalizedVerts[tri[1]], normalizedVerts[tri[2]]);
        normals[tri[0]] += n;
        normals[tri[1]] += n;
        normals[tri[2]] += n;
    }
    for (auto& n : normals)
        n.normalize();

    int vertexCount = (int)triangles.size() * 3;
    int edgeVertexCount = (int)triangles.size() * 6;
    ModelShaderVertex* vertData = new ModelShaderVertex[vertexCount];
    ModelShaderVertex* edgeData = new ModelShaderVertex[edgeVertexCount];
    memset(vertData, 0, sizeof(ModelShaderVertex) * vertexCount);
    memset(edgeData, 0, sizeof(ModelShaderVertex) * edgeVertexCount);

    int vi = 0;
    int ei = 0;
    for (const auto& tri : triangles) {
        for (int j = 0; j < 3; ++j) {
            for (int e = 0; e < 2; ++e) {
                auto& ev = edgeData[ei++];
                int idx = (int)tri[(j + e) % 3];
                ev.posX = (float)normalizedVerts[idx].x();
                ev.posY = (float)normalizedVerts[idx].y();
                ev.posZ = (float)normalizedVerts[idx].z();
                ev.normX = (float)normals[idx].x();
                ev.normY = (float)normals[idx].y();
                ev.normZ = (float)normals[idx].z();
                ev.colorR = 0.0f;
                ev.colorG = 0.0f;
                ev.colorB = 0.0f;
                ev.roughness = 1.0f;
                ev.alpha = 1.0f;
            }

            auto& tv = vertData[vi++];
            int idx = (int)tri[j];
            tv.posX = (float)normalizedVerts[idx].x();
            tv.posY = (float)normalizedVerts[idx].y();
            tv.posZ = (float)normalizedVerts[idx].z();
            tv.normX = (float)normals[idx].x();
            tv.normY = (float)normals[idx].y();
            tv.normZ = (float)normals[idx].z();
            tv.colorR = 1.0f;
            tv.colorG = 0.996f;
            tv.colorB = 0.890f;
            tv.roughness = 1.0f;
            tv.alpha = 1.0f;
        }
    }

    return new ModelShaderMesh(vertData, vertexCount, edgeData, ei, &normalizedVerts, &triangles);
}

static ModelShaderMesh* buildUvRenderMesh(
    const std::vector<AutoRemesher::Vector3>& vertices,
    const std::vector<std::vector<size_t>>& triangles,
    const std::vector<std::vector<AutoRemesher::Vector2>>& triangleUvs,
    const std::vector<std::vector<AutoRemesher::Vector2>>& originalTriangleUvs,
    const std::vector<AutoRemesher::Vector3>& singularVertices,
    const std::vector<std::pair<AutoRemesher::Vector3, AutoRemesher::Vector3>>& extractedConnections,
    const std::vector<uint8_t>& extractedConnectionMoved)
{
    if (vertices.empty() || triangles.empty() || triangleUvs.empty())
        return new ModelShaderMesh;

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
    AutoRemesher::Vector3 origin = {
        (maxX + minX) * 0.5, (maxY + minY) * 0.5, (maxZ + minZ) * 0.5
    };
    double maxLength = std::max({ maxX - minX, maxY - minY, maxZ - minZ }) * 0.5;
    if (maxLength < 1e-10)
        maxLength = 1.0;

    std::vector<AutoRemesher::Vector3> normalizedVerts(vertices.size());
    for (size_t i = 0; i < vertices.size(); ++i)
        normalizedVerts[i] = (vertices[i] - origin) / maxLength;

    std::vector<AutoRemesher::Vector3> normals(vertices.size());
    for (const auto& tri : triangles) {
        AutoRemesher::Vector3 n = AutoRemesher::Vector3::normal(
            normalizedVerts[tri[0]], normalizedVerts[tri[1]], normalizedVerts[tri[2]]);
        normals[tri[0]] += n;
        normals[tri[1]] += n;
        normals[tri[2]] += n;
    }
    for (auto& n : normals)
        n.normalize();

    auto normalizeUv = [](double x) {
        return 0.5 + x * 0.5;
    };

    int vertexCount = (int)triangles.size() * 3;
    int edgeVertexCount = (int)triangles.size() * 6;
    ModelShaderVertex* vertData = new ModelShaderVertex[vertexCount];
    ModelShaderVertex* edgeData = new ModelShaderVertex[edgeVertexCount];
    memset(vertData, 0, sizeof(ModelShaderVertex) * vertexCount);
    memset(edgeData, 0, sizeof(ModelShaderVertex) * edgeVertexCount);

    int vi = 0;
    int ei = 0;
    for (size_t i = 0; i < triangles.size(); ++i) {
        const auto& tri = triangles[i];
        const auto& gridUvs = originalTriangleUvs.size() == triangleUvs.size()
            ? originalTriangleUvs
            : triangleUvs;
        const auto& uvTri = (i < gridUvs.size()) ? gridUvs[i] : gridUvs.back();
        for (int j = 0; j < 3; ++j) {
            for (int e = 0; e < 2; ++e) {
                auto& ev = edgeData[ei++];
                int idx = (int)tri[(j + e) % 3];
                ev.posX = (float)normalizedVerts[idx].x();
                ev.posY = (float)normalizedVerts[idx].y();
                ev.posZ = (float)normalizedVerts[idx].z();
                ev.normX = (float)normals[idx].x();
                ev.normY = (float)normals[idx].y();
                ev.normZ = (float)normals[idx].z();
                ev.colorR = 0.0f;
                ev.colorG = 0.0f;
                ev.colorB = 0.0f;
                ev.roughness = 1.0f;
                ev.alpha = 1.0f;
            }

            auto& tv = vertData[vi++];
            int idx = (int)tri[j];
            tv.posX = (float)normalizedVerts[idx].x();
            tv.posY = (float)normalizedVerts[idx].y();
            tv.posZ = (float)normalizedVerts[idx].z();
            tv.normX = (float)normals[idx].x();
            tv.normY = (float)normals[idx].y();
            tv.normZ = (float)normals[idx].z();
            tv.colorR = 1.0f;
            tv.colorG = 1.0f;
            tv.colorB = 1.0f;
            tv.texU = (float)normalizeUv(uvTri[j].x());
            tv.texV = (float)normalizeUv(uvTri[j].y());
            tv.roughness = 1.0f;
            tv.alpha = 1.0f;
        }
    }

    ModelShaderMesh* mesh = new ModelShaderMesh(vertData, vertexCount, edgeData, ei, &normalizedVerts, &triangles);

    QImage* textureImage = new QImage(":/resources/crossuv.png");
    mesh->setTextureImage(textureImage);

    if (!extractedConnections.empty()) {
        auto* connectionVertices = new ModelShaderVertex[extractedConnections.size() * 2];
        memset(connectionVertices, 0, sizeof(ModelShaderVertex) * extractedConnections.size() * 2);
        size_t connectionVertexIndex = 0;
        for (size_t connectionIndex = 0; connectionIndex < extractedConnections.size();
            ++connectionIndex) {
            const auto& connection = extractedConnections[connectionIndex];
            const uint8_t flag = connectionIndex < extractedConnectionMoved.size()
                ? extractedConnectionMoved[connectionIndex]
                : 0;
            const bool moved = 1 == flag;
            const bool added = 2 == flag;
            for (const auto& point : { connection.first, connection.second }) {
                auto& vertex = connectionVertices[connectionVertexIndex++];
                const AutoRemesher::Vector3 normalizedPoint = (point - origin) / maxLength;
                vertex.posX = static_cast<float>(normalizedPoint.x());
                vertex.posY = static_cast<float>(normalizedPoint.y());
                vertex.posZ = static_cast<float>(normalizedPoint.z());
                vertex.colorR = added ? 1.0f : (moved ? 0.95f : 1.0f);
                vertex.colorG = added ? 0.55f : (moved ? 0.10f : 1.0f);
                vertex.colorB = added ? 0.05f : (moved ? 0.10f : 1.0f);
                vertex.alpha = 1.0f;
            }
        }
        mesh->updateConnectionEdges(connectionVertices, static_cast<int>(connectionVertexIndex));
    }

    if (!singularVertices.empty()) {
        const float phi = (1.0f + std::sqrt(5.0f)) * 0.5f;
        std::vector<AutoRemesher::Vector3> sphereVerts = {
            { -1, phi, 0 }, { 1, phi, 0 }, { -1, -phi, 0 }, { 1, -phi, 0 },
            { 0, -1, phi }, { 0, 1, phi }, { 0, -1, -phi }, { 0, 1, -phi },
            { phi, 0, -1 }, { phi, 0, 1 }, { -phi, 0, -1 }, { -phi, 0, 1 }
        };
        for (auto& v : sphereVerts) {
            float len = std::sqrt(v.x() * v.x() + v.y() * v.y() + v.z() * v.z());
            if (len > 1e-10f) {
                v.setX(v.x() / len);
                v.setY(v.y() / len);
                v.setZ(v.z() / len);
            }
        }
        int indices[20][3] = {
            { 0, 11, 5 }, { 0, 5, 1 }, { 0, 1, 7 }, { 0, 7, 10 }, { 0, 10, 11 },
            { 1, 5, 9 }, { 5, 11, 4 }, { 11, 10, 2 }, { 10, 7, 6 }, { 7, 1, 8 },
            { 3, 9, 4 }, { 3, 4, 2 }, { 3, 2, 6 }, { 3, 6, 8 }, { 3, 8, 9 },
            { 4, 9, 5 }, { 2, 4, 11 }, { 6, 2, 10 }, { 8, 6, 7 }, { 9, 8, 1 }
        };
        std::vector<AutoRemesher::Vector3> subVerts;
        std::vector<int> subIndices;
        for (int f = 0; f < 20; ++f) {
            int i0 = indices[f][0], i1 = indices[f][1], i2 = indices[f][2];
            auto& v0 = sphereVerts[i0];
            auto& v1 = sphereVerts[i1];
            auto& v2 = sphereVerts[i2];
            auto mid = [](const AutoRemesher::Vector3& a, const AutoRemesher::Vector3& b) {
                AutoRemesher::Vector3 m = { (a.x() + b.x()) * 0.5f, (a.y() + b.y()) * 0.5f, (a.z() + b.z()) * 0.5f };
                float len = std::sqrt(m.x() * m.x() + m.y() * m.y() + m.z() * m.z());
                if (len > 1e-10f) {
                    m.setX(m.x() / len);
                    m.setY(m.y() / len);
                    m.setZ(m.z() / len);
                }
                return m;
            };
            AutoRemesher::Vector3 m01 = mid(v0, v1);
            AutoRemesher::Vector3 m12 = mid(v1, v2);
            AutoRemesher::Vector3 m20 = mid(v2, v0);
            int bi = (int)subVerts.size();
            subVerts.push_back(v0);
            subVerts.push_back(v1);
            subVerts.push_back(v2);
            subVerts.push_back(m01);
            subVerts.push_back(m12);
            subVerts.push_back(m20);
            subIndices.push_back(bi + 0);
            subIndices.push_back(bi + 3);
            subIndices.push_back(bi + 5);
            subIndices.push_back(bi + 3);
            subIndices.push_back(bi + 1);
            subIndices.push_back(bi + 4);
            subIndices.push_back(bi + 5);
            subIndices.push_back(bi + 4);
            subIndices.push_back(bi + 2);
            subIndices.push_back(bi + 3);
            subIndices.push_back(bi + 4);
            subIndices.push_back(bi + 5);
        }

        double sphereRadius = 0.000625;
        if (sphereRadius < 0.0005)
            sphereRadius = 0.0005;

        int sphereTriCount = (int)subIndices.size() / 3;
        int totalToolVerts = (int)singularVertices.size() * sphereTriCount * 3;
        ModelShaderVertex* toolVerts = new ModelShaderVertex[totalToolVerts];
        memset(toolVerts, 0, sizeof(ModelShaderVertex) * totalToolVerts);

        int toolVi = 0;
        for (const auto& svPos : singularVertices) {
            AutoRemesher::Vector3 normalizedPos = (svPos - origin) / maxLength;
            for (int t = 0; t < sphereTriCount; ++t) {
                for (int j = 0; j < 3; ++j) {
                    auto& tv = toolVerts[toolVi++];
                    int vi = subIndices[t * 3 + j];
                    tv.posX = (float)(normalizedPos.x() + subVerts[vi].x() * sphereRadius);
                    tv.posY = (float)(normalizedPos.y() + subVerts[vi].y() * sphereRadius);
                    tv.posZ = (float)(normalizedPos.z() + subVerts[vi].z() * sphereRadius);
                    tv.normX = subVerts[vi].x();
                    tv.normY = subVerts[vi].y();
                    tv.normZ = subVerts[vi].z();
                    tv.colorR = 1.0f;
                    tv.colorG = 1.0f;
                    tv.colorB = 1.0f;
                    tv.roughness = 0.3f;
                    tv.alpha = 1.0f;
                }
            }
        }

        mesh->updateTool(toolVerts, totalToolVerts);
    }

    return mesh;
}

void PreviewMeshGenerator::generate()
{
    delete m_isotropicMesh;
    m_isotropicMesh = buildRenderMeshFromTriangles(m_isotropicVertices, m_isotropicTriangles);

    delete m_paramMesh;
    if (!m_isotropicVertices.empty() && !m_isotropicTriangles.empty()
        && !m_isotropicTriangleUvs.empty()) {
        m_paramMesh = buildUvRenderMesh(
            m_isotropicVertices, m_isotropicTriangles,
            m_isotropicTriangleUvs, m_isotropicOriginalTriangleUvs,
            m_isotropicSingularVertices,
            m_isotropicExtractedConnections, m_isotropicExtractedConnectionMoved);
    } else {
        m_paramMesh = new ModelShaderMesh;
    }
}
