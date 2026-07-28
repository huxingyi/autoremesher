#ifndef AUTO_REMESHER_MESH_IO_H
#define AUTO_REMESHER_MESH_IO_H

#include <AutoRemesher/Vector3>
#include <QString>
#include <string>
#include <vector>

namespace MeshIO {

bool loadMesh(const QString& filename,
    std::vector<AutoRemesher::Vector3>& outVertices,
    std::vector<std::vector<size_t>>& outTriangles,
    std::string& outError);

bool saveMesh(const QString& filename,
    const std::vector<AutoRemesher::Vector3>& vertices,
    const std::vector<std::vector<size_t>>& quads,
    std::string& outError);

}

#endif
