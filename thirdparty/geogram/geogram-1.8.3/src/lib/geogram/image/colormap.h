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
        ~Colormap() override;

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

