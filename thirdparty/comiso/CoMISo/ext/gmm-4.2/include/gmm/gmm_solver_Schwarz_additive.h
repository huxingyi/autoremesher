/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================
 
 Copyright (C) 2002-2012 Yves Renard
 
 This file is a part of GETFEM++
 
 Getfem++  is  free software;  you  can  redistribute  it  and/or modify it
 under  the  terms  of the  GNU  Lesser General Public License as published
 by  the  Free Software Foundation;  either version 3 of the License,  or
 (at your option) any later version along with the GCC Runtime Library
 Exception either version 3.1 or (at your option) any later version.
 This program  is  distributed  in  the  hope  that it will be useful,  but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or  FITNESS  FOR  A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 License and GCC Runtime Library Exception for more details.
 You  should  have received a copy of the GNU Lesser General Public License
 along  with  this program;  if not, write to the Free Software Foundation,
 Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA.
 
 As a special exception, you  may use  this file  as it is a part of a free
 software  library  without  restriction.  Specifically,  if   other  files
 instantiate  templates  or  use macros or inline functions from this file,
 or  you compile this  file  and  link  it  with other files  to produce an
 executable, this file  does  not  by itself cause the resulting executable
 to be covered  by the GNU Lesser General Public License.  This   exception
 does not  however  invalidate  any  other  reasons why the executable file
 might be covered by the GNU Lesser General Public License.
 
===========================================================================*/

/**@file gmm_solver_Schwarz_additive.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @author  Michel Fournie <fournie@mip.ups-tlse.fr>
   @date October 13, 2002.
*/

#ifndef GMM_SOLVERS_SCHWARZ_ADDITIVE_H__
#define GMM_SOLVERS_SCHWARZ_ADDITIVE_H__ 

#include "gmm_kernel.h"
#include "gmm_superlu_interface.h"
#include "gmm_solver_cg.h"
#include "gmm_solver_gmres.h"
#include "gmm_solver_bicgstab.h"
#include "gmm_solver_qmr.h"

namespace gmm {
      
  /* ******************************************************************** */
  /*		Additive Schwarz interfaced local solvers                 */
  /* ******************************************************************** */

  struct using_cg {};
  struct using_gmres {};
  struct using_bicgstab {};
  struct using_qmr {};

  template <typename P, typename local_solver, typename Matrix>
  struct actual_precond {
    typedef P APrecond;
    static const APrecond &transform(const P &PP) { return PP; }
  };

  template <typename Matrix1, typename Precond, typename Vector> 
  void AS_local_solve(using_cg, const Matrix1 &A, Vector &x, const Vector &b,
		 const Precond &P, iteration &iter)
  { cg(A, x, b, P, iter); }

  template <typename Matrix1, typename Precond, typename Vector> 
  void AS_local_solve(using_gmres, const Matrix1 &A, Vector &x,
		      const Vector &b, const Precond &P, iteration &iter)
  { gmres(A, x, b, P, 100, iter); }
  
  template <typename Matrix1, typename Precond, typename Vector> 
  void AS_local_solve(using_bicgstab, const Matrix1 &A, Vector &x,
		      const Vector &b, const Precond &P, iteration &iter)
  { bicgstab(A, x, b, P, iter); }

  template <typename Matrix1, typename Precond, typename Vector> 
  void AS_local_solve(using_qmr, const Matrix1 &A, Vector &x,
		      const Vector &b, const Precond &P, iteration &iter)
  { qmr(A, x, b, P, iter); }

#if defined(GMM_USES_SUPERLU)
  struct using_superlu {};

  template <typename P, typename Matrix>
  struct actual_precond<P, using_superlu, Matrix> {
    typedef typename linalg_traits<Matrix>::value_type value_type;
    typedef SuperLU_factor<value_type> APrecond;
    template <typename PR>
    static APrecond transform(const PR &) { return APrecond(); }
    static const APrecond &transform(const APrecond &PP) { return PP; }
  };

  template <typename Matrix1, typename Precond, typename Vector> 
  void AS_local_solve(using_superlu, const Matrix1 &, Vector &x,
		      const Vector &b, const Precond &P, iteration &iter)
  { P.solve(x, b); iter.set_iteration(1); }
#endif

  /* ******************************************************************** */
  /*		Additive Schwarz Linear system                            */
  /* ******************************************************************** */

  template <typename Matrix1, typename Matrix2, typename Precond,
	    typename local_solver>
  struct add_schwarz_mat{
    typedef typename linalg_traits<Matrix1>::value_type value_type;

