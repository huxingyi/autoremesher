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
 * As an exception to the GPL, Graphite can be linked with 
 *     the following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */

#include <exploragram/optimal_transport/optimal_transport_3d.h>
#include <exploragram/optimal_transport/linear_least_squares.h>

#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/mesh_reorder.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_AABB.h>
#include <geogram/voronoi/CVT.h>
#include <geogram/voronoi/generic_RVD_vertex.h>
#include <geogram/voronoi/RVD_callback.h>
#include <geogram/voronoi/generic_RVD_cell.h>
#include <geogram/delaunay/delaunay_3d.h>
#include <geogram/points/nn_search.h>
#include <geogram/basic/stopwatch.h>
#include <geogram/basic/progress.h>

#include <stack>
#include <iterator>

namespace {
    using namespace GEO;

    // Implementation notes on OTMPolyhedronCallback:
    //   [Bruno Levy, Sept. 13th, 2017]
    //
    //    OTMPolyhedronCallback computes the value, gradient
    // and Hessian of the objective function of semi-discrete
    // optimal transport.
    //
    //    Its operator() is called for each pair of:
    // (Laguerre cell of vertex v, background mesh tetrahedron t)
    // that have a non-empty intersection. The intersection is
    // represented as a GEOGen::ConvexCell object, that stores all
    // the geometric and combinatorial information associated with
    // the intersection.
    //
    //    The objective function, gradient and Hessian depend on integrals
    //  computed on the Laguerre cells (objective function and gradient)
    //  and Laguerre cells boundaries (Hessian). The integrals are
    //  integrals of linear functions over the
    //  (Laguerre cell /\ background tetrahedron) intersections, evaluated in
    //  closed form (integral of linear function over a polytope). Volumetric
    //  integrals over the cells are evaluated by decomposing the cell into
    //  tetrahedra. Surfacic integrals over the cells boundary are evaluated
    //  by decomposing the cells facets into triangles.
    //
    //     Each (Laguerre cell /\ background tetrahedron) is represented
    //  by a GEOGen::ConvexCell object, represented in dual form, that is its
    //  vertices correspond to facets, and its
    //  triangles to vertices. This is because computing the intersection
    //  between halfspaces can be done very efficiently with this form.
    //    The code looks more complicated than it should be, due to the dual
    // representation of GEOGen::ConvexCell. Two nested loops iterate on the
    // facets (i.e., the dual vertices) and around each facet (i.e. the loop
    // of dual triangles incident to each dual vertex). It's a bit painful,
    // but remember, computing the intersections in this form is much much
    // faster.
    //
    //    The value of the objective function is computed by
    // eval_F() and eval_F_weighted() (if varying density is attached
    // to the background mesh as a vertex attribute named "weight").
    // The objective function is the most painful to compute, but it
    // is not needed by the Newton solver (only the hierarchical BFGS
    // solver needs it). 
    //
    //    The gradient of the objective function corrresponds to the
    // difference between the (possibly weighted) mass (or volume) of
    // each Laguerre cell and the prescribed masses. The mass (and
    // optionally centers of mass) is computed by compute_m_and_mg().
    // The callback operator() only computes the mass of each
    // Laguerre cell. The prescribed mass is subtracted by the OptimalTransport
    // class.
    // The center of mass is used by some applications (for instance
    // the Euler fluid simulator). The center of mass is weighted by the
    // mass so that the contribution of different intersections can be
    // summed (it is divided in the end by the total mass in the
    // OptimalTransport class).
    //
    //    The Hessian of the objective function has a non-zero coefficient
    // hij each time the Laguerre cells of vertex i and vertex j have a
    // non-empty intersection (a common facet). The value of the coefficient
    // is the mass (area or weighted area) of the face divided by twice the
    // distance between vertex i and vertex j. They
    // are computed by update_Hessian(), that calls OpenNL functions to
    // construct the (sparse) Hessian. The right-hand side of the linear system
    // (minus the gradient) is computed by the OptimalTransport class (that
    // copies it from the gradient).
    //
    //    Important note/gotcha: Beware the signs of everything:
    //  F = \sum_i \nu_i \psi_i -
    //       \sum_i \int_{Lag_i} \|x - y_i\|^2 - \psi_i u(x) dx
    // \frac{\partial F}{\partial \psi_i} = \nu_i - \int_{Lag_i} u(x)dx
    //   There is a minus sign in the mass of the Laguerre cells in the gradient
    //   component.
    // ... but remember, we maximize F, and numerical code prefers to minimize
    // things, so finally we minimize -F, and there is no minus sign.
    // ... but again, when doing Newton, we solve \nabla^2 F p = -nabla F, so
    // finally, there is a minus sign for the components of the gradient when
    // assembling the RHS for the Newton solves.
   
