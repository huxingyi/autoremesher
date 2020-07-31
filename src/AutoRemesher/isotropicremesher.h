#ifndef AUTO_REMESHER_ISOTROPIC_REMESHER_H
#define AUTO_REMESHER_ISOTROPIC_REMESHER_H

namespace AutoRemesher
{
    
class IsotropicRemesher
{
public:
    IsotropicRemesher(const std::vector<Vector3> &vertices,
            std::vector<std::vector<size_t>> &triangles) :
        m_vertices(vertices),
        m_triangles(triangles)
    {
    }
    void setTargetEdgeLength(double edgeLength)
    {
        m_targetEdgeLength = edgeLength;
    }
    void setIterations(int iterations)
    {
        m_iterations = iterations;
    }
    
    const std::vector<Vector3> &remeshedVertices()
    {
        return m_remeshedVertices;
    }
    
    const std::vector<std::vector<size_t>> &remeshedTriangles()
    {
        return m_remeshedTriangles;
    }
    
    bool remesh();
    
    void debugExportObj(const char *filename);
private:
    std::vector<Vector3> m_vertices;
    std::vector<std::vector<size_t>> m_triangles;
    double m_targetEdgeLength = 3.9;
    //double m_sharpEdgeDegrees = 60;
    int m_iterations = 3;
    std::vector<Vector3> m_remeshedVertices;
    std::vector<std::vector<size_t>> m_remeshedTriangles;
};
    
}

#endif
