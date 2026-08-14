#include "monochromeopenglobject.h"
#include "openglbufferutil.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions>

void MonochromeOpenGLObject::update(const MonochromeOpenGLVertex* vertices, int vertexCount,
    const uint32_t* indices, int indexCount)
{
    QMutexLocker lock(&m_meshMutex);
    if (vertexCount > 0 && nullptr != vertices) {
        m_vertices = std::make_unique<MonochromeOpenGLVertex[]>(vertexCount);
        memcpy(m_vertices.get(), vertices, vertexCount * sizeof(MonochromeOpenGLVertex));
    } else {
        m_vertices = nullptr;
        vertexCount = 0;
    }
    if (indexCount > 0 && nullptr != indices && vertexCount > 0) {
        m_indices = std::make_unique<uint32_t[]>(indexCount);
        memcpy(m_indices.get(), indices, indexCount * sizeof(uint32_t));
    } else {
        m_indices = nullptr;
        indexCount = 0;
    }
    m_meshVertexCount = vertexCount;
    m_meshIndexCount = indexCount;
    m_meshIsDirty = true;
}

void MonochromeOpenGLObject::draw()
{
    copyMeshToOpenGL();
    if (0 == m_meshVertexCount)
        return;
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    QOpenGLVertexArrayObject::Binder binder(&m_vertexArrayObject);
    if (m_meshIndexCount > 0)
        f->glDrawElements(GL_LINES, m_meshIndexCount, GL_UNSIGNED_INT, nullptr);
    else
        f->glDrawArrays(GL_LINES, 0, m_meshVertexCount);
}

void MonochromeOpenGLObject::copyMeshToOpenGL()
{
    if (!m_meshIsDirty)
        return;
    m_meshIsDirty = false;
    if (m_meshVertexCount > 0 && m_vertices) {
        QOpenGLVertexArrayObject::Binder binder(&m_vertexArrayObject);
        if (m_buffer.isCreated())
            m_buffer.destroy();
        m_buffer.create();
        m_buffer.bind();
        if (!allocateOpenGLBuffer(m_buffer, m_vertices.get(),
                (size_t)m_meshVertexCount * sizeof(MonochromeOpenGLVertex))) {
            m_meshVertexCount = 0;
            m_meshIndexCount = 0;
            return;
        }
        QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
        f->glEnableVertexAttribArray(0);
        f->glEnableVertexAttribArray(1);
        f->glEnableVertexAttribArray(2);
        f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MonochromeOpenGLVertex), 0);
        f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MonochromeOpenGLVertex),
            reinterpret_cast<void*>(3 * sizeof(GLfloat)));
        f->glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(MonochromeOpenGLVertex),
            reinterpret_cast<void*>(6 * sizeof(GLfloat)));
        m_buffer.release();
        if (m_meshIndexCount > 0 && m_indices) {
            if (m_indexBuffer.isCreated())
                m_indexBuffer.destroy();
            m_indexBuffer.create();
            m_indexBuffer.bind();
            if (!allocateOpenGLBuffer(m_indexBuffer, m_indices.get(),
                    (size_t)m_meshIndexCount * sizeof(uint32_t))) {
                m_meshVertexCount = 0;
                m_meshIndexCount = 0;
            }
            // The element array binding belongs to the vertex array object, so
            // leave it bound and let the binder take it down with the object
        }
    }
}

void MonochromeOpenGLObject::cleanup()
{
    QMutexLocker lock(&m_meshMutex);
    m_vertices = nullptr;
    m_indices = nullptr;
    m_meshVertexCount = 0;
    m_meshIndexCount = 0;
    m_meshIsDirty = false;
    if (m_buffer.isCreated())
        m_buffer.destroy();
    if (m_indexBuffer.isCreated())
        m_indexBuffer.destroy();
}
