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

