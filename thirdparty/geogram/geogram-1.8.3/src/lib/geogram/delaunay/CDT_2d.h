/*
 *  Copyright (c) 2000-2022 Inria
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
 *  Contact: Bruno Levy
 *
 *     https://www.inria.fr/fr/bruno-levy
 *
 *     Inria,
 *     Domaine de Voluceau,
 *     78150 Le Chesnay - Rocquencourt
 *     FRANCE
 *
 */

#ifndef GEOGRAM_DELAUNAY_CDT_2D
#define GEOGRAM_DELAUNAY_CDT_2D

#include <geogram/basic/common.h>
#include <geogram/basic/geometry.h>
#include <geogram/numerics/predicates.h>
#include <functional>

/**
 * \file geogram/delaunay/CDT_2d.h
 * \brief Simple constained Delaunay triangulation in 2D
 * \details See documentation of CDT at the end of this file.
 */

namespace GEO {

    /**
     * \brief Forward declaration of a small data structure
     *  used by CDTBase2d::find_intersected_edges()
     */
    struct CDT2d_ConstraintWalker;
    
    /**
     * \brief Base class for constrained Delaunay triangulation
     * \details Manages the combinatorics of the constrained Delaunay
     *  triangulation. The points need to be stored elsewhere, and manipulated 
     *  through indices, with two predicates:
     *  - orient2d(i,j,k)
     *  - incircle(i,j,k,l)
     *  and one construction:
     *  - create_intersection(i,j,k,l)
     *  See \ref CDT for an example of implementation
     */
    class GEOGRAM_API CDTBase2d {
    public:
        /**
         * \brief CDTBase2d constructor
         */
        CDTBase2d();

        /**
         * \brief CDTBase2d destructor
         */
        virtual ~CDTBase2d();

        /**
         * \brief Removes everything from this triangulation
         */
        virtual void clear();
        
        /**
         * \brief Inserts a constraint
         * \param[in] i , j the indices of the two vertices
         *  of the constrained segment
         */
        void insert_constraint(index_t i, index_t j);

        /**
         * \brief Recursively removes all the triangles adjacent to
         *  the border, and keeps what's surrounded by constraints
         */
        void remove_external_triangles();
        
        /**
         * \brief Specifies whether a constrained Delaunay
         *  triangulation should be constructed, or just a
         *  plain constrained triangulation
         * \param[in] delaunay true if a Delaunay triangulation
         *  should be constructed, false otherwise.
         */
        void set_delaunay(bool delaunay) {
            delaunay_ = delaunay;
        }
        
        /**
         * \brief Gets the number of triangles
         */
        index_t nT() const {
            return T_.size()/3;
        }

        /**
         * \brief Gets the number of vertices
         */
        index_t nv() const {
            return nv_;
        }

        /**
         * \brief Gets the number of constraints
         */
        index_t ncnstr() const {
            return ncnstr_;
        }
        
        /**
         * \brief Gets a vertex of a triangle
         * \param[in] t the triangle
         * \param[in] lv the local index of the vertex, in 0,1,2
         * \return the global index of the vertex
         */
        index_t Tv(index_t t, index_t lv) const {
            geo_debug_assert(t<nT());
            geo_debug_assert(lv<3);
            return T_[3*t+lv];
        }

        /**
         * \brief Finds the local index of a vertex in a triangle
         * \param[in] t the triangle
         * \param[in] v the vertex
         * \return lv such that Tv(t,lv) = v
         */
        index_t Tv_find(index_t t, index_t v) const {
            geo_debug_assert(t<nT());
            geo_debug_assert(v<nv());
            return find_3(T_.data()+3*t, v); 
        }
        
        /**
         * \brief Gets a triangle adjacent to a triangle
         * \param[in] t the triangle
         * \param[in] le the local edge index, in 0,1,2
         * \return the triangle adjacent to \p t accross \p le,
         *  or index_t(-1) if there is no such triangle
         */
        index_t Tadj(index_t t, index_t le) const {
            geo_debug_assert(t<nT());
            geo_debug_assert(le<3);
            return Tadj_[3*t+le];
        }
        
        /**
         * \brief Finds the edge accross which a triangle is
         *  adjacent to another one
         * \param[in] t1 , t2 the two triangles
         * \return the local edge index le such that
         *  Tadj(t1,le) = t2
         */
        index_t Tadj_find(index_t t1, index_t t2) const {
            geo_debug_assert(t1<nT());
            geo_debug_assert(t2<nT());
            return find_3(Tadj_.data()+3*t1, t2); 
        }

        /**
         * \brief Gets a triangle incident to a given vertex
         * \param[in] v a vertex
         * \return a triangle t such that there exists lv in 
         *  0,1,2 such that Tv(t,lv) = v
         */
        index_t vT(index_t v) const {
            geo_debug_assert(v < nv());
            return v2T_[v];
        }

        /**
         * \brief Saves this CDT to a geogram mesh file.
         * \param[in] filename where to save this CDT
         */
        virtual void save(const std::string& filename) const = 0;


        /**
         * \brief Tests whether a triangle edge is Delaunay
         * \details returns true also for constrained edges and edges on borders
         */
        bool Tedge_is_Delaunay(index_t t, index_t le) const;

