/*
 *  Copyright (c) 2010-2017, ALICE project, Inria
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

#ifndef GEOGRAM_VORONOI_RVD_CALLBACK
#define GEOGRAM_VORONOI_RVD_CALLBACK

#include <geogram/basic/common.h>
#include <geogram/voronoi/generic_RVD_vertex.h>
#include <geogram/mesh/mesh.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/attributes.h>

namespace GEOGen {
    class SymbolicVertex;
    class Polygon;
    class ConvexCell;
}

namespace GEO {
    class RVDVertexMap;
    class Mesh;

    namespace Process {
        class SpinLockArray;
    }
}

/**
 * \file geogram/voronoi/RVD_callback.h
 * \brief Declaration of base types for implementing user-defined
 *  code that queries the cells of a restricted Voronoi diagram.
 */

namespace GEO {

    /***************************************************************/
    
    /**
     * \brief Baseclass for user functions called for each
     *  element (polygon or polyhedron) of a restricted Voronoi
     *  diagram traversal.
     */
    class GEOGRAM_API RVDCallback {
      public:

	/**
	 * \brief RVDCallback constructor.
	 */
	RVDCallback();

	/**
	 * \brief RVDCallback destructor.
	 */
	virtual ~RVDCallback();
	
	/**
	 * \brief Called at the beginning of the RVD traversal.
	 */
	virtual void begin();

	/**
	 * \brief Called at the end of the RVD traversal.
	 */
	virtual void end();

	/**
	 * \brief Gets the index of the seed that corresponds to the current
	 *  polygon/polyhedron.
	 * \return The index of the seed that corresponds to the 
	 *  current polygon/polyhedron.
	 * \details The current polygon/polyhedron is the intersection 
	 *  between a Voronoi cell (associted with a seed) and a simplex. 
	 */
	index_t seed() const {
	    return seed_;
	}

	/**
	 * \brief Gets the index of the simplex that corresponds to the 
	 *  current polygon/polyhedron.
	 * \return The index of the simplex that corresponds to the 
	 *  current polygon/polyhedron. Points to a simplex in the mesh that the
	 *  Voronoi diagram is restricted to.
	 * \details The current polygon/polyhedron is the intersection between 
	 *  a Voronoi cell and a simplex. 
	 */
	index_t simplex() const {
	    return simplex_;
	}

	/**
	 * \brief Sets the spinlocks array.
	 * \details In multithreading mode, a spinlocks array can
	 *  be used to manage concurrent accesses.
	 * \param[in] spinlocks a pointer to the Process::SpinLockArray
	 *  or nullptr if no spinlocks are used.
	 */
	void set_spinlocks(Process::SpinLockArray* spinlocks) {
	    spinlocks_ = spinlocks;
	}
	
      protected:
	index_t seed_;
	index_t simplex_;
        Process::SpinLockArray* spinlocks_;
    };

    /***************************************************************/

    /**
     * \brief Baseclass for user functions called for each
     *  polygon of a surfacic restricted Voronoi diagram.
     * \details A surfacic restricted Voronoi diagram is the
     *  intersection between Voronoi cells and the triangles of
     *  a given triangulated mesh. The member functions of this
     *  class are called for each intersection between a Voronoi
     *  cell and a triangle.
     */

    class GEOGRAM_API RVDPolygonCallback : public RVDCallback {
      public:

	/**
	 * \brief PolyhedronCallback constructor.
	 */
	RVDPolygonCallback();
	
	/**
	 * \brief PolyhedronCallback destructor.
	 */
	virtual ~RVDPolygonCallback();

	/**
	 * \copydoc RVDCallback::begin()
	 */
	virtual void begin();

	/**
	 * \copydoc RVDCallback::end()
	 */
	virtual void end();

	/**
	 * \brief The default callback called for each polygon
	 * \param[in] v index of current Delaunay seed
	 * \param[in] t index of current mesh triangle
	 * \param[in] C intersection between current mesh triangle
	 *  and the Voronoi cell of \p v
	 */
	virtual void operator() (
	    index_t v,
	    index_t t,
	    const GEOGen::Polygon& C
	) const;

    };
    
    /***************************************************************/    
    
    /**
     * \brief Baseclass for user functions called for each
     *  polyhedron of a volumetric restricted Voronoi diagram.
     * \details A volumetric restricted Voronoi diagram is the
     *  intersection between Voronoi cells and the tetrahedra of
     *  a given tetrahedral mesh. The member functions of this
     *  class are called for each intersection between a Voronoi
     *  cell and a tetrahedron.
     */
    class GEOGRAM_API RVDPolyhedronCallback : public RVDCallback {
      public:

	/**
	 * \brief PolyhedronCallback constructor.
	 */
	RVDPolyhedronCallback();
	
	/**
	 * \brief PolyhedronCallback destructor.
	 */
	virtual ~RVDPolyhedronCallback();

	/**
	 * \copydoc RVDCallback::begin()
	 */
	virtual void begin();

	/**
	 * \copydoc RVDCallback::end()
	 */
	virtual void end();

	
	/**
	 * \brief The default callback called for each polyhedron
	 * \details This default implementation routes the callback to the 
	 *  begin_polyhedron_internal(), end_polyhedron_internal(), 
	 *  begin_facet_internal(), end_facet_internal() and vertex_internal()
	 *  functions (that in turn route the callbacks to their without 
	 *  "_internal" counterparts).
	 * \param[in] v index of current Delaunay seed
	 * \param[in] t index of current mesh tetrahedron
	 * \param[in] C intersection between current mesh tetrahedron
	 *  and the Voronoi cell of \p v
	 */
	virtual void operator() (
	    index_t v,
	    index_t t,
	    const GEOGen::ConvexCell& C
	) const;
	    

	/**
	 * \brief Called at the beginning of each intersection polyhedron.
	 * \details Each intersection polyhedron is defined as the intersection
	 *   between a Voronoi cell and a tetrahedron.
	 * \param[in] seed index of the seed associated with the Voronoi cell
	 * \param[in] tetrahedron index of the tetrahedron 
	 */
	virtual void begin_polyhedron(index_t seed, index_t tetrahedron);

	/**
	 * \brief Called at the beginning of each facet of each intersection
	 *   polyhedron.
	 * \details A facet can be a subset of either a bisector (defined by
	 *   two seeds), or it can be a subset of a facet of a tetrahedron.
	 * \param[in] facet_seed if the facet corresponds to a bisector,
	 *   the index of the seed that defines the bisector, or index_t(-1)
	 *   otherwise
	 * \param[in] facet_tet if the facet corresponds to a facet
	 *   of the current tetrahedron, the index of the tetrahedron
	 *   adjacent to that facet, or index_t(-1) otherwise
	 */
	virtual void begin_facet(index_t facet_seed, index_t facet_tet);

	/**
	 * \brief Called for each vertex of the current facet.
	 * \param[in] geometry a pointer to the coordinates of the vertex
	 * \param[in] symb the symbolic representation of the vertex
	 */
	virtual void vertex(
	    const double* geometry, const GEOGen::SymbolicVertex& symb
	);

	/**
	 * \brief Called at the end of each polyhedron facet.
	 */
	virtual void end_facet();

	/**
	 * \brief Called at the end of each polyhedron.
	 */
	virtual void end_polyhedron();

	/**
	 * \brief Gets the index of the tetrahedron that corresponds to the 
	 *  current polyhedron.
	 * \return The index of the tetrahedron that corresponds to the 
	 *  current polyhedron.
	 * \details The current polyhedron is the intersection between a Voronoi
	 *  cell and a tetrahedron. 
	 */
	index_t tet() const {
	    return simplex();
	}

	/**
	 * \brief Gets the index of the seed that defines the bisector on which
	 *  the current facet lies, or index_t(-1). 
	 * \return The index of the seed that defines the bisector on which
	 *  the current facet lies, or index_t(-1).
	 * \details Each facet is either on a bisector or on a tetrahedron 
	 *  facet. If the current facet is on a bisector, it is defined by
	 *  seed() and facet_seed(), otherwise facet_seed() returns index_t(-1).
	 */
	index_t facet_seed() const {
	    return facet_seed_;
	}

	/**
	 * \brief Gets the index of the tetrahedron adjacent to the current 
	 *  facet or index_t(-1) if there is no such facet.
	 * \return the index of the tetrahedron adjacent to the current
	 *  facet or index_t(-1).
	 * \details Each facet is either on a bisector or on a tetrahedron 
	 *  facet. If the current facet is on a tetrahedron facet, then it
	 *  is defined by tet() and facet_tet(), otherwise 
	 *  facet_tet() returns index_t(-1).
	 */
	index_t facet_tet() const {
	    return facet_tet_;
	}

	/**
	 * \brief Specifies whether internal tetrahedron facets should be
	 *  removed.
	 * \details If set, a single polyhedron is generated for each
	 *  (connected component) of the restricted Voronoi cells. If not
	 *  set (default), each tetrahedron-Voronoi cell intersection 
	 *  generates a new polyhedron.
	 * \param[in] x true if internal facets should be removed, false
	 *  otherwise
	 */
	void set_simplify_internal_tet_facets(bool x) {
	    simplify_internal_tet_facets_ = x;
	}

