#include <Eigen/Core>
#include <igl/jet.h>
#include <igl/per_face_normals.h>
#include <igl/unproject_onto_mesh.h>
#include <igl/edge_topology.h>
#include <igl/cut_mesh.h>
#include <directional/visualization_schemes.h>
#include <directional/glyph_lines_raw.h>
#include <directional/seam_lines.h>
#include <directional/line_cylinders.h>
#include <directional/read_raw_field.h>
#include <directional/write_raw_field.h>
#include <directional/curl_matching.h>
#include <directional/effort_to_indices.h>
#include <directional/singularity_spheres.h>
#include <directional/combing.h>
#include <directional/setup_parameterization.h>
#include <directional/parameterize.h>
#include <directional/cut_mesh_with_singularities.h>
#include <directional/power_field.h>
#include <directional/power_to_representative.h>
#include <directional/power_to_raw.h>
#include <AutoRemesher/Parametrization>
#include <iostream>

namespace AutoRemesher
{
    
namespace Parametrization
{

bool miq(HalfEdge::Mesh &mesh, const Parameters &parameters)
{
    Eigen::MatrixXd V(mesh.vertexCount(), 3);
    Eigen::MatrixXi F(mesh.faceCount(), 3);

    size_t vertexNum = 0;
    for (HalfEdge::Vertex *vertex = mesh.firstVertex(); nullptr != vertex; vertex = vertex->_next) {
        vertex->outputIndex = vertexNum;
        V.row(vertexNum++) << 
            vertex->position.x(), 
            vertex->position.y(), 
            vertex->position.z();
    }

    size_t faceNum = 0;
    for (HalfEdge::Face *face = mesh.firstFace(); nullptr != face; face = face->_next) {
        HalfEdge::HalfEdge *h0 = face->anyHalfEdge;
        HalfEdge::HalfEdge *h1 = h0->nextHalfEdge;
        HalfEdge::HalfEdge *h2 = h1->nextHalfEdge;
        F.row(faceNum) << 
            h0->startVertex->outputIndex, 
            h1->startVertex->outputIndex, 
            h2->startVertex->outputIndex;
        ++faceNum;
    }
    
    // Prepare the rawField
    
    Eigen::MatrixXcd powerField;
    int N = 4;
    
    {
        Eigen::VectorXi b;
        Eigen::MatrixXd bc;
        b.resize(0);
        bc.resize(0, 3);
        directional::power_field(V, F, b, bc, N, powerField);
    }
    
    Eigen::MatrixXd representative;
    directional::power_to_representative(V, F, powerField, N, representative);
    representative.rowwise().normalize();
    
    Eigen::MatrixXd rawField;
    directional::representative_to_raw(V, F, representative, N, rawField);
    
    Eigen::MatrixXi EV, FE, EF;
    Eigen::MatrixXd barycenters;
    
    igl::edge_topology(V, F, EV, FE, EF);
    igl::barycenter(V, F, barycenters);
    
    //combing and cutting
    Eigen::VectorXd curlNorm;
    Eigen::VectorXi matching, combedMatching;
    Eigen::VectorXd effort;
    directional::curl_matching(V, F, EV, EF, FE, rawField, matching, effort, curlNorm);
    
    Eigen::VectorXi singIndices, singVertices;
    directional::effort_to_indices(V, F, EV, EF, effort, matching, N, singVertices, singIndices);

    directional::ParameterizationData pd;
    Eigen::MatrixXd combedField;
    directional::cut_mesh_with_singularities(V, F, singVertices, pd.face2cut);
    directional::combing(V, F, EV, EF, FE, pd.face2cut, rawField, matching, combedField, combedMatching);
    //directional::principal_matching(V, F,EV, EF, FE, combedField, combedMatching, combedEffort);
    std::cout << "curlNorm max: " << curlNorm.maxCoeff() << std::endl;

    std::cout << "Setting up parameterization" << std::endl;
    
    Eigen::MatrixXd VMeshCut;
    Eigen::MatrixXi FMeshCut;
    directional::setup_parameterization(N, V, F, EV, EF, FE, combedMatching, singVertices, pd, VMeshCut, FMeshCut);
    
    Eigen::MatrixXd cutUVFull, cutUVRot;
    double lengthRatio = 0.01;
    bool isInteger = false;  //do not do translational seamless.
    std::cout << "Solving rotationally-seamless parameterization" << std::endl;
    directional::parameterize(V, F, FE, combedField, lengthRatio, pd, VMeshCut, FMeshCut, isInteger, cutUVRot);
    std::cout << "Done!" << std::endl;

    isInteger = true;  //do not do translational seamless.
    std::cout << "Solving fully-seamless parameterization" << std::endl;
    directional::parameterize(V, F, FE, combedField, lengthRatio, pd, VMeshCut, FMeshCut, isInteger,  cutUVFull);
    std::cout << "Done!" << std::endl;
    
    std::cout << "mesh.faceCount():" << mesh.faceCount() << std::endl;
    std::cout << "VMeshCut.rows():" << VMeshCut.rows() << std::endl;
    std::cout << "FMeshCut.rows():" << FMeshCut.rows() << std::endl;
    std::cout << "cutUVFull.rows():" << cutUVFull.rows() << std::endl;
    
    if (FMeshCut.rows() != mesh.faceCount()) {
        std::cerr << "miq failed" << std::endl;
        return false;
    }
    
    faceNum = 0;
    for (HalfEdge::Face *face = mesh.firstFace(); nullptr != face; face = face->_next) {
        HalfEdge::HalfEdge *h0 = face->anyHalfEdge;
        HalfEdge::HalfEdge *h1 = h0->nextHalfEdge;
        HalfEdge::HalfEdge *h2 = h1->nextHalfEdge;
        const auto &triangleVertexIndices = FMeshCut.row(faceNum++);
        const auto &v0 = cutUVFull.row(triangleVertexIndices[0]);
        const auto &v1 = cutUVFull.row(triangleVertexIndices[1]);
        const auto &v2 = cutUVFull.row(triangleVertexIndices[2]);
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