    /**
     * \brief Computes the contribution of a polyhedron
     *  to the objective function minimized by a semi-discrete
     *  optimal transport map.
     * \details Works in uniform and weighted mode. Works in
     *  Newton and BFGS mode.
     */
    class OTMPolyhedronCallback :
	public OptimalTransportMap::Callback,
	public RVDPolyhedronCallback {

    public:
	/**
	 * \brief OTMPolyhedronCallback constructor.
	 * \param[in] OTM a pointer to the OptimalTransportMap3d
	 */
	OTMPolyhedronCallback(OptimalTransportMap3d* OTM) :
	    OptimalTransportMap::Callback(OTM) {
	}

	/**
	 * \copydoc RVDPolyhedronCallback::operator()
	 */
	void operator() (
	    index_t v,
	    index_t t,
	    const GEOGen::ConvexCell& C
	) const override {
	    // v can be an air particle.
	    if(v >= n_) {
		return;
	    }
	    
	    geo_argused(t);

	    double m, mgx, mgy, mgz;
	    compute_m_and_mg(C, m, mgx, mgy, mgz);

	    if(spinlocks_ != nullptr) {
		spinlocks_->acquire_spinlock(v);
	    }

	    // +m because we maximize F <=> minimize -F
	    g_[v] += m;

	    if(Newton_step_) {
		// ... but here -m because Newton step =
		//  solve H p = -g    (minus g in the RHS).
		OTM_->add_i_right_hand_side(v,-m);
	    }
	    
	    if(mg_ != nullptr) {
		mg_[3*v] += mgx;  
		mg_[3*v+1] += mgy;
		mg_[3*v+2] += mgz;
	    }
	    
	    if(spinlocks_ != nullptr) {
		spinlocks_->release_spinlock(v);
	    }

	    if(Newton_step_) {
		// Spinlocks are managed internally by update_Hessian().
		update_Hessian(C, v);
	    }

	    if(eval_F_) {
		Thread* thread = Thread::current();
		index_t current_thread_id = (thread == nullptr) ? 0 : thread->id();
		double F = weighted_ ? eval_F_weighted(C, v) : eval_F(C, v);
		const_cast<OTMPolyhedronCallback*>(this)->
		    funcval_[current_thread_id] += F;
	    }
	}

    protected:

	/**
	 * \brief Computes the mass and mass times centroid of the
	 *  current ConvexCell.
	 * \details Weights are taken into account if present.
	 * \param[in] C a const reference to the current ConvexCell
	 * \param[out] m , mgx , mgy , mgz the mass and the mass times the
	 *  centroid of the ConvexCell. mgx, mgy and mgz are not computed
	 *  if mg_ is nullptr.
	 */
	void compute_m_and_mg(
	    const GEOGen::ConvexCell& C, 
	    double& m, double& mgx, double& mgy, double& mgz
	) const {

	    m = 0.0;
	    mgx = 0.0;
	    mgy = 0.0;
	    mgz = 0.0;


	    // Find one vertex V0 of the Convex Cell. It will be then decomposed
	    // into tetrahedra radiating from V0.
	    const GEOGen::Vertex* V0 = nullptr;
	    for(index_t ct=0; ct < C.max_t(); ++ct) {
		if(C.triangle_is_used(ct)) {
		    V0 = &C.triangle_dual(ct);
		    break;
		}
	    }
	    if(V0 == nullptr) {
		return;
	    }
	    

	    // Iterate on all the vertices of the convex cell.
	    // The vertices associated to no triangle are skipped.
	    // Note: the convex cell is in dual form, thus a
	    // vertex of the ConvexCell corresponds to a polygonal
	    // facet.
	    for(index_t cv = 0; cv < C.max_v(); ++cv) {
		signed_index_t ct = C.vertex_triangle(cv);
		if(ct == -1) {
		    continue;
		}
		geo_debug_assert(C.triangle_is_used(index_t(ct)));


		// Iterate around the facet vertices (that correspond
		// to the current vertex in dual form). The polygonal
		// facet is triangulated. The triangles that contain
		// the vertex V0 are skipped (their volume is zero).
		GEOGen::ConvexCell::Corner first(
		    index_t(ct), C.find_triangle_vertex(index_t(ct), cv)
		);
		const GEOGen::Vertex* V1 = &C.triangle_dual(first.t);
		const GEOGen::Vertex* V2 = nullptr;
		const GEOGen::Vertex* V3 = nullptr;
		const double* p0 = V0->point();		
		GEOGen::ConvexCell::Corner c = first;
		do {
		    V2 = V3;
		    V3 = &C.triangle_dual(c.t);
		    if(
			V2 != nullptr && V3 != V1 &&
			V1 != V0 && V2 != V0 && V3 != V0
		    ) {
			const double* p1 = V1->point();
			const double* p2 = V2->point();
			const double* p3 = V3->point();
			double cur_m = GEO::Geom::tetra_volume<3>(p0,p1,p2,p3);
			if(weighted_) {
			    double w0 = V0->weight();
			    double w1 = V1->weight();
			    double w2 = V2->weight();
			    double w3 = V3->weight();
			    double S =  w0+w1+w2+w3;
			    if(mg_ != nullptr) {
				mgx += 0.25*cur_m*(
				    w0*p0[0]+w1*p1[0]+w2*p2[0]+w3*p3[0]
				);
				mgy += 0.25*cur_m*(
				    w0*p0[1]+w1*p1[1]+w2*p2[1]+w3*p3[1]
				);
				mgz += 0.25*cur_m*(
				    w0*p0[2]+w1*p1[2]+w2*p2[2]+w3*p3[2]
				);
			    }
			    cur_m *= (S/4.0);
			} else {
			    if(mg_ != nullptr) {
				mgx += 0.25*cur_m*(p0[0]+p1[0]+p2[0]+p3[0]);
				mgy += 0.25*cur_m*(p0[1]+p1[1]+p2[1]+p3[1]);
				mgz += 0.25*cur_m*(p0[2]+p1[2]+p2[2]+p3[2]);
			    }
			}
			m += cur_m;			
		    }
		    C.move_to_next_around_vertex(c);
		} while(c != first);
	    }

	}

	/**
	 * \brief Updates the Hessian according to the current ConvexCell.
	 * \param[in] C a const reference to the current ConvexCell
	 * \param[in] v the current seed
	 */
	void update_Hessian(
	    const GEOGen::ConvexCell& C, index_t v
	) const {
	    // The coefficient of the Hessian associated to a pair of
	    // adjacent cells Lag(i),Lag(j) is :
	    // - mass(Lag(i) /\ Lag(j)) / (2*distance(pi,pj))
	    
	    const double* p0 = OTM_->point_ptr(v);

	    // Iterate on all the vertices of the convex cell.
	    // The vertices associated to no triangle are skipped.
	    // Note: the convex cell is in dual form, thus a
	    // vertex of the ConvexCell corresponds to a polygonal
	    // facet.
	    
	    for(index_t cv = 0; cv < C.max_v(); ++cv) {
		signed_index_t ct = C.vertex_triangle(cv);
		if(ct == -1) {
		    continue;
		}
		geo_debug_assert(C.triangle_is_used(index_t(ct)));

		// Get index of adjacent seed if any.
		index_t v_adj = index_t(-1);
		signed_index_t adjacent = C.vertex_id(cv);
		if(adjacent > 0) {
		    // Positive adjacent indices correspond to
		    // Voronoi seed - Voronoi seed link
		    v_adj = index_t(adjacent - 1);
		}
		if(v_adj == index_t(-1)) {
		    continue;
		}
		
		double hij = 0;

		// Iterate around the facet vertices (that correspond
		// to the current vertex in dual form). The polygonal
		// facet is triangulated. 
		
		GEOGen::ConvexCell::Corner first(
		    index_t(ct), C.find_triangle_vertex(index_t(ct), cv)
		);
		const GEOGen::Vertex* V1 = &C.triangle_dual(first.t);
		const GEOGen::Vertex* V2 = nullptr;
		const GEOGen::Vertex* V3 = nullptr;		
		GEOGen::ConvexCell::Corner c = first;
		do {
		    V2 = V3;
		    V3 = &C.triangle_dual(c.t);
		    if(V2 != nullptr && V3 != V1) {
			double cur_m = GEO::Geom::triangle_area_3d(
			    V1->point(),V2->point(),V3->point()
			);
			if(weighted_) {
			    cur_m *= (
				V1->weight()+V2->weight()+V3->weight()
			    )/3.0;
			}
			hij += cur_m;
		    }
		    C.move_to_next_around_vertex(c);
		} while(c != first);

		const double* p1 = OTM_->point_ptr(v_adj);		
		hij /= (2.0 * GEO::Geom::distance(p0,p1,3));		

		if(spinlocks_ != nullptr) {
		    spinlocks_->acquire_spinlock(v);
		}
		
		// Diagonal is positive, extra-diagonal
		// coefficients are negative,
		// this is a convex function.

		if(v_adj < n_) {
		    OTM_->add_ij_coefficient(
			v, v_adj, -hij
		    );
		}
		OTM_->add_ij_coefficient(
		    v, v, hij
                );

		if(spinlocks_ != nullptr) {
		    spinlocks_->release_spinlock(v);
		}
	    }
	}

	/**
	 * \brief Computes the contribution of the current ConvexCell 
	 *  to the objective function, in the uniform (non-weighted)
	 *  case.
	 * \param[in] C a const reference to the current ConvexCell
	 * \param[in] v the current seed
	 */
	double eval_F(const GEOGen::ConvexCell& C, index_t v) const {

	    // Note: we compute F as a sum of integrals over (signed)
	    // tetrahedra; formed by vertex v and triplets of cell
	    // vertices. The contribution of F is simpler to compute
	    // if v is a vertex of the tetrahedron. Since we use
	    // signed tetrahedra, portions of tetrahedra that are
	    // outside the cell cancel-out ("a-la" Stokes theorem).
	    
	    geo_debug_assert(!weighted_);
	    
	    double F = 0.0;
	    
	    // Iterate on all the vertices of the convex cell.
	    // The vertices associated to no triangle are skipped.
	    // Note: the convex cell is in dual form, thus a
	    // vertex of the ConvexCell corresponds to a polygonal
	    // facet.

	    for(index_t cv = 0; cv < C.max_v(); ++cv) {
		signed_index_t ct = C.vertex_triangle(cv);
		if(ct == -1) {
		    continue;
		}
		geo_debug_assert(C.triangle_is_used(index_t(ct)));

		
		// Iterate around the facet vertices (that correspond
		// to the current vertex in dual form). The polygonal
		// facet is triangulated.
		
		GEOGen::ConvexCell::Corner first(
		    index_t(ct), C.find_triangle_vertex(index_t(ct), cv)
		);
		const GEOGen::Vertex* V1 = &C.triangle_dual(first.t);
		const GEOGen::Vertex* V2 = nullptr;
		const GEOGen::Vertex* V3 = nullptr;		
		GEOGen::ConvexCell::Corner c = first;
		do {
		    V2 = V3;
		    V3 = &C.triangle_dual(c.t);
		    if(V2 != nullptr && V3 != V1) {
			const double* p0 = OTM_->point_ptr(v);
			const double* p1 = V1->point();
			const double* p2 = V2->point();
			const double* p3 = V3->point();
			double m = Geom::tetra_signed_volume(p0, p1, p2, p3);
			double fT = 0.0;
			for(coord_index_t cc = 0; cc < 3; ++cc) {
			    double Uc = p1[cc] - p0[cc];
			    double Vc = p2[cc] - p0[cc];
			    double Wc = p3[cc] - p0[cc];
			    fT +=
				Uc * Uc +
				Vc * Vc +
				Wc * Wc +
				Uc * Vc +
				Vc * Wc +
				Wc * Uc;
			}
			fT = m * (fT / 10.0 - w_[v]);
			F += fT;
		    }
		    C.move_to_next_around_vertex(c);
		} while(c != first);
	    }
            // -F because we maximize F <=> minimize -F	    
	    return -F;
	}


	/**
	 * \brief Computes the contribution of the current ConvexCell 
	 *  to the objective function, in the weighted case.
	 * \param[in] C a const reference to the current ConvexCell
	 * \param[in] v the current seed
	 */
	double eval_F_weighted(const GEOGen::ConvexCell& C, index_t v) const {
	    double F = 0.0;

	    
	    // Find one vertex V0 of the Convex Cell. It will be then decomposed
	    // into tetrahedra radiating from V0.
	    const GEOGen::Vertex* V0 = nullptr;
	    for(index_t ct=0; ct < C.max_t(); ++ct) {
		if(C.triangle_is_used(ct)) {
		    V0 = &C.triangle_dual(ct);
		    break;
		}
	    }
	    if(V0 == nullptr) {
		return F;
	    }
	    
	    // Iterate on all the vertices of the convex cell.
	    // The vertices associated to no triangle are skipped.
	    // Note: the convex cell is in dual form, thus a
	    // vertex of the ConvexCell corresponds to a polygonal
	    // facet.
	    for(index_t cv = 0; cv < C.max_v(); ++cv) {
		signed_index_t ct = C.vertex_triangle(cv);
		if(ct == -1) {
		    continue;
		}
		geo_debug_assert(C.triangle_is_used(index_t(ct)));


		// Iterate around the facet vertices (that correspond
		// to the current vertex in dual form). The polygonal
		// facet is triangulated. The triangles that contain
		// the vertex V0 are skipped (their volume is zero).
		GEOGen::ConvexCell::Corner first(
		    index_t(ct), C.find_triangle_vertex(index_t(ct), cv)
		);
		const GEOGen::Vertex* V1 = &C.triangle_dual(first.t);
		const GEOGen::Vertex* V2 = nullptr;
		const GEOGen::Vertex* V3 = nullptr;
		GEOGen::ConvexCell::Corner c = first;
		do {
		    V2 = V3;
		    V3 = &C.triangle_dual(c.t);
		    if(
			V2 != nullptr && V3 != V1 &&
			V1 != V0 && V2 != V0 && V3 != V0
		    ) {

			const double* p0 = V0->point();
			const double* p1 = V1->point();
			const double* p2 = V2->point();
			const double* p3 = V3->point();

			double p0_mass = V0->weight();
			double p1_mass = V1->weight();
			double p2_mass = V2->weight();
			double p3_mass = V3->weight();
            
			const double* q = OTM_->point_ptr(v);

			double Tvol = GEO::Geom::tetra_volume<3>(p0,p1,p2,p3);
			double Sp = p0_mass + p1_mass + p2_mass + p3_mass;
            
			double m = (Tvol * Sp) / 4.0;
			double rho[4], alpha[4];
            
			rho[0] = p0_mass;
			rho[1] = p1_mass;
			rho[2] = p2_mass;
			rho[3] = p3_mass;
            
			alpha[0] = Sp + rho[0];
			alpha[1] = Sp + rho[1];
			alpha[2] = Sp + rho[2];
			alpha[3] = Sp + rho[3];
            
			double dotprod_00 = 0.0;
			double dotprod_10 = 0.0;
			double dotprod_11 = 0.0;
			double dotprod_20 = 0.0;
			double dotprod_21 = 0.0;
			double dotprod_22 = 0.0;
			double dotprod_30 = 0.0;
			double dotprod_31 = 0.0;
			double dotprod_32 = 0.0;
			double dotprod_33 = 0.0;
            
			for(coord_index_t cc = 0; cc < 3; ++cc) {
			    double sp0 = q[cc] - p0[cc];
			    double sp1 = q[cc] - p1[cc];
			    double sp2 = q[cc] - p2[cc];
			    double sp3 = q[cc] - p3[cc];                
			    dotprod_00 += sp0 * sp0;
			    dotprod_10 += sp1 * sp0;
			    dotprod_11 += sp1 * sp1;
			    dotprod_20 += sp2 * sp0;
			    dotprod_21 += sp2 * sp1;
			    dotprod_22 += sp2 * sp2;
			    dotprod_30 += sp3 * sp0;
			    dotprod_31 += sp3 * sp1;
			    dotprod_32 += sp3 * sp2;
			    dotprod_33 += sp3 * sp3;                
			}
            
			double fT = 0.0;
			fT += (alpha[0] + rho[0]) * dotprod_00;  
			fT += (alpha[1] + rho[0]) * dotprod_10;  
			fT += (alpha[1] + rho[1]) * dotprod_11;  
			fT += (alpha[2] + rho[0]) * dotprod_20;  
			fT += (alpha[2] + rho[1]) * dotprod_21;  
			fT += (alpha[2] + rho[2]) * dotprod_22;  
			fT += (alpha[3] + rho[0]) * dotprod_30;  
			fT += (alpha[3] + rho[1]) * dotprod_31;  
			fT += (alpha[3] + rho[2]) * dotprod_32;  
			fT += (alpha[3] + rho[3]) * dotprod_33;  

			fT = Tvol * fT / 60.0 - m * w_[v];
	    
			F += fT;
		    }
		    C.move_to_next_around_vertex(c);
		} while(c != first);
	    }
            // -F because we maximize F <=> minimize -F	    
	    return -F;
	}
	
    };

/**********************************************************************/

