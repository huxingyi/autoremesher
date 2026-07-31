// meshio.cpp — MeshIO: load/save OBJ, FBX (read), GLTF/GLB (read+write)
#include "meshio.h"
#include "tiny_obj_loader.h"
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QDataStream>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <cstring>
#include <cmath>

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "ufbx.h"


namespace MeshIO {

// ──────────────────────────────────────────────────────────
//  LOAD helpers
// ──────────────────────────────────────────────────────────

static bool loadObjFile(const QString& filename,
    std::vector<AutoRemesher::Vector3>& outVertices,
    std::vector<std::vector<size_t>>& outTriangles,
    std::string& outError)
{
    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool success = tinyobj::LoadObj(&attributes, &shapes, &materials, &warn, &err, filename.toUtf8().constData());
    if (!err.empty()) {
        outError = err;
    }
    if (!success) {
        if (outError.empty()) outError = "Failed to parse OBJ file.";
        return false;
    }

    outVertices.resize(attributes.vertices.size() / 3);
    for (size_t i = 0, j = 0; i < outVertices.size(); ++i) {
        auto& dest = outVertices[i];
        dest.setX(attributes.vertices[j++]);
        dest.setY(attributes.vertices[j++]);
        dest.setZ(attributes.vertices[j++]);
    }

    outTriangles.clear();
    for (const auto& shape : shapes) {
        for (size_t i = 0; i < shape.mesh.indices.size(); i += 3) {
            outTriangles.push_back(std::vector<size_t> {
                (size_t)shape.mesh.indices[i + 0].vertex_index,
                (size_t)shape.mesh.indices[i + 1].vertex_index,
                (size_t)shape.mesh.indices[i + 2].vertex_index });
        }
    }

    return true;
}

static bool loadFbxFile(const QString& filename,
    std::vector<AutoRemesher::Vector3>& outVertices,
    std::vector<std::vector<size_t>>& outTriangles,
    std::string& outError)
{
    ufbx_load_opts opts = { 0 };
    ufbx_error error;
    ufbx_scene* scene = ufbx_load_file(filename.toUtf8().constData(), &opts, &error);
    if (!scene) {
        outError = std::string(error.description.data, error.description.length);
        return false;
    }

    outVertices.clear();
    outTriangles.clear();

    for (size_t i = 0; i < scene->meshes.count; ++i) {
        ufbx_mesh* mesh = scene->meshes.data[i];
        size_t vOffset = outVertices.size();

        for (size_t v = 0; v < mesh->num_vertices; ++v) {
            ufbx_vec3 pos = mesh->vertices[v];
            outVertices.push_back(AutoRemesher::Vector3(pos.x, pos.y, pos.z));
        }

        for (size_t f = 0; f < mesh->num_faces; ++f) {
            ufbx_face face = mesh->faces[f];
            if (face.num_indices < 3) continue;

            for (size_t t = 0; t < face.num_indices - 2; ++t) {
                size_t i0 = mesh->vertex_indices[face.index_begin + 0];
                size_t i1 = mesh->vertex_indices[face.index_begin + t + 1];
                size_t i2 = mesh->vertex_indices[face.index_begin + t + 2];
                outTriangles.push_back({ vOffset + i0, vOffset + i1, vOffset + i2 });
            }

        }
    }

    ufbx_free_scene(scene);

    if (outVertices.empty()) {
        outError = "FBX scene contains no valid mesh geometry.";
        return false;
    }

    return true;
}

static bool loadGltfFile(const QString& filename,
    std::vector<AutoRemesher::Vector3>& outVertices,
    std::vector<std::vector<size_t>>& outTriangles,
    std::string& outError)
{
    cgltf_options options = { (cgltf_file_type)0 };
    cgltf_data* data = NULL;

    cgltf_result result = cgltf_parse_file(&options, filename.toUtf8().constData(), &data);
    if (result != cgltf_result_success) {
        outError = "Failed to parse glTF/GLB file.";
        return false;
    }

    result = cgltf_load_buffers(&options, data, filename.toUtf8().constData());
    if (result != cgltf_result_success) {
        outError = "Failed to load glTF/GLB buffers.";
        cgltf_free(data);
        return false;
    }

    outVertices.clear();
    outTriangles.clear();

    for (size_t m = 0; m < data->meshes_count; ++m) {
        cgltf_mesh* mesh = &data->meshes[m];
        for (size_t p = 0; p < mesh->primitives_count; ++p) {
            cgltf_primitive* prim = &mesh->primitives[p];
            size_t vOffset = outVertices.size();

            for (size_t a = 0; a < prim->attributes_count; ++a) {
                if (prim->attributes[a].type == cgltf_attribute_type_position) {
                    cgltf_accessor* acc = prim->attributes[a].data;
                    for (size_t i = 0; i < acc->count; ++i) {
                        float pos[3] = { 0.0f, 0.0f, 0.0f };
                        cgltf_accessor_read_float(acc, i, pos, 3);
                        outVertices.push_back(AutoRemesher::Vector3(pos[0], pos[1], pos[2]));
                    }
                }
            }

            if (prim->indices) {
                for (size_t i = 0; i < prim->indices->count; i += 3) {
                    size_t i0 = cgltf_accessor_read_index(prim->indices, i + 0);
                    size_t i1 = cgltf_accessor_read_index(prim->indices, i + 1);
                    size_t i2 = cgltf_accessor_read_index(prim->indices, i + 2);
                    outTriangles.push_back({ vOffset + i0, vOffset + i1, vOffset + i2 });
                }
            }
        }
    }

    cgltf_free(data);

    if (outVertices.empty()) {
        outError = "glTF/GLB file contains no valid mesh geometry.";
        return false;
    }

    return true;
}

bool loadMesh(const QString& filename,
    std::vector<AutoRemesher::Vector3>& outVertices,
    std::vector<std::vector<size_t>>& outTriangles,
    std::string& outError)
{
    QFileInfo info(filename);
    QString ext = info.suffix().toLower();

    if (ext == "fbx") {
        return loadFbxFile(filename, outVertices, outTriangles, outError);
    } else if (ext == "glb" || ext == "gltf") {
        return loadGltfFile(filename, outVertices, outTriangles, outError);
    } else {
        return loadObjFile(filename, outVertices, outTriangles, outError);
    }
}

// ──────────────────────────────────────────────────────────
//  SAVE helpers
// ──────────────────────────────────────────────────────────

static bool saveObjFile(const QString& filename,
    const std::vector<AutoRemesher::Vector3>& vertices,
    const std::vector<std::vector<size_t>>& quads,
    std::string& outError)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        outError = "Could not open file for writing.";
        return false;
    }

    QTextStream stream(&file);
    stream << "# Exported by AutoRemesher\n";

    for (const auto& v : vertices) {
        stream << "v " << v.x() << " " << v.y() << " " << v.z() << "\n";
    }

    for (const auto& q : quads) {
        stream << "f";
        for (auto idx : q) {
            stream << " " << (idx + 1);
        }
        stream << "\n";
    }

    file.close();
    return true;
}

