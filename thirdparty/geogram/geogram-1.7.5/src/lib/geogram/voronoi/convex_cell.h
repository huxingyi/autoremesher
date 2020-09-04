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

#ifndef GEOGRAM_VORONOI_CONVEX_CELL
#define GEOGRAM_VORONOI_CONVEX_CELL

#ifndef STANDALONE_CONVEX_CELL
#include <geogram/basic/common.h>
#include <geogram/basic/memory.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/geometry.h>
#  ifndef GEOGRAM_PSM
#  include <geogram/basic/attributes.h>
#  endif
#endif

#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <cassert>



/**
 * \file geogram/voronoi/convex_cell.h
 * \brief Class to compute the intersection of a set of half-spaces in 3D.
 * \details Has its own types for points and vectors because it can be used
 *  independently from Geogram. In that case, define STANDALONE_CONVEX_CELL
 */

#ifndef STANDALONE_CONVEX_CELL
namespace GEO {
    class Mesh;
}
#endif


namespace VBW {

#ifdef STANDALONE_CONVEX_CELL
    using std::vector;
    typedef unsigned int index_t;
    typedef unsigned int global_index_t;
#   define vbw_assert(x) assert(x)
    struct vec2 {
	double x;
	double y;
    };
    struct vec3 {
	double x;
	double y;
	double z;
    };
    struct vec4 {
	double x;
	double y;
	double z;
	double w;
    };
#else    
    using GEO::vector;
    typedef unsigned int index_t;        // Always 32 bits
    typedef GEO::index_t global_index_t; // Possibly 64 bits in GARGANTUA mode
#   define vbw_assert(x) geo_debug_assert(x)
    using GEO::vec2;
    using GEO::vec3;
    using GEO::vec4;
#endif    
    
/******************************************************************************/


    /**
     * \brief Creates a vec2 from its components.
     * \param[in] x , y the components of the 
     *  vector.
     * \return the created vector.
     */
    inline vec2 make_vec2(
	double x, double y
    ) {
	vec2 result;
	result.x = x;
	result.y = y;
	return result;
    }
    

    /**
     * \brief Creates a vec3 from its components.
     * \param[in] x , y , z the components of the 
     *  vector.
     * \return the created vector.
     */
    inline vec3 make_vec3(
	double x, double y, double z
    ) {
	vec3 result;
	result.x = x;
	result.y = y;
	result.z = z;
	return result;
    }


    /**
     * \brief Computes the cross product between 
     *  two vectors.
     * \param[in] v1 , v2 the two vectors.
     * \return the cross product between \p v1 and 
     *  \p v2.
     */
    inline vec3 cross(vec3 v1, vec3 v2) {
	return make_vec3(
	    v1.y*v2.z - v1.z*v2.y,
	    v1.z*v2.x - v1.x*v2.z,
	    v1.x*v2.y - v1.y*v2.x
	);
    }

    /**
     * \brief Computes the dot product between 
     *  two vectors.
     * \param[in] v1 , v2 the two vectors.
     * \return the dot product between \p v1 and 
     *  \p v2.
     */
    inline double dot(vec3 v1, vec3 v2) {
	return (
	    v1.x*v2.x + v1.y*v2.y + v1.z*v2.z
	);
    }

    /**
     * \brief Computes the squared length of a vector.
     * \param[in] v the vector.
     * \return the squared length of \p v.
     */
    inline double squared_length(vec3 v) {
	return (v.x*v.x + v.y*v.y + v.z*v.z);
    }

    /**
     * \brief Computes the squared distance between two points.
     * \param[in] v , w the two points.
     * \return the squared distance between \p v and \p w.
     */
    inline double squared_distance(vec3 v, vec3 w) {
	double dx = w.x-v.x;
	double dy = w.y-v.y;
	double dz = w.z-v.z;
	return (dx*dx+dy*dy+dz*dz);
    }

    /**
     * \brief Computes the length of a vector.
     * \param[in] v the vector.
     * \return the length of \p v.
     */
    inline double length(vec3 v) {
	return ::sqrt(squared_length(v));
    }

    /**
     * \brief Computes a normalized vector.
     * \param[in] v the vector.
     * \return a vector with the same direction
     *  as \v and unit length.
     */
    inline vec3 normalize(vec3 v) {
	double s = 1.0/length(v);
	return make_vec3(
	    s*v.x, s*v.y, s*v.z
	);
    }
    
    /**
     * \brief Creates a vec4 from its components.
     * \param[in] x , y , z , w the components of the 
     *  vector.
     * \return the created vector.
     */
    inline vec4 make_vec4(
	double x, double y, double z, double w
    ) {
	vec4 result;
	result.x = x;
	result.y = y;
	result.z = z;
	result.w = w;
	return result;
    }

    /**
     * \brief Computes the dot product between 
     *  two vectors.
     * \param[in] v1 , v2 the two vectors.
     * \return the dot product between \p v1 and 
     *  \p v2.
     */
    inline double dot(vec4 v1, vec4 v2) {
	return (
	    v1.x*v2.x + v1.y*v2.y +
	    v1.z*v2.z + v1.w*v2.w
	);
    }

    /**
     * \brief Computes the squared length of a vector.
     * \param[in] v the vector.
     * \return the squared length of \p v.
     */
    inline double squared_length(vec4 v) {
	return (
	    v.x*v.x + v.y*v.y +
	    v.z*v.z + v.w*v.w
	);
    }

