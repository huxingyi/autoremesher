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

#ifndef PERIODIC_DELAUNAY_TRIANGULATION_3D
#define PERIODIC_DELAUNAY_TRIANGULATION_3D

#include <geogram/basic/common.h>
#include <geogram/delaunay/delaunay.h>
#include <geogram/delaunay/periodic.h>
#include <geogram/voronoi/convex_cell.h>
#include <geogram/basic/process.h>
#include <geogram/basic/geometry.h>
#include <stack>

namespace GEO {

    typedef Numeric::uint8 thread_index_t;
    class PeriodicDelaunay3dThread;
    
     
    /**
     * \brief Multithreaded implementation of Delaunay in 3d with 
     *  optional periodic boundary conditions.
     * \details Periodicity is taken into account by detecting the
     *   vertices with Voronoi cells that straddle the boundary and
     *   duplicating them as need be (with an additional propagation to 
     *   have the correct neighborhoods).
     * \see Delaunay3d, ParallelDelaunay3d 
     */
    class GEOGRAM_API PeriodicDelaunay3d : public Delaunay, public Periodic {
    public:

	/**
	 * \brief Gathers some structures used by some algorithms, makes
	 *  multithreading more efficient by avoiding dynamic reallocations.
	 * \details It is used to compute the set of tetrahedra incident to
	 *  a vertex. It gathers a stack and the vector of incident tets
	 *  obtained so far.
	 */
        struct IncidentTetrahedra {
	    std::stack<index_t> S;
	    vector<index_t> incident_tets_set;

	    /**
	     * \brief Clears the set of incident tets.
	     */
	    void clear_incident_tets() {
		incident_tets_set.resize(0);
	    }

	    /**
	     * \brief Inserts a tet into the set of incident tets.
	     * \param[in] t the tet to be inserted.
	     */
	    void add_incident_tet(index_t t) {
		incident_tets_set.push_back(t);
	    }

	    /**
	     * \brief Tests whether a tet belongs to the set of incident 
	     *  tets.
	     * \param[in] t the tet to be tested
	     * \retval true if the tet belongs to the set of incident tets
	     * \retval false otherwise
	     */
	    bool has_incident_tet(index_t t) const {
		for(index_t i=0; i<incident_tets_set.size(); ++i) {
		    if(incident_tets_set[i] == t) {
			return true;
		    }
		}
		return false;
	    }

	    vector<index_t>::const_iterator begin() const {
		return incident_tets_set.begin();
	    }

	    vector<index_t>::const_iterator end() const {
		return incident_tets_set.end();
	    }
        };
	
	
        /**
         * \brief Constructs a new PeriodicDelaunay3d.
	 * \param[in] periodic if true, constructs a periodic triangulation.
	 * \param[in] period the edge length of the periodic domain.
         */
        PeriodicDelaunay3d(bool periodic, double period=1.0);

	/**
	 * \copydoc Delaunay::set_vertices()
	 * \note compute() needs to be called after.
	 */
        virtual void set_vertices(
            index_t nb_vertices, const double* vertices
        );

	/**
	 * \brief Sets the weights.
	 * \param[in] weights pointer to the array of 
	 *  weights. Size is the number of real vertices,
	 *  i.e., the parameter nb_vertices passed to
	 *  set_vertices().
	 * \note compute() needs to be called after.
	 */
	void set_weights(const double* weights);

	/**
	 * \brief Computes the Delaunay triangulation.
	 */
	void compute();

	/**
	 * \brief Use exact predicates in convex cell computations.
	 * \details Convex cell computations are used in periodic
	 *  mode for determining the cells that straddle the domain
	 *  boundary.
	 * \param[in] x true if exact predicates should be used 
	 *  (default), false otherwise.
	 */
	void use_exact_predicates_for_convex_cell(bool x) {
	    convex_cell_exact_predicates_ = x;
	}
	
	/**
	 * \brief Gets a vertex by index.
	 * \param[in] v a vertex index. Can be a virtual
	 *  vertex index when in periodic mode.
	 * \return the 3d point associated with the vertex.
	 *  In periodic mode, if \p v is a virtual vertex, 
	 *  then the translation is applied to the real vertex.
	 */
	vec3 vertex(index_t v) const {
	    if(!periodic_) {
		geo_debug_assert(v < nb_vertices());	    
		return vec3(vertices_ + 3*v);
	    }
	    index_t instance = v/nb_vertices_non_periodic_;
	    v = v%nb_vertices_non_periodic_;
	    vec3 result(vertices_ + 3*v);
	    result.x += double(translation[instance][0]) * period_;
	    result.y += double(translation[instance][1]) * period_;
	    result.z += double(translation[instance][2]) * period_;
	    return result;
	}

