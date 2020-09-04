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

#ifndef GEOGRAM_VORONOI_RVD
#define GEOGRAM_VORONOI_RVD

#include <geogram/basic/common.h>
#include <geogram/mesh/index.h>
#include <geogram/mesh/mesh.h>
#include <geogram/basic/geometry.h>
#include <geogram/basic/smart_pointer.h>
#include <geogram/basic/counted.h>
#include <geogram/basic/attributes.h>

#include <vector>

/**
 * \file geogram/voronoi/RVD.h
 * \brief Class and functions to compute restricted Voronoi diagrams
 *  and extract information from them.
 */


namespace GEOGen {
    class PointAllocator;
}

namespace GEO {

    class Delaunay;
    class Map;
    class IntegrationSimplex;
    class MeshFacetsAABB;
    class RVDPolyhedronCallback;
    class RVDPolygonCallback;

    /**
     * \brief Computes a Restricted Voronoi Diagram (RVD).
     *
     * \details A Restricted Voronoi Diagram is the intersection
     * between a surface mesh and a Voronoi diagram, possibly
     * embedded in high-dimensional space. This class is used
     *  (mostly) by CentroidalVoronoiTesselation (CVT),
     * that distributes points uniformly over a surface. This
     * class does all the geometric work for CVT.
     *
     * \note This class is mainly a wrapper around the generic implementation
     * GEOGen::RestrictedVoronoiDiagram.
     *
     * \see CentroidalVoronoiTesselation
     * \see GEOGen::RestrictedVoronoiDiagram
     */
    class GEOGRAM_API RestrictedVoronoiDiagram : public Counted {
    public:
        /**
         * \brief Creates a RestrictedVoronoiDiagram.
         *
         * \details The dimension is determined by \p mesh->dimension().
         * \param[in] delaunay the Delaunay triangulation that defines the
         * Voronoi diagram.
         * \param[in] mesh the mesh that restricts the Voronoi diagram
         * \param[in] R3_embedding gives for each vertex
         *  its mapping in 3D space.
         * \param[in] R3_embedding_stride gives the stride between
         *  two consecutive vertices in R3_embedding
         */
        static RestrictedVoronoiDiagram* create(
            Delaunay* delaunay, Mesh* mesh,
            const double* R3_embedding, index_t R3_embedding_stride
        );

        /**
         * \brief Creates a RestrictedVoronoiDiagram.
         *
         * \details The dimension is determined by \p mesh->dimension().
         * The first three coordinates of each vertex are supposed to be x,y,z.
         * (if it is not the case, use
         *  create(Delaunay*,Mesh*,const double*, index_t) or
         *  create(Delaunay*,Mesh*,const vector<vec3>&) instead).
         * \param[in] delaunay the Delaunay triangulation that defines the
         * Voronoi diagram.
         * \param[in] mesh the mesh that restricts the Voronoi diagram
         */
        static RestrictedVoronoiDiagram* create(
            Delaunay* delaunay, Mesh* mesh
        ) {
            return create(
                delaunay, mesh,
                (mesh->vertices.nb() > 0) ? mesh->vertices.point_ptr(0) : nullptr,
                mesh->vertices.dimension()
            );
        }

        /**
         * \brief Creates a RestrictedVoronoiDiagram.
         *
         * \details Use this function if the nD coordinates of each mesh vertex
         * are completely unrelated with x,y,z.
         * The dimension is determined by \p mesh->dimension().
         * \param[in] delaunay the Delaunay triangulation that defines the
         * Voronoi diagram.
         * \param[in] mesh the mesh that restricts the Voronoi diagram
         * \param[in] R3_embedding gives for each vertex its mapping
         *  in 3D space.
         */
        static RestrictedVoronoiDiagram* create(
            Delaunay* delaunay, Mesh* mesh,
            const vector<vec3>& R3_embedding
        ) {
            return create(delaunay, mesh, R3_embedding[0].data(), 3);
        }

