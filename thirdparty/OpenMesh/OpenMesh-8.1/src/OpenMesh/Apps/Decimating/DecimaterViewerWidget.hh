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




#ifndef OPENMESHAPPS_DECIMATERVIEWERWIDGET_HH
#define OPENMESHAPPS_DECIMATERVIEWERWIDGET_HH


//== INCLUDES =================================================================

#if !defined(OM_USE_OSG)
#  define OM_USE_OSG 0
#endif


#include <qtimer.h>
#include <string>
#include <memory>

//--------------------
#include <OpenMesh/Core/IO/MeshIO.hh>
//--------------------
#if OM_USE_OSG
#  include <OpenMesh/Tools/Kernel_OSG/TriMesh_OSGArrayKernelT.hh>
#  define DEFAULT_TRAITS Kernel_OSG::Traits
#  define TRIMESH_KERNEL Kernel_OSG::TriMesh_OSGArrayKernelT
#else
//--------------------
#  include <OpenMesh/Core/Mesh/Traits.hh>
#  include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#  define DEFAULT_TRAITS DefaultTraits
#  define TRIMESH_KERNEL TriMesh_ArrayKernelT
#endif

#include <OpenMesh/Apps/QtViewer/MeshViewerWidgetT.hh>

#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModNormalFlippingT.hh>
#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>


//== CLASS DEFINITION =========================================================


using namespace OpenMesh;

struct MyTraits : public DEFAULT_TRAITS
{
  VertexAttributes  ( Attributes::Normal       );
  FaceAttributes    ( Attributes::Normal       );  
};

typedef TRIMESH_KERNEL<MyTraits>                     mesh_t;
typedef MeshViewerWidgetT<mesh_t>                    MeshViewerWidgetDecimaterBase;

//== CLASS DEFINITION =========================================================


class DecimaterViewerWidget : public MeshViewerWidgetDecimaterBase
{
  Q_OBJECT
   
public:
  
  typedef MeshViewerWidgetDecimaterBase inherited_t;

  typedef Decimater::DecimaterT<mesh_t>                decimater_t;  
  typedef Decimater::ModQuadricT< mesh_t >::Handle        mod_quadric_t;
  typedef Decimater::ModNormalFlippingT< mesh_t >::Handle mod_nf_t;

  // object types
#if (defined(_MSC_VER) && (_MSC_VER >= 1800)) || __cplusplus > 199711L || defined(__GXX_EXPERIMENTAL_CXX0X__)
  typedef std::unique_ptr< decimater_t   >   decimater_o;
#else
  typedef std::auto_ptr< decimater_t   >   decimater_o;
#endif



  /// default constructor
  explicit DecimaterViewerWidget(QWidget* _parent=0)
    : MeshViewerWidgetDecimaterBase(_parent),
      animate_(false),
      timer_(0),
      steps_(1)
  {
    timer_ = new QTimer(this);
    
    connect( timer_, SIGNAL(timeout()), SLOT(animate()) );
  }
  
  DecimaterViewerWidget() :
          animate_(false),
          timer_(0),
          steps_(0)
  {
  }

  /// destructor
  ~DecimaterViewerWidget()
  {
    delete timer_;
  }

public: // inherited

  bool open_mesh(const char* _filename, OpenMesh::IO::Options _opt) override
  {
    bool rc;

    if ( (rc = inherited_t::open_mesh( _filename, _opt )) )
    {
      std::cout << "prepare decimater" << std::endl;

      decimater_ = decimater_o ( new decimater_t  ( mesh() ) );

      decimater_->add(mod_quadric_);
      decimater_->module(mod_quadric_).set_binary(false);

      decimater_->add(mod_nf_);

      decimater_->initialize();
    }
    return rc;
  }
  
protected slots:

  void animate( void );

protected:   

  virtual void keyPressEvent(QKeyEvent* _event) override;


private:
  
  bool              animate_;
  QTimer           *timer_;

  decimater_o       decimater_;  
  mod_quadric_t     mod_quadric_;
  mod_nf_t          mod_nf_;

  size_t            steps_;
};


//=============================================================================
#endif // OPENMESHAPPS_DECIMATERVIEWERWIDGET_HH defined
//=============================================================================