    protected:
        /**
         * \brief Inserts a new point 
         * \param[in] v the index of the new point, supposed to be
         *  equal to nv()
         * \param[in] hint an optional triangle, not too far away
         *  from the point to be inserted
         * \return the index of the created point. May be different
         *  from v if the point already existed in the triangulation
         */
        index_t insert(index_t v, index_t hint = index_t(-1));

        /**
         * \brief Creates the combinatorics for a first large enclosing
         *  triangle
         * \param[in] v1 , v2 , v3 the three vertices of the first triangle,
         *  in 0,1,2
         * \details create_enclosing_triangle() or create_enclosing_quad() 
         *  need to be called before anything else
         */
        void create_enclosing_triangle(index_t v1, index_t v2, index_t v3);

        /**
         * \brief Creates the combinatorics for a first large enclosing
         *  quad
         * \param[in] v1 , v2 , v3 , v4 the four vertices of the quad,
         *  in 0,1,2,3
         * \details create_enclosing_triangle() or create_enclosing_quad() 
         *  need to be called before anything else
         */
        void create_enclosing_quad(
            index_t v1, index_t v2, index_t v3, index_t v4
        );

        /**
         * \brief Sets a triangle flag 
         * \param[in] t the triangle
         * \param[in] flag the flag, in 0..7
         */
        void Tset_flag(index_t t, index_t flag) {
            geo_debug_assert(t < nT());
            geo_debug_assert(flag < 8);
            Tflags_[t] |= Numeric::uint8(1u << flag);
        }

        /**
         * \brief Resets a triangle flag 
         * \param[in] t the triangle
         * \param[in] flag the flag, in 0..7
         */
        void Treset_flag(index_t t, index_t flag) {
            geo_debug_assert(t < nT());
            geo_debug_assert(flag < 8);
            Tflags_[t] &= Numeric::uint8(~(1u << flag));
        }

        /**
         * \brief Tests a triangle flag 
         * \param[in] t the triangle
         * \param[in] flag the flag, in 0..7
         * \retval true if the flag is set
         * \retval false otherwise
         */
        bool Tflag_is_set(index_t t, index_t flag) {
            geo_debug_assert(t < nT());
            geo_debug_assert(flag < 8);
            return ((Tflags_[t] & (1u << flag)) != 0);
        }

        /**
         * \brief Constants for list_id
         */
        enum {
            DLIST_S_ID=0,
            DLIST_Q_ID=1,
            DLIST_N_ID=2,
            DLIST_NB=3
        };
        

        /**
         * \brief Constants for triangle flags
         */
        enum {T_MARKED_FLAG = DLIST_NB};

        /**
         * \brief Tests whether a triangle is in a DList
         * \param[in] t the triangle
         * \retval true if the triangle is in a list
         * \retval false otherwise
         */
        bool Tis_in_list(index_t t) const {
            return (
                (Tflags_[t] &
                 Numeric::uint8((1 << DLIST_NB)-1)
                ) != 0
            );
        }

        /**
         * \brief Doubly connected triangle list
         * \details DList is used to implement:
         *  - the stack S of triangles to flip in insert()
         *  - the queue Q of intersected edges in 
         *    detect_intersected_edges() and constrain_edges()
         *  - the list N of new edges in constrain_edges()
         *  Everything is stored in CDBase
         *  vectors Tnext_, Tprev_ and Tflags_. As
         *  a consequence, the same triangle can be only
         *  in a single DList at the same time. 
         */
        struct DList {
            /**
             * \brief Constructs an empty DList
             * \param[in] cdt a reference to the CDTBase2d
             * \param[in] list_id the DList id, in 0..DLIST_NB-1
             */
            DList(CDTBase2d& cdt, index_t list_id) :
                cdt_(cdt), list_id_(list_id),
                back_(index_t(-1)), front_(index_t(-1)) {
                geo_debug_assert(list_id < DLIST_NB);
            }

            /**
             * \brief Creates an uninitialized DList
             * \details One cannot do anything with an 
             *   uninitialized Dlist, except:
             *   - initializing it with DList::initialize()
             *   - testing its status with DList::initialized()
             *   - display it with DList::show()
             */
            DList(CDTBase2d& cdt) :
                cdt_(cdt), list_id_(index_t(-1)),
                back_(index_t(-1)), front_(index_t(-1)) {
            }

            /**
             * \brief Initializes a list
             * \param[in] list_id the DList id, in 0..DLIST_NB-1
             */
            void initialize(index_t list_id) {
                geo_debug_assert(!initialized());
                geo_debug_assert(list_id < DLIST_NB);
                list_id_ = list_id;
            }

            /**
             * \brief Tests whether a DList is initialized
             */
            bool initialized() const {
                return (list_id_ != index_t(-1));
            }
            
            ~DList() {
                if(initialized()) {
                    clear();
                }
            }

            bool empty() const {
                geo_debug_assert(initialized());
                geo_debug_assert(
                    (back_==index_t(-1))==(front_==index_t(-1))
                );
                return (back_==index_t(-1));
            }

            bool contains(index_t t) const {
                geo_debug_assert(initialized());                
                return cdt_.Tflag_is_set(t, list_id_);
            }

            index_t front() const {
                geo_debug_assert(initialized());
                return front_;
            }
            
            index_t back() const {
                geo_debug_assert(initialized());
                return back_;
            }
            