    const Matrix1 *A;
    const std::vector<Matrix2> *vB;
    std::vector<Matrix2> vAloc;
    mutable iteration iter;
    double residual;
    mutable size_type itebilan;
    mutable std::vector<std::vector<value_type> > gi, fi;
    std::vector<typename actual_precond<Precond, local_solver,
					Matrix1>::APrecond> precond1;

    void init(const Matrix1 &A_, const std::vector<Matrix2> &vB_,
	      iteration iter_, const Precond &P, double residual_);

    add_schwarz_mat(void) {}
    add_schwarz_mat(const Matrix1 &A_, const std::vector<Matrix2> &vB_,
		iteration iter_, const Precond &P, double residual_)
    { init(A_, vB_, iter_, P, residual_); }
  };

  template <typename Matrix1, typename Matrix2, typename Precond,
	    typename local_solver>
  void add_schwarz_mat<Matrix1, Matrix2, Precond, local_solver>::init(
       const Matrix1 &A_, const std::vector<Matrix2> &vB_,
       iteration iter_, const Precond &P, double residual_) {

    vB = &vB_; A = &A_; iter = iter_;
    residual = residual_;
    
    size_type nb_sub = vB->size();
    vAloc.resize(nb_sub);
    gi.resize(nb_sub); fi.resize(nb_sub);
    precond1.resize(nb_sub);
    std::fill(precond1.begin(), precond1.end(),
	      actual_precond<Precond, local_solver, Matrix1>::transform(P));
    itebilan = 0;
    
    if (iter.get_noisy()) cout << "Init pour sub dom ";
#ifdef GMM_USES_MPI
    int size,tranche,borne_sup,borne_inf,rank,tag1=11,tag2=12,tag3=13,sizepr = 0;
    //    int tab[4];
    double t_ref,t_final;
    MPI_Status status;
    t_ref=MPI_Wtime();
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    tranche=nb_sub/size;
    borne_inf=rank*tranche;
    borne_sup=(rank+1)*tranche;
    // if (rank==size-1) borne_sup = nb_sub;

    cout << "Nombre de sous domaines " << borne_sup - borne_inf << endl;

    int sizeA = mat_nrows(*A);
    gmm::csr_matrix<value_type> Acsr(sizeA, sizeA), Acsrtemp(sizeA, sizeA);
    gmm::copy(gmm::eff_matrix(*A), Acsr);
    int next = (rank + 1) % size;
    int previous = (rank + size - 1) % size;
    //communication of local information on ring pattern
    //Each process receive  Nproc-1 contributions 

    for (int nproc = 0; nproc < size; ++nproc) {
       for (size_type i = size_type(borne_inf); i < size_type(borne_sup); ++i) {
// 	for (size_type i = 0; i < nb_sub/size; ++i) {
// 	for (size_type i = 0; i < nb_sub; ++i) {
	// size_type i=(rank+size*(j-1)+nb_sub)%nb_sub;

	cout << "Sous domaines " << i << " : " << mat_ncols((*vB)[i]) << endl;
#else
	for (size_type i = 0; i < nb_sub; ++i) {
#endif
	  
	  if (iter.get_noisy()) cout << i << " " << std::flush;
	  Matrix2 Maux(mat_ncols((*vB)[i]), mat_nrows((*vB)[i]));
	  
#ifdef GMM_USES_MPI
	  Matrix2 Maux2(mat_ncols((*vB)[i]), mat_ncols((*vB)[i]));
	  if (nproc == 0) {
	    gmm::resize(vAloc[i], mat_ncols((*vB)[i]), mat_ncols((*vB)[i]));
	    gmm::clear(vAloc[i]);
	  }
	  gmm::mult(gmm::transposed((*vB)[i]), Acsr, Maux);
	  gmm::mult(Maux, (*vB)[i], Maux2);
	  gmm::add(Maux2, vAloc[i]);
#else
	  gmm::resize(vAloc[i], mat_ncols((*vB)[i]), mat_ncols((*vB)[i]));
	  gmm::mult(gmm::transposed((*vB)[i]), *A, Maux);
	  gmm::mult(Maux, (*vB)[i], vAloc[i]);
#endif

#ifdef GMM_USES_MPI
	  if (nproc == size - 1 ) {
#endif
	    precond1[i].build_with(vAloc[i]);
	    gmm::resize(fi[i], mat_ncols((*vB)[i]));
	    gmm::resize(gi[i], mat_ncols((*vB)[i]));
#ifdef GMM_USES_MPI
	  }
#else
	}
#endif
#ifdef GMM_USES_MPI
     }
      if (nproc != size - 1) {
	MPI_Sendrecv(Acsr.jc, sizeA+1, MPI_INT, next, tag2,
		     Acsrtemp.jc, sizeA+1,MPI_INT,previous,tag2,
		     MPI_COMM_WORLD,&status);
	if (Acsrtemp.jc[sizeA] > size_type(sizepr)) {
	  sizepr = Acsrtemp.jc[sizeA];
	  delete[] Acsrtemp.pr; delete[] Acsrtemp.ir;
	  Acsrtemp.pr = new value_type[sizepr];
	  Acsrtemp.ir = new unsigned int[sizepr];
	}
	MPI_Sendrecv(Acsr.ir, Acsr.jc[sizeA], MPI_INT, next, tag1,
		     Acsrtemp.ir, Acsrtemp.jc[sizeA],MPI_INT,previous,tag1,
		     MPI_COMM_WORLD,&status);
	
	MPI_Sendrecv(Acsr.pr, Acsr.jc[sizeA], mpi_type(value_type()), next, tag3, 
		     Acsrtemp.pr, Acsrtemp.jc[sizeA],mpi_type(value_type()),previous,tag3,
		     MPI_COMM_WORLD,&status);
	gmm::copy(Acsrtemp, Acsr);
      }
    }
      t_final=MPI_Wtime();
    cout<<"temps boucle precond "<< t_final-t_ref<<endl;
#endif
    if (iter.get_noisy()) cout << "\n";
  }
  
