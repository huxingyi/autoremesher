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

#include <geogram/image/image_serializer_xpm.h>
#include <geogram/image/image.h>
#include <geogram/basic/string.h>
#include <geogram/basic/logger.h>

#include <sstream>
#include <string.h>

namespace {
    using namespace GEO;

    /**
     * \brief Converts an hexadecimal digit into an integer.
     * \param[in] digit the character representation of the digit.
     * \return the corresonding integer value, in [0,15].
     */
    inline int htoi(char digit) {
        if(digit >= '0' && digit <= '9') {
            return digit - '0';
        }
        if(digit >= 'a' && digit <= 'f') {
            return digit - 'a' + 10;
        }
        if(digit >= 'A' && digit <= 'F') {
            return digit - 'A' + 10;
        }   
        Logger::err("Image") 
            << "XPM Image reader: hex digit to integer: invalid digit: \'" 
            << digit << "\'" << std::endl;
        abort();
    }

    /**
     * \brief Decodes an XPM colormap entry.
     * \param[in] colormap_entry the string with the colormap entry.
     * \param[out] colorcell the corresponding color.
     */
    bool decode_colormap_entry(
	const char* colormap_entry, Colormap::ColorCell& colorcell
    ) {
	const char* colorcode = strstr(colormap_entry, "c #");
	if(colorcode == nullptr) {
	    if(strstr(colormap_entry, "None") != nullptr) {
		colorcell = Colormap::ColorCell(0,0,0,0);
		return true;
	    } else {
		Logger::err("Image") 
		    << "XPM Image reader: Colormap entry without any color" 
		    << std::endl;
		Logger::err("Image") 
		    << "   entry = \'" << colormap_entry << "\'" << std::endl;
		return false;
	    }
	}
	colorcode += 3;

	Memory::byte r,g,b,a;

	if(strlen(colorcode) == 12) {
	    r = Memory::byte(16 * htoi(colorcode[0]) + htoi(colorcode[1]));
	    g = Memory::byte(16 * htoi(colorcode[4]) + htoi(colorcode[5]));
	    b = Memory::byte(16 * htoi(colorcode[8]) + htoi(colorcode[9]));
	    a = 255;
	} else {
	    r = Memory::byte(16 * htoi(colorcode[0]) + htoi(colorcode[1]));
	    g = Memory::byte(16 * htoi(colorcode[2]) + htoi(colorcode[3]));
	    b = Memory::byte(16 * htoi(colorcode[4]) + htoi(colorcode[5]));
	    a = 255;
	}
	colorcell = Colormap::ColorCell(r,g,b,a);
	return true;
    }
    
}

/******************************************************************************/

namespace GEO {

    Image* ImageSerializer_xpm::serialize_read(std::istream& stream) {
	return serialize_read_static(stream);
    }

    Image* ImageSerializer_xpm::create_image_from_xpm_data(const char* s) {
	std::istringstream in(s);
	return serialize_read_static(in);
    }
    
    Image* ImageSerializer_xpm::serialize_read_static(std::istream& stream) {

	Image* result = nullptr;
	
        int num_colors;
        int chars_per_pixels;
        int width;
        int height;

        // *********************** header
        {
            char* header = next_xpm_data(stream);
            if(header == nullptr) {
                Logger::err("Image") 
                    << "XPM image input: unexpected end of file" << std::endl;
                return nullptr;
            }
            std::istringstream in(header);
            in >> width >> height >> num_colors >> chars_per_pixels;
            if(num_colors > 1024) {
                Logger::err("Image") 
                    << "XPM image input: too many colors ("
                    << num_colors
                    << ")" << std::endl;
                Logger::err("Image") 
                    << "  should not be greater than 1024" << std::endl;
                return nullptr;
            }

            switch(chars_per_pixels) {
            case 1:
                result=read_xpm_1_byte_per_pixel(
                    width, height, num_colors, stream
                );
                break;
            case 2:
                result=read_xpm_2_bytes_per_pixel(
                    width, height, num_colors, stream
                );
                break;
            default:
                Logger::err("Image") 
                    << "XPM image input: invalid chars per pixels ("
                    << chars_per_pixels << ")" << std::endl;
                Logger::err("Image") << "  should be 2" << std::endl;
                return nullptr;
            }

	    // Convert colormapped to RGBA.
	    // We do that because texturing functions are not implemented for colormapped.
	    // TODO: move function to Image library.
	    Image* result_rgba = new Image(Image::RGBA, Image::BYTE, result->width(), result->height());
	    for(index_t y=0; y<result->height(); ++y) {
		for(index_t x=0; x<result->width(); ++x) {
		    index_t c = index_t(*result->pixel_base(x,y));
		    result_rgba->pixel_base(x,y)[0] = result->colormap()->color_cell(c).r();
		    result_rgba->pixel_base(x,y)[1] = result->colormap()->color_cell(c).g();
		    result_rgba->pixel_base(x,y)[2] = result->colormap()->color_cell(c).b();
		    result_rgba->pixel_base(x,y)[3] = result->colormap()->color_cell(c).a();		    
		}
	    }
	    delete result;
	    
	    return result_rgba;
        }
    }
    
