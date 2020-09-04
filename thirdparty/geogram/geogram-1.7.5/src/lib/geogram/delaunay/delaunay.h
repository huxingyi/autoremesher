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

#ifndef GEOGRAM_DELAUNAY_DELAUNAY
#define GEOGRAM_DELAUNAY_DELAUNAY

#include <geogram/basic/common.h>
#include <geogram/basic/counted.h>
#include <geogram/basic/smart_pointer.h>
#include <geogram/basic/packed_arrays.h>
#include <geogram/basic/factory.h>
#include <stdexcept>

/**
 * \file geogram/delaunay/delaunay.h
 * \brief Abstract interface for Delaunay
 */

namespace GEO {

    class Mesh;

    /************************************************************************/

    /**
     * \brief Abstract interface for Delaunay triangulation in Nd.
     * \details
     * Delaunay objects are created using method create() which
     * uses the Factory service. New Delaunay triangulations can be
     * implemented and registered to the factory using
     * geo_register_Delaunay_creator().
     * \see DelaunayFactory
     * \see geo_register_Delaunay_creator
     */
    class GEOGRAM_API Delaunay : public Counted {
    public:
        /**
         * \brief Invalid dimension exception
         * \details This exception is thrown by the Delaunay derived
         * constructors if the dimension in the constructor is not supported
         * by the implementation
         */
        struct InvalidDimension : std::logic_error {
            /**
             * \brief Creates a invalid dimension exception
             * \param[in] dimension the specified dimension
             * \param[in] name the name of the Delaunay implementation
             * \param[in] expected the expected dimension
             */
            InvalidDimension(
                coord_index_t dimension,
                const char* name,
                const char* expected
            );

            /**
             * \brief Gets the string identifying the exception
             */
            virtual const char* what() const GEO_NOEXCEPT;
        };


        /**
         * \brief Invalid input exception
         * \details This exception is thrown by Delaunay implementations
         *  in constrained mode, when constraints self-intersect.
         */
        struct InvalidInput : std::logic_error {

            /**
             * \brief InvalidInput constructor.
             * \param[in] error_code_in an implementation-dependent error code
             */
            InvalidInput(int error_code_in);

            /**
             * \brief InvalidInput copy constructor.
             * \param[in] rhs a const reference to the InvalidInput to be copied
             */
            InvalidInput(const InvalidInput& rhs);

            virtual ~InvalidInput() GEO_NOEXCEPT;
            
            /**
             * \brief Gets the string identifying the exception
             */
            virtual const char* what() const GEO_NOEXCEPT;

            /**
             * \brief An implementation-dependent error code.
             */
            int error_code;

            /**
             * \brief The indices of the constrained facets that
             *  have an intersection (or that are duplicated).
             */
            vector<index_t> invalid_facets;
        };
        
        /**
         * \brief Creates a Delaunay triangulation of the
         *  specified dimension.
         * \param[in] dim dimension of the triangulation
         * \param[in] name name of the implementation to use:
         * - "tetgen" - Delaunay with the Tetgen library (dimension 3 only)
         * - "BDEL" - Delaunay in 3D (dimension 3 only)
         * - "BPOW" - Weighted regular 3D triangulation (dimension 4 only)
         * - "NN" - Delaunay with NearestNeighborSearch (any dimension)
         * - "default" - uses the command line argument "algo:delaunay"
         * \retval nullptr if \p format is not a valid Delaunay algorithm name.
         * \retval otherwise, a pointer to a Delaunay algorithm object. The
         * returned pointer must be stored in an Delaunay_var that does
         * automatic destruction:
         * \code
         * Delaunay_var handler = Delaunay::create(3, "default");
         * \endcode
         */
        static Delaunay* create(
            coord_index_t dim, const std::string& name = "default"
        );


        /**
         * \brief This function needs to be called once before
         *  using the Delaunay class.
         * \details registers the factories.
         */
        static void initialize();