  template <typename Matrix1, typename Matrix2, typename Precond,
	    typename Vector2, typename Vector3, typename local_solver>
  void mult(const add_schwarz_mat<Matrix1, Matrix2, Precond, local_solver> &M,
	    const Vector2 &p, Vector3 &q) {
    size_type itebilan = 0;
#ifdef GMM_USES_MPI
    static double tmult_tot = 0.0;
    double t_ref = MPI_Wtime();
#endif
    // cout << "tmult AS begin " << endl;
    mult(*(M.A), p, q);
#ifdef GMM_USES_MPI
    tmult_tot += MPI_Wtime()-t_ref;
    cout << "tmult_tot = " << tmult_tot << endl;
#endif
    std::vector<double> qbis(gmm::vect_size(q));
    std::vector<double> qter(gmm::vect_size(q));
#ifdef GMM_USES_MPI
    //    MPI_Status status;
    //    MPI_Request request,request1;
    //    int tag=111;
    int size,tranche,borne_sup,borne_inf,rank;
    size_type nb_sub=M.fi.size();
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    tranche=nb_sub/size;
    borne_inf=rank*tranche;
    borne_sup=(rank+1)*tranche;
    // if (rank==size-1) borne_sup=nb_sub;
    //    int next = (rank + 1) % size;
    //    int previous = (rank + size - 1) % size;
    t_ref = MPI_Wtime();
     for (size_type i = size_type(borne_inf); i < size_type(borne_sup); ++i)
//        for (size_type i = 0; i < nb_sub/size; ++i)
      // for (size_type j = 0; j < nb_sub; ++j)
#else
    for (size_type i = 0; i < M.fi.size(); ++i)
#endif
      {
#ifdef GMM_USES_MPI
	// size_type i=j; // (rank+size*(j-1)+nb_sub)%nb_sub;
#endif
	gmm::mult(gmm::transposed((*(M.vB))[i]), q, M.fi[i]);
       M.iter.init();
       AS_local_solve(local_solver(), (M.vAloc)[i], (M.gi)[i],
		      (M.fi)[i],(M.precond1)[i],M.iter);
       itebilan = std::max(itebilan, M.iter.get_iteration());
       }

#ifdef GMM_USES_MPI
    cout << "First  AS loop time " <<  MPI_Wtime() - t_ref << endl;
#endif

    gmm::clear(q);
#ifdef GMM_USES_MPI
    t_ref = MPI_Wtime();
    // for (size_type j = 0; j < nb_sub; ++j)
    for (size_type i = size_type(borne_inf); i < size_type(borne_sup); ++i)

#else
      for (size_type i = 0; i < M.gi.size(); ++i)
#endif
	{

#ifdef GMM_USES_MPI
	  // size_type i=j; // (rank+size*(j-1)+nb_sub)%nb_sub;
// 	  gmm::mult((*(M.vB))[i], M.gi[i], qbis,qbis);
	  gmm::mult((*(M.vB))[i], M.gi[i], qter);
	  add(qter,qbis,qbis);
#else
	  gmm::mult((*(M.vB))[i], M.gi[i], q, q);
#endif
	}
#ifdef GMM_USES_MPI
     //WARNING this add only if you use the ring pattern below
  // need to do this below if using a n explicit ring pattern communication

//      add(qbis,q,q);
    cout << "Second AS loop time " <<  MPI_Wtime() - t_ref << endl;
#endif


#ifdef GMM_USES_MPI
    //    int tag1=11;
    static double t_tot = 0.0;
    double t_final;
    t_ref=MPI_Wtime();
//     int next = (rank + 1) % size;
//     int previous = (rank + size - 1) % size;
    //communication of local information on ring pattern
    //Each process receive  Nproc-1 contributions 

//     if (size > 1) {
//     for (int nproc = 0; nproc < size-1; ++nproc) 
//       {

// 	MPI_Sendrecv(&(qbis[0]), gmm::vect_size(q), MPI_DOUBLE, next, tag1,
// 		   &(qter[0]), gmm::vect_size(q),MPI_DOUBLE,previous,tag1,
// 		   MPI_COMM_WORLD,&status);
// 	gmm::copy(qter, qbis);
// 	add(qbis,q,q);
//       }
//     }
    MPI_Allreduce(&(qbis[0]), &(q[0]),gmm::vect_size(q), MPI_DOUBLE,
		  MPI_SUM,MPI_COMM_WORLD);
    t_final=MPI_Wtime();
    t_tot += t_final-t_ref;
     cout<<"["<< rank<<"] temps reduce Resol "<< t_final-t_ref << " t_tot = " << t_tot << endl;
#endif 

    if (M.iter.get_noisy() > 0) cout << "itebloc = " << itebilan << endl;
    M.itebilan += itebilan;
    M.iter.set_resmax((M.iter.get_resmax() + M.residual) * 0.5);
  }