// Binary GLB 2.0 writer (no external dependency)
// Spec: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#glb-file-format-specification
static bool saveGlbFile(const QString& filename,
    const std::vector<AutoRemesher::Vector3>& vertices,
    const std::vector<std::vector<size_t>>& quads,
    std::string& outError)
{
    // ── 1. Triangulate quads → index buffer ────────────────────
    std::vector<uint32_t> indices;
    for (const auto& face : quads) {
        if (face.size() == 3) {
            indices.push_back((uint32_t)face[0]);
            indices.push_back((uint32_t)face[1]);
            indices.push_back((uint32_t)face[2]);
        } else if (face.size() == 4) {
            // quad → 2 tris (0,1,2) + (0,2,3)
            indices.push_back((uint32_t)face[0]);
            indices.push_back((uint32_t)face[1]);
            indices.push_back((uint32_t)face[2]);
            indices.push_back((uint32_t)face[0]);
            indices.push_back((uint32_t)face[2]);
            indices.push_back((uint32_t)face[3]);
        } else {
            // fan triangulate for n-gons
            for (size_t i = 1; i + 1 < face.size(); ++i) {
                indices.push_back((uint32_t)face[0]);
                indices.push_back((uint32_t)face[i]);
                indices.push_back((uint32_t)face[i + 1]);
            }
        }
    }

    // ── 2. Build BIN chunk ─────────────────────────────────────
    // Layout: [index buffer (uint32)] [vertex buffer (float3)]
    // Both must be 4-byte aligned.

    size_t indexBytes  = indices.size() * sizeof(uint32_t);
    size_t vertexBytes = vertices.size() * 3 * sizeof(float);

    // pad index buffer to 4-byte boundary
    size_t indexPad = (4 - (indexBytes % 4)) % 4;

    QByteArray bin;
    bin.reserve((int)(indexBytes + indexPad + vertexBytes));

    // indices
    for (uint32_t idx : indices) {
        char buf[4];
        memcpy(buf, &idx, 4);
        bin.append(buf, 4);
    }
    for (size_t i = 0; i < indexPad; ++i) bin.append('\0');

    size_t vertexOffset = (size_t)(indexBytes + indexPad);

    // vertices (Y-up, no flip needed for standard glTF)
    float minPos[3] = { 1e30f, 1e30f, 1e30f };
    float maxPos[3] = { -1e30f, -1e30f, -1e30f };
    for (const auto& v : vertices) {
        float xyz[3] = { (float)v.x(), (float)v.y(), (float)v.z() };
        char buf[12];
        memcpy(buf, xyz, 12);
        bin.append(buf, 12);
        for (int c = 0; c < 3; ++c) {
            if (xyz[c] < minPos[c]) minPos[c] = xyz[c];
            if (xyz[c] > maxPos[c]) maxPos[c] = xyz[c];
        }
    }

    // pad bin to 4-byte boundary
    while (bin.size() % 4 != 0) bin.append('\0');

    // ── 3. Build JSON chunk ────────────────────────────────────
    QJsonObject asset;
    asset["generator"] = "AutoRemesher";
    asset["version"]   = "2.0";

    QJsonObject bufferView0; // indices
    bufferView0["buffer"]     = 0;
    bufferView0["byteOffset"] = 0;
    bufferView0["byteLength"] = (int)indexBytes;
    bufferView0["target"]     = 34963; // ELEMENT_ARRAY_BUFFER

    QJsonObject bufferView1; // positions
    bufferView1["buffer"]     = 0;
    bufferView1["byteOffset"] = (int)vertexOffset;
    bufferView1["byteLength"] = (int)vertexBytes;
    bufferView1["byteStride"] = 12;
    bufferView1["target"]     = 34962; // ARRAY_BUFFER

    QJsonObject accessor0; // index accessor
    accessor0["bufferView"]    = 0;
    accessor0["byteOffset"]    = 0;
    accessor0["componentType"] = 5125; // UNSIGNED_INT
    accessor0["count"]         = (int)indices.size();
    accessor0["type"]          = "SCALAR";

    QJsonObject accessor1; // position accessor
    accessor1["bufferView"]    = 1;
    accessor1["byteOffset"]    = 0;
    accessor1["componentType"] = 5126; // FLOAT
    accessor1["count"]         = (int)vertices.size();
    accessor1["type"]          = "VEC3";
    accessor1["min"] = QJsonArray() << minPos[0] << minPos[1] << minPos[2];
    accessor1["max"] = QJsonArray() << maxPos[0] << maxPos[1] << maxPos[2];

    QJsonObject attribute;
    attribute["POSITION"] = 1;

    QJsonObject primitive;
    primitive["attributes"] = attribute;
    primitive["indices"]    = 0;
    primitive["mode"]       = 4; // TRIANGLES

    QJsonObject mesh;
    mesh["name"]       = "AutoRemeshed";
    mesh["primitives"] = QJsonArray() << primitive;

    QJsonObject node;
    node["mesh"] = 0;

    QJsonObject scene;
    scene["nodes"] = QJsonArray() << 0;

    QJsonObject buffer;
    buffer["byteLength"] = bin.size();

    QJsonObject root;
    root["asset"]       = asset;
    root["bufferViews"] = QJsonArray() << bufferView0 << bufferView1;
    root["accessors"]   = QJsonArray() << accessor0 << accessor1;
    root["meshes"]      = QJsonArray() << mesh;
    root["nodes"]       = QJsonArray() << node;
    root["scenes"]      = QJsonArray() << scene;
    root["scene"]       = 0;
    root["buffers"]     = QJsonArray() << buffer;

    QByteArray jsonBytes = QJsonDocument(root).toJson(QJsonDocument::Compact);
    // pad JSON to 4-byte boundary with spaces (spec requirement)
    while (jsonBytes.size() % 4 != 0) jsonBytes.append(' ');

    // ── 4. Assemble GLB file ───────────────────────────────────
    // GLB Header (12 bytes): magic, version, total length
    uint32_t magic   = 0x46546C67; // "glTF"
    uint32_t version = 2;
    uint32_t jsonChunkLen = (uint32_t)jsonBytes.size();
    uint32_t binChunkLen  = (uint32_t)bin.size();
    uint32_t totalLen = 12 + 8 + jsonChunkLen + (binChunkLen > 0 ? 8 + binChunkLen : 0);

    QFile out(filename);
    if (!out.open(QIODevice::WriteOnly)) {
        outError = "Could not open file for writing.";
        return false;
    }

    auto writeU32 = [&](uint32_t v) {
        out.write(reinterpret_cast<const char*>(&v), 4);
    };

    // GLB header
    writeU32(magic);
    writeU32(version);
    writeU32(totalLen);

    // JSON chunk header (type 0x4E4F534A = "JSON")
    writeU32(jsonChunkLen);
    writeU32(0x4E4F534A);
    out.write(jsonBytes);

    // BIN chunk header (type 0x004E4942 = "BIN\0")
    if (binChunkLen > 0) {
        writeU32(binChunkLen);
        writeU32(0x004E4942);
        out.write(bin);
    }

    out.close();
    return true;
}

