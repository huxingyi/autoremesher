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
 
#ifndef H_OGF_IMAGE_TYPES_IMAGE_H
#define H_OGF_IMAGE_TYPES_IMAGE_H

#include <geogram/basic/common.h>
#include <geogram/image/colormap.h>

/**
 * \file geogram/image/image.h
 * \details Types for images.
 */

namespace GEO {

//_________________________________________________________

    
    /**
     * \brief An image.
     */
    class GEOGRAM_API Image : public Counted {
    public:

        /**
         * \brief Indicates how colors are encoded
         *  within the image.
         */
        enum ColorEncoding {
            GRAY, INDEXED, RGB, BGR, RGBA, YUV
        };

        /**
         * \brief Indicates the datatype used to 
         *  encode each component of the colors.
         */
        enum ComponentEncoding {
            BYTE, INT16, INT32, FLOAT32, FLOAT64
        };


        /**
         * \brief Image constructor.
         * \details Constructs an uninitialized Image.
         */
        Image();

        /**
         * \brief Image constructor.
         * \param[in] color_rep the ColorEncoding
         * \param[in] component_rep the ComponentEncoding
         * \param[in] width the width of the image
         * \param[in] height the height of the image, or 1 for 1D images
         * \param[in] depth the depth of the image for 3D images, or 1
         *  for 1D and 2D images
         */
        Image(
            ColorEncoding color_rep, ComponentEncoding component_rep,
            index_t width, index_t height=1, index_t depth=1
        ) {
            base_mem_ = nullptr;
            initialize(color_rep, component_rep, width, height, depth);
        }

        /**
         * \brief Image destructor.
         */
        virtual ~Image();

        /**
         * \brief Some implementations get the image from some sources.
         *  Default implementation does nothing.
         * \details There is a derived class to access the webcam for
         *  instance.
         */
        virtual void acquire();

        /**
         * \brief Gets the dimension of the image.
         * \retval 1 for 1D images
         * \retval 2 for 2D images
         * \retval 3 for 3D images
         */
        index_t dimension() const {
            return dimension_;
        }

        /**
         * \brief Gets the size of the image along one of the axes.
         * \param[in] axis the axis, one of (0,1,2)
         * \return the number of pixels along axis
         */
        index_t size(index_t axis) const { 
            geo_assert(axis < 3);
            return size_[axis];
        }
        
        /**
         * \brief Gets the width of the image.
         * \return the width of the image, in pixels
         */
        index_t width() const  {
            return size_[0];
        }

        /**
         * \brief Gets the height of the image.
         * \return the height of the image, in pixels, or 1
         *  for 1D images
         */
        index_t height() const  {
            return size_[1];
        }

        /**
         * \brief Gets the depth of the image.
         * \return for 3D images, the depth of the image in pixels, 
         *  or 1 for 1D and 2D images.
         */
        index_t depth() const  {
            return size_[2];
        }

        /**
         * \brief Gets the number of bytes per pixel.
         * \return the number of bytes used to store the color of one pixel.
         */
        size_t bytes_per_pixel() const {
            return bytes_per_pixel_;
        }

	/**
	 * \brief Gets the number of components per pixel.
	 * \return the number of color components in each pixel.
	 */
	size_t components_per_pixel() const {
	    return nb_components(color_encoding());
	}
	
        /**
         * \brief Gets the number of pixels.
         * \return the total number of pixels in this image
         */
        size_t nb_pixels() const {
            return size_t(size_[0]) * size_t(size_[1]) * size_t(size_[2]);
        }

        /**
         * \brief Gets the number of bytes.
         * \return the total number of bytes used to store the color
         *  data of this image
         */
        size_t bytes() const {
            return nb_pixels() * bytes_per_pixel();
        }

        /**
         * \brief Gets the ComponentEncoding.
         * \return the ComponentEncoding
         */
        ComponentEncoding component_encoding() const {
            return component_encoding_;
        }

