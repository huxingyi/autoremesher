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

#ifndef OPENMESH_APP_SERVERSIDEVDPM_HH
#define OPENMESH_APP_SERVERSIDEVDPM_HH

#include <OpenMesh/Core/Geometry/VectorT.hh>
#include <OpenMesh/Tools/VDPM/VHierarchy.hh>
#include <vector>
#include <string>


using OpenMesh::VDPM::VHierarchy;
using OpenMesh::VDPM::VHierarchyNodeHandleContainer;


class ServerSideVDPM
{
private:

  char  name_[256];

  std::vector<OpenMesh::Vec3f>  points_;        // points related to this pm
  std::vector<OpenMesh::Vec3ui> triangles_;     // base mesh triangles
  VHierarchy                    vhierarchy_;

  unsigned int  n_base_vertices_;
  unsigned int  n_base_faces_;
  unsigned int  n_details_;
  unsigned char tree_id_bits_;  

public:

  ServerSideVDPM()  { clear(); }
  void clear();

  const char* name() const { return name_; }

  void get_name(char _name[256])            { strcpy(_name, name_); }
  void set_name(const char _name[256])      { strcpy(name_, _name); }


  std::string
  basename(const std::string& _f)
  {
    std::string::size_type dot = _f.rfind("/");
    if (dot == std::string::npos)
      return std::string(_f);
    return std::string(_f.substr(dot+1, _f.length()-(dot+1)));
  }


  bool is_same_name(const char _name[256])  
  {     
    return (basename( name_ ) == basename( _name ));
  }

  OpenMesh::VertexHandle add_vertex(const OpenMesh::Vec3f &p);
  OpenMesh::FaceHandle add_face(const unsigned int _triangle[3]);

  const OpenMesh::Vec3f& point(OpenMesh::VertexHandle _vertex_handle) const
  {
    return  points_[_vertex_handle.idx()];
  }

  OpenMesh::Vec3f& point(OpenMesh::VertexHandle _vertex_handle)
  {
    return  points_[_vertex_handle.idx()];
  }

  const VHierarchy& vhierarchy() const      { return  vhierarchy_; }
  VHierarchy& vhierarchy()                  { return  vhierarchy_; }

  void vhierarchy_roots(VHierarchyNodeHandleContainer &roots) const;
  
  unsigned int n_base_vertices() const      { return  n_base_vertices_; }
  unsigned int n_base_faces() const         { return  n_base_faces_; }
  unsigned int n_details() const            { return  n_details_; }  

  bool open_vd_prog_mesh(const char *_filename);
};

std::ostream& operator << ( std::ostream& _os, const ServerSideVDPM& _o );


typedef std::vector<ServerSideVDPM>   ServerSideVDPMContainer;
typedef std::list<ServerSideVDPM>     ServerSideVDPMList;

#endif //OPENMESH_APP_SERVERSIDEVDPM_HH defined

