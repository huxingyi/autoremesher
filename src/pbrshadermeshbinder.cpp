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
#include <QMutexLocker>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <map>
#include <QDebug>
#include <QDir>
#include <QSurfaceFormat>
#include "pbrshadermeshbinder.h"
#include "ddsfile.h"

PbrShaderMeshBinder::PbrShaderMeshBinder(bool toolEnabled) :
    m_toolEnabled(toolEnabled)
{
}

PbrShaderMeshBinder::~PbrShaderMeshBinder()
{
    delete m_mesh;
    delete m_newMesh;
    delete m_texture;
    delete m_normalMap;
    delete m_metalnessRoughnessAmbientOcclusionMap;
    delete m_environmentIrradianceMap;
    delete m_environmentSpecularMap;
    delete m_toonNormalMap;
    delete m_toonDepthMap;
    delete m_newToonNormalMap;
    delete m_newToonDepthMap;
    delete m_currentToonNormalMap;
    delete m_currentToonDepthMap;
}

void PbrShaderMeshBinder::updateMesh(PbrShaderMesh *mesh)
{
    QMutexLocker lock(&m_newMeshMutex);
    if (mesh != m_mesh) {
        delete m_newMesh;
        m_newMesh = mesh;
        m_newMeshComing = true;
    }
}

void PbrShaderMeshBinder::reloadMesh()
{
    PbrShaderMesh *mesh = nullptr;
    {
        QMutexLocker lock(&m_newMeshMutex);
        if (nullptr == m_mesh)
            return;
        mesh = new PbrShaderMesh(*m_mesh);
    }
    if (nullptr != mesh)
        updateMesh(mesh);
}

void PbrShaderMeshBinder::initialize()
{
    m_vaoTriangle.create();
    m_vaoEdge.create();
    if (m_toolEnabled)
        m_vaoTool.create();
}

void PbrShaderMeshBinder::enableEnvironmentLight()
{
    m_environmentLightEnabled = true;
}

bool PbrShaderMeshBinder::isEnvironmentLightEnabled()
{
    return m_environmentLightEnabled;
}

