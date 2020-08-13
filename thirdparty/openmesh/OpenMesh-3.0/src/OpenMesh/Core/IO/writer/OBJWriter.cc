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


//STL
#include <fstream>
#include <limits>

// OpenMesh
#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/IO/BinaryHelper.hh>
#include <OpenMesh/Core/IO/writer/OBJWriter.hh>
#include <OpenMesh/Core/IO/IOManager.hh>
#include <OpenMesh/Core/System/omstream.hh>
#include <OpenMesh/Core/Utils/color_cast.hh>

//=== NAMESPACES ==============================================================


namespace OpenMesh {
namespace IO {


//=== INSTANCIATE =============================================================


// register the OBJLoader singleton with MeshLoader
_OBJWriter_  __OBJWriterinstance;
_OBJWriter_& OBJWriter() { return __OBJWriterinstance; }


//=== IMPLEMENTATION ==========================================================


_OBJWriter_::_OBJWriter_() { IOManager().register_module(this); }


//-----------------------------------------------------------------------------


bool
_OBJWriter_::
write(const std::string& _filename, BaseExporter& _be, Options _opt, std::streamsize _precision) const
{
  std::fstream out(_filename.c_str(), std::ios_base::out );

  if (!out)
  {
    omerr() << "[OBJWriter] : cannot open file "
	  << _filename << std::endl;
    return false;
  }

  out.precision(_precision);

  {
#if defined(WIN32)
    std::string::size_type dot = _filename.find_last_of("\\/");
#else
    std::string::size_type dot = _filename.rfind("/");
#endif

    if (dot == std::string::npos){
      path_ = "./";
      objName_ = _filename;
    }else{
      path_ = _filename.substr(0,dot+1);
      objName_ = _filename.substr(dot+1);
    }

    //remove the file extension
    dot = _filename.find_last_of(".");

    if(dot != std::string::npos)
      objName_ = objName_.substr(0,dot-1);
  }

  bool result = write(out, _be, _opt, _precision);

  out.close();
  return result;
}

//-----------------------------------------------------------------------------

size_t _OBJWriter_::getMaterial(OpenMesh::Vec3f _color) const
{
  for (size_t i=0; i < material_.size(); i++)
    if(material_[i] == _color)
      return i;

  //not found add new material
  material_.push_back( _color );
  return material_.size()-1;
}

//-----------------------------------------------------------------------------

size_t _OBJWriter_::getMaterial(OpenMesh::Vec4f _color) const
{
  for (size_t i=0; i < materialA_.size(); i++)
    if(materialA_[i] == _color)
      return i;

  //not found add new material
  materialA_.push_back( _color );
  return materialA_.size()-1;
}

//-----------------------------------------------------------------------------

bool
_OBJWriter_::
writeMaterial(std::ostream& _out, BaseExporter& _be, Options _opt) const
{
  OpenMesh::Vec3f c;
  OpenMesh::Vec4f cA;

  material_.clear();
  materialA_.clear();

  //iterate over faces
  for (size_t i=0, nF=_be.n_faces(); i<nF; ++i)
  {
    //color with alpha
    if ( _opt.color_has_alpha() ){
      cA  = color_cast<OpenMesh::Vec4f> (_be.colorA( FaceHandle(int(i)) ));
      getMaterial(cA);
    }else{
    //and without alpha
      c  = color_cast<OpenMesh::Vec3f> (_be.color( FaceHandle(int(i)) ));
      getMaterial(c);
    }
  }

  //write the materials
  if ( _opt.color_has_alpha() )
    for (size_t i=0; i < materialA_.size(); i++){
      _out << "newmtl " << "mat" << i << std::endl;
      _out << "Ka 0.5000 0.5000 0.5000" << std::endl;
      _out << "Kd " << materialA_[i][0] << materialA_[i][1] << materialA_[i][2] << std::endl;;
      _out << "Tr " << materialA_[i][3] << std::endl;
      _out << "illum 1" << std::endl;
    }
  else
    for (size_t i=0; i < material_.size(); i++){
      _out << "newmtl " << "mat" << i << std::endl;
      _out << "Ka 0.5000 0.5000 0.5000" << std::endl;
      _out << "Kd " << material_[i][0] << material_[i][1] << material_[i][2] << std::endl;;
      _out << "illum 1" << std::endl;
    }

  return true;
}

//-----------------------------------------------------------------------------


bool
_OBJWriter_::
write(std::ostream& _out, BaseExporter& _be, Options _opt, std::streamsize _precision) const
{
  unsigned int idx;
  size_t i, j,nV, nF;
  Vec3f v, n;
  Vec2f t;
  VertexHandle vh;
  std::vector<VertexHandle> vhandles;
  bool useMatrial = false;
  OpenMesh::Vec3f c;
  OpenMesh::Vec4f cA;

  omlog() << "[OBJWriter] : write file\n";

  _out.precision(_precision);

  // check exporter features
  if (!check( _be, _opt))
     return false;


  // check writer features
  if ( _opt.check(Options::Binary)     || // not supported by format
       _opt.check(Options::FaceNormal) ||
       _opt.check(Options::FaceColor))
     return false;


  //create material file if needed
  if ( _opt.check(Options::FaceColor) ){

    std::string matFile = path_ + objName_ + ".mat";

    std::fstream matStream(matFile.c_str(), std::ios_base::out );

    if (!_out)
    {
      omerr() << "[OBJWriter] : cannot write material file " << matFile << std::endl;

    }else{
      useMatrial = writeMaterial(matStream, _be, _opt);

      matStream.close();
    }
  }

  // header
  _out << "# " << _be.n_vertices() << " vertices, ";
  _out << _be.n_faces() << " faces" << std::endl;

  // material file
  if (useMatrial &&  _opt.check(Options::FaceColor) )
    _out << "mtllib " << objName_ << ".mat" << std::endl;

  // vertex data (point, normals, texcoords)
  for (i=0, nV=_be.n_vertices(); i<nV; ++i)
  {
    vh = VertexHandle(int(i));
    v  = _be.point(vh);
    n  = _be.normal(vh);
    t  = _be.texcoord(vh);

    _out << "v " << v[0] <<" "<< v[1] <<" "<< v[2] << std::endl;

    if (_opt.check(Options::VertexNormal))
      _out << "vn " << n[0] <<" "<< n[1] <<" "<< n[2] << std::endl;

    if (_opt.check(Options::VertexTexCoord))
      _out << "vt " << t[0] <<" "<< t[1] << std::endl;
  }

  size_t lastMat = std::numeric_limits<std::size_t>::max();

  // we do not want to write seperators if we only write vertex indices
  bool onlyVertices =    !_opt.check(Options::VertexTexCoord)
                      && !_opt.check(Options::VertexNormal);

  // faces (indices starting at 1 not 0)
  for (i=0, nF=_be.n_faces(); i<nF; ++i)
  {

    if (useMatrial &&  _opt.check(Options::FaceColor) ){
      size_t material = std::numeric_limits<std::size_t>::max();

      //color with alpha
      if ( _opt.color_has_alpha() ){
        cA  = color_cast<OpenMesh::Vec4f> (_be.colorA( FaceHandle(int(i)) ));
        material = getMaterial(cA);
      } else{
      //and without alpha
        c  = color_cast<OpenMesh::Vec3f> (_be.color( FaceHandle(int(i)) ));
        material = getMaterial(c);
      }

      // if we are ina a new material block, specify in the file which material to use
      if(lastMat != material) {
        _out << "usemtl mat" << material << std::endl;
        lastMat = material;
      }
    }

    _out << "f";

    _be.get_vhandles(FaceHandle(int(i)), vhandles);

    for (j=0; j< vhandles.size(); ++j)
    {

      // Write vertex index
      idx = vhandles[j].idx() + 1;
      _out << " " << idx;

      if (!onlyVertices) {
        // write separator
        _out << "/" ;

        // write vertex texture coordinate index
        if (_opt.check(Options::VertexTexCoord))
          _out  << idx;

        // write vertex normal index
        if ( _opt.check(Options::VertexNormal) ) {
          // write separator
          _out << "/" ;
          _out << idx;
        }
      }
    }

    _out << std::endl;
  }

  material_.clear();
  materialA_.clear();

  return true;
}


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
