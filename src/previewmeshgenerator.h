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
#ifndef AUTO_REMESHER_PREVIEW_MESH_GENERATOR_H
#define AUTO_REMESHER_PREVIEW_MESH_GENERATOR_H
#include "modelshadermesh.h"
#include <AutoRemesher/Vector2>
#include <AutoRemesher/Vector3>
#include <QObject>
#include <cstdint>
#include <utility>
#include <vector>

class PreviewMeshGenerator : public QObject {
    Q_OBJECT
public:
    PreviewMeshGenerator(const std::vector<AutoRemesher::Vector3>& isotropicVertices,
        const std::vector<std::vector<size_t>>& isotropicTriangles,
        const std::vector<std::vector<AutoRemesher::Vector2>>& isotropicTriangleUvs,
        const std::vector<std::vector<AutoRemesher::Vector2>>& isotropicOriginalTriangleUvs,
        const std::vector<AutoRemesher::Vector3>& isotropicSingularVertices,
        const std::vector<std::pair<AutoRemesher::Vector3, AutoRemesher::Vector3>>& isotropicExtractedConnections,
        const std::vector<uint8_t>& isotropicExtractedConnectionMoved)
        : m_isotropicVertices(isotropicVertices)
        , m_isotropicTriangles(isotropicTriangles)
        , m_isotropicTriangleUvs(isotropicTriangleUvs)
        , m_isotropicOriginalTriangleUvs(isotropicOriginalTriangleUvs)
        , m_isotropicSingularVertices(isotropicSingularVertices)
        , m_isotropicExtractedConnections(isotropicExtractedConnections)
        , m_isotropicExtractedConnectionMoved(isotropicExtractedConnectionMoved)
    {
    }

    ~PreviewMeshGenerator()
    {
        delete m_isotropicMesh;
        delete m_paramMesh;
    }

    ModelShaderMesh* takeIsotropicMesh()
    {
        ModelShaderMesh* mesh = m_isotropicMesh;
        m_isotropicMesh = nullptr;
        return mesh;
    }

    ModelShaderMesh* takeParamMesh()
    {
        ModelShaderMesh* mesh = m_paramMesh;
        m_paramMesh = nullptr;
        return mesh;
    }

    void generate();

signals:
    void finished();
public slots:
    void process();

private:
    std::vector<AutoRemesher::Vector3> m_isotropicVertices;
    std::vector<std::vector<size_t>> m_isotropicTriangles;
    std::vector<std::vector<AutoRemesher::Vector2>> m_isotropicTriangleUvs;
    std::vector<std::vector<AutoRemesher::Vector2>> m_isotropicOriginalTriangleUvs;
    std::vector<AutoRemesher::Vector3> m_isotropicSingularVertices;
    std::vector<std::pair<AutoRemesher::Vector3, AutoRemesher::Vector3>> m_isotropicExtractedConnections;
    std::vector<uint8_t> m_isotropicExtractedConnectionMoved;
    ModelShaderMesh* m_isotropicMesh = nullptr;
    ModelShaderMesh* m_paramMesh = nullptr;
};

#endif
