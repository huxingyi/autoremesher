#include "meshio.h"
#include <QByteArray>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "tiny_obj_loader.h"

namespace MeshIO {

Format detectFormat(const QString& filename)
{
    QString ext = QFileInfo(filename).suffix().toLower();
    if (ext == "obj")
        return Format::OBJ;
    if (ext == "ply")
        return Format::PLY;
    if (ext == "stl")
        return Format::STL;
    return Format::Unknown;
}

namespace {

struct VertexKey {
    int64_t x, y, z;
    bool operator==(const VertexKey& o) const
    {
        return x == o.x && y == o.y && z == o.z;
    }
};

struct VertexKeyHash {
    size_t operator()(const VertexKey& k) const
    {
        size_t h1 = std::hash<int64_t>()(k.x);
        size_t h2 = std::hash<int64_t>()(k.y);
        size_t h3 = std::hash<int64_t>()(k.z);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

inline VertexKey makeKey(double x, double y, double z, double scale = 1e5)
{
    return VertexKey {
        static_cast<int64_t>(std::round(x * scale)),
        static_cast<int64_t>(std::round(y * scale)),
        static_cast<int64_t>(std::round(z * scale))
    };
}

} // namespace

bool loadOBJ(const QString& filename,
    std::vector<AutoRemesher::Vector3>& vertices,
    std::vector<std::vector<size_t>>& faces,
    std::string& errorString)
{
    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool success = tinyobj::LoadObj(&attributes, &shapes, &materials, &warn, &err,
        filename.toUtf8().constData());
    if (!success) {
        errorString = err.empty() ? "Failed to parse OBJ file" : err;
        return false;
    }

    vertices.clear();
    faces.clear();

    vertices.resize(attributes.vertices.size() / 3);
    for (size_t i = 0, j = 0; i < vertices.size(); ++i) {
        double vx = attributes.vertices[j++];
        double vy = attributes.vertices[j++];
        double vz = attributes.vertices[j++];
        vertices[i] = AutoRemesher::Vector3(vx, vy, vz);
    }

    for (const auto& shape : shapes) {
        size_t indexOffset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
            size_t fv = shape.mesh.num_face_vertices[f];
            std::vector<size_t> face;
            face.reserve(fv);
            for (size_t v = 0; v < fv; ++v) {
                face.push_back(static_cast<size_t>(shape.mesh.indices[indexOffset + v].vertex_index));
            }
            indexOffset += fv;
            if (face.size() >= 3)
                faces.push_back(face);
        }
    }

    return true;
}

bool saveOBJ(const QString& filename,
    const std::vector<AutoRemesher::Vector3>& vertices,
    const std::vector<std::vector<size_t>>& faces,
    std::string& errorString)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        errorString = file.errorString().toStdString();
        return false;
    }

    QTextStream out(&file);
    out << "# AutoRemesher export\n";
    for (const auto& v : vertices) {
        out << "v " << v.x() << " " << v.y() << " " << v.z() << "\n";
    }
    for (const auto& f : faces) {
        out << "f";
        for (size_t idx : f) {
            out << " " << (idx + 1);
        }
        out << "\n";
    }

    return true;
}

bool loadSTL(const QString& filename,
    std::vector<AutoRemesher::Vector3>& vertices,
    std::vector<std::vector<size_t>>& faces,
    std::string& errorString)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        errorString = file.errorString().toStdString();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    if (data.size() < 84) {
        errorString = "STL file is too small";
        return false;
    }

    vertices.clear();
    faces.clear();

    std::unordered_map<VertexKey, size_t, VertexKeyHash> vertexMap;
    auto getOrAddVertex = [&](double x, double y, double z) -> size_t {
        VertexKey key = makeKey(x, y, z);
        auto it = vertexMap.find(key);
        if (it != vertexMap.end())
            return it->second;
        size_t newIdx = vertices.size();
        vertices.emplace_back(x, y, z);
        vertexMap[key] = newIdx;
        return newIdx;
    };

    // Check if binary STL
    uint32_t numTriangles = 0;
    std::memcpy(&numTriangles, data.constData() + 80, sizeof(uint32_t));
    bool isBinary = (data.size() == static_cast<qint64>(84 + static_cast<uint64_t>(numTriangles) * 50));

    if (isBinary) {
        const char* ptr = data.constData() + 84;
        faces.reserve(numTriangles);
        for (uint32_t i = 0; i < numTriangles; ++i) {
            // skip normal (12 bytes)
            ptr += 12;
            float v[9];
            std::memcpy(v, ptr, 36);
            ptr += 36;
            ptr += 2; // skip attribute byte count

            size_t i0 = getOrAddVertex(v[0], v[1], v[2]);
            size_t i1 = getOrAddVertex(v[3], v[4], v[5]);
            size_t i2 = getOrAddVertex(v[6], v[7], v[8]);
            if (i0 != i1 && i1 != i2 && i0 != i2) {
                faces.push_back({ i0, i1, i2 });
            }
        }
        return true;
    }

    // ASCII STL parser fallback
    QString textData = QString::fromUtf8(data);
    QStringList lines = textData.split('\n');
    std::vector<size_t> currentFace;
    for (const QString& line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.startsWith("vertex", Qt::CaseInsensitive)) {
            QStringList parts = trimmed.simplified().split(' ');
            if (parts.size() >= 4) {
                double vx = parts[1].toDouble();
                double vy = parts[2].toDouble();
                double vz = parts[3].toDouble();
                currentFace.push_back(getOrAddVertex(vx, vy, vz));
            }
        } else if (trimmed.startsWith("endfacet", Qt::CaseInsensitive)) {
            if (currentFace.size() == 3) {
                if (currentFace[0] != currentFace[1] && currentFace[1] != currentFace[2] && currentFace[0] != currentFace[2]) {
                    faces.push_back(currentFace);
                }
            }
            currentFace.clear();
        }
    }

    return !faces.empty();
}

