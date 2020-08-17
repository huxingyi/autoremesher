/*
 *  Copyright (c) 2012-2016, Bruno Levy
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
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     Bruno.Levy@inria.fr
 *     http://www.loria.fr/~levy
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 */

#include <geogram/parameterization/mesh_param_validator.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_geometry.h>
#include <algorithm>

namespace GEO {

    ParamValidator::ParamValidator() {
        graph_size_ = 1024;
        graph_mem_ = new Numeric::uint8[graph_size_ * graph_size_];
        x_left_  = new int[graph_size_];
        x_right_ = new int[graph_size_];
        max_overlap_ratio_ = 0.005;
        max_scaling_ = 20.0;
        min_fill_ratio_ = 0.25;
	verbose_ = false;
    }

    ParamValidator::~ParamValidator() {
        delete[] graph_mem_;
	graph_mem_ = nullptr;
        delete[] x_left_;
	x_left_ = nullptr;
        delete[] x_right_;
	x_right_ = nullptr;
    }

    bool ParamValidator::chart_is_valid(Chart& chart) {
	Attribute<double> tex_coord;
	tex_coord.bind_if_is_defined(
	    chart.mesh.facet_corners.attributes(), "tex_coord"
	);
	geo_assert(tex_coord.is_bound() && tex_coord.dimension() == 2);

	for(index_t ff=0; ff<chart.facets.size(); ++ff) {
	    index_t f = chart.facets[ff];
	    for(index_t c: chart.mesh.facets.corners(f)) {
		if(GEO::Numeric::is_nan(tex_coord[2*c]) ||
		   GEO::Numeric::is_nan(tex_coord[2*c+1])) {
		    if(verbose_) {
			Logger::out("ParamValidator")
			    << "NaN detected in tex coords" << std::endl;
		    }
		    return false;
		}
	    }
	}

        //   Check global overlaps and "wire-like" charts
        // (wasting parameter space)
        compute_fill_and_overlap_ratio(chart);
	if(verbose_) {
	    Logger::out("ParamValidator") 
		<< "Fill ratio = " << fill_ratio() << std::endl;
	    Logger::out("ParamValidator") 
		<< "Overlap ratio = " << overlap_ratio() << std::endl;
	}

        double comp_scaling = chart_scaling(chart);
	if(verbose_) {
	    Logger::out("ParamValidator")
		<< "Scaling = " << comp_scaling << std::endl;
	}


        // If more than 'min_fill_ratio_' of the parameter space is empty, 
        // reject chart.
        if(Numeric::is_nan(fill_ratio()) || fill_ratio() < min_fill_ratio_) {
	    if(verbose_) {
		Logger::out("ParamValidator")
		    << "----> REJECT: filling ratio"
		    << std::endl;
	    }
            return false;
        }

        // If more than 'max_overlap_ratio_' of the pixels correspond to
	// more than one facet, reject chart.
        if(
	   Numeric::is_nan(overlap_ratio()) ||
	   overlap_ratio() > max_overlap_ratio_
	) {
	    if(verbose_) {
		Logger::out("ParamValidator")
		    << "----> REJECT: overlap ratio"
		    << std::endl;
	    }
            return false;
        }

        if(Numeric::is_nan(comp_scaling) || comp_scaling > max_scaling_) {
	    if(verbose_) {
		Logger::out("ParamValidator")
		    << "----> REJECT: scaling "
		    << std::endl;
	    }
            return false;
        }

	if(verbose_) {
	    Logger::out("ParamValidator")
		<< "----> PASS." << std::endl;
	}
        return true;
    }

    double ParamValidator::chart_scaling(
	Chart& chart
    ) {
	Attribute<double> tex_coord;
	tex_coord.bind_if_is_defined(
	    chart.mesh.facet_corners.attributes(), "tex_coord"
	);
	geo_assert(tex_coord.is_bound() && tex_coord.dimension() == 2);

        // Compute largest facet area.
        double max_area = 0;
	for(index_t ff=0; ff<chart.facets.size(); ++ff) {
	    index_t f = chart.facets[ff];
            max_area = std::max(
		GEO::Geom::mesh_facet_area(chart.mesh,f), max_area
	    );
        }

        // Ignore facets smaller than 1% of the largest facet.
        double area_treshold = 0.001 * max_area;

        std::vector<double> facet_scaling;
        facet_scaling.reserve(chart.facets.size());
	
	for(index_t ff=0; ff<chart.facets.size(); ++ff) {
	    index_t f = chart.facets[ff];
            double area   = Geom::mesh_facet_area(chart.mesh,f)  ;
            double area2d = Geom::mesh_facet_area_2d(chart.mesh,f,tex_coord);
            if(area > area_treshold) {
                facet_scaling.push_back(area2d / area);
            } 
        }

        // Ignore 1% of the values at each end.
        std::sort(facet_scaling.begin(), facet_scaling.end());
        index_t offset = index_t(double(facet_scaling.size()) * 0.01);
        index_t begin = offset;
        index_t end = index_t(facet_scaling.size()) - 1 - offset;
        return facet_scaling[end] / facet_scaling[begin];
    }