        /**
         * \brief Gets the ColorEncoding.
         * \return the ColorEncoding
         */
        ColorEncoding color_encoding() const {
            return color_encoding_;
        }

        /**
         * \brief Gets the Colormap
         * \return a const pointer to the Colormap.
         */
        const Colormap* colormap() const {
            return colormap_;
        }

        /**
         * \brief Gets the Colormap
         * \return a pointer to the Colormap.
         */
        Colormap* colormap()             {
            return colormap_;
        }

        /**
         * \brief Sets the Colormap
         * \param[in] colormap a pointer to the Colormap, 
         *  ownership is transfered to this Image
         */
        void set_colormap(Colormap* colormap) {
            colormap_ = colormap; 
        }

        /**
         * \brief Gets the base memory.
         * \return a pointer to the color data associated
         *  with this image.
         */
        Memory::pointer base_mem() const {
            return base_mem_;
        }

        /**
         * \brief Gets the base memory as a byte pointer.
         * \return a byte pointer to the color data associated
         *  with this image.
         * \pre ComponentEncoding == BYTE
         */
        Memory::byte* base_mem_byte_ptr() const {
            return byte_ptr(base_mem_);
        }

        /**
         * \brief Gets the base memory as a 16 bits integer pointer.
         * \return a 16 bits integer pointer to the color data associated
         *  with this image.
         * \pre ComponentEncoding == INT16
         */
        Numeric::int16* base_mem_int16_ptr() const {
            return int16_ptr(base_mem_);
        }

        /**
         * \brief Gets the base memory as a 32 bits integer pointer.
         * \return a 32 bits integer pointer to the color data associated
         *  with this image.
         * \pre ComponentEncoding == FLOAT32
         */
        Numeric::int32* base_mem_int32_ptr() const {
            return int32_ptr(base_mem_);
        }

        /**
         * \brief Gets the base memory as a 32 bits floating point pointer.
         * \return a 32 bits floating point pointer to the color data associated
         *  with this image.
         * \pre ComponentEncoding == FLOAT32
         */
        Numeric::float32* base_mem_float32_ptr() const {
            return float32_ptr(base_mem_);
        }

        /**
         * \brief Gets the base memory as a 64 bits floating point pointer.
         * \return a 64 bits floating point pointer to the color data associated
         *  with this image.
         * \pre ComponentEncoding == FLOAT64
         */
        Numeric::float64* base_mem_float64_ptr() const {
            return float64_ptr(base_mem_);
        }

        /**
         * \brief Gets the address of a pixel in a 1D image.
         * \param[in] x the x coordinate of the pixel
         * \return a pointer to the color data associated with the pixel
         * \pre x < width()
         */
        Memory::pointer pixel_base(index_t x) {
            return base_mem() + x * factor_[0];
        }


        /**
         * \brief Gets the address of a pixel in a 1D image as a byte pointer.
         * \param[in] x the x coordinate of the pixel
         * \return a pointer to the color data associated with the pixel
         * \pre x < width() && COMPONENT_ENCODING == BYTE
         */
        Memory::byte* pixel_base_byte_ptr(index_t x) {
            return byte_ptr(base_mem() + x * factor_[0]);
        }

        /**
         * \brief Gets the address of a pixel in a 1D image as a int16 pointer.
         * \param[in] x the x coordinate of the pixel
         * \return a pointer to the color data associated with the pixel
         * \pre x < width() && COMPONENT_ENCODING == INT16
         */
	Numeric::int16* pixel_base_int16_ptr(index_t x) {
            return int16_ptr(base_mem() + x * factor_[0]);
        }

        /**
         * \brief Gets the address of a pixel in a 1D image as a int32 pointer.
         * \param[in] x the x coordinate of the pixel
         * \return a pointer to the color data associated with the pixel
         * \pre x < width() && COMPONENT_ENCODING == INT32
         */
	Numeric::int32* pixel_base_int32_ptr(index_t x) {
            return int32_ptr(base_mem() + x * factor_[0]);
        }

