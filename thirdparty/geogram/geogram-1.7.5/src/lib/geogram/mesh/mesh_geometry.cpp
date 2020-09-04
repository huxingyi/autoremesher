/*
 *  Copyright (c) 2012-2014, Bruno Levy
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  * Neither the name of the ALICE Project-Team nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     Bruno.Levy@inria.fr
 *     http://www.loria.fr/~levy
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 */

#include <geogram/mesh/mesh_geometry.h>
#include <geogram/delaunay/LFS.h>
#include <geogram/voronoi/CVT.h>
#include <geogram/basic/attributes.h>
#include <geogram/basic/geometry.h>
#include <geogram/basic/logger.h>

namespace {

    using namespace GEO;

    /**
     * \brief Computes a sizing field using local feature size
     * \details The sizing field is stored into the vertex weights
     *  of the mesh
     * \param[in] M the mesh
     * \param[in] LFS the local feature size
     * \param[in] gradation power to be applied to the sizing field
     */
    void compute_sizing_field_lfs(
        Mesh& M, const LocalFeatureSize& LFS, double gradation
    ) {
        Attribute<double> weight(M.vertices.attributes(),"weight");
        for(index_t v: M.vertices) {
            double lfs2 = LFS.squared_lfs(M.vertices.point_ptr(v));
            double w = pow(lfs2, -2.0 * gradation);
            weight[v] = w;
        }
    }
}

/****************************************************************************/

namespace GEO {

    namespace Geom {

        vec3 mesh_facet_normal(const Mesh& M, index_t f) {
	    vec3 result(0.0, 0.0, 0.0);
	    index_t c1 = M.facets.corners_begin(f);
	    index_t v1 = M.facet_corners.vertex(c1);
	    const vec3& p1 = mesh_vertex(M, v1);
	    for(index_t c2=c1+1; c2<M.facets.corners_end(f); ++c2) {
		index_t c3 = M.facets.next_corner_around_facet(f,c2);
		index_t v2 = M.facet_corners.vertex(c2);
		index_t v3 = M.facet_corners.vertex(c3);
		const vec3& p2 = mesh_vertex(M, v2);
		const vec3& p3 = mesh_vertex(M, v3);
		result += cross(p2 - p1, p3 - p1);
	    }
	    return result;
	}

	double mesh_unsigned_normal_angle(
	    const Mesh& M, index_t f1, index_t f2
	) {
            vec3 n1 = mesh_facet_normal(M,f1);
            vec3 n2 = mesh_facet_normal(M,f2);
	    double l = length(n1)*length(n2);
            double cos_angle = (l > 1e-30) ? dot(n1, n2)/l : 1.0;
            // Numerical precision problem may occur, and generate
            // normalized dot products that are outside the valid
            // range of acos.
            geo_clamp(cos_angle, -1.0, 1.0);
            return acos(cos_angle);
	}
	
        double mesh_normal_angle(const Mesh& M, index_t c) {
            geo_debug_assert(M.facets.are_simplices());
            index_t f1 = c/3;
            index_t f2 = M.facet_corners.adjacent_facet(c);
            geo_debug_assert(f2 != NO_FACET);
            vec3 n1 = mesh_facet_normal(M,f1);
            vec3 n2 = mesh_facet_normal(M,f2);
	    double l = length(n1)*length(n2);
            double sign = 1.0;
            if(dot(cross(n1,n2),mesh_corner_vector(M,c)) > 0.0) {
                sign = -1.0;
            }
            double cos_angle = (l > 1e-30) ? dot(n1, n2)/l : 1.0;
            // Numerical precision problem may occur, and generate
            // normalized dot products that are outside the valid
            // range of acos.
            geo_clamp(cos_angle, -1.0, 1.0);
            return sign*acos(cos_angle);
        }

        double mesh_area(const Mesh& M, index_t dim) {
            double result = 0.0;
            for(index_t f: M.facets) {
                result += mesh_facet_area(M, f, dim);
            }
            return result;
        }
    }

