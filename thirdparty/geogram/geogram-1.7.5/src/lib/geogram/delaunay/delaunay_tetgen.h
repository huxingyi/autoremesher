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

#ifndef GEOGRAM_DELAUNAY_DELAUNAY_TETGEN
#define GEOGRAM_DELAUNAY_DELAUNAY_TETGEN

/**
 * \file geogram/delaunay/delaunay_tetgen.h
 * \brief Implementation of Delaunay in 3D using the tetgen library
 *  by Hang Si.
 */

#ifdef GEOGRAM_WITH_TETGEN

#include <geogram/basic/common.h>
#include <geogram/delaunay/delaunay.h>
#include <geogram/third_party/tetgen/tetgen.h>

namespace GEO {

    /**
     * \brief Implementation of Delaunay using Hang Si's tetgen library.
     */
    class GEOGRAM_API DelaunayTetgen : public Delaunay {
    public:
        /**
         * \brief Creates a new DelaunayTetgen.
         * \details DelaunayTetgen triangulations are only supported for
         * dimension 3. If a different dimension is specified in the
         * constructor, a InvalidDimension exception is thrown.
         * \param[in] dimension dimension of the triangulation
         * \throw InvalidDimension This exception is thrown if dimension is
         * different than 3.
         */
        DelaunayTetgen(coord_index_t dimension = 3);

	/**
	 * \copydoc Delaunay::set_vertices()
	 */
        virtual void set_vertices(
            index_t nb_vertices, const double* vertices
        );

	/**
	 * \copydoc Delaunay::supports_constraints()
	 */
        virtual bool supports_constraints() const;


	/**
	 * \copydoc Delaunay::region()
	 */
        virtual index_t region(index_t t) const;
       
    protected:

        /**
         * \brief Implementation of set_vertices() used when
         *  no constraint is defined.
         * \param[in] nb_vertices number of vertices
         * \param[in] vertices a const pointer to the 
         *  coordinates of the vertices, as a continuous
         *  array of doubles.
         */
        void set_vertices_unconstrained(
            index_t nb_vertices, const double* vertices
        );

        /**
         * \brief Implementation of set_vertices() used when
         *  constraints are defined.
         * \details The constraints are specified by 
         *  Delaunay::set_constraints().
         * \param[in] nb_vertices number of vertices
         * \param[in] vertices a const pointer to the 
         *  coordinates of the vertices, as a continuous
         *  array of doubles.
         */
        void set_vertices_constrained(
            index_t nb_vertices, const double* vertices
        );


        /**
         * \brief DelaunayTetGen destructor.
         */
        virtual ~DelaunayTetgen();

    protected:
        GEO_3rdParty::tetgenio tetgen_out_;
        GEO_3rdParty::tetgenio tetgen_in_;
        GEO_3rdParty::tetgenbehavior tetgen_args_;
    };
}

#endif

#endif

