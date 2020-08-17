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


#ifndef GEOGRAM_DELAUNAY_CAVITY
#define GEOGRAM_DELAUNAY_CAVITY

#include <geogram/basic/common.h>
#include <geogram/basic/memory.h>
#include <geogram/basic/numeric.h>
#include <string.h>

// Uncomment to display histogram of
// number of collisions per set() and
// get() operations.
// There is probably room for improvement
// in my hash function, but for large
// pointsets, more then 99% of queries are
// in the first slot (seems to be good enough).
//#define CAVITY_WITH_STATS
#ifdef CAVITY_WITH_STATS
#define CAVITY_STATS(x) x
#else
#define CAVITY_STATS(x)
#endif

namespace GEO {

    /**
     * \brief Represents the set of tetrahedra on the boundary
     *  of the cavity in a 3D Delaunay triangulation.
     */
    class Cavity {

      public:

	/**
	 * \brief Type used for local indices.
	 */
	typedef Numeric::uint8 local_index_t;
	
	/**
	 * \brief Cavity constructor.
	 */
	Cavity() {
	    clear();
#ifdef CAVITY_WITH_STATS	    
	    Memory::clear(stats_set_, sizeof(stats_set_));
	    Memory::clear(stats_get_, sizeof(stats_get_));
#endif	    
	}
	
	/**
	 * \brief Clears this cavity.
	 */
	void clear() {
	    nb_f_ = 0;
	    OK_ = true;
	    ::memset(h2t_, END_OF_LIST, sizeof(h2t_));
	}

	~Cavity() {
#ifdef CAVITY_WITH_STATS	    
	    for(index_t i=0; i<MAX_H; ++i) {
		std::cerr << i << ": get=" << stats_get_[i] << "   set=" << stats_set_[i] << std::endl;
	    }
#endif	    
	}
	
	/**
	 * \brief Tests whether this Cavity is valid.
	 * \retval true if this Cavity is valid.
	 * \retval false otherwise. A Cavity is not valid 
	 *  when there was overflow.
	 */
	bool OK() const {
	    return OK_;
	}
	
	/**
	 * \brief Inserts a new boundary facet in the structure.
	 * \param[in] tglobal global tetrahedron index
	 * \param[in] boundary_f index of the facet that is on the boundary
	 * \param[in] v0 , v1 , v2 the three vertices of the facet that
	 *  is on the boundary
	 */
	void new_facet(
	    index_t tglobal, index_t boundary_f,
	    signed_index_t v0, signed_index_t v1, signed_index_t v2
	) {
	    if(!OK_) {
		return;
	    }
	    
	    geo_debug_assert(v0 != v1);
	    geo_debug_assert(v1 != v2);
	    geo_debug_assert(v2 != v0);	    

	    local_index_t new_t = local_index_t(nb_f_);
	    
	    if(nb_f_ == MAX_F) {
		OK_ = false;
		return;
	    }
	    
	    set_vv2t(v0, v1, new_t);
	    set_vv2t(v1, v2, new_t);
	    set_vv2t(v2, v0, new_t);
	    
	    if(!OK_) {
		return;
	    }
	    
	    ++nb_f_;
	    tglobal_[new_t] = tglobal;
	    boundary_f_[new_t] = boundary_f;
	    f2v_[new_t][0] = v0;
	    f2v_[new_t][1] = v1;
	    f2v_[new_t][2] = v2;
	}

	/**
	 * \brief Gets the number of facets.
	 * \return the number of facets.
	 */
	index_t nb_facets() const {
	    return nb_f_;
	}

	/**
	 * \brief Gets the tetrahedron associated with a facet.
	 * \param[in] f the facet
	 * \return the tetrahedron associated with \p f.
	 */
	index_t facet_tet(index_t f) const {
	    geo_debug_assert(f < nb_facets());
	    return tglobal_[f];
	}

	/**
	 * \brief Sets the tetrahedron associated with a facet.
	 * \param[in] f the facet.
	 * \param[in] t the tetrahedron to be associated with \p f.
	 */
	void set_facet_tet(index_t f, index_t t) {
	    geo_debug_assert(f < nb_facets());
	    tglobal_[f] = t;
	}

	/**
	 * \brief Gets the local tetrahedron facet that corresponds
	 *  to a facet.
	 * \param[in] f the facet.
	 * \return the local index of the tetrahedron facet associated 
	 *  with \p f, in 0..3
	 */
	index_t facet_facet(index_t f) const {
	    geo_debug_assert(f < nb_facets());
	    return boundary_f_[f];
	}