    /**
     * \brief Computes the length of a vector.
     * \param[in] v the vector.
     * \return the length of \p v.
     */
    inline double length(vec4 v) {
	return ::sqrt(squared_length(v));
    }

    /**
     * \brief Computes the squared distance between a point and a plane
     * \param[in] p the point
     * \param[in] P the plane equation
     * \return the squared distance between p and P
     */
    inline double squared_point_plane_distance(VBW::vec3 p, VBW::vec4 P) {
	double result = P.x*p.x + P.y*p.y + P.z*p.z + P.w;
	result = (result*result) / (P.x*P.x + P.y*P.y + P.z*P.z);
	return result;
    }
    
    /**
     * \brief Some constants for the flags
     *  in TriangleWithFlags.
     * \see TriangleWithFlags.
     */
    enum {
	CONFLICT_MASK  = 32768, /**< \brief The mask for conflict triangles. */
	MARKED_MASK    = 16384, /**< \brief The mask for marked triangles.   */	
	END_OF_LIST    = 16383, /**< \brief Constant to indicate end of list.*/
	VERTEX_AT_INFINITY = 0  /**< \brief Vertex at infinity.              */
    };


    /**
     * \brief Type for flags.
     */
    typedef unsigned char uchar;
    
    /**
     * \brief Type for local indices.
     * \details Valid values are between 0 and 32766. 
     *  Full range is not used due to bookkeeping reasons,
     *  \see TriangleWithFlags.
     */
    typedef unsigned short ushort;

    /**
     * \brief A triangle with the local indices of its three
     *  vertices.
     */
    struct Triangle {
	ushort i;
	ushort j;
	ushort k;
    };

    /**
     * \brief Creates a triangle from its three vertices.
     * \param[in] i , j , k the local indices of the three
     *  vertices.
     * \return The created triangle.
     */
    inline Triangle make_triangle(
	ushort i, ushort j, ushort k
    ) {
	Triangle result;
	result.i = i;
	result.j = j;
	result.k = k;
	return result;
    }

    /**
     * \brief A triangle with flags.
     * \details The flags are used for two purposes:
     *  - bits [0..15] are used to chain the triangles: 
     *   there are two lists of triangles, the valid triangles 
     *   and the free list. End of list is indicated by value 32767.
     *  - bit 16 (32768) is set if the triangle is in conflict.
     */
    struct TriangleWithFlags : public Triangle {
	ushort flags;
    };

    inline TriangleWithFlags make_triangle_with_flags(
	ushort i, ushort j, ushort k, ushort f
    ) {
	TriangleWithFlags result;
	result.i = i;
	result.j = j;
	result.k = k;
	result.flags = f;
	return result;
    }


/******************************************************************************/

    inline double det2x2(
	double a11, double a12,
	double a21, double a22
    ) {
	return a11*a22 - a12*a21;
    }
    
    inline double det3x3(
	double a11, double a12, double a13,
	double a21, double a22, double a23,
	double a31, double a32, double a33
    ) {
	return
	    a11*det2x2(a22,a23,a32,a33)
	    -a21*det2x2(a12,a13,a32,a33)
	    +a31*det2x2(a12,a13,a22,a23);
    }

    inline double det4x4(
	double a11, double a12, double a13, double a14,
	double a21, double a22, double a23, double a24,               
	double a31, double a32, double a33, double a34,  
	double a41, double a42, double a43, double a44  
    ) {
	double m12 = a21*a12 - a11*a22;
	double m13 = a31*a12 - a11*a32;
	double m14 = a41*a12 - a11*a42;
	double m23 = a31*a22 - a21*a32;
	double m24 = a41*a22 - a21*a42;
	double m34 = a41*a32 - a31*a42;
	
	double m123 = m23*a13 - m13*a23 + m12*a33;
	double m124 = m24*a13 - m14*a23 + m12*a43;
	double m134 = m34*a13 - m14*a33 + m13*a43;
	double m234 = m34*a23 - m24*a33 + m23*a43;
	
	return (m234*a14 - m134*a24 + m124*a34 - m123*a44);
    }   

/******************************************************************************/

    enum ConvexCellFlag {
	None        = 0, /**< \brief default */
	WithVGlobal = 1, /**< \brief store global vertex indices */ 
	WithTFlags  = 2  /**< \brief store user triange flags */
    };

    typedef index_t ConvexCellFlags;
    
    /**
     * \brief Computes the intersection between a set of halfplanes using
     *  Bowyer-Watson algorithm.
     * \details Do not use with a large number of planes.
     */
    class GEOGRAM_API ConvexCell {
      public:

	/**
	 * \brief ConvexCell constructor.
	 * \param[in] flags a combination of WithVGlobal, WithTFlags
	 */
	ConvexCell(ConvexCellFlags flags = None);

#ifndef STANDALONE_CONVEX_CELL
	/**
	 * \brief Specifies whether exact predicates should be used.
	 * \param[in] x true if exact predicates should be used.
	 * \details Not supported if ConvexCell distributed 
	 *  as standalone file.
	 */
	void use_exact_predicates(bool x) {
	    use_exact_predicates_ = x;
	}
#endif
	
