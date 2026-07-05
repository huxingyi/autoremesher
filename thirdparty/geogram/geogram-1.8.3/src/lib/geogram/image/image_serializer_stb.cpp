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
 
 
#include <geogram/image/image_serializer_stb.h>
#include <geogram/image/image_library.h>
#include <geogram/basic/file_system.h>
#include <geogram/basic/string.h>
#include <geogram/basic/logger.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

// Use internal linkage for symbols from stb_image as it is a very commonly embedded library.
// Making these symbols visible  causes duplicate symbol problems if geogram is linked
// statically together with another library or executable that also embeds stb_image.
#define STB_IMAGE_STATIC

// [Bruno] I got too many complaints in STB so I "close my eyes" :-)
#ifdef __GNUC__
#ifndef __ICC
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wunused-function"
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

#ifdef _MSC_VER
#pragma warning( disable : 4505 )
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

