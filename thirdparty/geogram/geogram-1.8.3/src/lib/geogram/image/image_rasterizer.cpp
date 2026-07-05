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

#include <geogram/image/image_rasterizer.h>
#include <stack>

namespace GEO {

    ImageRasterizer::ImageRasterizer(
	Image* image
    ) : image_(image) {
	component_encoding_ = image_->component_encoding();
	nb_components_ = index_t(
	    Image::nb_components(image_->color_encoding())
	);
    }

    void ImageRasterizer::clear() {
	Memory::clear(image_->base_mem(), image_->bytes());
    }

    void ImageRasterizer::triangle(
	const vec2i& P1, const Color& c1, 
	const vec2i& P2, const Color& c2, 
	const vec2i& P3, const Color& c3
    ) {
	// Find triangle's bounding box.
	int xmin = std::min(P1.x,std::min(P2.x,P3.x));
	int ymin = std::min(P1.y,std::min(P2.y,P3.y));
	int xmax = std::max(P1.x,std::max(P2.x,P3.x));
	int ymax = std::max(P1.y,std::max(P2.y,P3.y));
	
	geo_clamp(xmin, 0, int(image_->width()-1));
	geo_clamp(xmax, 0, int(image_->width()-1));
	geo_clamp(ymin, 0, int(image_->height()-1));
	geo_clamp(ymax, 0, int(image_->height()-1));

	int D = (P2.x - P1.x) * (P3.y - P1.y) - (P2.y - P1.y) * (P3.x - P1.x);

	if(D == 0) {
	    return;
	}

	// Iterative computation of barycentric coordinates.
	
	int cxl1 = P2.y - P3.y;
	int cxl2 = P3.y - P1.y;
	int cxl3 = P1.y - P2.y;
	
	int cyl1 = P3.x - P2.x;
	int cyl2 = P1.x - P3.x;
	int cyl3 = P2.x - P1.x;
	
	int c0l1 = P2.x*P3.y-P3.x*P2.y;
	int c0l2 = P3.x*P1.y-P1.x*P3.y;
	int c0l3 = P1.x*P2.y-P2.x*P1.y;
	
	int row_l1 = xmin * cxl1 + ymin * cyl1 + c0l1;
	int row_l2 = xmin * cxl2 + ymin * cyl2 + c0l2;
	int row_l3 = xmin * cxl3 + ymin * cyl3 + c0l3;

	for(int y=ymin; y<ymax; ++y) {
	    int l1 = row_l1;
	    int l2 = row_l2;
	    int l3 = row_l3;
	    for(int x=xmin; x<xmax; ++x) {
		if( 
		    (D > 0 && l1 >= 0.0 && l2 >= 0.0 && l3 >= 0.0) ||
		    (D < 0 && l1 <= 0.0 && l2 <= 0.0 && l3 <= 0.0)  
		) {
		    Color c;
		    interpolate_color(
			c1,c2,c3,
			double(l1)/double(D),
			double(l2)/double(D),
			double(l3)/double(D),
			c
		    );
		    set_pixel(x,y,c);
		}
		l1 += cxl1;
		l2 += cxl2;
		l3 += cxl3;
	    }
	    row_l1 += cyl1;
	    row_l2 += cyl2;
	    row_l3 += cyl3;
	}
    }


    void ImageRasterizer::segment(
        const vec2i& P1, const vec2i& P2, const Color& c
    ) {
        // Bresenham line drawing
        int dy = int(P2.y - P1.y);
        int sy = 1;
        if(dy < 0) {
            sy = -1;
            dy = -dy;
        }

        int dx = int(P2.x - P1.x);
        int sx = 1;
        if(dx < 0) {
            sx = -1;
            dx = -dx;
        }

        int x = int(P1.x);
        int y = int(P1.y);
        if(dy > dx) {
            int ex = (dx << 1) - dy;
            for(int u=0; u<dy; u++) {
                set_pixel(x,y,c);
                y += sy;
                while(ex >= 0)  {
                    set_pixel(x,y,c);		
                    x += sx;
                    ex -= dy << 1;
                }
                ex += dx << 1;
            }
        } else {
            int ey = (dy << 1) - dx;
            for(int u=0; u<dx; u++) {
                set_pixel(x,y,c);
                x += sx;
                while(ey >= 0) {
                    set_pixel(x,y,c);
                    y += sy;
                    ey -= dx << 1;
                }
                ey += dy << 1;
            }
        }
    }

    void ImageRasterizer::fillcircle(
        const vec2i& C, int R, const Color& c
    ) {
        // TODO if need be: more efficient algorithm using
        // Bresenham for circles
        
        int x1 = std::max(C.x-R,0);
        int y1 = std::max(C.y-R,0);
        int x2 = std::min(C.x+R,int(image_->width()-1));
        int y2 = std::min(C.y+R,int(image_->height()-1));

        for(int y=y1; y<y2; ++y) {
            for(int x=x1; x<x2; ++x) {
                if((x-C.x)*(x-C.x)+(y-C.y)*(y-C.y) <= R*R) {
                    set_pixel(x,y,c);
                }
            }
        }
    }
    
    void ImageRasterizer::flood_fill(int x, int y, const Color& c) {
        // TODO if need be: more efficient flood fill using scanline
        if(!pixel_is_black(x,y)) {
            return;
        }
        std::stack<vec2i> S;
        set_pixel(x,y,c);
        S.push(vec2i(x,y));
        while(!S.empty()) {
            vec2i p = S.top();
            S.pop();
            if(p.x > 0 && pixel_is_black(p.x-1,p.y)) {
                set_pixel(p.x-1,p.y,c);
                S.push(vec2i(p.x-1,p.y));
            }
            if(p.x < int(image_->width()-1) && pixel_is_black(p.x+1,p.y)) {
                set_pixel(p.x+1,p.y,c);
                S.push(vec2i(p.x+1,p.y));
            }
            if(p.y > 0 && pixel_is_black(p.x,p.y-1)) {
                set_pixel(p.x,p.y-1,c);
                S.push(vec2i(p.x,p.y-1));
            }
            if(p.y < int(image_->height()-1) && pixel_is_black(p.x,p.y+1)) {
                set_pixel(p.x,p.y+1,c);
                S.push(vec2i(p.x,p.y+1));
            }
        }
    }
    
}

