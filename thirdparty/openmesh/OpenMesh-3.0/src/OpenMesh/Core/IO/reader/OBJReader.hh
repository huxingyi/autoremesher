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
//  Implements an reader module for OBJ files
//
//=============================================================================


#ifndef __OBJREADER_HH__
#define __OBJREADER_HH__


//=== INCLUDES ================================================================


#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>

#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Utils/SingletonT.hh>
#include <OpenMesh/Core/IO/importer/BaseImporter.hh>
#include <OpenMesh/Core/IO/reader/BaseReader.hh>


//== NAMESPACES ===============================================================


namespace OpenMesh {
namespace IO {


//== IMPLEMENTATION ===========================================================


/**
    Implementation of the OBJ format reader.
*/
class OPENMESHDLLEXPORT _OBJReader_ : public BaseReader
{
public:

  _OBJReader_();

  virtual ~_OBJReader_() { }

  std::string get_description() const { return "Alias/Wavefront"; }
  std::string get_extensions()  const { return "obj"; }

  bool read(const std::string& _filename,
	    BaseImporter& _bi,
	    Options& _opt);

  bool read(std::istream& _in,
          BaseImporter& _bi,
          Options& _opt);

private:

#ifndef DOXY_IGNORE_THIS
  class Material
  {
  public:

    Material() { cleanup(); }

    void cleanup()
    {
      Kd_is_set_     = false;
      Ka_is_set_     = false;
      Ks_is_set_     = false;
      Tr_is_set_     = false;
      map_Kd_is_set_ = false;
    }

    bool is_valid(void) const
    { return Kd_is_set_ || Ka_is_set_ || Ks_is_set_ || Tr_is_set_; }

    bool has_Kd(void)     { return Kd_is_set_;     }
    bool has_Ka(void)     { return Ka_is_set_;     }
    bool has_Ks(void)     { return Ks_is_set_;     }
    bool has_Tr(void)     { return Tr_is_set_;     }
    bool has_map_Kd(void) { return map_Kd_is_set_; }

    void set_Kd( float r, float g, float b )
    { Kd_=Vec3f(r,g,b); Kd_is_set_=true; }

    void set_Ka( float r, float g, float b )
    { Ka_=Vec3f(r,g,b); Ka_is_set_=true; }

    void set_Ks( float r, float g, float b )
    { Ks_=Vec3f(r,g,b); Ks_is_set_=true; }

    void set_Tr( float t )
    { Tr_=t;            Tr_is_set_=true; }

    void set_map_Kd( std::string _name, int _index_Kd )
    { map_Kd_ = _name, index_Kd_ = _index_Kd; map_Kd_is_set_ = true; };

    const Vec3f& Kd( void ) const { return Kd_; }
    const Vec3f& Ka( void ) const { return Ka_; }
    const Vec3f& Ks( void ) const { return Ks_; }
    float  Tr( void ) const { return Tr_; }
    const std::string& map_Kd( void ) { return map_Kd_ ; }
    const int& map_Kd_index( void ) { return index_Kd_ ; }

  private:

    Vec3f Kd_;                          bool Kd_is_set_; // diffuse
    Vec3f Ka_;                          bool Ka_is_set_; // ambient
    Vec3f Ks_;                          bool Ks_is_set_; // specular
    float Tr_;                          bool Tr_is_set_; // transperency

    std::string map_Kd_; int index_Kd_; bool map_Kd_is_set_; // Texture

  };
#endif

  typedef std::map<std::string, Material> MaterialList;

  MaterialList materials_;

  bool read_material( std::fstream& _in );

private:

  std::string path_;

};


//== TYPE DEFINITION ==========================================================


extern _OBJReader_  __OBJReaderInstance;
OPENMESHDLLEXPORT _OBJReader_& OBJReader();


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
#endif
//=============================================================================
