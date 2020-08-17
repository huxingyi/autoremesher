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

#ifndef GEOGRAM_POINTS_KD_TREE
#define GEOGRAM_POINTS_KD_TREE

#include <geogram/basic/common.h>
#include <geogram/points/nn_search.h>
#include <algorithm>

/**
 * \file geogram/points/kd_tree.h
 * \brief An implementation of NearestNeighborSearch with a kd-tree
 */

namespace GEO {

    /**
     * \brief Base class for all Kd-tree implementations.
     */
    class GEOGRAM_API KdTree : public NearestNeighborSearch {
    public:
        /**
         * \brief KdTree constructor.
         * \param[in] dim dimension of the points.
         */
	KdTree(coord_index_t dim);

	/** \copydoc NearestNeighborSearch::set_points() */
        virtual void set_points(index_t nb_points, const double* points);

	/** \copydoc NearestNeighborSearch::stride_supported() */	
        virtual bool stride_supported() const;

	/** \copydoc NearestNeighborSearch::set_points() */
        virtual void set_points(
            index_t nb_points, const double* points, index_t stride
        );

	/** \copydoc NearestNeighborSearch::get_nearest_neighbors() */
        virtual void get_nearest_neighbors(
            index_t nb_neighbors,
            const double* query_point,
            index_t* neighbors,
            double* neighbors_sq_dist
        ) const;

	/** \copydoc NearestNeighborSearch::get_nearest_neighbors() */
        virtual void get_nearest_neighbors(
            index_t nb_neighbors,
            const double* query_point,
            index_t* neighbors,
            double* neighbors_sq_dist,
	    KeepInitialValues
        ) const;

	/** \copydoc NearestNeighborSearch::get_nearest_neighbors() */	
        virtual void get_nearest_neighbors(
            index_t nb_neighbors,
            index_t query_point,
            index_t* neighbors,
            double* neighbors_sq_dist
        ) const;
	
	/**********************************************************************/
	
        /**
         * \brief The context for traversing a KdTree.
         * \details Stores a sorted sequence of (point,distance)
         *  couples.
         */
        struct NearestNeighbors {

            /**
             * \brief Creates a new NearestNeighbors
             * \details Storage is provided and managed by the caller.
             * Initializes neighbors_sq_dist[0..nb_neigh-1]
             * to Numeric::max_float64() and neighbors[0..nb_neigh-1]
             * to index_t(-1).
             * \param[in] nb_neighbors_in number of neighbors to retrieve
             * \param[in] user_neighbors_in storage for the neighbors, allocated
             *  and managed by caller, with space for nb_neighbors_in integers
             * \param[in] user_neighbors_sq_dist_in storage for neighbors 
	     *  squared distance, allocated and managed by caller, 
	     *  with space for nb_neighbors_in doubles
             * \param[in] work_neighbors_in storage for the neighbors, allocated
             *  and managed by caller, with space 
	     *  for nb_neighbors_in + 1 integers
             * \param[in] work_neighbors_sq_dist_in storage 
	     *  for neighbors squared distance, allocated and managed 
	     *  by caller, with space for nb_neighbors_in + 1 doubles
             */
            NearestNeighbors(
                index_t nb_neighbors_in,
		index_t* user_neighbors_in,
		double* user_neighbors_sq_dist_in,
                index_t* work_neighbors_in,
                double* work_neighbors_sq_dist_in
            ) :
                nb_neighbors(0),
		nb_neighbors_max(nb_neighbors_in),
                neighbors(work_neighbors_in),
		neighbors_sq_dist(work_neighbors_sq_dist_in),
	        user_neighbors(user_neighbors_in),
                user_neighbors_sq_dist(user_neighbors_sq_dist_in),
	        nb_visited(0)
	    {
		// Yes, '<=' because we got space for n+1 neigbors
		// in the work arrays.
		for(index_t i = 0; i <= nb_neighbors; ++i) {
		    neighbors[i] = index_t(-1);
		    neighbors_sq_dist[i] = Numeric::max_float64();
		}
            }
	    
            /**
             * \brief Gets the squared distance to the furthest
             *  neighbor.
             */
            double furthest_neighbor_sq_dist() const {
                return
		    nb_neighbors == nb_neighbors_max ?
		    neighbors_sq_dist[nb_neighbors - 1] :
		    Numeric::max_float64()
		    ;
            }