bool saveSTL(const QString& filename,
    const std::vector<AutoRemesher::Vector3>& vertices,
    const std::vector<std::vector<size_t>>& faces,
    std::string& errorString,
    bool binary)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        errorString = file.errorString().toStdString();
        return false;
    }

    // Triangulate quads/polygons for STL
    struct Triangle {
        AutoRemesher::Vector3 p0, p1, p2;
        AutoRemesher::Vector3 normal;
    };
    std::vector<Triangle> triangles;
    for (const auto& face : faces) {
        if (face.size() < 3)
            continue;
        for (size_t i = 1; i + 1 < face.size(); ++i) {
            const auto& p0 = vertices[face[0]];
            const auto& p1 = vertices[face[i]];
            const auto& p2 = vertices[face[i + 1]];
            AutoRemesher::Vector3 n = AutoRemesher::Vector3::normal(p0, p1, p2);
            triangles.push_back({ p0, p1, p2, n });
        }
    }

    if (binary) {
        char header[80] = "AutoRemesher binary STL export";
        file.write(header, 80);
        uint32_t count = static_cast<uint32_t>(triangles.size());
        file.write(reinterpret_cast<const char*>(&count), 4);
        for (const auto& tri : triangles) {
            float buffer[12] = {
                static_cast<float>(tri.normal.x()), static_cast<float>(tri.normal.y()), static_cast<float>(tri.normal.z()),
                static_cast<float>(tri.p0.x()), static_cast<float>(tri.p0.y()), static_cast<float>(tri.p0.z()),
                static_cast<float>(tri.p1.x()), static_cast<float>(tri.p1.y()), static_cast<float>(tri.p1.z()),
                static_cast<float>(tri.p2.x()), static_cast<float>(tri.p2.y()), static_cast<float>(tri.p2.z())
            };
            file.write(reinterpret_cast<const char*>(buffer), 48);
            uint16_t attr = 0;
            file.write(reinterpret_cast<const char*>(&attr), 2);
        }
    } else {
        QTextStream out(&file);
        out << "solid AutoRemesher\n";
        for (const auto& tri : triangles) {
            out << "  facet normal " << tri.normal.x() << " " << tri.normal.y() << " " << tri.normal.z() << "\n";
            out << "    outer loop\n";
            out << "      vertex " << tri.p0.x() << " " << tri.p0.y() << " " << tri.p0.z() << "\n";
            out << "      vertex " << tri.p1.x() << " " << tri.p1.y() << " " << tri.p1.z() << "\n";
            out << "      vertex " << tri.p2.x() << " " << tri.p2.y() << " " << tri.p2.z() << "\n";
            out << "    endloop\n";
            out << "  endfacet\n";
        }
        out << "endsolid AutoRemesher\n";
    }

    return true;
}

