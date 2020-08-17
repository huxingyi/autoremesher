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

#ifndef GEOGRAM_MESH_MESH
#define GEOGRAM_MESH_MESH

#include <geogram/basic/common.h>
#include <geogram/basic/range.h>
#include <geogram/basic/attributes.h>
#include <geogram/basic/geometry.h>

/**
 * \file geogram/mesh/mesh.h
 * \brief The class that represents a mesh.
 */

namespace GEO {

    class Mesh;

    const index_t NO_VERTEX = index_t(-1);
    const index_t NO_EDGE = index_t(-1);
    const index_t NO_FACET = index_t(-1);
    const index_t NO_CELL  = index_t(-1);
    const index_t NO_CORNER = index_t(-1);
    
    /**
     * \brief Base class for mesh sub-element storage.
     * \details Sub-elements are those that cannot exist
     *  independently (such as MeshFacetCorner, MeshCellCorner
     *  and MeshCellFacet).
     * \relates Mesh
     */
    class GEOGRAM_API MeshSubElementsStore {
    public:

        /**
         * \brief Constructs a new MeshSubElementStore.
         * \param[in] mesh a reference to the mesh
         *  this MeshElementStore belongs to.
         */
        MeshSubElementsStore(Mesh& mesh);

        /**
         * \brief MeshElementStore destructor.
         */
        virtual ~MeshSubElementsStore();

        /**
         * \brief Gets the number of (sub-)elements.
         * \return the number of (sub-)elements in this store
         */
        index_t nb() const {
            return nb_;
        }

        /**
         * \brief Gets the attributes manager.
         * \details The returned reference is not a const one,
         *  so that attributes can be bound / unbound / accessed
         *  even if the mesh is const.
         * \return a modifiable reference to the attributes manager.
         */
        AttributesManager& attributes() const {
            return const_cast<AttributesManager&>(attributes_);
        }

        /**
	 * \brief Used by range-based for.
	 * \return The index of the first position.
	 */
        no_iterator begin() const {
	    return no_iterator(0);
	}

        /**
	 * \brief Used by range-based for.
	 * \return The index of one position past the last position.
	 */
        no_iterator end() const {
	    return no_iterator(nb());
	}
    
    protected:
        
        /**
         * \brief Removes all the elements and attributes.
         * \param[in] keep_attributes if true, then all the
         *  existing attribute names / bindings are kept (but 
         *  they are cleared). If false, they are destroyed.
         * \param[in] keep_memory if true, then memory is
         *  kept and can be reused by subsequent mesh
         *  element creations.
         */
        virtual void clear_store(
            bool keep_attributes, bool keep_memory = false
        );

        /**
         * \brief Resizes this MeshSubElementsStore.
         * \details On exit, nb() == new_size, elements are
         *  created or destroyed if needed.
         * \param[in] new_size the desired size
         */
        virtual void resize_store(index_t new_size);

        /**
         * \brief Creates a contiguous chunk of attributes for sub-elements.
         * \param[in] nb number of sub-elements to create
         * \return the index of the first created sub-element
         */
        index_t create_sub_elements(index_t nb) {
            index_t result = nb_;
            if(nb_ + nb > attributes_.size()) {
                index_t new_capacity=nb_ + nb;
                if(nb < 128) {
                    new_capacity = std::max(index_t(16),attributes_.size());
                    while(new_capacity < nb_ + nb) {
                        new_capacity *= 2;
                    }
                }
                attributes_.reserve(new_capacity);
            }
            nb_ += nb;
	    attributes_.resize(nb_);
            return result;
        }

        /**
         * \brief Creates attributes for a sub-element
         * \return the index of the created element
         */
        index_t create_sub_element() {
            index_t result = nb_;
            ++nb_;
            if(attributes_.capacity() < nb_) {
                index_t new_capacity =
		    std::max(index_t(16),attributes_.capacity()*2);
		attributes_.reserve(new_capacity);
            }
	    attributes_.resize(nb_);
            return result;
        }

        /**
         * \brief Makes the size of the store tightly match
         *   the number of the elements.
         * \details When elements are created one by one, the
         *   system may allocate more memory than necessary, to
         *   amortize the cost of container reallocation. Once
         *   the object is constructed, this function may be called
         *   to release the memory that was not used.
         */
        void adjust_store() {
            attributes_.resize(nb_);
        }

        /**
         * \brief Copies a MeshSubElementsStore into
         *   this one.
         * \param[in] rhs a const reference to the 
         *   MeshSubElementsStore to be copied.
         * \param[in] copy_attributes if true, copies
         *   also the attributes, else attributes are
         *   cleared.
         */
        void copy(
            const MeshSubElementsStore& rhs,
            bool copy_attributes = true
        ) {
            nb_ = rhs.nb();
            if(copy_attributes) {
                attributes_.copy(rhs.attributes_);
            } else {
                attributes_.clear(false,false);
                attributes_.resize(rhs.attributes_.size());
            }
        }
        
    protected:
        Mesh& mesh_;
        AttributesManager attributes_;
        index_t nb_;
    };


    /**************************************************************************/

    /**
     * \brief Base class for mesh elements.
     * \details Mesh elements can be created / manipulated indepdendantly,
     *  in contrast with sub-elements that cannot. Mesh elements are
     *  vertices, facets and cells.
     * \relates Mesh
     */
    class GEOGRAM_API MeshElements {
    public:
        MeshElements();
        virtual ~MeshElements();

        /**
         * \brief Deletes a set of elements.
         * \param[in] to_delete a vector of size nb(). If to_delete[e]
         *  is different from 0, then element e will be destroyed, else
         *  it will be kept. On exit, to_delete is modified (it is used
         *  for internal bookkeeping).
         * \param[in] remove_isolated_vertices if true, then the vertices
         *  that are no longer incident to any element are deleted.
         */
        virtual void delete_elements(
            vector<index_t>& to_delete,
            bool remove_isolated_vertices=true
        ) = 0;

        /**
         * \brief Applies a permutation to the elements and their attributes.
         * \details On exit, permutation is modified (used for internal
         *  bookkeeping). Applying a permutation \p permutation is equivalent 
         * to:
         * \code
         * for(i=0; i<permutation.size(); i++) {
         *    data2[i] = data[permutation[i]]
         * }
         * data = data2 ;
         * \endcode
         */
        virtual void permute_elements(vector<index_t>& permutation) = 0;

        /**
         * \brief Removes all the elements and attributes.
         * \param[in] keep_attributes if true, then all the
         *  existing attribute names / bindings are kept (but 
         *  they are cleared). If false, they are destroyed.
         * \param[in] keep_memory if true, then memory is
         *  kept and can be reused by subsequent mesh
         *  element creations.
         */
        virtual void clear(
            bool keep_attributes=true, bool keep_memory=false
        ) = 0;

        /**
         * \brief Removes the last element.
         */
        virtual void pop() = 0;
        
    protected:
        /**
         * \brief Tests whether a vector contains a non-zero value.
         * \details This function is used internally by delete_elements()
         * \param[in] I a vector of signed integers
         * \retval true if \p I contains at least a non-zero value
         * \retval false otherwise
         */
        static bool has_non_zero(const GEO::vector<index_t>& I) {
            for(index_t i = 0; i < I.size(); i++) {
                if(I[i] != 0) {
                    return true;
                }
            }
            return false;
        }
    };

    /**************************************************************************/

    class MeshEdges;
    class MeshFacetCornersStore;
    class MeshCellCornersStore;

