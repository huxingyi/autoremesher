#include <AutoRemesher/Remesher>
#include <AutoRemesher/HalfEdge>
#include <cassert>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#ifndef _WIN32
#include <QtCore>
#endif

static void help()
{
    printf("Usage: autoremesher <input.obj> -o <output.obj> [-s <gradient size>]\n");
    printf("       \n");
    printf("       https://github.com/huxingyi/autoremesher\n");
    exit(1);
}

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

int main(int argc, char *argv[]) 
{
#ifndef _WIN32
    QCoreApplication a(argc, argv);
#endif
    
    const char *inputFilename = nullptr;
    const char *outputFilename = nullptr;
    double gradientSize = 100;
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
    splitToIslands(inputTriangles, inputTrianglesIslands);
    
    if (inputTrianglesIslands.empty()) {
        std::cerr << "Input mesh is empty" << std::endl;
        exit(1);
    }
    
    auto pickedTriangleIsland = std::max_element(inputTrianglesIslands.begin(), inputTrianglesIslands.end(), [](
            const std::vector<std::vector<size_t>> &lhs,
            const std::vector<std::vector<size_t>> &rhs) {
        return lhs.size() < rhs.size();
    });
    
    if (inputTrianglesIslands.size() > 1) {
        std::cerr << "Input mesh contains multiple surfaces, here only pick the one with most triangles to remesh" << std::endl;
    }
    
    std::vector<AutoRemesher::Vector3> pickedVertices;
    std::vector<std::vector<size_t>> pickedTriangles;
    std::unordered_set<size_t> addedIndices;
    std::unordered_map<size_t, size_t> oldToNewVertexMap;
    for (const auto &face: *pickedTriangleIsland) {
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
    
    {
        AutoRemesher::HalfEdge::Mesh mesh(pickedVertices, pickedTriangles);
        mesh.exportPly("C:\\Users\\Jeremy\\Desktop\\test.ply");
    }
    
    /*
    AutoRemesher::Remesher remesher(pickedVertices, pickedTriangles);
    remesher.setGradientSize(gradientSize);
    if (!remesher.remesh()) {
        std::cerr << "Remesh failed" << std::endl;
        exit(1);
    }
    if (!saveObj(outputFilename, remesher.remeshedVertices(), remesher.remeshedQuads())) {
        exit(1);
    }
    */
    
    return 0;
}
