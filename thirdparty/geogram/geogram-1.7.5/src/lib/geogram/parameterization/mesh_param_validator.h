/*
 *  Copyright (c) 2012-2016, Bruno Levy
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

#ifndef GEOGRAM_MESH_MESH_PARAM_VALIDATOR
#define GEOGRAM_MESH_MESH_PARAM_VALIDATOR

#include <geogram/basic/common.h>
#include <geogram/parameterization/mesh_segmentation.h>
#include <geogram/basic/geometry.h>
#include <geogram/basic/attributes.h>

/**
 * \file geogram/mesh/mesh_param_validator.h
 * \brief A class to test the validity of a parameterized chart.
 */

namespace GEO {

    class Mesh;

    /**
     * \brief Tests whether texture coordinates attached to a surface mesh 
     *  define a valid parameterization.
     */
    class GEOGRAM_API ParamValidator {
    public:
	/**
	 * \brief ParamValidator constructor.
	 */
        ParamValidator();

	/**
	 * \brief ParamValidator destructor.
	 */
        ~ParamValidator();

	/**
	 * \brief Tests whether a chart and associated texture 
	 *  coordinates defines a valid parameterization.
	 * \param[in] chart a reference to the chart
	 * \details The mesh this chart belongs to is supposed to have a
	 *  2d vector attribute "tex_coord" attached to the 
	 *  vertices of the mesh with the texture coordinates.
	 * \retval true if texture coordinates define a valid parameterization.
	 * \retval false otherwise.
	 */
        bool chart_is_valid(Chart& chart);

	/**
	 * \brief Computes the scaling induced by the parameterization.
	 * \return the ratio between the maximum area scaling and minimum
	 *  area scaling of the triangles.
	 * \param[in] chart a reference to the chart
	 */
        double chart_scaling(Chart& chart);

	/**
	 * \brief Computes the filling and overlapping ratio of a 
	 *  parameterized chart.
	 * \details The filling and overlapping ratio are stored in this 
	 *  ParamValidator and can be subsequently queried with fill_ratio() 
	 *  and overlap_ratio() respectively.
	 */
        void compute_fill_and_overlap_ratio(Chart& chart);

	/**
	 * \brief Gets the computed filling ratio.
	 * \details chart_is_valid() or compute_fill_and_overlap_ration() 
	 *  need to be called before.
	 * \return The ratio between the area used by the triangles in 
	 *  parameter space and the total area of the bounding rectangle.
	 */
        double fill_ratio() const {
	    return fill_ratio_;
	}

	/**
	 * \brief Gets the computed overlap ratio.
	 * \details chart_is_valid() or compute_fill_and_overlap_ration() 
	 *  need to be called before.
	 * \return The ratio between the area that correspond to overlapping 
	 *  triangles in parameter space and the total area of the bounding 
	 *  rectangle.
	 */
	double overlap_ratio() const {
	    return overlap_ratio_;
	}

	/**
	 * \brief Gets the maximum overlapping ratio.
	 * \details If the overlapping ratio is greater than this threshold 
	 *  then chart_is_valid() returns false.
	 * \return the maximum ratio between the area that correspond 
	 *  to overlapping triangles in parameter space and the total area of 
	 *  the bounding rectangle. 
	 */
        double get_max_overlap_ratio() const {
	    return max_overlap_ratio_;
	}

	/**
	 * \brief Sets the maximum overlapping ratio.
	 * \param[in] x the maximum ratio between the area that correspond 
	 *  to overlapping triangles in parameter space and the total area of 
	 *  the bounding rectangle.
	 * \details If the overlapping ratio is greater than this threshold 
	 *  then chart_is_valid() returns false.
	 */
	void set_max_overlap_ratio(double x) {
	    max_overlap_ratio_ = x;
	}

	/**
	 * \brief Gets the maximum scaling.
	 * \details If the scaling is greater than this threshold then 
	 *  chart_is_valid() returns false.
	 * \return the maximum scaling between the area of a facet in 
	 *  parameter space and the area of the facet in 3D, 
	 *  relative to the mimium scaling evaluated on all the
	 *  facets of the mesh.
	 */  
	double get_max_scaling() const {
	    return max_scaling_;
	}

