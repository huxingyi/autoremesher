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
#include <QTextStream>
#include <QFile>
#include <cmath>
#include "pbrshadermesh.h"

float PbrShaderMesh::m_defaultMetalness = 0.0;
float PbrShaderMesh::m_defaultRoughness = 1.0;

PbrShaderMesh::PbrShaderMesh(const PbrShaderMesh &mesh)
{
    if (nullptr != mesh.m_triangleVertices &&
            mesh.m_triangleVertexCount > 0) {
        this->m_triangleVertices = new PbrShaderVertex[mesh.m_triangleVertexCount];
        this->m_triangleVertexCount = mesh.m_triangleVertexCount;
        for (int i = 0; i < mesh.m_triangleVertexCount; i++)
            this->m_triangleVertices[i] = mesh.m_triangleVertices[i];
    }
    if (nullptr != mesh.m_edgeVertices &&
            mesh.m_edgeVertexCount > 0) {
        this->m_edgeVertices = new PbrShaderVertex[mesh.m_edgeVertexCount];
        this->m_edgeVertexCount = mesh.m_edgeVertexCount;
        for (int i = 0; i < mesh.m_edgeVertexCount; i++)
            this->m_edgeVertices[i] = mesh.m_edgeVertices[i];
    }
    if (nullptr != mesh.m_toolVertices &&
            mesh.m_toolVertexCount > 0) {
        this->m_toolVertices = new PbrShaderVertex[mesh.m_toolVertexCount];
        this->m_toolVertexCount = mesh.m_toolVertexCount;
        for (int i = 0; i < mesh.m_toolVertexCount; i++)
            this->m_toolVertices[i] = mesh.m_toolVertices[i];
    }
    if (nullptr != mesh.m_textureImage) {
        this->m_textureImage = new QImage(*mesh.m_textureImage);
    }
    if (nullptr != mesh.m_normalMapImage) {
        this->m_normalMapImage = new QImage(*mesh.m_normalMapImage);
    }
    if (nullptr != mesh.m_metalnessRoughnessAmbientOcclusionImage) {
        this->m_metalnessRoughnessAmbientOcclusionImage = new QImage(*mesh.m_metalnessRoughnessAmbientOcclusionImage);
        this->m_hasMetalnessInImage = mesh.m_hasMetalnessInImage;
        this->m_hasRoughnessInImage = mesh.m_hasRoughnessInImage;
        this->m_hasAmbientOcclusionInImage = mesh.m_hasAmbientOcclusionInImage;
    }
    this->m_vertices = mesh.m_vertices;
    this->m_faces = mesh.m_faces;
    this->m_meshId = mesh.meshId();
}

void PbrShaderMesh::removeColor()
{
    delete this->m_textureImage;
    this->m_textureImage = nullptr;
    
    delete this->m_normalMapImage;
    this->m_normalMapImage = nullptr;
    
    delete this->m_metalnessRoughnessAmbientOcclusionImage;
    this->m_metalnessRoughnessAmbientOcclusionImage = nullptr;
    
    this->m_hasMetalnessInImage = false;
    this->m_hasRoughnessInImage = false;
    this->m_hasAmbientOcclusionInImage = false;
    
    for (int i = 0; i < this->m_triangleVertexCount; ++i) {
        auto &vertex = this->m_triangleVertices[i];
        vertex.colorR = 1.0;
        vertex.colorG = 1.0;
        vertex.colorB = 1.0;
    }
}

PbrShaderMesh::PbrShaderMesh(PbrShaderVertex *triangleVertices, int vertexNum, PbrShaderVertex *edgeVertices, int edgeVertexCount,
        std::vector<AutoRemesher::Vector3> *vertices, std::vector<std::vector<size_t>> *faces) :
    m_triangleVertices(triangleVertices),
    m_triangleVertexCount(vertexNum),
    m_edgeVertices(edgeVertices),
    m_edgeVertexCount(edgeVertexCount)
{
    if (nullptr != vertices)
        m_vertices = *vertices;
    if (nullptr != faces)
        m_faces = *faces;
}

PbrShaderMesh::PbrShaderMesh(const std::vector<AutoRemesher::Vector3> &vertices, const std::vector<std::vector<size_t>> &triangles,
    const std::vector<std::vector<AutoRemesher::Vector3>> &triangleVertexNormals,
    const QColor &color)
{
    m_vertices = vertices;
    m_faces = triangles;
    
    m_triangleVertexCount = (int)triangles.size() * 3;
    m_triangleVertices = new PbrShaderVertex[m_triangleVertexCount];
    int destIndex = 0;
    for (size_t i = 0; i < triangles.size(); ++i) {
        for (auto j = 0; j < 3; j++) {
            int vertexIndex = (int)triangles[i][j];
            const AutoRemesher::Vector3 *srcVert = &vertices[vertexIndex];
            const AutoRemesher::Vector3 *srcNormal = &(triangleVertexNormals)[i][j];
            PbrShaderVertex *dest = &m_triangleVertices[destIndex];
            dest->colorR = color.redF();
            dest->colorG = color.greenF();
            dest->colorB = color.blueF();
            dest->alpha = color.alphaF();
            dest->posX = srcVert->x();
            dest->posY = srcVert->y();
            dest->posZ = srcVert->z();
            dest->texU = 0;
            dest->texV = 0;
            dest->normX = srcNormal->x();
            dest->normY = srcNormal->y();
            dest->normZ = srcNormal->z();
            dest->metalness = m_defaultMetalness;
            dest->roughness = m_defaultRoughness;
            dest->tangentX = 0;
            dest->tangentY = 0;
            dest->tangentZ = 0;
            dest->alpha = 1.0;
            destIndex++;
        }
    }
}