        /**
         * \brief Gets the dimension of this Delaunay.
         * \return the dimension of this Delauna
         */
        coord_index_t dimension() const {
            return dimension_;
        }

        /**
         * \brief Gets the number of vertices in each cell
         * \details Cell_size =  dimension + 1
         * \return the number of vertices in each cell
         */
        index_t cell_size() const {
            return cell_size_;
        }

        /**
         * \brief Sets the vertices of this Delaunay, and recomputes the cells.
         * \param[in] nb_vertices number of vertices
         * \param[in] vertices a pointer to the coordinates of the vertices, as
         *  a contiguous array of doubles
         */
        virtual void set_vertices(
            index_t nb_vertices, const double* vertices
        );


        /**
         * \brief Specifies whether vertices should be reordered.
         * \details Reordering is activated by default. Some special
         *  usages of Delaunay3d may require to deactivate it (for
         *  instance if vertices are already known to be ordered).
         * \param[in] x if true, then vertices are reordered using
         *  BRIO-Hilbert ordering. This improves speed significantly
         *  (enabled by default).
         */
        void set_reorder(bool x) {
            do_reorder_ = x;
        }


        /**
         * \brief Specifies the bounds of each level to be used
         *  when hierarchic ordering is specified from outside.
         * \details This function is used by some implementation
         *  when set_reorder(false) was called.
         * \param[in] levels specifies the bounds of each level
         *  used by the hierarchical index. First level has
         *  indices between levels[0] ... levels[1].
         */
        virtual void set_BRIO_levels(const vector<index_t>& levels);

        /**
         * \brief Gets a pointer to the array of vertices.
         * \return A const pointer to the array of vertices.
         */
        const double* vertices_ptr() const {
            return vertices_;
        }

        /**
         * \brief Gets a pointer to a vertex by its global index.
         * \param[in] i global index of the vertex
         * \return a pointer to vertex \p i
         */
        const double* vertex_ptr(index_t i) const {
            geo_debug_assert(i < nb_vertices());
            return vertices_ + vertex_stride_ * i;
        }

        /**
         * \brief Gets the number of vertices.
         * \return the number of vertices in this Delaunay
         */
        index_t nb_vertices() const {
            return nb_vertices_;
        }

        /**
         * \brief Tests whether constraints are supported
         *  by this Delaunay.
         * \retval true if constraints are supported
         * \retval false otherwise
         */
        virtual bool supports_constraints() const;

        /**
         * \brief Defines the constraints.
         * \details The triangulation will be constrained
         *  to pass through the vertices and triangles of
         *  the mesh. This function should be called
         *  before set_vertices().
         * \param[in] mesh the definition of the constraints
         * \pre constraints_supported()
         */
        virtual void set_constraints(const Mesh* mesh) {
            geo_assert(supports_constraints());
            constraints_ = mesh;
        }

        /**
         * \brief Specifies whether the mesh should be refined.
         * \details If set, then the mesh elements are improved
         *  by inserting additional vertices in the mesh.
         *  It is not taken into account by all implementations.
         *  This function should be called before set_vertices().
         * \param[in] x true if the mesh should be refined, false 
         *  otherwise.
         */
        void set_refine(bool x) {
            refine_ = x;
        }

        /**
         * \brief Tests whether mesh refinement is selected.
         * \retval true if mesh refinement is selected
         * \retval false otherwise
         * \see set_refine()
         */
        bool get_refine() const {
            return refine_;
        }

        /**
         * \brief Specifies the desired quality for mesh elements
         *  when refinement is enabled (\see set_refine).
         * \details
         *  Only taken into account after set_refine(true) is called.
         *  It is not taken into account by all implementations.
         *  This function should be called before set_vertices().
         * \param[in] qual typically in [1.0, 2.0], specifies
         *  the desired quality of mesh elements (1.0 means maximum
         *  quality, and generates a higher number of elements).
         */
        void set_quality(double qual) {
            quality_ = qual;
        }
        
