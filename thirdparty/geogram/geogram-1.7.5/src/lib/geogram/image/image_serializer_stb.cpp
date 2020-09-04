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
 
 
#include <geogram/image/image_serializer_stb.h>
#include <geogram/image/image_library.h>
#include <geogram/basic/file_system.h>
#include <geogram/basic/string.h>
#include <geogram/basic/logger.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

// [Bruno] I got too many complaints in STB so I "close my eyes" :-)
#ifdef __GNUC__
#ifndef __ICC
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#endif

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wdisabled-macro-expansion"
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wreserved-id-macro"
#pragma GCC diagnostic ignored "-Wcomma"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
#endif

#include <geogram/third_party/stb_image/stb_image.h>
#include <geogram/third_party/stb_image/stb_image_write.h>

namespace GEO {

    ImageSerializerSTB::ImageSerializerSTB(bool read, bool write) :
	read_(read),
	write_(write)
    {
    }
	
    Image* ImageSerializerSTB::serialize_read(const std::string& file_name) {
	std::string extension = String::to_lowercase(
	    FileSystem::extension(file_name)
	);
	if(
	    extension != "png" &&
	    extension != "jpg" &&
	    extension != "jpeg" &&
	    extension != "tga" &&
	    extension != "bmp" 
	) {
	    return nullptr;
	}

	Image* result = nullptr;
	int width, height, bpp;
	int desired_bpp = 4;
	unsigned char* data = stbi_load(
	    file_name.c_str(), &width, &height, &bpp, desired_bpp
	);
	if(data == nullptr) {
	    Logger::err("Image")
		<< file_name << " : could not load file." << std::endl;
	} else {
	    result = new Image(
		Image::RGBA, 
		Image::BYTE, index_t(width), index_t(height)
	    );
	    // bpp: how many bytes per pixel there was in the file
	    // desired_bpp set to 4, thus stbi expands it to r,g,b,a always.
	    Memory::copy(result->base_mem(), data, size_t(width*height*4));
	    stbi_image_free(data);
	    // Seems that STB has inverse convention as mine for image Y axis,
	    result->flip_vertically();
	}
	
	return result;
    }
    
    bool ImageSerializerSTB::serialize_write(
	const std::string& file_name, const Image* image_in
    ) {
	bool result = true;
	std::string extension = String::to_lowercase(
	    FileSystem::extension(file_name)
	);

	// Seems that STB has inverse convention as mine for image Y axis,
	// so we flip before saving and flip back after.
	
	Image* image = const_cast<Image*>(image_in);
	image->flip_vertically();
	
	int w = int(image->width());
	int h = int(image->height());
	int comp = int(image->components_per_pixel());
	const char* data = (const char*)(image->base_mem());
	
	if(extension == "png") {
	    result = (stbi_write_png(file_name.c_str(), w, h, comp, data, w*comp) != 0);
	} else if(extension == "bmp") {
	    result = (stbi_write_bmp(file_name.c_str(), w, h, comp, data) != 0);	    
	} else if(extension == "jpeg" || extension == "jpg") {
	    result = (stbi_write_jpg(file_name.c_str(), w, h, comp, data, 80) != 0);	    	    
	} else if(extension == "tga") {
	    result = (stbi_write_tga(file_name.c_str(), w, h, comp, data) != 0);	    	    	    
	}
	
	image->flip_vertically();	
	return result;
    }

    bool ImageSerializerSTB::binary() const {
	return true;
    }
    
    bool ImageSerializerSTB::streams_supported() const {
	return false;
    }
    
    bool ImageSerializerSTB::read_supported() const {
	return read_;
    }
    
    bool ImageSerializerSTB::write_supported() const {
	return write_;
    }

    /*****************************************************/
    
    ImageSerializerSTBRead::ImageSerializerSTBRead() :
	ImageSerializerSTB(true, false) {
    }

    ImageSerializerSTBRead::~ImageSerializerSTBRead() {
    }

    /*****************************************************/    
    
    ImageSerializerSTBReadWrite::ImageSerializerSTBReadWrite() :
	ImageSerializerSTB(true, true) {
    }

    ImageSerializerSTBReadWrite::~ImageSerializerSTBReadWrite() {
    }

    /*****************************************************/    
    
}