  template <typename Matrix1, typename Matrix2, typename Precond,
	    typename Vector2, typename Vector3, typename local_solver>
  void mult(const add_schwarz_mat<Matrix1, Matrix2, Precond, local_solver> &M,
	    const Vector2 &p, const Vector3 &q) {
    mult(M, p, const_cast<Vector3 &>(q));
  }

  template <typename Matrix1, typename Matrix2, typename Precond,
	    typename Vector2, typename Vector3, typename Vector4,
	    typename local_solver>
  void mult(const add_schwarz_mat<Matrix1, Matrix2, Precond, local_solver> &M,
	    const Vector2 &p, const Vector3 &p2, Vector4 &q)
  { mult(M, p, q); add(p2, q); }

  template <typename Matrix1, typename Matrix2, typename Precond,
	    typename Vector2, typename Vector3, typename Vector4,
	    typename local_solver>
  void mult(const add_schwarz_mat<Matrix1, Matrix2, Precond, local_solver> &M,
	    const Vector2 &p, const Vector3 &p2, const Vector4 &q)
  { mult(M, p, const_cast<Vector4 &>(q)); add(p2, q); }

  /* ******************************************************************** */
  /*		Additive Schwarz interfaced global solvers                */
  /* ******************************************************************** */

  template <typename ASM_type, typename Vect>
  void AS_global_solve(using_cg, const ASM_type &ASM, Vect &x,
		       const Vect &b, iteration &iter)
  { cg(ASM, x, b, *(ASM.A), identity_matrix(), iter); }

  template <typename ASM_type, typename Vect>
  void AS_global_solve(using_gmres, const ASM_type &ASM, Vect &x,
		       const Vect &b, iteration &iter)
  { gmres(ASM, x, b, identity_matrix(), 100, iter); }

  template <typename ASM_type, typename Vect>
  void AS_global_solve(using_bicgstab, const ASM_type &ASM, Vect &x,
		       const Vect &b, iteration &iter)
  { bicgstab(ASM, x, b, identity_matrix(), iter); }

  template <typename ASM_type, typename Vect>
  void AS_global_solve(using_qmr,const ASM_type &ASM, Vect &x,
		       const Vect &b, iteration &iter)
  { qmr(ASM, x, b, identity_matrix(), iter); }

#if defined(GMM_USES_SUPERLU)
  template <typename ASM_type, typename Vect>
  void AS_global_solve(using_superlu, const ASM_type &, Vect &,
		       const Vect &, iteration &) {
    GMM_ASSERT1(false, "You cannot use SuperLU as "
		"global solver in additive Schwarz meethod");
  }
#endif
  
  /* ******************************************************************** */
  /*	            Linear Additive Schwarz method                        */
  /* ******************************************************************** */
  /* ref : Domain decomposition algorithms for the p-version finite       */
  /*       element method for elliptic problems, Luca F. Pavarino,        */
  /*       PhD thesis, Courant Institute of Mathematical Sciences, 1992.  */
  /* ******************************************************************** */

