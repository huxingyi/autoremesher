#include <AutoRemesher/QuadRemesher>
#include <AutoRemesher/IsotropicRemesher>
#include <AutoRemesher/HalfEdge>
#include <cassert>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <set>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#ifndef _WIN32
#include <QtCore>
#endif

static void copyright()
{
    std::cerr << "libigl        - Copyright (c) 2019 Alec Jacobson, Daniele Panozzo, Christian Schüller, Olga Diamanti, Qingnan Zhou, Sebastian Koch, Jeremie Dumas, Amir Vaxman, Nico Pietroni, Stefan Brugger, Kenshi Takayama, Wenzel Jakob, Nikolas De Giorgis, Luigi Rocca, Leonardo Sacht, Kevin Walliman, Olga Sorkine-Hornung, Teseo Schneider, and others." << std::endl;
    std::cerr << "libqex        - Copyright (c) Ebke, Hans-Christian and Bommes, David and Campen, Marcel and Kobbelt, Leif" << std::endl;
    std::cerr << "openmesh      - Copyright (c) 2001-2015, RWTH-Aachen University" << std::endl;
    std::cerr << "eigen         - http://eigen.tuxfamily.org/" << std::endl;
    std::cerr << "tinyobjloader - Copyright (c) 2012-2019 Syoyo Fujita and many contributors." << std::endl;
    std::cerr << "clapack       - http://icl.cs.utk.edu/lapack-forum/" << std::endl;
    std::cerr << "autoremesher  - Copyright (c) 2020 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved." << std::endl;
    std::cerr << "       " << std::endl;
}

static void help()
{
    std::cerr << "Usage: autoremesher <input.obj> -o <output.obj> [-s <gradient size>]" << std::endl;
    std::cerr << "       " << std::endl;
    exit(1);
}

int gDebugIndex = 0;

