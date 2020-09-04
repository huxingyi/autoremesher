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

#include <geogram/mesh/mesh_intersection.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_AABB.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_reorder.h>
#include <geogram/mesh/mesh_fill_holes.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_preprocessing.h>
#include <geogram/mesh/mesh_tetrahedralize.h>
#include <geogram/mesh/triangle_intersection.h>
#include <geogram/mesh/index.h>
#include <geogram/voronoi/convex_cell.h>
#include <geogram/numerics/predicates.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/stopwatch.h>
#include <stack>

// WIP: exact implementation
//    - robust preficate
//    - symbolic perturbation
//    - rules for keeping facet are probably more subtle
// #define ROBUST_BOOLEANS

#ifdef ROBUST_BOOLEANS
   // This makes sure the compiler will not optimize y = a*x+b
   // with fused multiply-add, this would break the exact
   // predicates.
#   ifdef GEO_COMPILER_MSVC
#      pragma fp_contract(off)
#   endif

#   define FPG_UNCERTAIN_VALUE 0
#   include <geogram/numerics/expansion_nt.h>
    namespace GEO {
        #include <geogram/numerics/predicates/side_for_boolean.h>
    }
#endif



/****************************************************************************/

// TODO:
//    1: fully robust predicate - think (hard) about SOS perturbations.
//    2: triangulate facets: use ear cutting in ppal plane whenever
//       it is a good idea.

namespace {

    using namespace GEO;

    enum MeshBooleanOperation { Union, Intersection, Difference };

    /**
     * \brief Appends an axis-aligned box to this mesh.
     * \param[in,out] to a pointer to a mesh
     * \param[in] box an array of 6 doubles with
     *   xmin, ymin, zmin, xmax, ymax, zmax
     */
    void append_box(Mesh* to, double* box) {

	static index_t fv[12][3] = {
	    { 3, 7, 6 },
	    { 3, 6, 2 },
	    { 1, 3, 0 },
	    { 0, 3, 2 },
	    { 5, 7, 1 },
	    { 1, 7, 3 },
	    { 4, 6, 5 },
	    { 6, 7, 5 }, 
	    { 4, 5, 0 },
	    { 0, 5, 1 },
	    { 2, 6, 4 },
	    { 2, 4, 0 }
	};

	index_t v_ofs = to->vertices.nb();

	double x0 = box[0];
	double y0 = box[1];
	double z0 = box[2];
	double x1 = box[3];
	double y1 = box[4];
	double z1 = box[5];	
	
	to->vertices.create_vertex(vec3(x0,y0,z0).data());
	to->vertices.create_vertex(vec3(x0,y0,z1).data());
	to->vertices.create_vertex(vec3(x0,y1,z0).data());
	to->vertices.create_vertex(vec3(x0,y1,z1).data());
	to->vertices.create_vertex(vec3(x1,y0,z0).data());
	to->vertices.create_vertex(vec3(x1,y0,z1).data());
	to->vertices.create_vertex(vec3(x1,y1,z0).data());
	to->vertices.create_vertex(vec3(x1,y1,z1).data());	

	for(index_t f=0; f<12; ++f) {
	    to->facets.create_triangle(
		fv[f][0] + v_ofs, fv[f][1] + v_ofs, fv[f][2] + v_ofs
	    );
	}
	
    }
    
    /**
     * \brief Appends a surface mesh to this mesh.
     * \param[in,out] to a pointer to a mesh
     * \param[in] comp the mesh to be appended to \p to
     */
    void append_surface_mesh(Mesh* to, Mesh* comp) {
        index_t offset = to->vertices.nb();
        for(index_t v=0; v<comp->vertices.nb(); ++v) {
            to->vertices.create_vertex(comp->vertices.point_ptr(v));
        }
        for(index_t f=0; f<comp->facets.nb(); ++f) {
            index_t nf = to->facets.create_polygon(comp->facets.nb_vertices(f));
            for(index_t lv=0; lv<comp->facets.nb_vertices(f); ++lv) {
                to->facets.set_vertex(
                    nf,lv,comp->facets.vertex(f,lv) + offset
                );
            }
        }
    }

    /**
     * \brief Re-assign the cell region attribute in such a way that
     *  region 0 corresponds to the exterior region.
     * \param[in] mesh the volumetric mesh to be fixed.
     */
    void fix_regions(Mesh* mesh) {
	bool single_region = true;
	Attribute<index_t> region(mesh->cells.attributes(),"region");
	
	// Determine the index of the exterior region
	// (with tetgen it varies !!)
	index_t ext_region = index_t(-1);
	for(index_t t: mesh->cells) {
	    for(index_t lf=0; lf<4; ++lf) {
		if(mesh->cells.tet_adjacent(t,lf) == index_t(-1)) {
		    ext_region = region[t];
		    break;
		}
	    }
	}

	for(index_t t: mesh->cells) {
	    if(region[t] != ext_region) {
		single_region = false;
		break;
	    }
	}

	if(single_region) {
	    // Case 1: everything same region (not using complementary),
	    // set to everything inside object.
	    // NOTE: supposing there is no internal boundary.
	    for(index_t t: mesh->cells) {
		region[t] = 1;
	    }
	} else {
	    // Case 2: using complementary. 
	    for(index_t t: mesh->cells) {
		region[t] = (region[t] == ext_region) ? 0 : 1;
	    }
	}
    }

    
    /**
     * \brief Computes the supporting plane of a tet facet.
     * \param[in] M a pointer to a tetrahedralized mesh
     * \param[in] t a tetrahedron
     * \param[in] lf a local tetrahedron facet index, in 0..3
     * \return the equation of the supporting plane of facet \p f in
     *  tetrahedron \p t
     */
    inline VBW::vec4 tet_facet_plane(Mesh* M, index_t t, index_t lf) {
	index_t i1 = M->cells.tet_facet_vertex(t,lf,0);
	index_t i2 = M->cells.tet_facet_vertex(t,lf,1);
	index_t i3 = M->cells.tet_facet_vertex(t,lf,2);
	vec3 p1(M->vertices.point_ptr(i1));
	vec3 p2(M->vertices.point_ptr(i2));
	vec3 p3(M->vertices.point_ptr(i3));
	vec3 N = cross(p2-p1, p3-p1);
	return VBW::vec4(N.x, N.y, N.z, -dot(N,p1));
    }