PbrShaderMesh::PbrShaderMesh()
{
}

PbrShaderMesh::~PbrShaderMesh()
{
    delete[] m_triangleVertices;
    m_triangleVertexCount = 0;
    delete[] m_edgeVertices;
    m_edgeVertexCount = 0;
    delete[] m_toolVertices;
    m_toolVertexCount = 0;
    delete m_textureImage;
    delete m_normalMapImage;
    delete m_metalnessRoughnessAmbientOcclusionImage;
}

const std::vector<AutoRemesher::Vector3> &PbrShaderMesh::vertices()
{
    return m_vertices;
}

const std::vector<std::vector<size_t>> &PbrShaderMesh::faces()
{
    return m_faces;
}

PbrShaderVertex *PbrShaderMesh::triangleVertices()
{
    return m_triangleVertices;
}

int PbrShaderMesh::triangleVertexCount()
{
    return m_triangleVertexCount;
}

PbrShaderVertex *PbrShaderMesh::edgeVertices()
{
    return m_edgeVertices;
}

int PbrShaderMesh::edgeVertexCount()
{
    return m_edgeVertexCount;
}

PbrShaderVertex *PbrShaderMesh::toolVertices()
{
    return m_toolVertices;
}

int PbrShaderMesh::toolVertexCount()
{
    return m_toolVertexCount;
}

void PbrShaderMesh::setTextureImage(QImage *textureImage)
{
    m_textureImage = textureImage;
}

const QImage *PbrShaderMesh::textureImage()
{
    return m_textureImage;
}

void PbrShaderMesh::setNormalMapImage(QImage *normalMapImage)
{
    m_normalMapImage = normalMapImage;
}

const QImage *PbrShaderMesh::normalMapImage()
{
    return m_normalMapImage;
}

const QImage *PbrShaderMesh::metalnessRoughnessAmbientOcclusionImage()
{
    return m_metalnessRoughnessAmbientOcclusionImage;
}

void PbrShaderMesh::setMetalnessRoughnessAmbientOcclusionImage(QImage *image)
{
    m_metalnessRoughnessAmbientOcclusionImage = image;
}

bool PbrShaderMesh::hasMetalnessInImage()
{
    return m_hasMetalnessInImage;
}

void PbrShaderMesh::setHasMetalnessInImage(bool hasInImage)
{
    m_hasMetalnessInImage = hasInImage;
}

bool PbrShaderMesh::hasRoughnessInImage()
{
    return m_hasRoughnessInImage;
}

void PbrShaderMesh::setHasRoughnessInImage(bool hasInImage)
{
    m_hasRoughnessInImage = hasInImage;
}

bool PbrShaderMesh::hasAmbientOcclusionInImage()
{
    return m_hasAmbientOcclusionInImage;
}

void PbrShaderMesh::setHasAmbientOcclusionInImage(bool hasInImage)
{
    m_hasAmbientOcclusionInImage = hasInImage;
}

void PbrShaderMesh::updateTool(PbrShaderVertex *toolVertices, int vertexNum)
{
    delete[] m_toolVertices;
    m_toolVertices = nullptr;
    m_toolVertexCount = 0;
    
    m_toolVertices = toolVertices;
    m_toolVertexCount = vertexNum;
}

void PbrShaderMesh::updateEdges(PbrShaderVertex *edgeVertices, int edgeVertexCount)
{
    delete[] m_edgeVertices;
    m_edgeVertices = nullptr;
    m_edgeVertexCount = 0;
    
    m_edgeVertices = edgeVertices;
    m_edgeVertexCount = edgeVertexCount;
}

void PbrShaderMesh::updateTriangleVertices(PbrShaderVertex *triangleVertices, int triangleVertexCount)
{
    delete[] m_triangleVertices;
    m_triangleVertices = 0;
    m_triangleVertexCount = 0;
    
    m_triangleVertices = triangleVertices;
    m_triangleVertexCount = triangleVertexCount;
}

quint64 PbrShaderMesh::meshId() const
{
    return m_meshId;
}

void PbrShaderMesh::setMeshId(quint64 id)
{
    m_meshId = id;
}
