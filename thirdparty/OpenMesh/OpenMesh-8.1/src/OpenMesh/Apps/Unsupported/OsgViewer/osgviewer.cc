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



// -------------------- STL
#include <memory>
#include <iostream>
#include <iomanip>
#include <stdexcept>
// -------------------- OpenSG
#include <GL/glut.h>
#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGGLUTWindow.h>
#include <OpenSG/OSGSimpleSceneManager.h>
#include <OpenSG/OSGSceneFileHandler.h>
#include <OpenSG/OSGTriangleIterator.h>
#include <OpenSG/OSGFaceIterator.h>
// -------------------- OpenMesh
#include <OpenMesh/Core/IO/MeshIO.hh> // always before kernel type

#include <OpenMesh/Tools/Kernel_OSG/TriMesh_OSGArrayKernelT.hh>
#include <OpenMesh/Tools/Kernel_OSG/bindT.hh>

#include <OpenMesh/Tools/Utils/Timer.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/LoopT.hh>

// --------------------
#include <OpenMesh/Tools/Smoother/JacobiLaplaceSmootherT.hh>


// ----------------------------------------------------------- namespace -----

OSG_USING_NAMESPACE


// ------------------------------------------------------------- OpenMesh ----

struct MeshTraits : public OpenMesh::Kernel_OSG::Traits
{
  HalfedgeAttributes( OpenMesh::Attributes::PrevHalfedge);
  VertexAttributes  ( OpenMesh::Attributes::Normal      | 
                      OpenMesh::Attributes::Color       );
  FaceAttributes    ( OpenMesh::Attributes::Normal      );
};

typedef OpenMesh::Kernel_OSG::TriMesh_OSGArrayKernelT<MeshTraits>   mesh_t;
typedef OpenMesh::Subdivider::Uniform::LoopT< mesh_t >              loop_t;
typedef OpenMesh::Smoother::JacobiLaplaceSmootherT< mesh_t >        smoother_t;

// --------------------------------------------------------------- globals ----

class MeshContainer
{
public:

  typedef std::vector<mesh_t*> meshbag_t;

  const size_t InvalidIndex;

public:

  MeshContainer() : InvalidIndex(size_t(-1))
  { }

  ~MeshContainer()
  {
    meshbag_t::iterator it = meshes_.begin();
    for(;it != meshes_.end(); ++it)
      delete *it;    
  }

  size_t size() const { return meshes_.size(); }

  mesh_t& operator [] ( size_t idx )
  { 
    if (idx < meshes_.size())
      return *(meshes_[idx]);
    throw std::range_error("Invalid index");
  }

  const mesh_t& operator [] ( size_t idx ) const
  { 
    if (idx < meshes_.size())
      return *(meshes_[idx]);
    throw std::range_error("Invalid index");
  }

  bool bind( osg::GeometryPtr geo )
  {
    #if (defined(_MSC_VER) && (_MSC_VER >= 1800)) || __cplusplus > 199711L || defined(__GXX_EXPERIMENTAL_CXX0X__)
      std::unique_ptr<mesh_t> obj(new mesh_t);
    #else
      std::auto_ptr<mesh_t> obj(new mesh_t);
    #endif


    return (OpenMesh::Kernel_OSG::bind< mesh_t >( *obj, geo))
      ? (meshes_.push_back(obj.release()), true)
      : false;
  }



private:

  meshbag_t meshes_;

private: // non-copyable

  MeshContainer( const MeshContainer& );
  MeshContainer& operator = ( const MeshContainer& );
};


struct Globals
{  
  // OpenSG specific entities
  SimpleSceneManager* mgr;
  GLUTWindowPtr       gwin;
  NodePtr             root;
  std::vector<GeometryPtr> geos;
  size_t              sel;
  bool                statistics;

  // OpenMesh specific entities
  MeshContainer       meshes;

} g;


// -------------------------------------------------------------- forwards ----

void display(void);
void reshape(int width, int height);
void mouse(int button, int state, int x, int y);
void keyboard(unsigned char key, int x, int y);
void motion(int x, int y);


// ---------------------------------------------------------------- helper ----

// setup the GLUT library which handles the windows for us
int setupGLUT(int *argc, char *argv[])
{
    glutInit(argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    
    int winid = glutCreateWindow("OpenMesh within OpenSG");
    
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard);
    
    return winid;
}


// -------------------------------------------------- locate geometry node ----
/*
    This function uses the fact that ::dcast() acts like dynamic_cast. It tries
    to dcast the core to a GeometryPtr, and tests the result to see if it
    actually was derived from Geometry.
*/