    /**
     * \brief Gets the three vertices of a facet.
     * \param[out] p1 , p2 , p3 pointers to the coordinates of the three 
     *  vertices of the facet.
     * \param[in] A , B the two tetrahedral meshes.
     * \param[in] f the tet facet. If f is smaller than 4*A->cells.nb(), then
     *  f corresponds to a mesh A facet, else f correponds to a mesh B facet.
     */
    inline void get_tet_facet_vertices(
	double*& p1, double*& p2, double*& p3,
	Mesh* A, Mesh* B, index_t f
    ) {
	index_t f_offset = A->cells.nb()*4;
	if(f < f_offset) {
	    index_t t = f/4;
	    index_t lf = f%4;
	    p1 = A->vertices.point_ptr(A->cells.tet_facet_vertex(t,lf,0));
	    p2 = A->vertices.point_ptr(A->cells.tet_facet_vertex(t,lf,1));
	    p3 = A->vertices.point_ptr(A->cells.tet_facet_vertex(t,lf,2));
	} else {
	    index_t t = (f - f_offset)/4;
	    index_t lf = (f - f_offset)%4;
	    p1 = B->vertices.point_ptr(B->cells.tet_facet_vertex(t,lf,0));
	    p2 = B->vertices.point_ptr(B->cells.tet_facet_vertex(t,lf,1));
	    p3 = B->vertices.point_ptr(B->cells.tet_facet_vertex(t,lf,2));
	}
    }

    /**
     * \brief Computes the intersection between two sorted trindices.
     * \param[out] result the resulting trindex, empty slots are set 
     *  to index_t(-1)
     * \param[in] A , B the two trindices to be intersected. 
     *  They need to be sorted.
     */
    void trindex_intersection(
	trindex& result, const trindex& A, const trindex& B
    ) {
	result.indices[0] = index_t(-1);
	result.indices[1] = index_t(-1);
	result.indices[2] = index_t(-1);
	index_t i=0;
	index_t j=0;
	index_t k=0;
	while(i < 3 && j < 3) {
	    if(A.indices[i] < B.indices[j]) {
		++i;
	    } else if(B.indices[j] < A.indices[i]) {
		++j;
	    } else {
		result.indices[k] = A.indices[i];
		++i;
		++j;
		++k;
	    }
	}
    }

    /**
     * \brief Inserts an integer in a trindex,
     *  considered as a set of three values.
     * \details index_t(-1) is considered as the
     *  empty slot value. The three values are not
     *  sorted.
     * \param[in,out] T the trindex
     * \param[in] val the value to be inserted
     */
    void trindex_insert(trindex& T, index_t val) {
	for(index_t i=0; i<3; ++i) {
	    if(T.indices[i] == val) {
		return;
	    }
	}
	for(index_t i=0; i<3; ++i) {
	    if(T.indices[i] == index_t(-1)) {
		T.indices[i] = val;
		return;
	    }
	}
    }

    /**
     * \brief Gets the number of initialized entries
     *  in a trindex.
     * \return the number of entries different from
     *  index_t(-1)
     */
    index_t trindex_size(const trindex& A) {
	return
	    index_t(A.indices[0] != index_t(-1)) +
	    index_t(A.indices[1] != index_t(-1)) +
	    index_t(A.indices[2] != index_t(-1)) ;	    
    }

    /**
     * \brief Creates a unique identifier for a tet facet in one of two meshes.
     * \details There are in general two facets with the same three vertices,
     *  this function uniquely selects one of them.
     * \param[in] A , B the two tetrahedral meshes.
     * \param[in] f the tet facet. If f is smaller than 4*A->cells.nb(), then
     *  f corresponds to a mesh A facet, else f correponds to a mesh B facet.
     * \return a unique id for the mesh facet.
     */
    index_t normalized_tet_facet_index(Mesh* A, Mesh* B, index_t f) {
	index_t f_offset = A->cells.nb()*4;
	index_t result = f;
	if(f < f_offset) {
	    index_t t  = f/4;
	    index_t lf = f%4;
	    index_t t_adj = A->cells.tet_adjacent(t,lf);
	    if(t_adj != index_t(-1) && t_adj < t) {
		index_t lf_adj = A->cells.find_tet_adjacent(t_adj,t);
		result = t_adj*4+lf_adj;
	    }
	} else {
	    index_t t  = (f - f_offset)/4;
	    index_t lf = (f - f_offset)%4;
	    index_t t_adj = B->cells.tet_adjacent(t,lf);
	    if(t_adj != index_t(-1) && t_adj < t) {
		index_t lf_adj = B->cells.find_tet_adjacent(t_adj,t);
		result = f_offset+t_adj*4+lf_adj;
	    }
	}
	return result;
    }
    