    void cell_shrink_to_animation(Mesh& mesh) {
        Attribute<index_t> cell_region;
        Attribute<double> W(mesh.vertices.attributes(),"w");
        cell_region.bind_if_is_defined(
            mesh.cells.attributes(), "region"
        );
        if(!cell_region.is_bound()) {
            Logger::err("OTM") << "region: no such cell attribute"
                               << std::endl;
            return;
        }
        index_t nb_RVD_cells = 0;
        for(index_t c: mesh.cells) {
            nb_RVD_cells = std::max(nb_RVD_cells, cell_region[c]);
        }
        ++nb_RVD_cells;
        vector<vec3> center(nb_RVD_cells, vec3(0.0, 0.0, 0.0));
        vector<index_t> nb(nb_RVD_cells, 0);
        for(index_t c: mesh.cells) {
            index_t rvc = cell_region[c];
            for(index_t lv=0; lv<4; ++lv) {
                index_t v = mesh.cells.vertex(c,lv);
                center[rvc] += Geom::mesh_vertex(mesh,v);
                ++nb[rvc];
            }
        }
        for(index_t rvc=0; rvc<nb_RVD_cells; ++rvc) {
            if(nb[rvc] != 0) {
                center[rvc] /= double(nb[rvc]);
            }
        }
        for(index_t v: mesh.vertices) {
            W[v] = mesh.vertices.point_ptr(v)[3];
        }
        mesh.vertices.set_dimension(6);
        for(index_t c: mesh.cells) {
            index_t rvc = cell_region[c];
            for(index_t lv=0; lv<4; ++lv) {
                index_t v = mesh.cells.vertex(c,lv);
                const vec3& g = center[rvc];
                double* p = mesh.vertices.point_ptr(v);
                p[3] = g[0];
                p[4] = g[1];
                p[5] = g[2];
            }
        }
    }

/**********************************************************************/