            /**
             * \brief Inserts a new neighbor.
             * \details Only the nb_neighbor nearest points are kept.
             * \param[in] neighbor the index of the point
             * \param[in] sq_dist the squared distance between the point
             *  and the query point.
	     * \pre sq_dist <= furthest_neighbor_sq_dist() (needs to be tested
	     *  by client code before insertion).
             */
            void insert(
                index_t neighbor, double sq_dist
            ) {
		geo_debug_assert(
		    sq_dist <= furthest_neighbor_sq_dist()
		);

		int i;
		for(i=int(nb_neighbors); i>0; --i) {
		    if(neighbors_sq_dist[i - 1] < sq_dist) {
			break;
		    } 
		    neighbors[i] = neighbors[i - 1];
		    neighbors_sq_dist[i] = neighbors_sq_dist[i - 1];
		}

                neighbors[i] = neighbor;
                neighbors_sq_dist[i] = sq_dist;

		if(nb_neighbors < nb_neighbors_max) {
		    ++nb_neighbors;
		}
            }

	    /**
	     * \brief Copies the user neighbors and distances into 
	     *  the work zone and initializes nb_neighbors to max_nb_neighbors.
	     * \details This function is called by nearest neighbors search when
	     *  KeepInitialValues is specified, to initialize search 
	     *  from user-provided initial guess.
	     */
	    void copy_from_user() {
		for(index_t i=0; i<nb_neighbors_max; ++i) {
		    neighbors[i] = user_neighbors[i];
		    neighbors_sq_dist[i] = user_neighbors_sq_dist[i];
		}
		neighbors[nb_neighbors_max] = index_t(-1);
		neighbors_sq_dist[nb_neighbors_max] = Numeric::max_float64();
		nb_neighbors = nb_neighbors_max;
	    }

	    /**
	     * \brief Copies the found nearest neighbors from the work zone 
	     *  to the user neighbors and squared distance arrays.
	     * \details This function is called by find_nearest_neighbors() 
	     *  after traversal of the tree.
	     */
	    void copy_to_user() {
		for(index_t i=0; i<nb_neighbors_max; ++i) {
		    user_neighbors[i] = neighbors[i];
		    user_neighbors_sq_dist[i] = neighbors_sq_dist[i];
		}
	    }

	    /** \brief Current number of neighbors. */
	    index_t nb_neighbors;

	    /** \brief Maximum number of neighbors. */
	    index_t nb_neighbors_max;

	    /** 
	     * \brief Internal array of neighbors.
	     * \details size = nb_neigbors_max + 1 
	     */
            index_t* neighbors;

	    /** 
	     * \brief Internal squared distance to neigbors.
	     * \details size = nb_neigbors_max + 1 
	     */
            double* neighbors_sq_dist;

	    /**
	     * \brief User-provided array of neighbors.
	     * \details size = nb_neighbors_max
	     */
	    index_t* user_neighbors;

	    /**
	     * \brief User-provided array of neighbors 
	     *  squared distances.
	     * \details size = nb_neighbors_max
	     */
	    double* user_neighbors_sq_dist;

	    /**
	     * \brief Number of points visited during
	     *  traversal.
	     */
	    size_t nb_visited;
        };

        /**
         * \brief The recursive function to implement KdTree traversal and
         *  nearest neighbors computation.
	 * \note This is a lower-level function, most users will not use it.
         * \details Traverses the subtree under the
         *  node_index node that corresponds to the
         *  [b,e) point sequence. Nearest neighbors
         *  are inserted into neighbors during
         *  traversal.
         * \param[in] node_index index of the current node in the Kd tree
         * \param[in] b index of the first point in the subtree under
         *  node \p node_index
         * \param[in] e one position past the index of the last point in the
         *  subtree under node \p node_index
         * \param[in,out] bbox_min coordinates of the lower
         *  corner of the bounding box.
         *  Allocated and managed by caller.
         *  Modified by the function and restored on exit.
         * \param[in,out] bbox_max coordinates of the
         *  upper corner of the bounding box.
         *  Allocated and managed by caller.
         *  Modified by the function and restored on exit.
         * \param[in] bbox_dist squared distance between
         *  the query point and a bounding box of the
         *  [b,e) point sequence. It is used to early
         *  prune traversals that do not generate nearest
         *  neighbors.
         * \param[in] query_point the query point
         * \param[in,out] neighbors the computed nearest neighbors
         */
        virtual void get_nearest_neighbors_recursive(
            index_t node_index, index_t b, index_t e,
            double* bbox_min, double* bbox_max,
            double bbox_dist, const double* query_point,
            NearestNeighbors& neighbors
        ) const;

