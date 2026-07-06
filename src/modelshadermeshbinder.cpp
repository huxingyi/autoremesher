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
#include "modelshadermeshbinder.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QSurfaceFormat>
#include <QTextStream>
#include <map>

ModelShaderMeshBinder::ModelShaderMeshBinder(bool toolEnabled)
    : m_toolEnabled(toolEnabled)
{
}

ModelShaderMeshBinder::~ModelShaderMeshBinder()
{
    delete m_mesh;
    delete m_newMesh;
    delete m_texture;
    delete m_normalMap;
    delete m_metalnessRoughnessAmbientOcclusionMap;
    delete m_toonNormalMap;
    delete m_toonDepthMap;
    delete m_newToonNormalMap;
    delete m_newToonDepthMap;
    delete m_currentToonNormalMap;
    delete m_currentToonDepthMap;
}

void ModelShaderMeshBinder::updateMesh(ModelShaderMesh* mesh)
{
    QMutexLocker lock(&m_newMeshMutex);
    if (mesh != m_mesh) {
        delete m_newMesh;
        m_newMesh = mesh;
        m_newMeshComing = true;
    }
}

void ModelShaderMeshBinder::reloadMesh()
{
    ModelShaderMesh* mesh = nullptr;
    {
        QMutexLocker lock(&m_newMeshMutex);
        if (nullptr == m_mesh)
            return;
        mesh = new ModelShaderMesh(*m_mesh);
    }
    if (nullptr != mesh)
        updateMesh(mesh);
}

void ModelShaderMeshBinder::initialize()
{
    m_vaoTriangle.create();
    if (m_toolEnabled)
        m_vaoTool.create();
}

