#include "monochromeopenglobject.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions>

void MonochromeOpenGLObject::update(const MonochromeOpenGLVertex *vertices, int vertexCount)
{
    QMutexLocker lock(&m_meshMutex);
    if (vertexCount > 0 && nullptr != vertices) {
        m_vertices = std::make_unique<MonochromeOpenGLVertex[]>(vertexCount);
        memcpy(m_vertices.get(), vertices, vertexCount * sizeof(MonochromeOpenGLVertex));
    } else {
        m_vertices = nullptr;
    }
    m_meshVertexCount = vertexCount;
    m_meshIsDirty = true;
}

void MonochromeOpenGLObject::draw()
{
    copyMeshToOpenGL();
    if (0 == m_meshVertexCount)
        return;
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLVertexArrayObject::Binder binder(&m_vertexArrayObject);
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
        m_buffer.allocate(m_vertices.get(), m_meshVertexCount * sizeof(MonochromeOpenGLVertex));
        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
        f->glEnableVertexAttribArray(0);
        f->glEnableVertexAttribArray(1);
        f->glEnableVertexAttribArray(2);
        f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MonochromeOpenGLVertex), 0);
        f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MonochromeOpenGLVertex),
            reinterpret_cast<void *>(3 * sizeof(GLfloat)));
        f->glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(MonochromeOpenGLVertex),
            reinterpret_cast<void *>(6 * sizeof(GLfloat)));
        m_buffer.release();
    }
}

void MonochromeOpenGLObject::cleanup()
{
    QMutexLocker lock(&m_meshMutex);
    m_vertices = nullptr;
    m_meshVertexCount = 0;
    m_meshIsDirty = false;
    if (m_buffer.isCreated())
        m_buffer.destroy();
}