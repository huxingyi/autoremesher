#ifndef AUTO_REMESHER_QUAD_REMESHER_H
#define AUTO_REMESHER_QUAD_REMESHER_H
#include <vector>
#include <cstddef>
#include <AutoRemesher/Vector3>

namespace AutoRemesher
{

class QuadRemesher
{
public:
    QuadRemesher(const std::vector<Vector3> &vertices,
            const std::vector<std::vector<size_t>> &triangles) :
        m_vertices(vertices),
        m_triangles(triangles)
    {
    }
    
    void setGradientSize(double gradientSize)
    {
        m_gradientSize = gradientSize;
    }
    
    const std::vector<Vector3> &remeshedVertices()
    {
        return m_remeshedVertices;
    }
    
    const std::vector<std::vector<size_t>> &remeshedQuads()
    {
        return m_remeshedQuads;
    }
    
    bool remesh();
private:
    std::vector<Vector3> m_vertices;
    std::vector<std::vector<size_t>> m_triangles;
    double m_gradientSize = 100.0;
    std::vector<Vector3> m_remeshedVertices;
    std::vector<std::vector<size_t>> m_remeshedQuads;
};

}

#endif