            index_t next(index_t t) const {
                geo_debug_assert(initialized());
                geo_debug_assert(contains(t));
                return cdt_.Tnext_[t];
            }
            
            index_t prev(index_t t) const {
                geo_debug_assert(initialized());
                geo_debug_assert(contains(t));
                return cdt_.Tprev_[t];
            }

            void clear() {
                for(index_t t=front_; t!=index_t(-1); t = cdt_.Tnext_[t]) {
                    cdt_.Treset_flag(t,list_id_);
                }
                back_ = index_t(-1);
                front_ = index_t(-1);
            }

            index_t size() const {
                geo_debug_assert(initialized());
                index_t result = 0;
                for(index_t t=front(); t!=index_t(-1); t = next(t)) {
                    ++result;
                }
                return result;
            }
        
            void push_back(index_t t) {
                geo_debug_assert(initialized());
                geo_debug_assert(!cdt_.Tis_in_list(t));
                cdt_.Tset_flag(t,list_id_);
                if(empty()) {
                    back_ = t;
                    front_ = t;
                    cdt_.Tnext_[t] = index_t(-1);
                    cdt_.Tprev_[t] = index_t(-1);
                } else {
                    cdt_.Tnext_[t] = index_t(-1);
                    cdt_.Tnext_[back_] = t;
                    cdt_.Tprev_[t] = back_;
                    back_ = t;
                }
            }

            index_t pop_back() {
                geo_debug_assert(initialized());
                geo_debug_assert(!empty());
                index_t t = back_;
                back_ = cdt_.Tprev_[back_];
                if(back_ == index_t(-1)) {
                    geo_debug_assert(front_ == t);
                    front_ = index_t(-1);
                } else {
                    cdt_.Tnext_[back_] = index_t(-1);
                }
                geo_debug_assert(contains(t));
                cdt_.Treset_flag(t,list_id_);
                return t;
            }

            void push_front(index_t t) {
                geo_debug_assert(initialized());
                geo_debug_assert(!cdt_.Tis_in_list(t));
                cdt_.Tset_flag(t,list_id_);
                if(empty()) {
                    back_ = t;
                    front_ = t;
                    cdt_.Tnext_[t] = index_t(-1);
                    cdt_.Tprev_[t] = index_t(-1);
                } else {
                    cdt_.Tprev_[t] = index_t(-1);
                    cdt_.Tprev_[front_] = t;
                    cdt_.Tnext_[t] = front_;
                    front_ = t;
                }
            }

            index_t pop_front() {
                geo_debug_assert(initialized());
                geo_debug_assert(!empty());
                index_t t = front_;
                front_ = cdt_.Tnext_[front_];
                if(front_ == index_t(-1)) {
                    geo_debug_assert(back_ == t);
                    back_ = index_t(-1);
                } else {
                    cdt_.Tprev_[front_] = index_t(-1);
                }
                geo_debug_assert(contains(t));
                cdt_.Treset_flag(t,list_id_);
                return t;
            }

            void remove(index_t t) {
                geo_debug_assert(initialized());
                if(t == front_) {
                    pop_front();
                } else if(t == back_) {
                    pop_back();
                } else {
                    geo_debug_assert(contains(t));
                    index_t t_prev = cdt_.Tprev_[t];
                    index_t t_next = cdt_.Tnext_[t];
                    cdt_.Tprev_[t_next] = t_prev;
                    cdt_.Tnext_[t_prev] = t_next;
                    cdt_.Treset_flag(t,list_id_);
                }
            }

            void show(std::ostream& out = std::cerr) const {
                switch(list_id_) {
                case DLIST_S_ID:
                    out << "S";
                    break;
                case DLIST_Q_ID:
                    out << "Q";
                    break;
                case DLIST_N_ID:
                    out << "N";
                    break;
                case index_t(-1):
                    out << "<uninitialized list>";
                    break;
                default:
                    out << "<unknown list id:" << list_id_ << ">";
                    break;
                }
                out << "=";
                for(index_t t=front(); t!=index_t(-1); t = next(t)) {
                    out << t << ";";
                }
                out << std::endl;
            }
            
        private:
            CDTBase2d& cdt_;
            index_t list_id_;
            index_t back_;
            index_t front_;
        };

        /**
         * \brief Inserts a vertex in an edge
         * \param[in] v the vertex to be inserted
         * \param[in] t a triangle incident to the edge
         * \param[in] le the local index of the edge in \p t
         * \param[out] S DList of created triangles, ignored if uninitialized
         */
        void insert_vertex_in_edge(index_t v, index_t t, index_t le, DList& S);

        /**
         * \brief Inserts a vertex in an edge
         * \param[in] v the vertex to be inserted
         * \param[in] t a triangle incident to the edge
         * \param[in] le the local index of the edge in \p t
         */
        void insert_vertex_in_edge(index_t v, index_t t, index_t le) {
            DList S(*this);
            insert_vertex_in_edge(v,t,le,S);
        }

        /**
         * \brief Inserts a vertex in a triangle
         * \param[in] v the vertex to be inserted
         * \param[in] t the triangle
         * \param[out] S optional DList of created triangles
         */
        void insert_vertex_in_triangle(index_t v, index_t t, DList& S);
        
