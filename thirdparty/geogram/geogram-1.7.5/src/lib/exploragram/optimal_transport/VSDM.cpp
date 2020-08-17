
/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2009 INRIA - Project ALICE
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy - levy@loria.fr
 *
 *     Project ALICE
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs. 
 *
 * As an exception to the GPL, Graphite can be linked with the following 
 *  (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */

#include <exploragram/optimal_transport/VSDM.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_subdivision.h>
#include <geogram/basic/progress.h>
#include <geogram/bibliography/bibliography.h>

namespace {
    using namespace GEO;

    /**
     * \brief A callback class for mesh subdivision that interpolates
     *  attributes and constructs the subdivision matrix.
     */
    class SymbolicMeshSplitCallbacks : public MeshSplitCallbacks {
    public:

	/**
	 * \brief SymbolicMeshSplitCallbacks constructor.
	 * \param[in] mesh a pointer to the mesh.
	 * \param[in] matrix a pointer to the matrix. Should contain
	 *  the identity matrix with nv*dim columns, where nv denotes the
	 *  number of vertices in the mesh and dim the dimension of the
	 *  vertices (mostly 3 in the present case).
	 */
	SymbolicMeshSplitCallbacks(Mesh* mesh, NLSparseMatrix* matrix) :
	    MeshSplitCallbacks(mesh), matrix_(matrix) {
	}

	/**
	 * \copydoc MeshSplitCallbacks::create_vertex()
	 */
	index_t create_vertex() override {
	    FOR(i,mesh_->vertices.dimension()) {
		nlSparseMatrixAddRow(matrix_);
	    }
	    return MeshSplitCallbacks::create_vertex();
	}

	/**
	 * \copydoc MeshSplitCallbacks::scale_vertex()
	 */
	void scale_vertex(index_t v, double s) override {
	    index_t dim = mesh_->vertices.dimension();
	    FOR(i,dim) {
		nlSparseMatrixScaleRow(matrix_, v*dim+i, s);
	    }
	    MeshSplitCallbacks::scale_vertex(v,s);
	}

	/**
	 * \copydoc MeshSplitCallbacks::zero_vertex()
	 */
	void zero_vertex(index_t v) override {
	    index_t dim = mesh_->vertices.dimension();
	    FOR(i,dim) {
		nlSparseMatrixZeroRow(matrix_, v*dim+i);
	    }
	    MeshSplitCallbacks::zero_vertex(v);
	}

	/**
	 * \copydoc MeshSplitCallbacks::madd_vertex()
	 */
	void madd_vertex(
	    index_t v1, double s, index_t v2
	) override {
	    index_t dim = mesh_->vertices.dimension();
	    FOR(i,dim) {
		nlSparseMatrixMAddRow(matrix_, v1*dim+i, s, v2*dim+i);
	    }
	    MeshSplitCallbacks::madd_vertex(v1,s,v2);
	}
	
    private:
	NLSparseMatrix* matrix_;
    };
}

namespace GEO {

    VSDM* VSDM::instance_ = nullptr;
    
    VSDM::VSDM(Mesh* S, Mesh* T):
	S_(S),
	T_(T),
	affinity_(1.0),
	progress_(nullptr),
        nb_iter_(0),
        cur_iter_(0),
        subd_(nullptr) {
	compute_graph_Laplacian(S_,&L_);
	temp_V1_.resize(S_->vertices.nb());
	temp_V2_.resize(S_->vertices.nb());
	delaunay_ = Delaunay::create(3);
	RVD_ = RestrictedVoronoiDiagram::create(delaunay_, T_);
	optimizer_ = Optimizer::create("HLBFGS");
	subd_matrix_ = nullptr;
	geo_cite("DBLP:conf/imr/NivoliersYL11");
	geo_cite("DBLP:journals/cgf/LuLW12");
        geo_cite("DBLP:journals/ewc/NivoliersYL14");
    }

    VSDM::~VSDM() {
	nlSparseMatrixDestroy(&L_);
	nlDeleteMatrix(subd_matrix_);
    }
    
    void VSDM::optimize(index_t nb_iter) {
	if(nb_iter == 0) {
	    return;
	}
	
	if(progress_ != nullptr) {
	    progress_->reset(nb_iter);
	}

	double S = Geom::mesh_area(*T_);
	double R = ::pow(S, 1.0 / 2.0);
	double CVT_normalization = 1.0 / pow(R, 4.0);
	double affinity_normalization = 0.001 / pow(R, 2.0);
	affinity_scaling_ =
	    affinity_ * affinity_normalization / CVT_normalization;

	nb_iter_ = nb_iter;
	cur_iter_ = 0;
        instance_ = this;
	index_t n = S_->vertices.nb() * 3;
	index_t m = 7;
	double* x = S_->vertices.point_ptr(0);
	optimizer_->set_epsg(0.0);
	optimizer_->set_epsf(0.0);
	optimizer_->set_epsx(0.0);
	optimizer_->set_newiteration_callback(VSDM::newiteration_CB);
	optimizer_->set_funcgrad_callback(VSDM::funcgrad_CB);
	optimizer_->set_N(n);
	optimizer_->set_M(m);
	optimizer_->set_max_iter(nb_iter);
	optimizer_->optimize(x);
        instance_ = nullptr;
    }

    void VSDM::funcgrad(index_t n, double* x, double& f, double* g) {
	f = 0.0;
        Memory::clear(g, n * sizeof(double));
	if(subd_ == nullptr) {
	    delaunay_->set_vertices(n/3, x);	
	    RVD_->compute_CVT_func_grad(f, g);
	} else {
	    subd_g_.assign(subd_->vertices.nb()*3, 0.0);
	    nlMultMatrixVector(subd_matrix_, x, subd_->vertices.point_ptr(0));
	    delaunay_->set_vertices(
		subd_->vertices.nb(), subd_->vertices.point_ptr(0)
	    );
	    RVD_->compute_CVT_func_grad(f, subd_g_.data());
	    
	    // g = transpose(subd_matrix_) * subd_g_
	    {
		NLCRSMatrix* CRS = (NLCRSMatrix*)(subd_matrix_);
		FOR(i,CRS->m) {
		    for(index_t jj=CRS->rowptr[i]; jj<CRS->rowptr[i+1]; ++jj) {
			index_t j = CRS->colind[jj];
			double a = CRS->val[jj];
			g[j] += a * subd_g_[i];
		    }
		}
	    }
	}
	if(affinity_ != 0.0) {
	    add_funcgrad_affinity(n,x,f,g);
	}
    }

    void VSDM::add_funcgrad_affinity(
	index_t n, double* x, double& f, double* g
    ) {
	geo_assert(L_.n*3 == n);
	FOR(coord,3) {
	    FOR(i,L_.n) {
		temp_V1_[i] = x[3*i+coord];
	    }
	    nlSparseMatrixMult(&L_, temp_V1_.data(), temp_V2_.data());
	    double F = 0.0;
	    FOR(i,L_.n) {
		F += temp_V1_[i] * temp_V2_[i];
	    }
	    f += affinity_scaling_ * F;
	    FOR(i,L_.n) {
		g[3*i + coord] += 2.0 * affinity_scaling_ * temp_V2_[i];
	    }
	}
    }
    
    void VSDM::newiteration() {
	cur_iter_++;
	if(cur_iter_ <= nb_iter_) {
	    Logger::out("VSDM") 
		<< "Iter: " << cur_iter_ << "/" << nb_iter_ << std::endl; 
	}
        if(progress_ != nullptr) {
            progress_->next();
        }
    }
    
    void VSDM::funcgrad_CB(index_t n, double* x, double& f, double* g) {
	geo_assert(instance_ != nullptr);
        instance_->funcgrad(n, x, f, g);
    }
    
    void VSDM::newiteration_CB(
	index_t n, const double* x, double f, const double* g, double gnorm
    ) {
	geo_argused(n);
	geo_argused(x);
	geo_argused(f);
	geo_argused(g);
	geo_argused(gnorm);
	geo_assert(instance_ != nullptr);
        instance_->newiteration();
    }

    void VSDM::compute_graph_Laplacian(Mesh* S, NLSparseMatrix* L) {
	index_t n = S->vertices.nb();
	nlSparseMatrixConstruct(L, n, n, NL_MATRIX_STORE_ROWS);
	vector<index_t> v_degree(S->vertices.nb(),0);
	FOR(c,S->facet_corners.nb()) {
	    ++v_degree[S->facet_corners.vertex(c)];
	}
	FOR(f,S->facets.nb()) {
	    for(
		index_t c1 = S->facets.corners_begin(f);
		c1 < S->facets.corners_end(f); ++c1) {
		index_t c2 = S->facets.next_corner_around_facet(f,c1);
		index_t v1 = S->facet_corners.vertex(c1);
		index_t v2 = S->facet_corners.vertex(c2);
		double a = 2.0 / (double(v_degree[v1]) + double(v_degree[v2]));
		nlSparseMatrixAdd(L, v1, v2, -a);
		nlSparseMatrixAdd(L, v1, v1,  a);
	    }
	}
    }

    void VSDM::set_subdivision_surface(Mesh* mesh, index_t nb_subdiv) {
	subd_ = mesh;
	index_t n = S_->vertices.nb()*3;
	subd_matrix_ = nlSparseMatrixNew(n, n, NL_MATRIX_STORE_ROWS);
	FOR(i,n) {
	    nlSparseMatrixAdd((NLSparseMatrix*)subd_matrix_, i, i, 1.0);
	}
	subd_->clear();
	subd_->copy(*S_);
	SymbolicMeshSplitCallbacks cb(subd_, (NLSparseMatrix*)subd_matrix_);
	FOR(i,nb_subdiv) {
	    mesh_split_catmull_clark(*subd_, &cb);
	}
	nlMatrixCompress(&subd_matrix_);
	subd_g_.assign(subd_->vertices.nb()*3, 0);
    }
}

