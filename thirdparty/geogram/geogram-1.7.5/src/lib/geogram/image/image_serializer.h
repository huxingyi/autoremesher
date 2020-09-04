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

