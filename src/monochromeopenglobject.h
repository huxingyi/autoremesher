#ifndef AUTO_REMESHER_MONOCHROME_OPENGL_OBJECT_H
#define AUTO_REMESHER_MONOCHROME_OPENGL_OBJECT_H
#include "monochromeopenglvertex.h"
#include <QMutex>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <memory>

class MonochromeOpenGLObject {
public:
    void update(const MonochromeOpenGLVertex *vertices, int vertexCount);
    void draw();
    void cleanup();
    int vertexCount() const { return m_meshVertexCount; }
private:
    void copyMeshToOpenGL();
    QOpenGLVertexArrayObject m_vertexArrayObject;
    QOpenGLBuffer m_buffer;
    std::unique_ptr<MonochromeOpenGLVertex[]> m_vertices;
    int m_meshVertexCount = 0;
    bool m_meshIsDirty = false;
    QMutex m_meshMutex;
};

#endif