   /**
    * \brief Gets the Delaunay implementation that best fits 
    *  user desire.
    * \param[in] user_delaunay the desired implementation.
    * \return the available implementation nearest to user desire.
    */
   std::string default_delaunay(const std::string& user_delaunay)  {
      std::string result = "PDEL";
      if(user_delaunay != "default") {
	 result = user_delaunay;
      }
#ifndef GEOGRAM_WITH_PDEL
      if(result == "PDEL") {
	 result = "BPOW";
      }
#endif
      return result;
   }
   
   
}




namespace GEO {

    OptimalTransportMap3d::OptimalTransportMap3d(
        Mesh* mesh, const std::string& delaunay, bool BRIO
    ) : OptimalTransportMap(
	3,
	mesh,
        default_delaunay(delaunay),			    
	BRIO
    ) {
	callback_ = new OTMPolyhedronCallback(this);
	total_mass_ = total_mesh_mass();	
    }

    double OptimalTransportMap3d::total_mesh_mass() const {
	double result = 0.0;
        //   This is terribly confusing, the parameters for
        // a power diagram are called "weights", and the
        // standard attribute name for vertices density is
        // also called "weight" (and is unrelated).
        //   In this program, what is called weight corresponds
        // to the parameters of the power diagram (except the
        // name of the attribute), and everything that corresponds
        // to mass/density is called mass.
        Attribute<double> vertex_mass;
        vertex_mass.bind_if_is_defined(
            mesh_->vertices.attributes(), "weight"
        );
        
        for(index_t t: mesh_->cells) {
            double tet_mass = GEO::Geom::tetra_volume<3>(
                mesh_->vertices.point_ptr(mesh_->cells.tet_vertex(t, 0)),
                mesh_->vertices.point_ptr(mesh_->cells.tet_vertex(t, 1)),
                mesh_->vertices.point_ptr(mesh_->cells.tet_vertex(t, 2)),
                mesh_->vertices.point_ptr(mesh_->cells.tet_vertex(t, 3))
            );
            if(vertex_mass.is_bound()) {
                tet_mass *= (
                    vertex_mass[mesh_->cells.tet_vertex(t, 0)] +
                    vertex_mass[mesh_->cells.tet_vertex(t, 1)] +
                    vertex_mass[mesh_->cells.tet_vertex(t, 2)] +
                    vertex_mass[mesh_->cells.tet_vertex(t, 3)]
                ) / 4.0;
            }
            result += tet_mass;
        }
	return result;
    }
    
