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

#ifndef GEOGRAM_VORONOI_INTEGRATION_SIMPLEX
#define GEOGRAM_VORONOI_INTEGRATION_SIMPLEX

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/counted.h>
#include <geogram/basic/smart_pointer.h>
#include <geogram/basic/geometry.h>

/**
 * \file geogram/voronoi/integration_simplex.h
 * \brief base classes for computing integrals over
 *  the cells of a restricted Voronoi diagram
 */

namespace GEOGen {
    class Vertex;
}

namespace GEO {

    class Mesh;

    namespace Process {
        class SpinLockArray;
    }

    /**
     * \brief Computes an objective function and its gradient
     *  over a restricted Voronoi diagram.
     * \details This function can be used by CentroidalVoronoiTesselation
     *  and its variants to optimize the placement of points by minimizing
     *  an objective function. Sub-classing this class makes it possible
     *  to define new objective functions.
     */
    class GEOGRAM_API IntegrationSimplex : public Counted {

    public:

        /**
         * \brief IntegrationSimplex destructor.
         */
        virtual ~IntegrationSimplex();
        
        /**
         * \brief Computes the contribution of a given integration
         *  simplex to the function and its gradient. An integration
         *  simplex is obtained as the intersection between a Voronoi
         *  cell and a triangle or tetrahedron of a background mesh.
         * \param[in] center_vertex_index index of the first vertex
         *  of the integration simplex, that corresponds to one of 
         *  the vertices of the Delaunay triangulation
         * \param[in] v0 second vertex of the integration simplex, in
         *  both geometric and symbolic forms
         * \param[in] v1 third vertex of the integration simplex, in
         *  both geometric and symbolic forms
         * \param[in] v2 fourth vertex of the integration simplex, in
         *  both geometric and symbolic forms
         * \param[in] t the triangle or tetrahedron of the background mesh
         * \param[in] t_adj the background mesh tetrahedron adjacent to this
         *  integration simplex accros (\p v0, \p v1, \p v2) or index_t(-1)
         *  if no such tetrahedron exists.
         * \param[in] v_adj if (\p v0, \p v1, \p v2) is supported by a bisector,
         *  the index of the other extremity of the bisector, else index_t(-1)
         */
         virtual double eval(
             index_t center_vertex_index,
             const GEOGen::Vertex& v0,
             const GEOGen::Vertex& v1,
             const GEOGen::Vertex& v2,
             index_t t,
             index_t t_adj = index_t(-1),
             index_t v_adj = index_t(-1)
         ) = 0;


        /**
         * \brief Sets the input points and the location where
         *  the computed gradient will be stored.
         * \details This function needs to be called once per evaluation
         *  of the objective function, before evaluating the 
         *  contribution of the simplices with eval().
         * \param[in] dimension number of coordinates of the points,
         *  or number of doubles between two consecutive points
         * \param[in] nb_points number of points
         * \param[in] points a const pointer to the 
         *   contiguous array of coordinates of the points
         * \param[out] g a pointer to the components 
         *   of the gradient of the objective function
         * \param[in] spinlocks a pointer to the spinlocks array to
         *  be used in multithreading mode, or nullptr in single-threaded
         *  mode
         */
         void set_points_and_gradient(
             coord_index_t dimension,
             index_t nb_points, 
             const double* points, 
             double* g,
             Process::SpinLockArray* spinlocks=nullptr
         ) {
             nb_points_ = nb_points;
             points_stride_ = index_t(dimension);
             points_ = points;
             g_ = g;
             spinlocks_ = spinlocks;
         }

         /**
          * \brief Tests whether this IntegrationSimplex is volumetric.
          * \details A volumetric IntegrationSimplex is meant to be computed
          *  over the Voronoi cells restricted to the tetrahedra of the mesh.
          *  A surfacic one is meant to be computed over the Voronoi cells 
          *  restricted to the facets of the mesh.
          * \retval true if this IntegrationSimplex is volumetric
          * \retval false otherwise (surfacic)
          */
         bool volumetric() const {
             return volumetric_;
         }

         /**
          * \brief Specifies whether the background 
          *  mesh has varying attributes used in the
          *  computation.
          * \details The RestrictedVoronoiDiagram class
          *  computes the intersection between a Voronoi
          *  diagram and a background mesh. If the triangles 
          *  or tetrahedra of this background mesh have a 
          *  property that varies on each triangle / tetrahedron,
          *  then the intersection between the Voronoi cells and
          *  each individual triangle / tetrahedron is computed.
          *  Default mode is constant, subclasses may override.
          *  \retval true if the background mesh has varying
          *   attributes
          *  \retval false otherwise
          */
         bool background_mesh_has_varying_attribute() const {
             return varying_background_;
         }

         /**
          * \brief Before starting computation, resets 
          *  thread local storage variables. 
          * \details RestrictedVoronoiDiagram can operate
          *  in multi-threading mode. Some derived classes
          *  may need to reset some thread local storage 
          *  variables before starting each thread. 
          */
         virtual void reset_thread_local_storage();
         
    protected:
        /**
         * \brief Constructs a new IntegrationSimplex.
         * \param[in] mesh the mesh
         * \param[in] volumetric true if volumetric, false if surfacic
         * \param[in] nb_frames number of frames, typically number of 
         *  elements of the background mesh
         * \param[in] nb_comp_per_frame number of components per frame,
         *   3 for 3-axis anisotropy, 1 for vector anisotropy.
         * \param[in] frames a const pointer to the array of 
         *   3*nb_frames*nb_comp_per_frame of frame coordinates.
         */
         IntegrationSimplex(
             const Mesh& mesh, 
             bool volumetric,
             index_t nb_frames,
             index_t nb_comp_per_frame,
             const double* frames
         );


         /**
          * \brief Gets a point by index.
          * \param[in] i index of the point
          * \return a const pointer to the coordinates of the point
          */
         const double* point(index_t i) const {
             geo_debug_assert(i < nb_points_);
             return points_ + i * points_stride_;
         }

         /**
          * \brief Gets a frame by index.
          * \param[in] i index of the frame
          * \return a const pointer to the nb_components_per_frame
          *   coordinates of the frame
          */
         const double* frame(index_t i) const {
             geo_debug_assert(i < nb_frames_);
             return frames_ + i * nb_comp_per_frame_;
         }

         
    protected:
        const Mesh& mesh_;
        bool volumetric_;
        index_t nb_points_;
        index_t points_stride_;
        const double* points_;
        double* g_;
        index_t nb_frames_;
        index_t nb_comp_per_frame_;
        const double* frames_;
        Process::SpinLockArray* spinlocks_;
        bool varying_background_;
    };   

    typedef SmartPointer<IntegrationSimplex> 
    IntegrationSimplex_var;

}

#endif