        /**
         * \brief Finds the edges intersected by a constraint
         * \param[in] i , j the two vertices of the constraint
         * \param[out] Q for each intersected edge, a triangle t
         *  will be pushed-back to Q, such that vT(t,1) and
         *  vT(t,2) are the extremities of the intersected edge.
         *  In addition, each triangle t is marked.
         * \details If a vertex k that is exactly on the constraint
         *  is found, then traversal stops there and k is returned.
         *  One can find the remaining intersections by continuing
         *  to call the function with (k,j) until \p j is returned.
         * \return the first vertex on [i,j] encountered when
         *  traversing the segment [i,j]. 
         */
        index_t find_intersected_edges(index_t i, index_t j, DList& Q);

        /**
         * \brief Used by find_intersected_edges()
         */
        void walk_constraint_v(CDT2d_ConstraintWalker& W);

        /**
         * \brief Used by find_intersected_edges()
         */
        void walk_constraint_t(CDT2d_ConstraintWalker& W, DList& Q);
        
        /**
         * \brief Constrains an edge by iteratively flipping
         *  the intersected edges.
         * \param[in] i , j the extremities of the edge
         * \param[in] Q the list of intersected edges, computed by
         *  find_intersected_edges()
         * \param[out] N optional  DList with the new edges 
         *  that need to be re-Delaunized by find_intersected_edges(), 
         *  ignored if uninitialized
         */
        void constrain_edges(index_t i, index_t j, DList& Q, DList& N);

        /**
         * \brief Restores Delaunay condition starting from the
         *  triangles incident to a given vertex.
         * \param[in] v the vertex
         * \param[in] S a stack of triangles, initialized with 
         *  the triangles incident to the vertex. Each triangle t
         *  is Trot()-ed in such a way that the vertex v
         *  corresponds to Vt(t,0)
         * \details Each time a triangle edge is swapped, the
         *  two new neighbors are recursively examined.
         */
        void Delaunayize_vertex_neighbors(index_t v, DList& S);
        
        /**
         * \brief Restores Delaunay condition for a set of
         *  edges after inserting a constrained edge
         * \param[in] N the edges for which Delaunay condition
         *  should be restored.
         */
        void Delaunayize_new_edges(DList& N);

        
        /**
         * \brief Sets all the combinatorial information
         *  of a triangle and edge flags
         * \param[in] t the triangle
         * \param[in] v1 , v2 , v3 the three vertices 
         * \param[in] adj1 , adj2 , adj3 the three triangles
         *  adjacent to \p t
         * \param[in] e1cnstr , e2cnstr , e3cnstr optional
         *  edge constraints
         */
        void Tset(
            index_t t,
            index_t v1,   index_t v2,   index_t v3,
            index_t adj1, index_t adj2, index_t adj3,
            index_t e1cnstr = index_t(-1),
            index_t e2cnstr = index_t(-1),
            index_t e3cnstr = index_t(-1)            
        ) {
            geo_debug_assert(t < nT());
            geo_debug_assert(v1 < nv());
            geo_debug_assert(v2 < nv());
            geo_debug_assert(v3 < nv());                        
            geo_debug_assert(adj1 < nT() || adj1 == index_t(-1));
            geo_debug_assert(adj2 < nT() || adj2 == index_t(-1));
            geo_debug_assert(adj3 < nT() || adj3 == index_t(-1));
            geo_debug_assert(v1 != v2);
            geo_debug_assert(v2 != v3);
            geo_debug_assert(v3 != v1);            
            geo_debug_assert(adj1 != adj2 || adj1 == index_t(-1));
            geo_debug_assert(adj2 != adj3 || adj2 == index_t(-1));
            geo_debug_assert(adj3 != adj1 || adj3 == index_t(-1));            
            T_[3*t  ]    = v1;
            T_[3*t+1]    = v2;
            T_[3*t+2]    = v3;                        
            Tadj_[3*t  ] = adj1;
            Tadj_[3*t+1] = adj2;
            Tadj_[3*t+2] = adj3;
            Tecnstr_[3*t]   = e1cnstr;
            Tecnstr_[3*t+1] = e2cnstr;
            Tecnstr_[3*t+2] = e3cnstr;
            v2T_[v1] = t;
            v2T_[v2] = t;
            v2T_[v3] = t;
        }

        /**
         * \brief Rotates indices in triangle t in such a way
         *  that a given vertex becomes vertex 0
         * \details On exit, vertex \p lv of \p t becomes vertex 0
         * \param[in] t a triangle index
         * \param[in] lv local vertex index in 0,1,2
         */
        void Trot(index_t t, index_t lv) {
            geo_debug_assert(t < nT());
            geo_debug_assert(lv < 3);
            if(lv != 0) {
                index_t i = 3*t+lv;
                index_t j = 3*t+((lv+1)%3);
                index_t k = 3*t+((lv+2)%3);
                Tset(
                    t,
                    T_[i], T_[j], T_[k],
                    Tadj_[i], Tadj_[j], Tadj_[k],
                    Tecnstr_[i], Tecnstr_[j], Tecnstr_[k]
                );
            }
        }

        /**
         * \brief Swaps an edge.
         * \details Swaps edge 0 of \p t1.
         *    Vertex 0 of \p t1 is vertex 0 of
         *    the two new triangles.
         * \param[in] t1 a triangle index. Its edge
         *    opposite to vertex 0 is swapped
         * \param[in] swap_t1_t2 if set, swap which triangle will be
         *    t1 and which triangle will be Tadj(t1,0) in the
         *    new pair of triange (needed for two configurations
         *    of the optimized constraint enforcement algorithm).
         */
        void swap_edge(index_t t1, bool swap_t1_t2=false);
    