	/**
	 * \brief Tests whether global vertex indices are stored.
	 * \retval true if global vertex indices are stored.
	 * \retval false otherwise.
	 */
	bool has_vglobal() const {
	    return has_vglobal_;
	}

	/**
	 * \brief Tests whether triangle flags are stored.
	 * \retval true if triangle flags are stored.
	 * \retval false otherwise.
	 */
	bool has_tflags() const {
	    return has_tflags_;
	}

	/**
	 * \brief Creates vertex global indices if they are 
	 *  not present.
	 */
	void create_vglobal() {
	    if(!has_vglobal()) {
		has_vglobal_ = true;
		vglobal_.assign(max_v(), global_index_t(-1));
	    }
	}
	
	/**
	 * \brief Removes all vertices and triangles from this
	 *  ConvexCell.
	 * \details Keeps allocated memory for future use.
	 */
	void clear();
	
	/**
	 * \brief Initializes this ConvexCell to an axis-aligned
	 *  box.
	 * \details Previous contents of this ConvexCell are 
	 *  discarded. Vertex 0 is vertex at infinity.
	 * \param[in] xmin , ymin , zmin , xmax , ymax , zmax
	 *  the coordinates of the box.
	 */
	void init_with_box(
	    double xmin, double ymin, double zmin,
	    double xmax, double ymax, double zmax
	);

        /**
         * \brief Initializes this ConvexCell to a tetrahedron.
	 * \details Previous contents of this ConvexCell are 
	 *  discarded. Vertex 0 is vertex at infinity.
	 * \param[in] P0 , P1 , P2 , P3 the plane equations of 
	 *  the four faces of the tetrahedron.
	 */
        void init_with_tet(
	    vec4 P0, vec4 P1, vec4 P2, vec4 P3
	);

        /**
         * \brief Initializes this ConvexCell to a tetrahedron.
	 * \details Previous contents of this ConvexCell are 
	 *  discarded. Vertex 0 is vertex at infinity.
	 * \param[in] P0 , P1 , P2 , P3 the plane equations of 
	 *  the four faces of the tetrahedron.
	 * \param[in] P0_global_index , P1_global_index ,
	 *            P1_global_index , P2_global_index the global
	 *  indices associated with the plane equations. 
	 * \pre has_vglobal()
	 */
        void init_with_tet(
	    vec4 P0, vec4 P1, vec4 P2, vec4 P3,
	    global_index_t P0_global_index,
	    global_index_t P1_global_index,
	    global_index_t P2_global_index,
	    global_index_t P3_global_index	    
	);
      
	/**
	 * \brief Saves the computed cell in alias wavefront
	 *  file format.
	 * \param[in] filename the name of the file where to
	 *  save the cell.
	 * \param[in] shrink shrinking factor to ease visualization.
	 */
	void save(const std::string& filename, double shrink=0.0) const;


	/**
	 * \brief Saves the computed cell in alias wavefront 
	 *  file format.
	 * \param[out] out a stream where to save the output.
	 * \param[in] v_offset offset applied to vertex indices.
	 * \param[in] shrink shrinking factor to ease visualization.
	 * \param[in] borders_only if set, only facets that correspond
	 *  to vertex global index -1 are saved.
	 * \return the number of created vertices. 
	 */
	index_t save(
	    std::ostream& out, global_index_t v_offset=1, double shrink=0.0,
	    bool borders_only=false
	) const;

#if !defined(STANDALONE_CONVEX_CELL) && !defined(GEOGRAM_PSM)
	/**
	 * \brief Appends the computed cell to a GEO::Mesh.
	 * \param[out] mesh a pointer to the mesh.
	 * \param[in] shrink shrinking factor to ease visualization.
	 * \param[in] borders_only if set, only facets that correspond
	 *  to vertex global index -1 are saved.
	 * \param[in] facet_attr optional facet attribute that stores
	 *  global facet (dual vertex) ids.
	 * \note One needs to call mesh->facets.connect() afterwards to 
	 *  have facets adjacencies. It is not called because one may 
	 *  want to append multiple cells to the same mesh.
	 */
        void append_to_mesh(
	    GEO::Mesh* mesh,
	    double shrink=0.0, bool borders_only=false,
	    GEO::Attribute<GEO::index_t>* facet_attr=nullptr
	) const;

#endif      

        /**
         * \brief Calls a user-defined function for each vertex of a Voronoi
	 *  facet.
	 * \details One needs to call compute_geometry() before calling this
	 *  function.
	 * \param[in] v the index of the (dual) Voronoi Facet, that is a
	 *  (primal) vertex, in [0..nb_v()-1]
	 * \param[in] vertex a function that takes an index_t as an argument,
	 *  with the index of the triangle that corresponds to the current
	 *  Voronoi vertex.
	 */
        void for_each_Voronoi_vertex(
	    index_t v,
	    std::function<void(index_t)> vertex
        );
      
	/**
	 * \brief Clips this convex cell by a new plane.
	 * \details The positive side of the plane equation corresponds to
	 *  what is kept. In other words, the normal vector P.x, P.y, P.z 
	 *  points towards the interior of this ConvexCell.
	 * \param[in] P the plane equation.
	 */
	void clip_by_plane(vec4 P);

