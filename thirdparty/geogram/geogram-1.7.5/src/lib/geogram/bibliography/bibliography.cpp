/*
 *  Copyright (c) 2012-2014, Bruno Levy
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

#include <geogram/bibliography/bibliography.h>
#include <geogram/basic/memory.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/stopwatch.h>
#include <string>

namespace {
    using namespace GEO;

    double timeorigin;
    
    vector<const char*> bib_refs_;

    struct CitationRecord {
	CitationRecord(
	    const std::string& k,
	    const std::string& f, int l,
	    const std::string& func,
	    const std::string& inf
	) : key(k), file(f), line(l), function(func), info(inf) {
	    timestamp = SystemStopwatch::now() - timeorigin;
	}
	std::string key;
	std::string file;
	int line;
	std::string function;
	std::string info;
	double timestamp;
    };
    
    vector<CitationRecord> citations_;
}

void register_embedded_bib_file(void);

namespace GEO {

    namespace Biblio {

	void initialize() {
	    register_embedded_bib_file();
	    timeorigin = SystemStopwatch::now();
	    geo_cite("WEB:GEOGRAM");
	}

	void terminate() {
	    if(
		CmdLine::arg_is_declared("biblio") &&		
		CmdLine::get_arg_bool("biblio") &&
		citations_.size() != 0
	    ) {
		Logger::div("Bibliography");
		{
		    Logger::out("Bibliography")
			<< "Saving references to geogram.bib"
			<< std::endl;
		    std::ofstream out("geogram.bib");
		    FOR(i,bib_refs_.size()) {
			out << bib_refs_[i];
		    }
		}
		{
		    Logger::out("Bibliography")
			<< "Saving citations to geogram.tex"
			<< std::endl;
		    std::ofstream out("geogram.tex");
		    out << "\\documentclass{article}" << std::endl;
		    out << "\\usepackage{url}" << std::endl;
		    out << "\\title{Geogram Bibliography Report}" << std::endl;
		    out << "\\date{\\today}" << std::endl;
		    out << "\\author{Geogram ver. "
			<< Environment::instance()->get_value("version")
			<< " citation subsystem"
			<< "}"
			<< std::endl;
		    out << "\\begin{document}" << std::endl;
		    out << "\\maketitle" << std::endl;

		    if(CmdLine::get_arg_bool("biblio:command_line")) {
			    out << "\\section*{Command Line}" << std::endl;
			    std::vector<std::string> args;
			    CmdLine::get_args(args);
			    out << "\\begin{small}" << std::endl;
			    out << "\\begin{enumerate}" << std::endl;
			    FOR(i,args.size()) {
				out << "\\item \\verb|"
				    << args[i] << "| " << std::endl;
			    }
			    out << "\\end{enumerate}" << std::endl;
			    out << "\\end{small}" << std::endl;
		    }

		    out << "\\section*{Citation report}" << std::endl;
		    
		    out << "\\begin{enumerate}" << std::endl;
		    FOR(i,citations_.size()) {
			const CitationRecord& R = citations_[i];
			std::string context =
			    R.file + ":" +
			    String::to_string(R.line);
			out << "\\item "
			    << "\\cite{" << R.key << "}: cited from: "
			    << "\\verb|" << context << "| \\\\" << std::endl;
			out << "\\begin{small}" << std::endl;
			out << "\\verb|" << R.function << "|" << std::endl;
			out << "\\end{small} \\\\" << std::endl;
			if(R.info != "") {
			    out << "Info: " << R.info << "\\\\" << std::endl;
			}
			out << "Timestamp: " << R.timestamp << std::endl;
		    }
		    out << "\\end{enumerate}" << std::endl;
		    out << "\\bibliographystyle{alpha}" << std::endl;
		    out << "\\bibliography{geogram}" << std::endl;
		    out << "\\end{document}" << std::endl;
		}
	    }
	}
	
	void register_references(const char* bib_refs) {
	    bib_refs_.push_back(bib_refs);
	}	

	void cite(
	    const char* ref, const char* file, int line, const char* function,
	    const char* info
	) {
	    std::string shortfile = file;
	    size_t pos = shortfile.find("src/lib/");
	    if(pos != std::string::npos) {
		shortfile = shortfile.substr(pos+8, shortfile.length()-pos-8);
	    }
	    pos = shortfile.find("OGF/");
	    if(pos != std::string::npos) {
		shortfile = shortfile.substr(pos, shortfile.length()-pos);
	    }

	    std::string shortfunction = function;
	    pos = shortfunction.find('(');
	    if(pos != std::string::npos) {
		shortfunction = shortfunction.substr(0,pos);
		shortfunction += "()";
	    }

	    pos = 0;
	    for(int i = int(shortfunction.length())-1; i>0; --i) {
		if(shortfunction[size_t(i)] == ' ') {
		    pos = size_t(i);
		    break;
		}
	    }
	    shortfunction = shortfunction.substr(pos, shortfunction.length()-pos);
	    
	    citations_.push_back(
		CitationRecord(
		    ref, shortfile, line, shortfunction, (info != nullptr) ? info : ""
		)
	    );
	    
	    std::string context = std::string(shortfunction) + " (" +
		shortfile + ":" +
		String::to_string(line) + ")" ;
	    
	    if(
		CmdLine::arg_is_declared("biblio") &&
		CmdLine::get_arg_bool("biblio")
	    ) {
		Logger::out("Bibliography")
		    << "[" << ref << "] cited from: "
		    << context << std::endl;
	    }
	}

	void reset_citations() {
	    citations_.clear();
	    timeorigin = SystemStopwatch::now();	    
	}
    }
    
}