    /**
     * \brief Creates a unique identifier for a vertex at the intersection 
     *  of three facets.
     * \param[out] K the created unique identifier, stored in a trindex.
     * \param[in] A , B the two tetrahedral meshes
     * \param[in] f an array of three facet indices. If f[i] is 
     *  smaller than 4*A->cells.nb(), then f[i] corresponds to a 
     *  mesh A facet, else f[i]-4*A->cells.nb() correponds to a mesh B facet.
     */
    void get_vertex_key(trindex& K, Mesh* A, Mesh* B, index_t f[3]) {
	trindex T[3];

	// Get the three vertices of each facet.
	for(index_t i=0; i<3; ++i) {
	    if(f[i] < 4*A->cells.nb()) {
		index_t t  = f[i]/4;
		index_t lf = f[i]%4;
		T[i] = trindex(
		    A->cells.tet_facet_vertex(t,lf,0),
		    A->cells.tet_facet_vertex(t,lf,1),
		    A->cells.tet_facet_vertex(t,lf,2)
		);
	    } else {
		index_t t  = (f[i] - 4*A->cells.nb()) / 4;
		index_t lf = (f[i] - 4*A->cells.nb()) % 4;
		T[i] = trindex(
		    B->cells.tet_facet_vertex(t,lf,0) + A->vertices.nb(),
		    B->cells.tet_facet_vertex(t,lf,1) + A->vertices.nb(),
		    B->cells.tet_facet_vertex(t,lf,2) + A->vertices.nb()	
		);
	    }
	}

	trindex T01, T12, T20, T012;
	trindex_intersection(T01, T[0], T[1]);
	trindex_intersection(T12, T[1], T[2]);
	trindex_intersection(T20, T[2], T[0]);
	trindex_intersection(T012, T01, T12);

	K.indices[0] = index_t(-1);
	K.indices[1] = index_t(-1);
	K.indices[2] = index_t(-1);

	// Note: three facet indices do not uniquely identify a vertex in
	// a tet mesh: there can be arbitrary many facets incicent to the
	// same vertex, and any triplet of them identifies the vertex, so
	// we do things differently:
	// There are two configurations:
	// * If there exists a vertex index common to the three facets, then
	//  what we have is an original vertex v of one of the input meshes,
	//  then we encode the vertex as [v, -1, -1]
	// * Else, the vertex is the intersection between an edge (v1,v2)
	//  of the input meshes and a facet f, then we encode the
	//  vertex as [v1, v2, -2-f]
	
	if(trindex_size(T012) == 1) {
	    K = T012;
	} else if (trindex_size(T01) == 2) {
	    K = trindex(
		T01.indices[0], T01.indices[1],
		index_t(-2-int(normalized_tet_facet_index(A,B,f[2])))
	    );
	} else if (trindex_size(T12) == 2) {
	    K = trindex(
		T12.indices[0], T12.indices[1],
		index_t(-2-int(normalized_tet_facet_index(A,B,f[0])))		
	    );
	} else if (trindex_size(T20) == 2) {
	    K = trindex(
		T20.indices[0], T20.indices[1],
		index_t(-2-int(normalized_tet_facet_index(A,B,f[1])))		
	    );	
	} else {
	    geo_assert_not_reached;
	}
    }

    /**
     * \brief Determines whether a tet-tet intersection facet should be kept
     *  in the result of a boolean operation.
     * \param[in] operation one of Union,Intersection,Difference.
     * \param[in] A the first operand, as a tetrahedralized mesh.
     * \param[in] t1 a tetrahedron of A.
     * \param[in] A_region a tet attribute with 1 if tet is in A and 0 otherwise
     * \param[in] B the second operand, as a tetrahedralized mesh.
     * \param[in] t2 a tetrahedron of B.
     * \param[in] B_region a tet attribute with 1 if tet is in B and 0 otherwise
     * \param[in] f if in [0..3] a local facet index of A, if in [4..7] a local
     *   facet index of B.
     * \retval  1 if facet should be kept.
     * \retval  0 if facet should not be kept.
     * \retval -1 if facet should be kept in reverse orientation.
     */
    int facet_status(
	MeshBooleanOperation operation,	
	Mesh* A, index_t t1, Attribute<index_t>& A_region,
	Mesh* B, index_t t2, Attribute<index_t>& B_region,
	index_t f
    ) {
	bool in_A = (A_region[t1] != 0);
	bool in_B = (B_region[t2] != 0);	
	bool on_A_border = false;
	bool on_B_border = false;
	if(f < 4) {
	    if(A_region[t1] != 0) {
		index_t t_adj = A->cells.tet_adjacent(t1,f);
		if(t_adj == index_t(-1) || A_region[t_adj] == 0) {
		    on_A_border = true;
		}
	    }
	} else {
	    if(B_region[t2] != 0) {
		index_t t_adj = B->cells.tet_adjacent(t2,f-4);
		if(t_adj == index_t(-1) || B_region[t_adj] == 0) {
		    on_B_border = true;
		}
	    }
	}
	int result = 0;
	switch(operation) {
	    case Union:
		result = int((on_A_border && !in_B) || (on_B_border && !in_A));
		break;
	    case Intersection:
		result = int((on_A_border && in_B) || (on_B_border && in_A));
		break;
	    case Difference:
		result = int((on_A_border && !in_B));
		if(on_B_border && in_A) {
		    result = -1;
		}
		break;
	}
	return result;
    }