    void compute_normals(Mesh& M) {
        if(M.vertices.dimension() < 6) {
            M.vertices.set_dimension(6);
        } else {
            for(index_t i: M.vertices) {
                Geom::mesh_vertex_normal_ref(M, i) = vec3(0.0, 0.0, 0.0);
            }
        }
        for(index_t f: M.facets) {
            vec3 N = Geom::mesh_facet_normal(M, f);
            for(index_t corner: M.facets.corners(f)) {
                index_t v = M.facet_corners.vertex(corner);
                Geom::mesh_vertex_normal_ref(M, v) += N;
            }
        }
        for(index_t i: M.vertices) {
            Geom::mesh_vertex_normal_ref(M, i) = normalize(
                Geom::mesh_vertex_normal(M, i)
            );
        }
    }

    void simple_Laplacian_smooth(Mesh& M, index_t nb_iter, bool normals_only) {
        geo_assert(M.vertices.dimension() >= 6);
        std::vector<vec3> p(M.vertices.nb());
        std::vector<double> c(M.vertices.nb());

        for(index_t k = 0; k < nb_iter; k++) {
            p.assign(M.vertices.nb(), vec3(0.0, 0.0, 0.0));
            c.assign(M.vertices.nb(), 0);
            for(index_t f: M.facets) {
                index_t b = M.facets.corners_begin(f);
                index_t e = M.facets.corners_end(f);
                for(index_t c1 = b; c1 != e; c1++) {
                    index_t c2 = (c1 == e - 1) ? b : c1 + 1;
                    index_t v1 = M.facet_corners.vertex(c1);
                    index_t v2 = M.facet_corners.vertex(c2);
                    if(v1 < v2) {
                        double a = 1.0;
                        c[v1] += a;
                        c[v2] += a;
                        if(normals_only) {
                            p[v1] += a * Geom::mesh_vertex_normal(M, v2);
                            p[v2] += a * Geom::mesh_vertex_normal(M, v1);
                        } else {
                            p[v1] += a * Geom::mesh_vertex(M, v2);
                            p[v2] += a * Geom::mesh_vertex(M, v1);
                        }
                    }
                }
            }
            for(index_t v: M.vertices) {
                if(normals_only) {
                    double l = length(p[v]);
                    if(l > 1e-30) {
                        Geom::mesh_vertex_normal_ref(M,v) = (1.0 / l) * p[v];
                    }
                } else {
                    Geom::mesh_vertex_ref(M, v) = 1.0 / c[v] * p[v];
                }
            }
        }
        if(!normals_only) {
            compute_normals(M);
        }
    }

    void get_bbox(const Mesh& M, double* xyzmin, double* xyzmax) {
        geo_assert(M.vertices.dimension() >= 3);
        for(index_t c = 0; c < 3; c++) {
            xyzmin[c] = Numeric::max_float64();
            xyzmax[c] = Numeric::min_float64();
        }
        for(index_t v: M.vertices) {
            const double* p = M.vertices.point_ptr(v);
            for(index_t c = 0; c < 3; c++) {
                xyzmin[c] = std::min(xyzmin[c], p[c]);
                xyzmax[c] = std::max(xyzmax[c], p[c]);
            }
        }
    }

    double bbox_diagonal(const Mesh& M) {
        geo_assert(M.vertices.dimension() >= 3);
        double xyzmin[3];
        double xyzmax[3];
        get_bbox(M, xyzmin, xyzmax);
        return ::sqrt(
            geo_sqr(xyzmax[0] - xyzmin[0]) +
            geo_sqr(xyzmax[1] - xyzmin[1]) +
            geo_sqr(xyzmax[2] - xyzmin[2])
        );
    }

    void set_anisotropy(Mesh& M, double s) {
        if(M.vertices.dimension() < 6) {
            compute_normals(M);
        }
        if(s == 0.0) {
            unset_anisotropy(M);
            return;
        }
        s *= bbox_diagonal(M);
        for(index_t i: M.vertices) {
            Geom::mesh_vertex_normal_ref(M, i) =
                s * normalize(Geom::mesh_vertex_normal(M, i));
        }
    }

    void unset_anisotropy(Mesh& M) {
        if(M.vertices.dimension() < 6) {
            return;
        }
        for(index_t i: M.vertices) {
            Geom::mesh_vertex_normal_ref(M, i) = normalize(
                Geom::mesh_vertex_normal(M, i)
            );
        }
    }

