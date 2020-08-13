/*===========================================================================*\
 *                                                                           *
 *                               OpenMesh                                    *
 *      Copyright (C) 2001-2014 by Computer Graphics Group, RWTH Aachen      *
 *                           www.openmesh.org                                *
 *                                                                           *
 *---------------------------------------------------------------------------*
 *  This file is part of OpenMesh.                                           *
 *                                                                           *
 *  OpenMesh is free software: you can redistribute it and/or modify         *
 *  it under the terms of the GNU Lesser General Public License as           *
 *  published by the Free Software Foundation, either version 3 of           *
 *  the License, or (at your option) any later version with the              *
 *  following exceptions:                                                    *
 *                                                                           *
 *  If other files instantiate templates or use macros                       *
 *  or inline functions from this file, or you compile this file and         *
 *  link it with other files to produce an executable, this file does        *
 *  not by itself cause the resulting executable to be covered by the        *
 *  GNU Lesser General Public License. This exception does not however       *
 *  invalidate any other reasons why the executable file might be            *
 *  covered by the GNU Lesser General Public License.                        *
 *                                                                           *
 *  OpenMesh is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU Lesser General Public License for more details.                      *
 *                                                                           *
 *  You should have received a copy of the GNU LesserGeneral Public          *
 *  License along with OpenMesh.  If not,                                    *
 *  see <http://www.gnu.org/licenses/>.                                      *
 *                                                                           *
\*===========================================================================*/

/*===========================================================================*\
 *                                                                           *
 *   $Revision$                                                         *
 *   $Date$                   *
 *                                                                           *
\*===========================================================================*/


//=============================================================================
//
//  Implements a writer module for STL ascii files
//
//=============================================================================
// $Id: STLWriter.hh,v 1.2 2007-05-18 15:17:43 habbecke Exp $

#ifndef __STLWRITER_HH__
#define __STLWRITER_HH__


//=== INCLUDES ================================================================

// -------------------- STL
#if defined( OM_CC_MIPS )
#  include <stdio.h>
#else
#  include <cstdio>
#endif
#include <string>
// -------------------- OpenMesh
#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Utils/SingletonT.hh>
#include <OpenMesh/Core/IO/exporter/BaseExporter.hh>
#include <OpenMesh/Core/IO/writer/BaseWriter.hh>


//== NAMESPACES ===============================================================


namespace OpenMesh {
namespace IO {


//=== IMPLEMENTATION ==========================================================


/**
    Implementation of the STL format writer. This class is singleton'ed by
    SingletonT to STLWriter.
*/
class OPENMESHDLLEXPORT _STLWriter_ : public BaseWriter
{
public:

  _STLWriter_();

  /// Destructor
  virtual ~_STLWriter_() {};

  std::string get_description() const { return "Stereolithography Format"; }
  std::string get_extensions()  const { return "stla stlb"; }

  bool write(const std::string&, BaseExporter&, Options, std::streamsize _precision = 6) const;

  bool write(std::ostream&, BaseExporter&, Options, std::streamsize _precision = 6) const;

  size_t binary_size(BaseExporter&, Options) const;

private:
  bool write_stla(const std::string&, BaseExporter&, Options) const;
  bool write_stla(std::ostream&, BaseExporter&, Options, std::streamsize _precision = 6) const;
  bool write_stlb(const std::string&, BaseExporter&, Options) const;
  bool write_stlb(std::ostream&, BaseExporter&, Options, std::streamsize _precision = 6) const;
};


//== TYPE DEFINITION ==========================================================


// Declare the single entity of STL writer.
extern _STLWriter_  __STLWriterInstance;
OPENMESHDLLEXPORT _STLWriter_& STLWriter();


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
#endif
//=============================================================================