	/**
	 * \brief Clips this convex cell by a new plane and stores
	 *  the corresponding global index in the newly created vertex.
	 * \details The positive side of the plane equation corresponds to
	 *  what is kept. In other words, the normal vector P.x, P.y, P.z 
	 *  points towards the interior of this ConvexCell.
	 *  This function can only be called if global indices are stored.
	 * \param[in] P the plane equation.
	 * \param[in] j the global index of the plane.
	 */
	void clip_by_plane(vec4 P, global_index_t j);


	/**
	 * \brief Clips this convex cell by a new plane, using a user-defined
	 *  geometric predicate.
	 * \details It is useful to be able to have a user-defined geometric
	 *  predicates when the vertices have a symbolic representation, stored
	 *  in the global indices associated with the plane. It is used by
	 *  the robust mesh boolean operations.
	 *  The positive side of the plane equation corresponds to
	 *  what is kept. In other words, the normal vector P.x, P.y, P.z 
	 *  points towards the interior of this ConvexCell.
	 *  If global indices are stored, then j is stored as the global index
	 *  of the plane equation.
	 * \param[in] P the plane equation.
	 * \param[in] P_global_index the global index of the plane.
	 * \param[in] triangle_conflict_predicate a function that takes as
	 *  arguments a local triangle index and local vertex (plane eqn)
	 *  index, and that returns true if the triangle is in conflict with
	 *  the vertex.
	 */
        void clip_by_plane(
	    vec4 P, global_index_t P_global_index,
	    std::function<bool(ushort,ushort)> triangle_conflict_predicate
	);
      
	/**
	 * \brief Clips this convex cell by a new plane and stores
	 *  the corresponding global index in the newly created vertex.
	 * \details For a ConvexCell with a large number of facets, this
	 *  version is faster than clip_by_plane(). However, it cannot be
	 *  used with a ConvexCell that has infinite faces.
	 * \param[in] P the plane equation.
	 * \see clip_by_plane()
	 */
        void clip_by_plane_fast(vec4 P);

	/**
	 * \brief Clips this convex cell by a new plane and stores
	 *  the corresponding global index in the newly created vertex.
	 * \details For a ConvexCell with a large number of facets, this
	 *  version is faster than clip_by_plane(). However, it cannot be
	 *  used with a ConvexCell that has infinite faces.
	 * \param[in] P the plane equation.
	 * \param[in] j the global index of the plane.
	 * \see clip_by_plane()
	 */
        void clip_by_plane_fast(vec4 P, global_index_t j);      
      
	/**
	 * \brief Gets the number of triangles.
	 * \return the number of created triangles.
	 * \details The created triangles are not
	 *  necessarily valid ones. To get the valid triangles,
	 *  one needs to traverse the list from first_valid_.
	 */
	index_t nb_t() const {
	    return nb_t_;
	}

	/**
	 * \brief Gets the number of vertices.
	 * \return the number of vertices.
	 * \details Some vertices can be incident to no triangle.
	 *  The first six vertices correspond to the facets of the
	 *  initial axis aligned box passed to the constructor.
	 */
	index_t nb_v() const {
	    return nb_v_;
	}

	/**
	 * \brief Directly creates a new vertex.
	 * \param[in] P the plane equation attached to the vertex.
	 * \return the index of the newly created vertex.
	 */
	index_t create_vertex(vec4 P) {
	    if(nb_v_ == max_v_) {
		grow_v();
	    }
	    plane_eqn_[nb_v_] = P;
	    index_t result = nb_v_;
	    ++nb_v_;
	    return result;
	}

	/**
	 * \brief Directly creates a new vertex.
	 * \param[in] P the plane equation attached to the vertex.
	 * \param[in] v the global index associated with the vertex.
	 * \return the index of the newly created vertex.
	 * \pre global vertex indices are stored
	 */
	index_t create_vertex(vec4 P, global_index_t v) {
	    index_t result = create_vertex(P);
	    vglobal_[nb_v()-1] = v;
	    return result;
	}
	
	/**
	 * \brief Directly creates a new triangle.
	 * \param[in] i , j, k the three vertices of the
	 *  triangle.
	 * \details The triangle is inserted into the list
	 *  of valid triangles.
	 * \return the index of the newly created triangle.
	 */
	index_t create_triangle(index_t i, index_t j, index_t k) {
	    vbw_assert(i < nb_v());
	    vbw_assert(j < nb_v());
	    vbw_assert(k < nb_v());
	    return new_triangle(i,j,k);
	}

	/**
	 * \brief Replaces a vertex with the vertex at infinity
	 *  in all facets.
	 * \param[in] v the vertex to be killed.
	 */
	void kill_vertex(index_t v);

	/**
	 * \brief Tests whether a vertex has a corresponding
	 *  facet in the cell.
	 * \details Calling compute_geometry() before makes
	 *  this function faster.
	 */
	bool vertex_is_contributing(index_t v) const {
	    if(!geometry_dirty_) {
		return v2t_[v] != END_OF_LIST;
	    }
	    index_t t = first_valid_;
	    while(t != END_OF_LIST) { 
		TriangleWithFlags T = get_triangle_and_flags(t);
		if(T.i == v || T.j == v || T.k == v) {
		    return true;
		}
		t = index_t(T.flags);
	    }
	    return false;
	}

