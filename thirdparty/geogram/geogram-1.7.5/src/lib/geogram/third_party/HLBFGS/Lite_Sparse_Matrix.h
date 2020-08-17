///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// HLBFGS                                                                    //
// http://www.loria.fr/~liuyang/software/HLBFGS/                             //
//                                                                           //
// HLBFGS is a hybrid L-BFGS optimization framework which unifies L-BFGS     //
// method, Preconditioned L-BFGS method and                                  //
// Preconditioned Conjugate Gradient method.                                 //
//                                                                           //
// Version 1.2                                                               //
// March 09, 2010                                                            //
//                                                                           //
// Copyright (C) 2009--2010                                                  //
// Yang Liu                                                                  //
//                                                                           //
// xueyuhanlang@gmail.com                                                    //
//                                                                           //
// HLBFGS is freely available for non-commercial purposes.                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


#ifndef LITE_SPARSE_MATRIX_H
#define LITE_SPARSE_MATRIX_H

#include <vector>
#include <algorithm>
#include <cassert>
#include <ostream>
#include "Sparse_Entry.h"

// [Bruno Levy] added HLBFGS_API declarations to have linkage.
//              replaced int with unsigned int when needed.
//              added const qualifiers to const functions.
#include <geogram/api/defs.h>
#define HLBFGS_API GEOGRAM_API


// \addtogroup MathSuite
//@{

//! Symmetric status
enum SYMMETRIC_STATE {
    NOSYM,     /*!< general case */
    SYM_UPPER, /*!< symmetric (store upper triangular part) */
    SYM_LOWER, /*!< symmetric (store lower triangular part) */
    SYM_BOTH   /*!< symmetric (store both upper and lower triangular part) */
};

//!     Storage
enum SPARSE_STORAGE {
    CCS,   /*!< compress column format */
    CRS,   /*!< compress row format */
    TRIPLE /*!< row-wise coordinate format */
};

//! Array type
enum ARRAYTYPE {
    FORTRAN_TYPE, /*!< the index starts from 1 */
    C_TYPE        /*!< the index starts from 0 */
};

//////////////////////////////////////////////////////////////////////////
//! Lite Sparse Matrix Class
class HLBFGS_API Lite_Sparse_Matrix {
private:
    
    //!     Status for creating sparse solver
    enum STORE_STATE {
        ENABLE, DISABLE, LOCK
    };

    STORE_STATE state_fill_entry;
    SYMMETRIC_STATE sym_state;
    SPARSE_STORAGE s_store;
    ARRAYTYPE arraytype;

    unsigned int nrows; //!< number of rows
    unsigned int ncols; //!< number of columns
    unsigned int nonzero; //!< number of nonzeros
    //! pointers to where columns begin in rowind and values 0-based, length is (col+1)
    /*!
     * When s_store is CRS, colptr stores column indices;
     */
    std::vector<unsigned int> colptr;
    //! row indices, 0-based
    /*!
     * When s_store is CRS, rowind stores row-pointers
     */
    std::vector<unsigned int> rowind;
    std::vector<double> values; //!< nonzero values of the sparse matrix
    std::vector<std::vector<Sparse_Entry> > entryset; //!< store temporary sparse entries
    std::vector<double> diag; //! special usage for some libraries

    bool save_diag_separetely;

public:

    //! Sparse matrix constructor
    /*!
     * \param m row dimension
     * \param n column dimension
     * \param [in] symmetric_state one of 
     *  (NOSYM, SYM_UPPER, SYM_LOWER, SYM_BOTH)    
     * \param [in] m_store the storage format
     * \param [in] atype Fortran or C type of array
     */
    Lite_Sparse_Matrix(
        unsigned int m, unsigned int n,
        SYMMETRIC_STATE symmetric_state = NOSYM,
        SPARSE_STORAGE m_store = CCS,
        ARRAYTYPE atype = C_TYPE,
        bool save_diag = false
    ) : state_fill_entry(DISABLE),
        sym_state(symmetric_state), s_store(m_store), 
        arraytype(atype), 
        nrows(m), ncols(n), 
        nonzero(0),
        save_diag_separetely(save_diag) {
            if (m != n) {
                symmetric_state = NOSYM;
            }
            
            unsigned int nn = (m_store == CCS ? ncols : nrows);
            entryset.resize(nn);
            if (save_diag_separetely) {
                diag.resize(nrows < ncols ? nrows : ncols);
                std::fill(diag.begin(), diag.end(), 0.0);
            }
        }
    
    //! Sparse matrix destructor
    ~Lite_Sparse_Matrix() {
        clear_mem();
    }

