/*
 *  Copyright (c) 2020 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
#ifndef AUTO_REMESHER_QUAD_MESH_GENERATOR_H
#define AUTO_REMESHER_QUAD_MESH_GENERATOR_H
#include <AutoRemesher/AutoRemesher>
#include <AutoRemesher/Vector2>
#include <QObject>
#include <cstdint>
#include <memory>
#include <utility>

class QuadMeshGenerator : public QObject {
    Q_OBJECT
public:
    struct Parameters {
        double scaling = 0.0;
        size_t targetTriangleCount = 0;
        AutoRemesher::ModelType modelType = AutoRemesher::ModelType::Organic;
        double adaptivity = 1.0;
        double anisotropy = 1.0;
        double sharpEdgeDegrees = 90.0;
        double smoothNormalDegrees = 0.0;
    };

    QuadMeshGenerator(const std::vector<AutoRemesher::Vector3>& vertices,
        const std::vector<std::vector<size_t>>& triangles)
        : m_vertices(vertices)
        , m_triangles(triangles)
    {
    }

    void setParameters(const Parameters& parameters)
    {
        m_parameters = parameters;
    }

    std::vector<AutoRemesher::Vector3>* takeRemeshedVertices()
    {
        return m_remeshedVertices.release();
    }

    std::vector<std::vector<size_t>>* takeRemeshedQuads()
    {
        return m_remeshedQuads.release();
    }

    const std::vector<AutoRemesher::Vector3>& decimatedVertices() const
    {
        return m_decimatedVertices;
    }

    const std::vector<std::vector<size_t>>& decimatedTriangles() const
    {
        return m_decimatedTriangles;
    }

    bool decimated() const
    {
        return m_decimated;
    }

    const std::vector<AutoRemesher::Vector3>& isotropicVertices() const
    {
        return m_isotropicVertices;
    }

    const std::vector<std::vector<size_t>>& isotropicTriangles() const
    {
        return m_isotropicTriangles;
    }

    const std::vector<uint8_t>& isotropicExtractedConnectionMoved()
    {
        return m_isotropicExtractedConnectionMoved;
    }

    const std::vector<std::vector<AutoRemesher::Vector2>>& isotropicOriginalTriangleUvs()
    {
        return m_isotropicOriginalTriangleUvs;
    }

    const std::vector<std::vector<AutoRemesher::Vector2>>& isotropicTriangleUvs() const
    {
        return m_isotropicTriangleUvs;
    }

    const std::vector<AutoRemesher::Vector3>& isotropicSingularVertices() const
    {
        return m_isotropicSingularVertices;
    }

    const std::vector<std::pair<AutoRemesher::Vector3, AutoRemesher::Vector3>>& isotropicExtractedConnections() const
    {
        return m_isotropicExtractedConnections;
    }

    void generate();
    void printProgress(float progress, const QString& status);
    void emitProgress(float progress);
    void emitProgress(float progress, const QString& status);

signals:
    void reportProgress(float progress);
    void reportProgressDetailed(float progress, const QString& status);
    void finished();

public slots:
    void process();

private:
    // The pipeline now reports many times per whole percent, so the console line
    // is only reprinted when the percentage or the step actually changes.
    int m_lastPrintedPercent = -1;
    QString m_lastPrintedStatus;
    std::vector<AutoRemesher::Vector3> m_vertices;
    std::vector<std::vector<size_t>> m_triangles;
    std::unique_ptr<std::vector<AutoRemesher::Vector3>> m_remeshedVertices;
    std::unique_ptr<std::vector<std::vector<size_t>>> m_remeshedQuads;
    std::vector<AutoRemesher::Vector3> m_decimatedVertices;
    std::vector<std::vector<size_t>> m_decimatedTriangles;
    bool m_decimated = false;
    std::vector<AutoRemesher::Vector3> m_isotropicVertices;
    std::vector<std::vector<size_t>> m_isotropicTriangles;
    std::vector<std::vector<AutoRemesher::Vector2>> m_isotropicTriangleUvs;
    std::vector<std::vector<AutoRemesher::Vector2>> m_isotropicOriginalTriangleUvs;
    std::vector<uint8_t> m_isotropicExtractedConnectionMoved;
    std::vector<AutoRemesher::Vector3> m_isotropicSingularVertices;
    std::vector<std::pair<AutoRemesher::Vector3, AutoRemesher::Vector3>> m_isotropicExtractedConnections;
    std::unique_ptr<AutoRemesher::AutoRemesher> m_autoRemesher;
    Parameters m_parameters;
};

#endif
