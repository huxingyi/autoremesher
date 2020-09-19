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
#include <QElapsedTimer>
#include <QDebug>
#include "quadmeshgenerator.h"

void QuadMeshGenerator::process()
{
    QElapsedTimer timer;
    timer.start();
    
    generate();
    
    auto timeUsed = timer.elapsed();
    qDebug() << "Quad mesh generation took" << timeUsed << "milliseconds";
    
    emit finished();
}

void QuadMeshGenerator::emitProgress(float progress)
{
    emit reportProgress(progress);
}

static void reportProgressHandler(void *tag, float progress)
{
    QuadMeshGenerator *generator = (QuadMeshGenerator *)tag;
    generator->emitProgress(progress);
}

void QuadMeshGenerator::generate()
{
    delete m_autoRemesher;
    m_autoRemesher = new AutoRemesher::AutoRemesher(m_vertices, m_triangles);
    if (m_parameters.scaling > 0)
        m_autoRemesher->setScaling(m_parameters.scaling);
    if (m_parameters.targetTriangleCount > 0)
        m_autoRemesher->setTargetTriangleCount(m_parameters.targetTriangleCount);
    m_autoRemesher->setModelType(m_parameters.modelType);
    m_autoRemesher->setTag(this);
    m_autoRemesher->setProgressHandler(reportProgressHandler);
    if (!m_autoRemesher->remesh())
        return;
    
    delete m_remeshedVertices;
    m_remeshedVertices = new std::vector<AutoRemesher::Vector3>(m_autoRemesher->remeshedVertices());
    
    delete m_remeshedQuads;
    m_remeshedQuads = new std::vector<std::vector<size_t>>(m_autoRemesher->remeshedQuads());
}
