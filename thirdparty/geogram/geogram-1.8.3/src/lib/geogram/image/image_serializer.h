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
 
#ifndef H_OGF_IMAGE_IO_IMAGE_SERIALIZER_H
#define H_OGF_IMAGE_IO_IMAGE_SERIALIZER_H

#include <geogram/basic/common.h>
#include <geogram/image/image.h>
#include <geogram/basic/counted.h>
#include <geogram/basic/smart_pointer.h>

#include <iostream>

/**
 * \file geogram/image/image_serializer.h
 * \brief Baseclass for loading and saving images.
 */

namespace GEO {

//_________________________________________________________


    class Image ;

    /**
     * \brief Loads and saves images.
     */
    class GEOGRAM_API ImageSerializer : public Counted {
    public:

        /**
         * \brief reads an image from a file.
         * \param[in] file_name the name of the file
         * \return a pointer to the loaded image or nil if
         *  the image could not be loaded
         * \note the loaded image can be stored in an Image_var
         */
        virtual Image* serialize_read(const std::string& file_name) ;

        /**
         * \brief writes an image into a file.
         * \param[in] file_name the name of the file
         * \param[in] image a pointer to the image to be saved
         * \retval true if the image could be saved
         * \retval false otherwise
         */
        virtual bool serialize_write(
            const std::string& file_name, const Image* image
        ) ;

        /**
         * \brief reads an image from a stream.
         * \param[in,out] stream the stream where the image should
         *  be read from
         * \return a pointer to the loaded image or nil if
         *  the image could not be loaded
         * \note the loaded image can be stored in an Image_var
         */
        virtual Image* serialize_read(std::istream& stream) ;

        /**
         * \brief writes an image into a stream.
         * \param[in,out] stream the stream where the image should be written
         * \param[in] image a pointer to the image to be saved
         * \retval true if the image could be saved
         * \retval false otherwise
         */
        virtual bool serialize_write(
            std::ostream& stream, const Image* image
        ) ;

        /**
         * \brief Tests whether the file format is binary or ASCII.
         * \details It is used to determine whether a stream should be
         *  opened in ASCII or binary mode for loading image files.
         *  Default implementation in base class returns true.
         * \retval true if the file format is binary
         * \retval false if the file format is ASCII
         */
        virtual bool binary() const ;

        /**
         * \brief Tests whether the functions that use streams
         *  are supported.
         * \details Default implementation in base class returns true.
         * \retval true if the functions that use streams are 
         *  supported
         * \retval false otherwise
         */
        virtual bool streams_supported() const ;

        /**
         * \brief Tests whether reading is implemented.
         * \details Some serializers implement only reading or only
         *  writing. Default implementation returns false
         * \retval true if reading is supported
         * \retval false otherwise
         */
        virtual bool read_supported() const ;

        /**
         * \brief Tests whether writing is implemented.
         * \details Some serializers implement only reading or only
         *  writing. Default implementation returns false
         * \retval true if writing is supported
         * \retval false otherwise
         */
        virtual bool write_supported() const ;
    } ; 

    /**
     * \brief An automatic reference-counted pointer to an ImageSerializer
     */
    typedef SmartPointer<ImageSerializer> ImageSerializer_var ;

//_________________________________________________________

}
#endif