void ModelShaderMeshBinder::paint(ModelShaderProgram* program)
{
    ModelShaderMesh* newMesh = nullptr;
    bool hasNewMesh = false;
    if (m_newMeshComing) {
        QMutexLocker lock(&m_newMeshMutex);
        if (m_newMeshComing) {
            newMesh = m_newMesh;
            m_newMesh = nullptr;
            m_newMeshComing = false;
            hasNewMesh = true;
        }
    }
    if (m_newToonMapsComing) {
        QMutexLocker lock(&m_toonNormalAndDepthMapMutex);
        if (m_newToonMapsComing) {

            delete m_toonNormalMap;
            m_toonNormalMap = nullptr;
            delete m_currentToonNormalMap;
            m_currentToonNormalMap = nullptr;
            if (nullptr != m_newToonNormalMap) {
                m_toonNormalMap = new QOpenGLTexture(*m_newToonNormalMap);
                m_currentToonNormalMap = m_newToonNormalMap;
                m_newToonNormalMap = nullptr;
            }

            delete m_toonDepthMap;
            m_toonDepthMap = nullptr;
            delete m_currentToonDepthMap;
            m_currentToonDepthMap = nullptr;
            if (nullptr != m_newToonDepthMap) {
                m_toonDepthMap = new QOpenGLTexture(*m_newToonDepthMap);
                m_currentToonDepthMap = m_newToonDepthMap;
                m_newToonDepthMap = nullptr;
            }

            m_newToonMapsComing = false;
        }
    }
    {
        QMutexLocker lock(&m_meshMutex);
        if (hasNewMesh) {
            delete m_mesh;
            m_mesh = newMesh;
            if (m_mesh) {
                m_hasTexture = nullptr != m_mesh->textureImage();
                delete m_texture;
                m_texture = nullptr;
                if (m_hasTexture) {
                    m_texture = new QOpenGLTexture(*m_mesh->textureImage());
                }

                m_hasNormalMap = nullptr != m_mesh->normalMapImage();
                delete m_normalMap;
                m_normalMap = nullptr;
                if (m_hasNormalMap)
                    m_normalMap = new QOpenGLTexture(*m_mesh->normalMapImage());

                m_hasMetalnessMap = m_mesh->hasMetalnessInImage();
                m_hasRoughnessMap = m_mesh->hasRoughnessInImage();
                m_hasAmbientOcclusionMap = m_mesh->hasAmbientOcclusionInImage();
                delete m_metalnessRoughnessAmbientOcclusionMap;
                m_metalnessRoughnessAmbientOcclusionMap = nullptr;
                if (nullptr != m_mesh->metalnessRoughnessAmbientOcclusionImage() && (m_hasMetalnessMap || m_hasRoughnessMap || m_hasAmbientOcclusionMap))
                    m_metalnessRoughnessAmbientOcclusionMap = new QOpenGLTexture(*m_mesh->metalnessRoughnessAmbientOcclusionImage());

                {
                    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vaoTriangle);
                    if (m_vboTriangle.isCreated())
                        m_vboTriangle.destroy();
                    m_vboTriangle.create();
                    m_vboTriangle.bind();
                    m_vboTriangle.allocate(m_mesh->triangleVertices(), m_mesh->triangleVertexCount() * sizeof(ModelShaderVertex));
                    m_renderTriangleVertexCount = m_mesh->triangleVertexCount();
                    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
                    f->glEnableVertexAttribArray(0);
                    f->glEnableVertexAttribArray(1);
                    f->glEnableVertexAttribArray(2);
                    f->glEnableVertexAttribArray(3);
                    f->glEnableVertexAttribArray(4);
                    f->glEnableVertexAttribArray(5);
                    f->glEnableVertexAttribArray(6);
                    f->glEnableVertexAttribArray(7);
                    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ModelShaderVertex), 0);
                    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ModelShaderVertex), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ModelShaderVertex), reinterpret_cast<void*>(6 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(ModelShaderVertex), reinterpret_cast<void*>(9 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ModelShaderVertex), reinterpret_cast<void*>(11 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(ModelShaderVertex), reinterpret_cast<void*>(12 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(ModelShaderVertex), reinterpret_cast<void*>(13 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(ModelShaderVertex), reinterpret_cast<void*>(16 * sizeof(GLfloat)));
                    m_vboTriangle.release();
                }
                {
                    // Convert edge vertices from ModelShaderVertex to MonochromeOpenGLVertex
                    int edgeCount = m_mesh->edgeVertexCount();
                    if (edgeCount > 0) {
                        const ModelShaderVertex* src = m_mesh->edgeVertices();
                        std::vector<MonochromeOpenGLVertex> monochromeVertices(edgeCount);
                        for (int i = 0; i < edgeCount; ++i) {
                            monochromeVertices[i].posX = src[i].posX;
                            monochromeVertices[i].posY = src[i].posY;
                            monochromeVertices[i].posZ = src[i].posZ;
                            monochromeVertices[i].colorR = 0.1137f;
                            monochromeVertices[i].colorG = 0.098f;
                            monochromeVertices[i].colorB = 0.0667f;
                            monochromeVertices[i].alpha = 0.9f;
                        }
                        m_wireframeObject.update(monochromeVertices.data(), edgeCount);
                    } else {
                        m_wireframeObject.update(nullptr, 0);
                    }
                }
                if (m_toolEnabled) {
                    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vaoTool);
                    if (m_vboTool.isCreated())
                        m_vboTool.destroy();
                    m_vboTool.create();
                    m_vboTool.bind();
                    m_vboTool.allocate(m_mesh->toolVertices(), m_mesh->toolVertexCount() * sizeof(ModelShaderVertex));
                    m_renderToolVertexCount = m_mesh->toolVertexCount();
                    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
                    f->glEnableVertexAttribArray(0);
                    f->glEnableVertexAttribArray(1);
                    f->glEnableVertexAttribArray(2);
                    f->glEnableVertexAttribArray(3);
                    f->glEnableVertexAttribArray(4);
                    f->glEnableVertexAttribArray(5);
                    f->glEnableVertexAttribArray(6);
                    f->glEnableVertexAttribArray(7);
                    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ModelShaderVertex), 0);
                    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ModelShaderVertex), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ModelShaderVertex), reinterpret_cast<void*>(6 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(ModelShaderVertex), reinterpret_cast<void*>(9 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ModelShaderVertex), reinterpret_cast<void*>(11 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(ModelShaderVertex), reinterpret_cast<void*>(12 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(ModelShaderVertex), reinterpret_cast<void*>(13 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(ModelShaderVertex), reinterpret_cast<void*>(16 * sizeof(GLfloat)));
                    m_vboTool.release();
                } else {
                    m_renderToolVertexCount = 0;
                }
            } else {
                m_renderTriangleVertexCount = 0;
                m_renderToolVertexCount = 0;
            }
        }
    }
    program->setTextureIdValue(0);
    program->setNormalMapIdValue(1);
    program->setMetalnessRoughnessAoMapIdValue(2);
    if (m_renderTriangleVertexCount > 0) {
        QOpenGLVertexArrayObject::Binder vaoBinder(&m_vaoTriangle);
        QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
        if (m_hasTexture) {
            if (m_texture)
                m_texture->bind(0);
            program->setTextureEnabledValue(1);
        } else {
            program->setTextureEnabledValue(0);
        }
        if (m_hasNormalMap) {
            if (m_normalMap)
                m_normalMap->bind(1);
            program->setNormalMapEnabledValue(1);
        } else {
            program->setNormalMapEnabledValue(0);
        }
        if (m_hasMetalnessMap || m_hasRoughnessMap || m_hasAmbientOcclusionMap) {
            if (m_metalnessRoughnessAmbientOcclusionMap)
                m_metalnessRoughnessAmbientOcclusionMap->bind(2);
        }
        program->setMetalnessMapEnabledValue(m_hasMetalnessMap ? 1 : 0);
        program->setRoughnessMapEnabledValue(m_hasRoughnessMap ? 1 : 0);
        program->setAoMapEnabledValue(m_hasAmbientOcclusionMap ? 1 : 0);
        f->glDrawArrays(GL_TRIANGLES, 0, m_renderTriangleVertexCount);
    }
    if (m_toolEnabled) {
        if (m_renderToolVertexCount > 0) {
            QOpenGLVertexArrayObject::Binder vaoBinder(&m_vaoTool);
            QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
            program->setTextureEnabledValue(0);
            program->setNormalMapEnabledValue(0);
            program->setMetalnessMapEnabledValue(0);
            program->setRoughnessMapEnabledValue(0);
            program->setAoMapEnabledValue(0);
            f->glDrawArrays(GL_TRIANGLES, 0, m_renderToolVertexCount);
        }
    }
}