void PbrShaderMeshBinder::paint(PbrShaderProgram *program)
{
    PbrShaderMesh *newMesh = nullptr;
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
                if (nullptr != m_mesh->metalnessRoughnessAmbientOcclusionImage() &&
                        (m_hasMetalnessMap || m_hasRoughnessMap || m_hasAmbientOcclusionMap))
                    m_metalnessRoughnessAmbientOcclusionMap = new QOpenGLTexture(*m_mesh->metalnessRoughnessAmbientOcclusionImage());
                
                delete m_environmentIrradianceMap;
                m_environmentIrradianceMap = nullptr;
                delete m_environmentSpecularMap;
                m_environmentSpecularMap = nullptr;
                if (program->isCoreProfile() && 
                        m_environmentLightEnabled &&
                        (m_hasMetalnessMap || m_hasRoughnessMap)) {
                    DdsFileReader irradianceFile(":/resources/cedar_bridge_irradiance.dds");
                    m_environmentIrradianceMap = irradianceFile.createOpenGLTexture();
                    
                    DdsFileReader specularFile(":/resources/cedar_bridge_specular.dds");
                    m_environmentSpecularMap = specularFile.createOpenGLTexture();
                }
                
                {
                    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vaoTriangle);
                    if (m_vboTriangle.isCreated())
                        m_vboTriangle.destroy();
                    m_vboTriangle.create();
                    m_vboTriangle.bind();
                    m_vboTriangle.allocate(m_mesh->triangleVertices(), m_mesh->triangleVertexCount() * sizeof(PbrShaderVertex));
                    m_renderTriangleVertexCount = m_mesh->triangleVertexCount();
                    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
                    f->glEnableVertexAttribArray(0);
                    f->glEnableVertexAttribArray(1);
                    f->glEnableVertexAttribArray(2);
                    f->glEnableVertexAttribArray(3);
                    f->glEnableVertexAttribArray(4);
                    f->glEnableVertexAttribArray(5);
                    f->glEnableVertexAttribArray(6);
                    f->glEnableVertexAttribArray(7);
                    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), 0);
                    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(3 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(6 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(9 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(11 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(12 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(13 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(16 * sizeof(GLfloat)));
                    m_vboTriangle.release();
                }
                {
                    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vaoEdge);
                    if (m_vboEdge.isCreated())
                        m_vboEdge.destroy();
                    m_vboEdge.create();
                    m_vboEdge.bind();
                    m_vboEdge.allocate(m_mesh->edgeVertices(), m_mesh->edgeVertexCount() * sizeof(PbrShaderVertex));
                    m_renderEdgeVertexCount = m_mesh->edgeVertexCount();
                    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
                    f->glEnableVertexAttribArray(0);
                    f->glEnableVertexAttribArray(1);
                    f->glEnableVertexAttribArray(2);
                    f->glEnableVertexAttribArray(3);
                    f->glEnableVertexAttribArray(4);
                    f->glEnableVertexAttribArray(5);
                    f->glEnableVertexAttribArray(6);
                    f->glEnableVertexAttribArray(7);
                    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), 0);
                    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(3 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(6 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(9 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(11 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(12 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(13 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(16 * sizeof(GLfloat)));
                    m_vboEdge.release();
                }
                if (m_toolEnabled) {
                    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vaoTool);
                    if (m_vboTool.isCreated())
                        m_vboTool.destroy();
                    m_vboTool.create();
                    m_vboTool.bind();
                    m_vboTool.allocate(m_mesh->toolVertices(), m_mesh->toolVertexCount() * sizeof(PbrShaderVertex));
                    m_renderToolVertexCount = m_mesh->toolVertexCount();
                    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
                    f->glEnableVertexAttribArray(0);
                    f->glEnableVertexAttribArray(1);
                    f->glEnableVertexAttribArray(2);
                    f->glEnableVertexAttribArray(3);
                    f->glEnableVertexAttribArray(4);
                    f->glEnableVertexAttribArray(5);
                    f->glEnableVertexAttribArray(6);
                    f->glEnableVertexAttribArray(7);
                    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), 0);
                    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(3 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(6 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(9 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(11 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(12 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(13 * sizeof(GLfloat)));
                    f->glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(PbrShaderVertex), reinterpret_cast<void *>(16 * sizeof(GLfloat)));
                    m_vboTool.release();
                } else {
                    m_renderToolVertexCount = 0;
                }
            } else {
                m_renderTriangleVertexCount = 0;
                m_renderEdgeVertexCount = 0;
                m_renderToolVertexCount = 0;
            }
        }
    }
    program->setTextureIdValue(0);
    program->setNormalMapIdValue(1);
    program->setMetalnessRoughnessAmbientOcclusionMapIdValue(2);
    program->setEnvironmentIrradianceMapIdValue(3);
    program->setEnvironmentSpecularMapIdValue(4);
    if (m_showWireframe) {
        if (m_renderEdgeVertexCount > 0) {
            QOpenGLVertexArrayObject::Binder vaoBinder(&m_vaoEdge);
			QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
            // glDrawArrays GL_LINES crashs on Mesa GL
            if (program->isCoreProfile()) {
                program->setTextureEnabledValue(0);
                program->setNormalMapEnabledValue(0);
                program->setMetalnessMapEnabledValue(0);
                program->setRoughnessMapEnabledValue(0);
                program->setAmbientOcclusionMapEnabledValue(0);
                if (program->isCoreProfile()) {
                    program->setEnvironmentIrradianceMapEnabledValue(0);
                    program->setEnvironmentSpecularMapEnabledValue(0);
                }
                f->glDrawArrays(GL_LINES, 0, m_renderEdgeVertexCount);
            }
        }
    }
    if (m_renderTriangleVertexCount > 0) {
        QOpenGLVertexArrayObject::Binder vaoBinder(&m_vaoTriangle);
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
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
        program->setAmbientOcclusionMapEnabledValue(m_hasAmbientOcclusionMap ? 1 : 0);
        if (program->isCoreProfile()) {
            if (nullptr != m_environmentIrradianceMap) {
                m_environmentIrradianceMap->bind(3);
                program->setEnvironmentIrradianceMapEnabledValue(1);
            } else {
                program->setEnvironmentIrradianceMapEnabledValue(0);
            }
            if (nullptr != m_environmentSpecularMap) {
                m_environmentSpecularMap->bind(4);
                program->setEnvironmentSpecularMapEnabledValue(1);
            } else {
                program->setEnvironmentSpecularMapEnabledValue(0);
            }
        }
        f->glDrawArrays(GL_TRIANGLES, 0, m_renderTriangleVertexCount);
    }
    if (m_toolEnabled) {
        if (m_renderToolVertexCount > 0) {
            QOpenGLVertexArrayObject::Binder vaoBinder(&m_vaoTool);
            QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
            program->setTextureEnabledValue(0);
            program->setNormalMapEnabledValue(0);
            program->setMetalnessMapEnabledValue(0);
            program->setRoughnessMapEnabledValue(0);
            program->setAmbientOcclusionMapEnabledValue(0);
            if (program->isCoreProfile()) {
                program->setEnvironmentIrradianceMapEnabledValue(0);
                program->setEnvironmentSpecularMapEnabledValue(0);
            }
            f->glDrawArrays(GL_TRIANGLES, 0, m_renderToolVertexCount);
        }
    }
}

void PbrShaderMeshBinder::fetchCurrentToonNormalAndDepthMaps(QImage *normalMap, QImage *depthMap)
{
    QMutexLocker lock(&m_toonNormalAndDepthMapMutex);
    if (nullptr != normalMap && nullptr != m_currentToonNormalMap)
        *normalMap = *m_currentToonNormalMap;
    if (nullptr != depthMap && nullptr != m_currentToonDepthMap)
        *depthMap = *m_currentToonDepthMap;
}

void PbrShaderMeshBinder::updateToonNormalAndDepthMaps(QImage *normalMap, QImage *depthMap)
{
    QMutexLocker lock(&m_toonNormalAndDepthMapMutex);
    
    delete m_newToonNormalMap;
    m_newToonNormalMap = normalMap;
    
    delete m_newToonDepthMap;
    m_newToonDepthMap = depthMap;
    
    m_newToonMapsComing = true;
}

void PbrShaderMeshBinder::cleanup()
{
    if (m_vboTriangle.isCreated())
        m_vboTriangle.destroy();
    if (m_vboEdge.isCreated())
        m_vboEdge.destroy();
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
    delete m_environmentIrradianceMap;
    m_environmentIrradianceMap = nullptr;
    delete m_environmentSpecularMap;
    m_environmentSpecularMap = nullptr;
    delete m_toonNormalMap;
    m_toonNormalMap = nullptr;
    delete m_toonDepthMap;
    m_toonDepthMap = nullptr;
}

void PbrShaderMeshBinder::showWireframe()
{
    m_showWireframe = true;
}

void PbrShaderMeshBinder::hideWireframe()
{
    m_showWireframe = false;
}

bool PbrShaderMeshBinder::isWireframeVisible()
{
    return m_showWireframe;
}