  /** Function to call if the ASM matrix is precomputed for successive solve
   * with the same system.
   */
  template <typename Matrix1, typename Matrix2,
	    typename Vector2, typename Vector3, typename Precond,
	    typename local_solver, typename global_solver>
  void additive_schwarz(
    add_schwarz_mat<Matrix1, Matrix2, Precond, local_solver> &ASM, Vector3 &u,
    const Vector2 &f, iteration &iter, const global_solver&) {

    typedef typename linalg_traits<Matrix1>::value_type value_type;

    size_type nb_sub = ASM.vB->size(), nb_dof = gmm::vect_size(f);
    ASM.itebilan = 0;
    std::vector<value_type> g(nb_dof);
    std::vector<value_type> gbis(nb_dof);
#ifdef GMM_USES_MPI
    double t_init=MPI_Wtime();
    int size,tranche,borne_sup,borne_inf,rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    tranche=nb_sub/size;
    borne_inf=rank*tranche;
    borne_sup=(rank+1)*tranche;
    // if (rank==size-1) borne_sup=nb_sub*size;
    for (size_type i = size_type(borne_inf); i < size_type(borne_sup); ++i)
//     for (size_type i = 0; i < nb_sub/size; ++i)
      // for (size_type j = 0; j < nb_sub; ++j)
      // for (size_type i = rank; i < nb_sub; i+=size)
#else
    for (size_type i = 0; i < nb_sub; ++i)
#endif
    {

#ifdef GMM_USES_MPI
      // size_type i=j; // (rank+size*(j-1)+nb_sub)%nb_sub;
#endif
      gmm::mult(gmm::transposed((*(ASM.vB))[i]), f, ASM.fi[i]);
      ASM.iter.init();
      AS_local_solve(local_solver(), ASM.vAloc[i], ASM.gi[i], ASM.fi[i],
		     ASM.precond1[i], ASM.iter);
      ASM.itebilan = std::max(ASM.itebilan, ASM.iter.get_iteration());
#ifdef GMM_USES_MPI
    gmm::mult((*(ASM.vB))[i], ASM.gi[i], gbis,gbis);
#else   
    gmm::mult((*(ASM.vB))[i], ASM.gi[i], g, g);
#endif
    }
#ifdef GMM_USES_MPI
    cout<<"temps boucle init "<< MPI_Wtime()-t_init<<endl;
    double t_ref,t_final;
    t_ref=MPI_Wtime();
    MPI_Allreduce(&(gbis[0]), &(g[0]),gmm::vect_size(g), MPI_DOUBLE,
		  MPI_SUM,MPI_COMM_WORLD);
    t_final=MPI_Wtime();
    cout<<"temps reduce init "<< t_final-t_ref<<endl;
#endif
#ifdef GMM_USES_MPI
    t_ref=MPI_Wtime();
    cout<<"begin global AS"<<endl;
#endif
    AS_global_solve(global_solver(), ASM, u, g, iter);
#ifdef GMM_USES_MPI
    t_final=MPI_Wtime();
    cout<<"temps AS Global Solve "<< t_final-t_ref<<endl;
#endif
    if (iter.get_noisy())
      cout << "Total number of internal iterations : " << ASM.itebilan << endl;
  }

  /** Global function. Compute the ASM matrix and call the previous function.
   *  The ASM matrix represent the preconditionned linear system.
   */
  template <typename Matrix1, typename Matrix2,
	    typename Vector2, typename Vector3, typename Precond,
	    typename local_solver, typename global_solver>
  void additive_schwarz(const Matrix1 &A, Vector3 &u,
				  const Vector2 &f, const Precond &P,
				  const std::vector<Matrix2> &vB,
				  iteration &iter, local_solver,
				  global_solver) {
    iter.set_rhsnorm(vect_norm2(f));
    if (iter.get_rhsnorm() == 0.0) { gmm::clear(u); return; }
    iteration iter2 = iter; iter2.reduce_noisy();
    iter2.set_maxiter(size_type(-1));
    add_schwarz_mat<Matrix1, Matrix2, Precond, local_solver>
      ASM(A, vB, iter2, P, iter.get_resmax());
    additive_schwarz(ASM, u, f, iter, global_solver());
  }

