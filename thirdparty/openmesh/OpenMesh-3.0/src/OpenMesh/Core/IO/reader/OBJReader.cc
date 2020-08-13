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


//== INCLUDES =================================================================


// OpenMesh
#include <OpenMesh/Core/IO/reader/OBJReader.hh>
#include <OpenMesh/Core/IO/IOManager.hh>
#include <OpenMesh/Core/System/omstream.hh>
#include <OpenMesh/Core/Utils/vector_cast.hh>
#include <OpenMesh/Core/Utils/color_cast.hh>
// STL
#if defined(OM_CC_MIPS)
#  include <ctype.h>
/// \bug Workaround for STLPORT 4.6: isspace seems not to be in namespace std!
#elif defined(_STLPORT_VERSION) && (_STLPORT_VERSION==0x460)
#  include <cctype>
#else
#  include <cctype>
using std::isspace;
#endif

#ifndef WIN32
#include <string.h>
#endif

//=== NAMESPACES ==============================================================


namespace OpenMesh {
namespace IO {


//=== INSTANCIATE =============================================================


_OBJReader_  __OBJReaderInstance;
_OBJReader_& OBJReader() { return __OBJReaderInstance; }


//=== IMPLEMENTATION ==========================================================

//-----------------------------------------------------------------------------

void trimString( std::string& _string) {
  // Trim Both leading and trailing spaces

  size_t start = _string.find_first_not_of(" \t\r\n");
  size_t end   = _string.find_last_not_of(" \t\r\n");

  if(( std::string::npos == start ) || ( std::string::npos == end))
    _string = "";
  else
    _string = _string.substr( start, end-start+1 );
}

//-----------------------------------------------------------------------------

_OBJReader_::
_OBJReader_()
{
  IOManager().register_module(this);
}


//-----------------------------------------------------------------------------


bool
_OBJReader_::
read(const std::string& _filename, BaseImporter& _bi, Options& _opt)
{
  std::fstream in( _filename.c_str(), std::ios_base::in );

  if (!in.is_open() || !in.good())
  {
    omerr() << "[OBJReader] : cannot not open file "
          << _filename
          << std::endl;
    return false;
  }

  {
#if defined(WIN32)
    std::string::size_type dot = _filename.find_last_of("\\/");
#else
    std::string::size_type dot = _filename.rfind("/");
#endif
    path_ = (dot == std::string::npos)
      ? "./"
      : std::string(_filename.substr(0,dot+1));
  }

  bool result = read(in, _bi, _opt);

  in.close();
  return result;
}

//-----------------------------------------------------------------------------

bool
_OBJReader_::
read_material(std::fstream& _in)
{
  std::string line;
  std::string keyWrd;
  std::string textureName;

  std::string key;
  Material    mat;
  float       f1,f2,f3;
  bool        indef = false;
  int         textureId = 1;


  materials_.clear();
  mat.cleanup();

  while( _in && !_in.eof() )
  {
    std::getline(_in,line);
    if ( _in.bad() ){
      omerr() << "  Warning! Could not read file properly!\n";
      return false;
    }

    if ( line.empty() )
      continue;

    std::stringstream stream(line);

    stream >> keyWrd;

    if( ( isspace(line[0]) && line[0] != '\t' ) || line[0] == '#' )
    {
      if (indef && !key.empty() && mat.is_valid())
      {
        materials_[key] = mat;
        mat.cleanup();
      }
    }

    else if (keyWrd == "newmtl") // begin new material definition
    {
      stream >> key;
      indef = true;
    }

    else if (keyWrd == "Kd") // diffuse color
    {
      stream >> f1; stream >> f2; stream >> f3;

      if( !stream.fail() )
        mat.set_Kd(f1,f2,f3);
    }

    else if (keyWrd == "Ka") // ambient color
    {
      stream >> f1; stream >> f2; stream >> f3;

      if( !stream.fail() )
        mat.set_Ka(f1,f2,f3);
    }

    else if (keyWrd == "Ks") // specular color
    {
      stream >> f1; stream >> f2; stream >> f3;

      if( !stream.fail() )
        mat.set_Ks(f1,f2,f3);
    }
#if 0
    else if (keyWrd == "illum") // diffuse/specular shading model
    {
      ; // just skip this
    }

    else if (keyWrd == "Ns") // Shininess [0..200]
    {
      ; // just skip this
    }

    else if (keyWrd == "map_") // map images
    {
      // map_Ks, specular map
      // map_Ka, ambient map
      // map_Bump, bump map
      // map_d,  opacity map
      ; // just skip this
    }
#endif
    else if (keyWrd == "map_Kd" ) {
      // Get the rest of the line, removing leading or trailing spaces
      // This will define the filename of the texture
      std::getline(stream,textureName);
      trimString(textureName);
      if ( ! textureName.empty() )
        mat.set_map_Kd( textureName, textureId++ );
    }
    else if (keyWrd == "Tr") // transparency value
    {
      stream >> f1;

      if( !stream.fail() )
        mat.set_Tr(f1);
    }
    else if (keyWrd == "d") // transparency value
    {
      stream >> f1;

      if( !stream.fail() )
        mat.set_Tr(f1);
    }

    if ( _in && indef && mat.is_valid() && !key.empty())
      materials_[key] = mat;
  }
  return true;
}

//-----------------------------------------------------------------------------

bool
_OBJReader_::
read(std::istream& _in, BaseImporter& _bi, Options& _opt)
{
  omlog() << "[OBJReader] : read file\n";


  std::string line;
  std::string keyWrd;

  float                     x, y, z, u, v;
  int                       r, g, b;
  BaseImporter::VHandles    vhandles;
  std::vector<Vec3f>        normals;
  std::vector<Vec3uc>       colors;
  std::vector<Vec2f>        texcoords;
  std::vector<Vec2f>        face_texcoords;
  std::vector<VertexHandle> vertexHandles;

  std::string               matname;


  // Options supplied by the user
  Options userOptions = _opt;

  // Options collected via file parsing
  Options fileOptions;


  while( _in && !_in.eof() )
  {
    std::getline(_in,line);
    if ( _in.bad() ){
      omerr() << "  Warning! Could not read file properly!\n";
      return false;
    }

    // Trim Both leading and trailing spaces
    trimString(line);

    // comment
    if ( line.size() == 0 || line[0] == '#' || isspace(line[0]) ) {
      continue;
    }

    std::stringstream stream(line);

    stream >> keyWrd;

    // material file
    if (keyWrd == "mtllib")
    {
      std::string matFile;

      // Get the rest of the line, removing leading or trailing spaces
      // This will define the filename of the texture
      std::getline(stream,matFile);
      trimString(matFile);

      matFile = path_ + matFile;

      omlog() << "Load material file " << matFile << std::endl;

      std::fstream matStream( matFile.c_str(), std::ios_base::in );

      if ( matStream ){

        if ( !read_material( matStream ) )
	        omerr() << "  Warning! Could not read file properly!\n";
        matStream.close();

      }else
	      omerr() << "  Warning! Material file '" << matFile << "' not found!\n";

      omlog() << "  " << materials_.size() << " materials loaded.\n";

      for ( MaterialList::iterator material = materials_.begin(); material != materials_.end(); ++material )
      {
        // Save the texture information in a property
        if ( (*material).second.has_map_Kd() )
          _bi.add_texture_information( (*material).second.map_Kd_index() , (*material).second.map_Kd() );
      }

    }

    // usemtl
    else if (keyWrd == "usemtl")
    {
      stream >> matname;
      if (materials_.find(matname)==materials_.end())
      {
        omerr() << "Warning! Material '" << matname
              << "' not defined in material file.\n";
        matname="";
      }
    }

    // vertex
    else if (keyWrd == "v")
    {
      stream >> x; stream >> y; stream >> z;

      if ( !stream.fail() )
      {
        vertexHandles.push_back(_bi.add_vertex(OpenMesh::Vec3f(x,y,z)));
        stream >> r; stream >> g; stream >> b;

        if ( !stream.fail() )
        {
          if (  userOptions.vertex_has_color() ) {
            fileOptions += Options::VertexColor;
            colors.push_back(OpenMesh::Vec3uc((unsigned char)r,(unsigned char)g,(unsigned char)b));
          }
        }
      }
    }

    // texture coord
    else if (keyWrd == "vt")
    {
      stream >> u; stream >> v;

      if ( !stream.fail()  ){

        if ( userOptions.vertex_has_texcoord() || userOptions.face_has_texcoord() ) {
          texcoords.push_back(OpenMesh::Vec2f(u, v));

          // Can be used for both!
          fileOptions += Options::VertexTexCoord;
          fileOptions += Options::FaceTexCoord;
        }

      }else{

        omerr() << "Only single 2D texture coordinate per vertex"
              << "allowed!" << std::endl;
        return false;
      }
    }

    // color per vertex
    else if (keyWrd == "vc")
    {
      stream >> r; stream >> g; stream >> b;

      if ( !stream.fail()   ){
        if ( userOptions.vertex_has_color() ) {
          colors.push_back(OpenMesh::Vec3uc((unsigned char)r,(unsigned char)g,(unsigned char)b));
          fileOptions += Options::VertexColor;
        }
      }
    }

    // normal
    else if (keyWrd == "vn")
    {
      stream >> x; stream >> y; stream >> z;

      if ( !stream.fail() ) {
        if (userOptions.vertex_has_normal() ){
          normals.push_back(OpenMesh::Vec3f(x,y,z));
          fileOptions += Options::VertexNormal;
        }
      }
    }


    // face
    else if (keyWrd == "f")
    {
      int component(0), nV(0);
      int value;

      vhandles.clear();
      face_texcoords.clear();

      // read full line after detecting a face
      std::string faceLine;
      std::getline(stream,faceLine);
      std::stringstream lineData( faceLine );

      FaceHandle fh;
      BaseImporter::VHandles faceVertices;

      // work on the line until nothing left to read
      while ( !lineData.eof() )
      {
        // read one block from the line ( vertex/texCoord/normal )
        std::string vertex;
        lineData >> vertex;

        do{

          //get the component (vertex/texCoord/normal)
          size_t found=vertex.find("/");

          // parts are seperated by '/' So if no '/' found its the last component
          if( found != std::string::npos ){

            // read the index value
            std::stringstream tmp( vertex.substr(0,found) );

            // If we get an empty string this property is undefined in the file
            if ( vertex.substr(0,found).empty() ) {
              // Switch to next field
              vertex = vertex.substr(found+1);

              // Now we are at the next component
              ++component;

              // Skip further processing of this component
              continue;
            }

            // Read current value
            tmp >> value;

            // remove the read part from the string
            vertex = vertex.substr(found+1);

          } else {

            // last component of the vertex, read it.
            std::stringstream tmp( vertex );
            tmp >> value;

            // Clear vertex after finished reading the line
            vertex="";

            // Nothing to read here ( garbage at end of line )
            if ( tmp.fail() ) {
              continue;
            }
          }

          // store the component ( each component is referenced by the index here! )
          switch (component)
          {
            case 0: // vertex
              if ( value < 0 ) {
                // Calculation of index :
                // -1 is the last vertex in the list
                // As obj counts from 1 and not zero add +1
                value = int(_bi.n_vertices() + value + 1);
              }
              // Obj counts from 1 and not zero .. array counts from zero therefore -1
              vhandles.push_back(VertexHandle(value-1));
              faceVertices.push_back(VertexHandle(value-1));
              if (fileOptions.vertex_has_color() )
                _bi.set_color(vhandles.back(), colors[value-1]);
              break;
	      
            case 1: // texture coord
              if ( value < 0 ) {
                // Calculation of index :
                // -1 is the last vertex in the list
                // As obj counts from 1 and not zero add +1
                value = int(texcoords.size()) + value + 1;
              }
              assert(!vhandles.empty());


              if ( fileOptions.vertex_has_texcoord() && userOptions.vertex_has_texcoord() ) {

                if (!texcoords.empty() && (unsigned int) (value - 1) < texcoords.size()) {
                  // Obj counts from 1 and not zero .. array counts from zero therefore -1
                  _bi.set_texcoord(vhandles.back(), texcoords[value - 1]);
                } else {
                  omerr() << "Error setting Texture coordinates" << std::endl;
                }

                }

                if (fileOptions.face_has_texcoord() && userOptions.face_has_texcoord() ) {

                  if (!texcoords.empty() && (unsigned int) (value - 1) < texcoords.size()) {
                    face_texcoords.push_back( texcoords[value-1] );
                  } else {
                    omerr() << "Error setting Texture coordinates" << std::endl;
                  }
                }


              break;

            case 2: // normal
              if ( value < 0 ) {
                // Calculation of index :
                // -1 is the last vertex in the list
                // As obj counts from 1 and not zero add +1
                value = int(normals.size()) + value + 1;
              }

              // Obj counts from 1 and not zero .. array counts from zero therefore -1
              if (fileOptions.vertex_has_normal() ) {
                assert(!vhandles.empty());
                assert((unsigned int)(value-1) < normals.size());
                _bi.set_normal(vhandles.back(), normals[value-1]);
              }
              break;
          }

          // Prepare for reading next component
          ++component;

          // Read until line does not contain any other info
        } while ( !vertex.empty() );

        component = 0;
        nV++;

      }

      // note that add_face can possibly triangulate the faces, which is why we have to
      // store the current number of faces first
      size_t n_faces = _bi.n_faces();
      fh = _bi.add_face(faceVertices);

      if (!vhandles.empty() && fh.is_valid() )
        _bi.add_face_texcoords(fh, vhandles[0], face_texcoords);

      if ( !matname.empty()  )
      {
        std::vector<FaceHandle> newfaces;

        for( size_t i=0; i < _bi.n_faces()-n_faces; ++i )
          newfaces.push_back(FaceHandle(int(n_faces+i)));

        Material& mat = materials_[matname];

        if ( mat.has_Kd() ) {
          Vec3uc fc = color_cast<Vec3uc, Vec3f>(mat.Kd());

          if ( userOptions.face_has_color()) {

            for (std::vector<FaceHandle>::iterator it = newfaces.begin(); it != newfaces.end(); ++it)
              _bi.set_color(*it, fc);

            fileOptions += Options::FaceColor;
          }
        }

        // Set the texture index in the face index property
        if ( mat.has_map_Kd() ) {

          if (userOptions.face_has_texcoord()) {

            for (std::vector<FaceHandle>::iterator it = newfaces.begin(); it != newfaces.end(); ++it)
              _bi.set_face_texindex(*it, mat.map_Kd_index());

            fileOptions += Options::FaceTexCoord;

          }

        } else {

          // If we don't have the info, set it to no texture
          if (userOptions.face_has_texcoord()) {

            for (std::vector<FaceHandle>::iterator it = newfaces.begin(); it != newfaces.end(); ++it)
              _bi.set_face_texindex(*it, 0);

          }
        }

      } else {
        std::vector<FaceHandle> newfaces;

        for( size_t i=0; i < _bi.n_faces()-n_faces; ++i )
          newfaces.push_back(FaceHandle(int(n_faces+i)));

        // Set the texture index to zero as we don't have any information
        if ( userOptions.face_has_texcoord() )
          for (std::vector<FaceHandle>::iterator it = newfaces.begin(); it != newfaces.end(); ++it)
            _bi.set_face_texindex(*it, 0);
      }

    }

  }

  // If we do not have any faces,
  // assume this is a point cloud and read the normals and colors directly
  if (_bi.n_faces() == 0)
  {
    int i = 0;
    // add normal per vertex

    if (normals.size() == _bi.n_vertices()) {
      if ( fileOptions.vertex_has_normal() && userOptions.vertex_has_normal() ) {
        for (std::vector<VertexHandle>::iterator it = vertexHandles.begin(); it != vertexHandles.end(); ++it, i++)
          _bi.set_normal(*it, normals[i]);
      }
    }

    // add color per vertex
    i = 0;
    if (colors.size() >= _bi.n_vertices())
      if (fileOptions.vertex_has_color() && userOptions.vertex_has_color()) {
        for (std::vector<VertexHandle>::iterator it = vertexHandles.begin(); it != vertexHandles.end(); ++it, i++)
          _bi.set_color(*it, colors[i]);
      }

  }

  // Return, what we actually read
  _opt = fileOptions;

  return true;
}


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
