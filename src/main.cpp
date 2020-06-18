#include <autoremesher.h>
#include <cassert>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#ifndef _WIN32
#include <QtCore>
#endif

static void help()
{
    printf("Usage: autoremesher.exe input.obj -o output.obj\n");
    exit(1);
}

static bool saveObj(const char *filename,
    const std::vector<autoremesher::Vector3> &vertices,
    const std::vector<std::vector<size_t>> &faces)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        std::cerr << "Output to file failed:" << filename << std::endl;
        return false;
    }
    for (std::vector<autoremesher::Vector3>::const_iterator it = vertices.begin() ; it != vertices.end(); ++it) {
        fprintf(fp, "v %f %f %f\n", (*it).x, (*it).y, (*it).z);
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
            } else if (0 == strcmp(argv[i], "-size")) {
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
    
    std::vector<autoremesher::Vector3> inputVertices(attributes.vertices.size() / 3);
    for (size_t i = 0, j = 0; i < inputVertices.size(); ++i) {
        auto &dest = inputVertices[i];
        dest.x = attributes.vertices[j++];
        dest.y = attributes.vertices[j++];
        dest.z = attributes.vertices[j++];
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
    std::vector<std::vector<size_t>> inputQuads;
    
    std::vector<autoremesher::Vector3> outputVertices;
    std::vector<std::vector<size_t>> outputQuads;
    autoremesher::remesh(inputVertices,
        inputTriangles,
        inputQuads,
        &outputVertices,
        &outputQuads,
        gradientSize);
    if (!saveObj(outputFilename, outputVertices, outputQuads)) {
        exit(1);
    }
    
    return 0;
}