    OptimalTransportMap3d::~OptimalTransportMap3d() {
    }
    
    void OptimalTransportMap3d::get_RVD(Mesh& RVD_mesh) {
        RVD_mesh.clear();
        Attribute<index_t> tet_region(RVD_mesh.cells.attributes(),"region");
        RVD()->compute_RVD(
            RVD_mesh,
            0,             // dim (0 means use default)
            false,         // borders_only
            show_RVD_seed_ // integration_simplices
        );
        if(!show_RVD_seed_) {
            cell_shrink_to_animation(RVD_mesh);
        }
    }

    void OptimalTransportMap3d::compute_Laguerre_centroids(double* centroids) {
        vector<double> g(nb_points(), 0.0);
        Memory::clear(centroids, nb_points()*sizeof(double)*3);

	callback_->set_Laguerre_centroids(centroids);
	callback_->set_g(g.data());
	{
	    Stopwatch* W = nullptr;
	    if(newton_) {
		W = new Stopwatch("RVD");
		Logger::out("OTM") << "In RVD (centroids)..." << std::endl;
	    }
	    RVD_->for_each_polyhedron(
		*dynamic_cast<RVDPolyhedronCallback*>(callback_),
		false,false,true
	    );
	    if(newton_) {
		delete W;
	    }
	}
	callback_->set_Laguerre_centroids(nullptr);	    
	
        for(index_t v=0; v<nb_points(); ++v) {
            centroids[3*v  ] /= g[v];
            centroids[3*v+1] /= g[v];
            centroids[3*v+2] /= g[v];            
        }
    }

