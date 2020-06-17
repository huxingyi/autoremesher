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

#include "qex.h"
#include "../../src/MeshExtractorT.hh"
#include "../../src/QuadExtractorPostprocT.hh"
#include "../../src/Globals.hh"

#include <iostream>
#include <algorithm>
#include <iterator>
#include <cassert>

namespace QEx {
void extractPolyMesh(TriMesh_t in_triMesh, const_UVVector_t in_uvs, const_ValenceVector_t in_vertexValences,
                     QuadMesh_t out_quadMesh, QEx::PropMgr<QuadMesh>::LocalUvsPropertyManager &heLocalUvProp) {

    /*
     * Convert UV representation.
     */
    std::vector<double> uvs; uvs.reserve(in_uvs->size() * 2);
    for (std::vector<OpenMesh::Vec2d>::const_iterator it = in_uvs->begin(), it_end = in_uvs->end();
            it_end != it; ++it) {
        uvs.push_back((*it)[0]);
        uvs.push_back((*it)[1]);
    }

    out_quadMesh->request_face_status();
    out_quadMesh->request_halfedge_status();
    out_quadMesh->request_edge_status();
    out_quadMesh->request_vertex_status();

    QEx::MeshExtractorT<TriMesh> qme(*in_triMesh);
    qme.extract<QuadMesh>(uvs, heLocalUvProp, *out_quadMesh, in_vertexValences);
}

void mergePolyToQuad(QuadMesh_t inout_polyMesh, QEx::PropMgr<QuadMesh>::LocalUvsPropertyManager &heLocalUvProp) {
    QEx::QuadExtractorPostprocT<QuadMesh> qexPp(*inout_polyMesh, heLocalUvProp);
    qexPp.ngons_to_quads();

    inout_polyMesh->garbage_collection();

    inout_polyMesh->release_face_status();
    inout_polyMesh->release_halfedge_status();
    inout_polyMesh->release_edge_status();
    inout_polyMesh->release_vertex_status();
}
}

extern "C" {

namespace QEx {
void extractQuadMeshOM(TriMesh_t in_triMesh, const_UVVector_t in_uvs, const_ValenceVector_t in_vertexValences, QuadMesh_t out_quadMesh) {

    // Create temporary local UVs property.
    QEx::PropMgr<QuadMesh>::LocalUvsPropertyManager heLocalUvProp(
            *out_quadMesh, QEx::QExGlobals::LOCAL_UVS_HANDLE_NAME());

    extractPolyMesh(in_triMesh, in_uvs, in_vertexValences, out_quadMesh, heLocalUvProp);
    mergePolyToQuad(out_quadMesh, heLocalUvProp);
}

}

void qex_extractQuadMesh(qex_TriMesh const * in_triMesh, qex_Valence *in_vertexValences, qex_QuadMesh * out_quadMesh) {

    /*
     * Convert input into OpenMesh format.
     */
    std::vector<unsigned int> valences, *valences_ptr = 0;
    if (in_vertexValences != 0) {
        valences.reserve(in_triMesh->tri_count);
        std::copy(in_vertexValences, in_vertexValences + in_triMesh->tri_count, std::back_inserter(valences));
        valences_ptr = &valences;
    }

    /*
     * This estimate is accurate for genus 1 meshes. For all other reasonable meshes
     * it shouldn't be off by too much.
     */
    const unsigned int estimatedEdgeCount = in_triMesh->vertex_count + in_triMesh->tri_count;
    QEx::TriMesh triMesh;
    triMesh.reserve(in_triMesh->vertex_count, estimatedEdgeCount, in_triMesh->tri_count);

    /*
     * Transfer vertices.
     */
    for (qex_Point3 *vtx_it = in_triMesh->vertices, *vtx_end = in_triMesh->vertices + in_triMesh->vertex_count; vtx_it != vtx_end; ++vtx_it) {
        triMesh.add_vertex(QEx::TriMesh::Point(vtx_it->x[0], vtx_it->x[1], vtx_it->x[2]));
    }

    /*
     * Transfer faces (implicitly creating the edges).
     */
    for (qex_Tri *tri_it = in_triMesh->tris, *tri_end = in_triMesh->tris + in_triMesh->tri_count; tri_it != tri_end; ++tri_it) {
        QEx::TriMesh::FaceHandle fh = triMesh.add_face(
            triMesh.vertex_handle(tri_it->indices[0]),
            triMesh.vertex_handle(tri_it->indices[1]),
            triMesh.vertex_handle(tri_it->indices[2]));


    }

    /*
     * Transfer UVs.
     */
    std::vector<OpenMesh::Vec2d> uvs; uvs.resize(triMesh.n_halfedges());
    QEx::TriMesh::FaceIter f_it = triMesh.faces_begin();
    for (qex_UVTri *uvTri_it = in_triMesh->uvTris, *uvTri_end = in_triMesh->uvTris + in_triMesh->tri_count; uvTri_it != uvTri_end; ++uvTri_it, ++f_it) {
        qex_Point2 *uv_it = uvTri_it->uvs;
        for (QEx::TriMesh::FHIter fh_it = triMesh.fh_begin(*f_it), fh_end = triMesh.fh_end(*f_it); fh_it != fh_end; ++fh_it, ++uv_it) {
            uvs[fh_it->idx()] = OpenMesh::Vec2d(uv_it->x[0], uv_it->x[1]);
        }
    }

    QEx::QuadMesh quadMesh;
    QEx::extractQuadMeshOM(&triMesh, &uvs, valences_ptr, &quadMesh);

    /*
     * Convert output back into raw format.
     */

    /*
     * Transfer vertices.
     */
    out_quadMesh->vertex_count = quadMesh.n_vertices();
    out_quadMesh->vertices = static_cast<qex_Point3*>(malloc(sizeof(qex_Point3) * out_quadMesh->vertex_count));
    qex_Point3 *out_vtx = out_quadMesh->vertices;
    for (QEx::QuadMesh::VertexIter v_it = quadMesh.vertices_begin(), v_end = quadMesh.vertices_end(); v_it != v_end; ++v_it, ++out_vtx) {
        const QEx::QuadMesh::Point cur_point = quadMesh.point(*v_it);
        std::copy(cur_point.data(), cur_point.data() + 3, out_vtx->x);
    }

    /*
     * Transfer faces.
     */
    out_quadMesh->quad_count = quadMesh.n_faces();
    out_quadMesh->quads = static_cast<qex_Quad*>(malloc(sizeof(qex_Quad) * out_quadMesh->quad_count));
    qex_Quad *out_quad = out_quadMesh->quads;
    size_t nonQuadFaces = 0;
    for (QEx::QuadMesh::FaceIter f_it = quadMesh.faces_begin(), f_end = quadMesh.faces_end(); f_it != f_end; ++f_it, ++out_quad) {
        if (quadMesh.valence(*f_it) != 4) {
            ++nonQuadFaces;
            continue;
        }
        assert(quadMesh.valence(*f_it) == 4);
        QEx::QuadMesh::FaceVertexIter fv_it = quadMesh.fv_begin(*f_it);
        qex_Index *out_quad_idx = out_quad->indices;
        for (int i = 0; i < 4; ++i, ++fv_it, ++out_quad_idx) {
            assert(fv_it != quadMesh.fv_end(*f_it));
            assert(fv_it->is_valid());
            *out_quad_idx = static_cast<unsigned int>(fv_it->idx());
        }
    }

    if (nonQuadFaces > 0)
        std::cerr << "Skipped " << nonQuadFaces << " non-quad faces." << std::endl;
}

} /* extern "C" */