        /**
         * \brief Sets a triangle adjacency relation
         * \param[in] t a triangle
         * \param[in] le local edge index, in 0,1,2
         * \param[in] adj the triangle adjacent to \p t 
         *  accross \p le
         */
        void Tadj_set(index_t t, index_t le, index_t adj) {
            geo_debug_assert(t < nT());
            geo_debug_assert(adj < nT());
            geo_debug_assert(le < 3);
            Tadj_[3*t+le] = adj;
        }

        /**
         * \brief After having changed connections from triangle
         *  to a neighbor, creates connections from neighbor
         *  to triangle.
         * \details edge flags are copied from the neighbor to \p t1.
         *  If there is no triangle accross \p le1, then
         *  nothing is done
         * \param[in] t1 a triangle
         * \param[in] le1 a local edge of \p t1, in 0,1,2
         * \param[in] prev_t2_adj_e2 the triangle adjacent to t2 that
         *  \p t1 will replace, where t2 = Tadj(t1,le1)
         */
        void Tadj_back_connect(
            index_t t1, index_t le1, index_t prev_t2_adj_e2
        ) {
            geo_debug_assert(t1 < nT());
            geo_debug_assert(le1 < 3);
            index_t t2 = Tadj(t1,le1);
            if(t2 == index_t(-1)) {
                return;
            }
            index_t le2 = Tadj_find(t2,prev_t2_adj_e2);
            Tadj_set(t2,le2,t1);
            Tset_edge_cnstr(t1,le1,Tedge_cnstr(t2,le2));
        }
        
        /**
         * \brief Creates a new triangle
         * \return the index of the new triange
         */
        index_t Tnew() {
            index_t t = nT();
            index_t nc = (t+1)*3; // new number of corners
            T_.resize(nc, index_t(-1));
            Tadj_.resize(nc, index_t(-1));
            Tecnstr_.resize(nc, index_t(-1));
            Tflags_.resize(t+1,0);
            Tnext_.resize(t+1,index_t(-1));
            Tprev_.resize(t+1,index_t(-1));
            return t;
        }

        /**
         * \brief Gets the constraint associated with an edge
         * \param[in] t a triangle
         * \param[in] le local edge index, in 0,1,2
         * \return the constraint associated with this edge, or
         *  index_t(-1) if there is no such constraint.
         */
        index_t Tedge_cnstr(index_t t, index_t le) const {
            geo_debug_assert(t < nT());
            geo_debug_assert(le < 3);
            return Tecnstr_[3*t+le];
        }
        
        /**
         * \brief Sets an edge as constrained
         * \param[in] t a triangle
         * \param[in] le local edge index, in 0,1,2
         * \param[in] cnstr_id identifier of the constrained edge
         */
        void Tset_edge_cnstr(
            index_t t, index_t le, index_t cnstr_id 
        ) {
            geo_debug_assert(t < nT());
            geo_debug_assert(le < 3);
            Tecnstr_[3*t+le] = cnstr_id;
        }

        /**
         * \brief Sets an edge as constrained in triangle and in neighbor
         * \param[in] t a triangle
         * \param[in] le local edge index, in 0,1,2
         * \param[in] cnstr_id identifier of the constrained edge
         */
        void Tset_edge_cnstr_with_neighbor(
            index_t t, index_t le, index_t cnstr_id 
        ) {
            geo_debug_assert(t < nT());
            geo_debug_assert(le < 3);
            Tset_edge_cnstr(t, le, cnstr_id);
            index_t t2 = Tadj(t,le);
            if(t2 != index_t(-1)) {
                index_t le2 = Tadj_find(t2,t);
                Tset_edge_cnstr(t2,le2,cnstr_id);
            }
        }
        
        /**
         * \brief Tests whether an edge is constrained
         * \param[in] t a triangle
         * \param[in] le local edge index, in 0,1,2
         * \retval true if the edge is constrained
         * \retval false otherwise
         */
        bool Tedge_is_constrained(index_t t, index_t le) const {
            return (Tedge_cnstr(t,le) != index_t(-1));
        }

        /**
         * \brief Calls a user-defined function for each triangle 
         * around a vertex
         * \param[in] v the vertex
         * \param[in] doit the function, that takes as argument the 
         *  current triangle t and the local index lv of \p v in t. 
         *  The function returns true if iteration is finished and can be 
         *  exited, false otherwise.
         */
        void for_each_T_around_v(
            index_t v, std::function<bool(index_t t, index_t lv)> doit
        ) {
            index_t t = vT(v);
            index_t lv = index_t(-1);
            do {
                lv = Tv_find(t,v);
                if(doit(t,lv)) {
                    return;
                }
                t = Tadj(t, (lv+1)%3);
            } while(t != vT(v) && t != index_t(-1));
            
            // We are done, this was an interior vertex
            if(t != index_t(-1)) {
                return;
            }
            
            // It was a vertex on the border, so we need
            // to traverse the triangle fan in the other
            // direction until we reach the border again
            t = vT(v);
            lv = Tv_find(t,v);
            t = Tadj(t, (lv+2)%3);
            while(t != index_t(-1)) {
                lv = Tv_find(t,v);
                if(doit(t,lv)) {
                    return;
                }
                t = Tadj(t, (lv+2)%3);
            }
        }

        
        /**
         * \brief Locates a vertex
         * \param[in] v the vertex index
         * \param[in] hint an optional triangle, not too far away from the
         *  point to be inserted
         * \param[out] orient a pointer to the three orientations in the 
         *  triangle. If one of them is zero, the point is on an edge, and
         *  if two of them are zero, it is on a vertex.
         * \return a triangle that contains \p v
         */
        index_t locate(
            index_t v, index_t hint = index_t(-1), Sign* orient = nullptr
        ) const;
        