        /**
         * \brief Gets the address of a pixel in a 1D image as a float32 pointer.
         * \param[in] x the x coordinate of the pixel
         * \return a pointer to the color data associated with the pixel
         * \pre x < width() && COMPONENT_ENCODING == FLOAT32
         */
	Numeric::float32* pixel_base_float32_ptr(index_t x) {
            return float32_ptr(base_mem() + x * factor_[0]);
        }

        /**
         * \brief Gets the address of a pixel in a 1D image as a float64 pointer.
         * \param[in] x the x coordinate of the pixel
         * \return a pointer to the color data associated with the pixel
         * \pre x < width() && COMPONENT_ENCODING == FLOAT64
         */
        Numeric::float64* pixel_base_float64_ptr(index_t x) {
            return float64_ptr(base_mem() + x * factor_[0]);
        }
	
	
        /**
         * \brief Gets the address of a pixel in a 2D image.
         * \param[in] x , y the coordinates of the pixel
         * \return a pointer to the color data associated with the pixel
         * \pre x < width() && y < height()
         */
        Memory::pointer pixel_base(index_t x, index_t y) {
            return base_mem() + x * factor_[0] + y * factor_[1];
        }

        /**
         * \brief Gets the address of a pixel in a 2D image as a byte pointer.
         * \param[in] x , y the coordinates of the pixel
         * \return a pointer to the color data associated with the pixel
         * \pre x < width() && y < height() && component_encoding() && BYTE
         */
        Memory::byte* pixel_base_byte_ptr(index_t x, index_t y) {
            return byte_ptr(base_mem() + x * factor_[0] + y * factor_[1]);
        }

        /**
         * \brief Gets the address of a pixel in a 2D image as an int16 pointer.
         * \param[in] x , y the coordinates of the pixel
         * \return a pointer to the color data associated with the pixel
         * \pre x < width() && y < height() && component_encoding() && INT16
         */
	Numeric::int16* pixel_base_int16_ptr(index_t x, index_t y) {
            return int16_ptr(base_mem() + x * factor_[0] + y * factor_[1]);
        }

        /**
         * \brief Gets the address of a pixel in a 2D image as an int32 pointer.
         * \param[in] x , y the coordinates of the pixel
         * \return a pointer to the color data associated with the pixel
         * \pre x < width() && y < height() && component_encoding() && INT32
         */
	Numeric::int32* pixel_base_int32_ptr(index_t x, index_t y) {
            return int32_ptr(base_mem() + x * factor_[0] + y * factor_[1]);
        }

        /**
         * \brief Gets the address of a pixel in a 2D image as a float32 pointer.
         * \param[in] x , y the coordinates of the pixel
         * \return a pointer to the color data associated with the pixel
         * \pre x < width() && y < height() && component_encoding() && FLOAT32
         */
	Numeric::float32* pixel_base_float32_ptr(index_t x, index_t y) {
            return float32_ptr(base_mem() + x * factor_[0] + y * factor_[1]);
        }

        /**
         * \brief Gets the address of a pixel in a 2D image as a float64 pointer.
         * \param[in] x , y the coordinates of the pixel
         * \return a pointer to the color data associated with the pixel
         * \pre x < width() && y < height() && component_encoding() && FLOAT64
         */
	Numeric::float64* pixel_base_float64_ptr(index_t x, index_t y) {
            return float64_ptr(base_mem() + x * factor_[0] + y * factor_[1]);
        }
	
        /**
         * \brief Gets the address of a pixel in a 3D image.
         * \param[in] x , y , z the coordinates of the pixel
         * \return a pointer to the color data associated with the pixel
         * \pre x < width() && y < height() && z < depth()
         */
        Memory::pointer pixel_base(index_t x, index_t y, index_t z) {
            return base_mem() + 
                x * factor_[0] + y * factor_[1] + z * factor_[2];
        }

