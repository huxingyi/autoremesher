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

#include <geogram/image/image_serializer_pgm.h>
#include <geogram/image/image.h>
#include <geogram/basic/logger.h>

#include <sstream>
#include <string.h>

namespace GEO {

/***************************************************************/

    Image* ImageSerializer_pgm::serialize_read(const std::string& file_name) {
	FILE* f = fopen(file_name.c_str(),"rb");
	if(f == nullptr) {
	    Logger::err("Image")<< file_name << ": could not open file"
				<< std::endl;
	    return nullptr;
	}
 
	char magic[255];
	int width, height, maxval;
	if(
	    (fscanf(f, "%s", magic) != 1)   ||
	    (strcmp(magic,"P5")	!= 0)       ||    
	    (fscanf(f, "%d", &width)  != 1) ||
	    (fscanf(f, "%d", &height) != 1) ||
	    (fscanf(f, "%d", &maxval) != 1)

	) {
	    Logger::err("PGM") << "Invalid header" << std::endl;	    
	    return nullptr;	    
	}

	Image* result = nullptr;
	if(maxval == 255) {
	    result = new Image(Image::GRAY, Image::BYTE, index_t(width), index_t(height));
	} else if(maxval == 65535) {
	    result = new Image(Image::GRAY, Image::INT16, index_t(width), index_t(height));
	} else {
	    Logger::err("PGM") << maxval << " unsupported maxval"
			       << std::endl;
	    return nullptr;
	}
	size_t nbread = fread(result->base_mem(), 1, result->bytes(), f);
	if(nbread != result->bytes()) {
	    Logger::warn("PGM") << "file is truncated"
				<< std::endl;
	}
	fclose(f);
	return result;
    }
    
    bool ImageSerializer_pgm::binary() const {
        return true;
    }


    bool ImageSerializer_pgm::read_supported() const {
        return true ;
    }

/**************************************************************/

}

