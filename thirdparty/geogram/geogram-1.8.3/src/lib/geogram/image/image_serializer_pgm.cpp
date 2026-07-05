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

