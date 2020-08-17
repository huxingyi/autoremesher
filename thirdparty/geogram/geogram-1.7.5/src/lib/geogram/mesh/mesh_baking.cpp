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

#include <geogram/mesh/mesh_baking.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_AABB.h>
#include <geogram/image/image_rasterizer.h>
#include <geogram/points/kd_tree.h>

namespace {
    using namespace GEO;

    /**
     * \brief Reads a (facet or vertex or facet corner) vector attribute 
     *  as a color.
     * \details If the vector attribute has less components than 4, then 
     *  the additional color components are set to (0 + \p bias)* \p scale.
     *  If the vector attribute has more components than 4, then the additional
     *  attribute components are ignored.
     * \param[out] C the color
     * \param[in] attribute a reference to the attribute.
     * \param[in] attrib_loc one of MESH_FACETS,MESH_VERTICES,MESH_FACET_CORNERS
     * \param[in] f the facet
     * \param[in] c the facet corner
     * \param[in] v the vertex
     * \param[in] bias optional value to be added to the attribute values 
     *  before storing them into the color components.
     * \param[in] scale optional value that scales the attribute values 
     *  after \p bias is added and before storing them into the color 
     *  components.
     */
    inline void get_attribute_as_color(
	Color& C,
	Attribute<double>& attribute,
	MeshElementsFlags attrib_loc,
	index_t f, index_t c, index_t v,
	double bias = 0.0,
	double scale = 1.0
    ) {
	index_t dim = attribute.dimension();
	index_t base = index_t(-1);
	if(attrib_loc == MESH_FACETS) {
	    base = f*dim;
	} else if(attrib_loc == MESH_VERTICES) {
	    base = v*dim;
	} else if(attrib_loc == MESH_FACET_CORNERS) {
	    base = c*dim;
	} else {
	    geo_assert_not_reached;
	}
	C.set_r(attribute[base]);
	C.set_g((dim >= 2) ? attribute[base+1] : 0.0);
	C.set_b((dim >= 3) ? attribute[base+2] : 0.0);
	C.set_a((dim >= 4) ? attribute[base+3] : 0.0);

	C.set_r(scale*(C.r() + bias));
	C.set_g(scale*(C.g() + bias));
	C.set_b(scale*(C.b() + bias));
	C.set_a(scale*(C.a() + bias));	
    }
}

namespace GEO {

   /**************************************************************************/
    
    void bake_mesh_facet_normals(Mesh* mesh, Image* target) {
	Attribute<double> tex_coord;
	tex_coord.bind_if_is_defined(
	    mesh->facet_corners.attributes(), "tex_coord"
	);
	geo_assert(tex_coord.is_bound() && tex_coord.dimension() == 2);
	ImageRasterizer rasterizer(target);
	for(index_t f: mesh->facets) {
	    vec3 N = normalize(Geom::mesh_facet_normal(*mesh, f));
	    Color C = 0.5*Color(N.x+1.0, N.y+1.0, N.z+1.0, 2.0);
	    index_t c1 = mesh->facets.corners_begin(f);
	    vec2 p1(tex_coord[2*c1], tex_coord[2*c1+1]);		
	    for(index_t c2 = c1+1;
		c2+1 < mesh->facets.corners_end(f); ++c2) {
		index_t c3 = c2+1;
		vec2 p2(tex_coord[2*c2], tex_coord[2*c2+1]);
		vec2 p3(tex_coord[2*c3], tex_coord[2*c3+1]);		
		rasterizer.triangle(
		    p1, C,
		    p2, C,
		    p3, C
		);
	    }
	}
    }    
    
   /**************************************************************************/

