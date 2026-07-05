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
 
#include <geogram/image/colormap.h>

namespace GEO {

//_________________________________________________________

    Colormap::Colormap(index_t size_in) : size_(size_in) {
        cells_ = new ColorCell[size_in] ;
    }

    Colormap::~Colormap() {
        delete[] cells_ ;
    }


    void Colormap::color_ramp_component(
        index_t component,
        index_t index1, Numeric::uint8 alpha1,
        index_t index2, Numeric::uint8 alpha2
    ) {
        if(index1 == index2) {
            color_cell(index2)[component] = alpha2 ;
        } else {
            int n = std::abs(int(index2) - int(index1)) ;
            float delta = (float(alpha2) - float(alpha1)) / float(n) ;
            int sgn = geo_sgn(int(index2) - int(index1)) ;
            float alpha = alpha1 ;
            int index = int(index1) ;
            for(int i=0 ; i<=n ; i++) {
                color_cell(index_t(index))[component] =
                    Numeric::uint8(alpha) ;
                index += sgn ;
                alpha += delta ;
            }
        }
    }


    void Colormap::color_ramp_rgba(
        index_t index1, const Color& c1,
        index_t index2, const Color& c2
    ) {
        if(index1 == index2) {
            Colormap::ColorCell c(
                Numeric::uint8(c2.r() * 255.0),
                Numeric::uint8(c2.g() * 255.0),
                Numeric::uint8(c2.b() * 255.0),
                Numeric::uint8(c2.a() * 255.0)
            ) ; 
            color_cell(index2) = c ;
        } else {
        
            int n = std::abs(int(index2) - int(index1)) ;
            int sgn = geo_sgn(int(index2) - int(index1)) ;
        
            float r = float(c1.r()) ;
            float g = float(c1.g()) ;
            float b = float(c1.b()) ;
            float a = float(c1.a()) ;
            float dr = float(c2.r() - c1.r()) / float(n) ;
            float dg = float(c2.g() - c1.g()) / float(n) ;
            float db = float(c2.b() - c1.b()) / float(n) ;
            float da = float(c2.a() - c1.a()) / float(n) ;
            int index = int(index1) ;

            for(int i=0 ; i<=n ; i++) {
                set_color(index_t(index), r, g, b, a) ;
                index += sgn ;
                r += dr ;
                g += dg ;
                b += db ;
                a += da ;
            }
        }
    }


    void Colormap::color_ramp_rgb(
        index_t index1, const Color& c1,
        index_t index2, const Color& c2
    ) {
        if(index1 == index2) {
            Colormap::ColorCell c(
                Numeric::uint8(c2.r() * 255.0),
                Numeric::uint8(c2.g() * 255.0),
                Numeric::uint8(c2.b() * 255.0),
                color_cell(index2).a()
            ) ; 
            color_cell(index2) = c ;
        } else {
        
            int n = std::abs(int(index2) - int(index1)) ;
            int sgn = geo_sgn(int(index2) - int(index1)) ;
        
            float r = float(c1.r()) ;
            float g = float(c1.g()) ;
            float b = float(c1.b()) ;

            float dr = (float(c2.r()) - float(c1.r())) / float(n);
            float dg = (float(c2.g()) - float(c1.g())) / float(n);
            float db = (float(c2.b()) - float(c1.b())) / float(n);
            int index = int(index1) ;

            for(int i=0 ; i<=n ; i++) {
                set_color(index_t(index), r, g, b) ;
                index += sgn ;
                r += dr ;
                g += dg ;
                b += db ;
            }
        }
    }


    void Colormap::set_color(index_t index, float r, float g, float b) {
        r *= 255.0f ;
        g *= 255.0f ;
        b *= 255.0f ;
        geo_clamp(r, float(0), float(255)) ;
        geo_clamp(g, float(0), float(255)) ;
        geo_clamp(b, float(0), float(255)) ;
        Colormap::ColorCell c = Colormap::ColorCell(
            Numeric::uint8(r),
            Numeric::uint8(g),
            Numeric::uint8(b),
            color_cell(index).a()
        ) ; 
        color_cell(index) = c ;
    }

    void Colormap::set_color(
        index_t index, float r, float g, float b, float a
    ) {
        r *= 255.0f ;
        g *= 255.0f ;
        b *= 255.0f ;
        a *= 255.0f ;
        geo_clamp(r, float(0), float(255)) ;
        geo_clamp(g, float(0), float(255)) ;
        geo_clamp(b, float(0), float(255)) ;
        geo_clamp(a, float(0), float(255)) ;
        Colormap::ColorCell c = Colormap::ColorCell(
            Numeric::uint8(r),
            Numeric::uint8(g),
            Numeric::uint8(b),
            Numeric::uint8(a)
        ) ; 
        color_cell(index) = c ;
    }


//_________________________________________________________

}