	/**
	 * \brief Specifies whether Voronoi facets should be simplified.
	 * \details By default, the computed Voronoi facets are composed
	 *  of multiple polyhedra that correspond to the intersection with
	 *  the tetrahedra of the input volume mesh. They can be simplified
	 *  and replaced by a single polygon. This implies simplifying the
	 *  internal tetrahedron facets and using a mesh.
	 * \param[in] x true if Voronoi facets should be simplified, 
	 *  false otherwise.
	 */
	void set_simplify_voronoi_facets(bool x) {
	    simplify_voronoi_facets_ = x;
	    if(x) {
		set_simplify_internal_tet_facets(true);
		set_use_mesh(true);
	    }
	}

	/**
	 * \brief Specifies whether boundary facets should be simplified.
	 * \details By default, the intersection between a Voronoi cell and
	 *  the boundary is possibly composed of multiple polygons, that 
	 *  correspond to the initial polygons of the boundary. They can be 
	 *  simplified as a single polygon per Voronoi cell. This implies 
	 *  simplifying the internal tetrahedron facets, simplifying the 
	 *  Voronoi facets and using a mesh.
	 * \param[in] x true if boundary facets should be simplified, 
	 *  false otherwise.
	 * \param[in] angle_threshold an edge shared by two adjacent facets 
	 *  is suppressed if the angle between the facet normals is smaller 
	 *  than \p angle_threshold
	 */
	void set_simplify_boundary_facets(bool x, double angle_threshold=45.0) {
	    simplify_boundary_facets_ = x;
	    if(x) {
		set_simplify_voronoi_facets(true);
		simplify_boundary_facets_angle_threshold_ = angle_threshold;
	    } else {
		simplify_boundary_facets_angle_threshold_ = 0.0;		
	    }
	}

	/**
	 * \brief Specifies whether non-convex facets should be tessellated.
	 * \param[in] x true if non-convex facets should be tessellated, 
	 *  false otherwise.
	 * \details Only taken into account if set_use_mesh(true) was called.
	 */
	void set_tessellate_non_convex_facets(bool x) {
	    tessellate_non_convex_facets_ = x;
	}

	
	/**
	 * \brief Specifies whether a mesh should be built for each
	 *  traversed polyhedron.
	 * \details The build mesh can then be modified (e.g., simplified)
	 *  by overloading process_mesh().
	 * \param[in] x true if a mesh should be build, false otherwise
	 */
	void set_use_mesh(bool x);

	/**
	 * \brief Sets the dimension of the internal mesh if need be.
	 * \details This function is called automatically by 
	 *  RestrictedVoronoiDiagram::for_each_polyhedron().
	 * \param[in] dim the dimension of the mesh (3 for 3d).
	 */
	void set_dimension(index_t dim) {
	    if(use_mesh_) {
		mesh_.vertices.set_dimension(dim);
	    }
	}

      protected:
	/**
	 * \brief Filters callbacks between operator() and client callbacks.
	 * \details This is used to implement cells simplifications (remove
	 *  internal boundaries and intersections with tetrahedra).
	 * \see begin_polyhedron()
	 */
	virtual void begin_polyhedron_internal(
	    index_t seed, index_t tetrahedron
	);

	/**
	 * \brief Filters callbacks between operator() and client callbacks.
	 * \details This is used to implement cells simplifications (remove
	 *  internal boundaries and intersections with tetrahedra).
	 * \see begin_facet()
	 */
	virtual void begin_facet_internal(
	    index_t facet_seed, index_t facet_tet
	);

	/**
	 * \brief Filters callbacks between operator() and client callbacks.
	 * \details This is used to implement cells simplifications (remove
	 *  internal boundaries and intersections with tetrahedra).
	 * \see vertex()
	 */
	virtual void vertex_internal(
	    const double* geometry, const GEOGen::SymbolicVertex& symb
	);

	/**
	 * \brief Filters callbacks between operator() and client callbacks.
	 * \details This is used to implement cells simplifications (remove
	 *  internal boundaries and intersections with tetrahedra).
	 * \see end_facet()
	 */
	virtual void end_facet_internal();

	/**
	 * \brief Filters callbacks between operator() and client callbacks.
	 * \details This is used to implement cells simplifications (remove
	 *  internal boundaries and intersections with tetrahedra).
	 * \see end_polyhedron()
	 */
	virtual void end_polyhedron_internal();

