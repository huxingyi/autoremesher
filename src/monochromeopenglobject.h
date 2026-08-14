#ifndef AUTO_REMESHER_MONOCHROME_OPENGL_OBJECT_H
#define AUTO_REMESHER_MONOCHROME_OPENGL_OBJECT_H
#include "monochromeopenglvertex.h"
#include <QMutex>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <cstdint>
#include <memory>

class MonochromeOpenGLObject {
public:
    // Pass indices to draw shared vertices with glDrawElements, which is what
    // large models need: expanding every line into its own pair of vertices
    // overflows what a single buffer can hold. Without indices the vertices are
    // drawn in order, two per line.
    void update(const MonochromeOpenGLVertex* vertices, int vertexCount,
        const uint32_t* indices = nullptr, int indexCount = 0);
    void draw();
    void cleanup();
    int vertexCount() const { return m_meshVertexCount; }

private:
    void copyMeshToOpenGL();
    QOpenGLVertexArrayObject m_vertexArrayObject;
    QOpenGLBuffer m_buffer;
    QOpenGLBuffer m_indexBuffer { QOpenGLBuffer::IndexBuffer };
    std::unique_ptr<MonochromeOpenGLVertex[]> m_vertices;
    std::unique_ptr<uint32_t[]> m_indices;
    int m_meshVertexCount = 0;
    int m_meshIndexCount = 0;
    bool m_meshIsDirty = false;
    QMutex m_meshMutex;
};

#endif