void ModelShaderMeshBinder::fetchCurrentToonNormalAndDepthMaps(QImage* normalMap, QImage* depthMap)
{
    QMutexLocker lock(&m_toonNormalAndDepthMapMutex);
    if (nullptr != normalMap && nullptr != m_currentToonNormalMap)
        *normalMap = *m_currentToonNormalMap;
    if (nullptr != depthMap && nullptr != m_currentToonDepthMap)
        *depthMap = *m_currentToonDepthMap;
}

void ModelShaderMeshBinder::updateToonNormalAndDepthMaps(QImage* normalMap, QImage* depthMap)
{
    QMutexLocker lock(&m_toonNormalAndDepthMapMutex);

    delete m_newToonNormalMap;
    m_newToonNormalMap = normalMap;

    delete m_newToonDepthMap;
    m_newToonDepthMap = depthMap;

    m_newToonMapsComing = true;
}

void ModelShaderMeshBinder::paintWireframe()
{
    if (!m_showWireframe)
        return;

    m_wireframeObject.draw();
}

void ModelShaderMeshBinder::cleanup()
{
    if (m_vboTriangle.isCreated())
        m_vboTriangle.destroy();
    m_wireframeObject.cleanup();
    if (m_toolEnabled) {
        if (m_vboTool.isCreated())
            m_vboTool.destroy();
    }
    delete m_texture;
    m_texture = nullptr;
    delete m_normalMap;
    m_normalMap = nullptr;
    delete m_metalnessRoughnessAmbientOcclusionMap;
    m_metalnessRoughnessAmbientOcclusionMap = nullptr;
    delete m_toonNormalMap;
    m_toonNormalMap = nullptr;
    delete m_toonDepthMap;
    m_toonDepthMap = nullptr;
}

void ModelShaderMeshBinder::showWireframe()
{
    m_showWireframe = true;
}

void ModelShaderMeshBinder::hideWireframe()
{
    m_showWireframe = false;
}

bool ModelShaderMeshBinder::isWireframeVisible()
{
    return m_showWireframe;
}