    void OptimalTransportMap3d::call_callback_on_RVD() {
	RVD_->for_each_polyhedron(
	    *dynamic_cast<RVDPolyhedronCallback*>(callback_),
	    false, // symbolic
	    false, // connected components priority
	    !clip_by_balls_ // parallel
	);
	// clip_by_balls deactivates parallel mode, because it needs to access
	// the PointAllocator of the current thread, and we do not have any
	// access (for now).
    }

    /**********************************************************************/
    
    void compute_Laguerre_centroids_3d(
        Mesh* omega,
        index_t nb_points,
        const double* points,
        double* centroids,
	RVDPolyhedronCallback* cb,
	bool verbose,
	index_t nb_iter
    ) {
        omega->vertices.set_dimension(4);

        // false = no BRIO
        // (OTM does not use multilevel and lets Delaunay
        //  reorder the vertices)
        OptimalTransportMap3d OTM(
	    omega,
	    "PDEL",
	    false
	);

        OTM.set_regularization(1e-3);
        OTM.set_Newton(true);
        OTM.set_points(nb_points, points);
        OTM.set_epsilon(0.01);
	OTM.set_Laguerre_centroids(centroids);
	OTM.set_verbose(verbose);
        OTM.optimize(nb_iter);

	if(cb != nullptr) {
	    OTM.RVD()->for_each_polyhedron(*cb,false,false,false);
	}
	
        omega->vertices.set_dimension(3);        
    }
}

/****************************************************************************/

namespace {
    using namespace GEO;


    /**
     * \brief Gets the number of connected components
     *  of each tetrahedra regions in a mesh.
     * \param[in] RVD a const reference to the mesh
     * \param[out] nb_cnx_comps on exit, nb_cnx_comps[r]
     *  contains the number of connected components of
     *  region r.
     */
    void get_nb_connected_components(
        const Mesh& RVD, vector<index_t>& nb_cnx_comps
    ) {
        Attribute<index_t> tet_region(RVD.cells.attributes(),"region");
        vector<bool> marked(RVD.cells.nb(), false);
        std::stack<index_t> S;
        for(index_t t = 0; t < RVD.cells.nb(); ++t) {
            if(!marked[t]) {
                index_t cur_v = index_t(tet_region[t]);
                marked[t] = true;
                S.push(t);
                while(!S.empty()) {
                    index_t cur_t = S.top();
                    S.pop();
                    for(index_t lf = 0; lf < 4; ++lf) {
                        index_t neigh = RVD.cells.tet_adjacent(cur_t, lf);
                        if(neigh != NO_CELL) {
                            if(
                                tet_region[neigh] == cur_v &&
                                !marked[neigh]
                            ) {
                                marked[neigh] = true;
                                S.push(neigh);
                            }
                        }
                    }
                }
                if(cur_v >= nb_cnx_comps.size()) {
                    nb_cnx_comps.resize(cur_v,0);
                }
                ++nb_cnx_comps[cur_v];
            }
        }
    }


