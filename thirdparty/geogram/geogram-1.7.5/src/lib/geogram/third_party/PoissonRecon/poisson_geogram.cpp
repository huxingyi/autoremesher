/*
 *  Copyright (c) 2012-2014, Bruno Levy
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  * Neither the name of the ALICE Project-Team nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     Bruno.Levy@inria.fr
 *     http://www.loria.fr/~levy
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 */

#include "poisson_geogram.h"
#include <geogram/bibliography/bibliography.h>

// Poisson Reconstruction code triggers many warnings,
// for now we ignore them...

#ifdef __GNUC__
#ifndef __ICC
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
#endif

#ifdef __ICC
#pragma warning disable 869 177 300 2415
#endif

#include "MyTime.h"
#include "MarchingCubes.h"
#include "Octree.h"
#include "SparseMatrix.h"
#include "PPolynomial.h"
#include "MemoryUsage.h"

#ifdef _OPENMP
#include <omp.h>
#endif

#include <geogram/basic/logger.h>

// TODO: redirect to geogram Logger.
void DumpOutput( const char* format , ... ) {
}

void DumpOutput2( std::vector< char* >& comments , const char* format , ... ) {
}

#include "MultiGridOctreeData.h"

#include <geogram/mesh/mesh.h>

namespace {
    using namespace GEO;

    /**
     * \brief Reads a pointset stored in a Geogram mesh in a way that
     *  is understood by Kahzdan's PoissonReconstruction code.
     */
    class Mesh_OrientedPointStream : public OrientedPointStream<double> {
    public:
        
        Mesh_OrientedPointStream(Mesh& M) : mesh_(M), cur_(0) {
            normal_.bind_if_is_defined(mesh_.vertices.attributes(), "normal");
            geo_assert(normal_.is_bound());
        }
        
        virtual void reset() {
            cur_ = 0;
        }

        virtual bool nextPoint( OrientedPoint3D<double>& out) {
            if(cur_ >= mesh_.vertices.nb()) {
                return false;
            }

            out.p = Point3D<double>(
                mesh_.vertices.point_ptr(cur_)[0],
                mesh_.vertices.point_ptr(cur_)[1],
                mesh_.vertices.point_ptr(cur_)[2]                
            );

            out.n = Point3D<double>(
                normal_[3*cur_],
                normal_[3*cur_+1],
                normal_[3*cur_+2]
            );
            
            ++cur_;
            return true;
        }
        
    private:
        Mesh& mesh_;
        Attribute<double> normal_;
        index_t cur_;
    };
    
}


namespace GEO {

    PoissonReconstruction::PoissonReconstruction() {
        performance_ = false;
        complete_ = false;
        no_comments_ = false;
        polygon_mesh_ = false;
        confidence_ = false;
        normal_weights_ = false;
        non_manifold_ = false;
        dirichlet_ = false;
        ascii_ = false;
        density_ = false;
        linear_fit_ = false;
        primal_voxel_ = false;
        verbose_ = false;
        
        degree_ = 2;
        depth_ = 8;
        cg_depth_ = 0;
        kernel_depth_ = 0;
        adaptive_exponent_ = 1;
        iters_ = 8;
        voxel_depth_ = -1;
        full_depth_ = 5;
        min_depth_ = 0;
        max_solve_depth_ = 0;
        threads_ = int(Process::number_of_cores());

        color_ = 16.f;
        samples_per_node_ = 1.5f;
        scale_ = 1.1f;
        cg_accuracy_ = 1e-3f;
        point_weight_ = 4.0f;

        keep_voxel_ = false;
        voxel_res_ = 0;
        voxel_values_ = nullptr;

	geo_cite("DBLP:journals/tog/KazhdanH13");
    }

    PoissonReconstruction::~PoissonReconstruction() {
        delete[] voxel_values_;
        voxel_values_ = nullptr;
        voxel_res_ = 0;
    }
    