    Image* ImageSerializer_xpm::read_xpm_2_bytes_per_pixel(        
        int width, int height, int num_colors, std::istream& stream
    ) {

        // Converts a two-digit XPM color code into
        //  a color index.
        static int conv_table[256][256];

        // For checking, put a negative value to
        //  detect invalid color codes.
        for(int k1=0; k1 < 256; k1++) {
            for(int k2=0; k2 < 256; k2++) {
                conv_table[k1][k2] = -1;
            }
        }
    
        // **********************  colormap
    
        typedef Numeric::uint8 byte;

        Colormap* colormap = new Colormap(index_t(num_colors));

        for(int entry_num=0; entry_num<num_colors; entry_num++) {
            char* entry = next_xpm_data(stream);
            if(entry == nullptr) {
                Logger::err("Image") 
                    << "XPM Image reader: Unexpected end of file" 
                    << std::endl;
                delete colormap;
                return nullptr;
            }
     
            int key1 = entry[0];
            int key2 = entry[1];
      
	    Colormap::ColorCell cell;
	    if(!decode_colormap_entry(entry, cell)) {
		return nullptr;
	    }
	    
            colormap-> color_cell(index_t(entry_num)) = cell;
            conv_table[key1][key2] = (unsigned char)entry_num;
        }
        
        // ****************** image
        
        Image* result = new Image(
	    Image::INDEXED, Image::BYTE, index_t(width), index_t(height)
	);
        result-> set_colormap(colormap);
    
        for(int y=0; y<height; y++) {
            char* scan_line = next_xpm_data(stream);
            if(scan_line == nullptr) {
                Logger::err("Image") 
                    << "XPM Image reader: Unexpected end of file"
                    << std::endl;
                delete result;
                return nullptr;
            }
            for(int x=0; x<width; x++) {
                int key1 = scan_line[2*x];
                int key2 = scan_line[2*x+1];
                int color_index = conv_table[key1][key2];
                if(color_index < 0 || color_index > num_colors) {
                    Logger::err("Image") 
                        << "XPM Image reader: Invalid color index in image" 
                        << std::endl;
                    delete result;
                    return nullptr;
                }
                result-> base_mem()[y * width + x] = byte(color_index);
            }
        }

        return result;
    }


    Image* ImageSerializer_xpm::read_xpm_1_byte_per_pixel(        
        int width, int height, int num_colors, std::istream& stream
    ) {
        
        // Converts a two-digit XPM color code into
        //  a color index.
        static int conv_table[256];

        // For checking, put a negative value to
        //  detect invalid color codes.
        for(int k1=0; k1 < 256; k1++) {
            conv_table[k1] = -1;
        }
        
        // *********************   colormap
        
        typedef Numeric::uint8 byte;
        
        Colormap* colormap = new Colormap(index_t(num_colors));
        
        for(int entry_num=0; entry_num<num_colors; entry_num++) {
            char* entry = next_xpm_data(stream);
            if(entry == nullptr) {
                Logger::err("Image") 
                    << "XPM Image reader: Unexpected end of file" 
                    << std::endl;
                delete colormap;
                return nullptr;
            }
            
            int key1 = entry[0];

	    Colormap::ColorCell cell;
	    if(!decode_colormap_entry(entry, cell)) {
		return nullptr;
	    }
            colormap-> color_cell(index_t(entry_num)) = cell;
            conv_table[key1] = (unsigned char)entry_num;
        }
        
        // *********************  image
        
        Image* result = new Image(
	    Image::INDEXED, Image::BYTE, index_t(width), index_t(height)
	);
        result-> set_colormap(colormap);
    
        for(int y=0; y<height; y++) {
            char* scan_line = next_xpm_data(stream);
            if(scan_line == nullptr) {
                Logger::err("Image") 
                    << "XPM Image reader: Unexpected end of file"
                    << std::endl;
                delete result;
                return nullptr;
            }
            for(int x=0; x<width; x++) {
                int key1 = scan_line[x];
                int color_index = conv_table[key1];
                if(color_index < 0 || color_index > num_colors) {
                    Logger::err("Image") 
                        << "XPM Image reader: Invalid color index in image" 
                        << std::endl;
                    delete result;
                    return nullptr;
                }
                result-> base_mem()[y * width + x] = byte(color_index);
            }
        }
    
        return result;
    }

    bool ImageSerializer_xpm::binary() const {
        return false;
    }

    char* ImageSerializer_xpm::next_xpm_data(std::istream& input) {
        static char line_buffer[4096];
        char* result = nullptr;
        bool found = false;
        while(!found && !input.eof()) {
            input.getline(line_buffer,4096);
            char* p1 = strchr(line_buffer,'\"');
            char* p2 = strchr(line_buffer + 1, '\"');
            found = (p1 != nullptr && p2 != nullptr);
            if(found) {
                result = p1 + 1;
                *p2 = '\0';
            }
        }
        return result;
    }

    bool ImageSerializer_xpm::read_supported() const {
        return true;
    }

/**********************************************************************/

}

