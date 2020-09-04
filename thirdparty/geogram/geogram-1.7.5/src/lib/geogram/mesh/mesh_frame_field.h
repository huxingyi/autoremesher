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

#ifndef GEOGRAM_MESH_MESH_FRAME_FIELD
#define GEOGRAM_MESH_MESH_FRAME_FIELD

#include <geogram/basic/common.h>
#include <geogram/basic/geometry.h>
#include <geogram/points/nn_search.h>

/**
 * \file mesh_frame_field.h
 */

namespace GEO {

    class Mesh;

    /**
     * \brief Represents a 3D frame field, i.e. a function that
     *  associates a 3d orthonormal basis to each point in 3D space.
     */
    class GEOGRAM_API FrameField {
    public:
        /**
         * \brief Constructs a new uninitialized FrameField.
         */
         FrameField() : use_NN_(true) {
	 }

	/**
	 * \brief Specifies whether a spatial search structure
	 *  should be created.
	 * \details If a spatial search structure is created, then
	 *  the field can be queried at any 3D location using
	 *  get_nearest_frame() / get_nearest_frame_index(),
	 *  this is the default mode. Otherwise, get_nearest_frame()
	 *  and get_nearest_frame_index() cannot be used.
	 * \param[in] x true if spatial search should be used, false
	 *  otherwise.
	 */
	 void set_use_spatial_search(bool x) {
	     use_NN_ = x;
	 }
	
        /**
         * \brief Loads a frame field from a file.
         * \param[in] M a tetrahedral mesh
         * \param[in] filename name of the file that contains the
         *  frame vectors
         * \param[in] volumetric if true, the frames are attached
         *  to the tets of \p M, else they are attached to the facets
         * \details The file is supposed to be ASCII, with one vector
         *  per line. Alternatively, the frames can be attached to 
         *  specified points. In this case, \p volumetric is ignored, and
         *  the file has 12 scalars per line, that correspond to the 
         *  coordinates of a point and the three vectors attached to the point. 
         * \retval true on success
         * \retval false otherwise
         */
        bool load(
            const Mesh& M, bool volumetric, const std::string& filename
        ); 

        
        /**
         * \brief Creates a frame field that matches a given mesh.
         * \details The frames are interpolated from the sharp features
         *  of the mesh.
         * \param[in] M the input mesh
         * \param[in] volumetric if true, the frame field is extrapolated
         *  to the tetrahedra of the mesh (using nearest neighbors for now)
         * \param[in] sharp_angle_threshold angles smaller than this threshold
         *  (in degrees) are considered to be sharp features
         */
        void create_from_surface_mesh(
            const Mesh& M, bool volumetric, double sharp_angle_threshold=45.0
        );

        /**
         * \brief Gets the index of the frame nearest to a given point.
	 * \details Cannot be used if set_use_spatial_search(false) was
	 *  called.
         * \param[in] p the 3d coordinates of the point
         * \return the index of the frame nearest to \p p
         */
        index_t get_nearest_frame_index(const double* p) const {
	    geo_assert(use_NN_);
            return NN_->get_nearest_neighbor(p);
        }

        /**
         * \brief Gets the frame nearest to a given point.
	 * \details Cannot be used if set_use_spatial_search(false) was
	 *  called.
         * \param[in] p the 3d coordinates of the point
         * \param[out] f the 9 coordinates of the three
         *  vectors that compose the fram
         */
        void get_nearest_frame(const double* p, double* f) const {
	    geo_assert(use_NN_);	    
            index_t fi = get_nearest_frame_index(p);
            for(index_t c = 0; c < 9; ++c) {
                f[c] = frames_[fi * 9 + c];
            }
        }

        /**
         * \brief Gets the vector that contains all the frames
         *   coordinates.
         * \return a const reference to the vector of all the
         *   frame coordinates.
         */
        const vector<double>& frames() const {
            return frames_;
        }

        /*
         * \brief Scales one of the vectors of a frame.
         * \details On exit, the norm of the vector nearest to \p N in
         *   \p frame has a norm equal to \p s.
         * \param[in,out] frame the frame, as an array of 9 doubles
         * \param[in] N the vector to be scaled (retrieved in the frame)
         * \param[in] s scaling factor
         */
        static void scale_frame_vector(double* frame, const vec3& N, double s); 

        /**
         * \brief Fixes a frame in such a way that it is orthogonal
         *  to a given vector.
         * \details Makes one of the frame vectors aligned with \p N
         *  and the two other ones orthogonal to N. 
         * \param[in,out] frame the frame to fix
         * \param[in] N the normal vector to be preserved
         */
        static void fix_frame(double* frame, const vec3& N);

    private:
        NearestNeighborSearch_var NN_;
        vector<double> frames_;
        vector<double> centers_;
	bool use_NN_;
    };


}

#endif
