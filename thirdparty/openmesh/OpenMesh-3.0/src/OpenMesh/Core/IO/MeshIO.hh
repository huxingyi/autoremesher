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


#ifndef OM_MESHIO_HH
#define OM_MESHIO_HH


//=== INCLUDES ================================================================

// -------------------- system settings
#include <OpenMesh/Core/System/config.h>
// -------------------- check include order
#if defined (OPENMESH_TRIMESH_ARRAY_KERNEL_HH) || \
    defined (OPENMESH_POLYMESH_ARRAY_KERNEL_HH)
#  error "Include MeshIO.hh before including a mesh type!"
#endif
// -------------------- OpenMesh
#include <OpenMesh/Core/IO/SR_store.hh>
#include <OpenMesh/Core/IO/IOManager.hh>
#include <OpenMesh/Core/IO/importer/ImporterT.hh>
#include <OpenMesh/Core/IO/exporter/ExporterT.hh>


//== NAMESPACES ==============================================================

namespace OpenMesh {
namespace IO   {


//=== IMPLEMENTATION ==========================================================


/** \name Mesh Reading / Writing
    Convenience functions the map to IOManager functions.
    \see OpenMesh::IO::_IOManager_
*/
//@{


//-----------------------------------------------------------------------------


/** \brief Read a mesh from file _filename.

    The file format is determined by the file extension.

    \note If you link statically against OpenMesh, you have to add
          the define OM_STATIC_BUILD to your application. This will
          ensure that readers and writers get initialized correctly.

    @param _mesh     The target mesh that will be filled with the read data
    @param _filename fill to load

    @return Successful?
 */
template <class Mesh>
bool
read_mesh(Mesh&         _mesh,
	  const std::string&  _filename)
{
  Options opt;
  return read_mesh(_mesh, _filename, opt, true);
}


/** \brief Read a mesh from file _filename.

    The file format is determined by the file extension.

    \note If you link statically against OpenMesh, you have to add
          the define OM_STATIC_BUILD to your application. This will
          ensure that readers and writers get initialized correctly.

    @param _mesh     The target mesh that will be filled with the read data
    @param _filename fill to load
    @param _opt      Reader options (e.g. skip loading of normals ... depends
                     on the reader capabilities). Note that simply passing an
                     Options::Flag enum is not sufficient.
    @param _clear    Clear the target data before filling it (allows to
                     load multiple files into one Mesh). If you only want to read a mesh
                     without clearing set _clear to false. Providing a default Options
                     object is sufficient in this case.

    @return Successful?
*/
template <class Mesh>
bool
read_mesh(Mesh&         _mesh,
	  const std::string&  _filename,
	  Options&            _opt,
	  bool                _clear = true)
{
  if (_clear) _mesh.clear();
  ImporterT<Mesh> importer(_mesh);
  return IOManager().read(_filename, importer, _opt);
}


/** \brief Read a mesh from file open std::istream.

    The file format is determined by parameter _ext. _ext has to include
    ".[format]" in order to work properly (e.g. ".OFF")

    \note If you link statically against OpenMesh, you have to add
          the define OM_STATIC_BUILD to your application. This will
          ensure that readers and writers get initialized correctly.

    @param _mesh     The target mesh that will be filled with the read data
    @param _is       stream to load the data from
    @param _ext      The file format that is written to the stream
    @param _opt      Reader options (e.g. skip loading of normals ... depends
                     on the reader capabilities)
    @param _clear    Clear the target data before filling it (allows to
                     load multiple files into one Mesh)

    @return Successful?
*/
template <class Mesh>
bool
read_mesh(Mesh&         _mesh,
	  std::istream&       _is,
	  const std::string&  _ext,
	  Options&            _opt,
	  bool                _clear = true)
{
  if (_clear) _mesh.clear();
  ImporterT<Mesh> importer(_mesh);
  return IOManager().read(_is,_ext, importer, _opt);
}



//-----------------------------------------------------------------------------


/** \brief Write a mesh to the file _filename.

    The file format is determined by _filename's extension.

    \note If you link statically against OpenMesh, you have to add
          the define OM_STATIC_BUILD to your application. This will
          ensure that readers and writers get initialized correctly.

    @param _mesh     The mesh that will be written to file
    @param _filename output filename
    @param _opt      Writer options (e.g. writing of normals ... depends
                     on the writer capabilities)
    @param _precision specifies stream precision for ascii files

    @return Successful?
*/
template <class Mesh>
bool write_mesh(const Mesh&        _mesh,
                const std::string& _filename,
                Options            _opt = Options::Default,
                std::streamsize    _precision = 6)
{
  ExporterT<Mesh> exporter(_mesh);
  return IOManager().write(_filename, exporter, _opt, _precision);
}


//-----------------------------------------------------------------------------


/** Write a mesh to an open std::ostream.

    The file format is determined by parameter _ext. _ext has to include
    ".[format]" in order to work properly (e.g. ".OFF")

    \note If you link statically against OpenMesh, you have to add
          the define OM_STATIC_BUILD to your application. This will
          ensure that readers and writers get initialized correctly.

    @param _mesh     The mesh that will be written to file
    @param _os       output stream to write into
    @param _ext      extension defining the type of output
    @param _opt      Writer options (e.g. writing of normals ... depends
                     on the writer capabilities)
    @param _precision specifies stream precision for ascii files

    @return Successful?
*/
template <class Mesh>
bool write_mesh(const Mesh&        _mesh,
		std::ostream&      _os,
	        const std::string& _ext,
                Options            _opt = Options::Default,
                std::streamsize    _precision = 6)
{
  ExporterT<Mesh> exporter(_mesh);
  return IOManager().write(_os,_ext, exporter, _opt, _precision);
}


//-----------------------------------------------------------------------------

/** \brief Get binary size of data

  This function calls the corresponding writer which calculates the size
  of the data that would be written to a binary file

  The file format is determined by parameter _ext. _ext has to include
  ".[format]" in order to work properly (e.g. ".OFF")

  @param _mesh Mesh to write
  @param _ext extension of the file (used to determine the writing module)
  @param _opt  Writer options (e.g. writing of normals ... depends
               on the writer capabilities)

  @return Binary size in bytes used when writing the data
*/
template <class Mesh>
size_t binary_size(const Mesh&        _mesh,
                   const std::string& _ext,
                   Options            _opt = Options::Default)
{
  ExporterT<Mesh> exporter(_mesh);
  return IOManager().binary_size(_ext, exporter, _opt);
}


//-----------------------------------------------------------------------------

//@}


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
#if defined(OM_STATIC_BUILD) || defined(ARCH_DARWIN)
#  include <OpenMesh/Core/IO/IOInstances.hh>
#endif
//=============================================================================
#endif
//=============================================================================