       /**
	* \brief Gets a triangle incident to a vertex.
	* \param[in] v vertex index.
	* \return a triangle incident to v.
	*/
	index_t vertex_triangle(index_t v) const {
	    geo_assert(!geometry_dirty_);	    
	    return v2t_[v];
	}
	
	/**
	 * \brief Computes the geometry and some cached information.
	 * \details Needs to be called before volume(),
	 *   facet_area() and barycenter().
	 */
	void compute_geometry();

	/**
	 * \brief Gets the dual facet area of a given vertex.
	 * \details compute_geometry() needs to be called before.
	 * \param[in] v the vertex.
	 * \return the dual facet area associated with v.
	 * \details terminate() needs to be called before 
	 *  calling this function.
	 */
	double facet_area(index_t v) const;

	/**
	 * \brief Computes the volume of this convex cell.
	 * \details compute_geometry() needs to be called before.
	 * \return the volume.
	 */
	double volume() const;

	/**
	 * \brief Computes the barycenter of this convex cell.
	 * \details compute_geometry() needs to be called before.
	 * \return the barycenter.
	 */
	vec3 barycenter() const;

	/**
	 * \brief Computes volume and barycenter.
	 * \param[out] m the computed volume
	 * \param[out] mg the computed volume times the barycenter
	 * \details compute_geometry() needs to be called before.
	 */
        void compute_mg(double& m, vec3& mg) const ;

      
	/**
	 * \brief Computes the squared radius of the smallest sphere
	 *  containing the cell and centered on a point.
	 * \return the maximum squared distance between center and
	 *  all the vertices of the cell.
	 */
	double squared_radius(vec3 center) const;

	/**
	 * \brief Computes the squared radius of the largest sphere contained
	 *  in the cell and centered on a point.
	 * \return the minimum squared distance between center and
	 *  all facets of the cell.
	 */
	double squared_inner_radius(vec3 center) const;

	
	/**
	 * \brief Tests whether this ConvexCell is empty.
	 * \details ConvexCell can be empty if everything was
	 *  clipped out.
	 * \retval true if this ConvexCell is empty.
	 * \retval false otherwise.
	 */
	bool empty() const {
	    return first_valid_ == END_OF_LIST;
	}

	/**
	 * \brief Gets the global vertex index from a local 
	 *  vertex index.
	 * \details Vertex indices correspond to planes (remember,
	 *  we are in dual form).
	 * \param[in] lv the local vertex index
	 * \return the global vertex index that corresponds to
	 *  lv.
	 */
	global_index_t v_global_index(index_t lv) const {
	    vbw_assert(has_vglobal_);
	    vbw_assert(lv < nb_v());
	    return vglobal_[lv];
	}

	/**
	 * \brief Sets the global vertex index associated with a local 
	 *  vertex index.
	 * \details Vertex indices correspond to planes (remember,
	 *  we are in dual form).
	 * \param[in] lv the local vertex index
	 * \param[in] v the global vertex index that corresponds to
	 *  lv.
	 */
	void set_v_global_index(index_t lv, global_index_t v) {
	    vbw_assert(has_vglobal_);
	    vbw_assert(lv < nb_v());
	    vglobal_[lv] = v;
	}
	
	/**
	 * \brief Tests whether a vertex with a given global index
	 *  exists in this ConvexCell.
	 * \param[in] v the global index.
	 * \retval true if there exists in this ConvexCell a vertex with
	 *  global index \p v.
	 * \retval false otherwise.
	 */
	bool has_v_global_index(global_index_t v) const;

	/**
	 * \brief Gets the first triangle.
	 * \return the index of the first triangle, or END_OF_LIST
	 *  if this ConvexCell is empty.
	 */
	ushort first_triangle() const {
	    return ushort(first_valid_);
	}

	/**
	 * \brief Gets the successor of a triangle.
	 * \param[in] t the index of a valid triangle.
	 * \return the index of the successor of \p t, or END_OF_LIST
	 *  if \p t is the last triangle.
	 */
	ushort next_triangle(ushort t) const {
	    return get_triangle_flags(t);
	}

	/**
	 * \brief Gets the point that corresponds to a triangle.
	 * \details If compute_geometry() was called, this gets
	 *  the previously computed point, else it is computed
	 *  and returned.
	 * \param[in] t the index of the triangle.
	 * \return the point that corresponds to triangle \p t.
	 */
	vec3 triangle_point(ushort t) const {
	    if(geometry_dirty_) {
		TriangleWithFlags T = get_triangle_and_flags(t);
		vec4 result = compute_triangle_point(T);
		vbw_assert(result.w != 0.0);
		return make_vec3(
		    result.x/result.w, result.y/result.w, result.z/result.w
		);
	    }
	    return triangle_point_[t];
	}

	/**
	 * \brief Gets the global index of a triangle vertex.
	 * \param[in] t the triangle.
	 * \param[in] llv one of 0,1,2.
	 * \return the global index of the vertex.
	 * \pre global indices are stored.
	 */
	global_index_t triangle_v_global_index(ushort t, index_t llv) const {
	    Triangle T = get_triangle(t);
	    ushort lv = ushort((llv==0)*T.i + (llv==1)*T.j + (llv==2)*T.k);
	    return v_global_index(lv);
	}