    /**
     * \brief Computes the original and final vertices of
     *  the optimal transport.
     * \param[in] CVT the Centroidal Voronoi Tesselation that
     *  samples the target mesh M2
     * \param[in] OTM the Optimal Transport Map that back-projects
     *  the samples of the target mesh M2 onto the source mesh M1
     * \param[out] M1_vertices the source vertices, computed from
     *  the centroids of the power cells restricted to M1
     * \param[out] M2_vertices the destination vertices, copied
     *  from the sampling of M2
     * \param[out] nb_cnx_comps gives for each vertex the number of
     *  connected components of the power cell restricted to M1
     */
    void get_vertices(
        CentroidalVoronoiTesselation& CVT,
        OptimalTransportMap3d& OTM,
        vector<vec3>& M1_vertices,
        vector<vec3>& M2_vertices,
        vector<index_t>& nb_cnx_comps
    ) {
        index_t nb_vertices = OTM.RVD()->delaunay()->nb_vertices();
        M1_vertices.resize(nb_vertices);
        M2_vertices.resize(nb_vertices);
        nb_cnx_comps.assign(nb_vertices, 0);
        {
            for(index_t v = 0; v < nb_vertices; ++v) {
                const double* p = CVT.RVD()->delaunay()->vertex_ptr(v);
                M2_vertices[v] = vec3(p[0], p[1], p[2]);
            }
        }

        {
            vector<vec3> mg(nb_vertices, vec3(0.0, 0.0, 0.0));
            vector<double> m(nb_vertices, 0.0);

            Mesh RVD;
            Attribute<index_t> tet_region(RVD.cells.attributes(),"region");
            OTM.RVD()->compute_RVD(
                RVD,
                0,     // dim (0 means use default)
                false, // cells_borders_only
                true   // integration_simplices
            );
            RVD.vertices.set_dimension(3);
            RVD.cells.connect();

	    /*
            if(CmdLine::get_arg_bool("RVD")) {
                MeshIOFlags flags;
                flags.set_element(MESH_CELLS);
                flags.set_attribute(MESH_CELL_REGION);
                mesh_save(RVD, "RVD.meshb", flags);
            }
	    */

            for(index_t t = 0; t < RVD.cells.nb(); ++t) {
                index_t v =  tet_region[t];
                index_t v0 = RVD.cells.tet_vertex(t, 0);
                index_t v1 = RVD.cells.tet_vertex(t, 1);
                index_t v2 = RVD.cells.tet_vertex(t, 2);
                index_t v3 = RVD.cells.tet_vertex(t, 3);
                vec3 p0(RVD.vertices.point_ptr(v0));
                vec3 p1(RVD.vertices.point_ptr(v1));
                vec3 p2(RVD.vertices.point_ptr(v2));
                vec3 p3(RVD.vertices.point_ptr(v3));
                double mt = GEO::Geom::tetra_signed_volume(p0, p1, p2, p3);
                mg[v] += (mt / 4.0) * (p0 + p1 + p2 + p3);
                m[v] += mt;
            }
            for(index_t v = 0; v < nb_vertices; ++v) {
                double s = ::fabs(m[v]);
                if(s != 0.0) {
                    s = 1.0 / s;
                }
                M1_vertices[v] = s * mg[v];
            }
            get_nb_connected_components(RVD, nb_cnx_comps);
        }
    }

    /**
     * \brief Tests whether a tetrahedron is included inside
     *  a given tetrahedral mesh.
     * \details Implemented by sampling the tetrahedron and
     *  testing whether each sample is inside the mesh.
     * \param[in] AABB a const reference to a MeshCellsAABB
     * \param[in] p1 a const reference to the first vertex 
     *  of the tetrahedron
     * \param[in] p2 a const reference to the second vertex 
     *  of the tetrahedron
     * \param[in] p3 a const reference to the third vertex 
     *  of the tetrahedron
     * \param[in] p4 a const reference to the fourth vertex 
     *  of the tetrahedron
     */
    bool mesh_contains_tet(
        const MeshCellsAABB& AABB,
        const vec3& p1,
        const vec3& p2,
        const vec3& p3,
        const vec3& p4
    ) {
        const index_t NB = 5;
        for(index_t u1=0; u1<=NB; ++u1) {
            double s1 = double(u1)/double(NB);            
            for(index_t u2=0; u1+u2<=NB; ++u2) {
                double s2 = double(u2)/double(NB);                
                for(index_t u3=0; u1+u2+u3<=NB; ++u3) {
                    double s3 = double(u3)/double(NB);                    
                    index_t u4=NB-u1-u2-u3;
                    double s4 = double(u4)/double(NB);

                    // Skip the four vertices of the tetrahedron
                    if(
                        (u1 == NB) || (u2 == NB) ||
                        (u3 == NB) || (u4 == NB)
                    ) {
                        continue;
                    }
                    
                    vec3 g = s1*p1+s2*p2+s3*p3+s4*p4;
                    if(AABB.containing_tet(g) == MeshCellsAABB::NO_TET) {
                        return false;
                    }
                }
            }
        }
        return true;
    }
}

/****************************************************************************/

namespace GEO {
    