        /**
         * \brief Gets the constraints.
         * \return the constraints or nullptr if no constraints
         *  were definied.
         */
        const Mesh* constraints() const {
            return constraints_;
        }

        /**
         * \brief Gets the number of cells.
         * \return the number of cells in this Delaunay
         */
        index_t nb_cells() const {
            return nb_cells_;
        }

        /**
         * \brief Gets the number of finite cells.
         * \pre this function can only be called if
         *   keep_finite is set
         * \details Finite cells have indices 0..nb_finite_cells()-1
         *  and infinite cells have indices nb_finite_cells()..nb_cells()-1
         * \see set_keeps_infinite(), keeps_infinite()
         */
        index_t nb_finite_cells() const {
            geo_debug_assert(keeps_infinite());
            return nb_finite_cells_;
        }

        /**
         * \brief Gets a pointer to the cell-to-vertex incidence array.
         * \return a const pointer to the cell-to-vertex incidence array
         */
        const signed_index_t* cell_to_v() const {
            return cell_to_v_;
        }

        /**
         * \brief Gets a pointer to the cell-to-cell adjacency array.
         * \return a const pointer to the cell-to-cell adjacency array
         */
        const signed_index_t* cell_to_cell() const {
            return cell_to_cell_;
        }

        /**
         * \brief Computes the nearest vertex from a query point.
         * \param[in] p query point
         * \return the index of the nearest vertex
         */
        virtual index_t nearest_vertex(const double* p) const;

        /**
         * \brief Gets a vertex index by cell index and local vertex index.
         * \param[in] c cell index
         * \param[in] lv local vertex index in cell \p c
         * \return the index of the lv-th vertex of cell c.
         */
        signed_index_t cell_vertex(index_t c, index_t lv) const {
            geo_debug_assert(c < nb_cells());
            geo_debug_assert(lv < cell_size());
            return cell_to_v_[c * cell_v_stride_ + lv];
        }

        /**
         * \brief Gets an adjacent cell index by cell index and
         *  local facet index.
         * \param[in] c cell index
         * \param[in] lf local facet index
         * \return the index of the cell adjacent to \p c accros
         *  facet \p lf if it exists, or -1 if on border
         */
        signed_index_t cell_adjacent(index_t c, index_t lf) const {
            geo_debug_assert(c < nb_cells());
            geo_debug_assert(lf < cell_size());
            return cell_to_cell_[c * cell_neigh_stride_ + lf];
        }

        /**
         * \brief Tests whether a cell is infinite.
         * \retval true if cell \p c is infinite
         * \retval false otherwise
         * \see keeps_infinite(), set_keeps_infinite()
         */
        bool cell_is_infinite(index_t c) const;

        /**
         * \brief Tests whether a cell is finite.
         * \retval true if cell \p c is finite
         * \retval false otherwise
         * \see keeps_infinite(), set_keeps_infinite()
         */
        bool cell_is_finite(index_t c) const {
            return !cell_is_infinite(c);
        }
        
        /**
         * \brief Retrieves a local vertex index from cell index
         *  and global vertex index.
         * \param[in] c cell index
         * \param[in] v global vertex index
         * \return the local index of vertex \p v in cell \p c
         * \pre cell \p c is incident to vertex \p v
         */
        index_t index(index_t c, signed_index_t v) const {
            geo_debug_assert(c < nb_cells());
            geo_debug_assert(v < (signed_index_t) nb_vertices());
            for(index_t iv = 0; iv < cell_size(); iv++) {
                if(cell_vertex(c, iv) == v) {
                    return iv;
                }
            }
            geo_assert_not_reached;
        }

