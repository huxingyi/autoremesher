/*
 * Copyright 2013 Computer Graphics Group, RWTH Aachen University
 * Author: Hans-Christian Ebke <ebke@cs.rwth-aachen.de>
 *
 * This file is part of QEx.
 *
 * QEx is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * QEx is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QEx.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <qex.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#define USE_POSIX
#include <libgen.h>
#include <cstring>
#endif

using namespace QEx;

void printUsage(const std::string &cmd) {
#ifdef USE_POSIX
    char *cmdCpy = new char[cmd.size() + 1];
    strcpy(cmdCpy, cmd.data());
    std::string basename = ::basename(cmdCpy);
    delete[] cmdCpy;
#endif
    std::cout << "Usage: "
            << basename
            << " <infile> <outfile> [<vertex valences>]" << std::endl
            << std::endl
            << "Reads the mesh with face-based UVs from <infile> which must be an" << std::endl
            << "OBJ file extracts the quad mesh from the UVs and stores the resulting" << std::endl
            << "mesh into <outfile> in OBJ format. Optionally reads vertex valences in" << std::endl
            << "VVAL file format from file <vertex valences>." << std::endl;
}

bool infileGood(const std::string &filePath) {
    std::ifstream is(filePath.c_str());
    return is.good();
}

bool outfileGood(const std::string &filePath) {
    std::ofstream is(filePath.c_str());
    return is.good();
}

bool read_vertex_valences(const char *file_name,
        size_t expected_vertex_count,
        std::vector<unsigned int> &out_vertex_valences) {

    std::ifstream is(file_name);
    std::string header;

    static const char *EXPECTED_HEADER =
        "{\"file_format\":\"VVAL\",\"version\":1}";
    std::getline(is, header);
    if (header != EXPECTED_HEADER) {
        std::cerr << "Error when reading vertex valence file: "
                "Unexpected file format." << std::endl;
        return false;
    }

    size_t vertex_count;
    is >> vertex_count;
    if (vertex_count != expected_vertex_count) {
        std::cerr << "Error when reading vertex valence file: "
                "Unexpected number of vertices. Expected: " <<
                expected_vertex_count << ", actual: " <<
                vertex_count << "." << std::endl;
        return false;
    }

    out_vertex_valences.clear();
    out_vertex_valences.reserve(vertex_count);
    for (; vertex_count; --vertex_count) {
        int val;
        is >> val;
        out_vertex_valences.push_back(val);
    }

    return true;
}

int main(int argc, const char *argv[]) {
    if (argc != 3 && argc != 4) {
        printUsage(argv[0]);
        return 1;
    }

    if (!infileGood(argv[1])) {
        std::cout << "Can't read input file." << std::endl;
        return 2;
    }

    if (!outfileGood(argv[2])) {
        std::cout << "Can't write output file." << std::endl;
        return 3;
    }

    if (argc >= 4 && !infileGood(argv[3])) {
        std::cout << "Can't read vertex valences file." << std::endl;
        return 4;
    }

    /*
     * Read input mesh.
     */
    TriMesh inputMesh;
    inputMesh.request_halfedge_texcoords2D();
    OpenMesh::IO::Options readOpts(OpenMesh::IO::Options::FaceTexCoord);
    OpenMesh::IO::read_mesh(inputMesh, argv[1], readOpts);

    /*
     * Convert texture coordinates into separate uv vector.
     */
    std::vector<OpenMesh::Vec2d> uvVector;
    uvVector.reserve(inputMesh.n_halfedges());
    for (TriMesh::HalfedgeIter he_it = inputMesh.halfedges_begin(), he_end = inputMesh.halfedges_end();
            he_it != he_end; ++he_it) {
        const OpenMesh::Vec2f &uv_f = inputMesh.texcoord2D(*he_it);
        OpenMesh::Vec2d uv(uv_f[0], uv_f[1]);
        uvVector.push_back(uv);
    }

    std::vector<unsigned int> vertex_valences;
    if (argc >= 4) {
        if (!read_vertex_valences(argv[3], inputMesh.n_vertices(),
                vertex_valences)) return 5;
    }

    QuadMesh out;
    extractQuadMeshOMT(&inputMesh, &uvVector,
            (vertex_valences.empty() ? 0 : &vertex_valences), &out);

    /*
     * Write output mesh.
     */
    OpenMesh::IO::write_mesh(out, argv[2], OpenMesh::IO::Options::Default, 12);

    return 0;
}