    /**
     * \brief The vertices of a mesh.
     * \relates Mesh
     */
    class GEOGRAM_API MeshVertices :
        public MeshSubElementsStore, public MeshElements {
    public:
        MeshVertices(Mesh& mesh);
        virtual ~MeshVertices();

        /**
         * \brief Removes the vertices that have no mesh element
         *  incident to them.
         */
        void remove_isolated();
        
        virtual void delete_elements(
            vector<index_t>& to_delete, bool remove_isolated_vertices=true
        );
        
        virtual void permute_elements(vector<index_t>& permutation);

        /**
         * \brief Creates a new vertex
         * \return the index of the created vertex
         */
        index_t create_vertex() {
            return MeshSubElementsStore::create_sub_element();
        }

        /**
         * \brief Creates a new vertex
         * \param[in] coords a pointer to dimension() coordinates
         * \return the index of the created vertex
         */
        index_t create_vertex(const double* coords) {
            // Sanity check:
            // It is not correct to call create_vertex(point_ptr(v)) since
            // create_vertex may realloc the points coordinates vector, thus
            // invalidate point_ptr(v).
            geo_debug_assert(
                nb() == 0 ||
                coords < point_ptr(0) ||
                coords >= point_ptr(0) + nb() * dimension()
            );
            index_t result = create_vertex();
            for(index_t c=0; c<dimension(); ++c) {
                point_ptr(result)[c] = coords[c];
            }
            return result;
        }
        
        /**
         * \brief Creates a contiguous chunk of vertices.
         * \param[in] nb number of sub-elements to create
         * \return the index of the first created vertex
         */
        index_t create_vertices(index_t nb) {
            return MeshSubElementsStore::create_sub_elements(nb);
        }

        virtual void clear(
            bool keep_attributes=true, bool keep_memory=false
        ) ;

        /**
         * \brief Sets single precision mode.
         * \details Single-precision mode is used for instance
         *  by Vorpaview, to make it more memory efficient.
         *  Existing point coordinates are copied and converted to 
         *  single precision.
         */
        void set_single_precision();

        /**
         * \brief Sets double precision mode.
         * \details Double precision mode is the default.
         *  Single-precision mode is used for instance
         *  by Vorpaview, to make it more memory efficient.
         *  Existing point coordinates are copied and converted to 
         *  double precision.
         */
        void set_double_precision();
        
        /**
         * \brief Tests whether vertices are stored in
         *  single-precision mode.
         * \details Single-precision mode is used for instance
         *  by Vorpaview, to make it more memory efficient.
         */
        bool single_precision() const {
            return point_fp32_.is_bound();
        }

        /**
         * \brief Tests whether vertices are stored in
         *  double-precision mode.
         * \details Double precision mode is the default.
         *  Single-precision mode is used for instance
         *  by Vorpaview, to make it more memory efficient.
         */
        bool double_precision() const {
            return point_.is_bound();
        }
        
        /**
         * \brief Gets the dimension of the vertices.
         * \return the number of coordinates in each vertex
         */
        index_t dimension() const {
            return
                single_precision() ?
                point_fp32_.dimension() :
                point_.dimension() ;
        }

        /**
         * \brief Sets the dimension of the vertices
         * \details Existing coordinates are kept, newly created
         *  coordinates are initialized to zero.
         * \param[in] dim new dimension
         */
        void set_dimension(index_t dim) {
            if(single_precision()) {
                point_fp32_.redim(dim);
            } else {
                point_.redim(dim);
            }
        }

        /**
         * \brief Gets a point
         * \param[in] v the index of the vertex
         * \return a const pointer to the coordinates of the point
         *  that correspond to the vertex
         * \pre !single_precision()
         */
        const double* point_ptr(index_t v) const {
            geo_debug_assert(v < nb());
            geo_debug_assert(!single_precision());
            return &point_[v*point_.dimension()];
        }

        /**
         * \brief Gets a point
         * \param[in] v the vertex, in 0..nb()-1
         * \return a pointer to the coordinates of the point
         *  that correspond to the vertex
         * \pre !single_precision()
         */
        double* point_ptr(index_t v) {
            geo_debug_assert(v < nb());            
            geo_debug_assert(!single_precision());
            return &point_[v*point_.dimension()];
        }


        /**
         * \brief Gets a point
         * \param[in] v the vertex, in 0..nb()-1
         * \return a modifiable reference to the point 
         *  that corresponds to the vertex
         * \pre !single_precision()
         */
        vec3& point(index_t v) {
            geo_debug_assert(v < nb());            
            geo_debug_assert(!single_precision());
            geo_debug_assert(dimension() >= 3);
            return *(vec3*)(&point_[v*point_.dimension()]);
        }

        /**
         * \brief Gets a point
         * \param[in] v the vertex, in 0..nb()-1
         * \return a const reference to the point 
         *  that corresponds to the vertex
         * \pre !single_precision()
         */
        const vec3& point(index_t v) const {
            geo_debug_assert(v < nb());            
            geo_debug_assert(!single_precision());
            geo_debug_assert(dimension() >= 3);
            return *(const vec3*)(&point_[v*point_.dimension()]);
        }
        
        /**
         * \brief Gets a (single-precision) point
         * \param[in] v the index of the vertex
         * \return a const pointer to the coordinates 
         *  of the point that corresponds to the vertex
         * \pre single_precision()
         */
        const float* single_precision_point_ptr(index_t v) const {
            geo_debug_assert(v < nb());
            geo_debug_assert(single_precision());
            return &point_fp32_[v*point_fp32_.dimension()];
        }

        /**
         * \brief Gets a (single-precision) point
         * \param[in] v the index of the vertex
         * \return a pointer to the coordinates of the point
         *  that corresponds to the vertex
         * \pre single_precision()
         */
        float* single_precision_point_ptr(index_t v) {
            geo_debug_assert(v < nb());
            geo_debug_assert(single_precision());
            return &point_fp32_[v*point_fp32_.dimension()];
        }

        /**
         * \brief Assigns all the points.
         * \param[in] points a vector that contains all the coordinates
         *  of the points
         * \param[in] dim the dimension of the points, i.e. number of
         *  coordinates per point
         * \param[in] steal_arg if true, memory is stolen from \p points,
         *  using std::vector::swap (no memory copy). 
         */
        void assign_points(
            vector<double>& points, index_t dim, bool steal_arg
        );

        /**
         * \brief Assigns all the points.
         * \param[in] points a const pointer to the (\p dim * \p nb_pts)
         *  coordinates of al the points 
         * \param[in] dim the dimension of the points, i.e. number of
         *  coordinates per point
         * \param[in] nb_pts number of points
         */
        void assign_points(
            const double* points, index_t dim, index_t nb_pts
        );

        virtual void pop();
        
    protected:
        
        virtual void clear_store(
            bool keep_attributes, bool keep_memory = false
        );
        virtual void resize_store(index_t new_size);

        void bind_point_attribute(index_t dim, bool single_precision=false);

        void copy(const MeshVertices& rhs, bool copy_attributes=true) {
            index_t dim = rhs.dimension();
            if(point_fp32_.is_bound()) {
                point_fp32_.destroy();
            }
            if(point_.is_bound()) {
                point_.destroy();
            }
            MeshSubElementsStore::copy(rhs, copy_attributes);
            if(rhs.single_precision()) {
                point_fp32_.bind_if_is_defined(attributes(),"point_fp32");
                if(!point_fp32_.is_bound()) {
                    point_fp32_.create_vector_attribute(
                        attributes(), "point_fp32", dim
                    );
                }
            } else {
                point_.bind_if_is_defined(attributes(),"point");
                if(!point_.is_bound()) {
                    point_.create_vector_attribute(
                        attributes(), "point", dim
                    );
                }
            }
            // Even if we do not copy the attributes, we need at least
            // to copy the coordinates of the points !!
            if(!copy_attributes) {
                if(rhs.single_precision()) {
                    Memory::copy(
                        single_precision_point_ptr(0),
                        rhs.single_precision_point_ptr(0),
                        rhs.dimension()*rhs.nb()*sizeof(float)
                    );
                } else {
                    Memory::copy(
                        point_ptr(0),
                        rhs.point_ptr(0),
                        rhs.dimension()*rhs.nb()*sizeof(double)
                    );
                }
            }
        }
        
        MeshEdges& edges_;
        MeshFacetCornersStore& facet_corners_;
        MeshCellCornersStore& cell_corners_;
        Attribute<double> point_;
        Attribute<float> point_fp32_;

        friend class Mesh;
        friend class GeogramIOHandler;
    };
    
    /*************************************************************************/

    /**
     * \brief The edges of a mesh.
     * \relates Mesh
     */
    class GEOGRAM_API MeshEdges :
        public MeshSubElementsStore, public MeshElements {
    public:
        MeshEdges(Mesh& mesh);
        virtual ~MeshEdges();

        /**
         * \brief Gets the index of an edge vertex
         * \param[in] e index of the edge
         * \param[in] lv local index of the vertex, in {0,1}
         * \return the global index of vertex \p lv in edge \p e
         */
        index_t vertex(index_t e, index_t lv) const {
            geo_debug_assert(e < nb());
            geo_debug_assert(lv < 2);
            return edge_vertex_[2*e+lv];
        }

        /**
         * \brief Sets a vertex of an edge
         * \param[in] e index of the edge
         * \param[in] lv local index of the vertex, in {0,1}
         * \param[in] v global index of the vertex
         */
        void set_vertex(index_t e, index_t lv, index_t v) {
            geo_debug_assert(e < nb());
            geo_debug_assert(lv < 2);
            edge_vertex_[2*e+lv] = v;
        }


        /**
         * \brief Gets a pointer to a vertex index by corner index
         * \param[in] c corner index (2 * edge index + 0 or 1)
         * \return a pointer to the index of the vertex. 
         * \note Normal uses do not call this function
         */
        index_t* vertex_index_ptr(index_t c) {
            geo_debug_assert(c < 2*nb());
            return &(edge_vertex_[c]);
        }

        /**
         * \brief Gets a pointer to a vertex index by corner index
         * \param[in] c corner index (2 * edge index + 0 or 1)
         * \return a pointer to the index of the vertex. 
         * \note Normal uses do not call this function
         */
        const index_t* vertex_index_ptr(index_t c) const {
            geo_debug_assert(c < 2*nb());
            return &(edge_vertex_[c]);
        }

        /**
         * \brief Creates a new edge
         * \return the index of the created edge
         */
        index_t create_edge() {
            return create_sub_element();
        }

        /**
         * \brief Creates a batch of edges
         * \param[in] nb number of edges to create
         * \return the index of the first created edge
         */
        index_t create_edges(index_t nb) {
            return create_sub_elements(nb);
        }
        
        /**
         * \brief Creates a new edge
         * \param[in] v1 , v2 global indices of the vertices of the edge
         * \return the index of the created edge
         */
        index_t create_edge(index_t v1, index_t v2) {
            index_t result = create_edge();
            set_vertex(result,0,v1);
            set_vertex(result,1,v2);
            return result;
        }

        virtual void delete_elements(
            vector<index_t>& to_delete, bool remove_isolated_vertices=true
        );
        
        virtual void permute_elements(vector<index_t>& permutation);

        virtual void clear(
            bool keep_attributes=true, bool keep_memory=false
        );

        virtual void pop();
        
    protected:
        virtual void clear_store(
            bool keep_attributes, bool keep_memory = false
        );
        
        virtual void resize_store(index_t new_size);

        index_t create_sub_element() {
            edge_vertex_.push_back(NO_VERTEX);
            edge_vertex_.push_back(NO_VERTEX);
            return MeshSubElementsStore::create_sub_element();
        }

        index_t create_sub_elements(index_t nb_in) {
            edge_vertex_.resize(2*(nb()+nb_in),NO_VERTEX);
            return MeshSubElementsStore::create_sub_elements(nb_in);
        }

        void copy(const MeshEdges& rhs, bool copy_attributes=true) {
            MeshSubElementsStore::copy(rhs, copy_attributes);
            edge_vertex_ = rhs.edge_vertex_;
        }
        
        vector<index_t> edge_vertex_;
        friend class Mesh;
        friend class GeogramIOHandler;
    };
    
    /**************************************************************************/
    
    /**
     * \brief Stores the facets of a mesh (low-level store)
     * \relates MeshFacets
     */
    class GEOGRAM_API MeshFacetsStore : public MeshSubElementsStore {
    public:
        MeshFacetsStore(Mesh& mesh);

        /**
         * \brief Gets the first element for iterating over
         *  the corners of a facet
         * \param[in] f the facet
         * \return the first corner of the facet
         */
        index_t corners_begin(index_t f) const {
            geo_debug_assert(f < nb());
            return (is_simplicial_ ? 3*f : facet_ptr_[f]);
        }

        /**
         * \brief Gets the upper limit for iterating over the
         *  corners of a facet
         * \param[in] f the facet
         * \return one position past the last corner of the facet
         */
        index_t corners_end(index_t f) const {
            geo_debug_assert(f < nb());
            return (is_simplicial_ ? 3*(f+1): facet_ptr_[f+1]);
        }

        /**
         * \brief Gets the number of corners in a facet
         * \param[in] f the facet
         * \return the number of corners in facet \p f
         */
        index_t nb_corners(index_t f) const {
            geo_debug_assert(f < nb());
            return (is_simplicial_ ? 3 : facet_ptr_[f+1] - facet_ptr_[f]);
        }

        /**
         * \brief Gets a corner by facet and local vertex index
         * \param[in] f the facet
         * \param[in] lv the local index of the vertex in facet \p f
         * \return the \p lv%th corner of facet \p f
         * \pre lv < nb_corners(f)
         */
        index_t corner(index_t f, index_t lv) const {
            geo_debug_assert(f < nb());
            geo_debug_assert(lv < nb_corners(f));
            return corners_begin(f)+lv;
        }

        /**
         * \brief Tests whether all the facets are triangles
         * \details When all the facets are triangles, storage
         *  and access is optimized.
         * \retval true if all the facets are triangles
         * \retval false otherwise
         */
        bool are_simplices() const {
            return is_simplicial_;
        }
	
        /**
         * \brief Gets a pointer to the first element for iterating over
         *  the corners of a facet
         * \param[in] f the facet
         * \return a pointer to the first corner of the facet
         */
	const index_t* corners_begin_ptr(index_t f) const {
	    geo_debug_assert(!is_simplicial_);
	    geo_debug_assert(f < nb());
	    return &facet_ptr_[f];
	}

    protected:
        virtual void clear_store(
            bool keep_attributes, bool keep_memory = false
        );
        
        virtual void resize_store(index_t new_size);

        index_t create_sub_element() {
            if(!is_simplicial_) {
                facet_ptr_.push_back(NO_CORNER);
            }
            return MeshSubElementsStore::create_sub_element();
        }