        /**
         * \brief Gets the dimension used by this RestrictedVoronoiDiagram.
         */
        coord_index_t dimension() const {
            return dimension_;
        }

        /**
         * \brief Gets the Delaunay triangulation.
         */
        Delaunay* delaunay() {
            return delaunay_;
        }

        /**
         * \brief Sets the Delaunay triangulation.
         */
        virtual void set_delaunay(Delaunay* delaunay);

        /**
         * \brief Tests whether volumetric mode is used.
         */
        bool volumetric() const {
            return volumetric_;
        }

        /**
         * \brief Sets volumetric mode.
         * \param[in] x if true, volumetric mode is used, otherwise
         *  surfacic mode is used.
         */
        virtual void set_volumetric(bool x) = 0;

        /**
         * \brief Computes a random initial sampling in nD.
         * \details Depending on the value of the volumetric() flag,
         *  this functions samples either the triangles or the tetrahedra
         *  of the mesh. The coordinates of the computed points are stored in
         *  array \p p which must be large enough to contain
         *  \c dimension()*nb_points point coordinates.
         * \param[out] p stores the computed points.
         * \param[in] nb_points number of points to compute
         */
        bool compute_initial_sampling(
            double* p, index_t nb_points
        ) {
            bool result = true;
            if(volumetric()) {
                result = compute_initial_sampling_in_volume(p, nb_points);
            } else {
                result = compute_initial_sampling_on_surface(p, nb_points);
            }
            return result;
        }

        /**
         * \brief Computes a random initial sampling of the surface in nD.
         *
         * \details This function is used to initialize
         *  a CentroidalVoronoiTesselation. The coordinates of the computed
         *  points are stored in array \p p which must be large enough to
         *  contain \c dimension()*nb_points point coordinates.
         * \param[out] p stores the computed points
         * \param[in] nb_points number of points to compute
         */
        virtual bool compute_initial_sampling_on_surface(
            double* p, index_t nb_points
        ) = 0;

        /**
         * \brief Computes a random initial sampling of the volume in nD.
         *
         * \details This function is used to initialize
         *  a CentroidalVoronoiTesselation. The coordinates of the computed
         *  points are stored in array \p p which must be large enough to
         *  contain \c dimension()*nb_points point coordinates.
         * \param[out] p stores the computed points
         * \param[in] nb_points number of points to compute
         */
        virtual bool compute_initial_sampling_in_volume(
            double* p, index_t nb_points
        ) = 0;

        /**
         * \brief Computes the centroids and masses
         *  of the Voronoi cells restricted to the surface.
         *
         * \details This function is used by Lloyd relaxation in
         *  CentroidalVoronoiTesselation.
         *
         * \param[out] mg (size = dimension()*delaunay()->nb_vertices()) :
         *  stores for each point the mass times the centroid of
         *  the restricted voronoi cell.
         * \param[out] m (size = delaunay()->nb_vertices()) :
         *  stores for each point the mass of the restricted voronoi cell.
         */
        virtual void compute_centroids_on_surface(double* mg, double* m) = 0;

        /**
         * \brief Computes the centroids and masses
         *  of the Voronoi cells restricted to the volume.
         *
         * \details This function is used by Lloyd relaxation in
         *  CentroidalVoronoiTesselation.
         *
         * \param[out] mg (size = dimension()*delaunay()->nb_vertices()) :
         *  stores for each point the mass times the centroid of
         *  the restricted voronoi cell.
         * \param[out] m (size = delaunay()->nb_vertices()) :
         *  stores for each point the mass of the restricted voronoi cell.
         */
        virtual void compute_centroids_in_volume(double* mg, double* m) = 0;