    void ParamValidator::compute_fill_and_overlap_ratio(Chart& chart) {
	Attribute<double> tex_coord;
	tex_coord.bind_if_is_defined(
	    chart.mesh.facet_corners.attributes(), "tex_coord"
	);
	geo_assert(tex_coord.is_bound() && tex_coord.dimension() == 2);
        begin_rasterizer(chart,tex_coord);
	for(index_t ff=0; ff<chart.facets.size(); ++ff) {
	    index_t f = chart.facets[ff];
	    index_t c1 = chart.mesh.facets.corners_begin(f);
	    vec2 p1(tex_coord[2*c1], tex_coord[2*c1+1]);
	    for(
		index_t c2=c1+1; c2+1<chart.mesh.facets.corners_end(f); ++c2
	    ) {
		index_t c3=c2+1;
		vec2 p2(tex_coord[2*c2], tex_coord[2*c2+1]);
		vec2 p3(tex_coord[2*c3], tex_coord[2*c3+1]);
		rasterize_triangle(p1,p2,p3);
	    }
	}
        end_rasterizer();
    }

    void ParamValidator::begin_rasterizer(Chart& chart, Attribute<double>& tex_coord) {
	Memory::clear(graph_mem_, size_t(graph_size_ * graph_size_));
	double xmin, ymin, xmax, ymax;
	Geom::get_chart_bbox_2d(chart, tex_coord, xmin, ymin, xmax, ymax);
        user_x_min_  = xmin;
        user_y_min_  = ymin;
        user_width_  = xmax - xmin;
        user_height_ = ymax - ymin;
        user_size_ = std::max(user_width_, user_height_);
    }

    
    void ParamValidator::transform(const vec2& p, int& x, int& y) {
        x = int( double(graph_size_-1) * (p.x - user_x_min_) / user_size_);
        y = int( double(graph_size_-1) * (p.y - user_y_min_) / user_size_);
        geo_clamp(x,0,int(graph_size_-1));
        geo_clamp(y,0,int(graph_size_-1));
    }
    
    void ParamValidator::rasterize_triangle(
        const vec2& p1, const vec2& p2, const vec2& p3
    ) {
        int x[3];
        int y[3];

        transform(p1,x[0],y[0]);
        transform(p2,x[1],y[1]);
        transform(p3,x[2],y[2]);

        int ymin = 32767;
        int ymax = -1;
        
        for(int i=0; i<3; i++) {
            ymin = std::min(ymin, y[i]);
            ymax = std::max(ymax, y[i]);
        }

        int signed_area = 
            (x[1] - x[0]) * (y[2] - y[0]) - 
            (x[2] - x[0]) * (y[1] - y[0]);
        bool ccw = (signed_area < 0);

        if(ymin == ymax) {
            return;
        }

        for(int i=0; i<3; i++) {
            int j=(i+1)%3;
            int x1 = x[i];
            int y1 = y[i];
            int x2 = x[j];
            int y2 = y[j];
            if(y1 == y2) {
                continue;
            }
            bool is_left = (y2 < y1) ^ ccw;

            // I want the set of lit pixels to be
            // independent from the order of the 
            // extremities.
            bool swp = false;
            if(y2 == y1) {
                if(x1 > x2) {
                    swp = 1;
                }
            } else {
                if(y1 > y2) {
                    swp = 1;
                }
            }
            if(swp) {
                int tmp;
                tmp = x2;
                x2 = x1;
                x1 = tmp;
                tmp = y2;
                y2 = y1;
                y1 = tmp;
            }

            // Bresenham algo.
            int dx = x2 - x1;
            int dy = y2 - y1;
            int sx = dx > 0 ? 1 : -1;
            int sy = dy > 0 ? 1 : -1;
            dx *= sx;
            dy *= sy;
            int X = x1;
            int Y = y1;
            
            int* line_x = is_left ? x_left_ : x_right_;
            line_x[Y] = X;
            
            int e = dy - 2 * dx;
            while(Y < y2 - 1) {
                
                Y += sy;
                e -= 2 * dx;
                
                while(e < 0) {
                    X += sx;
                    e += 2 * dy;
                }
                
                line_x[Y] = X;
            }
            
            line_x[y2] = x2;
        }
        
        for(int Y = ymin; Y < ymax; ++Y) {
            for(int X = x_left_[Y]; X < x_right_[Y]; ++X) {
                graph_mem_[Y * graph_size_ + X]++;
            }
        }
    }
    
    void ParamValidator::end_rasterizer() {
        int nb_filled = 0;
        int nb_overlapped = 0;
        int width = 0;
        int height = 0;
        if(user_width_ > user_height_) {
            width  = graph_size_;
            height = int((user_height_ * double(graph_size_)) / user_width_);
        } else {
            height = graph_size_;
            width  = int((user_width_ * double(graph_size_)) / user_height_);
        }

        for(int x=0; x<width; x++) {
            for(int y=0; y<height; y++) {
                Numeric::uint8 pixel = graph_mem_[y * graph_size_ + x];
                if(pixel > 0) {
                    nb_filled++;
                    if(pixel > 1) {
                        nb_overlapped++;
                    }
                }
            }
        }

        fill_ratio_ = double(nb_filled) / double(width * height);
        overlap_ratio_ = double(nb_overlapped) / double(width * height);
    }
}