	/**
	 * \brief Gets a weight by index.
	 * \param[in] v a vertex index. Can be a virtual
	 *  vertex index in periodic mode.
	 * \return the weight associated with the vertex.
	 */
	double weight(index_t v) const {
	    if(weights_ == nullptr) {
		return 0.0;
	    }
	    return periodic_ ? weights_[periodic_vertex_real(v)] : weights_[v] ;
	}

	/**
	 * \copydoc Delaunay::nearest_vertex()
	 */
        virtual index_t nearest_vertex(const double* p) const;

	/**
	 * \copydoc Delaunay::set_BRIO_levels()
	 */
        virtual void set_BRIO_levels(const vector<index_t>& levels);

	/**
	 * \brief computes the set of tetrahedra that are incident to
	 *  a vertex.
	 * \param[in] v the index of the vertex.
	 * \param[in,out] W a reference to a 
	 *  PeriodicDelaunay3d::IncidentTetrahedra.
	 *  On exit it contains the list of incident tets.
	 */
	void get_incident_tets(index_t v, IncidentTetrahedra& W) const;

	/**
	 * \brief Copies a Laguerre cell from the triangulation.
	 * \details Delaunay neigbhors are stored in ConvexCell vertex global
	 *  indices.
	 * \param[in] i the index of the vertex of which the Laguerre cell
	 *  should be computed.
	 * \param[out] C the Laguerre cell.
	 * \param[in,out] W a reference to a 
	 *  PeriodicDelaunay3d::IncidentTetrahedra
	 */
	void copy_Laguerre_cell_from_Delaunay(
	    GEO::index_t i,
	    ConvexCell& C,
	    IncidentTetrahedra& W
	) const;         

	/**
	 * \brief Copies a Laguerre cell from the triangulation.
	 * \details Delaunay neigbhors are stored in ConvexCell vertex global
	 *  indices.
	 * \param[in] i the index of the vertex of which the Laguerre cell
	 *  should be computed.
	 * \param[out] C the Laguerre cell.
	 */
	void copy_Laguerre_cell_from_Delaunay(
	    GEO::index_t i,
	    ConvexCell& C
	) const {
	    IncidentTetrahedra W;
	    copy_Laguerre_cell_from_Delaunay(i,C,W);
	}
	
	/**
	 * \brief Tests whether the Laguerre diagram has empty cells.
	 * \details If the Laguerre diagram has empty cells, then
	 *  computation is stopped, and all the queries on the Laguerre
	 *  diagram will not work (including the non-empty cells).
	 * \retval true if the Laguerre diagram has empty cells.
	 * \retval false otherwise.
	 */
	bool has_empty_cells() const {
	    return has_empty_cells_;
	}

	/**
	 * \brief Saves the cells in an Alias-Wavefront file.
	 * \param[in] basename the basename of the file. Filename
	 *   is basename_callid.obj, where callid is the number of
	 *   times the function was invoked.
	 * \param[in] clipped if true, clip the cells by the domain
	 *   without saving, else keep the cells as is.
	 */
	void save_cells(const std::string& basename, bool clipped);

   protected:

	/**
	 * \brief Copies a Laguerre cell facet from the triangulation.
	 * \param[in] i the index of the vertex of which the Laguerre cell
	 *  should be computed.
	 * \param[in] Pi the coordinates of vertex \p i
	 * \param[in] wi the weight associated to vertex \p i
	 * \param[in] Pi_len2 the squared length of vertex \p i (considered as
	 *  a vector).
	 * \param[in] t a tetrahedron of the Delaunay triangulation, 
	 *  incident to vertex i
	 * \param[out] C the Laguerre cell.
	 * \param[in,out] W a reference to a 
	 *  PeriodicDelaunay3d::IncidentTetrahedra
	 * \return the local index of vertex \p i within tetrahedron \p t
	 */
	GEO::index_t copy_Laguerre_cell_facet_from_Delaunay(
	    GEO::index_t i,
	    const GEO::vec3& Pi,
	    double wi,
	    double Pi_len2,
	    GEO::index_t t,
	    ConvexCell& C,
	    IncidentTetrahedra& W
	) const;
	 
	 
	/**
	 * \brief Removes unused tetrahedra.
	 * \return the final number of tetrahedra.
	 * \param[in] shrink if true, then array space is shrunk
	 *  to fit the new number of tetrahedra.
	 */
	index_t compress(bool shrink=true);
	
