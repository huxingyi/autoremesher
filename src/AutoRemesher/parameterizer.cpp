/*
 *  Copyright (c) 2020 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved. 
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
#include <AutoRemesher/Parameterizer>
#include <exploragram/hexdom/quad_cover.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/mesh_frame_field.h>
#include <AutoRemesher/RelativeHeight>
#include <unordered_map>
#include <unordered_set>

namespace AutoRemesher
{

bool Parameterizer::parameterize()
{
#if AUTO_REMESHER_DEV
    {
        FILE *fp = fopen("debug-input-for-parameterization.obj", "wb");
        for (size_t i = 0; i < m_vertices->size(); ++i) {
            const auto &vertex = (*m_vertices)[i];
            fprintf(fp, "v %f %f %f\n", vertex[0], vertex[1], vertex[2]);
        }
        for (size_t i = 0; i < m_triangles->size(); ++i) {
            const auto &indices = (*m_triangles)[i];
            fprintf(fp, "f %zu %zu %zu\n", indices[0] + 1, indices[1] + 1, indices[2] + 1);
        }
        fclose(fp);
    }
#endif
    
    RelativeHeight relativeHeight(m_vertices, m_triangles);
    relativeHeight.calculate();
    std::vector<double> *vertexRelativeHeights = relativeHeight.takeVertexRelativeHeights();
    std::vector<Vector3> *vertexNormals = relativeHeight.takeVertexNormals();
    std::map<size_t, std::vector<size_t>> *faceAroundVertexMap = relativeHeight.takeFaceAroundVertexMap();
#if AUTO_REMESHER_DEV
    {
        FILE *fp = fopen("debug-relativeheights.ply", "wb");
        fprintf(fp, "ply\n");
        fprintf(fp, "format ascii 1.0\n");
        fprintf(fp, "element vertex %zu\n", m_vertices->size());
        fprintf(fp, "property float x\n");
        fprintf(fp, "property float y\n");
        fprintf(fp, "property float z\n");
        fprintf(fp, "property uchar red\n");
        fprintf(fp, "property uchar green\n");
        fprintf(fp, "property uchar blue\n");
        fprintf(fp, "element face %zu\n", m_triangles->size());
        fprintf(fp, "property list uchar uint vertex_indices\n");
        fprintf(fp, "end_header\n");
        for (size_t vertexIndex = 0; vertexIndex < m_vertices->size(); ++vertexIndex) {
            const auto &vertex = (*m_vertices)[vertexIndex];
            int r = 255 * (*vertexRelativeHeights)[vertexIndex];
            if (r >= 0) {
                fprintf(fp, "%f %f %f %d %d %d\n", 
                    vertex.x(), vertex.y(), vertex.z(),
                    r, 0, 0);
            } else {
                fprintf(fp, "%f %f %f %d %d %d\n", 
                    vertex.x(), vertex.y(), vertex.z(),
                    0, r, 0);
            }
        }
        for (const auto &it: *m_triangles) {
            fprintf(fp, "3 %zu %zu %zu\n",
                it[0], it[1], it[2]);
        }
        fclose(fp);
    }
#endif
    double averageEdgeLength = relativeHeight.averageEdgeLength();
    std::vector<Vector3> deformedVertices = *m_vertices;
    for (size_t i = 0; i < deformedVertices.size(); ++i) {
        const auto &height = (*vertexRelativeHeights)[i];
        if (height <= 0)
            continue;
        deformedVertices[i] += (*vertexNormals)[i] * averageEdgeLength;
    }
    int deformRound = 2;
    while (--deformRound >= 0) {
        std::vector<Vector3> newPositions = deformedVertices;
        for (size_t i = 0; i < deformedVertices.size(); ++i) {
            const auto &height = (*vertexRelativeHeights)[i];
            if (height <= 0)
                continue;
            Vector3 position = deformedVertices[i];
            size_t vertexCount = 1;
            for (const auto &face: (*faceAroundVertexMap)[i]) {
                for (const auto &it: (*m_triangles)[face]) {
                    position += deformedVertices[it];
                    ++vertexCount;
                }
            }
            newPositions[i] = position / vertexCount;
        }
        deformedVertices = newPositions;
    }
#if AUTO_REMESHER_DEV
    {
        FILE *fp = fopen("debug-relativeheights-deformed.ply", "wb");
        fprintf(fp, "ply\n");
        fprintf(fp, "format ascii 1.0\n");
        fprintf(fp, "element vertex %zu\n", deformedVertices.size());
        fprintf(fp, "property float x\n");
        fprintf(fp, "property float y\n");
        fprintf(fp, "property float z\n");
        fprintf(fp, "property uchar red\n");
        fprintf(fp, "property uchar green\n");
        fprintf(fp, "property uchar blue\n");
        fprintf(fp, "element face %zu\n", m_triangles->size());
        fprintf(fp, "property list uchar uint vertex_indices\n");
        fprintf(fp, "end_header\n");
        for (size_t vertexIndex = 0; vertexIndex < deformedVertices.size(); ++vertexIndex) {
            const auto &vertex = deformedVertices[vertexIndex];
            int r = 255 * (*vertexRelativeHeights)[vertexIndex];
            if (r >= 0) {
                fprintf(fp, "%f %f %f %d %d %d\n", 
                    vertex.x(), vertex.y(), vertex.z(),
                    r, 0, 0);
            } else {
                fprintf(fp, "%f %f %f %d %d %d\n", 
                    vertex.x(), vertex.y(), vertex.z(),
                    0, r, 0);
            }
        }
        for (const auto &it: *m_triangles) {
            fprintf(fp, "3 %zu %zu %zu\n",
                it[0], it[1], it[2]);
        }
        fclose(fp);
    }
#endif
    delete vertexRelativeHeights;
    vertexRelativeHeights = nullptr;
    delete vertexNormals;
    vertexNormals = nullptr;
    delete faceAroundVertexMap;
    faceAroundVertexMap = nullptr;

    auto makeMesh = [](GEO::Mesh &M, const std::vector<Vector3> &vertices, const std::vector<std::vector<size_t>> &triangles) {
        M.vertices.set_dimension(3);
        std::vector<GEO::index_t> meshVertices(vertices.size());
        for (size_t i = 0; i < vertices.size(); ++i) {
            const auto &row = vertices[i];
            auto v = M.vertices.create_vertex();
            meshVertices[i] = v;
            double coords[] = {row[0], row[1], row[2]};
            if (M.vertices.single_precision()) {
                float *p = M.vertices.single_precision_point_ptr(v);
                for (GEO::index_t c = 0; c < 3; ++c) {
                    p[c] = float(coords[c]);
                }
            } else {
                double *p = M.vertices.point_ptr(v);
                for (GEO::index_t c = 0; c < 3; ++c) {
                    p[c] = coords[c];
                }
            }
        }
        
        for (size_t i = 0; i < triangles.size(); ++i) {
            const auto &row = triangles[i];
            GEO::index_t f = M.facets.create_polygon(3);
            for (GEO::index_t lv = 0; lv < 3; ++lv) {
                M.facets.set_vertex(f, lv, meshVertices[row[lv]]);
            }
        }
        
        M.facets.connect();
    };
    
    GEO::Mesh M;
    makeMesh(M, deformedVertices, *m_triangles);
    
    GEO::Attribute<GEO::vec3> B(M.facets.attributes(), "B");
    if (nullptr == m_triangleFieldVectors) {
        GEO::Mesh originalM;
        makeMesh(originalM, *m_vertices, *m_triangles);
    
        GEO::FrameField FF;
        FF.set_use_spatial_search(false);
        FF.create_from_surface_mesh(originalM, false, 45);
        const auto &frames = FF.frames();
        for (GEO::index_t f: originalM.facets) {
            B[f] = GEO::vec3(
                frames[9*f+0],
                frames[9*f+1],
                frames[9*f+2]		
            );
        }
    } else {
        for (size_t i = 0; i < m_triangleFieldVectors->size(); ++i) {
            const auto &row = (*m_triangleFieldVectors)[i];
            B[i] = GEO::vec3(row[0], row[1], row[2]);
        }
    }
    
    GEO::Attribute<GEO::vec2> U(M.facet_corners.attributes(), "U");
    bool constrain_hard_edges = true;
	bool do_brush = true;
    bool integer_constraints = true;
    GEO::GlobalParam2d::quad_cover(&M, B, U, m_scaling, constrain_hard_edges, do_brush, integer_constraints);

    delete m_triangleUvs;
    m_triangleUvs = new std::vector<std::vector<Vector2>>;
    m_triangleUvs->reserve(m_triangles->size());
    size_t faceCornerIndex = 0;
    for (size_t i = 0; i < m_triangles->size(); ++i) {
        const auto &v0 = U[faceCornerIndex++];
        const auto &v1 = U[faceCornerIndex++];
        const auto &v2 = U[faceCornerIndex++];
        m_triangleUvs->push_back({
            Vector2 {v0[0], v0[1]},
            Vector2 {v1[0], v1[1]},
            Vector2 {v2[0], v2[1]}
        });
    }
    
#if AUTO_REMESHER_DEV
    {
        FILE *fp = fopen("debug-uv.obj", "wb");
        for (size_t i = 0; i < m_triangles->size(); ++i) {
            const auto &uv = (*m_triangleUvs)[i];
            fprintf(fp, "v %f %f %f\n", uv[0][0], 0.0, uv[0][1]);
            fprintf(fp, "v %f %f %f\n", uv[1][0], 0.0, uv[1][1]);
            fprintf(fp, "v %f %f %f\n", uv[2][0], 0.0, uv[2][1]);
        }
        for (size_t i = 0; i < m_triangles->size(); ++i) {
            fprintf(fp, "f %zu %zu %zu\n", i * 3 + 1, i * 3 + 2, i * 3 + 3);
        }
        fclose(fp);
    }
    
    auto normalizeUv = [](double x) {
        return 0.5 + x * 0.5;
    };
    {
        FILE *fp = fopen("quadcover.obj", "wb");
        fprintf(fp, "mtllib quadcover.mtl\n");
        fprintf(fp, "usemtl quadcover\n");
        for (size_t i = 0; i < m_vertices->size(); ++i) {
            const auto &row = (*m_vertices)[i];
            fprintf(fp, "v %f %f %f\n", row[0], row[1], row[2]);
        }
        size_t faceCornerIndex = 0;
        for (size_t i = 0; i < m_triangles->size(); ++i) {
            const auto &v0 = U[faceCornerIndex++];
            const auto &v1 = U[faceCornerIndex++];
            const auto &v2 = U[faceCornerIndex++];
            fprintf(fp, "vt %f %f\n", normalizeUv(v0[0]), normalizeUv(v0[1]));
            fprintf(fp, "vt %f %f\n", normalizeUv(v1[0]), normalizeUv(v1[1]));
            fprintf(fp, "vt %f %f\n", normalizeUv(v2[0]), normalizeUv(v2[1]));
        }
        
        for (size_t i = 0; i < m_triangles->size(); ++i) {
            const auto &row = (*m_triangles)[i];
            fprintf(fp, "f %zu/%zu %zu/%zu %zu/%zu\n", 
                1 + row[0], i * 3 + 1,
                1 + row[1], i * 3 + 2,
                1 + row[2], i * 3 + 3);
        }
        fclose(fp);
    }
    {
        FILE *fp = fopen("quadcover.mtl", "wb");
        fprintf(fp, "newmtl quadcover\n");
        fprintf(fp, "Ka 1.000 1.000 1.000\n");
        fprintf(fp, "Kd 1.000 1.000 1.000\n");
        fprintf(fp, "Ks 0.000 0.000 0.000\n");
        fprintf(fp, "d 1.0\n");
        fprintf(fp, "illum 2\n");
        fprintf(fp, "map_Kd crossuv.png\n");
        fclose(fp);
    }
#endif
    
    return false;
}

}
