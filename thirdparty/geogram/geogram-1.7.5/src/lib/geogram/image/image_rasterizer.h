/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000 Bruno Levy
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     levy@loria.fr
 *
 *     ISA Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs. 
 */
 
#ifndef H_OGF_IMAGE_IO_IMAGE_RASTERIZER_H
#define H_OGF_IMAGE_IO_IMAGE_RASTERIZER_H

#include <geogram/basic/common.h>
#include <geogram/image/image.h>

/**
 * \file geogram/image/image_serializer.h
 * \brief Class to draw triangles in an image.
 */

namespace GEO {

    /**
     * \brief Draws triangles in an image.
     */
    class GEOGRAM_API ImageRasterizer {
      public:

	/**
	 * \brief ImageRasterizer constructor.
	 * \param[in] image a pointer to the target image.
	 * \param x1 , y1 , x2 , y2 viewport coordinates.
	 */
        ImageRasterizer(
	    Image* image,
	    double x1=0.0, double y1=0.0,
	    double x2=1.0, double y2=1.0
	); 

	/**
	 * \brief Clears the target image.
	 */
	void clear();

	/**
	 * \brief Draws a triangle in the target image.
	 * \details Colors are linearly interpolated (Gouraud shading).
	 * \param p1 , p2 , p3 the three vertices of the triangle.
	 * \param c1 , c2 , c3 the three colors of the vertices.
	 */
	void triangle(
	    const vec2& p1, const Color& c1, 
	    const vec2& p2, const Color& c2, 
	    const vec2& p3, const Color& c3
	);

	/**
	 * \brief Sets a pixel of the image.
	 * \details Only BYTE, FLOAT32 and FLOAT64 component encoding
	 *  are supported. If the image has less than 4 channels, the
	 *  extra channels in \p c are ignored.
	 * \param[in] x , y the integer pixel coordinates
	 * \param[in] c the color of the pixel
	 */
	void set_pixel(int x, int y, const Color& c) {
	    geo_debug_assert(x >= 0 && x < int(image_->width()));
	    geo_debug_assert(y >= 0 && y < int(image_->height()));
	    switch(component_encoding_) {
		case Image::BYTE: {
		    Memory::byte* pixel_ptr =
			(Memory::byte*)(
			    image_->pixel_base(index_t(x),index_t(y))
		    );
		    for(index_t comp=0; comp<nb_components_; ++comp) {
			pixel_ptr[comp] = Memory::byte(c[comp] * 255.0);
		    }
		} break;
		case Image::FLOAT32: {
		    Numeric::float32* pixel_ptr =
			(Numeric::float32*)(void*)(
			    image_->pixel_base(index_t(x),index_t(y))
		    );
		    for(index_t comp=0; comp<nb_components_; ++comp) {
			pixel_ptr[comp] = Numeric::float32(c[comp]);
		    }
		} break;
		case Image::FLOAT64: {
		    Numeric::float64* pixel_ptr =
			(Numeric::float64*)(void*)(
			    image_->pixel_base(index_t(x),index_t(y))
		    );
		    for(index_t comp=0; comp<nb_components_; ++comp) {
			pixel_ptr[comp] = Numeric::float64(c[comp]);
		    }
		} break;
		case Image::INT16:
		case Image::INT32: {
		    geo_assert_not_reached;
		} 
	    }
	}

	
      protected:

	/**
	 * \brief Transforms a 2d point from world space to pixel coordinates.
	 * \param[in] p the world-space coordinates of the point.
	 * \param[out] transformed the pixel coordinates of the point.
	 */
	void transform(const vec2& p, vec2i& transformed) {
	    transformed.x = Numeric::int32(
		double(image_->width())*(p.x - x1_) / (x2_ - x1_)
	    );
	    transformed.y = Numeric::int32(
		double(image_->height())*(p.y - y1_) / (y2_ - y1_)
	    );
	}

	/**
	 * \brief Computes the linear interpolation between three colors.
	 * \param[in] c1 , c2 , c3 the three colors to be interpolated.
	 * \param[in] l1 , l2 , l3 the coefficients of the interpolation.
	 * \param[out] c the interpolated color.
	 */
	void interpolate_color(
	    const Color& c1, const Color& c2, const Color& c3,
	    double l1, double l2, double l3,
	    Color& c
	) {
	    c[0] = l1*c1[0] + l2*c2[0] + l3*c3[0];
	    c[1] = l1*c1[1] + l2*c2[1] + l3*c3[1];
	    c[2] = l1*c1[2] + l2*c2[2] + l3*c3[2];
	    c[3] = l1*c1[3] + l2*c2[3] + l3*c3[3];	    
	}

      private:
	Image* image_;
	Image::ComponentEncoding component_encoding_;
	index_t nb_components_;
	double x1_;
	double y1_;
	double x2_;
	double y2_;	
    };
}

#endif