	/**
	 * \brief Gets the local index of a triangle vertex.
	 * \param[in] t the triangle.
	 * \param[in] llv one of 0,1,2.
	 * \return the local index of the vertex, in 0..nb_v()-1
	 */
	index_t triangle_v_local_index(ushort t, index_t llv) const {
	    Triangle T = get_triangle(t);
	    return index_t((llv==0)*T.i + (llv==1)*T.j + (llv==2)*T.k);
	}

	/**
	 * \brief Tests whether a triangle is marked by the user.
	 * \param[in] t the triangle.
	 * \retval true if the triangle is marked.
	 * \retval false otherwise.
	 * \pre triangle flags are stored.
	 */
	bool triangle_is_user_marked(ushort t) {
	    vbw_assert(has_tflags_);
	    vbw_assert(t < max_t_);
	    return (tflags_[t] != 0);
	}

	/**
	 * \brief Sets the user mark on a triangle.
	 * \param[in] t the triangle.
	 * \pre triangle flags are stored.
	 */
	void triangle_user_mark(ushort t) {
	    vbw_assert(has_tflags_);
	    vbw_assert(t < max_t_);
	    tflags_[t] = 1;
	}

	/**
	 * \brief Resets the user mark on a triangle.
	 * \param[in] t the triangle.
	 * \pre triangle flags are stored.
	 */
	void triangle_user_unmark(ushort t) {
	    vbw_assert(has_tflags_);
	    vbw_assert(t < max_t_);
	    tflags_[t] = 0;
	}

	/**
	 * \brief Tests whether a cell has at least one vertex in conflict with
	 *  a halfspace.
	 * \param[in] P the equation of the halfspace.
	 * \retval true if there exists a triangle t such that 
	 *  triangle_is_in_conflict(P)
	 * \retval false otherwise.
	 */
	bool cell_has_conflict(const vec4& P) {
	    for(
		ushort t = first_triangle();
		t!=END_OF_LIST; t=next_triangle(t)
	    ) {
		TriangleWithFlags T = get_triangle_and_flags(t);
		if(triangle_is_in_conflict(T,P)) {
		    return true;
		}
	    }
	    return false;
	}

	/**
	 * \brief Tests whether a cell has all its vertices in conflict
	 *  with a plane.
	 * \param[in] P the equation of the halfspace.
	 * \retval true if all the triangles are in conflict with P.
	 * \retval false otherwise.
	 */
	bool cell_is_totally_in_conflict(const vec4& P) {
	    for(
		ushort t = first_triangle();
		t!=END_OF_LIST; t=next_triangle(t)
	    ) {
		TriangleWithFlags T = get_triangle_and_flags(t);
		if(!triangle_is_in_conflict(T,P)) {
		    return false;
		}
	    }
	    return true;
	}
	
	/**
	 * \brief Gets a triangle adjacent to another triangle by edge
	 *  local index.
	 * \param[in] t a triangle.
	 * \param[in] le local index of an edge of \p t (in 0..2).
	 * \return the triangle adjacent to \p t along \ p e.
	 */
	 index_t triangle_adjacent(index_t t, index_t le) const {
	     vbw_assert(t < max_t());
	     vbw_assert(le < 3);
	     Triangle T = get_triangle(t);
	     index_t v1 =
		 index_t((le == 0)*T.j + (le == 1)*T.k + (le == 2)*T.i);
	     index_t v2 =
		 index_t((le == 0)*T.k + (le == 1)*T.i + (le == 2)*T.j);
	     vbw_assert(vv2t(v1,v2) == t);
	     vbw_assert(vv2t(v2,v1) != END_OF_LIST);
	     return vv2t(v2,v1);
	 }

	/**
	 * \brief Gets a triangle vertex.
	 * \param[in] t a triangle.
	 * \param[in] lv local index of a vertex of \p t (in 0..2).
	 * \return the vertex
	 */
         index_t triangle_vertex(index_t t, index_t lv) const {
	     vbw_assert(t < max_t());
	     vbw_assert(lv < 3);
	     Triangle T = get_triangle(t);
	     return index_t((lv==0)*T.i+(lv==1)*T.j+(lv==2)*T.k);
	 }

      
	/**
	 * \brief Gets the local index of a vertex in a triangle.
	 * \param[in] t a triangle.
	 * \param[in] v a vertex index.
	 * \return the local index of \p v in \p t (in 0..2).
	 */
	index_t triangle_find_vertex(index_t t, index_t v) const {
	    vbw_assert(t < max_t());
	    Triangle T = get_triangle(t);
	    index_t result = index_t((T.j == v) + 2*(T.k == v));
	    return result;
	}

	/**
	 * \brief Tests whether a triangle is infinite.
	 * \param[in] t the triangle
	 * \retval true if t is incident to the vertex at
	 *  infinity.
	 * \retval false otherwise.
	 */
	bool triangle_is_infinite(index_t t) const {
	    vbw_assert(t < max_t());
	    Triangle T = get_triangle(t);
	    return (
		T.i == VERTEX_AT_INFINITY ||
		T.j == VERTEX_AT_INFINITY ||
		T.k == VERTEX_AT_INFINITY
	    );
	}
	
	/**
	 * \brief Gets the equation of a plane associated with a vertex.
	 * \details The first six equations correspond to the six 
	 *  facets of a cube.
	 * \param[in] v the local index of the vertex.
	 */
        vec4 vertex_plane(index_t v) const {
	    vbw_assert(v < max_v());
	    return plane_eqn_[v];
	}