        index_t create_sub_elements(index_t nb) {
            if(!is_simplicial_) {            
                for(index_t i=0; i<nb; ++i) {
                    facet_ptr_.push_back(NO_CORNER);
                }
            }
            return MeshSubElementsStore::create_sub_elements(nb);
        }

        void copy(const MeshFacetsStore& rhs, bool copy_attributes=true) {
            MeshSubElementsStore::copy(rhs,copy_attributes);
            is_simplicial_ = rhs.is_simplicial_;
            facet_ptr_ = rhs.facet_ptr_;
        }
        
    protected:
        bool is_simplicial_;
        vector<index_t> facet_ptr_;
        friend class Mesh;
        friend class GeogramIOHandler;
    };

    /*************************************************************************/

    /**
     * \brief Stores the facet corners of a mesh (low-level store)
     * \relates MeshFacets
     */
    class GEOGRAM_API MeshFacetCornersStore : public MeshSubElementsStore {
    public:
        MeshFacetCornersStore(Mesh& mesh);

        /**
         * \brief Gets the vertex that a corner is incident to
         * \param[in] c the corner
         * \return the vertex that corner \p c is incident to
         */
        index_t vertex(index_t c) const {
            geo_assert(c < nb());
            return corner_vertex_[c];
        }

        /**
         * \brief Gets the facet that a corner is adjacent to
         * \param[in] c the corner
         * \return the facet that corner \p is adjacent to or
         *  NO_FACET if \p c is on the border
         */
        index_t adjacent_facet(index_t c) const {
            geo_assert(c < nb());
            return corner_adjacent_facet_[c];
        }

        /**
         * \brief Gets a pointer to the the facet index 
	 *  that a corner is adjacent to
         * \param[in] c the corner
         * \return a pointer to the the facet index 
	 *  that corner \p is adjacent to.
         */
        const index_t* adjacent_facet_ptr(index_t c) const {
            geo_assert(c < nb());
            return &corner_adjacent_facet_[c];
        }


        /**
         * \brief Gets a pointer to the the facet index 
	 *  that a corner is adjacent to
         * \param[in] c the corner
         * \return a pointer to the the facet index 
	 *  that corner \p is adjacent to.
         */
	index_t* adjacent_facet_ptr(index_t c) {
            geo_assert(c < nb());
            return &corner_adjacent_facet_[c];
        }
	
        /**
         * \brief Sets the vertex that a corner is incident to
         * \param[in] c the corner
         * \param[in] v the vertex that corner \p c is incident to
         * \pre v < mesh.vertices.nb()
         */
        void set_vertex(index_t c, index_t v) {
            geo_debug_assert(c < nb());
            geo_debug_assert(v < vertices_.nb());
            corner_vertex_[c] = v;
        }

        /**
         * \brief Sets the vertex that a corner is incident to
         * \details Does not check whether \p v is a valid vertex
         *  index. This function is useful for some algorithms that
         *  need to create/update the facets before creating the
         *  vertices. 
         * \param[in] c the corner
         * \param[in] v the vertex that corner \p c is incident to
         * \note Normal uses do not call this function
         */
        void set_vertex_no_check(index_t c, index_t v) {
            geo_debug_assert(c < nb());
            corner_vertex_[c] = v;
        }

        /**
         * \brief Sets the facet that a corner is adjacent to
         * \param[in] c the corner
         * \param[in] f the facet that corner \p is adjacent to or
         *  NO_FACET if \p c is on the border
         */
        void set_adjacent_facet(index_t c, index_t f) {
            geo_debug_assert(c < nb());
            geo_debug_assert(f == NO_FACET || f < facets_.nb());
            corner_adjacent_facet_[c] = f;
        }

        /**
         * \brief Gets a pointer to the vertex that a corner is incident to
         * \param[in] c the corner
         * \return a pointer to the index of the vertex that this corner
         *  is incident to
         * \note Normal uses do not call this function
         */
        index_t* vertex_index_ptr(index_t c) {
            geo_debug_assert(c < nb());
            return &(corner_vertex_[c]);
        }

        /**
         * \brief Gets a pointer to the vertex that a corner is incident to
         * \param[in] c the corner
         * \return a pointer to the index of the vertex that this corner
         *  is incident to
         * \note Normal uses do not call this function
         */
        const index_t* vertex_index_ptr(index_t c) const {
            geo_debug_assert(c < nb());
            return &(corner_vertex_[c]);
        }

    protected:
        virtual void clear_store(
            bool keep_attributes, bool keep_memory = false
        );
        virtual void resize_store(index_t new_size);

        index_t create_sub_element(index_t v, index_t f = NO_FACET) {
            corner_vertex_.push_back(v);
            corner_adjacent_facet_.push_back(f);
            return MeshSubElementsStore::create_sub_element();
        }

        index_t create_sub_elements(index_t nb) {
            for(index_t i=0; i<nb; ++i) {
                corner_vertex_.push_back(NO_VERTEX);
            }
            for(index_t i=0; i<nb; ++i) {
                corner_adjacent_facet_.push_back(NO_FACET);
            }
            return MeshSubElementsStore::create_sub_elements(nb);
        }

        void copy(
            const MeshFacetCornersStore& rhs, bool copy_attributes=true
        ) {
            MeshSubElementsStore::copy(rhs, copy_attributes);
            corner_vertex_ = rhs.corner_vertex_;
            corner_adjacent_facet_ = rhs.corner_adjacent_facet_;
        }
        
    protected:
        MeshVertices& vertices_;
        MeshFacetsStore& facets_;
        vector<index_t> corner_vertex_;
        vector<index_t> corner_adjacent_facet_;

        friend class MeshFacets;
        friend class Mesh;
        friend class GeogramIOHandler;        
    };

    /*************************************************************************/

    /**
     * \brief The facets of a mesh
     * \relates Mesh
     */
    class GEOGRAM_API MeshFacets : public MeshFacetsStore, public MeshElements {
    public:

        /**
         * \brief MeshFacets constructor
         * \param[in] mesh a reference to the mesh 
         *  this MeshFacets is attached to
         */
        MeshFacets(Mesh& mesh);

        /**
         * \brief Gets the number of vertices of a facet
         * \param[in] f the facet
         * \return the number of vertices of facet \p f
         */
        index_t nb_vertices(index_t f) const {
            return nb_corners(f);
        }

        /**
         * \brief Gets a vertex by facet and local vertex index
         * \param[in] f the facet
         * \param[in] lv the local vertex index in \p f
         * \return the \p lv%th vertex of facet \p f
         * \pre lv < nb_vertices(f)
         */
        index_t vertex(index_t f, index_t lv) const {
            return facet_corners_.vertex(corner(f,lv));
        }

        /**
         * \brief Sets a vertex by facet and local vertex index
         * \param[in] f the facet
         * \param[in] lv the local vertex index in \p f
         * \param[in] v specifies the \p lv%th vertex of facet \p f
         * \pre lv < nb_vertices(f)
         */
        void set_vertex(index_t f, index_t lv, index_t v) {
            facet_corners_.set_vertex(corner(f,lv),v);
        }

	/**
	 * \brief Gets the local index of a vertex in a facet.
	 * \param[in] f a facet
	 * \param[in] v a vertex
	 * \return lv such that vertex(f,lv) == v or NO_VERTE if f is
	 *  not incident to v
	 */
	index_t find_vertex(index_t f, index_t v) const {
	    for(index_t lv=0; lv<nb_vertices(f); ++lv) {
		if(vertex(f,lv) == v) {
		    return lv;
		}
	    }
	    return NO_VERTEX;
	}
	
        /**
         * \brief Gets an adjacent facet by facet and local edge index
         * \param[in] f the facet
         * \param[in] le the local index of an edge in facet \p f
         * \return the facet incident to \p f along edge \p le or 
         *  NO_FACET if \p le is on the border
         */
        index_t adjacent(index_t f, index_t le) const {
            return facet_corners_.adjacent_facet(corner(f,le));
        }

	/**
	 * \brief Gets the local index of a facet adjacent to another one.
	 * \param[in] f a facet
	 * \param[in] f2 another facet
	 * \return le such that adjacent(f,le) == f2 or NO_FACET if f and f2
	 *  are not adjacent.
	 */
	index_t find_adjacent(index_t f, index_t f2) const {
	    for(index_t le=0; le<nb_vertices(f); ++le) {
		if(adjacent(f,le) == f2) {
		    return le;
		}
	    }
	    return NO_FACET;
	}
	
        /**
         * \brief Sets an adjacent facet by facet and local edge index
         * \param[in] f the facet
         * \param[in] le the local index of an edge in facet \p f
         * \param[in] f2 specifies the facet incident to \p f along edge 
         *  \p le or NO_FACET if \p le is on the border
         */
        void set_adjacent(index_t f, index_t le, index_t f2) {
            facet_corners_.set_adjacent_facet(corner(f,le),f2);
        }

        /**
         * \brief Gets the successor of a corner around a facet
         * \param[in] f the facet
         * \param[in] c the corner
         * \return the successor of corner \p c around facet \p f
         * \pre c >= corners_begin(f) && c < corners_end(f)
         */
        index_t next_corner_around_facet(index_t f, index_t c) const {
            geo_debug_assert(f < nb());
            geo_debug_assert(c >= corners_begin(f) && c < corners_end(f));
            return c + 1 == corners_end(f) ? corners_begin(f) : c + 1;
        }

        /**
         * \brief Gets the predecessor of a corner around a facet
         * \param[in] f the facet
         * \param[in] c the corner
         * \return the predecessor of corner \p c around facet \p f
         * \pre c >= corners_begin(f) && c < corners_end(f)
         */
        index_t prev_corner_around_facet(index_t f, index_t c) const {
            geo_debug_assert(f < nb());
            geo_debug_assert(c >= corners_begin(f) && c < corners_end(f));
            return c == corners_begin(f) ? corners_end(f) - 1 : c - 1;
        }
        
        virtual void delete_elements(
            vector<index_t>& to_delete,
            bool remove_isolated_vertices=true
        );
        
        virtual void permute_elements(vector<index_t>& permutation);

        virtual void clear(
            bool keep_attributes=true, bool keep_memory=false
        ) ;

        /**
         * \brief Creates a contiguous chunk of facets
         * \param[in] nb_facets number of facets to create
         * \param[in] nb_vertices_per_polygon number of vertices
         *   in each facet
         * \return the index of the first facet
         */
        index_t create_facets(
            index_t nb_facets, index_t nb_vertices_per_polygon
        ) {
            if(nb_vertices_per_polygon != 3) {
                is_not_simplicial();
            }
            
            index_t first_facet = nb();
            index_t co = facet_corners_.nb();
            facet_corners_.create_sub_elements(
                nb_facets*nb_vertices_per_polygon
            );
            index_t result = create_sub_elements(nb_facets);

            if(!is_simplicial_) {
                for(index_t f=first_facet; f<=first_facet+nb_facets; ++f) {
                    facet_ptr_[f] = co;
                    co += nb_vertices_per_polygon;
                }
                geo_debug_assert(facet_ptr_.size() == nb()+1);
                geo_debug_assert(facet_ptr_[nb()] == facet_corners_.nb());
            }
            return result;
        }

