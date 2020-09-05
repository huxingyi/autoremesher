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
#ifndef AUTO_REMESHER_RENDER_MESH_GENERATOR_H
#define AUTO_REMESHER_RENDER_MESH_GENERATOR_H
#include <QObject>
#include <AutoRemesher/Vector3>
#include "pbrshadermesh.h"

class RenderMeshGenerator: public QObject
{
    Q_OBJECT
public:
    RenderMeshGenerator(const std::vector<AutoRemesher::Vector3> &vertices,
            const std::vector<std::vector<size_t>> &faces) :
        m_vertices(new std::vector<AutoRemesher::Vector3>(vertices)),
        m_faces(new std::vector<std::vector<size_t>>(faces))
    {
    }
    
    ~RenderMeshGenerator()
    {
        delete m_renderMesh;
    }

    PbrShaderMesh *takeRenderMesh()
    {
        PbrShaderMesh *renderMesh = m_renderMesh;
        m_renderMesh = nullptr;
        return renderMesh;
    }
    
    void generate();
    
signals:
    void finished();
public slots:
    void process();
    
private:
    std::vector<AutoRemesher::Vector3> *m_vertices = nullptr;
    std::vector<std::vector<size_t>> *m_faces = nullptr;
    PbrShaderMesh *m_renderMesh = nullptr;
    
    void normalizeVertices();
    static void calculateNormalizedFactors(const std::vector<AutoRemesher::Vector3> &vertices, 
        AutoRemesher::Vector3 *origin, 
        double *maxLength);
};

#endif
