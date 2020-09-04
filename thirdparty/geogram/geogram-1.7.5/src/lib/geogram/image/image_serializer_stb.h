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
 
#ifndef H_OGF_IMAGE_IO_IMAGE_SERIALIZER_STB_H
#define H_OGF_IMAGE_IO_IMAGE_SERIALIZER_STB_H

#include <geogram/basic/common.h>
#include <geogram/image/image_serializer.h>

/**
 * \file geogram/image/image_serializer_stb.h
 * \brief Implementation of image serializer using
 *   stb_image.h and stb_image_write.h
 */

namespace GEO {

//_________________________________________________________


    class Image;

    /**
     * \brief Loads and saves images.
     */
    class GEOGRAM_API ImageSerializerSTB : public ImageSerializer {
    public:

	/**
	 * \brief ImageSerializerSTB constructor.
	 * \param[in] read true if reading is supported
	 * \param[in] write true if writing is supported
	 */
        ImageSerializerSTB(bool read, bool write);
	
	/**
	 * \copydoc ImageSerializer::read()
	 */
        Image* serialize_read(const std::string& file_name) override;

	/**
	 * \copydoc ImageSerializer::write()
	 */
        bool serialize_write(
            const std::string& file_name, const Image* image
        ) override;

        /**
	 * \copydoc ImageSerializer::binary()
         */
        bool binary() const override;

        /**
	 * \copydoc ImageSerializer::streams_supported()
	 * \details This version returns false
         */
        bool streams_supported() const override;

        /**
	 * \copydoc ImageSerializer::read_supported()
         */
        bool read_supported() const override;

        /**
	 * \copydoc ImageSerializer::write_supported()
         */
        bool write_supported() const override;

      private:
	bool read_;
	bool write_;
    }; 

    /**
     * \brief An image serializer that can read images.
     */
    class GEOGRAM_API ImageSerializerSTBRead : public ImageSerializerSTB {
      public:
	/**
	 * \brief ImageSerializerSTBRead constructor.
	 */
	 ImageSerializerSTBRead();

	/**
	 * \brief ImageSerializerSTBRead destructor.
	 */
	 ~ImageSerializerSTBRead() override;
    };

    /**
     * \brief An image serializer that can read and write images.
     */
    class GEOGRAM_API ImageSerializerSTBReadWrite : public ImageSerializerSTB {
      public:
	/**
	 * \brief ImageSerializerSTBReadWrite constructor.
	 */
	ImageSerializerSTBReadWrite();

	/**
	 * \brief ImageSerializerSTBReadWrite destructor.
	 */
	~ImageSerializerSTBReadWrite() override;	
    };
    
//***************************************************************************

}
#endif