        /**
         * \brief Creates a contiguous chunk of triangles
         * \param[in] nb_triangles number of triangles to create
         * \return the index of the first triangle
         */
        index_t create_triangles(index_t nb_triangles) {
            return create_facets(nb_triangles, 3);
        }

        /**
         * \brief Creates a contiguous chunk of quads
         * \param[in] nb_quads number of quads to create
         * \return the index of the first quad
         */
         index_t create_quads(index_t nb_quads) {
            return create_facets(nb_quads, 4);
        }

        /**
         * \brief Creates a triangle
         * \param[in] v1 , v2 , v3 the vertices of the triangle
         * \return the index of the created triangle
         */
        index_t create_triangle(index_t v1, index_t v2, index_t v3) {
            facet_corners_.create_sub_element(v1);
            facet_corners_.create_sub_element(v2);
            facet_corners_.create_sub_element(v3);
            index_t result = create_sub_element();
            if(!is_simplicial_) {
                facet_ptr_[result+1] = facet_corners_.nb();
                geo_debug_assert(facet_ptr_.size() == nb()+1);
                geo_debug_assert(facet_ptr_[nb()] == facet_corners_.nb());
            }
            return result;
        }

        /**
         * \brief Creates a quad
         * \param[in] v1 , v2 , v3 , v4 the vertices of the quad
         * \return the index of the created quad
         */
        index_t create_quad(index_t v1, index_t v2, index_t v3, index_t v4) {
            is_not_simplicial();
            facet_corners_.create_sub_element(v1);
            facet_corners_.create_sub_element(v2);
            facet_corners_.create_sub_element(v3);
            facet_corners_.create_sub_element(v4);            
            index_t result = create_sub_element();
            facet_ptr_[result+1] = facet_corners_.nb();
            geo_debug_assert(facet_ptr_.size() == nb()+1);
            geo_debug_assert(facet_ptr_[nb()] == facet_corners_.nb());
            return result;
        }

        /**
         * \brief Creates a polygonal facet
         * \param[in] nb_vertices number of vertices of the facet
         * \return the index of the created facet
         */
        index_t create_polygon(index_t nb_vertices) {
            if(nb_vertices != 3) {
                is_not_simplicial();
            }
            for(index_t i=0; i<nb_vertices; ++i) {
                facet_corners_.create_sub_element(NO_VERTEX);
            }
            index_t result = create_sub_element();
            if(!is_simplicial_) {
                facet_ptr_[result+1] = facet_corners_.nb();
                geo_debug_assert(facet_ptr_.size() == nb()+1);
                geo_debug_assert(facet_ptr_[nb()] == facet_corners_.nb());
            }
            return result;
        }

        /**
         * \brief Creates a polygonal facet
         * \param[in] nb_vertices number of vertices of the facet
         * \param[in] vertices a const pointer to the \p nb_vertices vertices 
         * \return the index of the created facet
         */
        index_t create_polygon(index_t nb_vertices, const index_t* vertices) {
            if(nb_vertices != 3) {
                is_not_simplicial();
            }
            for(index_t i=0; i<nb_vertices; ++i) {
                facet_corners_.create_sub_element(vertices[i]);
            }
            index_t result = create_sub_element();
            if(!is_simplicial_) {
                facet_ptr_[result+1] = facet_corners_.nb();
                geo_debug_assert(facet_ptr_.size() == nb()+1);
                geo_debug_assert(facet_ptr_[nb()] == facet_corners_.nb());
            }
            return result;
        }

        /**
         * \brief Creates a polygonal facet
         * \param[in] vertices a const reference to a vector that
         *  contains the vertices
         * \return the index of the created facet
         */
        index_t create_polygon(const vector<index_t>& vertices) {
            return create_polygon(vertices.size(), vertices.data());
        }

        /**
         * \brief Connects the facets
         */
        void connect();

        /**
         * \brief Triangulates the facets
         * \note Attributes are zeroed
         */
        void triangulate();

        /**
         * \brief Flips a facet
         * \details The order of the corners is reversed
         * \param[in] f the facet to be flipped
         */
        void flip(index_t f);

        /**
         * \brief Replaces the edges of this mesh
         *   with the borders of the surfacic part.
         */
        void compute_borders();

        /**
         * \brief Copies a triangle mesh into this Mesh.
         * \details Facet adjacence are not computed.
         *   Facet and corner attributes are zeroed.
         * \param[in] dim dimension of the vertices
         * \param[in] vertices coordinates of the vertices
         * \param[in] triangles facet to vertex links
         * \param[in] steal_args if set, vertices and triangles
         * are 'stolen' from the arguments
         * (using vector::swap).
         */
        void assign_triangle_mesh(
            coord_index_t dim,
            vector<double>& vertices,
            vector<index_t>& triangles,
            bool steal_args
        );

        /*
         * \brief Copies a triangle mesh into this Mesh.
         * \details Facet adjacence are not computed.
         *   Facet and corner attributes are zeroed.
         * \param[in] triangles facet to vertex links
         * \param[in] steal_args if set, vertices and triangles
         * are 'stolen' from the arguments
         * (using vector::swap).
         */
        void assign_triangle_mesh(
            vector<index_t>& triangles,
            bool steal_args
        );

        virtual void pop();


	/**
	 * \brief Gets the corners of a facet.
	 * \param[in] f the index of the facet.
	 * \return a range with all the corners of the facet.
	 */
	range<no_iterator> corners(index_t f) const {
	    geo_debug_assert(f < nb());
	    return range<no_iterator>(
		no_iterator(corners_begin(f)),
		no_iterator(corners_end(f))
	    );
	}
	
    protected:

        /**
         * \brief Indicates that the stored elements are only triangles.
         */
        void is_simplicial() {
	    if(!is_simplicial_) {
		is_simplicial_ = true;
		facet_ptr_.resize(1);
		facet_ptr_[0] = 0;
	    }
	}
	
        /**
         * \brief Indicates that the stored elements are no
         *  longer only triangles.
         * \details Creates the facet pointers for the pre-existing
         *  triangles if any.
         */
        void is_not_simplicial() {
            if(is_simplicial_) {
                is_simplicial_ = false;
                facet_ptr_.resize(nb()+1);
                for(index_t f=0; f<facet_ptr_.size(); ++f) {
                    facet_ptr_[f] = 3*f;
                }
            }
        }

    protected:
        MeshVertices& vertices_;        
        MeshFacetCornersStore& facet_corners_;
        friend class Mesh;
        friend class GeogramIOHandler;
	friend void GEOGRAM_API tessellate_facets(
	    Mesh& M, index_t max_nb_vertices
	);
    };
    
    /*************************************************************************/

    enum MeshCellType {
        MESH_TET = 0,
        MESH_HEX = 1,
        MESH_PRISM = 2,
        MESH_PYRAMID = 3,
        MESH_CONNECTOR = 4,
        MESH_NB_CELL_TYPES = 5
    };
    
    /**
     * \brief Lookup tables that describe the combinatorics
     *  of each cell type.
     * \relates MeshCells
     */
    struct CellDescriptor {
        /** Number of vertices */
        index_t nb_vertices;
        
        /** Number of facets */
        index_t nb_facets;
        
        /** Number of vertices in each facet */
        index_t nb_vertices_in_facet[6];
        
        /** 
         * Cell vertex index by (facet index,facet vertex index).
         */
        index_t facet_vertex[6][4];

        /**
         * Number of edges */
        index_t nb_edges;
        
        /**
         * Cell vertex index by (edge index, edge vertex index).
         */
        index_t edge_vertex[12][2];

        /**
         * Cell facet index by (edge index, adjacent facet index).
         */
        index_t edge_adjacent_facet[12][2];
    };

    
    /**
     * \brief Gathers declarations of global cell descriptors.
     * \details Cannot be declared as static variables in 
     *  MeshCellsStore, since visual C++ does not allows
     *  exporting class static variables from a DLL.
     */
    namespace MeshCellDescriptors {
        /**
         * \brief Maps a cell type to the associated cell descriptor.
         */
        GEOGRAM_API extern CellDescriptor*
             cell_type_to_cell_descriptor[GEO::MESH_NB_CELL_TYPES];

        GEOGRAM_API extern CellDescriptor tet_descriptor;
        GEOGRAM_API extern CellDescriptor hex_descriptor;
        GEOGRAM_API extern CellDescriptor prism_descriptor;
        GEOGRAM_API extern CellDescriptor pyramid_descriptor;
        GEOGRAM_API extern CellDescriptor connector_descriptor;
    }
    
    /**
     * \brief Stores the cells of a mesh (low-level store)
     * \relates MeshCells
     */
    class GEOGRAM_API MeshCellsStore : public MeshSubElementsStore {
    public:
        MeshCellsStore(Mesh& mesh);

        /**
         * \brief Tests whether all the cells are tetrahedra
         * \details Storage and access are optimized when all the
         *  cells are tetrahedra
         * \retval true if all the cells are tetrahedra
         * \retval false otherwise
         */
        bool are_simplices() const {
            return is_simplicial_;
        }

        /**
         * \brief Gets the type of a cell
         * \param[in] c the cell, in 0..nb()-1
         * \return one of 
         *   MESH_TET, MESH_HEX, MESH_PRISM, MESH_PYRAMID, MESH_CONNECTOR.
         */
        MeshCellType type(index_t c) const {
            geo_debug_assert(c < nb());
            return is_simplicial_ ? MESH_TET : MeshCellType(cell_type_[c]);
        }

        /**
         * \brief Gets the descriptor of a cell
         * \details The descriptor of a cell is a set of static
         *  arrays that facilitate some accesses (most client
         *  code do not need to use this function)
         * \param[in] c a cell, in 0..nb()-1
         * \return the descriptor of cell \p c
         */
        const CellDescriptor& descriptor(index_t c) const {
            geo_debug_assert(c < nb());            
            return is_simplicial_ ? MeshCellDescriptors::tet_descriptor :
                *(
                    MeshCellDescriptors::cell_type_to_cell_descriptor[
                        cell_type_[c]
                    ]
                );
        }

        /**
         * \brief Gets a descriptor by cell type
         * \details The descriptor of a cell is a set of static
         *  arrays that facilitate some accesses (most client
         *  code do not need to use this function)
         * \param[in] t one of 
         *   MESH_TET, MESH_HEX, MESH_PRISM, MESH_PYRAMID, MESH_CONNECTOR
         * \return the descriptor of cell \p c
         */
        static const CellDescriptor& cell_type_to_cell_descriptor(
            MeshCellType t
        ) {
            geo_debug_assert(t < GEO::MESH_NB_CELL_TYPES);
            return *(MeshCellDescriptors::cell_type_to_cell_descriptor[t]);
        }

        /**
         * \brief Gets the number of corners of a cell
         * \param[in] c a cell, in 0..nb()-1
         * \return the number of corners of cell \p c
         */
        index_t nb_corners(index_t c) const {
            geo_debug_assert(c < nb());
            return descriptor(c).nb_vertices;
        }

