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
// HLBFGS is HLBFGS is freely available for non-commercial purposes.         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "Lite_Sparse_Matrix.h"

void Lite_Sparse_Matrix::end_fill_entry() {
    assert (state_fill_entry == ENABLE);
    clear_mem();
    state_fill_entry = LOCK;
    
    unsigned int inc = (arraytype == FORTRAN_TYPE ? 1 : 0);
    
    if (s_store == CCS) {
        //construct map and ccs matrix
        unsigned int i, j, k = 0;
        colptr.resize(ncols + 1);
        colptr[0] = inc;
        for (j = 1; j < ncols + 1; j++) {
            colptr[j] = (unsigned int) entryset[j - 1].size() + colptr[j - 1];
        }
        nonzero = colptr[ncols];
        if (nonzero > 0) {
            rowind.resize(nonzero);
            values.resize(nonzero);
            
            for (j = 0; j < ncols; j++) {
                for (i = 0; i < colptr[j + 1] - colptr[j]; i++) {
                    rowind[k] = entryset[j][i].index + inc;
                    values[k] = entryset[j][i].value;
                    k++;
                }
            }
        }
    } else if (s_store == CRS) {
        //construct map and crs matrix
        unsigned int i, j, k = 0;
        rowind.resize(nrows + 1);
        rowind[0] = inc;
        for (j = 1; j < nrows + 1; j++) {
            rowind[j] = (unsigned int) entryset[j - 1].size() + rowind[j - 1];
        }
        nonzero = rowind[nrows];
        if (nonzero > 0) {
            colptr.resize(nonzero);
            values.resize(nonzero);
            for (j = 0; j < nrows; j++) {
                for (i = 0; i < rowind[j + 1] - rowind[j]; i++) {
                    colptr[k] = entryset[j][i].index + inc;
                    values[k] = entryset[j][i].value;
                    k++;
                }
            }
        }
    } else if (s_store == TRIPLE) {
        unsigned int i, j, k = 0;
        nonzero = 0;
        for (i = 0; i < nrows; i++) {
            nonzero += (unsigned int) entryset[i].size();
        }
        if (nonzero > 0) {
            rowind.resize(nonzero);
            colptr.resize(nonzero);
            values.resize(nonzero);
            
            for (i = 0; i < nrows; i++) {
                unsigned int jsize = (int) entryset[i].size();
                for (j = 0; j < jsize; j++) {
                    rowind[k] = i + inc;
                    colptr[k] = entryset[i][j].index + inc;
                    values[k] = entryset[i][j].value;
                    k++;
                }
            }
        }
    }
    entryset.clear();
}

std::ostream & operator<<(std::ostream & s, Lite_Sparse_Matrix* A) {
    s.precision(16);
    if (A == nullptr) {
        s << "matrix pointer is NULL !\n ";
    }
    
    unsigned int row = A->rows();
    unsigned int col = A->cols();
    unsigned int nonzero = A->get_nonzero();
    const unsigned int *rowind = A->get_rowind();
    const unsigned int *colptr = A->get_colptr();
    const double *values = A->get_values();
    
    s << "row :" << row << " col :" << col << " Nonzero: " << nonzero << "\n\n";
    s << "matrix --- (i, j, value)\n\n";

    SPARSE_STORAGE s_store = A->storage();
    int inc = (A->get_arraytype() == FORTRAN_TYPE ? -1 : 0);
    if (s_store == CCS) {
        unsigned int k = 0;
        for (unsigned int i = 1; i < col + 1; i++) {
            for (unsigned int j = 0; j < colptr[i] - colptr[i - 1]; j++) {
                s << int(rowind[k]) + inc << " " << i - 1 << " " << std::scientific
                  << values[k] << "\n";
                k++;
            }
        }
    } else if (s_store == CRS) {
        unsigned int k = 0;
        for (unsigned int i = 1; i < row + 1; i++) {
            for (unsigned int j = 0; j < rowind[i] - rowind[i - 1]; j++) {
                s << i - 1 << " " << colptr[k] + inc << " " << std::scientific
                  << values[k] << "\n";
                k++;
            }
        }
    } else if (s_store == TRIPLE) {
        for (unsigned int k = 0; k < nonzero; k++) {
            s << rowind[k] + inc << " " << colptr[k] + inc << " "
              << std::scientific << values[k] << "\n";
        }
    }
    return s;
}
