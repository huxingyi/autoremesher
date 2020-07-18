#include <igl/avg_edge_length.h>
#include <igl/barycenter.h>
#include <igl/comb_cross_field.h>
#include <igl/comb_frame_field.h>
#include <igl/compute_frame_field_bisectors.h>
#include <igl/cross_field_mismatch.h>
#include <igl/cut_mesh_from_singularities.h>
#include <igl/find_cross_field_singularities.h>
#include <igl/local_basis.h>
#include <igl/rotate_vectors.h>
#include <igl/copyleft/comiso/miq.h>
#include <igl/copyleft/comiso/nrosy.h>
#include <AutoRemesher/Parametrization>
#include <iostream>

namespace AutoRemesher
{
    
namespace Parametrization
{

bool miq(HalfEdge::Mesh &mesh, const Parameters &parameters)
{
    // https://github.com/libigl/libigl/blob/master/tutorial/505_MIQ/main.cpp

    double iter = 0;
    double stiffness = 5.0;
    bool direct_round = 0;
    
    std::vector<std::vector<int>> featuredEdges;
    
    Eigen::MatrixXd V(mesh.vertexCount(), 3);
    Eigen::MatrixXi F(mesh.faceCount(), 3);
    
    std::cerr << "miq preparing..." << std::endl;
    
    size_t vertexNum = 0;
    for (HalfEdge::Vertex *vertex = mesh.firstVertex(); nullptr != vertex; vertex = vertex->_next) {
        vertex->outputIndex = vertexNum;
        V.row(vertexNum++) << 
            vertex->position.x(), 
            vertex->position.y(), 
            vertex->position.z();
    }
    
    Eigen::VectorXi b_soft(1);
    Eigen::VectorXd w_soft(1);
    Eigen::MatrixXd bc_soft(1, 3);
    size_t faceNum = 0;
    for (HalfEdge::Face *face = mesh.firstFace(); nullptr != face; face = face->_next) {
        HalfEdge::HalfEdge *h0 = face->anyHalfEdge;
        HalfEdge::HalfEdge *h1 = h0->nextHalfEdge;
        HalfEdge::HalfEdge *h2 = h1->nextHalfEdge;
        F.row(faceNum) << 
            h0->startVertex->outputIndex, 
            h1->startVertex->outputIndex, 
            h2->startVertex->outputIndex;
        if (0 != h0->featured || 0 != h1->featured || 0 != h2->featured) {
            b_soft << faceNum;
            w_soft << 0.5;
            bc_soft << face->guidelineDirection.x(), face->guidelineDirection.y(), face->guidelineDirection.z();
        }
        //TODO: hardFeatures not work on the current igl::copyleft::comiso::miq implementation
        //if (0 != h0->featured)
        //    featuredEdges.push_back({(int)faceNum, (int)0});
        //if (0 != h1->featured)
        //    featuredEdges.push_back({(int)faceNum, (int)1});
        //if (0 != h2->featured)
        //    featuredEdges.push_back({(int)faceNum, (int)2});
        ++faceNum;
    }
    
    bool extend_arrows = false;

    // Cross field
    Eigen::MatrixXd X1,X2;

    // Bisector field
    Eigen::MatrixXd BIS1, BIS2;

    // Combed bisector
    Eigen::MatrixXd BIS1_combed, BIS2_combed;

    // Per-corner, integer mismatches
    Eigen::Matrix<int, Eigen::Dynamic, 3> MMatch;

    // Field singularities
    Eigen::Matrix<int, Eigen::Dynamic, 1> isSingularity, singularityIndex;

    // Per corner seams
    Eigen::Matrix<int, Eigen::Dynamic, 3> Seams;

    // Combed field
    Eigen::MatrixXd X1_combed, X2_combed;

    // Global parametrization
    Eigen::MatrixXd UV;
    Eigen::MatrixXi FUV;
    
    Eigen::VectorXi b(1);
    b << 0;
    Eigen::MatrixXd bc(1, 3);
    bc << 1, 0, 0;
    
    Eigen::VectorXd S;
    std::cerr << "igl::copyleft::comiso::nrosy..." << std::endl;
    igl::copyleft::comiso::nrosy(V, F, b, bc, b_soft, w_soft, bc_soft, 4, 0.5, X1, S);

    // Find the orthogonal vector
    Eigen::MatrixXd B1, B2, B3;
    std::cerr << "igl::local_basis..." << std::endl;
    igl::local_basis(V, F, B1, B2, B3);
    std::cerr << "igl::rotate_vectors..." << std::endl;
    X2 = igl::rotate_vectors(X1, Eigen::VectorXd::Constant(1, igl::PI / 2), B1, B2);

    // Always work on the bisectors, it is more general
    std::cerr << "igl::compute_frame_field_bisectors..." << std::endl;
    igl::compute_frame_field_bisectors(V, F, X1, X2, BIS1, BIS2);

    // Comb the field, implicitly defining the seams
    std::cerr << "igl::comb_cross_field..." << std::endl;
    igl::comb_cross_field(V, F, BIS1, BIS2, BIS1_combed, BIS2_combed);

    // Find the integer mismatches
    std::cerr << "igl::cross_field_mismatch..." << std::endl;
    igl::cross_field_mismatch(V, F, BIS1_combed, BIS2_combed, true, MMatch);

    // Find the singularities
    std::cerr << "igl::find_cross_field_singularities..." << std::endl;
    igl::find_cross_field_singularities(V, F, MMatch, isSingularity, singularityIndex);

    // Cut the mesh, duplicating all vertices on the seams
    std::cerr << "igl::cut_mesh_from_singularities..." << std::endl;
    igl::cut_mesh_from_singularities(V, F, MMatch, Seams);

    // Comb the frame-field accordingly
    std::cerr << "igl::comb_frame_field..." << std::endl;
    igl::comb_frame_field(V, F, X1, X2, BIS1_combed, BIS2_combed, X1_combed, X2_combed);

    // Global parametrization
    std::cerr << "start miq..." << std::endl;
    std::vector<int> roundVertices;
    igl::copyleft::comiso::miq(V,
        F,
        X1_combed,
        X2_combed,
        MMatch,
        isSingularity,
        Seams,
        UV,
        FUV,
        parameters.gradientSize,
        stiffness,
        direct_round,
        iter,
        5,
        true,
        true,
        roundVertices,
        featuredEdges);
    std::cerr << "miq done" << std::endl;
    
    if (FUV.rows() != mesh.faceCount()) {
        std::cerr << "miq failed" << std::endl;
        return false;
    }
    
    faceNum = 0;
    for (HalfEdge::Face *face = mesh.firstFace(); nullptr != face; face = face->_next) {
        HalfEdge::HalfEdge *h0 = face->anyHalfEdge;
        HalfEdge::HalfEdge *h1 = h0->nextHalfEdge;
        HalfEdge::HalfEdge *h2 = h1->nextHalfEdge;
        const auto &triangleVertexIndices = FUV.row(faceNum++);
        const auto &v0 = UV.row(triangleVertexIndices[0]);
        const auto &v1 = UV.row(triangleVertexIndices[1]);
        const auto &v2 = UV.row(triangleVertexIndices[2]);
        h0->startVertexUv[0] = v0[0];
        h0->startVertexUv[1] = v0[1];
        h1->startVertexUv[0] = v1[0];
        h1->startVertexUv[1] = v1[1];
        h2->startVertexUv[0] = v2[0];
        h2->startVertexUv[1] = v2[1];
    }
    
    return true;
}

}

}