// ──────────────────────────────────────────────────────────
//  Binary FBX 7.4 writer (mesh geometry only)
//
//  Written by hand because ufbx is a reader only. Binary (not ASCII) is
//  required: Blender's FBX importer rejects ASCII FBX outright.
//  Node/property layout follows the community specification:
//  https://code.blender.org/2013/08/fbx-binary-file-format-specification/
//  Quads survive the round trip — FBX stores polygons of any arity in
//  PolygonVertexIndex, with the last index of each polygon bitwise-negated.
// ──────────────────────────────────────────────────────────

namespace {

const int FBX_VERSION = 7400;
const int FBX_NULL_RECORD_SIZE = 13; // EndOffset + NumProperties + PropertyListLen + NameLen

class FbxBinaryWriter {
public:
    void writeHeader()
    {
        m_data.append("Kaydara FBX Binary  ", 20);
        putU8(0x00);
        putU8(0x1a);
        putU8(0x00);
        putU32(FBX_VERSION);
    }

    void writeFooter()
    {
        m_data.append(QByteArray(FBX_NULL_RECORD_SIZE, '\0')); // terminates the root node list
        m_data.append(QByteArray(16, '\0')); // file id; importers ignore it
        m_data.append(QByteArray(16 - (m_data.size() % 16), '\0')); // 1..16 bytes, keeps 16-byte alignment
        putU32(0);
        putU32(FBX_VERSION);
        m_data.append(QByteArray(120, '\0'));
        static const unsigned char footMagic[16] = {
            0xf8, 0x5a, 0x8c, 0x6a, 0xde, 0xf5, 0xd9, 0x7e,
            0xec, 0xe9, 0x0c, 0xe3, 0x75, 0x8f, 0x29, 0x0b
        };
        m_data.append(reinterpret_cast<const char*>(footMagic), 16);
    }