        /**
         * \brief Tests whether triange t and its neighbor accross edge 0 form 
         *  a strictly convex quad
         * \retval true if triange \p t and its neighbor accross edge 0 form
         *  a strictly convex quad
         * \retval false otherwise
         */
        bool is_convex_quad(index_t t) const;

        /**
         * \brief Orientation predicate
         * \param[in] i , j , k three vertices
         * \return the sign of det(pj-pi,pk-pi)
         */
        virtual Sign orient2d(index_t i,index_t j,index_t k) const=0;

        /**
         * \brief Incircle predicate
         * \param[in] i , j , k the three vertices of a triangle
         * \param[in] l another vertex
         * \retval POSITIVE if \p l is inside the circumscribed circle of
         *  the triangle
         * \retval ZERO if \p l is on the circumscribed circle of
         *  the triangle
         * \retval NEGATIVE if \p l is outside the circumscribed circle of
         *  the triangle
         */
        virtual Sign incircle(index_t i,index_t j,index_t k,index_t l) const=0;

        /**
         * \brief Given two segments that have an intersection, create the
         *  intersection
         * \details The intersection is given both as the indices of segment
         *  extremities (i,j) and (k,l), that one can use to retreive the 
         *  points in derived classes, and constraint indices E1 and E2, that
         *  derived classes may use to retreive symbolic information attached
         *  to the constraint
         * \param[in] E1 the index of the first edge, corresponding to the
         *  value of ncnstr() when insert_constraint() was called for
         *  that edge
         * \param[in] i , j the vertices of the first segment
         * \param[in] E2 the index of the second edge, corresponding to the
         *  value of ncnstr() when insert_constraint() was called for
         *  that edge
         * \param[in] k , l the vertices of the second segment
         * \return the index of a newly created vertex that corresponds to
         *  the intersection between [\p i , \p j] and [\p k , \p l]
         */
        virtual index_t create_intersection(
            index_t E1, index_t i, index_t j,
            index_t E2, index_t k, index_t l
        ) = 0;

        /**
         * \brief Finds the index of an integer in an array of three integers.
         * \param[in] T a const pointer to an array of three integers
         * \param[in] v the integer to retrieve in \p T
         * \return the index (0,1 or 2) of \p v in \p T
         * \pre The three entries of \p T are different and one of them is
         *  equal to \p v.
         */
        static inline index_t find_3(const index_t* T, index_t v) {
            // The following expression is 10% faster than using
            // if() statements. This uses the C++ norm, that 
            // ensures that the 'true' boolean value converted to 
            // an int is always 1. With most compilers, this avoids 
            // generating branching instructions.
            // Thank to Laurent Alonso for this idea.
            index_t result = index_t( (T[1] == v) | ((T[2] == v) * 2) );
            // Sanity check, important if it was T[0], not explicitly
            // tested (detects input that does not meet the precondition).
            geo_debug_assert(T[result] == v);
            return result; 
        }
        
        /******************** Debugging ************************************/

        /**
         * \brief Consistency check for a triangle
         * \details aborts if inconsistency is detected
         * \param[in] t the triangle to be tested
         */
        void Tcheck(index_t t) const {
            if(t == index_t(-1)) {
                return;
            }
            for(index_t e=0; e<3; ++e) {
                geo_assert(Tv(t,e) != Tv(t,(e+1)%3));
                if(Tadj(t,e) == index_t(-1)) {
                    continue;
                }
                geo_assert(Tadj(t,e) != Tadj(t,(e+1)%3));
                index_t t2 = Tadj(t,e);
                index_t e2 = Tadj_find(t2,t);
                geo_assert(Tadj(t2,e2) == t);
            }
        }

        /**
         * \brief Consistency check for a triangle in debug mode,
         *  ignored in release mode
         * \details aborts if inconsistency is detected
         * \param[in] t the triangle to be tested
         */
        void debug_Tcheck(index_t t) const {
#ifdef GEO_DEBUG
            Tcheck(t);
#else
            geo_argused(t);
#endif            
        }
        
        /**
         * \brief Consistency combinatorial check for all the triangles
         * \details aborts if inconsistency is detected
         */        
        void check_combinatorics() const {
            for(index_t t=0; t<nT(); ++t) {
                Tcheck(t);
            }
        }

        /**
         * \brief Consistency combinatorial check for all the triangles
         *  in debug mode, ignored in release mode
         * \details aborts if inconsistency is detected
         */        
        void debug_check_combinatorics() const {
#ifdef GEO_DEBUG
            check_combinatorics();
#endif            
        }