        /**
         * \brief Gets the address of a pixel in a 3D image as a byte pointer.
         * \param[in] x , y , z the coordinates of the pixel
         * \return a pointer to the color data associated with the pixel
         * \pre x < width() && y < height() && z < depth() && 
	 *    component_encoding() && BYTE
         */
        Memory::byte* pixel_base_byte_ptr(index_t x, index_t y, index_t z) {
            return byte_ptr(base_mem() + 
                x * factor_[0] + y * factor_[1] + z * factor_[2]
	    );
        }

        /**
         * \brief Gets the address of a pixel in a 3D image as an int16 pointer.
         * \param[in] x , y , z the coordinates of the pixel
         * \return a pointer to the color data associated with the pixel
         * \pre x < width() && y < height() && z < depth() && 
	 *    component_encoding() && INT16
         */
        Numeric::int16* pixel_base_int16_ptr(index_t x, index_t y, index_t z) {
            return int16_ptr(base_mem() + 
                x * factor_[0] + y * factor_[1] + z * factor_[2]
	    );
        }

        /**
         * \brief Gets the address of a pixel in a 3D image as an int32 pointer.
         * \param[in] x , y , z the coordinates of the pixel
         * \return a pointer to the color data associated with the pixel
         * \pre x < width() && y < height() && z < depth() && 
	 *    component_encoding() && INT32
         */
        Numeric::int32* pixel_base_int32_ptr(index_t x, index_t y, index_t z) {
            return int32_ptr(base_mem() + 
                x * factor_[0] + y * factor_[1] + z * factor_[2]
	    );
        }

        /**
         * \brief Gets the address of a pixel in a 3D image as a float32 
	 *  pointer.
         * \param[in] x , y , z the coordinates of the pixel
         * \return a pointer to the color data associated with the pixel
         * \pre x < width() && y < height() && z < depth() && 
	 *    component_encoding() && FLOAT32
         */
        Numeric::float32* pixel_base_float32_ptr(
	    index_t x, index_t y, index_t z
	) {
            return float32_ptr(base_mem() + 
                x * factor_[0] + y * factor_[1] + z * factor_[2]
	    );
        }

        /**
         * \brief Gets the address of a pixel in a 3D image as a float64
	 *  pointer.
         * \param[in] x , y , z the coordinates of the pixel
         * \return a pointer to the color data associated with the pixel
         * \pre x < width() && y < height() && z < depth() && 
	 *    component_encoding() && FLOAT64
         */
        Numeric::float64* pixel_base_float64_ptr(
	    index_t x, index_t y, index_t z
	) {
            return float64_ptr(base_mem() + 
                x * factor_[0] + y * factor_[1] + z * factor_[2]
	    );
        }
	
        /**
         * \brief Gets the number of components associated with 
         *  a ColorEncoding.
         * \param[in] color_rep the ColorEncoding
         * \return the number of components used by \p color_rep
         */
        static size_t nb_components(ColorEncoding color_rep);

        /**
         * \brief Gets the number of bytes used by a ComponentEncoding.
         * \param[in] component_rep the ComponentEncoding
         * \return the number of bytes used to represent a color component
         *  encoded with \p component_rep
         */
        static size_t bytes_per_component(ComponentEncoding component_rep);

        /**
         * \brief Converts an untyped pointer into a byte pointer.
         * \param[in] ptr the pointer to be converted
         * \return pointer \p ptr converted to a byte pointer
         * \pre component_encoding_ == BYTE
         * \note This function does nothing else than casting the pointer. In 
         *  addition, in debug mode, it tests that the color encoding is the
         *  right one (and throws an assertion failure if it is not the case).
         */
        Memory::byte* byte_ptr(Memory::pointer ptr) const {
            geo_debug_assert(component_encoding_ == BYTE);
            return ptr;
        }