    void beginNode(const char* name)
    {
        if (!m_stack.empty()) {
            Frame& parent = m_stack.back();
            if (parent.propsEnd < 0)
                parent.propsEnd = m_data.size();
            parent.hasChild = true;
        }
        Frame frame;
        frame.startPos = m_data.size();
        putU32(0); // EndOffset, patched in endNode()
        putU32(0); // NumProperties
        putU32(0); // PropertyListLen
        int nameLen = (int)strlen(name);
        putU8((quint8)nameLen);
        m_data.append(name, nameLen);
        frame.propsStart = m_data.size();
        m_stack.push_back(frame);
    }

    void endNode()
    {
        Frame frame = m_stack.back();
        m_stack.pop_back();
        if (frame.propsEnd < 0)
            frame.propsEnd = m_data.size();
        if (frame.hasChild)
            m_data.append(QByteArray(FBX_NULL_RECORD_SIZE, '\0'));
        patchU32(frame.startPos, (quint32)m_data.size());
        patchU32(frame.startPos + 4, (quint32)frame.propCount);
        patchU32(frame.startPos + 8, (quint32)(frame.propsEnd - frame.propsStart));
    }

    void propInt(qint32 value)
    {
        beginProp('I');
        putU32((quint32)value);
    }

    void propInt64(qint64 value)
    {
        beginProp('L');
        putU64((quint64)value);
    }

