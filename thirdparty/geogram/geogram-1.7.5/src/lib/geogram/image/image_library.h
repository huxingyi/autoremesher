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
 
#ifndef H_OGF_IMAGE_TYPES_IMAGE_LIBRARY_H
#define H_OGF_IMAGE_TYPES_IMAGE_LIBRARY_H

#include <geogram/basic/common.h>
#include <geogram/image/image.h>
#include <geogram/image/image_serializer.h>

#include <geogram/basic/environment.h>

#include <string>
#include <map>

/**
 * \file geogram/image/image_library.h
 * \brief Management of image serializers and named images.
 */

namespace GEO {

//_________________________________________________________

    class ImageSerializer;
    class Image;

    /**
     * \brief Manages the ImageSerializer repository and
     *  the named images.
     */
    class GEOGRAM_API ImageLibrary : public Environment {
    public:

        /**
         * \brief Gets the instance.
         * \return a pointer to the unique instance of ImageLibrary.
         */
        static ImageLibrary* instance();

        /**
         * \brief Initializes the ImageLibrary instance.
         * \details This function is automatically called during
         *  Geogram startup. It should not be called by client
         *  code.
         */
        static void initialize();

        /**
         * \brief Terminates the ImageLibrary instance.
         * \details This function is automatically called during
         *  Geogram shutdown. It should not be called by client
         *  code.
         */
        static void terminate();

        /**
         * \brief Binds an ImageSerializer.
         * \param[in] extension the file extension without the "."
         * \param[in] serializer a pointer to an ImageSerializer. Ownership
         *  is transferred to this ImageLibrary
         * \retval true if the ImageSerializer could be successfully bound
         * \retval false otherwise (i.e. if there was already a serializer
         *  with the same name).
         */
        bool bind_image_serializer(
            const std::string& extension, ImageSerializer* serializer
        );

        /**
         * \brief Finds an ImageSerializer by extension.
         * \param[in] extension the file extension without the "."
         * \return a pointer to the ImageSerializer that can serialize
         *  images in the file format that corresponds to the extension,
         *  or nil if no such ImageSerializer was found
         */
        ImageSerializer* resolve_image_serializer(
            const std::string& extension
        ) const;

        /**
         * \brief Binds an image with a name.
         * \param[in] name the name of the image
         * \param[in] image a pointer to the image to be bound. Ownership
         *  is transferred to this ImageLibrary.
         * \retval true if the Image could be successfully bound
         * \retval false otherwise, i.e. if there was already an Image
         *  bound with the same name
         */
        bool bind_image(const std::string& name, Image* image);

        /**
         * \brief Unbinds a named image.
         * \param[in] name the name of the image
         * \retval true if the Image could be successfully unbound
         * \retval false otherwise, i.e. if there was no Image
         *  bound with the specified name
         */
        bool unbind_image(const std::string& name);

        /**
         * \brief Finds an image by name.
         * \param[in] name the name of the image
         * \return a pointer to the image, or nil if no image
         *  is bound to the name
         * \see bind_image(), unbind_image()
         */
        Image* resolve_image(const std::string& name) const;

        /**
         * \brief Loads an image from a file.
         * \param[in] file_name the name of the file that contains the image
         * \return a pointer to the loaded image, or nil if the image could
         *  not be loaded.
         * \note The returned pointer can be stored in an Image_var
         */
        Image* load_image(const std::string& file_name);

        /**
         * \brief Saves an image into a file.
         * \param[in] file_name the name of the file that will receive the
         *  image
         * \param[in] image a pointer to the Image to be saved
         * \retval true if the image could be successfully saved
         * \retval false otherwise
         */
        bool save_image(const std::string& file_name, Image* image);

        /**
         * \brief Copies an image to the clipboard of the operating system.
         * \param[in] image the image to be copied to the clipboard.
         * \note Only implemented under Windows
         */
        void copy_image_to_clipboard(Image* image);
        
        /**
         * \copydoc Environment::get_local_value()
         * \details Provides the following environment variables:
         *  - image_read_extensions
         *  - image_write_extensions
         */
         bool get_local_value(
            const std::string& name, std::string& value
        ) const override;

        /**
         * \copydoc Environment::set_local_value()
         */
         bool set_local_value(
            const std::string& name, const std::string& value
        ) override;
        
    protected:
        ImageLibrary();
        ~ImageLibrary() override;
        friend class World;

    private:
        static ImageLibrary* instance_;
        std::map<std::string, ImageSerializer_var> image_serializers_;
        std::map<std::string, Image_var> images_;
    };

//_________________________________________________________

    /**
     * \brief Declares an image serializer for a given extension.
     * \tparam T the type of the ImageSerializer
     */
    template <class T> class geo_declare_image_serializer {
    public:
        /**
         * \brief Declares an image serializer for a given extension.
         * \param[in] extension the extension of the image file names
         *  without the "."
         * \details This function is supposed to be used in the
         *  initializers of the libraries. An example of usage:
         * \code
         *   ogf_declare_image_serializer<ImageSerializer_png>("png");
         * \endcode
         */
        geo_declare_image_serializer(const std::string& extension) {
            ImageLibrary::instance()->bind_image_serializer(
                extension, new T
            );
        }
    };

//_________________________________________________________

}
#endif

