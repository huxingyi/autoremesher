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
#ifndef AUTO_REMESHER_QUAD_MESH_GENERATOR_H
#define AUTO_REMESHER_QUAD_MESH_GENERATOR_H
#include <QObject>
#include <AutoRemesher/AutoRemesher>

class QuadMeshGenerator: public QObject
{
    Q_OBJECT
public:
    struct Parameters
    {
        double scaling = 0.0;
        size_t targetTriangleCount = 0;
        AutoRemesher::ModelType modelType = AutoRemesher::ModelType::Organic;
    };

    QuadMeshGenerator(const std::vector<AutoRemesher::Vector3> &vertices,
            const std::vector<std::vector<size_t>> &triangles) :
        m_vertices(vertices),
        m_triangles(triangles)
    {
    }
    
    ~QuadMeshGenerator()
    {
        delete m_remeshedVertices;
        delete m_remeshedQuads;
        delete m_autoRemesher;
    }
    
    void setParameters(const Parameters &parameters)
    {
        m_parameters = parameters;
    }
    
    std::vector<AutoRemesher::Vector3> *takeRemeshedVertices()
    {
        std::vector<AutoRemesher::Vector3> *remeshedVertices = m_remeshedVertices;
        m_remeshedVertices = nullptr;
        return remeshedVertices;
    }
    
    std::vector<std::vector<size_t>> *takeRemeshedQuads()
    {
        std::vector<std::vector<size_t>> *remeshedQuads = m_remeshedQuads;
        m_remeshedQuads = nullptr;
        return remeshedQuads;
    }

    void generate();
    void emitProgress(float progress);
    
signals:
    void reportProgress(float progress);
    void finished();
public slots:
    void process();
    
private:
    std::vector<AutoRemesher::Vector3> m_vertices;
    std::vector<std::vector<size_t>> m_triangles;
    std::vector<AutoRemesher::Vector3> *m_remeshedVertices = nullptr;
    std::vector<std::vector<size_t>> *m_remeshedQuads = nullptr;
    AutoRemesher::AutoRemesher *m_autoRemesher = nullptr;
    Parameters m_parameters;
};

#endif
