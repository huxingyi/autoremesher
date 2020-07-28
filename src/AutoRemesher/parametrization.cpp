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
#include <igl/copyleft/comiso/frame_field.h>
#include <igl/readOBJ.h>
#include <igl/writeOBJ.h>
#include <igl/readDMAT.h>
#include <igl/frame_field_deformer.h>
#include <igl/frame_to_cross_field.h>
#include <igl/PI.h>
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
    
    std::cerr << "miq preparing..." << std::endl;
    
    //Eigen::MatrixXd V_check;
    //Eigen::MatrixXi F_check;
    //igl::readOBJ("C:\\Users\\Jeremy\\Desktop\\bumpy-cube.obj", V_check, F_check);
    
    size_t vertexNum = 0;
    for (HalfEdge::Vertex *vertex = mesh.firstVertex(); nullptr != vertex; vertex = vertex->_next) {
        vertex->outputIndex = vertexNum;
        V.row(vertexNum++) << 
            vertex->position.x(), 
            vertex->position.y(), 
            vertex->position.z();
    }
    
    Eigen::VectorXi b(1);
    //b << 0;
    Eigen::MatrixXd bc1(1, 3);
    //bc1 << 1, 0, 0;
    Eigen::MatrixXd bc2(1, 3);
    //bc2 << 0, 1, 0;
    
    std::vector<std::vector<AutoRemesher::Vector3>> debugConstraintQuads;

    size_t faceNum = 0;
    for (HalfEdge::Face *face = mesh.firstFace(); nullptr != face; face = face->_next) {
        HalfEdge::HalfEdge *h0 = face->anyHalfEdge;
        HalfEdge::HalfEdge *h1 = h0->nextHalfEdge;
        HalfEdge::HalfEdge *h2 = h1->nextHalfEdge;
        F.row(faceNum) << 
            h0->startVertex->outputIndex, 
            h1->startVertex->outputIndex, 
            h2->startVertex->outputIndex;
        auto addFeatured = [&](HalfEdge::HalfEdge *h) {
            if (0 == h->startVertex->heightId)
                return false;
            if (h->startVertex->heightDirection.isZero())
                return false;
            auto v1 = Vector3::crossProduct(h->startVertex->heightDirection, h->startVertex->averageNormal);
            auto v2 = Vector3::crossProduct(h->startVertex->averageNormal, v1);
            //std::cerr << "add constraint[" << faceNum << "]" << v1 << " " << v2 << std::endl;
            b << faceNum;
            bc1 << v1.x(), v1.y(), v1.z();
            bc2 << v2.x(), v2.y(), v2.z();
            
            debugConstraintQuads.push_back({
                h->startVertex->position - v1 + v2,
                h->startVertex->position + v1 + v2,
                h->startVertex->position + v1 - v2,
                h->startVertex->position - v1 - v2
            });
            
            return true;
        };
        addFeatured(h0) || addFeatured(h1) || addFeatured(h2);
        ++faceNum;
    }
    
    {
        FILE *fp = fopen("C:\\Users\\Jeremy\\Desktop\\test-constraint-quads.obj", "wb");
        for (size_t i = 0; i < debugConstraintQuads.size(); ++i) {
            const auto &it = debugConstraintQuads[i];
            for (size_t j = 0; j < 4; ++j) {
                fprintf(fp, "v %f %f %f\n", it[j][0], it[j][1], it[j][2]);
            }
        }
        for (size_t i = 0, offset = 0; i < debugConstraintQuads.size(); ++i, offset += 4) {
            fprintf(fp, "f %zu %zu %zu %zu\n", 
                1 + offset,
                2 + offset,
                3 + offset,
                4 + offset);
        }
        fclose(fp);
    }

    // Global parametrization
    Eigen::MatrixXd UV;
    Eigen::MatrixXi FUV;
    
    // Input frame field constraints
    //Eigen::VectorXi b;
    //Eigen::MatrixXd bc1;
    //Eigen::MatrixXd bc2;

    //Eigen::MatrixXd temp;
    //igl::readDMAT("C:\\Users\\Jeremy\\Desktop\\bumpy-cube.dmat", temp);

    //b   = temp.block(0,0,temp.rows(),1).cast<int>();
    //bc1 = temp.block(0,1,temp.rows(),3);
    //bc2 = temp.block(0,4,temp.rows(),3);
    
    // Interpolated frame field
    Eigen::MatrixXd FF1, FF2;

    // Deformed mesh
    Eigen::MatrixXd V_deformed;

    // Frame field on deformed
    Eigen::MatrixXd FF1_deformed;
    Eigen::MatrixXd FF2_deformed;

    // Cross field on deformed
    Eigen::MatrixXd X1_deformed;
    Eigen::MatrixXd X2_deformed;
    
    // Interpolate the frame field
    igl::copyleft::comiso::frame_field(V, F, b, bc1, bc2, FF1, FF2);

    // Deform the mesh to transform the frame field in a cross field
    igl::frame_field_deformer(
        V,F,FF1,FF2,V_deformed,FF1_deformed,FF2_deformed);
    
    igl::writeOBJ("C:\\Users\\Jeremy\\Desktop\\test-deformed-F.obj", V_deformed, F);
    {
        {
            FILE *fp = fopen("C:\\Users\\Jeremy\\Desktop\\test-deformed.obj", "wb");
            size_t vertexNum = 0;
            for (HalfEdge::Vertex *vertex = mesh.firstVertex(); nullptr != vertex; vertex = vertex->_next) {
                vertex->outputIndex = vertexNum;
                const auto &row = V_deformed.row(vertexNum++);
                fprintf(fp, "v %f %f %f\n", row[0], row[1], row[2]);
            }
            for (HalfEdge::Face *face = mesh.firstFace(); nullptr != face; face = face->_next) {
                HalfEdge::HalfEdge *h0 = face->anyHalfEdge;
                HalfEdge::HalfEdge *h1 = h0->nextHalfEdge;
                HalfEdge::HalfEdge *h2 = h1->nextHalfEdge;
                fprintf(fp, "f %zu %zu %zu\n", 
                    1 + h0->startVertex->outputIndex, 
                    1 + h1->startVertex->outputIndex, 
                    1 + h2->startVertex->outputIndex);
            }
            fclose(fp);
        }
        {
            FILE *fp = fopen("C:\\Users\\Jeremy\\Desktop\\test-original.obj", "wb");
            size_t vertexNum = 0;
            for (HalfEdge::Vertex *vertex = mesh.firstVertex(); nullptr != vertex; vertex = vertex->_next) {
                vertex->outputIndex = vertexNum;
                const auto &row = V.row(vertexNum++);
                fprintf(fp, "v %f %f %f\n", row[0], row[1], row[2]);
            }
            for (HalfEdge::Face *face = mesh.firstFace(); nullptr != face; face = face->_next) {
                HalfEdge::HalfEdge *h0 = face->anyHalfEdge;
                HalfEdge::HalfEdge *h1 = h0->nextHalfEdge;
                HalfEdge::HalfEdge *h2 = h1->nextHalfEdge;
                fprintf(fp, "f %zu %zu %zu\n", 
                    1 + h0->startVertex->outputIndex, 
                    1 + h1->startVertex->outputIndex, 
                    1 + h2->startVertex->outputIndex);
            }
            fclose(fp);
        }
        printf("test-deformed.obj saved\n");
    }

    // Find the closest crossfield to the deformed frame field
    igl::frame_to_cross_field(V_deformed, F, FF1_deformed, FF2_deformed, X1_deformed);

    // Find a smooth crossfield that interpolates the deformed constraints
    Eigen::MatrixXd bc_x(b.size(),3);
    for (unsigned i=0; i<b.size();++i)
        bc_x.row(i) = X1_deformed.row(b(i));

    Eigen::VectorXd S;
    igl::copyleft::comiso::nrosy(
             V,
             F,
             b,
             bc_x,
             Eigen::VectorXi(),
             Eigen::VectorXd(),
             Eigen::MatrixXd(),
             4,
             0.5,
             X1_deformed,
             S);

    // The other representative of the cross field is simply rotated by 90 degrees
    Eigen::MatrixXd B1,B2,B3;
    igl::local_basis(V_deformed,F,B1,B2,B3);
    X2_deformed =
    igl::rotate_vectors(X1_deformed, Eigen::VectorXd::Constant(1,igl::PI/2), B1, B2);

    // Global seamless parametrization
    igl::copyleft::comiso::miq(V_deformed,
           F,
           X1_deformed,
           X2_deformed,
           UV,
           FUV,
           parameters.gradientSize,
           5.0,
           false,
           2);
    
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