        /**
         * \brief Computes the centroids and masses
         *  of the restricted Voronoi cells.
         *
         * \details Depending on the volumetric() flag, the
         *  Voronoi cells are restricted to the surface (facets of the mesh)
         *  or to the volume (tetrahedra of the mesh). This function is used
         *  by Lloyd relaxation in CentroidalVoronoiTesselation.
         *
         * \param[out] mg (size = dimension()*delaunay()->nb_vertices()) :
         *  stores for each point the mass times the centroid of
         *  the restricted voronoi cell.
         * \param[out] m (size = delaunay()->nb_vertices()) :
         *  stores for each point the mass of the restricted voronoi cell.
         */
        void compute_centroids(double* mg, double* m) {
            if(volumetric()) {
                compute_centroids_in_volume(mg, m);
            } else {
                compute_centroids_on_surface(mg, m);
            }
        }

        /**
         * \brief Computes the value and gradient of
         *  Lloyd's function (quantization noise power) on
         *  the surface.
         *
         * \details This function is used by Newton optimization
         * in CentroidalVoronoiTesselation.
         *
         * \param[out] f the computed value of the quantization noise power.
         * \param[out] g (size = dimension()*delaunay()->nb_vertices()) :
         *  the gradient of the quantization noise power.
         */
        virtual void compute_CVT_func_grad_on_surface(double& f, double* g) = 0;

        /**
         * \brief Computes the value and gradient of
         *  Lloyd's function (quantization noise power) in
         *  the volume.
         *
         * \details This function is used by Newton optimization
         * in CentroidalVoronoiTesselation.
         *
         * \param[out] f the computed value of the quantization noise power.
         * \param[out] g (size = dimension()*delaunay()->nb_vertices()) :
         *  the gradient of the quantization noise power.
         */
        virtual void compute_CVT_func_grad_in_volume(double& f, double* g) = 0;

        /**
         * \brief Computes the value and gradient of
         *  Lloyd's function (quantization noise power).
         *
         * \details Does computations either on the
         *  surface (triangles of the mesh) or in the volume
         *  (tetrahedra of the mesh), depending on the
         *   volumetric() flag.
         *  This function is used by Newton optimization
         * in CentroidalVoronoiTesselation.
         *
         * \param[out] f the computed value of the quantization noise power.
         * \param[out] g (size = dimension()*delaunay()->nb_vertices()) :
         *  the gradient of the quantization noise power.
         */
        void compute_CVT_func_grad(double& f, double* g) {
            if(volumetric()) {
                compute_CVT_func_grad_in_volume(f, g);
            } else {
                compute_CVT_func_grad_on_surface(f, g);
            }
        }

        /**
         * \brief Computes the value and gradient of
         *  an objective function over Voronoi cells decomposed
         *  into simplices.
         * \details This function is used by Newton optimization
         * in CentroidalVoronoiTesselation.
         * \param[out] f the value of the objective function
         * \param[out] g the gradient of the objective function
         * \param[in,out] F the object that computes the objective function
         *  and its gradients over a simplex. The contribution of each simplex
         *  is added to the gradient referenced by \p F.
         */
        virtual void compute_integration_simplex_func_grad(
            double& f, double* g, IntegrationSimplex* F
        )=0; 

        /**
         * \brief Computes the projection of points onto the surface
         *  in nD space.
         * \param[in] nb_points number of points to projects
         * \param[in] points array of the coordinates of the points to
         * project. Must contain at least \c dimension()*nb_points
         * coordinates.
         * \param[out] nearest (size=nb_points) the computed projections
         *  mapped to 3D space.
         * \param[in] do_project if set, the input points are moved and
         * projected onto the surface, else only the 3D projections
         * are computed, without changing the input.
         */
        virtual void project_points_on_surface(
            index_t nb_points, double* points,
            vec3* nearest, bool do_project = false
        ) = 0;

        /**
         * \brief Computes the projection of points onto the surface
         *  in nD space.
         * \param[in] nb_points number of points to projects
         * \param[in] points array of the coordinates of the points to
         * project. Must contain at least \c dimension()*nb_points
         * coordinates.
         * \param[out] nearest the computed projections
         *  mapped to 3D space.
         * \param[in] do_project if set, the input points are moved and
         * projected onto the surface, else only the 3D projections
         * are computed, without changing the input.
         */
        void project_points_on_surface(
            index_t nb_points, double* points,
            vector<vec3>& nearest, bool do_project = false
        ) {
            nearest.resize(nb_points);
            project_points_on_surface(
                nb_points, points, &nearest[0], do_project
            );
        }