        /**
         * \brief Gets the first element for iterating over
         *  the corners of a cell
         * \param[in] c the cell, in 0..nb()-1
         * \return the first corner of the cell
         */
        index_t corners_begin(index_t c) const {
            geo_debug_assert(c < nb());            
            return is_simplicial_ ? 4*c : cell_ptr_[c];
        }

        /**
         * \brief Gets the upper limit for iterating over the
         *  corners of a cell
         * \param[in] c the cell, in 0..nb()-1
         * \return one position past the last corner of the cell
         */
        index_t corners_end(index_t c) const {
            geo_debug_assert(c < nb());            
            return is_simplicial_ ? 4*(c+1) : cell_ptr_[c] + nb_corners(c);
        }

        /**
         * \brief Gets a corner of a cell by local vertex index
         * \param[in] c the cell, in 0..nb()-1
         * \param[in] lv the local vertex index, in 0..nb_corners(c)-1
         * \return the corner incident to vertex \p lv in cell \p c
         */
        index_t corner(index_t c, index_t lv) const {
            geo_debug_assert(c < nb());
            // There seems to be a linkage problem under MSVC for the
            // following assertion check...
#ifndef GEO_OS_WINDOWS            
            geo_debug_assert(lv < nb_corners(c));
#endif            
            return corners_begin(c) + lv;
        }

        /**
         * \brief Gets the number of facets of a cell
         * \param[in] c the cell, in 0..nb()-1
         * \return the number of facets of cell \p c
         */
        index_t nb_facets(index_t c) const {
            geo_debug_assert(c < nb());
            return descriptor(c).nb_facets;
        }

        /**
         * \brief Gets the first element for iterating over
         *  the facets of a cell
         * \param[in] c the cell, in 0..nb()-1
         * \return the first facet of the cell
         */
        index_t facets_begin(index_t c) const {
            geo_debug_assert(c < nb());            
            return is_simplicial_ ? 4*c : cell_ptr_[c];
        }

        /**
         * \brief Gets the upper limit for iterating over the
         *  facets of a cell
         * \param[in] c the cell, in 0..nb()-1
         * \return one position past the last facet of the facet
         */
        index_t facets_end(index_t c) const {
            geo_debug_assert(c < nb());            
            return is_simplicial_ ? 4*(c+1) : cell_ptr_[c] + nb_facets(c);
        }

        /**
         * \brief Gets a facet of a cell by local facet index
         * \param[in] c the cell, in 0..nb()-1
         * \param[in] lf the local facet index, in 0..nb_facets(c)-1
         * \return the facet \p lf in cell \p c
         */
        index_t facet(index_t c, index_t lf) const {
            geo_debug_assert(c < nb());
            geo_debug_assert(lf < nb_facets(c));
            return facets_begin(c) + lf;
        }

        /**
         * \brief Gets the number of edges in a cell
         * \param[in] c the cell, in 0..nb()-1
         * \return the number of edges in cell \p c
         */
        index_t nb_edges(index_t c) const {
            return descriptor(c).nb_edges;
        }
        
    protected:
        virtual void clear_store(
            bool keep_attributes, bool keep_memory = false
        );
        
        virtual void resize_store(index_t new_size);

        index_t create_sub_element(MeshCellType type) {
            if(!is_simplicial_) {
                cell_ptr_.push_back(NO_CORNER);
                cell_type_.push_back(Numeric::uint8(type));
            }
            return MeshSubElementsStore::create_sub_element();
        }

        index_t create_sub_elements(index_t nb, MeshCellType type) {
            if(!is_simplicial_) {
                for(index_t i=0; i<nb; ++i) {
                    cell_ptr_.push_back(NO_CORNER);
                    cell_type_.push_back(Numeric::uint8(type));                
                }
            }
            return MeshSubElementsStore::create_sub_elements(nb);
        }

        void copy(
            const MeshCellsStore& rhs, bool copy_attributes=true
        ) {
            MeshSubElementsStore::copy(rhs, copy_attributes);
            is_simplicial_ = rhs.is_simplicial_;
            cell_type_ = rhs.cell_type_;
            cell_ptr_ = rhs.cell_ptr_;
        }
        
    protected:
        bool is_simplicial_;
        vector<Numeric::uint8> cell_type_;
        vector<index_t> cell_ptr_;

    protected:
        friend class Mesh;
        friend class GeogramIOHandler;                
    };
    
    /*************************************************************************/

    /**
     * \brief Stores the cell corners of a mesh (low-level store)
     * \relates MeshCells
     */
    class GEOGRAM_API MeshCellCornersStore : public MeshSubElementsStore {
    public:
        MeshCellCornersStore(Mesh& mesh);

        /**
         * \brief Gets the vertex that a corner is incident to
         * \param[in] c the corner, in 0..nb()-1
         * \return the vertex that corner \p c is incident to
         */
        index_t vertex(index_t c) const {
            geo_assert(c < nb());
            return corner_vertex_[c];
        }

        /**
         * \brief Sets the vertex that a corner is incident to
         * \param[in] c the corner, in 0..nb()-1
         * \param[in] v specifies the vertex that corner \p c is incident to
         */
        void set_vertex(index_t c, index_t v) {
            geo_debug_assert(c < nb());
            geo_debug_assert(v < vertices_.nb());
            corner_vertex_[c] = v;
        }

        /**
         * \brief Gets a pointer to the vertex that a corner is incident to
         * \param[in] c the corner
         * \return a pointer to the index of the vertex that this corner
         *  is incident to
         * \note Normal uses do not call this function
         */
        index_t* vertex_index_ptr(index_t c) {
            geo_debug_assert(c < nb());
            return &(corner_vertex_[c]);
        }

        /**
         * \brief Gets a pointer to the vertex that a corner is incident to
         * \param[in] c the corner
         * \return a const pointer to the index of the vertex that this corner
         *  is incident to
         * \note Normal uses do not call this function
         */
        const index_t* vertex_index_ptr(index_t c) const {
            geo_debug_assert(c < nb());
            return &(corner_vertex_[c]);
        }
        
    protected:
        virtual void clear_store(
            bool keep_attributes, bool keep_memory = false
        );
        
        virtual void resize_store(index_t new_size);

        index_t create_sub_element(index_t v) {
            corner_vertex_.push_back(v);
            return MeshSubElementsStore::create_sub_element();
        }

        index_t create_sub_elements(index_t nb) {
            for(index_t i=0; i<nb; ++i) {
                corner_vertex_.push_back(NO_VERTEX);
            }
            return MeshSubElementsStore::create_sub_elements(nb);
        }

        void copy(
            const MeshCellCornersStore& rhs, bool copy_attributes=true
        ) {
            MeshSubElementsStore::copy(rhs, copy_attributes);
            corner_vertex_ = rhs.corner_vertex_;
        }
        
    protected:
        MeshVertices& vertices_;
        vector<index_t> corner_vertex_;

        friend class MeshCells;
        friend class Mesh;
        friend class GeogramIOHandler;                
    };

    /*************************************************************************/

    /**
     * \brief Stores the cell facets of a mesh (low-level store)
     * \relates MeshCells
     */
    class GEOGRAM_API MeshCellFacetsStore : public MeshSubElementsStore {
    public:
        /**
         * \brief MeshCellFacetsStore constructor
         * \param[in] mesh the mesh that this MeshCellFacetsStore is attached to
         */
        MeshCellFacetsStore(Mesh& mesh);

        /**
         * \brief Gets a cell adjacent to a facet
         * \param[in] f the facet, in 0..nb()-1
         * \return the cell adjacent to facet \p f, or NO_FACET if \p f
         *  is on the border
         */
        index_t adjacent_cell(index_t f) const {
            geo_assert(f < nb());
            return adjacent_cell_[f];
        }

        /**
         * \brief Sets a cell adjacent to a facet
         * \param[in] f the facet, in 0..nb()-1
         * \param[in] c specifies the cell adjacent 
         *  to facet \p f, or is set to NO_FACET 
         *  if \p f is on the border
         */
        void set_adjacent_cell(index_t f, index_t c) {
            geo_debug_assert(f < nb());            
            geo_debug_assert(c == NO_CELL || c < cells_.nb());
            adjacent_cell_[f] = c;
        }

        /**
         * \brief Gets a const pointer to a cell adjacent to a facet
         * \param[in] f the facet, in 0..nb()-1
         * \return a const pointer to the cell adjacent to facet \p f, 
	 *  or NO_FACET if \p f is on the border
         */
        const index_t* adjacent_cell_ptr(index_t f) const {
            geo_assert(f < nb());
            return &adjacent_cell_[f];
        }
	
        /**
         * \brief Gets a pointer to a cell adjacent to a facet
         * \param[in] f the facet, in 0..nb()-1
         * \return a pointer to the cell adjacent to facet \p f, 
	 *  or NO_FACET if \p f is on the border
         */
        index_t* adjacent_cell_ptr(index_t f) {
            geo_assert(f < nb());
            return &adjacent_cell_[f];
        }
        
    protected:
        virtual void clear_store(
            bool keep_attributes, bool keep_memory = false
        );
        
        virtual void resize_store(index_t new_size);

        index_t create_sub_element(index_t c = NO_CELL) {
            adjacent_cell_.push_back(c);
            return MeshSubElementsStore::create_sub_element();
        }

        index_t create_sub_elements(index_t nb) {
            for(index_t i=0; i<nb; ++i) {
                adjacent_cell_.push_back(NO_CELL);
            }
            return MeshSubElementsStore::create_sub_elements(nb);
        }

        void copy(
            const MeshCellFacetsStore& rhs, bool copy_attributes=true
        ) {
            MeshSubElementsStore::copy(rhs, copy_attributes);
            adjacent_cell_ = rhs.adjacent_cell_;
        }
        
    protected:
        MeshVertices& vertices_;
        MeshCellsStore& cells_;
        vector<index_t> adjacent_cell_;

        friend class MeshCells;
        friend class Mesh;
        friend class GeogramIOHandler;                
    };

    /*************************************************************************/

    /**
     * \brief The cells of a mesh.
     * \relates Mesh
     */
    class GEOGRAM_API MeshCells : public MeshCellsStore, public MeshElements {
    public:
        /**
         * \brief MeshCells constructor
         * \param[in] mesh the mesh this MeshCells is attached to
         */
        MeshCells(Mesh& mesh);

        /**
         * \brief Gets the number of vertices of a cell
         * \param[in] c a cell, in 0..nb()-1
         * \return the number of vertices of cell \p c
         */
        index_t nb_vertices(index_t c) const {
            return nb_corners(c);
        }

        /**
         * \brief Gets a vertex of a cell by local vertex index
         * \param[in] c the cell, in 0..nb()-1
         * \param[in] lv local vertex index, in 0..nb_vertices(c)-1
         * \return the vertex \p lv of cell \p c
         */
        index_t vertex(index_t c, index_t lv) const {
            return cell_corners_.vertex(corner(c,lv));
        }

        /**
         * \brief Sets a vertex of a cell by local vertex index
         * \param[in] c the cell, in 0..nb()-1
         * \param[in] lv local vertex index, in 0..nb_vertices(c)-1
         * \param[in] v specifies the vertex \p lv of cell \p c
         */
        void set_vertex(index_t c, index_t lv, index_t v) {
            cell_corners_.set_vertex(corner(c,lv),v);
        }

