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
 

#ifndef H_OGF_SHAPENET_ALGOS_POISSON_H
#define H_OGF_SHAPENET_ALGOS_POISSON_H

#include <geogram/basic/common.h>
#include <geogram/basic/assert.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/geometry.h>

// Note: This file is not part of Kahzdan's original Poisson reconstruction
// code.

namespace GEO {

    class Mesh;
    
    /**
     * \brief A wrapper to interface Kahzdan's Poisson reconstruction
     *  with Geogram datastructures.
     */
    class GEOGRAM_API PoissonReconstruction {
    public:
        
        /**
         * \brief PoissonReconstruction constructor.
         */
        PoissonReconstruction();

        /***
         * \brief PoissonReconstruction destructor.
         */
        ~PoissonReconstruction();
        
        /**
         * \brief Reconstructs a surface.
         * \param[in] points a pointer to a Mesh with the points.
         *  It needs to have a vector attribute of dimension 3 
         *  called "normal" and attached to the vertices.
         * \param[out] surface the reconstructed surface
         */
        void reconstruct(Mesh* points, Mesh* surface);

        /**
         * \brief Sets the depth of the octree.
         * \param[in] x the new depth of the octree
         * \details Default value is 8. Use 10 or 11 for highly 
         *  detailed models.
         */
        void set_depth(index_t x) {
            depth_ = int(x);
        }

        /**
         * \brief Gets the depth of the octree.
         * \return the depth of the octree
         */
        index_t get_depth() const {
            return index_t(depth_);
        }

        /**
         * \brief Sets whether voxel data should be kept.
         * \param[in] x true if voxel data should be kept,
         *  false otherwise (default).
         */
        void set_keep_voxel(bool x) {
            keep_voxel_ = x;
        }

        /**
         * \brief Tests whether voxel data is kept.
         * \retval true if voxel data is kept
         * \retval false otherwise
         */
        bool get_keep_voxel() const {
            return keep_voxel_;
        }

        /**
         * \brief Gets the voxel values.
         * \details Can be called if set_keel_voxel(true) 
         *  was called before calling reconstruct().
         * \return a pointer to an array of 
         *  voxel_resolution() *  voxel_resolution() * voxel_resolution()
         *  floating point coordinates. PoissonReconstruction keeps ownership
         *  of the allocated memory.
         */
        float* voxel_values() const {
            geo_assert(keep_voxel_);
            return voxel_values_;
        }

        /**
         * \brief Gets the resolution of the created voxel grid.
         * \details Can be called if set_keel_voxel(true) 
         *  was called before calling reconstruct().
         * \return The resolution of the created voxel grid.
         */
        index_t voxel_resolution() const {
            geo_assert(keep_voxel_);            
            return voxel_res_;
        }

        /**
         * \brief Gets the origin of the box.
         * \return the corner of the box with the
         *  smallest x,y,z coordinates
         */
        const vec3& box_origin() const {
            return box_origin_;
        }

        /**
         * \brief Gets the edge length of the box.
         * \return the edge length of the box
         */
        double box_edge_length() const {
            return box_edge_length_;
        }
        
        // TODO: add setters and getters for the
        // other parameters (not all of them are
        // needed though)

    private:
        /**
         * \brief Forbids copy.
         */
        PoissonReconstruction(const PoissonReconstruction& rhs);
        
        /**
         * \brief Forbids copy.
         */
        PoissonReconstruction& operator=(const PoissonReconstruction& rhs);
        
    private:
        bool performance_;
        bool complete_;
        bool no_comments_;
        bool polygon_mesh_;
        bool confidence_;
        bool normal_weights_;
        bool non_manifold_;
        bool dirichlet_;
        bool ascii_;
        bool density_;
        bool linear_fit_;
        bool primal_voxel_;
        bool verbose_;
        
        int degree_;
        int depth_;
        int cg_depth_;
        int kernel_depth_;
        int adaptive_exponent_;
        int iters_;
        int voxel_depth_;
        int full_depth_;
        int min_depth_;
        int max_solve_depth_;
        int threads_;

        float color_;
        float samples_per_node_;
        float scale_;
        float cg_accuracy_;
        float point_weight_;

        bool keep_voxel_;
        index_t voxel_res_;
        float* voxel_values_;

        vec3 box_origin_;
        double box_edge_length_;
    };
    
}

#endif
