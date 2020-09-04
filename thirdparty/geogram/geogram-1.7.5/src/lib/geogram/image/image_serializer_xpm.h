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
 
#ifndef OGF_IMAGE_IO_IMAGE_SERIALIZER_XPM
#define OGF_IMAGE_IO_IMAGE_SERIALIZER_XPM

#include <geogram/basic/common.h>
#include <geogram/image/image_serializer.h>

namespace GEO {

//_________________________________________________________

    class GEOGRAM_API ImageSerializer_xpm : public ImageSerializer {
    public:
        virtual Image* serialize_read(std::istream& stream);
        virtual bool read_supported() const;
        virtual bool binary() const;

	/**
	 * \brief Creates an image from XPM data.
	 * \param[in] s the string with the XPM data.
	 * \return the created image. 
	 */
	static Image* create_image_from_xpm_data(const char* s);

    protected:
	
	static Image* serialize_read_static(std::istream& stream);

        static Image* read_xpm_1_byte_per_pixel(
            int width, int height, int num_colors,
            std::istream& input
        );

        static Image* read_xpm_2_bytes_per_pixel(
            int width, int height, int num_colors,
            std::istream& input
        );

        static char* next_xpm_data(std::istream& input);
    };

//_________________________________________________________

}
#endif