	/**
	 * \brief If use_mesh is set, then this function is called for 
	 *  each generated mesh.
	 * \details Default implementation simplifies the mesh based on
	 *  sipmlify_xxx flags, then it calls user callbacks
	 *  begin_facet(), end_facet(), vertex() for each facet of the mesh,
	 *  as well as begin_polyhedron() and end_polyhedron() once per mesh.
	 *  Derived classes may modify (e.g., simplify) the mesh before calling
	 *  user callbacks.
	 */
	virtual void process_polyhedron_mesh();
	
      protected:
	
	index_t facet_seed_;
	index_t facet_tet_;
	index_t last_seed_;

	bool simplify_internal_tet_facets_;
	bool simplify_voronoi_facets_;
	bool simplify_boundary_facets_;
	double simplify_boundary_facets_angle_threshold_;
	bool tessellate_non_convex_facets_;
	
	bool use_mesh_;
	bool facet_is_skipped_;

	Mesh mesh_;
	Attribute<GEOGen::SymbolicVertex> mesh_vertex_sym_;
	Attribute<index_t> mesh_facet_seed_;
	Attribute<index_t> mesh_facet_tet_;
	RVDVertexMap* vertex_map_;
	vector<index_t> base_current_facet_;
    };

    /***************************************************************/    

    /**
     * \brief Constructs a polyhedral mesh from a restricted Voronoi diagram.
     * \details Its member functions are called for each RVD polyhedron, 
     *  i.e. the intersections between the volumetric mesh tetrahedra and
     *  the Voronoi cells. Based on set_simplify_xxx(), a smaller number of
     *  polyhedra can be generated.
     */
    class GEOGRAM_API BuildRVDMesh : public RVDPolyhedronCallback {
    public:

	/**
	 * \brief BuildRVDMesh constructor.
	 * \param[out] output_mesh a reference to the generated mesh 
	 */
	BuildRVDMesh(Mesh& output_mesh);

	/**
	 * \brief BuildRVDMesh destructor.
	 */
	~BuildRVDMesh();

	/**
	 * \brief Specifies whether ids should be generated.
	 * \details If enabled, unique vertex ids, seed ids and cell ids are
	 *  generated and attached to the mesh vertices ("vertex_id" attribute)
	 *  and mesh facets ("seed_id" and "cell_id" attributes) respectively.
	 *  There is a cell_id per generated polyhedron, and seed_id refers to
	 *  the Voronoi seed (the point that the Voronoi cell is associated 
	 *  with).
	 * \param[in] x true if ids should be generated, false
	 *  otherwise (default)
	 */
	void set_generate_ids(bool x);
	
	/**
	 * \brief Defines the optional shrink factor for cells.
	 * \param[in] x shrink factor, 0.0 means no shrink, 1.0 means
	 *  maximum shrink (cell reduced to a point).
	 */
	void set_shrink(double x);
	
	/**
	 * \brief Called at the beginning of RVD traversal.
	 */
	virtual void begin();

	/**
	 * \brief Called at the end of RVD traversal.
	 */
	virtual void end();

	/**
	 * \brief Called at the beginning of each RVD polyhedron.
	 * \param[in] seed , tetrahedron the (seed,tetrahedron) pair that
	 *  defines the RVD polyhedron, as the intersection between the Voronoi
	 *  cell of the seed and the tetrahedron.
	 */
	virtual void begin_polyhedron(index_t seed, index_t tetrahedron);

	/**
	 * \copydoc RVDPolyhedronCallback::begin_facet()
	 */
	virtual void begin_facet(index_t facet_seed, index_t facet_tet_facet);

	/**
	 * \copydoc RVDPolyhedronCallback::vertex()
	 */
	virtual void vertex(
	    const double* geometry, const GEOGen::SymbolicVertex& symb
	);

	/**
	 * \copydoc RVDPolyhedronCallback::end_facet()
	 */
	virtual void end_facet();

	/**
	 * \copydoc RVDPolyhedronCallback::end_polyhedron()
	 */
	virtual void end_polyhedron();

	/**
	 * \copydoc RVDPolyhedronCallback::process_polyhedron_mesh()
	 */
	virtual void process_polyhedron_mesh();
	
    private:
	vector<index_t> current_facet_; 
	Mesh& output_mesh_;
	RVDVertexMap* global_vertex_map_;
	RVDVertexMap* cell_vertex_map_;
	double shrink_;
	bool generate_ids_;
	Attribute<int> cell_id_;
	Attribute<int> seed_id_;
	Attribute<int> vertex_id_;
	Attribute<int> facet_seed_id_;
	index_t current_cell_id_;
    };


    /***************************************************************/    
     
}

#endif