    /**
     * \brief Computes the intersection between a straight line and a plane.
     * \param[in] q1 , q2 two points on the straight line.
     * \param[in] p1 , p2 , p3 three points on the plane.
     * \return the intersection between the straight line and the plane.
     */
    vec3 line_plane_intersection(
	const vec3& q1, const vec3& q2,
	const vec3& p1, const vec3& p2, const vec3& p3
    ) {
	// TODO: there is probably simpler/faster
	// for instance:
	// | p1 p2 p3 (tq2 + (1-t)q1) |
	// | 1  1  1  1               | = 0

	// Normal vector of the triangle
	vec3 N = cross(p2-p1,p3-p1);

	// Plane equation: dot(N,p) = dot(N,p1)
	// dot(q1 + t(q2-q1),N) = dot(p1, N)
	// dot(q1,N) + t dot(q2-q1,N) = dot(p1,N)
	// t = (dot(p1,N) - dot(q1,N)) / (dot(q2,N) - dot(q1,N))
	double denom = dot(q2,N) - dot(q1,N);
	double t = 0.0;

	// Not supposed to happen but happens in the difficult 'tangent cube'
	// examples. Need to work on this to figure out why.
	if(::fabs(denom) < 1e-20) {
	    std::cerr << "N          = " << N << std::endl;
	    std::cerr << "[q1 q2]    = " << q1 << " || " << q2 << std::endl;
	    std::cerr << "/\\" << std::endl;
	    std::cerr << "[p1 p2 p3] = "
		      << p1 << " || " << p2 << " || " << p3 << std::endl;
	    t = 0.5;
	} else {
	    t = (dot(p1,N) - dot(q1,N)) / denom;
	}
	return t*q2 + (1.0-t)*q1;
    }

    
    /**
     * \brief Gets the geometry of a vertex of a convex cell, using the
     *  combinatorial information and initial meshes.
     * \details One may use instead C.triangle_point(t), but the computations
     *  done here are more numerically stable, because they use the 
     *  combinatorial information and use existing vertices as much as possible.
     * \param[in] A , B the two operands of the boolean operation.
     * \param[in] C the ConvexCell.
     * \param[in] t a triangle (dual vertex) of the Convex Cell.
     * \return the geometry of the dual vertex.
     */
    vec3 get_cell_point(Mesh* A, Mesh* B, ConvexCell& C, VBW::ushort t) {
	// We could simply return C.triangle_point(t) here, but this
	// recomputes tet vertices as intersection between three tet faces,
	// even for vertices we already know ! While this works in most cases,
	// it is a bit stupid to take the risk of encountering numerical
	// instabilities for points that we already know. It requires a bit
	// of combinatorial trickery to retreive them, but well..
	// In addition, we also handle the segment /\ facet configuration,
	// that may be more stable than computing the intersection of three
	// facets, in particular if the two facets that share the segment
	// are nearly coplanar.
	
	VBW::Triangle T = C.get_triangle(t);
	index_t F[3]; 
	F[0] = C.v_global_index(T.i);
	F[1] = C.v_global_index(T.j);
	F[2] = C.v_global_index(T.k);
	trindex K;
	get_vertex_key(K, A, B, F);
	vec3 result;
	if(trindex_size(K) == 1) {
	    // Case 1/2: Original vertex in mesh A or B.
	    if(K.indices[0] < A->vertices.nb()) {
		result = vec3(A->vertices.point_ptr(K.indices[0]));
	    } else {
		result = vec3(B->vertices.point_ptr(
				  K.indices[0] - A->vertices.nb()
			    )
		);		
	    }
	} else {
	    // Case 2/2: Intersection between edge and triangular facet.
	    index_t i1 = K.indices[0];
	    index_t i2 = K.indices[1];
	    index_t v_offset = A->vertices.nb();	
	    vec3 p1(
		i1 < v_offset ?
		A->vertices.point_ptr(i1) : B->vertices.point_ptr(i1 - v_offset)
	    );
	    vec3 p2(
		i2 < v_offset ?
		A->vertices.point_ptr(i2) : B->vertices.point_ptr(i2 - v_offset)
	    );
	    index_t f  = index_t(-int(K.indices[2])-2);
	    double* pq1;
	    double* pq2;
	    double* pq3;
	    get_tet_facet_vertices(pq1,pq2,pq3,A,B,f);
	    vec3 q1(pq1);
	    vec3 q2(pq2);
	    vec3 q3(pq3);
	    result = line_plane_intersection(p1, p2, q1, q2, q3);
	}
	return result;
    }
    
    /**
     * \brief Glues the facets of a surface mesh resulting from a boolean 
     *   operation.
     * \details All facet corners with the same combinatorial information
     *   are replaced with a single vertex.
     * \param[in,out] result the surface mesh.
     * \param[in,out] corner_trindices for each mesh facet corner, 
     *  the associated combinatorial information, stored in a trindex. The
     *  vector is sorted on exit.
     */
    void boolean_ops_glue_result(
	Mesh* result,
	vector< std::pair<index_t, trindex> >& corner_trindices
    ) {
	Logger::out("Booleans") << "Glue" << std::endl;
	
	// Sort the (corner index, trindex) pairs by trindex
	GEO::sort(
	    corner_trindices.begin(), corner_trindices.end(),
	    [](
		const std::pair<index_t, trindex>& v1,
		const std::pair<index_t, trindex>& v2
	    ) {
		return v1.second < v2.second;
	    } // There may be already a function in the STL to do that.
	);

	//   The corners with the same trindex are now consecutive
	// in the sorted vector. Traverse all the [i1, i2[ sequences
	// of facet corners and make them point to the same vertex.
	index_t i1 = 0;
	while(i1 < result->facet_corners.nb()) {
	    index_t i2 = i1;
	    while(i2 < result->facet_corners.nb() &&
		  corner_trindices[i2].second == corner_trindices[i1].second
	    ) {
		++i2;
	    }
	    index_t v=result->facet_corners.vertex(corner_trindices[i1].first);
	    if(trindex_size(corner_trindices[i1].second) != 0) {
		for(index_t i=i1; i<i2; ++i) {
		    result->facet_corners.set_vertex(
			corner_trindices[i].first, v
		    );
		}
	    }
	    i1 = i2;
	}
	result->facets.connect();
	result->vertices.remove_isolated();
    }
    