	/**
	 * \brief Gets the normal to the plane associated with a vertex.
	 * \details The first six equations correspond to the six 
	 *  facets of a cube.
	 * \param[in] v the local index of the vertex.
	 */
	vec3 vertex_plane_normal(index_t v) const {
	    vbw_assert(v != VERTEX_AT_INFINITY);
	    vbw_assert(v < max_v());
	    return make_vec3(
		plane_eqn_[v].x,
		plane_eqn_[v].y,
		plane_eqn_[v].z
	    );
	}
	
	/**
	 * \brief Tests whether a triangle is marked as conflict.
	 * \param[in] t a triangle.
	 * \retval true if \p t is marked as conflict.
	 * \retval false otherwise.
	 */
	bool triangle_is_marked_as_conflict(index_t t) const {
	    vbw_assert(t < max_t());
	    return (get_triangle_flags(t) & ushort(CONFLICT_MASK)) != 0;
	}
       
	/**
	 * \brief Tests whether a triangle is in conflict with a plane.
	 * \details A triangle is in conflict with a plane if feeding the point
	 *  associated with the triangle in the equation of the plane yields 
	 *  a negative number.
	 * \param[in] T a triangle.
	 * \param[in] eqn the four coefficients of the equation of the plane.
	 * \retval true if \p t is in conflict with \p eqn.
	 * \retval false otherwise.
	 */
        bool triangle_is_in_conflict(
	    TriangleWithFlags T, const vec4& eqn
	) const;

	/**
	 * \brief Creates a new triangle.
	 * \param[in] i , j , k the three vertices of the triangle.
	 */
	index_t new_triangle(index_t i, index_t j, index_t k) {
	    index_t result = first_free_;
	    if(result == END_OF_LIST) {
		result = nb_t_;
		++nb_t_;
		if(nb_t_ > max_t()) {
		    grow_t();
		}
	    } else {
		first_free_ = index_t(
		    get_triangle_flags(first_free_) & ~ushort(CONFLICT_MASK)
		);
	    }
	    vbw_assert(result < max_t());
	    t_[result] = make_triangle_with_flags(
		ushort(i), ushort(j), ushort(k), ushort(first_valid_)
	    );
	    set_vv2t(i, j, result);
	    set_vv2t(j, k, result);
	    set_vv2t(k, i, result);	    
	    first_valid_ = result;
	    if(has_tflags_) {
		tflags_[result] = 0;
	    }
	    return result;
	}
	
	/**
	 * \brief Creates a new triangle.
	 * \details Adjacency information is not used (kept for reference).
	 * \param[in] i , j , k the three vertices of the triangle.
	 * \param[in] adj0 , adj1 , adj2 the three adjacent triangles
	 *  (unused in this version).
	 * \return the index of the new triangle.
	 */
	index_t new_triangle(
	    index_t i, index_t j, index_t k,
	    index_t adj0, index_t adj1, index_t adj2
	) {
	    // Silence warnings
	    (void)(adj0);
	    (void)(adj1);
	    (void)(adj2);	    
	    return new_triangle(i, j, k);
	}

	/**
	 * \brief Computes the coordinates of the point
	 *  associated with a triangle.
	 * \param[in] T the triangle.
	 * \return the intersection between the three planes
	 *  associated with the three vertices of the triangle,
	 *  in homogeneous coordinates.
	 */
	vec4 compute_triangle_point(TriangleWithFlags T) const;

	/**
	 * \brief Gets the three vertices of a triangle.
	 * \param[in] t the triangle.
	 * \return a Triangle with the indices of the three vertices
	 *  of the triangle.
	 */
	Triangle get_triangle(index_t t) const {
	    vbw_assert(t < max_t());
	    return t_[t];
	}
	
	/**
	 * \brief Gets the flags associated with a triangle.
	 * \details Contains both the conflict flag and the
	 *  chaining.
	 * \param[in] t the triangle.
	 * \return the flags associated with \p t.
	 */
	ushort get_triangle_flags(index_t t) const {
	    vbw_assert(t < max_t());
	    return t_[t].flags;
	}

	/**
	 * \brief Sets the flags of a triangle.
	 * \param[in] t the triangle.
	 * \param[in] flags the flags to be set.
	 */
	void set_triangle_flags(index_t t, ushort flags) {
	    vbw_assert(t < max_t());
	    t_[t].flags = flags;
	}

	/**
	 * \brief Gets the three vertices of a triangle and its flags.
	 * \param[in] t the triangle.
	 * \return a TriangleWithFlags with the indices of the three vertices
	 *  of the triangle and the flags.
	 */
	TriangleWithFlags get_triangle_and_flags(index_t t) const {
	    vbw_assert(t < max_t());
	    return t_[t];
	}
	
	/**
	 * \brief Gets the triangle incident to an oriented edge.
	 * \param[in] v1 , v2 the two vertices of the oriented edge.
	 * \return the triangle incident to the oriented edge.
	 */
	index_t vv2t(index_t v1, index_t v2) const {
	    vbw_assert(v1 < max_v());
	    vbw_assert(v2 < max_v());
	    return index_t(vv2t_[max_v_*v1 + v2]);
	}

