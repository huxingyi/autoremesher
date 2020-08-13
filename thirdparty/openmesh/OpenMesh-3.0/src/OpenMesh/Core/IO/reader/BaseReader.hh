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
//  Implements the baseclass for IOManager file access modules
//
//=============================================================================


#ifndef __BASEREADER_HH__
#define __BASEREADER_HH__


//=== INCLUDES ================================================================


// STD C++
#include <iostream>
#include <string>
#include <cctype>
#include <functional>

// OpenMesh
#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/IO/Options.hh>
#include <OpenMesh/Core/IO/importer/BaseImporter.hh>
#include <OpenMesh/Core/Utils/SingletonT.hh>


//== NAMESPACES ===============================================================


namespace OpenMesh {
namespace IO {


//=== IMPLEMENTATION ==========================================================


/**
   Base class for reader modules.
   Reader modules access persistent data and pass them to the desired
   data structure by the means of a BaseImporter derivative.
   All reader modules must be derived from this class.
*/
class OPENMESHDLLEXPORT BaseReader
{
public:

  /// Destructor
  virtual ~BaseReader() {};

  /// Returns a brief description of the file type that can be parsed.
  virtual std::string get_description() const = 0;
  
  /** Returns a string with the accepted file extensions separated by a 
      whitespace and in small caps.
  */
  virtual std::string get_extensions() const = 0;

  /// Return magic bits used to determine file format
  virtual std::string get_magic() const { return std::string(""); }


  /** Reads a mesh given by a filename. Usually this method opens a stream
      and passes it to stream read method. Acceptance checks by filename
      extension can be placed here.

      Options can be passed via _opt. After execution _opt contains the Options
      that were available
  */
  virtual bool read(const std::string& _filename, 
		    BaseImporter& _bi,
                    Options& _opt) = 0;
		
 /** Reads a mesh given by a std::stream. This method usually uses the same stream reading method
    that read uses. Options can be passed via _opt. After execution _opt contains the Options
      that were available.

      Please make sure that if _is is std::ifstream, the correct std::ios_base::openmode flags are set. 
  */
  virtual bool read(std::istream& _is, 
		    BaseImporter& _bi,
                    Options& _opt) = 0;


  /// Returns true if reader can parse _filename (checks extension)
  virtual bool can_u_read(const std::string& _filename) const;


protected:

  // case insensitive search for _ext in _fname.
  bool check_extension(const std::string& _fname, 
		       const std::string& _ext) const;
};


/** \brief Trim left whitespace
 *
 * Removes whitespace at the beginning of the string
 *
 * @param _string input string
 * @return trimmed string
 */
static inline std::string &left_trim(std::string &_string) {
  _string.erase(_string.begin(), std::find_if(_string.begin(), _string.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
  return _string;
}

/** \brief Trim right whitespace
 *
 * Removes whitespace at the end of the string
 *
 * @param _string input string
 * @return trimmed string
 */
static inline std::string &right_trim(std::string &_string) {
  _string.erase(std::find_if(_string.rbegin(), _string.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), _string.end());
  return _string;
}

/** \brief Trim whitespace
 *
 * Removes whitespace at the beginning and end of the string
 *
 * @param _string input string
 * @return trimmed string
 */
static inline std::string &trim(std::string &_string) {
  return left_trim(right_trim(_string));
}



//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
#endif
//=============================================================================
