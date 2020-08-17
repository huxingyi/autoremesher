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

#include <geogram/mesh/mesh_compare.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_topology.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/basic/logger.h>

namespace {

    using namespace GEO;

    /**
     * \brief Computes the number of facet borders
     */
    index_t mesh_nb_facet_borders(const Mesh& M) {
        index_t nb_borders = 0;
        for(index_t i: M.facet_corners) {
            if(M.facet_corners.adjacent_facet(i) == NO_FACET) {
                nb_borders++;
            }
        }
        return nb_borders;
    }

    /**
     * \brief Computes the number of tet borders
     */
    index_t mesh_nb_cell_borders(const Mesh& M) {
        index_t nb_borders = 0;
        for(index_t c: M.cells) {
            for(index_t lf = 0; lf < M.cells.nb_facets(c); ++lf) {
                if(M.cells.adjacent(c, lf) == NO_CELL) {
                    nb_borders++;
                }
            }
        }
        return nb_borders;
    }
}

namespace GEO {

    MeshCompareFlags mesh_compare(
        const Mesh& M1, const Mesh& M2,
        MeshCompareFlags flags,
        double tolerance,
        bool verbose
    ) {
        if(verbose) {
            M1.show_stats("Mesh1");
            M2.show_stats("Mesh2");
        }

        Logger::out("Compare")
            << "Using floating point tolerance = " << tolerance
            << std::endl;

        int status = MESH_COMPARE_OK;

        if(
            (flags & MESH_COMPARE_DIMENSIONS) &&
            M1.vertices.dimension() != M2.vertices.dimension()
        ) {
            if(verbose) {
                Logger::err("Compare")
                    << "Dimensions differ"
                    << std::endl;
            }
            status |= MESH_COMPARE_DIMENSIONS;
        }

        if(
            (flags & MESH_COMPARE_NB_VERTICES) &&
            M1.vertices.nb() != M2.vertices.nb()
        ) {
            if(verbose) {
                Logger::err("Compare")
                    << "Numbers of vertices differ"
                    << std::endl;
            }
            status |= MESH_COMPARE_NB_VERTICES;
        }

        if(
            (flags & MESH_COMPARE_NB_FACETS) &&
            M1.facets.nb() != M2.facets.nb()
        ) {
            if(verbose) {
                Logger::err("Compare")
                    << "Numbers of facets differ"
                    << std::endl;
            }
            status |= MESH_COMPARE_NB_FACETS;
        }

        if(
            (flags & MESH_COMPARE_NB_FACET_BORDERS) &&
            mesh_nb_facet_borders(M1) != mesh_nb_facet_borders(M2)
        ) {
            if(verbose) {
                Logger::err("Compare")
                    << "Numbers of facet borders differ"
                    << std::endl;
            }
            status |= MESH_COMPARE_NB_FACET_BORDERS;
        }

        if(flags & MESH_COMPARE_AREAS) {
            double M1_area = Geom::mesh_area(M1);
            double M2_area = Geom::mesh_area(M2);

            if(verbose) {
                Logger::out("Mesh1") << "area:" << M1_area << std::endl;
                Logger::out("Mesh2") << "area:" << M2_area << std::endl;
            }

            if(std::abs(M2_area - M1_area) > tolerance * M1_area) {
                if(verbose) {
                    Logger::err("Compare")
                        << "Areas differ"
                        << std::endl;
                }
                status |= MESH_COMPARE_AREAS;
            }
        }

        if(
            (flags & MESH_COMPARE_TOPOLOGY) &&
            !meshes_have_same_topology(M1, M2, verbose)
        ) {
            status |= MESH_COMPARE_TOPOLOGY;
        }

        if(
            (flags & MESH_COMPARE_NB_TETS) &&
            M1.cells.nb() != M2.cells.nb()
        ) {
            if(verbose) {
                Logger::err("Compare")
                    << "Numbers of cells differ"
                    << std::endl;
            }
            status |= MESH_COMPARE_NB_TETS;
        }

        if(flags & MESH_COMPARE_NB_TET_BORDERS) {
            index_t M1_nb_tet_borders = mesh_nb_cell_borders(M1);
            index_t M2_nb_tet_borders = mesh_nb_cell_borders(M2);

            if(verbose) {
                Logger::out("Mesh1")
                    << "nb_cell_borders:" << M1_nb_tet_borders
                    << std::endl;
                Logger::out("Mesh2")
                    << "nb_cell_borders:" << M2_nb_tet_borders
                    << std::endl;
            }

            if(M1_nb_tet_borders != M2_nb_tet_borders) {
                if(verbose) {
                    Logger::err("Compare")
                        << "Numbers of tet borders differ"
                        << std::endl;
                }
                status |= MESH_COMPARE_NB_TET_BORDERS;
            }
        }

        return MeshCompareFlags(status);
    }
}

