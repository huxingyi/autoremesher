/* ========================================================================= *
 *                                                                           *
 *                               OpenMesh                                    *
 *           Copyright (c) 2001-2015, RWTH-Aachen University                 *
 *           Department of Computer Graphics and Multimedia                  *
 *                          All rights reserved.                             *
 *                            www.openmesh.org                               *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * This file is part of OpenMesh.                                            *
 *---------------------------------------------------------------------------*
 *                                                                           *
 * Redistribution and use in source and binary forms, with or without        *
 * modification, are permitted provided that the following conditions        *
 * are met:                                                                  *
 *                                                                           *
 * 1. Redistributions of source code must retain the above copyright notice, *
 *    this list of conditions and the following disclaimer.                  *
 *                                                                           *
 * 2. Redistributions in binary form must reproduce the above copyright      *
 *    notice, this list of conditions and the following disclaimer in the    *
 *    documentation and/or other materials provided with the distribution.   *
 *                                                                           *
 * 3. Neither the name of the copyright holder nor the names of its          *
 *    contributors may be used to endorse or promote products derived from   *
 *    this software without specific prior written permission.               *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED *
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A           *
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,  *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,       *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR        *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              *
 *                                                                           *
 * ========================================================================= */




#ifndef OPENMESHAPPS_MESHVIEWERWIDGET_HH
#define OPENMESHAPPS_MESHVIEWERWIDGET_HH


//== INCLUDES =================================================================


// -------------------- OpenMesh
#include <OpenMesh/Apps/QtViewer/MeshViewerWidgetT.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/Composite/CompositeTraits.hh>

//

typedef OpenMesh::Subdivider::Uniform::CompositeTraits Traits;
typedef OpenMesh::TriMesh_ArrayKernelT<Traits>         Mesh;



//== CLASS DEFINITION =========================================================

	      

class MeshViewerWidgetSubdivider : public MeshViewerWidgetT<Mesh>
{
public:
   typedef MeshViewerWidgetT<Mesh> Base;
   
  /// default constructor
  explicit MeshViewerWidgetSubdivider(QWidget* _parent=0)
    : Base(_parent)
  {}

  /// destructor
  ~MeshViewerWidgetSubdivider() {}

  /// open mesh
  inline bool open_mesh(const char* _filename, OpenMesh::IO::Options _opt) override
  {
    if ( Base::open_mesh( _filename, _opt ) )
    {
      orig_mesh_ = mesh_;
      return true;
    }
    return false;
  }

  Mesh& orig_mesh() { return orig_mesh_; }
  const Mesh& orig_mesh() const { return orig_mesh_; }
   
protected:

  Mesh orig_mesh_;

};


//=============================================================================
#endif // OPENMESHAPPS_MESHVIEWERWIDGET_HH defined
//=============================================================================