  /* ******************************************************************** */
  /*		Sequential Non-Linear Additive Schwarz method             */
  /* ******************************************************************** */
  /* ref : Nonlinearly Preconditionned Inexact Newton Algorithms,         */
  /*       Xiao-Chuan Cai, David E. Keyes,                                */
  /*       SIAM J. Sci. Comp. 24: p183-200.  l                             */
  /* ******************************************************************** */

  template <typename Matrixt, typename MatrixBi> 
  class NewtonAS_struct {
    
  public :
    typedef Matrixt tangent_matrix_type;
    typedef MatrixBi B_matrix_type;
    typedef typename linalg_traits<Matrixt>::value_type value_type;
    typedef std::vector<value_type> Vector;
    
    virtual size_type size(void) = 0;
    virtual const std::vector<MatrixBi> &get_vB() = 0;
    
    virtual void compute_F(Vector &f, Vector &x) = 0;
    virtual void compute_tangent_matrix(Matrixt &M, Vector &x) = 0;
    // compute Bi^T grad(F(X)) Bi
    virtual void compute_sub_tangent_matrix(Matrixt &Mloc, Vector &x,
					    size_type i) = 0;
    // compute Bi^T F(X)
    virtual void compute_sub_F(Vector &fi, Vector &x, size_type i) = 0;

    virtual ~NewtonAS_struct() {}
  };

  template <typename Matrixt, typename MatrixBi> 
  struct AS_exact_gradient {
    const std::vector<MatrixBi> &vB;
    std::vector<Matrixt> vM;
    std::vector<Matrixt> vMloc;

    void init(void) {
      for (size_type i = 0; i < vB.size(); ++i) {
	Matrixt aux(gmm::mat_ncols(vB[i]), gmm::mat_ncols(vM[i]));
	gmm::resize(vMloc[i], gmm::mat_ncols(vB[i]), gmm::mat_ncols(vB[i]));
	gmm::mult(gmm::transposed(vB[i]), vM[i], aux);
	gmm::mult(aux, vB[i], vMloc[i]);
      }
    }
    AS_exact_gradient(const std::vector<MatrixBi> &vB_) : vB(vB_) {
      vM.resize(vB.size()); vMloc.resize(vB.size());
      for (size_type i = 0; i < vB.size(); ++i) {
	gmm::resize(vM[i], gmm::mat_nrows(vB[i]), gmm::mat_nrows(vB[i]));
      }
    }
  };

  template <typename Matrixt, typename MatrixBi,
	    typename Vector2, typename Vector3>
  void mult(const AS_exact_gradient<Matrixt, MatrixBi> &M,
	    const Vector2 &p, Vector3 &q) {
    gmm::clear(q);
    typedef typename gmm::linalg_traits<Vector3>::value_type T;
    std::vector<T> v(gmm::vect_size(p)), w, x;
    for (size_type i = 0; i < M.vB.size(); ++i) {
      w.resize(gmm::mat_ncols(M.vB[i]));
      x.resize(gmm::mat_ncols(M.vB[i]));
      gmm::mult(M.vM[i], p, v);
      gmm::mult(gmm::transposed(M.vB[i]), v, w);
      double rcond;
      SuperLU_solve(M.vMloc[i], x, w, rcond);
      // gmm::iteration iter(1E-10, 0, 100000);
      //gmm::gmres(M.vMloc[i], x, w, gmm::identity_matrix(), 50, iter);
      gmm::mult_add(M.vB[i], x, q);
    }
  }

  template <typename Matrixt, typename MatrixBi,
	    typename Vector2, typename Vector3>
  void mult(const AS_exact_gradient<Matrixt, MatrixBi> &M,
	    const Vector2 &p, const Vector3 &q) {
    mult(M, p, const_cast<Vector3 &>(q));
  }

  template <typename Matrixt, typename MatrixBi,
	    typename Vector2, typename Vector3, typename Vector4>
  void mult(const AS_exact_gradient<Matrixt, MatrixBi> &M,
	    const Vector2 &p, const Vector3 &p2, Vector4 &q)
  { mult(M, p, q); add(p2, q); }

  template <typename Matrixt, typename MatrixBi,
	    typename Vector2, typename Vector3, typename Vector4>
  void mult(const AS_exact_gradient<Matrixt, MatrixBi> &M,
	    const Vector2 &p, const Vector3 &p2, const Vector4 &q)
  { mult(M, p, const_cast<Vector4 &>(q)); add(p2, q); }

  struct S_default_newton_line_search {
    
    double conv_alpha, conv_r;
    size_t it, itmax, glob_it;