    void propDouble(double value)
    {
        beginProp('D');
        putF64(value);
    }

    void propString(const QByteArray& value)
    {
        beginProp('S');
        putU32((quint32)value.size());
        m_data.append(value);
    }

    void propDoubleArray(const std::vector<double>& values)
    {
        beginProp('d');
        putU32((quint32)values.size());
        putU32(0); // encoding: uncompressed
        putU32((quint32)(values.size() * 8));
        for (double v : values)
            putF64(v);
    }

    void propIntArray(const std::vector<qint32>& values)
    {
        beginProp('i');
        putU32((quint32)values.size());
        putU32(0); // encoding: uncompressed
        putU32((quint32)(values.size() * 4));
        for (qint32 v : values)
            putU32((quint32)v);
    }

    const QByteArray& data() const { return m_data; }

private:
    struct Frame {
        int startPos = 0;
        int propsStart = 0;
        int propsEnd = -1;
        int propCount = 0;
        bool hasChild = false;
    };

    void beginProp(char type)
    {
        ++m_stack.back().propCount;
        m_data.append(type);
    }

    void putU8(quint8 v) { m_data.append((char)v); }

    void putU32(quint32 v)
    {
        char bytes[4] = { (char)(v & 0xff), (char)((v >> 8) & 0xff),
            (char)((v >> 16) & 0xff), (char)((v >> 24) & 0xff) };
        m_data.append(bytes, 4);
    }

    void putU64(quint64 v)
    {
        char bytes[8];
        for (int i = 0; i < 8; ++i)
            bytes[i] = (char)((v >> (i * 8)) & 0xff);
        m_data.append(bytes, 8);
    }

    void putF64(double v)
    {
        quint64 bits;
        memcpy(&bits, &v, 8);
        putU64(bits);
    }

    void patchU32(int pos, quint32 v)
    {
        char* p = m_data.data() + pos;
        p[0] = (char)(v & 0xff);
        p[1] = (char)((v >> 8) & 0xff);
        p[2] = (char)((v >> 16) & 0xff);
        p[3] = (char)((v >> 24) & 0xff);
    }

    QByteArray m_data;
    std::vector<Frame> m_stack;
};

// FBX binary stores an object's name as "Name\0\1Class" (shown as "Class::Name" in ASCII dumps).
QByteArray fbxObjectName(const QByteArray& name, const char* className)
{
    QByteArray result = name;
    result.append('\0');
    result.append('\1');
    result.append(className);
    return result;
}

} // namespace

