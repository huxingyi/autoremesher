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

// OpenMesh
#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/System/omstream.hh>
#include <OpenMesh/Core/Geometry/VectorT.hh>
#include <OpenMesh/Core/IO/BinaryHelper.hh>
#include <OpenMesh/Core/IO/IOManager.hh>
#include <OpenMesh/Core/IO/writer/STLWriter.hh>

//=== NAMESPACES ==============================================================


namespace OpenMesh {
namespace IO {


//=== INSTANCIATE =============================================================


_STLWriter_  __STLWriterInstance;
_STLWriter_& STLWriter() { return __STLWriterInstance; }


//=== IMPLEMENTATION ==========================================================


_STLWriter_::_STLWriter_() { IOManager().register_module(this); }


//-----------------------------------------------------------------------------


bool
_STLWriter_::
write(const std::string& _filename, BaseExporter& _be, Options _opt, std::streamsize _precision) const
{
  // binary or ascii ?
  if (_filename.rfind(".stla") != std::string::npos)
  {
    _opt -= Options::Binary;
  }
  else if (_filename.rfind(".stlb") != std::string::npos)
  {
    _opt += Options::Binary;
  }

  // open file
  std::fstream out(_filename.c_str(), (_opt.check(Options::Binary) ? std::ios_base::binary | std::ios_base::out
                                                                   : std::ios_base::out) );

  bool result = write(out, _be, _opt, _precision);

  out.close();

  return result;
}

//-----------------------------------------------------------------------------


bool
_STLWriter_::
write(std::ostream& _os, BaseExporter& _be, Options _opt, std::streamsize _precision) const
{
  // check exporter features
  if (!check(_be, _opt)) return false;

  // check writer features
  if (_opt.check(Options::VertexNormal)   ||
      _opt.check(Options::VertexTexCoord) ||
      _opt.check(Options::FaceColor))
    return false;

  if (!_opt.check(Options::Binary))
    _os.precision(_precision);

  if (_opt & Options::Binary)
    return write_stlb(_os, _be, _opt);
  else
    return write_stla(_os, _be, _opt);

  return false;
}



//-----------------------------------------------------------------------------


bool
_STLWriter_::
write_stla(const std::string& _filename, BaseExporter& _be, Options /* _opt */) const
{
  omlog() << "[STLWriter] : write ascii file\n";


  // open file
  FILE* out = fopen(_filename.c_str(), "w");
  if (!out)
  {
    omerr() << "[STLWriter] : cannot open file " << _filename << std::endl;
    return false;
  }




  int i, nF(int(_be.n_faces())), nV;
  Vec3f  a, b, c, n;
  std::vector<VertexHandle> vhandles;
  FaceHandle fh;


  // header
  fprintf(out, "solid\n");


  // write face set
  for (i=0; i<nF; ++i)
  {
    fh = FaceHandle(i);
    nV = _be.get_vhandles(fh, vhandles);

    if (nV == 3)
    {
      a = _be.point(vhandles[0]);
      b = _be.point(vhandles[1]);
      c = _be.point(vhandles[2]);
      n = (_be.has_face_normals() ?
     _be.normal(fh) :
     ((c-b) % (a-b)).normalize());

      fprintf(out, "facet normal %f %f %f\nouter loop\n", n[0], n[1], n[2]);
      fprintf(out, "vertex %.10f %.10f %.10f\n", a[0], a[1], a[2]);
      fprintf(out, "vertex %.10f %.10f %.10f\n", b[0], b[1], b[2]);
      fprintf(out, "vertex %.10f %.10f %.10f",   c[0], c[1], c[2]);
    }
    else
      omerr() << "[STLWriter] : Warning non-triangle data!\n";

    fprintf(out, "\nendloop\nendfacet\n");
  }

  fprintf(out, "endsolid\n");

  fclose(out);

  return true;
}


//-----------------------------------------------------------------------------


bool
_STLWriter_::
write_stla(std::ostream& _out, BaseExporter& _be, Options /* _opt */, std::streamsize _precision) const
{
  omlog() << "[STLWriter] : write ascii file\n";

  int i, nF(int(_be.n_faces())), nV;
  Vec3f  a, b, c, n;
  std::vector<VertexHandle> vhandles;
  FaceHandle fh;
  _out.precision(_precision);


  // header
  _out << "solid\n";


  // write face set
  for (i=0; i<nF; ++i)
  {
    fh = FaceHandle(i);
    nV = _be.get_vhandles(fh, vhandles);

    if (nV == 3)
    {
      a = _be.point(vhandles[0]);
      b = _be.point(vhandles[1]);
      c = _be.point(vhandles[2]);
      n = (_be.has_face_normals() ?
     _be.normal(fh) :
     ((c-b) % (a-b)).normalize());

      _out << "facet normal " << n[0] << " " << n[1] << " " << n[2] << "\nouter loop\n";
      _out.precision(10);
      _out << "vertex " << a[0] << " " << a[1] << " " << a[2] << "\n";
      _out << "vertex " << b[0] << " " << b[1] << " " << b[2] << "\n";
      _out << "vertex " << c[0] << " " << c[1] << " " << c[2] << "\n";
    } else {
      omerr() << "[STLWriter] : Warning non-triangle data!\n";
    }

    _out << "\nendloop\nendfacet\n";
  }

  _out << "endsolid\n";

  return true;
}

//-----------------------------------------------------------------------------


bool
_STLWriter_::
write_stlb(const std::string& _filename, BaseExporter& _be, Options /* _opt */) const
{
  omlog() << "[STLWriter] : write binary file\n";


  // open file
  FILE* out = fopen(_filename.c_str(), "wb");
  if (!out)
  {
    omerr() << "[STLWriter] : cannot open file " << _filename << std::endl;
    return false;
  }


  int i, nF(int(_be.n_faces())), nV;
  Vec3f  a, b, c, n;
  std::vector<VertexHandle> vhandles;
  FaceHandle fh;


   // write header
  const char header[80] =
    "binary stl file"
    "                                                                ";
  fwrite(header, 1, 80, out);


  // number of faces
  write_int( int(_be.n_faces()), out);


  // write face set
  for (i=0; i<nF; ++i)
  {
    fh = FaceHandle(i);
    nV = _be.get_vhandles(fh, vhandles);

    if (nV == 3)
    {
      a = _be.point(vhandles[0]);
      b = _be.point(vhandles[1]);
      c = _be.point(vhandles[2]);
      n = (_be.has_face_normals() ?
     _be.normal(fh) :
     ((c-b) % (a-b)).normalize());

      // face normal
      write_float(n[0], out);
      write_float(n[1], out);
      write_float(n[2], out);

      // face vertices
      write_float(a[0], out);
      write_float(a[1], out);
      write_float(a[2], out);

      write_float(b[0], out);
      write_float(b[1], out);
      write_float(b[2], out);

      write_float(c[0], out);
      write_float(c[1], out);
      write_float(c[2], out);

      // space filler
      write_short(0, out);
    }
    else
      omerr() << "[STLWriter] : Warning: Skipped non-triangle data!\n";
  }


  fclose(out);
  return true;
}

//-----------------------------------------------------------------------------

bool
_STLWriter_::
write_stlb(std::ostream& _out, BaseExporter& _be, Options /* _opt */, std::streamsize _precision) const
{
  omlog() << "[STLWriter] : write binary file\n";


  int i, nF(int(_be.n_faces())), nV;
  Vec3f  a, b, c, n;
  std::vector<VertexHandle> vhandles;
  FaceHandle fh;
  _out.precision(_precision);


   // write header
  const char header[80] =
    "binary stl file"
    "                                                                ";
  _out.write(header, 80);


  // number of faces
  write_int(int(_be.n_faces()), _out);


  // write face set
  for (i=0; i<nF; ++i)
  {
    fh = FaceHandle(i);
    nV = _be.get_vhandles(fh, vhandles);

    if (nV == 3)
    {
      a = _be.point(vhandles[0]);
      b = _be.point(vhandles[1]);
      c = _be.point(vhandles[2]);
      n = (_be.has_face_normals() ?
     _be.normal(fh) :
     ((c-b) % (a-b)).normalize());

      // face normal
      write_float(n[0], _out);
      write_float(n[1], _out);
      write_float(n[2], _out);

      // face vertices
      write_float(a[0], _out);
      write_float(a[1], _out);
      write_float(a[2], _out);

      write_float(b[0], _out);
      write_float(b[1], _out);
      write_float(b[2], _out);

      write_float(c[0], _out);
      write_float(c[1], _out);
      write_float(c[2], _out);

      // space filler
      write_short(0, _out);
    }
    else
      omerr() << "[STLWriter] : Warning: Skipped non-triangle data!\n";
  }


  return true;
}


//-----------------------------------------------------------------------------


size_t
_STLWriter_::
binary_size(BaseExporter& _be, Options /* _opt */) const
{
  size_t bytes(0);
  size_t _12floats(12*sizeof(float));

  bytes += 80; // header
  bytes += 4;  // #faces


  int i, nF(int(_be.n_faces()));
  std::vector<VertexHandle> vhandles;

  for (i=0; i<nF; ++i)
    if (_be.get_vhandles(FaceHandle(i), vhandles) == 3)
      bytes += _12floats + sizeof(short);
    else
      omerr() << "[STLWriter] : Warning: Skipped non-triangle data!\n";

  return bytes;
}


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