        /**
         * \brief Consistency geometrical check for all the triangles
         * \details aborts if inconsistency is detected
         */        
        void check_geometry() const {
            for(index_t t=0; t<nT(); ++t) {
                for(index_t le=0; le<3; ++le) {
                    geo_assert(Tedge_is_Delaunay(t,le));
                }
            }
        }

        /**
         * \brief Consistency geometrical check for all the triangles
         *  in debug mode, ignored in release mode
         * \details aborts if inconsistency is detected
         */        
        void debug_check_geometry() const {
#ifdef GEO_DEBUG
            check_geometry();
#endif            
        }


    public:
        /**
         * \brief Checks both combinatorics and geometry,
         *  aborts on unconsistency
         */
        void check_consistency() const {
            check_combinatorics();
            if(delaunay_) {
                check_geometry();
            }
        }

    protected:
        /**
         * \brief Checks both combinatorics and geometry
         *  in debug mode, ignored in release mode,
         *  aborts on unconsistency
         */
        void debug_check_consistency() const {
            debug_check_combinatorics();
            if(delaunay_) {            
                debug_check_geometry();
            }
        }
        
        /**
         * \brief Tests whether two segments have a frank intersection
         * \param[in] u1 , u2 the two extremities of the first segment
         * \param[in] v1 , v2 the two extremities of the second segment
         * \retval true if \p u1 , \p u2 has a frank intersection with
         *  \p v1 , \p v2
         * \retval false otherwise
         */
        bool segment_segment_intersect(
            index_t u1, index_t u2, index_t v1, index_t v2
        ) const {
            if(orient2d(u1,u2,v1)*orient2d(u1,u2,v2) > 0) {
                return false;
            }
            return (orient2d(v1,v2,u1)*orient2d(v1,v2,u2) < 0);
        }
        
        /**
         * \brief Tests whether an edge triangle and a segment have a frank
         *  intersection
         * \param[in] v1 , v2 the two extremities of the segment
         * \param[in] t a triangle
         * \param[in] le local edge index in 0,1,2
         * \retval true if edge \p le of \p t has a frank intersection with edge
         *  \p v1 , \p v2
         * \retval false otherwise
         */
        bool segment_edge_intersect(
            index_t v1, index_t v2, index_t t, index_t le
        ) const {
            index_t u1 = Tv(t,(le + 1)%3);
            index_t u2 = Tv(t,(le + 2)%3);
            return segment_segment_intersect(u1,u2,v1,v2);
        }

        /**
         * \brief Checks that the edges stored in a DList exactly correspond
         *  to all edge intersections between a segment and the triangle edges
         * \param[in] v1 , v2 the two vertices of the constrained segment
         * \param[in] Q a list of triangle. For each triangle in Q, edge 0 
         *  is supposed to have an intersection with \p v1 , \p v2
         */
        void check_edge_intersections(
            index_t v1, index_t v2, const DList& Q
        );

        typedef std::pair<index_t, index_t> Edge;
        
        /**
         * \brief Gets a triangle incident a a given edge
         * \param[in] E the edge
         * \return a triangle with E as its edge 0
         */
        index_t eT(Edge E) {
            index_t v1 = E.first;
            index_t v2 = E.second;
            index_t result = index_t(-1);
            for_each_T_around_v(
                v1, [&](index_t t, index_t lv)->bool {
                    if(Tv(t, (lv+1)%3) == v2) {
                        if(Tv(t, (lv+2)%3) != v1) {
                            Trot(t, (lv+2)%3);
                        }
                        result = t;
                        return true;
                    } else if(Tv(t, (lv+1)%3) == v1) {
                        if(Tv(t, (lv+2)%3) != v2) {
                            Trot(t, (lv+2)%3);
                        }
                        result = t;                    
                        return true;
                    }
                    return false;
                }
            );
            geo_debug_assert(result != index_t(-1));
            geo_debug_assert(
                (Tv(result,1) == v1 && Tv(result,2) == v2) ||
                (Tv(result,1) == v2 && Tv(result,2) == v1) 
            );
            return result;
        }

        /**
         * \brief Simpler version of locate() kept for reference
         * \see locate()
         */
        index_t locate_naive(
            index_t v, index_t hint = index_t(-1), Sign* orient = nullptr
        ) const;
        
        /**
         * \brief Simpler version of constrain_edges() kept for reference
         * \see constrain_edges()
         */
        void constrain_edges_naive(
            index_t i, index_t j, DList& Q, vector<Edge>& N
        );

        /**
         * \brief Simpler version of Delaunayize_new_edges() that uses a vector
         *  instead of a DList, kept for reference
         */
        void Delaunayize_new_edges_naive(vector<Edge>& N);
        
    protected:
        index_t nv_;
        index_t ncnstr_;
        vector<index_t> T_;       /**< triangles vertices array              */
        vector<index_t> Tadj_;    /**< triangles adjacency array             */
        vector<index_t> v2T_;     /**< vertex to triangle back pointer       */
        vector<uint8_t> Tflags_;  /**< triangle flags                        */
        vector<index_t> Tecnstr_; /**< triangle edge constraint              */
        vector<index_t> Tnext_;   /**< doubly connected triangle list        */
        vector<index_t> Tprev_;   /**< doubly connected triangle list        */
        bool delaunay_;           /**< if set, compute a CDT, else just a CT */
        Sign orient_012_;         /**< global triangles orientation          */
    };

    /*****************************************************************/
    