    void bake_mesh_vertex_normals(Mesh* mesh, Image* normal_map) {
	
	// Step 1: compute vertex normals.
	Attribute<double> N;
	N.create_vector_attribute(mesh->vertices.attributes(), "N", 3);
	for(index_t v: mesh->vertices) {
	    N[3*v]   = 0.0;
	    N[3*v+1] = 0.0;
	    N[3*v+2] = 0.0;
	}
	for(index_t f: mesh->facets) {
	    vec3 Nf = GEO::Geom::mesh_facet_normal(*mesh, f);
            for(index_t corner : mesh->facets.corners(f)) {
                index_t v = mesh->facet_corners.vertex(corner);
		N[3*v]   += Nf.x;
		N[3*v+1] += Nf.y ;
		N[3*v+2] += Nf.z;
            }
	}
	for(index_t v: mesh->vertices) {
	    vec3 Nf(N[3*v], N[3*v+1], N[3*v+2]);
	    Nf = normalize(Nf);
	    N[3*v]   = Nf.x;
	    N[3*v+1] = Nf.y;
	    N[3*v+2] = Nf.z;	    	    
	}
	
	// Step 2: bake interpolated vertex normals.
	bake_mesh_attribute(mesh, normal_map, N);
	
	// Step 3: normalize interpolated normals.
	FOR(y, normal_map->height()) {
	    FOR(x, normal_map->width()) {
		double* pix = normal_map->pixel_base_float64_ptr(x,y);
		vec3 Npix(pix);
		Npix = normalize(Npix);
		pix[0] = Npix.x;
		pix[1] = Npix.y;
		pix[2] = Npix.z;		
	    }
	}
	N.unbind();
	mesh->vertices.attributes().delete_attribute_store("N");
    }
    
   /**************************************************************************/
    
    void bake_mesh_attribute(
	Mesh* mesh, Image* target, Attribute<double>& attribute,
	double bias, double scale
    ) {
	Attribute<double> tex_coord;
	tex_coord.bind_if_is_defined(
	    mesh->facet_corners.attributes(), "tex_coord"
	);
	geo_assert(tex_coord.is_bound() && tex_coord.dimension() == 2);
	geo_assert(attribute.is_bound());
	
	MeshElementsFlags attrib_loc = MESH_NONE;

	if(attribute.manager() == &mesh->vertices.attributes()) {
	    attrib_loc = MESH_VERTICES;
	} else if(attribute.manager() == &mesh->facets.attributes()) {
	    attrib_loc = MESH_FACETS;
	} else if(attribute.manager() == &mesh->facet_corners.attributes()) {
	    attrib_loc = MESH_FACET_CORNERS;
	} else {
	    // Attribute is not a vertex/facet/facet_corner attribute
	    // or it is bound to a different mesh.
	    geo_assert_not_reached;
	}
	
	ImageRasterizer rasterizer(target);
	for(index_t f: mesh->facets) {
	    Color C1,C2,C3;
	    index_t c1 = mesh->facets.corners_begin(f);
	    vec2 p1(tex_coord[2*c1], tex_coord[2*c1+1]);
	    index_t v1 = mesh->facet_corners.vertex(c1);
	    get_attribute_as_color(
		C1, attribute, attrib_loc, f, c1, v1, bias, scale
	    );
	    for(index_t c2 = c1+1;
		c2+1 < mesh->facets.corners_end(f); ++c2) {
		index_t c3 = c2+1;
		vec2 p2(tex_coord[2*c2], tex_coord[2*c2+1]);
		vec2 p3(tex_coord[2*c3], tex_coord[2*c3+1]);
		index_t v2 = mesh->facet_corners.vertex(c2);
		index_t v3 = mesh->facet_corners.vertex(c3);
		get_attribute_as_color(
		    C2, attribute, attrib_loc, f, c2, v2, bias, scale
		);
		get_attribute_as_color(
		    C3, attribute, attrib_loc, f, c3, v3, bias, scale
		);
		rasterizer.triangle(
		    p1, C1,
		    p2, C2,
		    p3, C3
		);
	    }
	}
    }

   /**************************************************************************/
    
