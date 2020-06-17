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

#include <gtest/gtest.h>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <qex.h>

#include <memory>

using namespace QEx;

namespace {

class Test_Reference_Mesh: public testing::TestWithParam<const char *> {
    public:

        virtual void SetUp() {
            if (!inputMesh) {
                const std::string infileName =
                        std::string("meshes/") + GetParam() + "_param.obj";
                inputMesh = new TriMesh;
                inputMesh->request_halfedge_texcoords2D();
//                OpenMesh::IO::read_mesh(*inputMesh, infileName);
                OpenMesh::IO::Options opts(OpenMesh::IO::Options::FaceTexCoord);
                OpenMesh::IO::read_mesh(*inputMesh, infileName, opts);
            }
            if (!referenceOutput) {
                referenceOutput = new QuadMesh;
                const std::string outfileName =
                        std::string("meshes/") + GetParam() + "_quad.obj";
                OpenMesh::IO::read_mesh(*referenceOutput, outfileName);
            }
        }

        static void TearDownTestCase() {
            delete inputMesh;
            inputMesh = 0;
            delete referenceOutput;
            referenceOutput = 0;
        }

        static TriMesh *inputMesh;
        static QuadMesh *referenceOutput;
};
TriMesh *Test_Reference_Mesh::inputMesh = 0;
QuadMesh *Test_Reference_Mesh::referenceOutput = 0;

TEST_P(Test_Reference_Mesh, basic_sanity) {
    EXPECT_LT(0u, inputMesh->n_vertices());
    EXPECT_LT(0u, inputMesh->n_edges());
    EXPECT_LT(0u, inputMesh->n_faces());

    EXPECT_LT(0u, referenceOutput->n_vertices());
    EXPECT_LT(0u, referenceOutput->n_edges());
    EXPECT_LT(0u, referenceOutput->n_faces());
}

::testing::AssertionResult polyMeshesAreEqual(QuadMesh &expected, QuadMesh &actual) {

    std::map<QuadMesh::Point, size_t> vertexPos2idx_expected, vertexPos2idx_actual;

    /*
     * Index vertices by their positions.
     */
    for (QuadMesh::VertexIter ex_it = expected.vertices_begin(), ex_end = expected.vertices_end(),
            ac_it = actual.vertices_begin(), ac_end = actual.vertices_end(); ex_it != ex_end && ac_it != ac_end; ++ex_it, ++ac_it) {
        vertexPos2idx_expected[expected.point(*ex_it)] = ex_it->idx();
        vertexPos2idx_actual[actual.point(*ac_it)] = ac_it->idx();
    }

    ::testing::AssertionResult failure = ::testing::AssertionFailure();
    bool is_failure = false;

    if (vertexPos2idx_expected.size() != vertexPos2idx_actual.size()) {
        is_failure = true;
        failure
            << "Expected and actual mesh have different number of vertices: "
            << "Expected = " << vertexPos2idx_expected.size()
            << ", actual = " << vertexPos2idx_actual.size() << "." << std::endl;
    }
    if (vertexPos2idx_expected.size() != expected.n_vertices()) {
        is_failure = true;
        failure << "Vertices with identical positions detected. Vertices: "
                << expected.n_vertices() << " Unique vertices: "
                << vertexPos2idx_expected.size() << std::endl;
    }

    /*
     * Match vertices.
     *
     * This is a slightly opimized O(V^2) all-pairs nearest neighbor search.
     */

    // Make sure our mapping stays bijective.
    std::set<size_t> mapped_in_actual;
    std::map<size_t, size_t> vidx_expected2actual;
    // Find an initial solution, by lexicographically sorting the vertex positions.
    for (std::map<QuadMesh::Point, size_t>::const_iterator
            ex_it = vertexPos2idx_expected.begin(), ex_end = vertexPos2idx_expected.end(),
            ac_it = vertexPos2idx_actual.begin(), ac_end = vertexPos2idx_actual.end();
            ex_it != ex_end && ac_it != ac_end; ++ex_it, ++ac_it) {

        /*
         * Now for each expected vertex we search for a vertex closer than the one
         * already assigned. We do this in the lexicographically sorted list so we can
         * stop the search once the x-coordinate becomes larger than the distance to the
         * closest actual vertex found so far.
         */
        double bestDistance = (ex_it->first - ac_it->first).sqrnorm();
        std::map<QuadMesh::Point, size_t>::const_iterator best_ac = ac_it;

        // Search backwards.
        for (std::map<QuadMesh::Point, size_t>::const_iterator ac_it_2 = ac_it; ac_it_2 != vertexPos2idx_actual.begin();) {
            --ac_it_2;
            if (pow(std::abs(ex_it->first[0] - ac_it_2->first[0]), 2.0) > bestDistance) break;
            if (bestDistance > (ex_it->first - ac_it_2->first).sqrnorm()) {
                bestDistance = (ex_it->first - ac_it_2->first).sqrnorm();
                best_ac = ac_it_2;
            }
        }

        // Search forward.
        for (std::map<QuadMesh::Point, size_t>::const_iterator ac_it_2 = ac_it; ac_it_2 != vertexPos2idx_actual.end(); ++ac_it_2) {
            if (pow(std::abs(ex_it->first[0] - ac_it_2->first[0]), 2.0) > bestDistance) break;
            if (bestDistance > (ex_it->first - ac_it_2->first).sqrnorm()) {
                bestDistance = (ex_it->first - ac_it_2->first).sqrnorm();
                best_ac = ac_it_2;
            }
        }

        if (mapped_in_actual.find(best_ac->second) != mapped_in_actual.end()) {
            is_failure = true;
            failure << "Ambiguous vertex positions." << std::endl;
        }
        mapped_in_actual.insert(best_ac->second);
        if ((ex_it->first - best_ac->first).sqrnorm() > 1e-9) {
            is_failure = true;
            failure << "Vertex position mismatch ("
                    << std::distance((std::map<QuadMesh::Point, size_t>::const_iterator)
                            vertexPos2idx_expected.begin(), ex_it) << "): "
                    << "expected #" << ex_it->second << ": " << ex_it->first
                    << ", actual #" << best_ac->second << ": " << best_ac->first
                    << ", squared distance: " << (ex_it->first - best_ac->first).sqrnorm()
                    << std::endl;
        }

        vidx_expected2actual[ex_it->second] = best_ac->second;
    }


    /*
     * Match edges.
     */
    for (QuadMesh::EdgeIter e_it = expected.edges_begin(), e_end = expected.edges_end(); e_it != e_end; ++e_it) {
        const QuadMesh::VertexHandle vh_to = expected.to_vertex_handle(expected.halfedge_handle(*e_it, 0)),
                vh_from = expected.from_vertex_handle(expected.halfedge_handle(*e_it, 0));
        const QuadMesh::VertexHandle ac_to = actual.vertex_handle(vidx_expected2actual[vh_to.idx()]),
                ac_from = actual.vertex_handle(vidx_expected2actual[vh_from.idx()]);

        bool found = false;
        for (QuadMesh::VVIter vv_it = actual.vv_begin(ac_from), vv_end = actual.vv_end(ac_from); vv_it != vv_end; ++vv_it) {
            if (*vv_it == ac_to) {
                found = true;
                break;
            }
        }
        if (!found) {
            is_failure = true;
            failure << "Expected-edge (" << vh_from.idx() << ", " << vh_to.idx()
                    << ") |-> (" << ac_from.idx() << ", " << ac_to.idx() << ") not matched in actual mesh.";
        }

    }

    /*
     * Match faces.
     */
    for (QuadMesh::FaceIter f_it = expected.faces_begin(), f_end = expected.faces_end(); f_it != f_end; ++f_it) {
        /*
         * Put face indices twice in a row.
         */
        unsigned int faceValence = expected.valence(*f_it);
        std::vector<size_t> expectedFaceIndices; expectedFaceIndices.reserve(faceValence * 2);
        for (QuadMesh::FVIter fv_it = expected.fv_begin(*f_it), fv_end = expected.fv_end(*f_it); fv_it != fv_end; ++fv_it) {
            expectedFaceIndices.push_back(vidx_expected2actual[fv_it->idx()]);
        }
        assert(!expectedFaceIndices.empty());
        assert(expectedFaceIndices.size() * 2 == expectedFaceIndices.capacity());
        std::copy(expectedFaceIndices.begin(), expectedFaceIndices.end(), std::back_inserter(expectedFaceIndices));

        /*
         * Search matching face in actual mesh.
         */
        QuadMesh::VertexHandle seed_vertex = actual.vertex_handle(expectedFaceIndices.front());
        bool found = false;
        for (QuadMesh::VFIter vf_it = actual.vf_begin(seed_vertex), vf_end = actual.vf_end(seed_vertex); vf_it != vf_end; ++vf_it) {
            if (actual.valence(*vf_it) != faceValence) continue;
            std::vector<size_t> actualFaceIndices; actualFaceIndices.reserve(faceValence);
            for (QuadMesh::FVIter fv_it = actual.fv_begin(*vf_it), fv_end = actual.fv_end(*vf_it); fv_it != fv_end; ++fv_it) {
                actualFaceIndices.push_back(fv_it->idx());
            }
            if (expectedFaceIndices.end() !=
                    std::search(expectedFaceIndices.begin(), expectedFaceIndices.end(), actualFaceIndices.begin(), actualFaceIndices.end())) {
                found = true;
                break;
            }
        }
        if (!found) {
            is_failure = true;
            failure << "Expected face " << f_it->idx() << " not found in actual mesh.";
        }
    }

    if (is_failure)
        return failure;
    else
        return ::testing::AssertionSuccess() << "Poly meshes are equal.";
}

TEST_P(Test_Reference_Mesh, output_matches_reference) {
    EXPECT_FALSE(inputMesh->has_vertex_texcoords2D());
    ASSERT_TRUE(inputMesh->has_halfedge_texcoords2D());

    /*
     * Convert texture coordinates into separate uv vector.
     */
    std::vector<OpenMesh::Vec2d> uvVector;
    uvVector.reserve(inputMesh->n_halfedges());
    for (TriMesh::HalfedgeIter he_it = inputMesh->halfedges_begin(), he_end = inputMesh->halfedges_end();
            he_it != he_end; ++he_it) {
        const OpenMesh::Vec2f &uv_f = inputMesh->texcoord2D(*he_it);
        OpenMesh::Vec2d uv(uv_f[0], uv_f[1]);
        uvVector.push_back(uv);
    }

    QuadMesh out;
    extractQuadMeshOM(inputMesh, &uvVector, 0, &out);

    ASSERT_EQ(referenceOutput->n_vertices(), out.n_vertices());
    ASSERT_EQ(referenceOutput->n_edges(), out.n_edges());
    ASSERT_EQ(referenceOutput->n_faces(), out.n_faces());

    EXPECT_TRUE(polyMeshesAreEqual(*referenceOutput, out));
}

INSTANTIATE_TEST_CASE_P(Bunny,
        Test_Reference_Mesh,
        ::testing::Values("bunny"));
INSTANTIATE_TEST_CASE_P(Armadillo,
        Test_Reference_Mesh,
        ::testing::Values("armadillo"));
INSTANTIATE_TEST_CASE_P(BotijoMIQ,
        Test_Reference_Mesh,
        ::testing::Values("botijo_miq"));
INSTANTIATE_TEST_CASE_P(BotijoQC,
        Test_Reference_Mesh,
        ::testing::Values("botijo_qc"));
INSTANTIATE_TEST_CASE_P(Fandisk,
        Test_Reference_Mesh,
        ::testing::Values("fandisk"));
INSTANTIATE_TEST_CASE_P(DuckMIQ4,
        Test_Reference_Mesh,
        ::testing::Values("duck_miq_4"));
INSTANTIATE_TEST_CASE_P(DuckMIQ8,
        Test_Reference_Mesh,
        ::testing::Values("duck_miq_8"));
INSTANTIATE_TEST_CASE_P(DuckMIQ16,
        Test_Reference_Mesh,
        ::testing::Values("duck_miq_16"));
INSTANTIATE_TEST_CASE_P(DuckMIQ28,
        Test_Reference_Mesh,
        ::testing::Values("duck_miq_28"));
INSTANTIATE_TEST_CASE_P(Dragon,
        Test_Reference_Mesh,
        ::testing::Values("dragon"));

} /* anonymous namespace */