static bool saveFbxFile(const QString& filename,
    const std::vector<AutoRemesher::Vector3>& vertices,
    const std::vector<std::vector<size_t>>& quads,
    std::string& outError)
{
    QByteArray meshName = QFileInfo(filename).baseName().toUtf8();
    if (meshName.isEmpty())
        meshName = "AutoRemeshed";

    std::vector<double> vertexData;
    vertexData.reserve(vertices.size() * 3);
    for (const auto& v : vertices) {
        vertexData.push_back(v.x());
        vertexData.push_back(v.y());
        vertexData.push_back(v.z());
    }

    // Polygons of any arity; the last index of each polygon is stored as ~index.
    std::vector<qint32> polygonIndices;
    for (const auto& face : quads) {
        if (face.size() < 3)
            continue;
        for (size_t i = 0; i < face.size(); ++i) {
            qint32 index = (qint32)face[i];
            polygonIndices.push_back(i + 1 == face.size() ? ~index : index);
        }
    }

    if (vertexData.empty() || polygonIndices.empty()) {
        outError = "Nothing to export.";
        return false;
    }

    const qint64 geometryUid = 1000000;
    const qint64 modelUid = 2000000;

    FbxBinaryWriter w;
    w.writeHeader();

    w.beginNode("FBXHeaderExtension");
    w.beginNode("FBXHeaderVersion");
    w.propInt(1003);
    w.endNode();
    w.beginNode("FBXVersion");
    w.propInt(FBX_VERSION);
    w.endNode();
    w.beginNode("Creator");
    w.propString("AutoRemesher");
    w.endNode();
    w.endNode();

    auto writeIntProperty = [&w](const char* name, qint32 value) {
        w.beginNode("P");
        w.propString(name);
        w.propString("int");
        w.propString("Integer");
        w.propString("");
        w.propInt(value);
        w.endNode();
    };
    auto writeDoubleProperty = [&w](const char* name, double value) {
        w.beginNode("P");
        w.propString(name);
        w.propString("double");
        w.propString("Number");
        w.propString("");
        w.propDouble(value);
        w.endNode();
    };

    w.beginNode("GlobalSettings");
    w.beginNode("Version");
    w.propInt(1000);
    w.endNode();
    w.beginNode("Properties70");
    // Standard FBX axis system (Y up, Z front, X right) — vertex data is written unchanged,
    // so importers apply their own up-axis conversion, matching how the OBJ was authored.
    writeIntProperty("UpAxis", 1);
    writeIntProperty("UpAxisSign", 1);
    writeIntProperty("FrontAxis", 2);
    writeIntProperty("FrontAxisSign", 1);
    writeIntProperty("CoordAxis", 0);
    writeIntProperty("CoordAxisSign", 1);
    // 100 cm per unit: declares the file as authored in metres, so one input unit
    // imports as one Blender/Unity unit instead of being scaled by 1/100.
    writeDoubleProperty("UnitScaleFactor", 100.0);
    writeDoubleProperty("OriginalUnitScaleFactor", 100.0);
    w.endNode();
    w.endNode();

    w.beginNode("Definitions");
    w.beginNode("Version");
    w.propInt(100);
    w.endNode();
    w.beginNode("Count");
    w.propInt(3);
    w.endNode();
    const char* objectTypes[] = { "GlobalSettings", "Geometry", "Model" };
    for (const char* objectType : objectTypes) {
        w.beginNode("ObjectType");
        w.propString(objectType);
        w.beginNode("Count");
        w.propInt(1);
        w.endNode();
        w.endNode();
    }
    w.endNode();

    w.beginNode("Objects");
    w.beginNode("Geometry");
    w.propInt64(geometryUid);
    w.propString(fbxObjectName(meshName, "Geometry"));
    w.propString("Mesh");
    w.beginNode("GeometryVersion");
    w.propInt(124);
    w.endNode();
    w.beginNode("Vertices");
    w.propDoubleArray(vertexData);
    w.endNode();
    w.beginNode("PolygonVertexIndex");
    w.propIntArray(polygonIndices);
    w.endNode();
    w.endNode();

    w.beginNode("Model");
    w.propInt64(modelUid);
    w.propString(fbxObjectName(meshName, "Model"));
    w.propString("Mesh");
    w.beginNode("Version");
    w.propInt(232);
    w.endNode();
    w.beginNode("Properties70");
    w.endNode();
    w.endNode();
    w.endNode();

    w.beginNode("Connections");
    w.beginNode("C");
    w.propString("OO");
    w.propInt64(geometryUid);
    w.propInt64(modelUid);
    w.endNode();
    w.beginNode("C");
    w.propString("OO");
    w.propInt64(modelUid);
    w.propInt64(0); // parent: scene root
    w.endNode();
    w.endNode();

    w.writeFooter();

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        outError = "Could not open file for writing.";
        return false;
    }
    if (file.write(w.data()) != w.data().size()) {
        outError = "Failed to write the whole FBX file.";
        file.close();
        return false;
    }
    file.close();
    return true;
}

bool saveMesh(const QString& filename,
    const std::vector<AutoRemesher::Vector3>& vertices,
    const std::vector<std::vector<size_t>>& quads,
    std::string& outError)
{
    QFileInfo info(filename);
    QString ext = info.suffix().toLower();

    if (ext == "glb" || ext == "gltf") {
        // Always write binary GLB (most compatible)
        QString glbPath = filename;
        if (ext == "gltf") {
            glbPath = info.absolutePath() + "/" + info.baseName() + ".glb";
        }
        return saveGlbFile(glbPath, vertices, quads, outError);
    } else if (ext == "fbx") {
        return saveFbxFile(filename, vertices, quads, outError);
    } else {
        // Default: OBJ
        return saveObjFile(filename, vertices, quads, outError);
    }
}

} // namespace MeshIO