        /**
         * \brief Converts an untyped pointer into a 16 bits integer pointer.
         * \param[in] ptr the pointer to be converted
         * \return pointer \p ptr converted to a 16 bits integer pointer
         * \pre component_encoding_ == INT16
         * \note This function does nothing else than casting the pointer. In 
         *  addition, in debug mode, it tests that the color encoding is the
         *  right one (and throws an assertion failure if it is not the case).
         */
        Numeric::int16* int16_ptr(Memory::pointer ptr) const {
            geo_debug_assert(component_encoding_ == INT16);
            return (Numeric::int16*)(void*)(ptr);
        }

        /**
         * \brief Converts an untyped pointer into a 32 bits integer pointer.
         * \param[in] ptr the pointer to be converted
         * \return pointer \p ptr converted to a 32 bits integer pointer
         * \pre component_encoding_ == INT32
         * \note This function does nothing else than casting the pointer. In 
         *  addition, in debug mode, it tests that the color encoding is the
         *  right one (and throws an assertion failure if it is not the case).
         */
        Numeric::int32* int32_ptr(Memory::pointer ptr) const {
            geo_debug_assert(component_encoding_ == INT32);
            return (Numeric::int32*)(void*)(ptr);
        }

        /**
         * \brief Converts an untyped pointer into a 32 bits floating point
         *  pointer.
         * \param[in] ptr the pointer to be converted
         * \return pointer \p ptr converted to a 32 bits floating point pointer
         * \pre component_encoding_ == FLOAT32
         * \note This function does nothing else than casting the pointer. In 
         *  addition, in debug mode, it tests that the color encoding is the
         *  right one (and throws an assertion failure if it is not the case).
         */
        Numeric::float32* float32_ptr(Memory::pointer ptr) const {
            geo_debug_assert(component_encoding_ == FLOAT32);
            return (Numeric::float32*)(void*)(ptr);
        }

        /**
         * \brief Converts an untyped pointer into a 64 bits floating point
         *  pointer.
         * \param[in] ptr the pointer to be converted
         * \return pointer \p ptr converted to a 64 bits floating point pointer
         * \pre component_encoding_ == FLOAT64
         * \note This function does nothing else than casting the pointer. In 
         *  addition, in debug mode, it tests that the color encoding is the
         *  right one (and throws an assertion failure if it is not the case).
         */
        Numeric::float64* float64_ptr(Memory::pointer ptr) const {
            geo_debug_assert(component_encoding_ == FLOAT64);
            return (Numeric::float64*)(void*)(ptr);
        }

        /**
         * \brief Flips this image along the y axis.
         */
        void flip_vertically();

        /**
         * \brief Swaps two color components of this image.
         * \param[in] channel1 , channel2 the two channels to
         *  be swapped
         * \pre channel1 < nb_components(color_encoding()) &&
         *  channel2 < nb_components(color_encoding())
         */
        void swap_components(index_t channel1, index_t channel2);

        /**
         * \brief Creates storage for the specified encoding
         *  and image dimensions.
         * \param[in] color_rep the ColorEncoding
         * \param[in] component_rep the ComponentEncoding
         * \param[in] size_x the image width
         * \param[in] size_y the image height, or 1 for 1D images
         * \param[in] size_z the image depth for 3D images, or 1 for 1D and
         *  2D images
         */
        void initialize(
            ColorEncoding color_rep, ComponentEncoding component_rep,
            index_t size_x, index_t size_y=1, index_t size_z=1
        );

    protected:
        ColorEncoding color_encoding_;
        ComponentEncoding component_encoding_;
        Colormap_var colormap_;
        size_t factor_[3];
        Memory::pointer base_mem_;
        index_t dimension_;
        index_t size_[3];
        size_t bytes_per_pixel_;

    private:
        /**
         * \brief Forbids copy-construction.
         */
        Image(const Image& rhs);

        /**
         * \brief Forbids assignment operator.
         */
        Image& operator=(const Image& rhs);
    };

    typedef SmartPointer<Image> Image_var;

//_________________________________________________________

}
#endif

