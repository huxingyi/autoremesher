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

#ifndef GEOGRAM_BASIC_RANGE
#define GEOGRAM_BASIC_RANGE

#include <geogram/basic/numeric.h>

/**
 * \file geogram/basic/range.h
 * \brief C++-20 like helpers for manipulating ranges of integers.
 * \note transform() not implemented yet, I need c++14 for that 
 *  (auto return type).
 */

namespace GEO {

    /**
     * \brief Wraps an integer for range-based for construct.
     * \details Not really an iterator, rather a pseudo-index.
     */
    class no_iterator {
      public:
        no_iterator(index_t val) : val_(val) {
	}

	void operator++() {
	    ++val_;
	}
   
	bool operator==(const no_iterator& rhs) {
	    return val_ == rhs.val_;
	}

	bool operator!=(const no_iterator& rhs) {
	    return val_ != rhs.val_;
	}

	bool operator<(const no_iterator& rhs) {
	    return val_ < rhs.val_;
	}

	index_t operator*() const  {
	    return val_;
	}

      private:
	index_t val_;
    };

    /**
     * \brief A generic range bounded by two "non-iterators".
     */
    template <class NO_ITERATOR> class range {
      public:
	typedef NO_ITERATOR no_iterator_t;
    
        range(NO_ITERATOR b, NO_ITERATOR e) : begin_(b), end_(e) {
	}

	NO_ITERATOR begin() const {
	    return begin_;
	}

	NO_ITERATOR end() const {
	    return end_;
	}
    
      private:
	NO_ITERATOR begin_;
	NO_ITERATOR end_;
    };
}

#endif