    void compute_morph(
        CentroidalVoronoiTesselation& CVT,
        OptimalTransportMap3d& OTM,
        Mesh& morph,
        bool filter_tets
    ) {
        geo_assert(CVT.volumetric());
        
        Logger::out("OTM")
            << "Computing coherent tet mesh"
            << std::endl;

        morph.clear();
        morph.vertices.set_dimension(6);
        
        // Step 1: Compute the candidate tets from the Delaunay triangulation
        // of the samples restricted to M2.
        vector<index_t> morph_tets;
        morph_tets.clear();
        {
            vector<double> embedding;
            CVT.RVD()->compute_RDT(
                morph_tets,
                embedding,
                RestrictedVoronoiDiagram::RDT_SEEDS_ALWAYS
            );
        }

        // Step 2: Compute the original vertices location (centroids
        //  of power cells restricted to M1) and get the final vertices
        //  locations (sampling of M2).
        vector<vec3> M1_vertices;
        vector<vec3> M2_vertices;
        vector<index_t> nb_cnx_comps;
        get_vertices(CVT, OTM, M1_vertices, M2_vertices, nb_cnx_comps);
        index_t nb_vertices = M1_vertices.size();

        //  Gather all the points coordinates in a vector of
        // 6d points.
        vector<double> morph_vertices(nb_vertices*6);
        for(index_t v=0; v<nb_vertices; ++v) {
            morph_vertices[v*6  ] = M2_vertices[v].x;
            morph_vertices[v*6+1] = M2_vertices[v].y;
            morph_vertices[v*6+2] = M2_vertices[v].z;
            morph_vertices[v*6+3] = M1_vertices[v].x;
            morph_vertices[v*6+4] = M1_vertices[v].y;
            morph_vertices[v*6+5] = M1_vertices[v].z;            
        }

        // Step 3: Filter-out the tets incident to a vertex
        // that splits during transport.
        index_t nb_tets = morph_tets.size()/4;
        vector<bool> tet_to_remove(nb_tets, false);
        for(index_t t=0; t<nb_tets; ++t) {
            for(index_t lv=0; lv<4; ++lv) {
                index_t v = morph_tets[4*t+lv];
                if(nb_cnx_comps[v] > 1) {
                    tet_to_remove[t] = true;
                    break;
                }
            }
        }

        // Step 4: Filter-out the tets that are not contained by
        // the initial mesh M1.
        if(filter_tets) {
            MeshCellsAABB AABB(*OTM.RVD()->mesh());
            try {
                ProgressTask progress("Classifying", 100);        
                for(index_t t=0; t<nb_tets; ++t) {
                    progress.progress(t * 100 / nb_tets);
                    if(!tet_to_remove[t]) {
                        vec3 p[4];
                        for(index_t lv=0; lv<4; ++lv) {
                            for(coord_index_t c=0; c<3; ++c) {
                                index_t v = morph_tets[4*t+lv];
                                p[lv][c] = morph_vertices[v*6+3+c];
                            }
                        }
                        if(!mesh_contains_tet(AABB, p[0], p[1], p[2], p[3])) {
                            tet_to_remove[t] = true;
                        }
                    }
                }
            } catch(...) {
            }
        }

        // Step 5: create the output mesh.
        vector<index_t> filtered_morph_tets;
        for(index_t t=0; t<nb_tets; ++t) {
            if(!tet_to_remove[t]) {
                filtered_morph_tets.push_back(morph_tets[4*t]);
                filtered_morph_tets.push_back(morph_tets[4*t+1]);
                filtered_morph_tets.push_back(morph_tets[4*t+2]);
                filtered_morph_tets.push_back(morph_tets[4*t+3]);
            }
        }
        
        morph.cells.assign_tet_mesh(
            6, morph_vertices, filtered_morph_tets, true
        );
        morph.cells.connect();
        morph.cells.compute_borders();
    }

    void compute_singular_surface(        
        CentroidalVoronoiTesselation& CVT,
        OptimalTransportMap3d& OTM,
        Mesh& singular
    ) {

        std::set<bindex> edges;
        {
            vector<index_t> simplices;
            vector<double> embedding;
            CVT.RVD()->compute_RDT(
                simplices, 
                embedding, 
                RestrictedVoronoiDiagram::RDT_SEEDS_ALWAYS
            );
            for(index_t t=0; t*4<simplices.size(); ++t) {
                index_t v1 = simplices[t*4];
                index_t v2 = simplices[t*4+1];
                index_t v3 = simplices[t*4+2];
                index_t v4 = simplices[t*4+3];
                edges.insert(bindex(v1,v2));
                edges.insert(bindex(v1,v3));
                edges.insert(bindex(v1,v4));
                edges.insert(bindex(v2,v3));
                edges.insert(bindex(v2,v4));
                edges.insert(bindex(v3,v4));
            }
        }

        Mesh RVD;
        MeshIOFlags flags;
        flags.set_element(MESH_CELLS);
        flags.set_attribute(MESH_CELL_REGION);
        OTM.RVD()->compute_RVD(
            RVD,
            0,
            false, // cells_borders_only
            true   // integration_simplices
        );
        RVD.vertices.set_dimension(3);
        RVD.cells.connect();

        singular.clear();
        singular.vertices.set_dimension(3);
        
        vector<index_t> triangles;

        Attribute<index_t> tet_region(RVD.cells.attributes(),"region");
        
        for(index_t t=0; t<RVD.cells.nb(); ++t) {
            index_t v1 = tet_region[t];
            for(index_t f=0; f<4; ++f) {
                index_t nt = RVD.cells.tet_adjacent(t,f);
                if(nt != NO_CELL) {
                    index_t v2 = tet_region[nt];
                    if(v1 != v2 && edges.find(bindex(v1,v2)) == edges.end()) {
                        for(index_t i=0; i<3; ++i) {
                            index_t lv =
                                RVD.cells.local_tet_facet_vertex_index(f,i);
                            index_t v = RVD.cells.tet_vertex(t,lv);
                            triangles.push_back(v);
                        }
                    }
                }
            }
        }

        singular.vertices.assign_points(
            RVD.vertices.point_ptr(0),
            RVD.vertices.dimension(), RVD.vertices.nb()
        );
        singular.facets.assign_triangle_mesh(
            triangles, true 
        );
    }
}