	/**
	 * \copydoc Delaunay::update_v_to_cell()
	 * \details if update_periodic_v_to_cell_ is set to true,
	 *  also updates the map periodic_v_to_cell_ that maps
	 *  each virtual vertex to a tet incident to it.
	 */
	virtual void update_v_to_cell();

	/**
	 * \copydoc Delaunay::update_cicl()
	 */
	virtual void update_cicl();

	/**
	 * \brief Duplicates the points with Voronoi cells 
	 *  that cross the boundary.
	 */
	void handle_periodic_boundaries();

	/**
	 * \brief Computes the periodic vertex instances 
	 *  that should be generated.
	 * \param[in] v vertex index, in 0..nb_vertices_non_periodic_-1
	 * \param[out] C the clipped Laguerre cell
	 * \param[out] use_instance the array of booleans that indicates which
	 *  instance should be generated.
	 * \param[out] cell_is_on_boundary true if the cell 
	 *  has an intersection with the cube, false otherwise.
	 * \param[out] cell_is_outside_cube true if the cell 
	 *  is completely outside the cube, false otherwise.
	 * \param[in,out] W a reference to a 
	 *  PeriodicDelaunay3d::IncidentTetrahedra
	 * \return the number of instances to generate.
	 */
	index_t get_periodic_vertex_instances_to_create(
	    index_t v,
	    ConvexCell& C,
	    bool use_instance[27],
	    bool& cell_is_on_boundary,
	    bool& cell_is_outside_cube,
	    IncidentTetrahedra& W
	);

	/**
	 * \brief Insert vertices from 
	 *  reorder_[b] to reorder_[e-1]
	 * \details If an empty cells is detected, has_empty_cells_ is
	 *  set and the function exits.
	 */
	void insert_vertices(index_t b, index_t e);

	/**
	 * \brief Checks the volume of Laguerre cells.
	 */
	void check_volume();

	/**
	 * \brief Gets a thread by index.
	 * \param[in] t the index of the thread, in 0..nb_threads()-1
	 * \return a pointer to the t-th thread.
	 */
	PeriodicDelaunay3dThread* thread(index_t t) {
	    geo_debug_assert(t < threads_.size());
	    return reinterpret_cast<PeriodicDelaunay3dThread*>(
		threads_[t].get()
	    );
	}

	/**
	 * \brief Gets the number of threads.
	 * \return the number of threads.
	 */
	index_t nb_threads() const {
	    return index_t(threads_.size());
	}
	
    private:
        friend class PeriodicDelaunay3dThread;
	
	bool periodic_;
	double period_;
	
	const double* weights_;
        vector<signed_index_t> cell_to_v_store_;
        vector<signed_index_t> cell_to_cell_store_;
        vector<index_t> cell_next_;
        vector<thread_index_t> cell_thread_;
        ThreadGroup threads_;
        vector<index_t> reorder_;
        vector<index_t> levels_;

        /**
         * Performs additional checks (costly !)
         */
         bool debug_mode_;

        /**
         * Displays the result of the additional checks.
         */
         bool verbose_debug_mode_;

        /**
         * Displays the timing of the core algorithm.
         */
        bool benchmark_mode_;
        

	/**
	 * \brief Bitmask that indicates for each real vertex
	 *  the virtual vertices that were created.
	 */
	vector<Numeric::uint32> vertex_instances_;

	bool update_periodic_v_to_cell_;
	vector<index_t> periodic_v_to_cell_rowptr_;
	vector<index_t> periodic_v_to_cell_data_;
	
	/**
	 * \brief Early detection of empty cells.
	 */
	bool has_empty_cells_;

	/**
	 * \brief Number of reallocations when inserting vertices
	 *  in sequential mode.
	 */
	index_t nb_reallocations_;

	/**
	 * \brief Use exact predicates in convex cell.
	 */
	bool convex_cell_exact_predicates_;
    };
    

}

#endif