	/**
	 * \brief Initializes bounding box and box distance for
	 *  Kd-Tree traversal.
	 * \note This is a lower-level function, most users will not use it.
	 * \details This functions needs to be called before 
	 *  get_nearest_neighbors_recursive()
	 * \param[out] bbox_min a pointer to an array of dimension() doubles,
	 *   managed by client code (typically on the stack).
	 * \param[out] bbox_max a pointer to an array of dimension() doubles,
	 *   managed by client code (typically on the stack).
	 * \param[out] box_dist the squared distance between the query point and
	 *   the box.
	 * \param[in] query_point a const pointer to the coordinates of
	 *   the query point.
	 */
	void init_bbox_and_bbox_dist_for_traversal(
	    double* bbox_min, double* bbox_max,
	    double& box_dist, const double* query_point
	) const;

	/**
	 * \brief Gets the root node.
	 * \return the index of the root node.
	 */
	index_t root() const {
	    return root_;
	}
	
    protected:
        /**
         * \brief Number of points stored in the leafs of the tree.
         */
        static const index_t MAX_LEAF_SIZE = 16;

	/**
	 * \brief Builds the tree.
	 * \return the index of the root node.
	 */
	virtual index_t build_tree() = 0 ;

	/**
	 * \brief Gets all the attributes of a node.
	 * \details This function is virtual, because indices can
	 *  be either computed on the fly (as in BalancedKdTree) or
	 *  stored (as in AdaptiveKdTree).
	 * \param[in] n a node index
	 * \param[in] b the first point in the node
	 * \param[in] e one position past the last point in the node
	 * \param[out] left_child the node index of the 
	 *   left child of node \p n.
	 * \param[out] right_child the node index of the 
	 *   right child of node \p n.
	 * \param[out] splitting_coord The coordinate along which \p n is split.
	 * \param[out] m the point m such that [b,m-1] corresponds 
	 *  to the points in the left child of \p n and [m,e-1] 
	 *  corresponds to the points in the right child of \p n.
	 * \param[out] splitting_val The coordinate value that separates points
	 *  in the left and right children.
	 */
	virtual void get_node(
	    index_t n, index_t b, index_t e,
	    index_t& left_child, index_t& right_child,
	    coord_index_t&  splitting_coord,
	    index_t& m,
	    double& splitting_val
	) const = 0;
	


        /**
         * \brief The recursive function to implement KdTree traversal and
         *  nearest neighbors computation in a leaf.
         * \details Traverses the node_index leaf that corresponds to the
         *  [b,e) point sequence. Nearest neighbors
         *  are inserted into neighbors during traversal.
         * \param[in] node_index index of the leaf to be traversed.
         * \param[in] b index of the first point in the leaf.
         * \param[in] e one position past the index of the last point in the
	 *  leaf.
         * \param[in] query_point the query point
         * \param[in,out] neighbors the computed nearest neighbors
         */
	virtual void get_nearest_neighbors_leaf(
            index_t node_index, index_t b, index_t e,
	    const double* query_point,
            NearestNeighbors& neighbors	    
	) const;

	/**
	 * \brief Computes the minimum and maximum point coordinates 
	 *   along a coordinate.
	 * \param[in] b first index of the point sequence
	 * \param[in] e one position past the last index of the point sequence
	 * \param[in] coord coordinate along which the extent is measured
	 * \param[out] minval , maxval minimum and maximum 
	 */
	void get_minmax(
	    index_t b, index_t e, coord_index_t coord,
	    double& minval, double& maxval
	) const {
	    minval = Numeric::max_float64();
	    maxval = Numeric::min_float64();
	    for(index_t i = b; i < e; ++i) {
		double val = point_ptr(point_index_[i])[coord];
		minval = std::min(minval, val);
		maxval = std::max(maxval, val);
	    }
	}

	/**
	 * \brief Computes the extent of a point sequence 
	 *  along a given coordinate.
	 * \param[in] b first index of the point sequence
	 * \param[in] e one position past the last index of the point sequence
	 * \param[in] coord coordinate along which the extent is measured
	 * \return the extent of the sequence along the coordinate
	 */
	double spread(index_t b, index_t e, coord_index_t coord) const {
	    double minval,maxval;
	    get_minmax(b,e,coord,minval,maxval);
	    return maxval - minval;
	}