    void PoissonReconstruction::reconstruct(Mesh* points, Mesh* surface) {
        delete[] voxel_values_;
        voxel_values_ = nullptr;
        
        Reset<double>();

        XForm4x4<double> xForm , iXForm;
        xForm = XForm4x4<double>::Identity();
        iXForm = xForm.inverse();        

        double isoValue = 0.0;
        Octree<double> tree;
        tree.threads = threads_;

        if(max_solve_depth_ == 0) {
            max_solve_depth_ = depth_;
        }

        OctNode< TreeNodeData >::SetAllocator( MEMORY_ALLOCATOR_BLOCK_SIZE );
        int kernelDepth = (kernel_depth_ != 0) ? kernel_depth_ : depth_-2;
        if( kernelDepth>depth_) {
            // TODO: warning
            kernelDepth = depth_;
        }

        tree.maxMemoryUsage=0;
        
        SparseNodeData< PointData< double > , 0 >* pointInfo =
            new SparseNodeData< PointData< double > , 0 >();
        
        SparseNodeData< Point3D< double > , NORMAL_DEGREE >* normalInfo =
            new SparseNodeData< Point3D< double > , NORMAL_DEGREE >();
        
        SparseNodeData< double , WEIGHT_DEGREE >* densityWeights =
            new SparseNodeData< double , WEIGHT_DEGREE >();
        
        SparseNodeData< double , NORMAL_DEGREE >* nodeWeights =
            new SparseNodeData< double , NORMAL_DEGREE >();
        
        typedef Octree< double >::ProjectiveData< Point3D< double > >
            ProjectiveColor;
        
        SparseNodeData< ProjectiveColor , DATA_DEGREE >* colorData = NULL;

        // TODO: colors if there are some...

        // Feeding my data into Misha Kahzdhan's code.
        {
            Mesh_OrientedPointStream pointStream(*points);
            tree.SetTree<
                double , NORMAL_DEGREE , WEIGHT_DEGREE , DATA_DEGREE ,
                Point3D<unsigned char>, Point3D<double>
            >(
                &pointStream , min_depth_ , depth_ , full_depth_ ,
                kernel_depth_ ,
                double(samples_per_node_),
                scale_, confidence_, normal_weights_,
                point_weight_, adaptive_exponent_,
                *densityWeights , *pointInfo , *normalInfo , *nodeWeights ,
                colorData , xForm , dirichlet_, complete_
            );
        }

        // Get bounding box
        {
            box_origin_ =
                vec3(tree.center()[0], tree.center()[1], tree.center()[2]);
            box_edge_length_ = tree.scale();
        }
        
        
        if( !density_ ) {
            delete densityWeights;
            densityWeights = NULL;
        }

        const int Degree = 2;
        typedef PlyVertex<double> Vertex;
        {
            std::vector< int > indexMap;
            if( NORMAL_DEGREE>Degree )
                tree.EnableMultigrid< NORMAL_DEGREE >( &indexMap );
            else
                tree.EnableMultigrid< Degree >( &indexMap );
            if( pointInfo ) pointInfo->remapIndices( indexMap );
            if( normalInfo ) normalInfo->remapIndices( indexMap );
            if( densityWeights ) densityWeights->remapIndices( indexMap );
            if( nodeWeights ) nodeWeights->remapIndices( indexMap );
            if( colorData ) colorData->remapIndices( indexMap );
        }

        DenseNodeData< double , Degree > constraints =
            tree.SetLaplacianConstraints< Degree >( *normalInfo );
        delete normalInfo;

        DenseNodeData< double , Degree > solution =
            tree.SolveSystem(
                *pointInfo , constraints , false , iters_ ,
                max_solve_depth_, cg_depth_, cg_accuracy_
            );
        delete pointInfo;
        constraints.resize( 0 );

        CoredFileMeshData< Vertex > mesh;
        isoValue = tree.GetIsoValue(solution, *nodeWeights);        
        delete nodeWeights;

        if(keep_voxel_) {
            int res;
            Pointer(double) values = tree.Evaluate(
                solution, res, isoValue, voxel_depth_, primal_voxel_
            );
            voxel_res_ = index_t(res);
            voxel_values_ = new float[res*res*res];
            for(index_t i=0; i<voxel_res_*voxel_res_*voxel_res_; ++i) {
                voxel_values_[i] = float(values[i]);
            }
            DeletePointer(values);
        }

        tree.GetMCIsoSurface< Degree , WEIGHT_DEGREE , DATA_DEGREE >(
            densityWeights , colorData , solution , isoValue ,
            mesh , !linear_fit_, !non_manifold_, polygon_mesh_
        );

        solution.resize(0);
        if( colorData ) {
            delete colorData ;
            colorData = NULL;
        }
        
        // Copy mesh to result
        surface->clear();
        surface->vertices.set_dimension(3);

        size_t nb_vertices =
            mesh.outOfCorePointCount() + mesh.inCorePoints.size();
        surface->vertices.create_vertices(index_t(nb_vertices));
        
        for(int i=0; i<int(mesh.inCorePoints.size()); ++i) {
            Vertex vertex =  mesh.inCorePoints[i];
            surface->vertices.point_ptr(i)[0] = vertex.point[0];
            surface->vertices.point_ptr(i)[1] = vertex.point[1];
            surface->vertices.point_ptr(i)[2] = vertex.point[2];
        }

        mesh.resetIterator();
        
        int offset = int(mesh.inCorePoints.size());
        for(int i=0; i<mesh.outOfCorePointCount(); ++i) {
            Vertex vertex;
            mesh.nextOutOfCorePoint(vertex);
            surface->vertices.point_ptr(i+offset)[0] = vertex.point[0];
            surface->vertices.point_ptr(i+offset)[1] = vertex.point[1];
            surface->vertices.point_ptr(i+offset)[2] = vertex.point[2];
        }

        mesh.resetIterator();

        int nb_faces = mesh.polygonCount();
        std::vector<CoredVertexIndex> face;
        for(index_t f=0; f<index_t(nb_faces); ++f) {
            mesh.nextPolygon(face);
            index_t nbv = index_t(face.size());
            surface->facets.create_polygon(nbv);
            for(index_t lv=0; lv<nbv; ++lv) {
                index_t v = index_t(face[lv].idx);
                surface->facets.set_vertex(f, lv, v);
            }
        }
        
        surface->facets.connect();
    }
    

}
