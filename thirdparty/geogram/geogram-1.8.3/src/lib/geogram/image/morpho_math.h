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