	/**
	 * \brief KdTree destructor.
	 */
	virtual ~KdTree();

      protected:
        vector<index_t> point_index_;
        vector<double> bbox_min_;
        vector<double> bbox_max_;
	index_t root_;
    };

    /*********************************************************************/
    
    /**
     * \brief Implements NearestNeighborSearch using a balanced
     *  Kd-tree.
     * \details The tree is perfectly balanced, thus no combinatorics
     *  is stored: the two children of node n are 2n+1 and 2n+2. For
     *  regular to moderately irregular pointsets it works well. For
     *  highly irregular pointsets, AdaptiveKdTree is more efficient.
     */
    class GEOGRAM_API BalancedKdTree : public KdTree {
    public:
        /**
         * \brief Creates a new BalancedKdTree.
         * \param[in] dim dimension of the points
         */
        BalancedKdTree(coord_index_t dim);

    protected:
        /**
         * \brief BalancedKdTree destructor
         */
        virtual ~BalancedKdTree();

        /**
         * \brief Returns the maximum node index in subtree.
         * \param[in] node_id node index of the subtree
         * \param[in] b first index of the points sequence in the subtree
         * \param[in] e one position past the last index of the point
         *  sequence in the subtree
         */
        static index_t max_node_index(
            index_t node_id, index_t b, index_t e
        ) {
            if(e - b <= MAX_LEAF_SIZE) {
                return node_id;
            }
            index_t m = b + (e - b) / 2;
            return std::max(
                max_node_index(2 * node_id, b, m),
                max_node_index(2 * node_id + 1, m, e)
            );
        }

        /**
         * \brief Computes the coordinate along which a point
         *   sequence will be split.
         * \param[in] b first index of the point sequence
         * \param[in] e one position past the last index of the point sequence
         */
        coord_index_t best_splitting_coord(index_t b, index_t e);

        /**
         * \brief Creates the subtree under a node.
         * \param[in] node_index index of the node that represents
         *  the subtree to create
         * \param[in] b first index of the point sequence in the subtree
         * \param[in] e one position past the last index of the point
         *  index in the subtree
         */
        void create_kd_tree_recursive(
            index_t node_index, index_t b, index_t e
        ) {
            if(e - b <= MAX_LEAF_SIZE) {
                return;
            }
            index_t m = split_kd_node(node_index, b, e);
            create_kd_tree_recursive(2 * node_index, b, m);
            create_kd_tree_recursive(2 * node_index + 1, m, e);
        }

        /**
         * \brief Computes and stores the splitting coordinate
         *  and splitting value of the node node_index, that
         *  corresponds to the [b,e) points sequence.
         *
         * \return a node index m. The point sequences
         *  [b,m) and [m,e) correspond to the left
         *  child (2*node_index) and right child (2*node_index+1)
         *  of node_index.
         */
        index_t split_kd_node(
            index_t node_index, index_t b, index_t e
        );

	/** \copydoc KdTree::build_tree() */
	virtual index_t build_tree();

	/** \copydoc KdTree::get_node() */
	virtual void get_node(
	    index_t n, index_t b, index_t e,
	    index_t& left_child, index_t& right_child,
	    coord_index_t&  splitting_coord,
	    index_t& m,
	    double& splitting_val
	) const;
	
    protected:
	
	/**
	 * \brief One per node, splitting coordinate.
	 */
        vector<coord_index_t> splitting_coord_;

	/**
	 * \brief One per node, splitting coordinate value.
	 */
        vector<double> splitting_val_;

	/**
	 * \brief Indices for multithreaded tree construction.
	 */
        index_t m0_, m1_, m2_, m3_, m4_, m5_, m6_, m7_, m8_;
    };

    /*********************************************************************/

    /**
     * \brief Implements NearestNeighborSearch using an Adaptive
     *  Kd-tree.
     * \details This corresponds to the same algorithm as in the 
     *  ANN library (by David Mount), but stored in flat arrays 
     *  (rather than dynamically allocated tree structure). The 
     *  data structure is more compact, and slightly faster.
     *  As compared with BalancedKdTree, when the distribution of 
     *  points is heterogeneous, it will be faster, at the expensen of 
     *  a slightly more requires storage (uses an additional 8 bytes
     *  per node), and construction is not parallel, because size of
     *  left subtree needs to be known before starting constructing
     *  the right subtree. This does not make a big difference since
     *  in general Kd-tree query time dominates construction time in
     *  most of the algorithms that use a Kd-tree.
     */
    class GEOGRAM_API AdaptiveKdTree : public KdTree {
    public:
        /**
         * \brief Creates a new BalancedKdTree.
         * \param[in] dim dimension of the points
         */
        AdaptiveKdTree(coord_index_t dim);