	/**
	 * \brief Sets the triangle incident to an oriented edge.
	 * \param[in] v1 , v2 the two vertices of the oriented edge.
	 * \param[in] t the triangle incident to the oriented edge.
	 */
	void set_vv2t(index_t v1, index_t v2, index_t t) {
	    vbw_assert(v1 < max_v());
	    vbw_assert(v2 < max_v());
	    vv2t_[max_v_*v1+v2] = ushort(t);
	}

	/**
	 * \brief Gets the maximum valid index for a triangle.
	 * \return the maximum valid index of a triangle.
	 */
	index_t max_t() const {
	    return max_t_;
	}

	/**
	 * \brief Gets the maximum valid index for a vertex.
	 * \return the maximum valid index of a vertex.
	 */
	index_t max_v() const {
	    return max_v_;
	}

	/**
	 * \brief Allocates more space for triangles.
	 * \details Makes max_t_ twice bigger.
	 */
	void grow_t();

	/**
	 * \brief Allocates more space for vertices.
	 * \details Makes max_v_ twice bigger.
	 */
	void grow_v();


        /**
	 * \brief Swaps two ConvexCells.
	 * \param[in] other the ConvexCell to be 
	 *  exchanged with this ConvexCell.
	 */
        void swap(ConvexCell& other) {
	    std::swap(max_t_,other.max_t_);
	    std::swap(max_v_,other.max_v_);
	    std::swap(t_,other.t_);
	    std::swap(vv2t_,other.vv2t_);
	    std::swap(plane_eqn_,other.plane_eqn_);
	    std::swap(nb_t_,other.nb_t_);
	    std::swap(nb_v_,other.nb_v_);
	    std::swap(first_free_,other.first_free_);
	    std::swap(first_valid_,other.first_valid_);
	    std::swap(geometry_dirty_,other.geometry_dirty_);
	    std::swap(triangle_point_,other.triangle_point_);
	    std::swap(v2t_,other.v2t_);
	    std::swap(vglobal_,other.vglobal_);
	    std::swap(has_vglobal_,other.has_vglobal_);
	    std::swap(tflags_,other.tflags_);
	    std::swap(has_tflags_,other.has_tflags_);
#ifndef STANDALONE_CONVEX_CELL	
	    std::swap(use_exact_predicates_,other.use_exact_predicates_);
#endif	
        }

        /**
         * \brief Gets a modifiable reference to a triangle point.
	 * \param[in] t the index
	 * \return a modifiable reference to the stored point
	 */
        vec3& stored_triangle_point(ushort t) {
	    return triangle_point_[t];	    
        }
      
      protected:

        /**
	 * \brief Triangulates the conflict zone.
	 * \param[in] lv the local index of the new vertex
	 * \param[in] conflict_head , conflict tail the first
	 *  and last triangle of the conflict zone stored 
	 *  as a linked list.
	 */
        void triangulate_conflict_zone(
	   index_t lv, index_t conflict_head, index_t conflict_tail
	);
      
	/**
	 * \brief Changes a vertex plane equation.
	 * \param[in] v the vertex.
	 * \param[in] P the plane equation.
	 * \details Does not update combinatorics.
	 * \note Use with care, for experts only.
	 */
        void set_vertex_plane(index_t v, vec4 P) {
	    vbw_assert(v < max_v());
	    plane_eqn_[v] = P;
	    geometry_dirty_ = true;
	}

      
      private:

	/** \brief number of allocated triangles */
	index_t max_t_;

	/** \brief number of allocated vertices */
	index_t max_v_;

	/** \brief indices of triangle vertices and flags */
	vector<TriangleWithFlags> t_;

	/**
	 * \brief vertex,vertex -> triangle adjacent to 
	 *   oriented edge.
	 */
	vector<ushort> vv2t_;

	/**
	 * \brief plane equation attached to each vertex,
	 *  as specified by clip_by_plane().
	 */
	vector<vec4> plane_eqn_;

	/** \brief number of used triangles. */
	index_t nb_t_;

	/** \brief number of used vertices. */	
	index_t nb_v_;

	/** \brief Head of the linked list of free triangles. */
	index_t first_free_;

	/** \brief Head of the linked list of valid triangles. */
	index_t first_valid_;

	/** 
	 * \brief true if triangle_point_ and t2v_ are 
	 *  not up to date. 
	 */
	bool geometry_dirty_;

	/**
	 * \brief dual vertex attached to each triangle.
	 */
	vector<vec3> triangle_point_;

	/** 
	 * \brief One triangle incident to each vertex, 
	 *  or END_OF_LIST if there is no such triangle.
	 */
	vector<ushort> v2t_;

	/**
	 * \brief Optional vector of gloval vertex indices.
	 */
	vector<global_index_t> vglobal_;
	
	/**
	 * \brief True if global vertex indices are stored.
	 */
	bool has_vglobal_;

	/**
	 * \brief Optional flags attached to the triangles.
	 */
	vector<uchar> tflags_;
	
	/**
	 * \brief True if triangle flags are stored.
	 */
	bool has_tflags_;

#ifndef STANDALONE_CONVEX_CELL	
	/**
	 * \brief True if exact predicates should be used.
	 */
	bool use_exact_predicates_;
#endif
    };
}

namespace GEO {
    using VBW::ConvexCell;
}

#endif
