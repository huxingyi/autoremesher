#ifndef AUTO_REMESHER_HEAT_MAP_GENERATOR_H
#define AUTO_REMESHER_HEAT_MAP_GENERATOR_H
#include <AutoRemesher/Vector3>
#include <vector>
#include <unordered_set>

namespace AutoRemesher
{
    
class HeatMapGenerator
{
public:
    HeatMapGenerator(const std::vector<Vector3> *vertices,
        const std::vector<std::vector<size_t>> *triangles);
    void generate(const std::unordered_set<size_t> &sourceVertices);
    const std::vector<double> &vertexHeatMap() const;
private:
    const std::vector<Vector3> *m_vertices = nullptr;
    const std::vector<std::vector<size_t>> *m_triangles = nullptr;
    std::vector<double> m_vertexHeatMap;
};
    
}

#endif