        /**
         * \brief Computes the projection of points onto the surface
         *  in nD space.
         * \param[in] nb_points number of points to projects
         * \param[in] points array of the coordinates of the points to
         * project. Must contain at least \c dimension()*nb_points
         * coordinates.
         * \param[out] nearest the computed projections
         *  mapped to 3D space.
         * \param[in] do_project if set, the input points are moved and
         * projected onto the surface, else only the 3D projections
         * are computed, without changing the input.
         */
        void project_points_on_surface(
            index_t nb_points, double* points,
            vector<double>& nearest, bool do_project = false
        ) {
            nearest.resize(nb_points * 3);
            project_points_on_surface(
                nb_points, points, (vec3*) (&nearest[0]), do_project
            );
        }


        /**
         * \brief Determines the operating mode of 
         *  compute_RDT().
         * \details The flags can be combined with the 
         *  'bitwise or' (|) operator.
         */
        enum RDTMode {

            /**
             * \brief Always use Delaunay seeds as
             *  vertex geometry.
             */
            RDT_SEEDS_ALWAYS=0,

            /** 
             * \brief If set, the dual of the connected
             *  components of the restricted Voronoi diagram
             *  is computed.
             */
            RDT_MULTINERVE=1,

            /**
             * \brief If set, the vertices are generated
             *  at the centroids of the restricted Voronoi cells,
             *  instead of using the seeds.
             */
            RDT_RVC_CENTROIDS=2,

            /**
             * \brief If set, the seeds are used whenever possible,
             *  i.e. whenever a restricted Voronoi cell has a single
             *  connected component. 
             */
            RDT_PREFER_SEEDS=4,

            /**
             * \brief If set, then the algorithm selects among the
             *  seed and the restricted voronoi cell centroid the
             *  one that is nearest to the surface.
             *  Important: before using this
             *  mode, the surface mesh needs to be reordered with 
             *  Morton order (see GEO::mesh_reorder).
             */
            RDT_SELECT_NEAREST=8,

            /**
             * \brief If set, then all the vertices are projected
             *  onto the surface.
             *  Important: before using this
             *  mode, the surface mesh needs to be reordered with 
             *  Morton order (see GEO::mesh_reorder).
             */
            RDT_PROJECT_ON_SURFACE=16,

            /**
             * \brief If set, the generated mesh is not repaired.
             *   As a result, triangles may be not properly
             *   oriented.
             */
            RDT_DONT_REPAIR=32
        };


        /**
         * \brief Computes the restricted Delaunay triangulation.
         *
         * \param[out] simplices the indices of all triangles vertices
         *   (or tetrahedra vertices in volumetric mode)
         * \param[out] embedding the nD embedding of all vertices
         * \param[in] mode specifies how vertices geometry is
         *  generated in surfacic mode (seeds or restricted Voronoi 
         *  cells centroids)
         * \param[in] seed_is_locked if set, specifies which seed
         *  is locked (size = delaunay()->nb_vertices()). Locked
         *  seeds are not replaced with the restricted Voronoi cell
         *  centroid.
         * \param[in] AABB used if one of (RDT_RVC_PROJECT_ON_SURFACE,
         *   RDT_SELECT_NEAREST) is set in \p mode. If needed but not
         *   specified, then a temporary one is created. 
         */
        virtual void compute_RDT(
            vector<index_t>& simplices,
            vector<double>& embedding,
            RDTMode mode = RDTMode(RDT_RVC_CENTROIDS | RDT_PREFER_SEEDS),
            const vector<bool>& seed_is_locked = vector<bool>(),
            MeshFacetsAABB* AABB = nullptr
        ) = 0;