        /**
         * \brief Retrieves a local facet index from two adacent
         *  cell global indices.
         * \param[in] c1 global index of first cell
         * \param[in] c2 global index of second cell
         * \return the local index of the face accros which
         *  \p c2 is adjacent to \p c1
         * \pre cell \p c1 and cell \p c2 are adjacent
         */
        index_t adjacent_index(index_t c1, index_t c2) const {
            geo_debug_assert(c1 < nb_cells());
            geo_debug_assert(c2 < nb_cells());
            for(index_t f = 0; f < cell_size(); f++) {
                if(cell_adjacent(c1, f) == signed_index_t(c2)) {
                    return f;
                }
            }
            geo_assert_not_reached;
        }

        /**
         * \brief Gets an incident cell index by a vertex index.
         * \details Can only be used if set_stores_cicl(true) was called.
         * \param[in] v a vertex index
         * \return the index of a cell incident to vertex \p v
         * \see stores_cicl(), set_store_cicl()
         */
        signed_index_t vertex_cell(index_t v) const {
            geo_debug_assert(v < nb_vertices());
            geo_debug_assert(v < v_to_cell_.size());
            return v_to_cell_[v];
        }

        
        /**
         * \brief Traverses the list of cells incident to a vertex.
         * \details Can only be used if set_stores_cicl(true) was called.
         * \param[in] c cell index
         * \param[in] lv local vertex index
         * \return the index of the next cell around vertex \p c or -1 if
         *  \p c was the last one in the list
         * \see stores_cicl(), set_store_cicl()
         */
        signed_index_t next_around_vertex(index_t c, index_t lv) const {
            geo_debug_assert(c < nb_cells());
            geo_debug_assert(lv < cell_size());
            return cicl_[cell_size() * c + lv];
        }

        /**
         * \brief Gets the one-ring neighbors of vertex v.
         * \details Depending on store_neighbors_ internal flag, the
         *  neighbors are computed or copied from the previously computed
         *  list. 
         * \param[in] v vertex index
         * \param[out] neighbors indices of the one-ring neighbors of
         *  vertex \p v
         * \see stores_neighbors(), set_stores_neighbors()
         */
        void get_neighbors(
            index_t v, vector<index_t>& neighbors
        ) const {
            geo_debug_assert(v < nb_vertices());
            if(store_neighbors_) {
                neighbors_.get_array(v, neighbors);
            } else {
                get_neighbors_internal(v, neighbors);
            }
        }

        /**
         * \brief Saves the histogram of vertex degree (can be
         *  visualized with gnuplot).
         * \param[out] out an ASCII stream where to output the histogram.
         */
        void save_histogram(std::ostream& out) const;

        /**
         * \brief Tests whether neighbors are stored.
         * \details Vertices neighbors (i.e. Delaunay 1-skeleton) can be
         *  stored for faster access (used for instance by
         *  RestrictedVoronoiDiagram).
         * \retval true if neighbors are stored.
         * \retval false otherwise.
         */
        bool stores_neighbors() const {
            return store_neighbors_;
        }

        /**
         * \brief Specifies whether neighbors should be stored.
         * \details Vertices neighbors (i.e. Delaunay 1-skeleton) can be
         *  stored for faster access (used for instance by
         *  RestrictedVoronoiDiagram).
         * \param[in] x if true neighbors will be stored, else they will not
         */
        void set_stores_neighbors(bool x) {
            store_neighbors_ = x;
            if(store_neighbors_) {
                set_stores_cicl(true);
            }
        }

        /**
         * \brief Tests whether incident tetrahedra lists
         *   are stored.
         * \retval true if incident tetrahedra lists are stored.
         * \retval false otherwise.
         */
        bool stores_cicl() const {
            return store_cicl_;
        }

        /**
         * \brief Specifies whether incident tetrahedra lists 
         *   should be stored.
         * \param[in] x if true, incident trahedra lists are stored,
         *   else they are not.
         */
        void set_stores_cicl(bool x) {
            store_cicl_ = x;
        }


