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


#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/System/omstream.hh>
#include <OpenMesh/Core/Utils/Endian.hh>
#include <OpenMesh/Core/IO/IOManager.hh>
#include <OpenMesh/Core/IO/BinaryHelper.hh>
#include <OpenMesh/Core/IO/writer/PLYWriter.hh>

#include <OpenMesh/Core/IO/SR_store.hh>

//=== NAMESPACES ==============================================================


namespace OpenMesh {
namespace IO {


//=== INSTANCIATE =============================================================


// register the PLYLoader singleton with MeshLoader
_PLYWriter_  __PLYWriterInstance;
_PLYWriter_& PLYWriter() { return __PLYWriterInstance; }


//=== IMPLEMENTATION ==========================================================


_PLYWriter_::_PLYWriter_() { IOManager().register_module(this); }


//-----------------------------------------------------------------------------


bool
_PLYWriter_::
write(const std::string& _filename, BaseExporter& _be, Options _opt, std::streamsize _precision) const
{
  // check exporter features
  if ( !check( _be, _opt ) )
    return false;


  // check writer features
  if ( _opt.check(Options::FaceNormal) ) {
    // Face normals are not supported
    // Uncheck these options and output message that
    // they are not written out even though they were requested
    _opt.unset(Options::FaceNormal);
    omerr() << "[PLYWriter] : Warning: Face normals are not supported and thus not exported! " << std::endl;
  }

  if ( _opt.check(Options::FaceColor) ) {
    // Face normals are not supported
    // Uncheck these options and output message that
    // they are not written out even though they were requested
    _opt.unset(Options::FaceColor);
    omerr() << "[PLYWriter] : Warning: Face colors are not supported and thus not exported! " << std::endl;
  }

  options_ = _opt;

  // open file
  std::fstream out(_filename.c_str(), (_opt.check(Options::Binary) ? std::ios_base::binary | std::ios_base::out
                                                         : std::ios_base::out) );
  if (!out)
  {
    omerr() << "[PLYWriter] : cannot open file "
    << _filename
    << std::endl;
    return false;
  }

  if (!_opt.check(Options::Binary))
    out.precision(_precision);

  // write to file
  bool result = (_opt.check(Options::Binary) ?
     write_binary(out, _be, _opt) :
     write_ascii(out, _be, _opt));

  // return result
  out.close();
  return result;
}

//-----------------------------------------------------------------------------


bool
_PLYWriter_::
write(std::ostream& _os, BaseExporter& _be, Options _opt, std::streamsize _precision) const
{
  // check exporter features
  if ( !check( _be, _opt ) )
    return false;


  // check writer features
  if ( _opt.check(Options::FaceNormal) || _opt.check(Options::FaceColor) ) // not supported yet
    return false;

  options_ = _opt;


  if (!_os.good())
  {
    omerr() << "[PLYWriter] : cannot write to stream "
    << std::endl;
    return false;
  }

  if (!_opt.check(Options::Binary))
    _os.precision(_precision);

  // write to file
  bool result = (_opt.check(Options::Binary) ?
     write_binary(_os, _be, _opt) :
     write_ascii(_os, _be, _opt));

  return result;
}


//-----------------------------------------------------------------------------


void _PLYWriter_::write_header(std::ostream& _out, BaseExporter& _be, Options& _opt) const {
  //writing header
  _out << "ply" << std::endl;

  if (_opt.is_binary()) {
    _out << "format ";
    if ( options_.check(Options::MSB) )
      _out << "binary_big_endian ";
    else
      _out << "binary_little_endian ";
    _out << "1.0" << std::endl;
  } else
    _out << "format ascii 1.0" << std::endl;

  _out << "element vertex " << _be.n_vertices() << std::endl;

  _out << "property float x" << std::endl;
  _out << "property float y" << std::endl;
  _out << "property float z" << std::endl;

  if ( _opt.vertex_has_normal() ){
    _out << "property float nx" << std::endl;
    _out << "property float ny" << std::endl;
    _out << "property float nz" << std::endl;
  }

  if ( _opt.vertex_has_texcoord() ){
    _out << "property float u" << std::endl;
    _out << "property float v" << std::endl;
  }

  if ( _opt.vertex_has_color() ){
    if ( _opt.color_is_float() ) {
      _out << "property float red" << std::endl;
      _out << "property float green" << std::endl;
      _out << "property float blue" << std::endl;

      if ( _opt.color_has_alpha() )
        _out << "property float alpha" << std::endl;
    } else {
      _out << "property uchar red" << std::endl;
      _out << "property uchar green" << std::endl;
      _out << "property uchar blue" << std::endl;

      if ( _opt.color_has_alpha() )
        _out << "property uchar alpha" << std::endl;
    }
  }

  _out << "element face " << _be.n_faces() << std::endl;
  _out << "property list uchar int vertex_indices" << std::endl;
  _out << "end_header" << std::endl;
}


//-----------------------------------------------------------------------------


bool
_PLYWriter_::
write_ascii(std::ostream& _out, BaseExporter& _be, Options _opt) const
{
  omlog() << "[PLYWriter] : write ascii file\n";

  unsigned int i, nV, nF;
  Vec3f v, n;
  OpenMesh::Vec3ui c;
  OpenMesh::Vec4ui cA;
  OpenMesh::Vec3f cf;
  OpenMesh::Vec4f cAf;
  OpenMesh::Vec2f t;
  VertexHandle vh;
  std::vector<VertexHandle> vhandles;

  write_header(_out, _be, _opt);

  if (_opt.color_is_float())
    _out << std::fixed;

  // vertex data (point, normals, colors, texcoords)
  for (i=0, nV=int(_be.n_vertices()); i<nV; ++i)
  {
    vh = VertexHandle(i);
    v  = _be.point(vh);

    //Vertex
    _out << v[0] << " " << v[1] << " " << v[2];

    // Vertex Normals
    if ( _opt.vertex_has_normal() ){
      n = _be.normal(vh);
      _out << " " << n[0] << " " << n[1] << " " << n[2];
    }

    // Vertex TexCoords
    if ( _opt.vertex_has_texcoord() ) {
    	t = _be.texcoord(vh);
    	_out << " " << t[0] << " " << t[1];
    }

    // VertexColor
    if ( _opt.vertex_has_color() ) {
      //with alpha
      if ( _opt.color_has_alpha() ){
        if (_opt.color_is_float()) {
          cAf = _be.colorAf(vh);
          _out << " " << cAf;
        } else {
          cA  = _be.colorAi(vh);
          _out << " " << cA;
        }
      }else{
        //without alpha
        if (_opt.color_is_float()) {
          cf = _be.colorf(vh);
          _out << " " << cf;
        } else {
          c  = _be.colori(vh);
          _out << " " << c;
        }
      }
    }

    _out << "\n";
  }

  // faces (indices starting at 0)
  if (_be.is_triangle_mesh())
  {
    for (i=0, nF=int(_be.n_faces()); i<nF; ++i)
    {
      _be.get_vhandles(FaceHandle(i), vhandles);
      _out << 3 << " ";
      _out << vhandles[0].idx()  << " ";
      _out << vhandles[1].idx()  << " ";
      _out << vhandles[2].idx();

//       //face color
//       if ( _opt.face_has_color() ){
//         //with alpha
//         if ( _opt.color_has_alpha() ){
//           cA  = _be.colorA( FaceHandle(i) );
//           _out << " " << cA[0] << " " << cA[1] << " " << cA[2] << " " << cA[3];
//         }else{
//           //without alpha
//           c  = _be.color( FaceHandle(i) );
//           _out << " " << c[0] << " " << c[1] << " " << c[2];
//         }
//       }
      _out << "\n";
    }
  }
  else
  {
    for (i=0, nF=int(_be.n_faces()); i<nF; ++i)
    {
      nV = _be.get_vhandles(FaceHandle(i), vhandles);
      _out << nV << " ";
      for (size_t j=0; j<vhandles.size(); ++j)
         _out << vhandles[j].idx() << " ";

//       //face color
//       if ( _opt.face_has_color() ){
//         //with alpha
//         if ( _opt.color_has_alpha() ){
//           cA  = _be.colorA( FaceHandle(i) );
//           _out << cA[0] << " " << cA[1] << " " << cA[2] << " " << cA[3];
//         }else{
//           //without alpha
//           c  = _be.color( FaceHandle(i) );
//           _out << c[0] << " " << c[1] << " " << c[2];
//         }
//       }

      _out << "\n";
    }
  }


  return true;
}


//-----------------------------------------------------------------------------

void _PLYWriter_::writeValue(ValueType _type, std::ostream& _out, int value) const {

  uint32_t tmp32;
  uint8_t tmp8;

  switch (_type) {
    case ValueTypeINT:
    case ValueTypeINT32:
      tmp32 = value;
      store(_out, tmp32, options_.check(Options::MSB) );
      break;
//     case ValueTypeUINT8:
default :
      tmp8 = value;
      store(_out, tmp8, options_.check(Options::MSB) );
      break;
//     default :
//       std::cerr << "unsupported conversion type to int: " << _type << std::endl;
//       break;
  }
}

void _PLYWriter_::writeValue(ValueType _type, std::ostream& _out, unsigned int value) const {

  uint32_t tmp32;
  uint8_t tmp8;

  switch (_type) {
    case ValueTypeINT:
    case ValueTypeINT32:
      tmp32 = value;
      store(_out, tmp32, options_.check(Options::MSB) );
      break;
//     case ValueTypeUINT8:
default :
      tmp8 = value;
      store(_out, tmp8, options_.check(Options::MSB) );
      break;
//     default :
//       std::cerr << "unsupported conversion type to int: " << _type << std::endl;
//       break;
  }
}

void _PLYWriter_::writeValue(ValueType _type, std::ostream& _out, float value) const {

  float32_t tmp;

  switch (_type) {
    case ValueTypeFLOAT32:
    case ValueTypeFLOAT:
      tmp = value;
      store( _out , tmp, options_.check(Options::MSB) );
      break;
    default :
      std::cerr << "unsupported conversion type to float: " << _type << std::endl;
      break;
  }
}

bool
_PLYWriter_::
write_binary(std::ostream& _out, BaseExporter& _be, Options _opt) const
{
  omlog() << "[PLYWriter] : write binary file\n";

  unsigned int i, nV, nF;
  Vec3f v, n;
  Vec2f t;
  OpenMesh::Vec4uc c;
  OpenMesh::Vec4f cf;
  VertexHandle vh;
  std::vector<VertexHandle> vhandles;

  write_header(_out, _be, _opt);

  // vertex data (point, normals, texcoords)
  for (i=0, nV=int(_be.n_vertices()); i<nV; ++i)
  {
    vh = VertexHandle(i);
    v  = _be.point(vh);

    //vertex
    writeValue(ValueTypeFLOAT, _out, v[0]);
    writeValue(ValueTypeFLOAT, _out, v[1]);
    writeValue(ValueTypeFLOAT, _out, v[2]);

    // Vertex Normal
    if ( _opt.vertex_has_normal() ){
      n = _be.normal(vh);
      writeValue(ValueTypeFLOAT, _out, n[0]);
      writeValue(ValueTypeFLOAT, _out, n[1]);
      writeValue(ValueTypeFLOAT, _out, n[2]);
    }

    // Vertex TexCoords
    if ( _opt.vertex_has_texcoord() ) {
    	t = _be.texcoord(vh);
    	writeValue(ValueTypeFLOAT, _out, t[0]);
    	writeValue(ValueTypeFLOAT, _out, t[1]);
    }

    // vertex color
    if ( _opt.vertex_has_color() ) {
        if ( _opt.color_is_float() ) {
          cf  = _be.colorAf(vh);
          writeValue(ValueTypeFLOAT, _out, cf[0]);
          writeValue(ValueTypeFLOAT, _out, cf[1]);
          writeValue(ValueTypeFLOAT, _out, cf[2]);

          if ( _opt.color_has_alpha() )
            writeValue(ValueTypeFLOAT, _out, cf[3]);
        } else {
          c  = _be.colorA(vh);
          writeValue(ValueTypeUCHAR, _out, (int)c[0]);
          writeValue(ValueTypeUCHAR, _out, (int)c[1]);
          writeValue(ValueTypeUCHAR, _out, (int)c[2]);

          if ( _opt.color_has_alpha() )
            writeValue(ValueTypeUCHAR, _out, (int)c[3]);
        }
    }
  }

  // faces (indices starting at 0)
  if (_be.is_triangle_mesh())
  {
    for (i=0, nF=int(_be.n_faces()); i<nF; ++i)
    {
      //face
      _be.get_vhandles(FaceHandle(i), vhandles);
      writeValue(ValueTypeUINT8, _out, 3);
      writeValue(ValueTypeINT32, _out, vhandles[0].idx());
      writeValue(ValueTypeINT32, _out, vhandles[1].idx());
      writeValue(ValueTypeINT32, _out, vhandles[2].idx());

//       //face color
//       if ( _opt.face_has_color() ){
//         c  = _be.colorA( FaceHandle(i) );
//         writeValue(_out, c[0]);
//         writeValue(_out, c[1]);
//         writeValue(_out, c[2]);
//
//         if ( _opt.color_has_alpha() )
//           writeValue(_out, c[3]);
//       }
    }
  }
  else
  {
    for (i=0, nF=int(_be.n_faces()); i<nF; ++i)
    {
      //face
      nV = _be.get_vhandles(FaceHandle(i), vhandles);
      writeValue(ValueTypeUINT8, _out, nV);
      for (size_t j=0; j<vhandles.size(); ++j)
        writeValue(ValueTypeINT32, _out, vhandles[j].idx() );

//       //face color
//       if ( _opt.face_has_color() ){
//         c  = _be.colorA( FaceHandle(i) );
//         writeValue(_out, c[0]);
//         writeValue(_out, c[1]);
//         writeValue(_out, c[2]);
//
//         if ( _opt.color_has_alpha() )
//           writeValue(_out, c[3]);
//       }
    }
  }

  return true;
}

// ----------------------------------------------------------------------------


size_t
_PLYWriter_::
binary_size(BaseExporter& _be, Options _opt) const
{
  size_t header(0);
  size_t data(0);
  size_t _3longs(3*sizeof(long));
  size_t _3floats(3*sizeof(float));
  size_t _3ui(3*sizeof(unsigned int));
  size_t _4ui(4*sizeof(unsigned int));

  if ( !_opt.is_binary() )
    return 0;
  else
  {
    header += 11;                             // 'OFF BINARY\n'
    header += _3longs;                        // #V #F #E
    data   += _be.n_vertices() * _3floats;    // vertex data
  }

  if ( _opt.vertex_has_normal() && _be.has_vertex_normals() )
  {
    header += 1; // N
    data   += _be.n_vertices() * _3floats;
  }

  if ( _opt.vertex_has_color() && _be.has_vertex_colors() )
  {
    header += 1; // C
    data   += _be.n_vertices() * _3floats;
  }

  if ( _opt.vertex_has_texcoord() && _be.has_vertex_texcoords() )
  {
    size_t _2floats(2*sizeof(float));
    header += 2; // ST
    data   += _be.n_vertices() * _2floats;
  }

  // topology
  if (_be.is_triangle_mesh())
  {
    data += _be.n_faces() * _4ui;
  }
  else
  {
    unsigned int i, nF;
    std::vector<VertexHandle> vhandles;

    for (i=0, nF=int(_be.n_faces()); i<nF; ++i)
    {
      data += _be.get_vhandles(FaceHandle(i), vhandles) * sizeof(unsigned int);

    }
  }

  // face colors
  if ( _opt.face_has_color() && _be.has_face_colors() ){
    if ( _opt.color_has_alpha() )
      data += _be.n_faces() * _4ui;
    else
      data += _be.n_faces() * _3ui;
  }

  return header+data;
}


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
