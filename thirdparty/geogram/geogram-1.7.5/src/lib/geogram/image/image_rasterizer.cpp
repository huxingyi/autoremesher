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

#include <geogram/image/image_rasterizer.h>

namespace GEO {

    ImageRasterizer::ImageRasterizer(
	Image* image, double x1, double y1, double x2, double y2
    ) : image_(image) {
	x1_ = x1;
	y1_ = y1;
	x2_ = x2;
	y2_ = y2;
	component_encoding_ = image_->component_encoding();
	nb_components_ = index_t(
	    Image::nb_components(image_->color_encoding())
	);
    }

    void ImageRasterizer::clear() {
	Memory::clear(image_->base_mem(), image_->bytes());
    }

    void ImageRasterizer::triangle(
	const vec2& p1, const Color& c1, 
	const vec2& p2, const Color& c2, 
	const vec2& p3, const Color& c3
    ) {
	vec2i P1,P2,P3;
	
	transform(p1,P1);
	transform(p2,P2);
	transform(p3,P3);

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
    
}

