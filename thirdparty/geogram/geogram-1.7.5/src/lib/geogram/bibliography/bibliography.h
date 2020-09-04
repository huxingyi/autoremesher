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

#ifndef GEOGRAM_BIBLIOGRAPHY_BIBLIOGRAPHY
#define GEOGRAM_BIBLIOGRAPHY_BIBLIOGRAPHY

#include <geogram/basic/common.h>
#include <geogram/basic/memory.h>

namespace GEO {
    
    namespace Biblio {

	/**
	 * \brief Initializes the bibliography system.
	 */
	void GEOGRAM_API initialize();

	/**
	 * \brief Terminates the bibliography system.
	 */
	void GEOGRAM_API terminate();
	
	/**
	 * \brief Registers a set of bibliographic references.
	 * \param[in] bib_refs a string with the bibliographic references,
	 *  in Bibtex format.
	 */
	void GEOGRAM_API register_references(const char* bib_refs);

	/**
	 * \brief Cites a bibliographic reference.
	 * \details Client code should not use this function and should use
	 *  the geo_cite() macro instead.
	 * \param[in] ref the citation key.
	 * \param[in] file the source filename from which the citation key is
	 *  cited.
	 * \param[in] line the source line number.
	 * \param[in] function the name of the function from which the citation
	 *  key is cited.
	 * \param[in] info more information about the context of the citation.
	 */
	void GEOGRAM_API cite(
	    const char* ref,
	    const char* file, int line,
	    const char* function,
	    const char* info = nullptr
	);

	/**
	 * \brief Resets all citations.
	 */
	void GEOGRAM_API reset_citations();
    }

/**
 * \brief Cites a reference.
 * \param [in] ref a string with the bibtex key of the reference.
 */
#ifdef GEO_COMPILER_GCC    
#define geo_cite(ref) ::GEO::Biblio::cite(           \
	ref, __FILE__, __LINE__, __PRETTY_FUNCTION__ \
)
#else
#define geo_cite(ref) ::GEO::Biblio::cite(ref, __FILE__, __LINE__, __FUNCTION__)    
#endif    

/**
 * \brief Cites a reference with information on the context of
 *  the citation.
 * \param [in] ref a string with the bibtex key of the reference.
 * \param [in] info more information on the context of the citation.
 */
#ifdef GEO_COMPILER_GCC    
#define geo_cite_with_info(ref, info) ::GEO::Biblio::cite( \
	ref, __FILE__, __LINE__, __PRETTY_FUNCTION__, info \
)
#else
#define geo_cite_with_info(ref, info) ::GEO::Biblio::cite( \
	ref, __FILE__, __LINE__, __FUNCTION__, info        \
)    
#endif    


    
}

#endif