        /**
         * \brief Tests whether infinite elements are kept.
         * \retval true if infinite elements are kept
         * \retval false otherwise
         */
        bool keeps_infinite() const {
            return keep_infinite_;
        }

        /**
         * \brief Sets whether infinite elements should be kept.
         * \details Internally, Delaunay implementation uses an 
         *  infinite vertex and infinite simplices indicent to it.
         *  By default they are discarded at the end of set_vertices().
         *  \param[in] x true if infinite elements should be kept, 
         *   false otherwise
         */
        void set_keeps_infinite(bool x) {
            keep_infinite_ = x;
        }
        
        /**
         * \brief Tests whether thread-safe mode is active.
         * \return true if thread-safe mode is active, false otherwise.
         */
        bool thread_safe() const {
            return neighbors_.thread_safe();
        }

        /**
         * \brief Specifies whether thread-safe mode should be used.
         * \param[in] x if true then thread-safe mode will be used, else
         *  it will not.
         */
        void set_thread_safe(bool x) {
            neighbors_.set_thread_safe(x);
        }

        /**
         * \brief Sets the default number of stored neighbors.
         * \details Storage of neighbors is optimized for a default
         *  neighborhood size.
         * \see store_neighbors()
         * \param[in] x default number of stored neighbors
         */
        void set_default_nb_neighbors(index_t x) {
            default_nb_neighbors_ = x;
        }

        /**
         * \brief Gets the default number of stored neighbors.
         * \details Storage of neighbors is optimized for a default
         *  neighborhood size.
         * \see store_neighbors()
         * \return The default number of stored neighbors.
         */
        index_t default_nb_neighbors() const {
            return default_nb_neighbors_;
        }

        /**
         * \brief Frees all memory used for neighbors storage.
         */
        void clear_neighbors() {
            neighbors_.clear();
        }

	/**
	 * \brief Specifies whether all internal regions should be kept.
	 * \details Only relevant in constrained mode.
	 * \param[in] x if true, all internal regions are kept, else only
	 *  the outer most region is kept (default).
	 */
        void set_keep_regions(bool x) {
	    keep_regions_ = x;
	}

	/**
	 * \brief Gets the region id associated with a tetrahedron.
	 * \details Only callable if set_keep_region(true) was called before
	 *  set_vertices() in constrained mode.
	 * \param[in] t a tetrahedron index.
	 * \return the region associated with \p t.
	 */
        virtual index_t region(index_t t) const;
	    
	
    protected:
        /**
         * \brief Creates a new Delaunay triangulation
         * \details This creates a new Delaunay triangulation for the
         * specified \p dimension. Specific implementations of the Delaunay
         * triangulation may not support the specified \p dimension and will
         * throw a InvalidDimension exception.
         * \param[in] dimension dimension of the triangulation
         * \throw InvalidDimension This exception is thrown if the specified
         * \p dimension is not supported by the Delaunay implementation.
         * \note This function is never called directly, use create()
         */
        Delaunay(coord_index_t dimension);

        /**
         * \brief Delaunay destructor.
         */
        virtual ~Delaunay();

        /**
         * \brief Internal implementation for get_neighbors (with vector).
         * \param[in] v index of the Delaunay vertex
         * \param[in,out] neighbors the computed neighbors of vertex \p v.
         *    Its size is used to determine the number of queried neighbors.
         */
        virtual void get_neighbors_internal(
            index_t v, vector<index_t>& neighbors
        ) const;

        /**
         * \brief Sets the arrays that represent the combinatorics
         *  of this Delaunay.
         * \param[in] nb_cells number of cells
         * \param[in] cell_to_v the cell-to-vertex incidence array
         * \param[in] cell_to_cell the cell-to-cell adjacency array
         */
        virtual void set_arrays(
            index_t nb_cells,
            const signed_index_t* cell_to_v, const signed_index_t* cell_to_cell
        );

        /**
         * \brief Stores for each vertex v a cell incident to v.
         */
        virtual void update_v_to_cell();