        /**
         * \brief Gets a cell adjacent to another one by local facet index
         * \param[in] c the cell, in 0..nb()-1
         * \param[in] lf local facet index, in 0..nb_facets(c)-1
         * \return the cell adjacent to \p c along facet \p lf or NO_CELL
         *  if no such cell exists
         */
        index_t adjacent(index_t c, index_t lf) const {
            return cell_facets_.adjacent_cell(facet(c,lf));
        }

        /**
         * \brief Sets a cell adjacent to another one by local facet index
         * \param[in] c the cell, in 0..nb()-1
         * \param[in] lf local facet index, in 0..nb_facets(c)-1
         * \param[in] c2 specifies the cell adjacent to \p c along 
         *  facet \p lf or NO_CELL if no such cell exists
         */
        void set_adjacent(index_t c, index_t lf, index_t c2) {
            cell_facets_.set_adjacent_cell(facet(c,lf),c2);
        }

        /**
         * \brief Gets the number of vertices in a cell facet
         * \param[in] c the cell, in 0..nb()-1
         * \param[in] lf the local facet index, in 0..nb_facets(c)-1
         * \return the number of vertices of facet \p lf in cell \p c
         */
        index_t facet_nb_vertices(index_t c, index_t lf) const {
            geo_debug_assert(lf < nb_facets(c));
            return descriptor(c).nb_vertices_in_facet[lf];
        }

        /**
         * \brief Gets a vertex of a cell by local facet index and
         *  local vertex index in the facet
         * \param[in] c the cell, in 0..nb()-1
         * \param[in] lf the local facet index, in 0..nb_facets(c)-1
         * \param[in] lv the local vertex index, in 0..facet_nb_vertices(c,lf)-1
         * \return vertex \p lv of facet \p lf in cell \p c
         */
        index_t facet_vertex(index_t c, index_t lf, index_t lv) const {
            geo_debug_assert(lv < facet_nb_vertices(c, lf));
            return cell_corners_.vertex(
                    corner(c, descriptor(c).facet_vertex[lf][lv])
                    );
        }
        /**
         * \brief Gets a corner of a cell by local facet index and
         *  local corner index in the facet
         * \param[in] c the cell, in 0..nb()-1
         * \param[in] lf the local facet index, in 0..nb_facets(c)-1
         * \param[in] lc the local corner index, in 0..facet_nb_vertices(c,lf)-1
         * \return corner \p lc of facet \p lf in cell \p c
         */
        index_t facet_corner(index_t c, index_t lf, index_t lc) const {
            geo_debug_assert(lc < facet_nb_vertices(c, lf));
            return corner(c, descriptor(c).facet_vertex[lf][lc]);
        }

        /**
         * \brief Gets a cell vertex by local edge index and local
         *  vertex index in the edge
         * \param[in] c the cell, in 0..nb()-1
         * \param[in] le the local edge index, in 0..nb_edges(c)-1
         * \param[in] lv the local index in the edge, one of 0,1
         * \return vertex \p lv of edge \p le in cell \p c
         */
        index_t edge_vertex(index_t c, index_t le, index_t lv) const {
            geo_debug_assert(le < nb_edges(c));
            geo_debug_assert(lv < 2);
            return cell_corners_.vertex(
                corner(c,descriptor(c).edge_vertex[le][lv])
            );
        }

        /**
         * \brief Gets a cell local facet index by local edge index and local
         *  facet index in the edge
         * \param[in] c the cell, in 0..nb()-1
         * \param[in] le the local edge index, in 0..nb_edges(c)-1
         * \param[in] lf the local index in the edge, one of 0,1
         * \return the local facet index of the facet adjacent to the edge.
         *  - If \p lf=0, it gets the facet on the left of the oriented edge.
         *  - If \p lf=1, it gets the facet on the right of the oriented edge.
         */
        index_t edge_adjacent_facet(index_t c, index_t le, index_t lf) const {
            geo_debug_assert(le < nb_edges(c));
            geo_debug_assert(lf < 2);
            return descriptor(c).edge_adjacent_facet[le][lf];
        }

	/**
	 * \brief Gets the corners of a cell.
	 * \param[in] c the index of the cell.
	 * \return a range with all the corners of the facet.
	 */
	range<no_iterator> corners(index_t c) const {
	    geo_debug_assert(c < nb());
	    return range<no_iterator>(
		no_iterator(corners_begin(c)),
		no_iterator(corners_end(c))
	    );
	}
        
        virtual void clear(
            bool keep_attributes=true, bool keep_memory=false
        ) ;
        
        virtual void delete_elements(
            vector<index_t>& to_delete,
            bool remove_isolated_vertices=true
        );
        
        virtual void permute_elements(vector<index_t>& permutation);

        /**
         * \brief Creates a contiguous chunk of cells of the
         *  same type
         * \param[in] nb_cells number of cells to create
         * \param[in] type type of the cells to create, one of
         *   MESH_TET, MESH_HEX, MESH_PRISM, MESH_PYRAMID, MESH_CONNECTOR.
         * \return the first created cell
         */
        index_t create_cells(index_t nb_cells, MeshCellType type) {

            if(nb_cells == 0) {
                return NO_CELL;
            }
           
           
            if(type != MESH_TET) {
                is_not_simplicial();
            }
            
            const CellDescriptor& desc = cell_type_to_cell_descriptor(type);

            //   Note: there is padding, the same number of corners and
            // faces is created for each cell, so that a single cell
            // pointer is used for both.
            
            index_t cell_size = std::max(desc.nb_vertices, desc.nb_facets);
            index_t first_cell = nb();
            index_t co = cell_corners_.nb();
            
            cell_corners_.create_sub_elements(
                nb_cells*cell_size
            );

            cell_facets_.create_sub_elements(
                nb_cells*cell_size
            );
            
            index_t result = create_sub_elements(nb_cells, type);

            if(!is_simplicial_) {
                for(index_t c=first_cell; c<=first_cell+nb_cells; ++c) {
                    cell_ptr_[c] = co;
                    co += cell_size;
                }
            
                geo_debug_assert(cell_ptr_.size() == nb()+1);
                geo_debug_assert(cell_ptr_[nb()] == cell_corners_.nb());
                geo_debug_assert(cell_ptr_[nb()] == cell_facets_.nb());
            }
            
            return result;
        }

        /**
         * \brief Creates a contiguous chunk of tetrahedra
         * \param[in] nb_tets number of tetrahedra to create
         * \return the first created tetrahedron
         */
        index_t create_tets(index_t nb_tets) {
            return create_cells(nb_tets, MESH_TET);
        }

        /**
         * \brief Creates a contiguous chunk of hexahedra
         * \param[in] nb_hexes number of hexahedra to create
         * \return the first created hexahedron
         */
        index_t create_hexes(index_t nb_hexes) {
            return create_cells(nb_hexes, MESH_HEX);
        }

        /**
         * \brief Creates a contiguous chunk of prisms
         * \param[in] nb_prisms number of prisms to create
         * \return the first created prism
         */
        index_t create_prisms(index_t nb_prisms) {
            return create_cells(nb_prisms, MESH_PRISM);
        }

        /**
         * \brief Creates a contiguous chunk of pyramids
         * \param[in] nb_pyramids number of pyramids to create
         * \return the first created pyramid
         */
        index_t create_pyramids(index_t nb_pyramids) {
            return create_cells(nb_pyramids, MESH_PYRAMID);
        }

        /**
         * \brief Creates a tetrahedron
         * \param[in] v1 , v2 , v3 , v4 the vertices of the tetrahedron,
         *  all in 0 .. mesh.vertices.nb()-1
         * \param[in] adj1 , adj2 , adj3 , adj4 
         *  adjacent cells, or NO_CELL if unspecified / on border
         * \return the created tetrahedron
         */
        index_t create_tet(
            index_t v1, index_t v2, index_t v3, index_t v4,
            index_t adj1 = NO_CELL,
            index_t adj2 = NO_CELL,
            index_t adj3 = NO_CELL,
            index_t adj4 = NO_CELL
        ) {
            cell_corners_.create_sub_element(v1);
            cell_corners_.create_sub_element(v2);
            cell_corners_.create_sub_element(v3);
            cell_corners_.create_sub_element(v4);
            cell_facets_.create_sub_element(adj1);
            cell_facets_.create_sub_element(adj2);
            cell_facets_.create_sub_element(adj3);
            cell_facets_.create_sub_element(adj4);
            index_t result = create_sub_element(MESH_TET);
            if(!is_simplicial_) {
                cell_ptr_[nb()] = cell_corners_.nb();
            }
            geo_debug_assert(cell_facets_.nb() == cell_corners_.nb());
            return result;
        }

        /**
         * \brief Creates an hexahedron
         * \param[in] v1 , v2 , v3 , v4 , v5 , v6 , v7 , v8 
         *  the vertices of the hexahedron,
         *  all in 0 .. mesh.vertices.nb()-1
         * \param[in] adj1 , adj2 , adj3 , adj4 , adj5 , adj6 
         *  adjacent cells, or NO_CELL if unspecified / on border
         * \return the created hexahedron
         */
        index_t create_hex(
            index_t v1, index_t v2, index_t v3, index_t v4,
            index_t v5, index_t v6, index_t v7, index_t v8,            
            index_t adj1 = NO_CELL,
            index_t adj2 = NO_CELL,
            index_t adj3 = NO_CELL,
            index_t adj4 = NO_CELL,
            index_t adj5 = NO_CELL,
            index_t adj6 = NO_CELL
        ) {
            is_not_simplicial();
            cell_corners_.create_sub_element(v1);
            cell_corners_.create_sub_element(v2);
            cell_corners_.create_sub_element(v3);
            cell_corners_.create_sub_element(v4);
            cell_corners_.create_sub_element(v5);
            cell_corners_.create_sub_element(v6);
            cell_corners_.create_sub_element(v7);
            cell_corners_.create_sub_element(v8);
            cell_facets_.create_sub_element(adj1);
            cell_facets_.create_sub_element(adj2);
            cell_facets_.create_sub_element(adj3);
            cell_facets_.create_sub_element(adj4);
            cell_facets_.create_sub_element(adj5);
            cell_facets_.create_sub_element(adj6);
            cell_facets_.create_sub_element(NO_CELL); // padding
            cell_facets_.create_sub_element(NO_CELL); // padding           
            index_t result = create_sub_element(MESH_HEX);
            cell_ptr_[nb()] = cell_corners_.nb();
            geo_debug_assert(cell_facets_.nb() == cell_corners_.nb());
            return result;
        }