	/**
	 * \brief Gets the vertex of a facet.
	 * \param[in] f a facet.
	 * \param[in] lv local index of the vertex, in 0..2.
	 * \return the global vertex index.
	 */
	signed_index_t facet_vertex(index_t f, index_t lv) const {
	    geo_debug_assert(f < nb_facets());
	    geo_debug_assert(lv < 3);
	    return f2v_[f][lv];
	}

	/**
	 * \brief Gets the neighbors of a facet.
	 * \param[in] f a facet
	 * \param[out] t0 , t1 , t2 the global tetrahedron
	 *  indices that correspond to the neighbors of \p f.
	 */
	void get_facet_neighbor_tets(
	    index_t f, index_t& t0, index_t& t1, index_t& t2
	) const {
	    signed_index_t v0 = f2v_[f][0];
	    signed_index_t v1 = f2v_[f][1];
	    signed_index_t v2 = f2v_[f][2];		
	    t0 = tglobal_[get_vv2t(v2,v1)];
	    t1 = tglobal_[get_vv2t(v0,v2)];
	    t2 = tglobal_[get_vv2t(v1,v0)];
	}
	
      private:
	static const index_t        MAX_H = 1024; 
	static const local_index_t  END_OF_LIST = 255;
	static const index_t        MAX_F = 128;

	/**
	 * \brief Computes the hash code associated with an oriented
	 *  edge.
	 * \param[in] v1 , v2 the global indices of the two vertices
	 * \return the hash code, in 0 .. MAX_H -1
	 */
	index_t hash(signed_index_t v1, signed_index_t v2) const {
	    return ((index_t(v1+1) ^ (419*index_t(v2+1))) % MAX_H);
	}

	/**
	 * \brief Sets the local facet associated with an oriented 
	 *  edge.
	 * \param[in] v1 , v2 the global indices of the two vertices
	 * \param[in] f the local face index.
	 */
	void set_vv2t(
	    signed_index_t v1, signed_index_t v2, local_index_t f
	) {
	    CAVITY_STATS(index_t cnt = 0;)
	    index_t h = hash(v1,v2);
	    index_t cur = h;
	    do {
		if(h2t_[cur] == END_OF_LIST) {
		    h2t_[cur] = f;
#ifdef GARGANTUA		    
		    h2v_[cur][0] = v1;
		    h2v_[cur][1] = v2;
#else		    
		    h2v_[cur] = (Numeric::uint64(v1+1) << 32) | Numeric::uint64(v2+1);
#endif		    
		    CAVITY_STATS(++stats_set_[cnt];)
		    return;
		}
		cur = (cur+1)%MAX_H;
		CAVITY_STATS(++cnt;)
	    } while(cur != h);
	    OK_ = false;
	}

	/**
	 * \brief gets the local facet associated with an oriented 
	 *  edge.
	 * \param[in] v1 , v2 the global indices of the two vertices
	 * \return the local facet index.
	 */
	local_index_t get_vv2t(signed_index_t v1, signed_index_t v2) const {
#ifndef GARGANTUA	    
	    Numeric::uint64 K = (Numeric::uint64(v1+1) << 32) | Numeric::uint64(v2+1);
#endif	    
	    CAVITY_STATS(index_t cnt = 0;)
	    index_t h = hash(v1,v2);
	    index_t cur = h;
	    do {
#ifdef GARGANTUA
		if((h2v_[cur][0] == v1) && (h2v_[cur][1] == v2)) {
#else
		if(h2v_[cur] == K) {
#endif		
	            CAVITY_STATS(++stats_get_[cnt];)
		    return h2t_[cur];
		}
		cur = (cur+1)%MAX_H;
		CAVITY_STATS(++cnt;)
	    } while(cur != h);
	    geo_assert_not_reached;
	}

	/** \brief Hash index to local facet id. */
	local_index_t  h2t_[MAX_H];

	/** \brief Hash index to global vertex id. */
#ifdef GARGANTUA	
	signed_index_t h2v_[MAX_H][2];
#else	
	Numeric::uint64 h2v_[MAX_H];
#endif
	
	/** \brief Number of facets. */
	index_t nb_f_;
	
	/** \brief Local facet index to tetrahedra index. */
	index_t tglobal_[MAX_F];

	/** \brief Local facet index to facet on border index. */
	index_t boundary_f_[MAX_F];

	/** \brief Local facet index to three global vertex indices. */
	signed_index_t f2v_[MAX_F][3];
	
	
	/** 
	 * \brief True if the structure is correct, false 
	 *  otherwise, if capacity was exceeded.
	 */
	bool OK_;

	CAVITY_STATS(mutable index_t stats_set_[MAX_H];)
	CAVITY_STATS(mutable index_t stats_get_[MAX_H];)
    };

}

#endif