        /**
         * \brief Computes the restricted Delaunay triangulation.
         * \param[out] RDT the computed restricted Delaunay triangulation
         * \param[in] mode specifies how vertices geometry is
         *  generated in surfacic mode (seeds or restricted Voronoi 
         *  cells centroids)
         * \param[in] seed_is_locked if set, specifies which seed
         *  is locked (size = delaunay()->nb_vertices()). Locked
         *  seeds are not replaced with the restricted Voronoi cell
         *  centroid
         * \param[in] AABB used if one of (RDT_RVC_PROJECT_ON_SURFACE,
         *   RDT_SELECT_NEAREST) is set in \p mode. If needed but not
         *   specified, then a temporary one is created. 
         */
        void compute_RDT(
            Mesh& RDT,
            RDTMode mode = RDTMode(RDT_RVC_CENTROIDS | RDT_PREFER_SEEDS),
            const vector<bool>& seed_is_locked = vector<bool>(),
            MeshFacetsAABB* AABB=nullptr
        );

        /**
         * \brief Computes the restricted Voronoi diagram and stores it
         *  in a mesh.
         * \param[out] M the computed restricted Voronoi diagram
         * \param[in] dim if different from 0, use only the
         *  first dim coordinates
         * \param[in] cell_borders_only in volumetric mode, computes only
         *  the surfacic borders of the volumetric cells (for visualization
         *  purpose)
         * \param[in] integration_simplices in volumetric mode, if set,
         *  the generated tetrahedra systematically have the Voronoi seed
         *  as the first vertex. As a consequence, the mesh is not necessarily
         *  geometrically correct (it may have inverted elements), but it is
         *  algebraically correct (the sum of signed volumes corresponds the
         *  the total volume of each cell).
         */
        virtual void compute_RVD(
            Mesh& M,
            coord_index_t dim = 0,
            bool cell_borders_only = false,
            bool integration_simplices = false
        ) = 0;


        /**
         * \brief Computes a restricted Voronoi cell.
         * \details A restricted Voronoi cell is the intersection
         *  between a Voronoi cell and a mesh.
         * \param[in] i the index of the Voronoi cell
         * \param[in] M the mesh the Voronoi cell will be restricted to.
         *   All its vertices should be of degree 3 (i.e., incident to 
         *   exactly three facets). 
         *   In volumetric mode, the surfacic part of the mesh corresponds
         *   to the boundary of a volume. In surfacic mode, the mesh is
         *   a set of polygonal facets.
         * \param[out] result on exit, contains the intersection of the Voronoi
         *   cell \p i and the mesh \p M
         * \param[in] copy_symbolic_info if true, symbolic
         *   information is copied. An attribute "id" is attached
         *   to the facets. The value of id[f] is either 1 + the index of
         *   the Voronoi vertex that generated with \p i the bisector that
         *   created the facet, or -1-g if the facet was an original facet
         *   of mesh \p M, where g is the index of the original facet in \p M.
         * \note For now, only volumetric mode is implemented.
         */
        virtual void compute_RVC(
            index_t i,
            Mesh& M,
            Mesh& result,
            bool copy_symbolic_info=false
        ) = 0;


	/**
	 * \brief Invokes a user callback for each intersection polyhedron
	 *  of the restricted Voronoi diagram (volumetric mode only).
	 * \details Each intersection polyhedron is defined as the intersection
	 *  between a Voronoi cell and a tetrahedron.
	 * \param[in] callback the set of user callbacks, as an instance of a
	 *  class derived from RVDPolyhedronCallback.
	 * \param[in] symbolic if true, generate symbolic information in the
	 *  vertices
	 * \param[in] connected_comp_priority if true, generate polyhedron 
	 *  intersections associated with the same Voronoi seed in order.
	 * \param[in] parallel if true, tentatively parallelize computation.
	 */
	virtual void for_each_polyhedron(
	    RVDPolyhedronCallback& callback,
	    bool symbolic = true,
	    bool connected_comp_priority = true,
	    bool parallel = false
	) = 0;