        /**
         * \brief Creates a prism
         * \param[in] v1 , v2 , v3 , v4 , v5 , v6
         *  the vertices of the prism
         *  all in 0 .. mesh.vertices.nb()-1
         * \param[in] adj1 , adj2 , adj3 , adj4 , adj5
         *  adjacent cells, or NO_CELL if unspecified / on border
         * \return the created prism
         */
        index_t create_prism(
            index_t v1, index_t v2,
            index_t v3, index_t v4,
            index_t v5, index_t v6, 
            index_t adj1 = NO_CELL,
            index_t adj2 = NO_CELL,
            index_t adj3 = NO_CELL,
            index_t adj4 = NO_CELL,
            index_t adj5 = NO_CELL
        ) {
            is_not_simplicial();
            cell_corners_.create_sub_element(v1);
            cell_corners_.create_sub_element(v2);
            cell_corners_.create_sub_element(v3);
            cell_corners_.create_sub_element(v4);
            cell_corners_.create_sub_element(v5);
            cell_corners_.create_sub_element(v6);
            cell_facets_.create_sub_element(adj1);
            cell_facets_.create_sub_element(adj2);
            cell_facets_.create_sub_element(adj3);
            cell_facets_.create_sub_element(adj4);
            cell_facets_.create_sub_element(adj5);
            cell_facets_.create_sub_element(NO_CELL); // padding           
            index_t result = create_sub_element(MESH_PRISM);
            cell_ptr_[nb()] = cell_corners_.nb();
            geo_debug_assert(cell_facets_.nb() == cell_corners_.nb());
            return result;
        }

        /**
         * \brief Creates a pyramid
         * \param[in] v1 , v2 , v3 , v4 , v5
         *  the vertices of the pyramid
         *  all in 0 .. mesh.vertices.nb()-1
         * \param[in] adj1 , adj2 , adj3 , adj4 , adj5
         *  adjacent cells, or NO_CELL if unspecified / on border
         * \return the created pyramid
         */
        index_t create_pyramid(
            index_t v1, index_t v2, index_t v3, index_t v4, index_t v5, 
            index_t adj1 = NO_CELL,
            index_t adj2 = NO_CELL,
            index_t adj3 = NO_CELL,
            index_t adj4 = NO_CELL,
            index_t adj5 = NO_CELL
        ) {
            is_not_simplicial();            
            cell_corners_.create_sub_element(v1);
            cell_corners_.create_sub_element(v2);
            cell_corners_.create_sub_element(v3);
            cell_corners_.create_sub_element(v4);
            cell_corners_.create_sub_element(v5);
            cell_facets_.create_sub_element(adj1);
            cell_facets_.create_sub_element(adj2);
            cell_facets_.create_sub_element(adj3);
            cell_facets_.create_sub_element(adj4);
            cell_facets_.create_sub_element(adj5);
            index_t result = create_sub_element(MESH_PYRAMID);
            cell_ptr_[nb()] = cell_corners_.nb();
            geo_debug_assert(cell_facets_.nb() == cell_corners_.nb());
            return result;
        }

        /**
         * \brief Creates a connector
         * \details Connector are automatically
         *  created by connect() (most client codes
         *  do not use this function)
         * \param[in] v1 , v2 , v3 , v4
         *  the vertices of the connector
         *  all in 0 .. mesh.vertices.nb()-1
         * \param[in] adj1 , adj2 , adj3
         *  adjacent cells, or NO_CELL if unspecified / on border
         * \return the created connector
         */
        index_t create_connector(
            index_t v1, index_t v2, index_t v3, index_t v4, 
            index_t adj1 = NO_CELL,
            index_t adj2 = NO_CELL,
            index_t adj3 = NO_CELL
        ) {
            is_not_simplicial();
            cell_corners_.create_sub_element(v1);
            cell_corners_.create_sub_element(v2);
            cell_corners_.create_sub_element(v3);
            cell_corners_.create_sub_element(v4);
            cell_facets_.create_sub_element(adj1);
            cell_facets_.create_sub_element(adj2);
            cell_facets_.create_sub_element(adj3);
            cell_facets_.create_sub_element(NO_CELL); // padding
            index_t result = create_sub_element(MESH_CONNECTOR);
            cell_ptr_[nb()] = cell_corners_.nb();
            geo_debug_assert(cell_facets_.nb() == cell_corners_.nb());
            return result;
        }

        /**
         * \brief Connects the cells.
         * \details This creates as needed the connectors that represent 
         *  non-conformal connections between a quadrilateral facet and 
         *  two triangular facets.
         * \param[in] remove_trivial_slivers if set, this removes the 
         *  slivers that are adjacent to a quadrilateral facet.
	 * \param[in] verbose_if_OK if set, says OK if no bad connector
	 *  configuration was detected.
         */
        void connect(
	    bool remove_trivial_slivers = true, bool verbose_if_OK=false
	);

        /**
         * \brief Replaces the surfacic part of this mesh
         *   with the borders of the volumetric part.
         */
        void compute_borders();

        /**
         * \brief Replaces the surfacic part of this mesh
         *   with the borders of the volumetric part.
	 * \param[out] facet_cell on exit, stores the
	 *   index of the cell adjacent to the facet
	 *   on the border.
         */
	void compute_borders(Attribute<index_t>& facet_cell);
	
        /**
         * \brief Copies a tetrahedron mesh into this Mesh.
         * \details Tetrahedron adjacences are not computed.
         * \param[in] dim dimension of the vertices
         * \param[in] vertices coordinates of the vertices
         * \param[in] tets tetrahedron to vertex links
         * \param[in] steal_args if set, vertices and tets
         * are 'stolen' from the arguments
         * (using vector::swap).
         */
        void assign_tet_mesh(
            coord_index_t dim,
            vector<double>& vertices,
            vector<index_t>& tets,
            bool steal_args
        );

        /**
         * \brief Copies a tetrahedron mesh into this Mesh.
         * \details Tetrahedron adjacences are not computed.
         * \param[in] tets tetrahedron to vertex links
         * \param[in] steal_args if set, vertices and tets
         * are 'stolen' from the arguments
         * (using vector::swap).
         */
        void assign_tet_mesh(
            vector<index_t>& tets,
            bool steal_args
        );

        virtual void pop();        
        
        index_t tet_adjacent(index_t t, index_t lf) const {
            geo_debug_assert(is_simplicial_);
            geo_debug_assert(t < nb());
            geo_debug_assert(lf < 4);
            return cell_facets_.adjacent_cell_[4*t+lf];
        }

        index_t find_tet_adjacent(index_t t, index_t t2) const {
            geo_debug_assert(is_simplicial_);
            geo_debug_assert(t < nb());
            geo_debug_assert(t2 < nb());
            for(index_t lf=0; lf<4; ++lf) {
                if(cell_facets_.adjacent_cell_[4*t+lf] == t2) {
                    return lf;
                }
            }
            return NO_FACET;
        }

        index_t tet_vertex(index_t t, index_t lv) const {
            geo_debug_assert(is_simplicial_);
            geo_debug_assert(t < nb());
            geo_debug_assert(lv < 4);
            return cell_corners_.corner_vertex_[4*t+lv];
        }

        index_t find_tet_vertex(index_t t, index_t v) const {
            geo_debug_assert(is_simplicial_);
            geo_debug_assert(t < nb());
            geo_debug_assert(v < vertices_.nb());
            for(index_t lv=0; lv<4; ++lv) {
                if(cell_corners_.corner_vertex_[4*t+lv] == v) {
                    return lv;
                }
            }
            return NO_VERTEX;
        }

        /**
         * \brief Gets a vertex of a tetrahedron by local facet
         *  index and local vertex index in facet.
         * \param[in] t global index of the tetrahedron
         * \param[in] lf local facet index (0,1,2 or 3)
         * \param[in] lv local vertex index in facet (0,1 or 2)
         * \return the global index of vertex \p lv in facet \p lf of
         *  tetrahedron \p t
         * \pre are_simplices()
         */
        index_t tet_facet_vertex(
            index_t t, index_t lf, index_t lv
        ) const {
            geo_debug_assert(is_simplicial_);            
            geo_debug_assert(t < nb());
            geo_debug_assert(lf < 4);
            geo_debug_assert(lv < 3);
            return cell_corners_.vertex(
                4 * t + local_tet_facet_vertex_index(lf,lv)
            );
        }
        
        /**
         * \brief Finds the local index of a facet in a tetrahedron
         *  by the global indices of its vertices.
         * \param[in] t index of the tetrahedron
         * \param[in] v1 global index of the first vertex
         * \param[in] v2 global index of the second vertex
         * \param[in] v3 global index of the third vertex
         * \return the local index (0,1,2 or 3) of the facet of
         *  \p t that has \p v1, \p v2, \p v3 as vertices modulo a
         *  circular permutation, or NO_FACET if such a facet does not
         *  exist in \p t.
         * \pre are_simplices()
         */
        index_t find_tet_facet(
            index_t t, index_t v1, index_t v2, index_t v3
        ) const {
            geo_debug_assert(is_simplicial_);            
            for(index_t lf = 0; lf < 4; ++lf) {
                index_t w1 = tet_facet_vertex(t, lf, 0);
                index_t w2 = tet_facet_vertex(t, lf, 1);
                index_t w3 = tet_facet_vertex(t, lf, 2);
                if(
                    (v1 == w1 && v2 == w2 && v3 == w3) ||
                    (v1 == w2 && v2 == w3 && v3 == w1) ||
                    (v1 == w3 && v2 == w1 && v3 == w2)
                ) {
                    return lf;
                }
            }
            return NO_FACET;
        }

        /**
         * \brief Gives the local index of a vertex in a
         *  tetrahedron from its facet and vertex local indices.
         * \param[in] lf local facet index (0,1,2 or 3)
         * \param[in] lv local vertex index in \p lf (0,1 or 2)
         * \return the local vertex index (0,1,2 or 3) of the
         * \p lv%th vertex in facet \p lf
         */
        static index_t local_tet_facet_vertex_index(index_t lf, index_t lv) {
            geo_debug_assert(lf < 4);
            geo_debug_assert(lv < 3);
            return MeshCellDescriptors::tet_descriptor.facet_vertex[lf][lv];
        }

    protected:
        
        /**
         * \brief Indicates that the stored elements are no
         *  longer only tetrahedra.
         * \details Creates the cell pointers and cell types
         *  for the pre-existing cells if any.
         */
        void is_not_simplicial() {
            if(is_simplicial_) {
                is_simplicial_ = false;
                cell_ptr_.resize(nb()+1);
                cell_type_.assign(nb(), MESH_TET);
                for(index_t c=0; c<cell_ptr_.size(); ++c) {
                    cell_ptr_[c] = 4*c;
                }
            }
        }

        /**
         * \brief Tests whether two cell facets can be connected.
         * \details Two cell facets can be connected if they have the
         *  same vertices in reverse order.
         * \param[in] c1 index of the first cell
         * \param[in] f1 index of the first facet in \p c1
         * \param[in] c2 index of the second cell
         * \param[in] f2 index of the second facet in \p c2
         * \retval true if \p c1 and \p c2 can be connected by \p f1 and \p f2
         * \retval false otherwise
         */
        bool facets_match(
            index_t c1, index_t f1, index_t c2, index_t f2
        ) const;
        
