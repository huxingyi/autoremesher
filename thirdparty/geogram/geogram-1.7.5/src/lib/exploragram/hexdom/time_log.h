/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2015 INRIA - Project ALICE
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
 *  Contact for Graphite: Bruno Levy - Bruno.Levy@inria.fr
 *  Contact for this Plugin: Nicolas Ray - nicolas.ray@inria.fr
 *
 *     Project ALICE
 *     LORIA, INRIA Lorraine,
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs.
 *
 * As an exception to the GPL, Graphite can be linked with the following
 * (non-GPL) libraries:
 *     Qt, tetgen, SuperLU, WildMagic and CGAL
 */

#ifndef H_HEXDOM_ALGO_TIME_LOG_H
#define H_HEXDOM_ALGO_TIME_LOG_H

#include <exploragram/basic/common.h>
#include <string>
#include <iostream>
#include <vector>
#include <time.h> 
#include <fstream>

/**
 * LogTime reports a hierarchical execution pipeline:
 *	A logger outputs the steps during execution
 *	It summarizes execution time in each step
 *	It outputs important values (stats) at the end of the execution
 */

struct EXPLORAGRAM_API LogTime {
    //                         _   ___ ___ 
    //                        /_\ | _ \_ _|
    //                       / _ \|  _/| | 
    //                      /_/ \_\_| |___|
    //                                                                        
	LogTime() { post_fix = ""; }
    ~LogTime(){}
    // construct API
    /**
     * put it before starting a new task of the pipeline
     */
    void add_step(const std::string& name);

    /**
     * start/end section to define tasks that will be split into smaller tasks
     */
    void start_section(const std::string& secname, const std::string& name = "begin section");
    void end_section();

    /**
     * values are used to get feedback (mostly quality stats)
     */
    void add_value(std::string str, double val);

    /**
     * values are used to get feedback (mostly quality stats)
     */
    void add_string(std::string str, std::string val);

    // output API
	
    /**
     * show is to have the summary in the console
     */
    void show();
    /**
     * drop_file is to have the summary in a file. 
     */
    void drop_file(std::string filename, bool append = false,unsigned int timing_depth=1000);

	void set_suffix(const std::string& suffix) { post_fix=suffix; }
    //                                _          _       
    //                       _ __ _ _(_)_ ____ _| |_ ___ 
    //                      | '_ \ '_| \ V / _` |  _/ -_)
    //                      | .__/_| |_|\_/\__,_|\__\___|
    //                      |_|                         

 private:
    /**
     * CheckPoint is the list item of LogTime
     * "up" is it's father
     * "right" is the next item at the same level
     */
    struct CheckPoint{
	CheckPoint(const std::string& p_n, unsigned int p_up){ n = p_n; up = p_up; t = clock(); right = (unsigned int)(-1); }
	std::string n;
	clock_t t;
	unsigned int right;
	unsigned int up;
    };

    bool is_start_section(unsigned int i);
    bool is_end_section(unsigned int i);
    bool is_final(unsigned int i);
    double time(unsigned int i);
    unsigned int dec(unsigned int i = (unsigned int)(-1));
    unsigned int lastdec();
    void debug();
    std::string cur_stack();
    void report(std::ostream &out, unsigned int timing_depth = 10000);
    void report_py(std::ostream &out, unsigned int timing_depth = 10000);
	
private:
	std::string post_fix;
    std::vector<CheckPoint> check;
    std::vector<std::pair<std::string, double> > out_values;
    std::vector<std::pair<std::string, std::string> > out_strings;
};

extern EXPLORAGRAM_API LogTime logt;

#endif
