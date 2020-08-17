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
 
#include <geogram/image/image.h>

namespace GEO {

//_________________________________________________________

    size_t Image::nb_components(ColorEncoding color_rep) {
        size_t result = 0;
        switch(color_rep) {
        case GRAY:
        case INDEXED:
            result = 1;
            break;
        case RGB:
        case BGR:
        case YUV:
            result = 3;
            break;
        case RGBA:
            result = 4;
            break;
        }
        return result;
    }
        
    size_t Image::bytes_per_component(ComponentEncoding component_rep) {
        size_t result = 0;
        switch(component_rep) {
        case BYTE:
            result = 1;
            break;
        case INT16:
            result = 2;
            break;
        case INT32:
        case FLOAT32:
            result = 4;
            break;
        case FLOAT64:
            result = 8;
            break;
        }
        return result;
    }    
    
    Image::Image() {
        bytes_per_pixel_ = 0 ;
        dimension_ = 0 ;
        size_[0] = 0 ;
        size_[1] = 0 ;
        size_[2] = 0 ;
        factor_[0] = 0 ;
        factor_[1] = 0 ;
        factor_[2] = 0 ;
        base_mem_ = nullptr ;
    }

    Image::~Image() {
        delete[] base_mem_;
        bytes_per_pixel_ = 0 ;
        dimension_ = 0 ;
        size_[0] = 0 ;
        size_[1] = 0 ;
        size_[2] = 0 ;
        factor_[0] = 0 ;
        factor_[1] = 0 ;
        factor_[2] = 0 ;
        base_mem_ = nullptr ;
    }
    
    void Image::initialize(
        ColorEncoding color_rep, ComponentEncoding component_rep,
        index_t size_x, index_t size_y, index_t size_z
    ) {
        color_encoding_ = color_rep;
        component_encoding_ = component_rep;
        bytes_per_pixel_ =
            nb_components(color_rep) * bytes_per_component(component_rep);
        size_[0] = size_x ;
        size_[1] = size_y ;
        size_[2] = size_z ;
        dimension_ = 3;
        if(size_z == 1) {
            dimension_ = 2 ;
            if(size_y == 1) {
                dimension_ = 1 ;
            }
        }
        factor_[0] = bytes_per_pixel_ ;
        factor_[1] = factor_[0] * size_x ;
        factor_[2] = factor_[1] * size_y ;
        delete[] base_mem_;
        base_mem_ = new Memory::byte[bytes()];
	Memory::clear(base_mem_, bytes());
    }

    void Image::acquire() {
    }

    void Image::flip_vertically() {
        index_t bpp=index_t(bytes_per_pixel());
        index_t h = height() ;
        index_t w = width() ;
        index_t row_len = w * bpp ;
        for(index_t j=0; j< h/2; j++) {
            // get a pointer to the two lines we will swap
            Memory::pointer row1 = base_mem() + j * row_len ;
            Memory::pointer row2 = base_mem() + (h - 1 - j) * row_len ;
            // for each point on line, swap all the channels
            for(index_t i=0; i<w; i++) {
                for (index_t k=0;k<bpp;k++) {
		    std::swap(row1[bpp*i+k], row2[bpp*i+k]);
                }
            }
        }
    }
    
    void Image::swap_components(index_t channel1, index_t channel2) {
        size_t nb_comp = nb_components(color_encoding());
        size_t bytes_per_comp = bytes_per_component(component_encoding());
        geo_assert(channel1 < nb_comp);
        geo_assert(channel2 < nb_comp);
        size_t bpp = bytes_per_pixel();
        size_t nb_pix = nb_pixels();
        Memory::pointer pixel_base = base_mem();
        size_t channel1_offset = bytes_per_comp*channel1;
        size_t channel2_offset = bytes_per_comp*channel2;
        for(size_t i=0; i<nb_pix; ++i) {
            Memory::pointer channel1_base = pixel_base + channel1_offset;
            Memory::pointer channel2_base = pixel_base + channel2_offset;
            for(index_t c=0; c<bytes_per_comp; ++c) {
		std::swap(channel1_base[c], channel2_base[c]);
            }
            pixel_base += bpp;
        }
    }
    

//_________________________________________________________

}