Action::ResultE bindGeo(NodePtr& node)
{   
  GeometryPtr geo = GeometryPtr::dcast(node->getCore());
    
  if (geo!=NullFC)
  {
    if ( g.meshes.bind( geo ) )
    {
      std::cout << "  Geometry connected to OpenMesh object\n";
      g.geos.push_back(geo);
      g.sel = g.meshes.size()-1;
      assert( g.geos.size() == g.meshes.size() );
    }
    else
        std::cerr << "  Warning! Could not bind the OpenMesh"
                  << " object to the geometry!\n";
  }
  return Action::Continue; 
}

// ------------------------------------------------------------------ main ----


int main(int argc, char **argv)
{
  // OSG init
  osgInit(argc,argv);

  int winid = setupGLUT(&argc, argv);

  // the connection between GLUT and OpenSG
  g.gwin = GLUTWindow::create();
  g.gwin->setId(winid);
  g.gwin->init();

  // -------------------- create root node with core
  std::cout << "Create root node with core\n";

  g.root           = Node::create();
  NodeCorePtr core = Group::create();
  
  osg::beginEditCP(g.root);
  {
    g.root->setCore(core);
  }
  osg::endEditCP(g.root);

  // -------------------- load the scene
  std::cout << "Load a scene from '" << argv[1] << "'\n";

  NodePtr     node = SceneFileHandler::the().read( argv[1] );

  if ( node != NullFC )
  {
    osg::beginEditCP(g.root);
    {
      g.root->addChild(node);
    }
    osg::endEditCP(g.root);
  }
  else
    return 1;

  // -------------------- bind all geometry nodes to an OpenMesh
  std::cout << "Bind all geometry nodes\n";

  traverse(g.root, 
	   osgTypedFunctionFunctor1CPtrRef<Action::ResultE,NodePtr>(bindGeo));

  if (!g.meshes.size())
  {
    std::cerr << "  No geometry found. Nothing to do!\n";
    return 1;
  }
  else
    std::cout << "  Number of bound geometry: " << g.meshes.size() << std::endl;

  // -------------------- create the SimpleSceneManager helper
  std::cout << "Create simple scene manager\n";

  g.mgr = new SimpleSceneManager;
  
  // tell the manager what to manage
  g.mgr->setWindow(g.gwin);
  g.mgr->setRoot  (g.root);

  // 
  g.mgr->useOpenSGLogo();
  g.mgr->setStatistics(false);
  
  // -------------------- show the whole scene
  std::cout << "Display everything\n";

  g.mgr->showAll();

  glutMainLoop();
  return 0;
}

// --------------------------------------------------------------- display ----

void display(void)
{
  g.mgr->redraw();
}

// --------------------------------------------------------------- reshape ----

void reshape(int w, int h)
{
  g.mgr->resize(w, h);
  glutPostRedisplay();
}

// ----------------------------------------------------------------- mouse ----

void mouse(int button, int state, int x, int y)
{
  if (state)
  {
    g.mgr->mouseButtonRelease(button, x, y);
//     if ( g.mode & FLYMODE )
//       glutIdleFunc(nullptr); 
  }
  else
  {
    g.mgr->mouseButtonPress(button, x, y);
//     if ( g.mode & FLYMODE )
//       glutIdleFunc(idle); 
  }        
  glutPostRedisplay();
}

// -------------------------------------------------------------- keyboard ----

