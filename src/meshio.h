#ifndef AUTO_REMESHER_MESH_IO_H
#define AUTO_REMESHER_MESH_IO_H

#include <AutoRemesher/Vector3>
#include <QString>
#include <string>
#include <vector>

namespace MeshIO {

enum class Format {
    Unknown,
    OBJ,
    PLY,
    STL
};

Format detectFormat(const QString& filename);

bool loadMesh(const QString& filename,
    std::vector<AutoRemesher::Vector3>& vertices,
    std::vector<std::vector<size_t>>& faces,
    std::string& errorString);

bool saveMesh(const QString& filename,
    const std::vector<AutoRemesher::Vector3>& vertices,
    const std::vector<std::vector<size_t>>& faces,
    std::string& errorString,
    Format format = Format::Unknown);

bool loadOBJ(const QString& filename,
    std::vector<AutoRemesher::Vector3>& vertices,
    std::vector<std::vector<size_t>>& faces,
    std::string& errorString);

bool saveOBJ(const QString& filename,
    const std::vector<AutoRemesher::Vector3>& vertices,
    const std::vector<std::vector<size_t>>& faces,
    std::string& errorString);

bool loadSTL(const QString& filename,
    std::vector<AutoRemesher::Vector3>& vertices,
    std::vector<std::vector<size_t>>& faces,
    std::string& errorString);

bool saveSTL(const QString& filename,
    const std::vector<AutoRemesher::Vector3>& vertices,
    const std::vector<std::vector<size_t>>& faces,
    std::string& errorString,
    bool binary = true);

bool loadPLY(const QString& filename,
    std::vector<AutoRemesher::Vector3>& vertices,
    std::vector<std::vector<size_t>>& faces,
    std::string& errorString);

bool savePLY(const QString& filename,
    const std::vector<AutoRemesher::Vector3>& vertices,
    const std::vector<std::vector<size_t>>& faces,
    std::string& errorString,
    bool binary = false);

} // namespace MeshIO

#endif // AUTO_REMESHER_MESH_IO_H