    void compute_sizing_field(
        Mesh& M, double gradation, index_t nb_lfs_samples
    ) {
        if(nb_lfs_samples != 0) {
            Logger::out("LFS") << "Sampling surface" << std::endl;
            CentroidalVoronoiTesselation CVT(&M, 3);
            CVT.compute_initial_sampling(nb_lfs_samples);
            Logger::out("LFS") << "Optimizing sampling (Lloyd)" << std::endl;
            CVT.Lloyd_iterations(5);
            Logger::out("LFS") << "Optimizing sampling (Newton)" << std::endl;
            CVT.Newton_iterations(10);
            Logger::out("LFS") << "Computing medial axis" << std::endl;
            LocalFeatureSize LFS(CVT.nb_points(), CVT.embedding(0));
            Logger::out("LFS") << "Computing sizing field" << std::endl;
            compute_sizing_field_lfs(M, LFS, gradation);
        } else {
            if(M.vertices.dimension() == 3) {
                LocalFeatureSize LFS(M.vertices.nb(), M.vertices.point_ptr(0));
                compute_sizing_field_lfs(M, LFS, gradation);
            } else {
                std::vector<double> pts;
                pts.reserve(M.vertices.nb() * 3);
                for(index_t v: M.vertices) {
                    pts.push_back(M.vertices.point_ptr(v)[0]);
                    pts.push_back(M.vertices.point_ptr(v)[1]);
                    pts.push_back(M.vertices.point_ptr(v)[2]);
                }
                LocalFeatureSize LFS(M.vertices.nb(), pts.data());
                compute_sizing_field_lfs(M, LFS, gradation);
            }
        }
    }

    void normalize_embedding_area(Mesh& M) {
        if(M.vertices.dimension() == 3) {
            if(M.vertices.attributes().is_defined("weight")) {
                M.vertices.attributes().delete_attribute_store("weight");
            }
            return;
        }
        Attribute<double> weight(M.vertices.attributes(), "weight");
        std::vector<double> area3d(M.vertices.nb(), 0.0);
        std::vector<double> areaNd(M.vertices.nb(), 0.0);
        for(index_t f: M.facets) {
            double A3d = Geom::mesh_facet_area(M, f, 3);
            double ANd = Geom::mesh_facet_area(M, f);
            for(index_t c: M.facets.corners(f)) {
                index_t v = M.facet_corners.vertex(c);
                area3d[v] += A3d;
                areaNd[v] += ANd;
            }
        }
        for(index_t v: M.vertices) {
            double A3d = area3d[v];
            double ANd = areaNd[v];
            ANd = std::max(ANd, 1e-6);
            double w = ::pow(A3d / ANd, 2.0);
            weight[v] = w;
        }
    }

