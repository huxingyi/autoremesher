#ifndef AUTO_REMESHER_DECIMATOR_H
#define AUTO_REMESHER_DECIMATOR_H
#include <vector>
#include <cstddef>
#include <AutoRemesher/Vector3>
#include <map>
#include <queue>

namespace AutoRemesher
{

class Decimator
{
public:
    Decimator(const std::vector<Vector3> &vertices,
            std::vector<std::vector<size_t>> &triangles) :
        m_vertices(vertices),
        m_triangles(triangles)
    {
    }
    bool step();
private:
    
    std::vector<Vector3> m_vertices;
    std::vector<std::vector<size_t>> m_triangles;
};

}

#endif