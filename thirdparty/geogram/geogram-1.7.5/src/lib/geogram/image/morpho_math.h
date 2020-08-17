/*
 *  GXML/Graphite: Geometry and Graphics Programming Library + Utilities
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

#ifndef H_IMAGE_ALGOS_MORPHO_MATH_H
#define H_IMAGE_ALGOS_MORPHO_MATH_H

#include <geogram/basic/common.h>
#include <geogram/image/image.h>

/**
 * \file geogram/image/morpho_math.h
 * \brief Classes for morphological operations on images.
 */

namespace GEO {

    /**
     * \brief A structuring element, that is the definition of
     *  neighborhood used by a morphological operation.
     */
    class GEOGRAM_API StructuringElement {
      public:

	/**
	 * \brief StructuringElement constructor.
	 * \param[in] source a pointer to the source image.
	 */
        StructuringElement(Image* source) : source_(source) {
            base_mem_ = source_->base_mem();
            width_ = source_->width();
            radius_ = 0;
            bytes_per_pixel_ = source->bytes_per_pixel();
        }

	/**
	 * \brief Gets the radius.
	 * \return the maximum difference of coordinate between the
	 *  center and one of the neighbors.
	 */
        index_t radius() const {
	    return radius_;
	}

	/**
	 * \brief Adds a neighbor to this structuting element.
	 * \param[in] xrel , yrel the coordinates of the neighbor, 
	 *  relative to the center of this structuring element.
	 */
        void add_neighbor(int xrel, int yrel) {
            offset_.push_back(
		(xrel + int(width_) * yrel) * int(bytes_per_pixel_)
	    );
            radius_ = std::max(radius_, index_t(std::abs(xrel)));
            radius_ = std::max(radius_, index_t(std::abs(yrel)));
        }

	/**
	 * \brief Computes the convolution at a given memory location.
	 * \param[in] from a pointer to the source pixel at the center 
	 *  of the structuring element.
	 * \param[in] to a pointer to the target pixel at the center of
	 *  the structuring element.
	 */
        void convolve(Memory::byte* from, Memory::byte* to) const;

	/**
	 * \brief Computes the convolution at a given pixel.
	 * \details The source image is the one that was specified to the
	 *  constructor of this StructuringElement.
	 * \param[in] x , y the coordinates of the pixel.
	 * \param[in] target_img a pointer to the target image.
	 */
        inline void convolve(int x, int y, Image* target_img) const {
            int pixel_base = ((x + int(width_) * y) * int(bytes_per_pixel_));
            convolve(
		base_mem_ + pixel_base, target_img->base_mem() + pixel_base
	    );
        }

    private:
        Memory::byte* base_mem_;
        index_t width_;
        Image* source_;
        index_t radius_;
	vector<int> offset_;
        size_t bytes_per_pixel_;
    };


    /**
     * \brief Implements morphological operators for images.
     */
    class GEOGRAM_API MorphoMath {
    public:
	/**
	 * \brief MorphoMath constructor.
	 * \details Only implemented for 2D images with byte components.
	 * \param[in] target a pointer to the target image.
	 * \pre target->component_encoding() == Image::BYTE
	 */
        MorphoMath(Image* target);

	/**
	 * \brief MorphoMath destructor;
	 */
        ~MorphoMath();

	/**
	 * \brief Computes a dilation.
	 * \param[in] elt a const reference to the structuring element.
	 * \param[in] nb_iterations number of dilations to be applied.
	 */
        void dilate(const StructuringElement& elt, index_t nb_iterations = 1);

	/**
	 * \brief Computes a dilation with a default structuring element.
	 * \param[in] nb_iterations number of dilations to be applied.
	 */
        void dilate(index_t nb_iterations = 1);
        
    private:
        Image* target_;
        Numeric::uint8* graph_mem_;
        index_t width_;
        index_t height_;
        size_t bytes_per_pixel_;
        size_t bytes_per_line_;
    };
}

#endif