    double mesh_cell_volume(
        const Mesh& M, index_t c
    ) {
        geo_assert(M.vertices.dimension() >= 3);

        //   Connectors are virtual cells, they
        //  do not have a geometry.
        if(M.cells.type(c) == MESH_CONNECTOR) {
            return 0.0;
        }
        
        //   Easy case: tetrahedra.
        if(M.cells.type(c) == MESH_TET) {
            const double* p0 = M.vertices.point_ptr(M.cells.vertex(c,0));
            const double* p1 = M.vertices.point_ptr(M.cells.vertex(c,1));
            const double* p2 = M.vertices.point_ptr(M.cells.vertex(c,2));
            const double* p3 = M.vertices.point_ptr(M.cells.vertex(c,3));
            return ::fabs(Geom::tetra_signed_volume(p0,p1,p2,p3));
        }

	//   Arbitrary cells are decomposed into tetrahedra, with one
	// vertex in the center of the cell, and one vertex in the
	// center of each face. This ensures that two adjacent cells
	// are not overlapping (if simply triangulating the faces,
	// there would be tiny overlaps / tiny gaps that would introduce
	// errors in the total volume of the mesh).
	//
	//  Note that the center point may fall outside the cell in some
	// degenerate configurations. It is not a problem since we compute
	// signed tetrahedra volumes, in such a way that overlapping volumes
	// will cancel-out in such a configuration.
	//  Therefore, we could take an arbitrary point as the enter point,
	// including the origin, but taking the center probably makes
	// computations more stable, by cancelling the translations.
        
        double result = 0.0;
        double center[3];
	index_t nbcv = M.cells.nb_vertices(c);
	center[0] = center[1] = center[2] = 0.0;
	for(index_t lv=0; lv<nbcv; ++lv) {
	    const double* p = M.vertices.point_ptr(M.cells.vertex(c,lv));
	    center[0] += p[0];
	    center[1] += p[1];
	    center[2] += p[2];
	}
	center[0] /= double(nbcv);
	center[1] /= double(nbcv);
	center[2] /= double(nbcv);
        for(index_t lf=0; lf<M.cells.nb_facets(c); ++lf) {
	    index_t nbcfv = M.cells.facet_nb_vertices(c,lf);
	    if(nbcfv == 3) {
		const double* p1 =
		    M.vertices.point_ptr(M.cells.facet_vertex(c,lf,0));
		const double* p2 =
		    M.vertices.point_ptr(M.cells.facet_vertex(c,lf,1));
		const double* p3 =
		    M.vertices.point_ptr(M.cells.facet_vertex(c,lf,2));
		result += Geom::tetra_signed_volume(center, p1, p2, p3);
	    } else {
		double facet_center[3];
		facet_center[0] = facet_center[1] = facet_center[2] = 0.0;
		for(index_t lfv=0; lfv<nbcfv; ++lfv) {
		    const double* p =
			M.vertices.point_ptr(M.cells.facet_vertex(c,lf,lfv));
		    facet_center[0] += p[0];
		    facet_center[1] += p[1];
		    facet_center[2] += p[2];
		}
		facet_center[0] /= double(nbcfv);
		facet_center[1] /= double(nbcfv);
		facet_center[2] /= double(nbcfv);
		for(index_t lfv1=0; lfv1<nbcfv; ++lfv1) {
		    index_t lfv2 = (lfv1 + 1) % nbcfv;
		    const double* p1 =
			M.vertices.point_ptr(M.cells.facet_vertex(c,lf,lfv1));
		    const double* p2 =
			M.vertices.point_ptr(M.cells.facet_vertex(c,lf,lfv2));
		    result +=
			Geom::tetra_signed_volume(center, facet_center, p1, p2);
		}
	    }
        }
        return ::fabs(result);
    }

    
    double mesh_cells_volume(const Mesh& M) {
        double result = 0.0;
        for(index_t c: M.cells) {
            result += mesh_cell_volume(M,c);
        }
        return result;
    }

    vec3 GEOGRAM_API mesh_cell_facet_normal(
        const Mesh& M, index_t c, index_t lf
    ) {
        geo_debug_assert(M.vertices.dimension() >= 3);
        geo_debug_assert(c < M.cells.nb());
        geo_debug_assert(lf < M.cells.nb_facets(c));

        index_t v1 = M.cells.facet_vertex(c,lf,0);
        index_t v2 = M.cells.facet_vertex(c,lf,1);
        index_t v3 = M.cells.facet_vertex(c,lf,2);

        const vec3& p1 = Geom::mesh_vertex(M,v1);
        const vec3& p2 = Geom::mesh_vertex(M,v2);
        const vec3& p3 = Geom::mesh_vertex(M,v3);

        return cross(p2 - p1, p3 - p1);        
    }
    

    
    double surface_average_edge_length(const Mesh& M) {
        double result = 0.0;
        index_t count = 0;
        for(index_t f: M.facets) {
            for(index_t c1: M.facets.corners(f)) {
                index_t c2 = M.facets.next_corner_around_facet(f,c1);
                index_t v1 = M.facet_corners.vertex(c1);
                index_t v2 = M.facet_corners.vertex(c2);
                result += Geom::distance(
                    M.vertices.point_ptr(v1),
                    M.vertices.point_ptr(v2),
                    coord_index_t(M.vertices.dimension())
                );
                ++count;
            }
        }
        if(count != 0) {
            result /= double(count);
        }
        return result;
    }
    
}

