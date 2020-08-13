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

#include <iostream>

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
          
#include <OpenMesh/Tools/Dualizer/meshDualT.hh>

typedef OpenMesh::PolyMesh_ArrayKernelT<>  MyMesh;


int main(int argc, char **argv)
{
    MyMesh mesh;

    if ( argc != 2 ) {
        std::cerr << "Please specify input filename only!" << std::endl;
        return 1;
    }

    // read mesh from argv[1]
    if ( !OpenMesh::IO::read_mesh(mesh, argv[1]) )
    {
        std::cerr << "Cannot read mesh from file" << argv[1] << std::endl;
        return 1;
    }

    MyMesh *dual = OpenMesh::Util::MeshDual(mesh);

    // write mesh to output.obj
    if ( !OpenMesh::IO::write_mesh(*dual, "output.obj") )
    {
        std::cerr << "Cannot write mesh to file 'output.obj'" << std::endl;
        return 1;
    }
    
    delete dual;

    return 0;
}