    /**
     * \brief Constrained Delaunay triangulation
     * \details
     *   Example:
     *   \code
     *    CDT cdt;
     *    vec2 p1(.,.), p2(.,.), p3(.,.);
     *    cdt.create_enclosing_triangle(p1,p2,p3); 
     *         // or create_enclosing_quad() or create_enclosing_rect()
     *    // insert points
     *    for(...) {
     *      vec2 p(.,.);
     *      index_t v = cdt.insert(p);
     *      ...
     *    }
     *    // insert constraints
     *    for(...) {
     *       index_t v1=..., v2=...;
     *       cdt.insert_constraint(v1,v2);   
     *    }
     *    // get triangles
     *    for(index_t t=0; t<cdt.nT(); ++t) {
     *       index_t v1 = cdt.Tv(t,0); 
     *       index_t v2 = cdt.Tv(t,1); 
     *       index_t v3 = cdt.Tv(t,2); 
     *       ... do something with v1,v2,v3
     *    }
     *   \endcode   
     *   If some constraints are intersecting, new vertices are generated. They
     *   can be accessed using the function vec2 CDT::point(index_t v). Vertices
     *   coming from an intersection are between indices nv1 and CDT::nv(), 
     *   where nv1 is the value of CDT::nv() before inserting the constraints
     *   (nv1 corresponds to the number of times CDT::insert() was called plus
     *   the number of points in the enclosing polygon).
     *
     *   If you want only a constrained triangulation (not Delaunay), 
     *   you can call CDT::set_Delaunay(false) before inserting the points.
     *
     *   If you have many points to insert, you can use the function:
     *   \code
     *     void CDT::insert(index_t nb_points, const double* points);
     *   \endcode
     *   It is much much faster than inserting the points one by one. Internally
     *   it uses Amenta et.al's BRIO method (multi-resolution spatial sort).
     */
    class GEOGRAM_API CDT2d: public CDTBase2d {
    public:

        CDT2d();
        
        ~CDT2d() override;
        
        /**
         * \copydoc CDTBase2d::clear()
         */
        void clear() override;

        /**
         * \brief Creates a first large enclosing triangle
         * \param[in] p1 , p2 , p3 the three vertices of the first triangle
         * \details create_enclosing_triangle(), create_enclosing_rectangle() 
         *  or create_enclosing_quad()  need to be called before anything else
         */
        void create_enclosing_triangle(
            const vec2& p1, const vec2& p2, const vec2& p3
        );

        /**
         * \brief Creates a first large enclosing quad
         * \param[in] p1 , p2 , p3 , p4 the four vertices of the quad
         * \details The quad needs to be convex. 
         * create_enclosing_triangle(), create_enclosing_rectangle() 
         *  or create_enclosing_quad()  need to be called before anything else
         */
        void create_enclosing_quad(
            const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4
        );


        /**
         * \brief Creates a first large enclosing rectangle
         * \param[in] x1 , y1 , x2 , y2 rectangle bounds
         * \details create_enclosing_triangle(), create_enclosing_rectangle() 
         *  or create_enclosing_quad() need to be called before anything else
         */
        void create_enclosing_rectangle(
            double x1, double y1, double x2, double y2
        ) {
            create_enclosing_quad(
                vec2(x1,y1),
                vec2(x2,y1),
                vec2(x2,y2),
                vec2(x1,y2)
            );
        }
        
        /**
         * \brief Inserts a point
         * \param[in] p the point to be inserted
         * \param[in] hint a triangle not too far away from the point to
         *  be inserted
         * \return the index of the created point. Duplicated points are
         *  detected (and them the index of the existing point is returned)
         */
        index_t insert(const vec2& p, index_t hint = index_t(-1)) {
            debug_check_consistency();            
            point_.push_back(p);
            index_t v = CDTBase2d::insert(point_.size()-1, hint);
            // If inserted point already existed in
            // triangulation, then nv() did not increase
            if(point_.size() > nv()) {
                point_.pop_back();
            }
            debug_check_consistency();                        
            return v;
        }

        /**
         * \brief Batch-inserts a set of point
         * \details In general, it is much faster than calling 
         *  insert() multiple times. Internally it uses a spatial
         *  sort (Amenta et.al's BRIO method).
         * \param[in] points a contiguous array of all point
         *  coordinates
         * \param[in] nb_points number of points
         */
        void insert(index_t nb_points, const double* points);
        
        /**
         * \copydoc CDTBase2d::save()
         */
        void save(const std::string& filename) const override;

        /**
         * \brief Gets a point by index
         * \param[in] v point index
         * \return the point at index \p v
         */
        const vec2 point(index_t v) const {
            geo_debug_assert(v < nv());
            return point_[v];
        }

    protected:
        /**
         * \copydoc CDTBase2d::orient_2d()
         */
        Sign orient2d(index_t i, index_t j, index_t k) const override;

        /**
         * \copydoc CDTBase2d::incircle()
         */
        Sign incircle(index_t i,index_t j,index_t k,index_t l) const override;

        /**
         * \copydoc CDTBase2d::create_intersection()
         */
        index_t create_intersection(
            index_t E1, index_t i, index_t j,
            index_t E2, index_t k, index_t l
        ) override;
        
    protected:
        vector<vec2> point_;
    };

    /*****************************************************************/    
}

#endif
