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

#include <geogram/image/morpho_math.h>
#include <geogram/image/image.h>
#include <vector>
#include <string.h>

namespace GEO {

    static inline bool has_value(Memory::byte* p, size_t bytes_per_pixel_) {
	bool result = false;
        switch(bytes_per_pixel_) {
        case 1:
            result = (*p != 0);
	    break;
        case 3:
            result = (p[0] != 0 || p[1] != 0 || p[2] != 0);
	    break;
        case 4:
	    result = (p[3] != 0);
	    break;
        default:
            geo_assert_not_reached;
        }
        return result;
    }

    void StructuringElement::convolve(
        Memory::byte* from, Memory::byte* to
    ) const {
        if(has_value(from, bytes_per_pixel_)) {
            for(size_t i=0; i<bytes_per_pixel_; i++) {
                to[i] = from[i];
            }
        } else {
            int rgb[4];
            int nb_neigh = 0;
            rgb[0] = 0;
            rgb[1] = 0;
            rgb[2] = 0;
            rgb[3] = 0;

            for(index_t i=0; i<offset_.size(); i++) {
                for(size_t j=0; j<bytes_per_pixel_; j++) {
                    rgb[j] += (from + offset_[i])[j];
                }
                if(has_value(from + offset_[i], bytes_per_pixel_)) {
                    nb_neigh++;
                }
            }

            if(nb_neigh == 0) {
                nb_neigh = 1;
            }
            
            for(size_t j=0; j<bytes_per_pixel_; j++) {
                to[j] = Memory::byte(rgb[j] / nb_neigh);
            }

        }
    }

    
    MorphoMath::MorphoMath(Image* target) {
        target_ = target;
        geo_assert(target_->component_encoding() == Image::BYTE);
        width_ = target_->width();
        height_ = target_->height();
        bytes_per_pixel_ = target_->bytes_per_pixel();
        bytes_per_line_ = bytes_per_pixel_ * width_;
        graph_mem_ = target_->base_mem();
    }

    MorphoMath::~MorphoMath() {
        target_ = nullptr;
    }

    void MorphoMath::dilate(index_t nb_iterations) {
        StructuringElement str(target_);
        str.add_neighbor(-1,-1);
        str.add_neighbor(-1, 0);
        str.add_neighbor(-1, 1);
        str.add_neighbor( 0,-1);
        str.add_neighbor( 0, 0);
        str.add_neighbor( 0, 1);
        str.add_neighbor( 1,-1); 
        str.add_neighbor( 1, 0); 
        str.add_neighbor( 1, 1);
        dilate(str, nb_iterations);
    }

    void MorphoMath::dilate(
	const StructuringElement& str, index_t nb_iterations
    ) {
        Image_var tmp = new Image(
	    target_->color_encoding(),
	    target_->component_encoding(),
	    target_->width(),
	    target_->height()
	);
	Memory::copy(tmp->base_mem(), target_->base_mem(), target_->bytes());

        index_t R = str.radius();

        index_t line_offset = (R * index_t(bytes_per_pixel_));

        for(index_t iter=0; iter<nb_iterations; iter++) {
            Memory::byte* from_line =
		target_->base_mem() + R * bytes_per_line_;
            Memory::byte* to_line   =
		tmp->base_mem()     + R * bytes_per_line_;
            for(index_t y=R; y<height_ - R; ++y) {
                Memory::byte* from = from_line + line_offset;
                Memory::byte* to   = to_line   + line_offset;
                for(index_t x=R; x<width_ - R; ++x) {
                    from += bytes_per_pixel_;
                    to   += bytes_per_pixel_;
                    str.convolve(from, to);
                }
                from_line += bytes_per_line_;
                to_line   += bytes_per_line_;
            }
            Memory::copy(
                target_->base_mem(), tmp->base_mem(), 
                target_->bytes()
            );
        }
    }
}