    /**
     * \brief Simplify a surface mesh resulting from a boolean operation.
     * \details Attemps to merge the facets f that have the same 
     *  result_region[f] value.
     * \param[in,out] result the surface mesh.
     * \param[in] result_region for each facet, indicates the original
     *  tet facet it comes from.
     */
    void boolean_ops_simplify_result(
	Mesh* result, vector<index_t>& result_region
    ) {
	Logger::out("Booleans") << "Simplify" << std::endl;
	
	index_t nb_f = result->facets.nb();
	index_t nb_v = result->vertices.nb();
	vector<index_t> f_status(nb_f, 0);
	vector<trindex> v_regions(
	    nb_v, trindex(index_t(-1), index_t(-1), index_t(-1))
	);
	for(index_t f=0; f<nb_f; ++f) {
	    for(index_t lv=0; lv<result->facets.nb_vertices(f); ++lv) {
		index_t v = result->facets.vertex(f,lv);
		trindex_insert(v_regions[v], result_region[f]);
	    }
	}

	// For each facet f of the result mesh ...
	vector<index_t> facet_vertices;
	vector<index_t> comp_facets;
	for(index_t f=0; f<nb_f; ++f) {
	    if(f_status[f] == 0) {
		index_t region = result_region[f];
		std::stack<index_t> S;
		std::map<index_t, index_t> border_map;
		comp_facets.resize(0);
		comp_facets.push_back(f);
		S.push(f);
		f_status[f] = 1;
		bool complex_border = false;

		// If f was not traversed yet, traverse the set of facets
		// incident to f that come from the same initial facet (from
		// A or from B).
		while(!S.empty()) {
		    index_t cur_f = S.top();
		    S.pop();
		    index_t n = result->facets.nb_vertices(cur_f);
		    for(index_t le=0; le<n; ++le) {
			index_t adj_f = result->facets.adjacent(cur_f,le);
			if(
			    adj_f == index_t(-1) ||
			    result_region[adj_f] != region
			) {
			    // Link the vertices of the region border.
			    index_t v1 = result->facets.vertex(cur_f, le);
			    index_t v2 = result->facets.vertex(cur_f, (le+1)%n);
			    // If the border has an '8' configurations,
			    // we are not going to simplify.
			    if(border_map.find(v1) != border_map.end()) {
				complex_border = true;
			    }
			    border_map[v1] = v2;
			} else {
			    if(f_status[adj_f] == 0) {
				S.push(adj_f);
				comp_facets.push_back(adj_f);
				f_status[adj_f] = 1;
			    }
			}
		    }
		}
		facet_vertices.resize(0);
		if(border_map.size() == 0) {
		    complex_border = true;
		} else {
		    // Traverse the linked list of border vertices,
		    // among them find the ones that are incident to
		    // 3 regions and more.
		    index_t nb_v_on_border = 0;
		    index_t v = border_map.begin()->first;
		    index_t cnt = 0;
		    do {
			if(trindex_size(v_regions[v]) >= 3) {
			    facet_vertices.push_back(v);
			}
			++nb_v_on_border;
			v = border_map[v];
			++cnt;
			// Sanity check (happens sometimes, do not know
			// why yet).
			if(cnt > 100000) {
			    complex_border = true;
			    break;
			}
		    } while(v != border_map.begin()->first);
		    // Test whether the region has a single border, if this
		    // is not the case, we are not going to simplify.
		    if(nb_v_on_border != border_map.size()) {
			complex_border = true;
		    }
		}
		if(facet_vertices.size() <= 2) {
		    complex_border = true;
		}
		if(complex_border) {
		    //Logger::warn("Booleans") << "Encountered complex border"
		    //  << std::endl;
		    // Mark the initial result facets as 2, which means
		    // 'belonging to complex facet, that was not simplified'
		    for(index_t ff: comp_facets) {
			f_status[ff] = 2;
		    }
		} else {
		    // Create the new facet, using all border vertices
		    // incident to 3 regions and more.
		    index_t new_f = result->facets.create_polygon(
			facet_vertices.size()
		    );
		    for(index_t lv=0; lv<facet_vertices.size(); ++lv) {
			result->facets.set_vertex(
			    new_f, lv, facet_vertices[lv]
			);
		    }
		}
	    }
	}
	// Delete the initial facets in the result mesh.
	// For facets that have a complex border, we keep initial subfacets.
	f_status.resize(result->facets.nb(), 0);
	for(index_t& f: f_status) {
	    if(f == 2) {
		f = 0;
	    }
	}
	result->facets.delete_elements(f_status, true);
	result->facets.connect();
    }


    
    /**
     * \brief Computes a boolean operation between two surface meshes.
     * \details We use a brute-force approach: we tetrahedralize the meshes, 
     *  compute the intersections between all tetrahedra, and select the facets
     *  that are on the border of the intersection. This may sound overkill,
     *  but the benefit is that we only need to compute intersections between
     *  convex objects (the tetrahedra), which can be easily done with the
     *  ConvexCell class.
     * \param[out] result the resulting mesh.
     * \param[in] A , B the two operands.
     * \param[in] operation one of Union, Intersection, Difference.
     */
    void mesh_boolean_operation(
	Mesh* result, Mesh* A, Mesh* B,	MeshBooleanOperation operation
    ) {
	// --------------------------------------------------------------
	// Phase I: pre-processing, compute bbox,
	// create tet meshes.
	// --------------------------------------------------------------	
	
	// Compute common bbox of both meshes
	double box[6];
	get_bbox(*A,box, box+3);
	{
	    double B_box[6];
	    get_bbox(*B, B_box, B_box+3);
	    box[0] = std::min(box[0], B_box[0]);
	    box[1] = std::min(box[1], B_box[1]);
	    box[2] = std::min(box[2], B_box[2]);
	    box[3] = std::max(box[3], B_box[3]);
	    box[4] = std::max(box[4], B_box[4]);
	    box[5] = std::max(box[5], B_box[5]);
	    double d = 0;
	    FOR(i,3) {
		d = std::max(d, box[i+3]-box[i]);
	    }
	    // enlarge a bit
	    d *= 1e-1;
	    box[0] -= d;
	    box[1] -= d;
	    box[2] -= d;
	    box[3] += d;
	    box[4] += d;
	    box[5] += d;	    
	}

	bool refine = true;

	// Comment on append_box() used below:
	// We decompose the operations as disjoint unions
	// When it involves CA (resp. CB), we mesh the space
	// between A (resp. B) and the common bbox of A and B.
	// A /\ B
	//    => no box needed
	// A U B = (A /\ CB) U (A /\ B) U (CA /\ B)
	//    => box needed for both A and B
	// A \ B = (A /\ CB)
	//    => box needed for B
	
	// Create two volumetric meshes
	Mesh A_vol;
	append_surface_mesh(&A_vol, A);
	if(operation == Union) { // see comment above
	    append_box(&A_vol, box);
	}
	A_vol.facets.connect();
	if(!mesh_tetrahedralize(A_vol, false, refine, 2.0, true)) {
	    Logger::err("Booleans") << "Could not tetrahedralize operand A"
				    << std::endl;
	    return;
	}
	fix_regions(&A_vol);
	Attribute<index_t> A_region(A_vol.cells.attributes(), "region");	

	Mesh B_vol;	
	append_surface_mesh(&B_vol, B);
	if(operation == Union || operation == Difference) { // see comment above
	    append_box(&B_vol, box);
	}
	B_vol.facets.connect();	
	if(!mesh_tetrahedralize(B_vol, false, refine, 2.0, true)) {
	    Logger::err("Booleans") << "Could not tetrahedralize operand B"
				    << std::endl;
	    return;
	}
	fix_regions(&B_vol);
	Attribute<index_t> B_region(B_vol.cells.attributes(), "region");

	double t0 = SystemStopwatch::now();

	// --------------------------------------------------------------	
	// Phase II: create Axis-Aligned Bounding Box trees (AABBs)
	// --------------------------------------------------------------	
	
	Logger::out("Booleans") << "Create AABB for A" << std::endl;
	MeshCellsAABB A_AABB(A_vol);

	Logger::out("Booleans") << "Create AABB for B" << std::endl;	
	MeshCellsAABB B_AABB(B_vol);
	
	double AABB_time = SystemStopwatch::now() - t0;
	t0 += AABB_time;
	Logger::out("Booleans")
	    << "AABB       time: " << AABB_time << "s" << std::endl;

	// ------------------------------------------------------------------	
	// Phase III: Compute candidate pairs of potentially intersected tets
	// ------------------------------------------------------------------	
	
	Logger::out("Booleans") << "Get intersection candidates" << std::endl;
	// Determine intersection candidates and store them in a vector
	// so that we can process them in parallel later.
	vector<index_t> intersection_candidates;
	A_AABB.compute_other_cell_bbox_intersections(
	    &B_AABB,
	    [&](index_t t1, index_t t2) {
		if(A_region[t1] == 0 && B_region[t2] == 0) {
		    return;
		}
		intersection_candidates.push_back(t1);
		intersection_candidates.push_back(t2);		
	    }
	);

	double candidates_time = SystemStopwatch::now() - t0;
	t0 += candidates_time;
	Logger::out("Booleans")
	    << "candidates time: " << candidates_time << "s" << std::endl;
	
	vector<index_t> result_region;
	result->clear();


	// --------------------------------------------------------------	
	// Phase IV: compute actual tet-tet intersections
	// --------------------------------------------------------------	
	
	Logger::out("Booleans") << "Compute intersections" << std::endl;

	Process::spinlock lock = GEOGRAM_SPINLOCK_INIT;

	// Associates a facet corner index and its symbolic information.
	// Used at the next step to glue the facets (by merging the vertices
	// that have the same symbolic representation).
	vector< std::pair<index_t, trindex> > corner_trindices;
	
	parallel_for_slice(
	    0, intersection_candidates.size()/2,
	    [&](index_t from, index_t to) {
		vector<index_t> facet_triangles;
		ConvexCell C(VBW::WithVGlobal);
		C.use_exact_predicates(true);
		for(index_t i=from; i<to; ++i) {
		    index_t t1 = intersection_candidates[2*i];
		    index_t t2 = intersection_candidates[2*i+1];

		    // Initialize a ConvexCell with the geometry and
		    // combinatorial information of t1
		    C.clear();
		    C.init_with_tet(
			tet_facet_plane(&A_vol, t1, 0), 
			tet_facet_plane(&A_vol, t1, 1),
			tet_facet_plane(&A_vol, t1, 2),
			tet_facet_plane(&A_vol, t1, 3),
			t1*4, t1*4+1, t1*4+2, t1*4+3
		    );

		    // Clip the ConvexCell with the four facets of t2
		    for(index_t lf=0; lf<4; ++lf) {
			C.clip_by_plane(
			    tet_facet_plane(&B_vol, t2, lf),
			    4*A_vol.cells.nb() + t2*4+lf
#ifdef ROBUST_BOOLEANS
                            // WIP: exact predicate (does not work yet...
			    //  needs consistent symbolic perturbation !)
			    // This version of clip_by_plane() takes
			    // the geometric predicate used to determine
			    // the vertices that are in conflict.
			    ,[&](VBW::ushort t, VBW::ushort v) {
				 double* p1; double* p2; double* p3;
				 double* p4; double* p5; double* p6;
				 double* p7; double* p8; double* p9;
				 double* pa; double* pb; double* pc;
				 VBW::Triangle T = C.get_triangle(t);
				 get_tet_facet_vertices(
				     p1, p2, p3,
				     &A_vol, &B_vol, C.v_global_index(T.i)
				 );
				 get_tet_facet_vertices(
				     p4, p5, p6,
				     &A_vol, &B_vol, C.v_global_index(T.j)
				 );
				 get_tet_facet_vertices(
				     p7, p8, p9,
				     &A_vol, &B_vol, C.v_global_index(T.k)
				 );
				 get_tet_facet_vertices(
				     pa, pb, pc,
				     &A_vol, &B_vol, C.v_global_index(v)
				 );
				 int s = side_for_boolean_3d_filter(
				     p1,p2,p3,p4,p5,p6,p7,p8,p9,pa,pb,pc
				 );
				 if(s > 0) { return true; }
				 if(s < 0) { return false; }
				 Sign result = side_for_boolean_3d_exact(
				     p1,p2,p3,p4,p5,p6,p7,p8,p9,pa,pb,pc
				 );

				 // Attempt for symbolic perturbation:
				 // "pull" intersection towards one of the
				 // two tets that share a given facet.
				 index_t f = C.v_global_index(v);
				 index_t f_norm = normalized_tet_facet_index(
				     &A_vol, &B_vol, f
				 );
				 
				 bool in_A = (A_region[t1] != 0);
				 bool in_B = (B_region[t2] != 0);
				 return(
				     (result == POSITIVE)
				     // || ((f != f_norm) && (result == ZERO))
				 );
			   }
#endif			    
			);
		    }
		    C.compute_geometry();
		    
		    // Store the intersection facets in the result surface
		    // Iterate on the (up to) 8 Voronoi facets
		    for(index_t f=0; f<8; ++f) {
			// +1 if facet kept
			//  0 if facet skipped (internal facet)
			// -1 if facet kept with reverse orientation
			int f_status = facet_status(
			    operation,
			    &A_vol, t1, A_region, &B_vol, t2, B_region, f
			);
			if(f_status == 0) {
			    continue;
			}
			// Get the primal triangles associated with each
			// Voronoi vertex (we need them to create the
			// combinatorial information).
			facet_triangles.resize(0);
			C.for_each_Voronoi_vertex(
			    VBW::index_t(f+1), // +1 because
			                       // 0 is vertex at infinity
			    [&facet_triangles](index_t t) {
				facet_triangles.push_back(t);
			    }
			);
			    
			if(facet_triangles.size() > 2) {
			    if(f_status < 0) {
				std::reverse(
				    facet_triangles.begin(),
				    facet_triangles.end()
				);
			    }
			    Process::acquire_spinlock(lock);
			    index_t v_offset = result->vertices.nb();

			    // Create the vertices
			    for(index_t ii=0; ii<facet_triangles.size(); ++ii){
				result->vertices.create_vertex(
				    get_cell_point(
					&A_vol, &B_vol, C,
					VBW::ushort(facet_triangles[ii])
				    ).data()
				);
			    }
			    
			    // Create the facet
			    index_t new_f = result->facets.create_polygon(
				facet_triangles.size()
			    );

			    // Store the tet facet index this facet comes from,
			    // Used at step VI to simplify the resulting mesh
			    result_region.push_back(
				C.v_global_index(VBW::index_t(f+1))
			    );

			    // Get the combinatorial representation of
			    // the vertices, used at step V to gue the facets.
			    for(index_t ii=0; ii<facet_triangles.size(); ++ii){
				result->facets.set_vertex(
				    new_f, ii, v_offset+ii
				);
				VBW::Triangle Ti = C.get_triangle(
				    VBW::index_t(facet_triangles[ii])
				);
				index_t F[3]; 
				F[0] = C.v_global_index(Ti.i);
				F[1] = C.v_global_index(Ti.j);
				F[2] = C.v_global_index(Ti.k);
				trindex K;
				get_vertex_key(K, &A_vol, &B_vol, F);
				corner_trindices.push_back(
				    std::make_pair(corner_trindices.size(), K)
				);
			    }
			    Process::release_spinlock(lock);
			}
		    }
		}
	    }
	);

	// --------------------------------------------------------------
	// Phase V: glue the facets, using combinatorial information
	// (unique vertex ids).
	// --------------------------------------------------------------	

	boolean_ops_glue_result(result, corner_trindices);
	corner_trindices.clear(); // we no longer need them.
	
	// --------------------------------------------------------------
	// Phase VI: simplify the facets
	// --------------------------------------------------------------	

	boolean_ops_simplify_result(result, result_region);
	
	double isect_time = SystemStopwatch::now() - t0;
	t0 += isect_time;

	Logger::out("Booleans")
	    << "AABB       time: " << AABB_time << "s" << std::endl;
	Logger::out("Booleans")
	    << "candidates time: " << candidates_time << "s" << std::endl;
	Logger::out("Booleans")
	    << "isect      time: " << isect_time << "s" << std::endl;		
    }
}

