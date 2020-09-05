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

namespace AutoRemesher
{

bool Parameterizer::parameterize()
{
    GEO::Mesh M;
    
    M.vertices.set_dimension(3);
    std::vector<GEO::index_t> meshVertices(m_vertices->size());
    for (size_t i = 0; i < m_vertices->size(); ++i) {
        const auto &row = (*m_vertices)[i];
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
    
    for (size_t i = 0; i < m_triangles->size(); ++i) {
        const auto &row = (*m_triangles)[i];
        GEO::index_t f = M.facets.create_polygon(3);
        for (GEO::index_t lv = 0; lv < 3; ++lv) {
            M.facets.set_vertex(f, lv, meshVertices[row[lv]]);
        }
    }
    
    M.facets.connect();
    
    GEO::Attribute<GEO::vec3> B(M.facets.attributes(), "B");
    if (nullptr == m_triangleFieldVectors) {
        GEO::FrameField FF;
        FF.set_use_spatial_search(false);
        FF.create_from_surface_mesh(M, false, 60);
        const auto &frames = FF.frames();
        for (GEO::index_t f: M.facets) {
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
