#ifndef AUTO_REMESHER_H
#define AUTO_REMESHER_H
#include <vector>

namespace autoremesher
{

struct Vector3
{
    float x;
    float y;
    float z;
};

bool remesh(const std::vector<Vector3> &inputVertices,
    std::vector<std::vector<size_t>> &inputTriangles,
    std::vector<std::vector<size_t>> &inputQuads,
    std::vector<Vector3> *outputVertices,
    std::vector<std::vector<size_t>> *outputQuads,
    double gradientSize);

}

#endif