bool saveObj(const char *filename,
    const std::vector<AutoRemesher::Vector3> &vertices,
    const std::vector<std::vector<size_t>> &faces)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        std::cerr << "Output to file failed:" << filename << std::endl;
        return false;
    }
    for (std::vector<AutoRemesher::Vector3>::const_iterator it = vertices.begin() ; it != vertices.end(); ++it) {
        fprintf(fp, "v %f %f %f\n", (*it).x(), (*it).y(), (*it).z());
    }
    for (std::vector<std::vector<size_t>>::const_iterator it = faces.begin() ; it != faces.end(); ++it) {
        fprintf(fp, "f");
        for (std::vector<size_t>::const_iterator subIt = (*it).begin() ; subIt != (*it).end(); ++subIt) {
            fprintf(fp, " %zu", (1 + *subIt));
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    return true;
}

static void buildEdgeToFaceMap(const std::vector<std::vector<size_t>> &triangles, std::map<std::pair<size_t, size_t>, size_t> &edgeToFaceMap)
{
    edgeToFaceMap.clear();
    for (size_t index = 0; index < triangles.size(); ++index) {
        const auto &face = triangles[index];
        for (size_t i = 0; i < 3; i++) {
            size_t j = (i + 1) % 3;
            edgeToFaceMap[{face[i], face[j]}] = index;
        }
    }
}

static void splitToIslands(const std::vector<std::vector<size_t>> &triangles, std::vector<std::vector<std::vector<size_t>>> &islands)
{
    std::map<std::pair<size_t, size_t>, size_t> edgeToFaceMap;
    buildEdgeToFaceMap(triangles, edgeToFaceMap);
    
    std::unordered_set<size_t> processedFaces;
    std::queue<size_t> waitFaces;
    for (size_t indexInGroup = 0; indexInGroup < triangles.size(); ++indexInGroup) {
        if (processedFaces.find(indexInGroup) != processedFaces.end())
            continue;
        waitFaces.push(indexInGroup);
        std::vector<std::vector<size_t>> island;
        while (!waitFaces.empty()) {
            size_t index = waitFaces.front();
            waitFaces.pop();
            if (processedFaces.find(index) != processedFaces.end())
                continue;
            const auto &face = triangles[index];
            for (size_t i = 0; i < 3; i++) {
                size_t j = (i + 1) % 3;
                auto findOppositeFaceResult = edgeToFaceMap.find({face[j], face[i]});
                if (findOppositeFaceResult == edgeToFaceMap.end())
                    continue;
                waitFaces.push(findOppositeFaceResult->second);
            }
            island.push_back(triangles[index]);
            processedFaces.insert(index);
        }
        if (island.empty())
            continue;
        islands.push_back(island);
    }
}

static void normalizeVertices(std::vector<AutoRemesher::Vector3> &vertices)
{
    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();
    double minZ = std::numeric_limits<double>::max();
    double maxZ = std::numeric_limits<double>::lowest();
    for (const auto &v: vertices) {
        if (v.x() < minX)
            minX = v.x();
        if (v.x() > maxX)
            maxX = v.x();
        if (v.y() < minY)
            minY = v.y();
        if (v.y() > maxX)
            maxY = v.y();
        if (v.z() < minZ)
            minZ = v.z();
        if (v.z() > maxZ)
            maxZ = v.z();
    }
    AutoRemesher::Vector3 length = {
        (maxX - minX) * 0.5,
        (maxY - minY) * 0.5,
        (maxZ - minZ) * 0.5,
    };
    auto maxLength = length[0];
    if (length[1] > maxLength)
        maxLength = length[1];
    if (length[2] > maxLength)
        maxLength = length[2];
    AutoRemesher::Vector3 origin = {
        (maxX + minX) * 0.5,
        (maxY + minY) * 0.5,
        (maxZ + minZ) * 0.5,
    };
    std::cerr << "origin:" << origin << std::endl;
    std::cerr << "length:" << length << std::endl;
    std::cerr << "maxLength:" << maxLength << std::endl;
    for (auto &v: vertices) {
        v = (v - origin) / maxLength;
    }
}

int main(int argc, char *argv[]) 
{
#ifndef _WIN32
    QCoreApplication a(argc, argv);
#endif

    copyright();
    
    const char *inputFilename = nullptr;
    const char *outputFilename = nullptr;
    double gradientSize = 100;
    double constraintStength = 0.5;
    for (int i = 1; i < argc; ++i) {
        if ('-' == argv[i][0]) {
            if (0 == strcmp(argv[i], "-o")) {
                if (i + 1 < argc) {
                    ++i;
                    outputFilename = argv[i];
                }
            } else if (0 == strcmp(argv[i], "-s")) {
                if (i + 1 < argc) {
                    ++i;
                    gradientSize = atof(argv[i]);
                }
            } else if (0 == strcmp(argv[i], "-c")) {
                if (i + 1 < argc) {
                    ++i;
                    constraintStength = atof(argv[i]);
                }
            } else if (0 == strcmp(argv[i], "-d")) {
                if (i + 1 < argc) {
                    ++i;
                    gDebugIndex = atof(argv[i]);
                }
            }
        } else {
            inputFilename = argv[i];
        }
    }
    
    if (nullptr == inputFilename || nullptr == outputFilename) {
        help();
    }
    
    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    
    std::cerr << "Loading input mesh..." << std::endl;

    bool loadSuccess = tinyobj::LoadObj(&attributes, &shapes, &materials, &warn, &err, inputFilename);
    if (!warn.empty()) {
        std::cerr << "WARN:" << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << err << std::endl;
    }
    if (!loadSuccess) {
        exit(1);
    }
    
    std::cerr << "Preprocessing..." << std::endl;
    
    std::vector<AutoRemesher::Vector3> inputVertices(attributes.vertices.size() / 3);
    for (size_t i = 0, j = 0; i < inputVertices.size(); ++i) {
        auto &dest = inputVertices[i];
        dest.setX(attributes.vertices[j++]);
        dest.setY(attributes.vertices[j++]);
        dest.setZ(attributes.vertices[j++]);
    }
    
    std::vector<std::vector<size_t>> inputTriangles;
    for (const auto &shape: shapes) {
        for (size_t i = 0; i < shape.mesh.indices.size(); i += 3) {
            inputTriangles.push_back(std::vector<size_t> {
                (size_t)shape.mesh.indices[i + 0].vertex_index,
                (size_t)shape.mesh.indices[i + 1].vertex_index,
                (size_t)shape.mesh.indices[i + 2].vertex_index
            });
        }
    }
    
    std::vector<std::vector<std::vector<size_t>>> inputTrianglesIslands;
    //splitToIslands(inputTriangles, inputTrianglesIslands);
    inputTrianglesIslands = {inputTriangles};
    
    if (inputTrianglesIslands.empty()) {
        std::cerr << "Input mesh is empty" << std::endl;
        exit(1);
    }
    
    std::cerr << "Start remeshing, this may take a few minutes..." << std::endl;
    
    std::vector<AutoRemesher::Vector3> resultVertices;
    std::vector<std::vector<size_t>> resultQuads;
    for (size_t islandIndex = 0; islandIndex < inputTrianglesIslands.size(); ++islandIndex) {
        const auto &island = inputTrianglesIslands[islandIndex];
        /*
        std::vector<AutoRemesher::Vector3> pickedVertices;
        std::vector<std::vector<size_t>> pickedTriangles;
        std::unordered_set<size_t> addedIndices;
        std::unordered_map<size_t, size_t> oldToNewVertexMap;
        for (const auto &face: island) {
            std::vector<size_t> triangle;
            for (size_t i = 0; i < 3; ++i) {
                auto insertResult = addedIndices.insert(face[i]);
                if (insertResult.second) {
                    oldToNewVertexMap.insert({face[i], pickedVertices.size()});
                    pickedVertices.push_back(inputVertices[face[i]]);
                }
                triangle.push_back(oldToNewVertexMap[face[i]]);
            }
            pickedTriangles.push_back(triangle);
        }
        std::cerr << "Remeshing surface #" << (islandIndex + 1) << "/" << inputTrianglesIslands.size() << "(vertices:" << pickedVertices.size() << " triangles:" << pickedTriangles.size() << ")..." << std::endl;
        */
        
        std::vector<AutoRemesher::Vector3> &pickedVertices = inputVertices;
        std::vector<std::vector<size_t>> &pickedTriangles = inputTrianglesIslands[islandIndex];
        
        normalizeVertices(pickedVertices);

        AutoRemesher::IsotropicRemesher isotropicRemesher(pickedVertices, pickedTriangles);
        isotropicRemesher.remesh();
        isotropicRemesher.debugExportObj("C:\\Users\\Jeremy\\Desktop\\test-isotropic.obj");
        
        AutoRemesher::QuadRemesher quadRemesher(isotropicRemesher.remeshedVertices(), isotropicRemesher.remeshedTriangles());
        //AutoRemesher::QuadRemesher quadRemesher(pickedVertices, pickedTriangles);
        quadRemesher.setGradientSize(gradientSize);
        quadRemesher.setConstraintStength(constraintStength);
        //auto coutBuffer = std::cout.rdbuf();
        //auto cerrBuffer = std::cerr.rdbuf();
        //std::cout.rdbuf(nullptr);
        //std::cerr.rdbuf(nullptr);
        bool remeshSucceed = quadRemesher.remesh();
        //std::cout.rdbuf(coutBuffer);
        //std::cerr.rdbuf(cerrBuffer);
        if (!remeshSucceed) {
            std::cerr << "Surface #" << (islandIndex + 1) << "/" << inputTrianglesIslands.size() << " failed to remesh" << std::endl;
            continue;
        }
        const auto &quads = quadRemesher.remeshedQuads();
        if (quads.empty())
            continue;
        const auto &vertices = quadRemesher.remeshedVertices();
        std::cerr << "Surface #" << (islandIndex + 1) << "/" << inputTrianglesIslands.size() << " remesh succeed(vertices:" << vertices.size() << " quads:" << quads.size() << ")" << std::endl;
        size_t vertexStartIndex = resultVertices.size();
        resultVertices.insert(resultVertices.end(), vertices.begin(), vertices.end());
        for (const auto &it: quads) {
            resultQuads.push_back({
                vertexStartIndex + it[0], 
                vertexStartIndex + it[1], 
                vertexStartIndex + it[2], 
                vertexStartIndex + it[3]
            });
        }
    }
    
    if (!saveObj(outputFilename, resultVertices, resultQuads)) {
        exit(1);
    }
    
    std::cerr << "All done!" << std::endl;
    
    return 0;
}
