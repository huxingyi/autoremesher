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
//  Implements the baseclass for IOManager writer modules
//
//=============================================================================


#ifndef __BASEWRITER_HH__
#define __BASEWRITER_HH__


//=== INCLUDES ================================================================


// STD C++
#include <iostream>
#include <string>

// OpenMesh
#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/IO/Options.hh>
#include <OpenMesh/Core/IO/exporter/BaseExporter.hh>


//== NAMESPACES ===============================================================


namespace OpenMesh {
namespace IO {


//=== IMPLEMENTATION ==========================================================


/**
   Base class for all writer modules. The module should register itself at
   the IOManager by calling the register_module function.
*/
class OPENMESHDLLEXPORT BaseWriter
{
public:

  typedef unsigned int Option;

  /// Destructor
  virtual ~BaseWriter() {};

  /// Return short description of the supported file format.
  virtual std::string get_description() const = 0;

  /// Return file format's extension.
  virtual std::string get_extensions() const = 0;

  /// Returns true if writer can parse _filename (checks extension)
  virtual bool can_u_write(const std::string& _filename) const;

  /** Write to a file
   * @param _filename write to file with the given filename
   * @param _be BaseExporter, which specifies the data source
   * @param _opt writing options
   * @param _precision can be used to specify the precision of the floating point notation.
   */
  virtual bool write(const std::string& _filename,
		     BaseExporter& _be,
                     Options _opt,
                     std::streamsize _precision = 6) const = 0;

  /** Write to a std::ostream
   * @param _os write to std::ostream
   * @param _be BaseExporter, which specifies the data source
   * @param _opt writing options
   * @param _precision can be used to specify the precision of the floating point notation.
   */
  virtual bool write(std::ostream& _os,
		     BaseExporter& _be,
                     Options _opt,
                     std::streamsize _precision = 6) const = 0;

  /// Returns expected size of file if binary format is supported else 0.
  virtual size_t binary_size(BaseExporter&, Options) const { return 0; }



protected:

  bool check(BaseExporter& _be, Options _opt) const
  {
    return (_opt.check(Options::VertexNormal ) <= _be.has_vertex_normals())
       &&  (_opt.check(Options::VertexTexCoord)<= _be.has_vertex_texcoords())
       &&  (_opt.check(Options::VertexColor)   <= _be.has_vertex_colors())
       &&  (_opt.check(Options::FaceNormal)    <= _be.has_face_normals())
       &&  (_opt.check(Options::FaceColor)     <= _be.has_face_colors());
  }
};


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
#endif
//=============================================================================