    //! Start to build sparse matrix pattern
    void begin_fill_entry() {
        state_fill_entry = ENABLE;
    }

    //! Construct sparse pattern
    void end_fill_entry();

    //! Fill matrix entry \f$  Mat_{row_index, col_index} += val \f$
    void fill_entry(unsigned int row_index, unsigned int col_index, double val = 0) {
        if (row_index >= nrows || col_index >= ncols) {
            return;
        }
        
        if (save_diag_separetely && row_index == col_index) {
            diag[row_index] += val;
        }
        
        if (sym_state == NOSYM) {
            fill_entry_internal(row_index, col_index, val);
        } else if (sym_state == SYM_UPPER) {
            if (row_index <= col_index) {
                fill_entry_internal(row_index, col_index, val);
            } else {
                fill_entry_internal(col_index, row_index, val);
            }
        } else if (sym_state == SYM_LOWER) {
            if (row_index <= col_index) {
                fill_entry_internal(col_index, row_index, val);
            } else {
                fill_entry_internal(row_index, col_index, val);
            }
        } else if (sym_state == SYM_BOTH) {
            fill_entry_internal(row_index, col_index, val);
            if (row_index != col_index) {
                fill_entry_internal(col_index, row_index, val);
            }
        }
    }

    //fill the diagonal entry
    void fill_diag(unsigned int diagid, double v = 0) {
        if (diag.size() == 0) {
            diag.resize(nrows < ncols ? nrows : ncols);
            std::fill(diag.begin(), diag.end(), 0.0);
        }
        diag[diagid] += v;
    }

    //! get the number of nonzeros
    unsigned int get_nonzero() const {
        return nonzero;
    }

    //! get the row dimension
    unsigned int rows() const {
        return nrows;
    }

    //! get the column dimension
    unsigned int cols() const {
        return ncols;
    }

    //! return the symmetric state
    bool issymmetric() const {
        return sym_state != NOSYM;
    }

    //! tell whether the matrix is upper or lower symmetric
    bool issym_store_upper_or_lower() const {
        return (sym_state == SYM_LOWER) || (sym_state == SYM_UPPER);
    }

    //! return symmetric state
    SYMMETRIC_STATE symmetric_state() const {
        return sym_state;
    }

    //! tell whether the matrix is square
    bool issquare() const {
        return nrows == ncols;
    }

    //! return the storage format
    SPARSE_STORAGE storage() const {
        return s_store;
    }

    //! return array type
    ARRAYTYPE get_arraytype() const {
        return arraytype;
    }

    //! get rowind
    unsigned int *get_rowind() {
        return &rowind[0];
    }

    const unsigned int *get_rowind() const {
        return &rowind[0];
    }

    //! get colptr
    unsigned int *get_colptr() {
        return &colptr[0];
    }
    
    const unsigned int *get_colptr() const {
        return &colptr[0];
    }
    
    //! get the values array
    double *get_values() {
        return &values[0];
    }

    const double *get_values() const {
        return &values[0];
    }
    
    //! get the diagonal array
    double *get_diag() {
        return &diag[0];
    }

    const double *get_diag() const {
        return &diag[0];
    }

    //////////////////////////////////////////////////////////////////////////
private:
    //! Clear memory
    void clear_mem() {
        colptr.clear();
        rowind.clear();
        values.clear();
    }

    //! fill matrix entry (internal) \f$ Mat[rowid][colid] += val \f$
    bool fill_entry_internal(unsigned int row_index, unsigned int col_index, double val = 0) {
        assert (state_fill_entry == ENABLE);
        
        unsigned int search_index = (s_store == CCS ? row_index : col_index);
        unsigned int pos_index = (s_store == CCS ? col_index : row_index);

        Sparse_Entry forcompare(search_index);

        std::vector<Sparse_Entry>::iterator iter = std::lower_bound(
            entryset[pos_index].begin(), entryset[pos_index].end(),
            forcompare
        );
        if (iter != entryset[pos_index].end()) {
            if (iter->index == search_index) {
                iter->value += val;
            } else {
                entryset[pos_index].insert(
                    iter,Sparse_Entry(search_index, val)
                );
            }
        } else {
            entryset[pos_index].push_back(Sparse_Entry(search_index, val));
        }
        return true;
    }
    //////////////////////////////////////////////////////////////////////////
};

//! print sparse matrix
std::ostream & operator<<(std::ostream & s, const Lite_Sparse_Matrix * A);

//@}


#endif //Lite_Sparse_Matrix_H
