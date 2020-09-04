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
 
#ifndef H_GEO_IMAGE_TYPES_COLORMAP_H
#define H_GEO_IMAGE_TYPES_COLORMAP_H

#include <geogram/basic/common.h>
#include <geogram/basic/counted.h>
#include <geogram/basic/smart_pointer.h>
#include <geogram/image/color.h>

/**
 * \file geogram/image/colormap.h
 * \brief Colormap type.
 */
namespace GEO {

/******************************************************************/

    /**
     * \brief A Colormap.
     * \details A Colormap is an array of colors,
     *  encoded with one byte per component.
     */
    class GEOGRAM_API Colormap : public Counted {
    public:

        /**
         * \brief Type of each cell of the Colormap.
         */
        typedef GenColor<Numeric::uint8> ColorCell ;

        /**
         * \brief Colormap constructor.
         * \param[in] size_in number of cells in the Colormap
         */
        Colormap(index_t size_in = 256) ;

        /**
         * \brief Colormap destructor.
         */
        virtual ~Colormap() ;

        /**
         * \brief Gets a ColorCell by index.
         * \param[in] index the index of the ColorCell
         * \return a const reference to the ColorCell
         * \pre index < size()
         */
        const ColorCell& color_cell(index_t index) const {
            geo_assert(index < size_) ;
            return cells_[index] ;
        }

        /**
         * \brief Gets a ColorCell by index.
         * \param[in] index the index of the ColorCell
         * \return a modifiable reference to the ColorCell
         * \pre index < size()
         */
        ColorCell& color_cell(index_t index) {
            geo_assert(index < size_) ;
            return cells_[index] ;
        }

        /**
         * \brief Gets the size.
         * \return the number of ColorCells in this Colormap
         */
        index_t size() const {
            return size_;
        }

        /**
         * \brief Sets a color by index and components.
         * \details The transparency a is left unchanged.
         * \param[in] index the index
         * \param[in] r , g , b the components, as single-precision
         *  floating points, between 0.0f, and 1.0f
         */
        void set_color(index_t index, float r, float g, float b) ;

        /**
         * \brief Sets a color by index and components.
         * \param[in] index the index
         * \param[in] r , g , b , a the components, as single-precision
         *  floating points, between 0.0f, and 1.0f
         */
        void set_color(index_t index, float r, float g, float b, float a) ;

        /**
         * \brief Make a color component linearly interpolate two values
         *  between two given indices.
         * \param[in] component the index of the component, one of 0,1,2,3
         * \param[in] index1 the first index
         * \param[in] val1 the first value of the component, associated 
         *  with \p index1
         * \param[in] index2 the second index
         * \param[in] val2 the second value of the component, associated 
         *  with \p index2
         */
        void color_ramp_component(
            index_t component,
            index_t index1, Numeric::uint8 val1,
            index_t index2, Numeric::uint8 val2
        ) ;


        /**
         * \brief Make a color linearly interpolate two values
         *  between two given indices.
         * \details All components and transparency are updated.
         * \param[in] index1 the first index
         * \param[in] c1 the first color, associated with \p index1
         * \param[in] index2 the second index
         * \param[in] c2 the first color, associated with \p index2
         */
        void color_ramp_rgba(
            index_t index1, const Color& c1,
            index_t index2, const Color& c2
        ) ;

        /**
         * \brief Make a color linearly interpolate two values
         *  between two given indices.
         * \details Only r,g,b are updated. Transparency a is left
         *  unmodified in the concerned color cells.
         * \param[in] index1 the first index
         * \param[in] c1 the first color, associated with \p index1
         * \param[in] index2 the second index
         * \param[in] c2 the first color, associated with \p index2
         */
        void color_ramp_rgb(
            index_t index1, const Color& c1,
            index_t index2, const Color& c2
        ) ;

    private:
        ColorCell* cells_ ;
        index_t size_ ;
    } ;

    /**
     * \brief An automatic reference-counted pointer to a Colormap.
     */
    typedef SmartPointer<Colormap> Colormap_var ;

/************************************************************************/

}
#endif

