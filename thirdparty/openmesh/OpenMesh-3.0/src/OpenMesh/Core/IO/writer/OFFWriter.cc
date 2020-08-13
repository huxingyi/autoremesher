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
#include <OpenMesh/Core/IO/writer/OFFWriter.hh>

#include <OpenMesh/Core/IO/SR_store.hh>

//=== NAMESPACES ==============================================================


namespace OpenMesh {
namespace IO {


//=== INSTANCIATE =============================================================


// register the OFFLoader singleton with MeshLoader
_OFFWriter_  __OFFWriterInstance;
_OFFWriter_& OFFWriter() { return __OFFWriterInstance; }


//=== IMPLEMENTATION ==========================================================


_OFFWriter_::_OFFWriter_() { IOManager().register_module(this); }


//-----------------------------------------------------------------------------


bool
_OFFWriter_::
write(const std::string& _filename, BaseExporter& _be, Options _opt, std::streamsize _precision) const
{
  // check exporter features
  if ( !check( _be, _opt ) )
    return false;


  // check writer features
  if ( _opt.check(Options::FaceNormal) ) // not supported by format
    return false;

  // open file
  std::fstream out(_filename.c_str(), (_opt.check(Options::Binary) ? std::ios_base::binary | std::ios_base::out
                                                         : std::ios_base::out) );
  if (!out)
  {
    omerr() << "[OFFWriter] : cannot open file "
	  << _filename
	  << std::endl;
    return false;
  }

  // write header line
  if (_opt.check(Options::VertexTexCoord)) out << "ST";
  if (_opt.check(Options::VertexColor) || _opt.check(Options::FaceColor))    out << "C";
  if (_opt.check(Options::VertexNormal))   out << "N";
  out << "OFF";
  if (_opt.check(Options::Binary)) out << " BINARY";
  out << "\n";


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
_OFFWriter_::
write(std::ostream& _os, BaseExporter& _be, Options _opt, std::streamsize _precision) const
{
  // check exporter features
  if ( !check( _be, _opt ) )
    return false;


  // check writer features
  if ( _opt.check(Options::FaceNormal) ) // not supported by format
    return false;


  if (!_os.good())
  {
    omerr() << "[OFFWriter] : cannot write to stream "
	  << std::endl;
    return false;
  }

  // write header line
  if (_opt.check(Options::VertexTexCoord)) _os << "ST";
  if (_opt.check(Options::VertexColor) || _opt.check(Options::FaceColor))    _os << "C";
  if (_opt.check(Options::VertexNormal))   _os << "N";
  _os << "OFF";
  if (_opt.check(Options::Binary)) _os << " BINARY";
  _os << "\n";

  if (!_opt.check(Options::Binary))
    _os.precision(_precision);

  // write to file
  bool result = (_opt.check(Options::Binary) ?
		 write_binary(_os, _be, _opt) :
		 write_ascii(_os, _be, _opt));


  // return result
  return result;
}

//-----------------------------------------------------------------------------


bool
_OFFWriter_::
write_ascii(std::ostream& _out, BaseExporter& _be, Options _opt) const
{
  omlog() << "[OFFWriter] : write ascii file\n";


  unsigned int i, nV, nF;
  Vec3f v, n;
  Vec2f t;
  OpenMesh::Vec3i c;
  OpenMesh::Vec4i cA;
  OpenMesh::Vec3f cf;
  OpenMesh::Vec4f cAf;
  VertexHandle vh;
  std::vector<VertexHandle> vhandles;


  // #vertices, #faces
  _out << _be.n_vertices() << " ";
  _out << _be.n_faces() << " ";
  _out << 0 << "\n";

  if (_opt.color_is_float())
    _out << std::fixed;


  // vertex data (point, normals, colors, texcoords)
  for (i=0, nV=int(_be.n_vertices()); i<nV; ++i)
  {
    vh = VertexHandle(i);
    v  = _be.point(vh);

    //Vertex
    _out << v[0] << " " << v[1] << " " << v[2];

    // VertexNormal
    if ( _opt.vertex_has_normal() ) {
      n  = _be.normal(vh);
      _out << " " << n[0] << " " << n[1] << " " << n[2];
    }

    // VertexColor
    if ( _opt.vertex_has_color() ) {
      if ( _opt.color_is_float() ) {
        //with alpha
        if ( _opt.color_has_alpha() ){
          cAf  = _be.colorAf(vh);
          _out << " " << cAf;
        }else{
          //without alpha
          cf  = _be.colorf(vh);
          _out << " " << cf;
        }
      } else {
        //with alpha
        if ( _opt.color_has_alpha() ){
          cA  = _be.colorA(vh);
          _out << " " << cA;
        }else{
          //without alpha
          c  = _be.color(vh);
          _out << " " << c;
        }
      }
    }

    // TexCoord
    if (_opt.vertex_has_texcoord() ) {
      t  = _be.texcoord(vh);
      _out << " " << t[0] << " " << t[1];
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

      //face color
      if ( _opt.face_has_color() ){
        if ( _opt.color_is_float() ) {
          //with alpha
          if ( _opt.color_has_alpha() ){
            cAf  = _be.colorAf( FaceHandle(i) );
            _out << " " << cAf;
          }else{
            //without alpha
            cf  = _be.colorf( FaceHandle(i) );
            _out << " " << cf;
          }
        } else {
          //with alpha
          if ( _opt.color_has_alpha() ){
            cA  = _be.colorA( FaceHandle(i) );
            _out << " " << cA;
          }else{
            //without alpha
            c  = _be.color( FaceHandle(i) );
            _out << " " << c;
          }
        }
      }
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

      //face color
      if ( _opt.face_has_color() ){
        if ( _opt.color_is_float() ) {
          //with alpha
          if ( _opt.color_has_alpha() ){
            cAf  = _be.colorAf( FaceHandle(i) );
            _out << " " << cAf;
          }else{
            //without alpha
            cf  = _be.colorf( FaceHandle(i) );
            _out << " " << cf;
          }
        } else {
          //with alpha
          if ( _opt.color_has_alpha() ){
            cA  = _be.colorA( FaceHandle(i) );
            _out << " " << cA;
          }else{
            //without alpha
            c  = _be.color( FaceHandle(i) );
            _out << " " << c;
          }
        }
      }

      _out << "\n";
    }
  }


  return true;
}


//-----------------------------------------------------------------------------

void _OFFWriter_::writeValue(std::ostream& _out, int value) const {

  uint32_t tmp = value;
  store(_out, tmp, false);
}

void _OFFWriter_::writeValue(std::ostream& _out, unsigned int value) const {

  uint32_t tmp = value;
  store(_out, tmp, false);
}

void _OFFWriter_::writeValue(std::ostream& _out, float value) const {

  float32_t tmp = value;
  store(_out, tmp, false);
}

bool
_OFFWriter_::
write_binary(std::ostream& _out, BaseExporter& _be, Options _opt) const
{
  omlog() << "[OFFWriter] : write binary file\n";


  unsigned int i, nV, nF;
  Vec3f v, n;
  Vec2f t;
  OpenMesh::Vec4i c;
  OpenMesh::Vec4f cf;
  VertexHandle vh;
  std::vector<VertexHandle> vhandles;

  // #vertices, #faces
  writeValue(_out, (uint)_be.n_vertices() );
  writeValue(_out, (uint) _be.n_faces() );
  writeValue(_out, 0 );

  // vertex data (point, normals, texcoords)
  for (i=0, nV=int(_be.n_vertices()); i<nV; ++i)
  {
    vh = VertexHandle(i);
    v  = _be.point(vh);

    //vertex
    writeValue(_out, v[0]);
    writeValue(_out, v[1]);
    writeValue(_out, v[2]);

    // vertex normal
    if ( _opt.vertex_has_normal() ) {
      n  = _be.normal(vh);
      writeValue(_out, n[0]);
      writeValue(_out, n[1]);
      writeValue(_out, n[2]);
    }
    // vertex color
    if ( _opt.vertex_has_color() ) {
      if ( _opt.color_is_float() ) {
        cf  = _be.colorAf(vh);
        writeValue(_out, cf[0]);
        writeValue(_out, cf[1]);
        writeValue(_out, cf[2]);

        if ( _opt.color_has_alpha() )
          writeValue(_out, cf[3]);
      } else {
        c  = _be.colorA(vh);
        writeValue(_out, c[0]);
        writeValue(_out, c[1]);
        writeValue(_out, c[2]);

        if ( _opt.color_has_alpha() )
          writeValue(_out, c[3]);
      }
    }
    // texCoords
    if (_opt.vertex_has_texcoord() ) {
      t  = _be.texcoord(vh);
      writeValue(_out, t[0]);
      writeValue(_out, t[1]);
    }

  }

  // faces (indices starting at 0)
  if (_be.is_triangle_mesh())
  {
    for (i=0, nF=int(_be.n_faces()); i<nF; ++i)
    {
      //face
      _be.get_vhandles(FaceHandle(i), vhandles);
      writeValue(_out, 3);
      writeValue(_out, vhandles[0].idx());
      writeValue(_out, vhandles[1].idx());
      writeValue(_out, vhandles[2].idx());

      //face color
      if ( _opt.face_has_color() ){
        if ( _opt.color_is_float() ) {
          cf  = _be.colorAf( FaceHandle(i) );
          writeValue(_out, cf[0]);
          writeValue(_out, cf[1]);
          writeValue(_out, cf[2]);

          if ( _opt.color_has_alpha() )
            writeValue(_out, cf[3]);
        } else {
          c  = _be.colorA( FaceHandle(i) );
          writeValue(_out, c[0]);
          writeValue(_out, c[1]);
          writeValue(_out, c[2]);

          if ( _opt.color_has_alpha() )
            writeValue(_out, c[3]);
        }
      }
    }
  }
  else
  {
    for (i=0, nF=int(_be.n_faces()); i<nF; ++i)
    {
      //face
      nV = _be.get_vhandles(FaceHandle(i), vhandles);
      writeValue(_out, nV);
      for (size_t j=0; j<vhandles.size(); ++j)
        writeValue(_out, vhandles[j].idx() );

      //face color
      if ( _opt.face_has_color() ){
        if ( _opt.color_is_float() ) {
          cf  = _be.colorAf( FaceHandle(i) );
          writeValue(_out, cf[0]);
          writeValue(_out, cf[1]);
          writeValue(_out, cf[2]);

          if ( _opt.color_has_alpha() )
            writeValue(_out, cf[3]);
        } else {
          c  = _be.colorA( FaceHandle(i) );
          writeValue(_out, c[0]);
          writeValue(_out, c[1]);
          writeValue(_out, c[2]);

          if ( _opt.color_has_alpha() )
            writeValue(_out, c[3]);
        }
      }
    }
  }

  return true;
}

// ----------------------------------------------------------------------------


size_t
_OFFWriter_::
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
      data += _be.get_vhandles(FaceHandle(i), vhandles) * sizeof(unsigned int);

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