bool loadPLY(const QString& filename,
    std::vector<AutoRemesher::Vector3>& vertices,
    std::vector<std::vector<size_t>>& faces,
    std::string& errorString)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        errorString = file.errorString().toStdString();
        return false;
    }

    // Read header
    bool isBinary = false;
    size_t numVertices = 0;
    size_t numFaces = 0;
    std::vector<std::string> vertexProperties;

    QByteArray lineBytes;
    while (!file.atEnd()) {
        lineBytes = file.readLine();
        QString line = QString::fromUtf8(lineBytes).trimmed();
        if (line == "end_header")
            break;
        QStringList parts = line.simplified().split(' ');
        if (parts.empty() || parts[0].isEmpty())
            continue;
        if (parts[0] == "format") {
            if (parts.size() >= 2 && parts[1].startsWith("binary"))
                isBinary = true;
        } else if (parts[0] == "element") {
            if (parts.size() >= 3 && parts[1] == "vertex")
                numVertices = parts[2].toULongLong();
            else if (parts.size() >= 3 && parts[1] == "face")
                numFaces = parts[2].toULongLong();
        } else if (parts[0] == "property") {
            if (parts.size() >= 3)
                vertexProperties.push_back(parts[2].toStdString());
        }
    }

    vertices.clear();
    faces.clear();
    vertices.reserve(numVertices);
    faces.reserve(numFaces);

    if (isBinary) {
        // Binary little endian
        for (size_t i = 0; i < numVertices; ++i) {
            float x = 0, y = 0, z = 0;
            file.read(reinterpret_cast<char*>(&x), sizeof(float));
            file.read(reinterpret_cast<char*>(&y), sizeof(float));
            file.read(reinterpret_cast<char*>(&z), sizeof(float));
            vertices.emplace_back(x, y, z);
        }
        for (size_t i = 0; i < numFaces; ++i) {
            uint8_t count = 0;
            file.read(reinterpret_cast<char*>(&count), sizeof(uint8_t));
            std::vector<size_t> face;
            face.reserve(count);
            for (uint8_t c = 0; c < count; ++c) {
                int32_t idx = 0;
                file.read(reinterpret_cast<char*>(&idx), sizeof(int32_t));
                face.push_back(static_cast<size_t>(idx));
            }
            if (face.size() >= 3)
                faces.push_back(face);
        }
        return true;
    }

    // ASCII PLY
    for (size_t i = 0; i < numVertices && !file.atEnd(); ++i) {
        QString line = QString::fromUtf8(file.readLine()).trimmed();
        QStringList parts = line.simplified().split(' ');
        if (parts.size() >= 3) {
            vertices.emplace_back(parts[0].toDouble(), parts[1].toDouble(), parts[2].toDouble());
        }
    }
    for (size_t i = 0; i < numFaces && !file.atEnd(); ++i) {
        QString line = QString::fromUtf8(file.readLine()).trimmed();
        QStringList parts = line.simplified().split(' ');
        if (!parts.empty() && !parts[0].isEmpty()) {
            int count = parts[0].toInt();
            if (parts.size() >= count + 1) {
                std::vector<size_t> face;
                face.reserve(count);
                for (int j = 1; j <= count; ++j) {
                    face.push_back(static_cast<size_t>(parts[j].toLongLong()));
                }
                if (face.size() >= 3)
                    faces.push_back(face);
            }
        }
    }

    return true;
}

bool savePLY(const QString& filename,
    const std::vector<AutoRemesher::Vector3>& vertices,
    const std::vector<std::vector<size_t>>& faces,
    std::string& errorString,
    bool binary)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | (binary ? QIODevice::Truncate : QIODevice::Text))) {
        errorString = file.errorString().toStdString();
        return false;
    }

    QTextStream out(&file);
    out << "ply\n";
    out << (binary ? "format binary_little_endian 1.0\n" : "format ascii 1.0\n");
    out << "comment AutoRemesher PLY export\n";
    out << "element vertex " << vertices.size() << "\n";
    out << "property float x\n";
    out << "property float y\n";
    out << "property float z\n";
    out << "element face " << faces.size() << "\n";
    out << "property list uchar int vertex_indices\n";
    out << "end_header\n";
    out.flush();

    if (binary) {
        for (const auto& v : vertices) {
            float coords[3] = { static_cast<float>(v.x()), static_cast<float>(v.y()), static_cast<float>(v.z()) };
            file.write(reinterpret_cast<const char*>(coords), 12);
        }
        for (const auto& face : faces) {
            uint8_t count = static_cast<uint8_t>(face.size());
            file.write(reinterpret_cast<const char*>(&count), 1);
            for (size_t idx : face) {
                int32_t val = static_cast<int32_t>(idx);
                file.write(reinterpret_cast<const char*>(&val), 4);
            }
        }
    } else {
        for (const auto& v : vertices) {
            out << v.x() << " " << v.y() << " " << v.z() << "\n";
        }
        for (const auto& face : faces) {
            out << face.size();
            for (size_t idx : face) {
                out << " " << idx;
            }
            out << "\n";
        }
    }

    return true;
}

bool loadMesh(const QString& filename,
    std::vector<AutoRemesher::Vector3>& vertices,
    std::vector<std::vector<size_t>>& faces,
    std::string& errorString)
{
    Format format = detectFormat(filename);
    switch (format) {
    case Format::OBJ:
        return loadOBJ(filename, vertices, faces, errorString);
    case Format::STL:
        return loadSTL(filename, vertices, faces, errorString);
    case Format::PLY:
        return loadPLY(filename, vertices, faces, errorString);
    default:
        // Try OBJ first, then STL, then PLY
        if (loadOBJ(filename, vertices, faces, errorString))
            return true;
        if (loadSTL(filename, vertices, faces, errorString))
            return true;
        if (loadPLY(filename, vertices, faces, errorString))
            return true;
        errorString = "Unsupported or unrecognized mesh file format: " + filename.toStdString();
        return false;
    }
}

bool saveMesh(const QString& filename,
    const std::vector<AutoRemesher::Vector3>& vertices,
    const std::vector<std::vector<size_t>>& faces,
    std::string& errorString,
    Format format)
{
    if (format == Format::Unknown)
        format = detectFormat(filename);

    switch (format) {
    case Format::STL:
        return saveSTL(filename, vertices, faces, errorString, true);
    case Format::PLY:
        return savePLY(filename, vertices, faces, errorString, false);
    case Format::OBJ:
    default:
        return saveOBJ(filename, vertices, faces, errorString);
    }
}

} // namespace MeshIO