	/**
	 * \brief Sets the maximum scaling.
	 * \details If the scaling is greater than this threshold then 
	 *  chart_is_valid() returns false.
	 * \param[in] x the maximum scaling between the area of a facet 
	 *  in parameter space and the area of the facet in 3D, relative 
	 *  to the mimium scaling evaluated on all the facets of the mesh.
	 */  
	void set_max_scaling(double x) {
	    max_scaling_ = x;
	}

	/**
	 * \brief Gets the minimum filling ratio.
	 * \details If the filling ratio is greater than this threshold then 
	 *  chart_is_valid() returns false.
	 * \return the minimum ratio between the area taken by the facets in
	 *  parameter space and the total area of the bounding rectangle. 
	 */
	double get_min_fill_ratio() const {
	    return min_fill_ratio_;

	}

	/**
	 * \brief Sets the minimum filling ratio.
	 * \details If the filling ratio is greater than this threshold then 
	 *  chart_is_valid() returns false.
	 * \param[in] x the minimum ratio between the area taken by the 
	 *  facets in parameter space and the total area of the bounding 
	 *  rectangle. 
	 */
	void set_min_fill_ratio(double x) {
	    min_fill_ratio_ = x;
	}

	/**
	 * \brief Enables or disables messages.
	 * \param[in] x if true, messages are displayed on the console
	 *  with charts statistics. Default is non-verbose.
	 */
	void set_verbose(bool x) {
	    verbose_ = x;
	}
	
    protected:

	/**
	 * \brief Initializes the software rasterizer for a chart.
	 * \param[in] chart a reference to the chart.
	 * \param[in] tex_coord a vector attribute of dimension 2 attached
	 *  to the facet corners of the chart with the texture coordinates.
	 */
        void begin_rasterizer(Chart& chart, Attribute<double>& tex_coord);

	/**
	 * \brief Terminates the software rasterizer.
	 * \details This counts the pixels to evaluate the filling and 
	 *  overlapping ratio.
	 */
        void end_rasterizer();

	/**
	 * \brief Rasterizes a triangle.
	 * \details This updates pixel counts for evaluating the filling 
	 *  and overlapping ratio.
	 * \param[in] p1 , p2 , p3 the 2d coordinates of the vertices of the
	 *  triangle.
	 */
        void rasterize_triangle(
            const vec2& p1, const vec2& p2, const vec2& p3
        );

	/**
	 * \brief Transforms a 2d point from parameter space to 
	 *  raterizer coordinates.
	 * \param[in] p the parameter-space 2d coordinates of the point.
	 * \param[out] x , y the integer rasterizer-space coordinates.
	 */
        void transform(const vec2& p, int& x, int& y);

    private:
	/**
	 * \brief Width and height of the rasterizer, in pixels.
	 */
        int graph_size_;

	/**
	 * \brief A pointer to rasterizer's memory, a 2d array of 
	 *  graph_size_ times graph_size_ pixels.
	 */
        Numeric::uint8* graph_mem_;

	/**
	 * \brief A pointer to an array of dimension graph_size_ with 
	 *  the left X pixel coordinate of the scanline,
	 */  
        int* x_left_;

	/**
	 * \brief A pointer to an array of dimension graph_size_ with 
	 *  the right X pixel coordinate of the scanline,
	 */  
        int* x_right_;

	/**
	 * \brief Viewport lower-left corner x coordinate.
	 */
        double user_x_min_;

	/**
	 * \brief Viewport lower-left corner y coordinate.
	 */
        double user_y_min_;

	/**
	 * \brief Viewport width.
	 */
        double user_width_;

	/**
	 * \brief Viewport height.
	 */
        double user_height_;

	/**
	 * \brief Viewport size, i.e. max(width,height).
	 */
        double user_size_;

	/**
	 * \brief The computed filling ratio.
	 */
        double fill_ratio_;

	/**
	 * \brief The computed overlapping ratio.
	 */
        double overlap_ratio_;

	/**
	 * \brief Maximum tolerated overlapping ratio.
	 */
        double max_overlap_ratio_;

	/**
	 * \brief Maximum tolerated facet scaling.
	 */
        double max_scaling_;

	/**
	 * \brief Minimum required filling ratio.
	 */
        double min_fill_ratio_;

	/**
	 * \brief If true, displays statistics on the logger.
	 */
	bool verbose_;
	
    private:
	/**
	 * \brief Forbids copy.
	 */
        ParamValidator(const ParamValidator& rhs);

	/**
	 * \brief Forbids copy.
	 */
        ParamValidator& operator=(const ParamValidator& rhs);
    };
    
}

#endif