    protected:	
	/** \copydoc KdTree::build_tree() */
	virtual index_t build_tree();

	/** \copydoc KdTree::get_node() */
	virtual void get_node(
	    index_t n, index_t b, index_t e,
	    index_t& left_child, index_t& right_child,
	    coord_index_t&  splitting_coord,
	    index_t& m,
	    double& splitting_val
	) const;

        /**
         * \brief Creates the subtree under a node.
         * \param[in] b first index of the point sequence in the subtree
         * \param[in] e one position past the last index of the point
         *  index in the subtree
         * \param[in,out] bbox_min coordinates of the lower
         *  corner of the bounding box.
         *  Allocated and managed by caller.
         *  Modified by the function and restored on exit.
	 * \return the node index of the root of the created tree.
         */
        virtual index_t create_kd_tree_recursive(
	    index_t b, index_t e,
            double* bbox_min, double* bbox_max	    	    
	);

        /**
         * \brief Computes and stores the splitting coordinate
         *  and splitting value of the node node_index, that
         *  corresponds to the [b,e) points sequence.
         *  The point sequences [b,m) and [m,e) correspond to the left
         *  child (2*node_index) and right child (2*node_index+1)
         *  of node_index.
         * \param[in,out] bbox_min coordinates of the lower
         *  corner of the bounding box.
         *  Allocated and managed by caller.
         *  Modified by the function and restored on exit.
	 * \param[out] m the point index.
	 * \param[out] cut_dim the coordinate along which the node is split.
	 * \param[out] cut_val the splitting value.
         */
        virtual void split_kd_node(
            index_t b, index_t e,
            double* bbox_min, double* bbox_max,
	    index_t& m, coord_index_t& cut_dim, double& cut_val
        );

	/**
	 * \brief Reorders the points in a sequence in such a way that 
	 *  the specified coordinate in the beginning of the sequence is 
	 *  smaller than the specified cutting value.
	 * \param[in] b first index of the point sequence
	 * \param[in] e one position past the last index of the point sequence
	 * \param[in] coord coordinate along which the extent is measured
	 * \param[in] val the cutting value
	 * \param[out] br1 , br2 on exit, point indices are reordered in such 
	 *  a way that:
	 *   - the sequence b   .. br1-1 has points with coord smaller than val
	 *   - the sequence br1 .. br2-1 has points with coord equal to val
	 *   - the sequence br2 .. e-1   has points with coord larger than val
	 */
	virtual void plane_split(
	    index_t b, index_t e, coord_index_t coord, double val,
	    index_t& br1, index_t& br2
	);

	/**
	 * \brief Gets a point coordinate by index and coordinate.
	 * \param[in] index index of the point.
	 * \param[in] coord coordinate, in 0..dimension()-1
	 * \return the coordinate of the point, after re-numerotation.
	 */
	double point_coord(int index, coord_index_t coord) {
	    geo_debug_assert(index >= 0);
	    geo_debug_assert(index_t(index) < nb_points());
	    geo_debug_assert(coord < dimension());
	    index_t direct_index = point_index_[index_t(index)];
	    geo_debug_assert(direct_index < nb_points());
	    return (points_ + direct_index * stride_)[coord];
	}
	
	
	/**
	 * \brief Gets the number of nodes.
	 * \return the number of nodes.
	 */
	index_t nb_nodes() const {
	    return splitting_coord_.size();
	}

	/**
	 * \brief Creates a new node.
	 * \return the index of the newly created node.
	 */
	virtual index_t new_node();
	
     protected:
	/**
	 * \brief One per node, splitting coordinate.
	 */
	vector<coord_index_t> splitting_coord_;

	/**
	 * \brief One per node, splitting coordinate value.
	 */
        vector<double> splitting_val_;

	/**
	 * \brief One per node, node splitting index.
	 * \details Children points sequences:
	 *  - left child points: b .. node_m_[node_index]-1
	 *  - right child points: node_m_[node_index] .. e-1
	 */
	vector<index_t> node_m_;
	
	/**
	 * \brief One per node, right child index.
	 * \details left child is implicit (left_child(n) = n+1).
	 */
	vector<index_t> node_right_child_;
    };
    
    /*********************************************************************/    
}

#endif