        /**
         * \brief Finds the local index of a vertex in a cell.
         * \param[in] c index of the cell
         * \param[in] v global index of the vertex
         * \return the local index 
         *  (in 0..cell_nb_vertices(c)-1) of the vertex in
         *  cell \p c or NO_VERTEX if \p c is not incident to \p v
         */
         index_t find_cell_vertex(index_t c, index_t v) const {
            geo_debug_assert(c < nb());
            geo_debug_assert(v < vertices_.nb());
            for(index_t lv=0; lv<nb_vertices(c); ++lv) {
                if(vertex(c,lv) == v) {
                    return lv;
                }
            }
            return NO_VERTEX;
        }
        
        /**
         * \brief Finds the local index of a facet in a cell
         *  that can be connected to a facet of another cell
         * \param[in] c1 index of the cell
         * \param[in] c2 index of the other cell
         * \param[in] f2 facet of the other cell
         * \return the local index (in 0 .. cell_nb_facets(c1)) of the facet of
         *  \p c1 that has the same vertices as \p f2 in \p c2 in reverse order,
         *  modulo a circular permutation, or NO_FACET if such a facet does not
         *  exist in \p c1.
         */
         index_t find_cell_facet(
            index_t c1, index_t c2, index_t f2
         ) const {
            for(index_t f1=0; f1<nb_facets(c1); ++f1) {
                if(facets_match(c1,f1,c2,f2)) {
                    return f1;
                }
            }
            return NO_FACET;
        }

        /**
         * \brief Tests whether a triangular facet matches a quad facet.
         * \details Used to detect non-conformal configurations that should
         *  be resolved by a connector.
         * \param[in] c1 index of the first cell
         * \param[in] lf1 index of a triangular facet in \p c1
         * \param[in] c2 index of the second cell
         * \param[in] lf2 index of a quadrangular facet in \p c2
         * \retval true if the three vertices of \p f1 appear in \p f2
         *   in reverse order
         * \retval false otherwise
         */
        bool triangular_facet_matches_quad_facet(
            index_t c1, index_t lf1,
            index_t c2, index_t lf2
        ) const;
        

        /**
         * \brief Tests whether two triangular cell facets have a common edge.
         * \param[in] c1 index of the first cell
         * \param[in] f1 index of a triangular facet of \p c1
         * \param[in] c2 index of the second cell
         * \param[in] f2 index of a triangular facet of \p c2
         * \param[out] e1 index of the common edge in \p f1
         *  or NO_EDGE if no such edge exists
         * \param[out] e2 index of the common edge in \p f2
         *  or NO_EDGE if no such edge exists
         * \retval true if \p f1 and \p f2 have a common edge
         * \retval false otherwise
         */
        bool triangular_facets_have_common_edge(
            index_t c1, index_t f1,
            index_t c2, index_t f2,
            index_t& e1, index_t& e2
        ) const;

        /**
         * \brief Creates a connector between a quadrandular facet and two 
         *  triangular facets.
         * \details This function is used by connect_cells()
         * \param[in] c1 index of the cell that has the quadrangular facet
         * \param[in] lf1 index of the quadrangular facet in \p c1
         * \param[in] matches a const reference to a vector of 
         *  (cell index, facet index) pairs that are candidate triangles to be 
         *  connected to the quadrangular facet. Each of them
         *  has three vertices in common with the quadrangular facet. 
         *  It may contain more than two (cell,facet) index pairs. 
         *  In this case, among them we select the pair of triangular facets
         *  that have an edge in common. 
         * \retval true if a connector was created. A connector is created if 
         *  among the candidate triangular facets there are exactly two facets 
         *  on the border with an edge in common.
         * \retval false otherwise
         */
        bool create_connector(
            index_t c1, index_t lf1,
            const std::vector< std::pair<index_t, index_t> >& matches
        ); 

        /**
         * \brief Optimized implementation of connect() used
         *  when the mesh is simplicial.
         */
        void connect_tets();
        
    protected:
        MeshVertices& vertices_;
        MeshCellCornersStore& cell_corners_;
        MeshCellFacetsStore& cell_facets_;
        friend class Mesh;
        friend class GeogramIOHandler;                
    };
    
    /*************************************************************************/

    /**
     * \brief Indicates the mesh elements (vertices, facets or cells)
     *  present in a mesh.
     * \details The set of elements present in a mesh is represented
     *  by a bitwise-or combination of the constants.
     * \relates Mesh
     */
    enum MeshElementsFlags {
        MESH_NONE = 0,
        MESH_VERTICES = 1,
        MESH_FACETS = 2,
        MESH_EDGES  = 4,
        MESH_CELLS  = 8,
        MESH_ALL_ELEMENTS = 15,
        MESH_FACET_CORNERS = 16,
        MESH_CELL_CORNERS = 32,        
        MESH_CELL_FACETS = 64,
        MESH_ALL_SUBELEMENTS = 65
    };

    /*************************************************************************/
    
    /**
     * \brief Represents a mesh.
     * \details A mesh can have vertices, optionally facets and
     *  optionally volumetric cells. Attributes can be attached
     *  to all elements and sub-elements. 
     */
    class GEOGRAM_API Mesh {
    public:
        MeshVertices          vertices;
        MeshEdges             edges;
        MeshFacets            facets;
        MeshFacetCornersStore facet_corners;
        MeshCells             cells;
        MeshCellCornersStore  cell_corners;
        MeshCellFacetsStore   cell_facets;

        /**
         * \brief Mesh constructor
         * \param[in] dimension dimension of the vertices
         * \param[in] single_precision if true, vertices are
         *  stored in single precision (float), else they are
         *  stored as double precision (double).
         */
        Mesh(index_t dimension=3, bool single_precision=false);

	/**
	 * \brief Mesh destructor.
	 */
	virtual ~Mesh();
	
        /**
         * \brief Removes all the elements and attributes of
         *  this mesh.
         * \param[in] keep_attributes if true, then all the
         *  existing attribute names / bindings are kept (but 
         *  they are cleared). If false, they are destroyed.
         * \param[in] keep_memory if true, then memory is
         *  kept and can be reused by subsequent mesh
         *  element creations.
         */
        void clear(bool keep_attributes=true, bool keep_memory=false);

        /**
         * \brief Displays number of vertices, facets and borders.
         */
        void show_stats(const std::string& tag = "Mesh") const;


        /**
         * \brief Does some validity checks.
         * \details Used for debugging. If the
         *  validity checks are not satisfied, 
         *  then it crashes with an assertion
         *  failure.
         */
        void assert_is_valid();


        /**
         * \brief Copies a mesh onto this one
         * \param[in] rhs a const reference to the mesh to be copied
         * \param[in] copy_attributes if true, all the attributes are
         *   copied.
         * \param[in] what a combination of MESH_VERTICES, MESH_EDGES,
         *  MESH_FACETS, MESH_CELLS flags. Set to MESH_ALL_ELEMENTS
         *  to copy everything (default). If MESH_VERTICES is not set,
         *  then the mesh is cleared.
         */
        void copy(
            const Mesh& rhs,
            bool copy_attributes=true,
            MeshElementsFlags what=MESH_ALL_ELEMENTS
        );


        /**
         * \brief Gets the list of all attributes.
         * \return a ';'-separated list of all attributes. 
         */
        std::string get_attributes() const;
    
        /**
         * \brief Gets the list of all scalar attributes.
         * \return a ';'-separated list of all scalar attributes. 
	 * \details Whenever there is a vector attribute v of dim d,
	 *  it appends v[0];v[1];...v[d-1] to the list.
         */
        std::string get_scalar_attributes() const;

	/**
	 * \brief Gets the list of all vector attributes.
	 * \param[in] max_dim if non-zero, only vector attributes of
	 *  dimension lower than \p max_dim are returned.
	 * \return a ';'-separated list of all vector attributes.
	 */
	std::string get_vector_attributes(index_t max_dim = 0) const;
	
        /**
         * \brief Gets the number of subelements types.
         * \return the number of subelements types.
         */
        index_t nb_subelements_types() const;

        /**
         * \brief Gets a MeshSubElementsStore by index.
         * \param[in] i index of the subelements
         * \return a reference to the corresponding MeshSubElementsStore
         * \pre i < nb_subelements_types()
         */
        MeshSubElementsStore& get_subelements_by_index(index_t i);

        /**
         * \brief Gets a MeshSubElementsStore by index.
         * \param[in] i index of the subelements
         * \return a const reference to the corresponding MeshSubElementsStore
         * \pre i < nb_subelements_types()
         */
        const MeshSubElementsStore& get_subelements_by_index(index_t i) const;
        
        
        /**
         * \brief Gets a MeshSubElementsStore by subelements type.
         * \param[in] what one of MESH_VERTICES, MESH_EDGES, MESH_FACETS,
         *  MESH_FACET_CORNERS, MESH_CELLS, MESH_CELL_CORNERS, MESH_CELL_FACETS
         * \return a reference to the corresponding MeshSubElementsStore
         */
        MeshSubElementsStore& get_subelements_by_type(MeshElementsFlags what);

        /**
         * \brief Gets a MeshSubElementsStore by subelements type.
         * \param[in] what one of MESH_VERTICES, MESH_EDGES, MESH_FACETS,
         *  MESH_FACET_CORNERS, MESH_CELLS, MESH_CELL_CORNERS, MESH_CELL_FACETS
         * \return a const reference to the corresponding MeshSubElementsStore
         */
        const MeshSubElementsStore& get_subelements_by_type(
            MeshElementsFlags what
        ) const;

        /**
         * \brief Gets a subelement name by subelement type.
         * \param[in] what one of MESH_VERTICES, MESH_EDGES, MESH_FACETS,
         *  MESH_FACET_CORNERS, MESH_CELLS, MESH_CELL_CORNERS, MESH_CELL_FACETS
         * \return a string with the name of the subelement.
         */
        static std::string subelements_type_to_name(MeshElementsFlags what);

        /**
         * \brief Gets a subelement type by subelement name.
         * \param[in] name the name of the subelement as a string
         * \return one of MESH_VERTICES, MESH_EDGES, MESH_FACETS,
         *  MESH_FACET_CORNERS, MESH_CELLS, MESH_CELL_CORNERS, MESH_CELL_FACETS
         *  or MESH_NONE if the name is invalid
         */
        static MeshElementsFlags name_to_subelements_type(
            const std::string& name
        );

        
    protected:
        /**
         * \brief Displays the list of attributes to the Logger.
         * \param[in] tag the tag to be sent to the Logger
         * \param[in] subelement_name the name of the subelement
         *   (vertices, facets, facet_corners ...)
         * \param[in] subelements a const reference to the MeshSubElementsStore
         */
        void display_attributes(
            const std::string& tag, const std::string& subelement_name,
            const MeshSubElementsStore& subelements 
        ) const;

    private:
        /**
         * \brief Forbids copy.
         * \details This is to make sure that client code does
         *   not unintentionlly copies a Mesh (for
         *   instance by passing it by-value to a function). 
         *   Use copy() instead.
         */
        Mesh(const Mesh& rhs);

        /**
         * \brief Forbids copy.
         * \details This is to make sure that client code does
         *   not unintentionlly copies a Mesh (for
         *   instance by passing it by-value to a function). 
         *   Use copy() instead.
         */
        const Mesh& operator=(const Mesh& rhs);
    };

    /*************************************************************************/
}

#endif
