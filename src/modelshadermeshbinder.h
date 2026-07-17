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
#ifndef AUTO_REMESHER_MODEL_SHADER_MESH_BINDER_H
#define AUTO_REMESHER_MODEL_SHADER_MESH_BINDER_H
#include "modelshadermesh.h"
#include "modelshaderprogram.h"
#include "monochromeopenglobject.h"
#include "monochromeopenglprogram.h"
#include <QMutex>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QString>

class ModelShaderMeshBinder {
public:
    ModelShaderMeshBinder(bool toolEnabled = false);
    ~ModelShaderMeshBinder();
    void updateMesh(ModelShaderMesh* mesh);
    void initialize();
    void paint(ModelShaderProgram* program);
    void paintWireframe();
    void cleanup();
    void showWireframe();
    void hideWireframe();
    bool isWireframeVisible();
    void reloadMesh();
    void fetchCurrentToonNormalAndDepthMaps(QImage* normalMap, QImage* depthMap);
    void updateToonNormalAndDepthMaps(QImage* normalMap, QImage* depthMap);

private:
    ModelShaderMesh* m_mesh = nullptr;
    ModelShaderMesh* m_newMesh = nullptr;
    int m_renderTriangleVertexCount = 0;
    int m_renderToolVertexCount = 0;
    bool m_newMeshComing = false;
    bool m_showWireframe = false;
    bool m_hasTexture = false;
    QOpenGLTexture* m_texture = nullptr;
    bool m_hasNormalMap = false;
    QOpenGLTexture* m_normalMap = nullptr;
    bool m_hasMetalnessMap = false;
    bool m_hasRoughnessMap = false;
    bool m_hasAmbientOcclusionMap = false;
    QOpenGLTexture* m_metalnessRoughnessAmbientOcclusionMap = nullptr;
    bool m_toolEnabled = false;
    QOpenGLTexture* m_toonNormalMap = nullptr;
    QOpenGLTexture* m_toonDepthMap = nullptr;
    QImage* m_newToonNormalMap = nullptr;
    QImage* m_newToonDepthMap = nullptr;
    QImage* m_currentToonNormalMap = nullptr;
    QImage* m_currentToonDepthMap = nullptr;
    bool m_newToonMapsComing = false;

private:
    QOpenGLVertexArrayObject m_vaoTriangle;
    QOpenGLBuffer m_vboTriangle;
    MonochromeOpenGLObject m_wireframeObject;
    MonochromeOpenGLObject m_connectionWireframeObject;
    QOpenGLVertexArrayObject m_vaoTool;
    QOpenGLBuffer m_vboTool;
    QMutex m_meshMutex;
    QMutex m_newMeshMutex;
    QMutex m_toonNormalAndDepthMapMutex;
};

#endif