namespace GEO {
    void mesh_union(Mesh& result, Mesh& A, Mesh& B) {
	mesh_boolean_operation(&result, &A, &B, Union);
    }

    void mesh_intersection(Mesh& result, Mesh& A, Mesh& B) {
	mesh_boolean_operation(&result, &A, &B, Intersection);	
    }

    void mesh_difference(Mesh& result, Mesh& A, Mesh& B) {
	mesh_boolean_operation(&result, &A, &B, Difference);		
    }
}

/****************************************************************************/

namespace {

    using namespace GEO;

    /**
     * \brief Computes the intersection between two triangular facets in
     *  a mesh
     * \param[in] M the mesh
     * \param[in] f1 index of the first facet
     * \param[in] f2 index of the second facet
     * \param[out] sym symbolic representation of the intersection (if any)
     * \return true if facets \p f1 and \p f2 have an intersection, false
     *  otherwise
     */
    bool triangles_intersect(
        const Mesh& M, index_t f1, index_t f2,
        vector<TriangleIsect>& sym
    ) {
        geo_debug_assert(M.facets.nb_vertices(f1) == 3);
        geo_debug_assert(M.facets.nb_vertices(f2) == 3);
        index_t c1 = M.facets.corners_begin(f1);
        const vec3& p1 = Geom::mesh_vertex(M, M.facet_corners.vertex(c1));
        const vec3& p2 = Geom::mesh_vertex(M, M.facet_corners.vertex(c1 + 1));
        const vec3& p3 = Geom::mesh_vertex(M, M.facet_corners.vertex(c1 + 2));
        index_t c2 = M.facets.corners_begin(f2);
        const vec3& q1 = Geom::mesh_vertex(M, M.facet_corners.vertex(c2));
        const vec3& q2 = Geom::mesh_vertex(M, M.facet_corners.vertex(c2 + 1));
        const vec3& q3 = Geom::mesh_vertex(M, M.facet_corners.vertex(c2 + 2));
        return triangles_intersections(p1, p2, p3, q1, q2, q3, sym);
    }

