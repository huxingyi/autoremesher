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
#ifndef AUTO_REMESHER_PBR_SHADER_MESH_H
#define AUTO_REMESHER_PBR_SHADER_MESH_H
#include <QObject>
#include <vector>
#include <AutoRemesher/Vector3>
#include <QColor>
#include <QImage>
#include <QTextStream>
#include "pbrshadervertex.h"

class PbrShaderMesh
{
public:
    PbrShaderMesh(const std::vector<AutoRemesher::Vector3> &vertices, const std::vector<std::vector<size_t>> &triangles,
        const std::vector<std::vector<AutoRemesher::Vector3>> &triangleVertexNormals,
        const QColor &color=Qt::white);
    PbrShaderMesh(PbrShaderVertex *triangleVertices, int vertexNum, PbrShaderVertex *edgeVertices=nullptr, int edgeVertexCount=0,
        std::vector<AutoRemesher::Vector3> *vertices=nullptr, std::vector<std::vector<size_t>> *faces=nullptr);
    PbrShaderMesh(const PbrShaderMesh &mesh);
    PbrShaderMesh();
    ~PbrShaderMesh();
    PbrShaderVertex *triangleVertices();
    int triangleVertexCount();
    PbrShaderVertex *edgeVertices();
    int edgeVertexCount();
    PbrShaderVertex *toolVertices();
    int toolVertexCount();
    const std::vector<AutoRemesher::Vector3> &vertices();
    const std::vector<std::vector<size_t>> &faces();
    void setTextureImage(QImage *textureImage);
    const QImage *textureImage();
    void setNormalMapImage(QImage *normalMapImage);
    const QImage *normalMapImage();
    const QImage *metalnessRoughnessAmbientOcclusionImage();
    void setMetalnessRoughnessAmbientOcclusionImage(QImage *image);
    bool hasMetalnessInImage();
    void setHasMetalnessInImage(bool hasInImage);
    bool hasRoughnessInImage();
    void setHasRoughnessInImage(bool hasInImage);
    bool hasAmbientOcclusionInImage();
    void setHasAmbientOcclusionInImage(bool hasInImage);
    static float m_defaultMetalness;
    static float m_defaultRoughness;
    void updateTool(PbrShaderVertex *toolVertices, int vertexNum);
    void updateEdges(PbrShaderVertex *edgeVertices, int edgeVertexCount);
    void updateTriangleVertices(PbrShaderVertex *triangleVertices, int triangleVertexCount);
    quint64 meshId() const;
    void setMeshId(quint64 id);
    void removeColor();
private:
    PbrShaderVertex *m_triangleVertices = nullptr;
    int m_triangleVertexCount = 0;
    PbrShaderVertex *m_edgeVertices = nullptr;
    int m_edgeVertexCount = 0;
    PbrShaderVertex *m_toolVertices = nullptr;
    int m_toolVertexCount = 0;
    std::vector<AutoRemesher::Vector3> m_vertices;
    std::vector<std::vector<size_t>> m_faces;
    QImage *m_textureImage = nullptr;
    QImage *m_normalMapImage = nullptr;
    QImage *m_metalnessRoughnessAmbientOcclusionImage = nullptr;
    bool m_hasMetalnessInImage = false;
    bool m_hasRoughnessInImage = false;
    bool m_hasAmbientOcclusionInImage = false;
    quint64 m_meshId = 0;
};

#endif