	/**
	 * \brief Invokes a user callback for each intersection polygon
	 *  of the restricted Voronoi diagram (surfacic mode only).
	 * \details Each intersection polygon is defined as the intersection
	 *  between a Voronoi cell and a triangle.
	 * \param[in] callback the set of user callbacks, as an instance of a
	 *  class derived from RVDPolygonCallback.
	 * \param[in] symbolic if true, generate symbolic information in the
	 *  vertices
	 * \param[in] connected_comp_priority if true, generate polyhedron 
	 *  intersections associated with the same Voronoi seed in order.
	 * \param[in] parallel if true, tentatively parallelize computation.
	 */
	virtual void for_each_polygon(
	    RVDPolygonCallback& callback,
	    bool symbolic = true,
	    bool connected_comp_priority = true,
	    bool parallel = false
	) = 0;
	
	
        /**
         * \brief Specifies whether the "radius of security"
         *  criterion should be enforced.
         */
        virtual void set_check_SR(bool x) = 0;

        /**
         * \brief Specifies whether exact predicates should
         *  be used.
         */
        virtual void set_exact_predicates(bool x) = 0;

        /**
         * \brief Tests whether exact predicates are used.
         */
        virtual bool exact_predicates() const = 0;

        /**
         * \brief Partitions the mesh and creates
         *  local storage for multithreaded implementation.
         */
        virtual void create_threads() = 0;

        /**
         * \brief Deletes all the local storage associated
         *  with the threads.
         */
        virtual void delete_threads() = 0;

        /**
         * \brief Restricts surfacic computations to a part of the input mesh.
         * \details The part of the input mesh should be specified as
         *    a contiguous range of facet indices.
         * \param[in] facets_begin first facet in the range
         * \param[in] facets_end one past last facet in the range
         */
        virtual void set_facets_range(
            index_t facets_begin, index_t facets_end
        ) = 0;

        /**
         * \brief Restricts volumetric computations to a part of the input mesh.
         * \details The part of the input mesh should be specified as
         *    a contiguous range of tetrahedra indices.
         * \param[in] tets_begin first tetrahedron in the range
         * \param[in] tets_end one past last tetrahedron in the range
         */
        virtual void set_tetrahedra_range(
            index_t tets_begin, index_t tets_end
        ) = 0;

        /**
         * \brief Gets the mapping in R3 of a point.
         * \param[in] v index of the point
         * \return a const reference to the mapping in R3 of the point
         */
        const vec3& R3_embedding(index_t v) const {
            geo_debug_assert(v < mesh_->vertices.nb());
            return *(const vec3*) (R3_embedding_base_ + v * R3_embedding_stride_);
        }

        /**
         * \brief Gets the input mesh.
         */
        Mesh* mesh() {
            return mesh_;
        }

	/**
	 * \brief Gets the PointAllocator.
	 * \return a pointer to the PointAllocator, used
	 *  to create the new vertices generated by 
	 *  intersections.
	 */
	virtual GEOGen::PointAllocator* point_allocator() = 0;
	
    protected:
        /**
         * \brief This constructor is never called directly.
         * \details Use one of the three versions of create() instead.
         */
        RestrictedVoronoiDiagram(
            Delaunay* delaunay, Mesh* mesh,
            const double* R3_embedding, index_t R3_embedding_stride
        );

        /**
         * \brief RestrictedVoronoiDiagram destructor
         */
        virtual ~RestrictedVoronoiDiagram();

    protected:
        coord_index_t dimension_;
        Delaunay* delaunay_;
        Mesh* mesh_;
        const double* R3_embedding_base_;
        index_t R3_embedding_stride_;
        bool has_weights_;
        Attribute<double> vertex_weight_;
        signed_index_t facets_begin_;
        signed_index_t facets_end_;
        signed_index_t tets_begin_;
        signed_index_t tets_end_;
        bool volumetric_;
    };

    /** \brief Smart pointer to a RestrictedVoronoiDiagram object */
    typedef SmartPointer<RestrictedVoronoiDiagram>
        RestrictedVoronoiDiagram_var;
}

#endif