    /**
     * \brief Tests whether two facets are adjacent
     * \details Two facets are adjacents if they share an edge
     * \param[in] M the mesh
     * \param[in] f1 index of the first facet
     * \param[in] f2 index of the second facet
     * \return true if facets \p f1 and \p f2 share an edge, false
     *  otherwise
     */
    bool facets_are_adjacent(const Mesh& M, index_t f1, index_t f2) {
        if(f1 == f2) {
            return true;
        }
        for(index_t c: M.facets.corners(f1)) {
            if(M.facet_corners.adjacent_facet(c) == f2) {
                return true;
            }
        }
        return false;
    }

    /**
     * \brief Action class for storing intersections when traversing
     *  a AABBTree.
     */
    class StoreIntersections {
    public:
        /**
         * \brief Constructs the StoreIntersections
         * \param[in] M the mesh
         * \param[out] has_isect the flag that indicates for each facet
         *  whether it has intersections
         */
        StoreIntersections(
            const Mesh& M, vector<index_t>& has_isect
        ) :
            M_(M),
            has_intersection_(has_isect) {
            has_intersection_.assign(M_.facets.nb(), 0);
        }

        /**
         * \brief Determines the intersections between two facets
         * \details It is a callback for AABBTree traversal
         * \param[in] f1 index of the first facet
         * \param[in] f2 index of the second facet
         */
        void operator() (index_t f1, index_t f2) {
            // TODO: if facets are adjacents, test for
            // coplanarity.
            if(
                !facets_are_adjacent(M_, f1, f2) &&
                f1 != f2 && triangles_intersect(M_, f1, f2, sym_)
            ) {
                has_intersection_[f1] = 1;
                has_intersection_[f2] = 1;
            }
        }