void keyboard(unsigned char key, int x, int y)
{
#define MESH g.meshes[g.sel]
#define GEO g.geos[g.sel]
#define YN(b) (b?"yes":"no")
  OpenMesh::Utils::Timer t;

  using namespace std;

  switch(key)
  {
    case 27: // escape
      exit(0);
      break;

    case 'i':
    {
      cout << "OpenMesh information for obj #" << g.sel << ":\n";
      cout << "  #Vertices: " << MESH.n_vertices() << endl;
      cout << "     #Faces: " << MESH.n_faces() << endl;
      cout << "     #Edges: " << MESH.n_edges() << endl;
      cout << "  v. normal: " << YN(MESH.has_vertex_normals()) << endl;
      cout << "   v. color: " << YN(MESH.has_vertex_colors()) << endl;
      cout << "v. texcoord: " << YN(MESH.has_vertex_texcoords()) << endl;
      cout << "  f. normal: " << YN(MESH.has_face_normals()) << endl;
      cout << "   f. color: " << YN(MESH.has_face_colors()) << endl;
      break;
    }
    case 'I':
      cout << "Geometry information for obj #" << g.sel << ":\n";
      cout << "    #Types: " << GEO->getTypes()->size() << endl;
      cout << "  #Lengths: " << GEO->getLengths()->size() << endl;
      cout << "  #Indices: " << GEO->getIndices()->size() << endl;
      cout << "#Positions: " << GEO->getPositions()->size() << endl;
      break;

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      if ((size_t(key)-size_t('0')) < g.meshes.size())
      {
        g.sel = (size_t(key)-size_t('0'));
        cout << "Selected geometry #" << g.sel << endl;
      }
      break;

    case '+':
      g.sel = ++g.sel % g.meshes.size(); 
      cout << "Selected geometry #" << g.sel << endl;
      break;
      
    case '-':
      g.sel = (g.sel + g.meshes.size()-1) % g.meshes.size();
      cout << "Selected geometry #" << g.sel << endl;
      break;

    case 'S':
      g.mgr->setStatistics( g.statistics=!g.statistics );
      g.statistics 
	? cout << "Statistics enabled.\n"
	: cout << "Statistics disabled.\n";
      glutPostRedisplay();
      break;

    case 'w':
    {
      OpenMesh::IO::Options opt;
      
      if (MESH.has_vertex_colors())
        opt += OpenMesh::IO::Options::VertexColor;
      if (MESH.has_face_colors())
        opt += OpenMesh::IO::Options::FaceColor;
      if (MESH.has_vertex_normals())
        opt += OpenMesh::IO::Options::VertexNormal;

      std::string ofname;
      {
        std::stringstream ostr;
        ostr << "object-" << g.sel << ".om";
        ofname = ostr.str();
      }

      cout << "Writing OpenMesh of geometry #" << g.sel 
           << " to " << ofname << std::endl;
      t.start();
      bool rc = OpenMesh::IO::write_mesh( MESH, ofname, opt);
      t.stop();
      rc 
        ? cout << "  Done (" << t.as_string() << ")\n"
        : cout << "  Failed to store OpenMesh\n";

      break;
    }

    case 's':
    {
      cout << "Appyling two smoothing steps on selected geometry..";
      t.start();
      smoother_t smoother( g.meshes[g.sel] );
      smoother.initialize( smoother_t::Tangential, smoother_t::C1 );
      beginEditCP(g.geos[g.sel]);
      smoother.smooth(2);
      endEditCP(g.geos[g.sel]);
      t.stop();
      cout << "done. " << t.as_string() << endl;
      glutPostRedisplay();
      break;
    }

    case 't':
    {
      cout << "Applying two smoothing steps on all bound geometry..";
      t.start();
      for(size_t i = 0; i < g.meshes.size(); ++i)
      {
        smoother_t smoother( g.meshes[i] );
        smoother.initialize( smoother_t::Tangential, smoother_t::C1 );
        beginEditCP(g.geos[i]);
        smoother.smooth(2);
        endEditCP(g.geos[i]);
      }
      t.stop();
      cout << "done. " << t.as_string() << endl;
      glutPostRedisplay();
      break;
    }

    case 'c':
    {
      OpenMesh::IO::Options opt;
      
      if (MESH.has_vertex_colors())
        opt += OpenMesh::IO::Options::VertexColor;
      if (MESH.has_face_colors())
        opt += OpenMesh::IO::Options::FaceColor;
      if (MESH.has_vertex_normals())
        opt += OpenMesh::IO::Options::VertexNormal;

      mesh_t m(MESH);

      std::string ofname;
      {
        std::stringstream ostr;
        ostr << "copy-" << g.sel << ".om";
        ofname = ostr.str();
      }

      cout << "Writing copy of geometry #" << g.sel 
           << " to " << ofname << std::endl;
      t.start();
      bool rc = OpenMesh::IO::write_mesh( MESH, ofname, opt);
      t.stop();
      rc 
        ? cout << "  Done (" << t.as_string() << ")\n"
        : cout << "  Failed to store OpenMesh\n";

      OpenMesh::IO::write_mesh( m, ofname );

      break;
    }
    
    case 'u':
    {
      cout << "Applying one step of loop subdivision..";
      t.start();
      loop_t loop;
      beginEditCP( GEO );
      loop( MESH, 1 );
      MESH.update_normals();
      endEditCP( GEO );
      t.stop();
      cout << "done. " << t.as_string() << endl;
      glutPostRedisplay();
      break;
    }

    default:
      cout << "key [0x" << setw(4) << hex << key << dec << "]\n";
  }
#undef YN
#undef MESH
}

//----------------------------------------------------------------- motion ----

void motion(int x, int y)
{
  g.mgr->mouseMove(x, y);
  glutPostRedisplay();
}