        /**
         * \brief Updates the circular incident cell lists.
         * \details Used by next_around_vertex().
         */
        virtual void update_cicl();

        /**
         * \brief Computes the stored neighbor lists.
         */
        virtual void update_neighbors();

        /**
         * \brief Sets the circular incident edge list.
         * \param[in] c1 index of a cell
         * \param[in] lv local index of a vertex of \p c1
         * \param[in] c2 index of the next cell around \p c1%'s vertex \p lv
         */
        void set_next_around_vertex(
            index_t c1, index_t lv, index_t c2
        ) {
            geo_debug_assert(c1 < nb_cells());
            geo_debug_assert(c2 < nb_cells());
            geo_debug_assert(lv < cell_size());
            cicl_[cell_size() * c1 + lv] = signed_index_t(c2);
        }

    public:
        /**
         * \brief Stores the neighbors of a vertex.
         * \details Used internally for parallel
         *  computation of the neighborhoods.
         * \param[in] i index of the vertex for which the
         *  neighbors should be stored.
         */
        virtual void store_neighbors_CB(index_t i);

    protected:
        /**
         * \brief Sets the dimension of this Delaunay.
         * \details Updates all the parameters related with
         *  the dimension. This includes vertex_stride (number
         *  of doubles between two consecutive vertices),
         *  cell size (number of vertices in a cell),
         *  cell_v_stride (number of integers between two
         *  consecutive cell vertex arrays),
         *  cell_neigh_stride (number of integers
         *  between two consecutive cell adjacency arrays).
         * \param[in] dim the dimension
         */
        void set_dimension(coord_index_t dim) {
            dimension_ = dim;
            vertex_stride_ = dim;
            cell_size_ = index_t(dim) + 1;
            cell_v_stride_ = cell_size_;
            cell_neigh_stride_ = cell_size_;
        }

        coord_index_t dimension_;
        index_t vertex_stride_;
        index_t cell_size_;
        index_t cell_v_stride_;
        index_t cell_neigh_stride_;

        const double* vertices_;
        index_t nb_vertices_;
        index_t nb_cells_;
        const signed_index_t* cell_to_v_;
        const signed_index_t* cell_to_cell_;
        vector<signed_index_t> v_to_cell_;
        vector<signed_index_t> cicl_;
        bool is_locked_;
        PackedArrays neighbors_;
        bool store_neighbors_;
        index_t default_nb_neighbors_;

        /**
         * \brief If true, uses BRIO reordering
         * (in some implementations)        
         */
        bool do_reorder_; 

        const Mesh* constraints_;

        bool refine_;
        double quality_;

        /**
         * \brief It true, circular incident tet
         * lists are stored.
         */
        bool store_cicl_; 

        /**
         * \brief If true, infinite vertex and
         * infinite simplices are kept.
         */
        bool keep_infinite_;

        /**
         * \brief If keep_infinite_ is true, then
         *  finite cells are 0..nb_finite_cells_-1
         *  and infinite cells are 
         *  nb_finite_cells_ ... nb_cells_
         */
        index_t nb_finite_cells_;

	bool keep_regions_;
    };

    /**
     * \brief Smart pointer that refers to a Delaunay object
     * \relates Delaunay
     */
    typedef SmartPointer<Delaunay> Delaunay_var;

    /**
     * \brief Delaunay Factory
     * \details
     * This Factory is used to create Delaunay objects.
     * It can also be used to register new Delaunay
     * implementations.
     * \see geo_register_Delaunay_creator
     * \see Factory
     * \relates Delaunay
     */
    typedef Factory1<Delaunay, coord_index_t> DelaunayFactory;

    /**
     * \brief Helper macro to register a Delaunay implementation
     * \see DelaunayFactory
     * \relates Delaunay
     */
#define geo_register_Delaunay_creator(type, name) \
    geo_register_creator(GEO::DelaunayFactory, type, name)
}

#endif