    private:
        const Mesh& M_;
        vector<index_t>& has_intersection_;
        vector<TriangleIsect> sym_;
    };

    /**
     * \brief Deletes the intersecting facets from a mesh
     * \param[in] M the mesh
     * \param[in] nb_neigh number of rings of facets to delete around
     *  the intersecting facets
     * \return the number of facets that were deleted
     */
    index_t remove_intersecting_facets(Mesh& M, index_t nb_neigh = 0) {
        geo_assert(M.vertices.dimension() >= 3);
        mesh_repair(M, MESH_REPAIR_DEFAULT);  // it repairs and triangulates.

        vector<index_t> has_intersection;
        StoreIntersections action(M, has_intersection);
        MeshFacetsAABB AABB(M);
        AABB.compute_facet_bbox_intersections(action);

        for(index_t i = 1; i <= nb_neigh; i++) {
            for(index_t f: M.facets) {
                if(has_intersection[f] == 0) {
                    for(index_t c: M.facets.corners(f)) {
                        index_t f2 = M.facet_corners.adjacent_facet(c);
                        if(f2 != NO_FACET && has_intersection[f2] == i) {
                            has_intersection[f] = i + 1;
                            break;
                        }
                    }
                }
            }
        }

        index_t count = 0;
        for(index_t f = 0; f < has_intersection.size(); f++) {
            if(has_intersection[f] != 0) {
                count++;
            }
        }

        if(count != 0) {
            M.facets.delete_elements(has_intersection);
            Logger::out("Intersect")
                << "Removed " << count << " facets"
                << std::endl;
        } 
        return count;
    }
}

/****************************************************************************/

namespace GEO {

    void mesh_remove_intersections(
        Mesh& M, index_t max_iter
    ) {
	if(M.facets.nb() == 0) {
	    return;
	}
	
        fill_holes(M, Numeric::max_float64());

        const double A = Geom::mesh_area(M);
        const double min_component_area = 5.0 * 0.001 * A;
        const index_t min_component_facets = 3;

        for(index_t i = 0; i < max_iter; i++) {
            index_t count = remove_intersecting_facets(M);
            if(count == 0) {
                return;
            }
            // Needs to be done before hole filling (to have clean hole borders)
            remove_small_connected_components(
                M, min_component_area, min_component_facets
            );
            fill_holes(M, Numeric::max_float64());
            // Needs to be done after hole filling
            // (removing bridges may generate small connected components)
            remove_small_connected_components(
                M, min_component_area, min_component_facets
            );
        }

        for(index_t neigh_size = 1; neigh_size < 4; neigh_size++) {
            for(index_t i = 0; i < max_iter; i++) {
                index_t count = remove_intersecting_facets(M, neigh_size);
                if(count == 0) {
                    return;
                }
                // Needs to be done before hole filling
                // (to have clean hole borders)
                remove_small_connected_components(
                    M, min_component_area, min_component_facets
                );
                fill_holes(M, Numeric::max_float64());
                // Needs to be done after hole filling
                // (removing bridges may generate small connected components)
                remove_small_connected_components(
                    M, min_component_area, min_component_facets
                );
            }
        }
    }
}

