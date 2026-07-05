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
