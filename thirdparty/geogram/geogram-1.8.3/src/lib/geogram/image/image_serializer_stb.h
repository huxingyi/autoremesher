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