    double alpha, alpha_old, alpha_mult, first_res, alpha_max_ratio;
    double alpha_min_ratio, alpha_min;
    size_type count, count_pat;
    bool max_ratio_reached;
    double alpha_max_ratio_reached, r_max_ratio_reached;
    size_type it_max_ratio_reached;

    
    double converged_value(void) { return conv_alpha; };
    double converged_residual(void) { return conv_r; };

    virtual void init_search(double r, size_t git, double = 0.0) {
      alpha_min_ratio = 0.9;
      alpha_min = 1e-10;
      alpha_max_ratio = 10.0;
      alpha_mult = 0.25;
      itmax = size_type(-1);
      glob_it = git; if (git <= 1) count_pat = 0;
      conv_alpha = alpha = alpha_old = 1.;
      conv_r = first_res = r; it = 0;
      count = 0;
      max_ratio_reached = false;
    }
    virtual double next_try(void) {
      alpha_old = alpha;
      if (alpha >= 0.4) alpha *= 0.5; else alpha *= alpha_mult; ++it;
      return alpha_old;
    }
    virtual bool is_converged(double r, double = 0.0) {
      // cout << "r = " << r << " alpha = " << alpha / alpha_mult << " count_pat = " << count_pat << endl;
      if (!max_ratio_reached && r < first_res * alpha_max_ratio) {
	alpha_max_ratio_reached = alpha_old; r_max_ratio_reached = r;
	it_max_ratio_reached = it; max_ratio_reached = true; 
      }
      if (max_ratio_reached && r < r_max_ratio_reached * 0.5
	  && r > first_res * 1.1 && it <= it_max_ratio_reached+1) {
	alpha_max_ratio_reached = alpha_old; r_max_ratio_reached = r;
	it_max_ratio_reached = it;
      }
      if (count == 0 || r < conv_r)
	{ conv_r = r; conv_alpha = alpha_old; count = 1; }
      if (conv_r < first_res) ++count;

      if (r < first_res *  alpha_min_ratio)
	{ count_pat = 0.; return true; }      
      if (count >= 5 || (alpha < alpha_min && max_ratio_reached)) {
	if (conv_r < first_res * 0.99) count_pat = 0;
	if (/*gmm::random() * 50. < -log(conv_alpha)-4.0 ||*/ count_pat >= 3)
	  { conv_r=r_max_ratio_reached; conv_alpha=alpha_max_ratio_reached; }
	if (conv_r >= first_res * 0.9999) count_pat++;
	return true;
      }
      return false;
    }
    S_default_newton_line_search(void) { count_pat = 0; }
  };


  
  template <typename Matrixt, typename MatrixBi, typename Vector,
	    typename Precond, typename local_solver, typename global_solver>
  void Newton_additive_Schwarz(NewtonAS_struct<Matrixt, MatrixBi> &NS,
			       const Vector &u_,
			       iteration &iter, const Precond &P,
			       local_solver, global_solver) {
    Vector &u = const_cast<Vector &>(u_);
    typedef typename linalg_traits<Vector>::value_type value_type;
    typedef typename number_traits<value_type>::magnitude_type mtype;
    typedef actual_precond<Precond, local_solver, Matrixt> chgt_precond;
    
    double residual = iter.get_resmax();

    S_default_newton_line_search internal_ls;
    S_default_newton_line_search external_ls;

    typename chgt_precond::APrecond PP = chgt_precond::transform(P);
    iter.set_rhsnorm(mtype(1));
    iteration iternc(iter);
    iternc.reduce_noisy(); iternc.set_maxiter(size_type(-1));
    iteration iter2(iternc);
    iteration iter3(iter2); iter3.reduce_noisy();
    iteration iter4(iter3);
    iternc.set_name("Local Newton");
    iter2.set_name("Linear System for Global Newton");
    iternc.set_resmax(residual/100.0);
    iter3.set_resmax(residual/10000.0);
    iter2.set_resmax(residual/1000.0);
    iter4.set_resmax(residual/1000.0);
    std::vector<value_type> rhs(NS.size()), x(NS.size()), d(NS.size());
    std::vector<value_type> xi, xii, fi, di;

    std::vector< std::vector<value_type> > vx(NS.get_vB().size());
    for (size_type i = 0; i < NS.get_vB().size(); ++i) // for exact gradient
      vx[i].resize(NS.size()); // for exact gradient

    Matrixt Mloc, M(NS.size(), NS.size());
    NS.compute_F(rhs, u);
    mtype act_res=gmm::vect_norm2(rhs), act_res_new(0), precond_res = act_res;
    mtype alpha;
    
    while(!iter.finished(std::min(act_res, precond_res))) {
      for (int SOR_step = 0;  SOR_step >= 0; --SOR_step) {
	gmm::clear(rhs);
	for (size_type isd = 0; isd < NS.get_vB().size(); ++isd) {
	  const MatrixBi &Bi = (NS.get_vB())[isd];
	  size_type si = mat_ncols(Bi);
	  gmm::resize(Mloc, si, si);
	  xi.resize(si); xii.resize(si); fi.resize(si); di.resize(si);
	  
	  iternc.init();
	  iternc.set_maxiter(30); // ?
	  if (iternc.get_noisy())
	    cout << "Non-linear local problem " << isd << endl;
	  gmm::clear(xi);
	  gmm::copy(u, x);
	  NS.compute_sub_F(fi, x, isd); gmm::scale(fi, value_type(-1));
	  mtype r = gmm::vect_norm2(fi), r_t(r);
	  if (r > value_type(0)) {
	    iternc.set_rhsnorm(std::max(r, mtype(1)));
	    while(!iternc.finished(r)) {
	      NS.compute_sub_tangent_matrix(Mloc, x, isd);

	      PP.build_with(Mloc);
	      iter3.init();
	      AS_local_solve(local_solver(), Mloc, di, fi, PP, iter3);
	      
	      internal_ls.init_search(r, iternc.get_iteration());
	      do {
		alpha = internal_ls.next_try();
		gmm::add(xi, gmm::scaled(di, -alpha), xii);
		gmm::mult(Bi, gmm::scaled(xii, -1.0), u, x);
		NS.compute_sub_F(fi, x, isd); gmm::scale(fi, value_type(-1));
		r_t = gmm::vect_norm2(fi);
	      } while (!internal_ls.is_converged(r_t));
	      
	      if (alpha != internal_ls.converged_value()) {
		alpha = internal_ls.converged_value();
		gmm::add(xi, gmm::scaled(di, -alpha), xii);
		gmm::mult(Bi, gmm::scaled(xii, -1.0), u, x);
		NS.compute_sub_F(fi, x, isd); gmm::scale(fi, value_type(-1));
		r_t = gmm::vect_norm2(fi);
	      }
	      gmm::copy(x, vx[isd]); // for exact gradient

	      if (iternc.get_noisy()) cout << "(step=" << alpha << ")\t";
	      ++iternc; r = r_t; gmm::copy(xii, xi); 
	    }
	    if (SOR_step) gmm::mult(Bi, gmm::scaled(xii, -1.0), u, u);
	    gmm::mult(Bi, gmm::scaled(xii, -1.0), rhs, rhs);
	  }
	}
	precond_res = gmm::vect_norm2(rhs);
	if (SOR_step) cout << "SOR step residual = " << precond_res << endl;
	if (precond_res < residual) break;
	cout << "Precond residual = " << precond_res << endl;
      }

      iter2.init();
      // solving linear system for the global Newton method
      if (0) {
	NS.compute_tangent_matrix(M, u);
	add_schwarz_mat<Matrixt, MatrixBi, Precond, local_solver>
	  ASM(M, NS.get_vB(), iter4, P, iter.get_resmax());
	AS_global_solve(global_solver(), ASM, d, rhs, iter2);
      }
      else {  // for exact gradient
	AS_exact_gradient<Matrixt, MatrixBi> eg(NS.get_vB());
	for (size_type i = 0; i < NS.get_vB().size(); ++i) {
	  NS.compute_tangent_matrix(eg.vM[i], vx[i]);
	}
	eg.init();
	gmres(eg, d, rhs, gmm::identity_matrix(), 50, iter2);
      }

      //      gmm::add(gmm::scaled(rhs, 0.1), u); ++iter;
      external_ls.init_search(act_res, iter.get_iteration());
      do {
	alpha = external_ls.next_try();
	gmm::add(gmm::scaled(d, alpha), u, x);
	NS.compute_F(rhs, x);
	act_res_new = gmm::vect_norm2(rhs);
      } while (!external_ls.is_converged(act_res_new));
      
      if (alpha != external_ls.converged_value()) {
	alpha = external_ls.converged_value();
	gmm::add(gmm::scaled(d, alpha), u, x);
	NS.compute_F(rhs, x);
	act_res_new = gmm::vect_norm2(rhs);
      }

      if (iter.get_noisy() > 1) cout << endl;
      act_res = act_res_new; 
      if (iter.get_noisy()) cout << "(step=" << alpha << ")\t unprecond res = " << act_res << " ";
      
      
      ++iter; gmm::copy(x, u);
    }
  }

}


#endif //  GMM_SOLVERS_SCHWARZ_ADDITIVE_H__