    void bake_mesh_geometry(Mesh* mesh, Image* target, bool clear) {
	geo_assert(target->component_encoding() == Image::FLOAT64);
	Attribute<double> point;
	point.bind_if_is_defined(mesh->vertices.attributes(), "point");
	geo_assert(point.is_bound());

	if(clear) {
	    double* base_mem = target->base_mem_float64_ptr();
	    size_t nb = target->nb_pixels()*target->components_per_pixel();
	    for(size_t i=0; i<nb; ++i) {
		base_mem[i] = Numeric::max_float64();
	    }
	}
	bake_mesh_attribute(mesh, target, point);
    }

   /**************************************************************************/
    
    void bake_mesh_facet_normals_indirect(
	Image* geometry, Image* target, Mesh* highres
    ) {
	geo_assert(geometry->dimension() == 2);
	geo_assert(target->dimension() == 2);
	geo_assert(
	    geometry->width() == target->width() &&
	    geometry->height() == target->height()
	);
	geo_assert(geometry->component_encoding() == Image::FLOAT64);

	MeshFacetsAABB AABB(*highres);	    
	vector<Color> normal(highres->facets.nb());
	for(index_t f: highres->facets) {
	    vec3 N = normalize(Geom::mesh_facet_normal(*highres,f));
	    normal[f] = Color(
		0.5*(N.x+1.0), 0.5*(N.y+1.0), 0.5*(N.z+1.0)
	    );
	}
	ImageRasterizer rasterizer(target);

	parallel_for(
	    0, target->height(),
	    [geometry, target, &AABB, &rasterizer, &normal](index_t y) {
		index_t nearest_facet = NO_FACET;
		vec3 nearest_point;
		double sq_dist;
		for(index_t x=0; x<target->width(); ++x) {
		    vec3 p((double*)(void*)geometry->pixel_base(x,y));
		    if(
			p[0] == Numeric::max_float64() &&
			p[1] == Numeric::max_float64() &&
			p[2] == Numeric::max_float64()
		    ) {
			continue;
		    }
		    if(nearest_facet == NO_FACET) {
			nearest_facet =
			    AABB.nearest_facet(p, nearest_point, sq_dist
			);
		    } else {
			sq_dist = length2(p - nearest_point);
			AABB.nearest_facet_with_hint(
			    p,nearest_facet,nearest_point,sq_dist
			);
		    }
		    rasterizer.set_pixel(
			int(x),int(y),normal[nearest_facet]
		    );
		}
	    }
	);
    }

   /**************************************************************************/

    void bake_mesh_points_attribute_indirect(
	Image* geometry, Image* target,
	Mesh* highres, Attribute<double>& attribute,
	double bias, double scale
    ) {
	geo_assert(geometry->dimension() == 2);
	geo_assert(target->dimension() == 2);
	geo_assert(
	    geometry->width() == target->width() &&
	    geometry->height() == target->height()
	);
	geo_assert(geometry->component_encoding() == Image::FLOAT64);
	geo_assert(highres->vertices.dimension() >= 3);
	
	NearestNeighborSearch_var kd_tree = new BalancedKdTree(3);
	
	kd_tree->set_points(
	    highres->vertices.nb(), highres->vertices.point_ptr(0), 
	    highres->vertices.dimension()
	);

	ImageRasterizer rasterizer(target);

	parallel_for(
	    0, target->height(),
	    [geometry, target, kd_tree,
	     &rasterizer, &attribute, bias, scale
	    ](index_t y) {
	    
		index_t nearest_vertex = NO_VERTEX;
		double sq_dist;
		Color C;
		for(index_t x=0; x<target->width(); ++x) {
		    vec3 p((double*)(void*)geometry->pixel_base(x,y));
		    if(
			p[0] == Numeric::max_float64() &&
			p[1] == Numeric::max_float64() &&
			p[2] == Numeric::max_float64()
		    ) {
			continue;
		    }
		    kd_tree->get_nearest_neighbors(
			1, p.data(), &nearest_vertex, &sq_dist
		    );
		    get_attribute_as_color(
			C, attribute, MESH_VERTICES,
			index_t(-1), index_t(-1), nearest_vertex,
			bias, scale
		    );
		    rasterizer.set_pixel(int(x),int(y),C);
		}
	    }
	);
    }
    
   /**************************************************************************/
    
}



