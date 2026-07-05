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

#ifndef SPARSE_ENTRY_H
#define SPARSE_ENTRY_H

//! Sparse Entry class \ingroup MathSuite
class Sparse_Entry {
public:
    //! Index ID
    unsigned int index;
    
    //! Real value
    double value;
    
public:
    //! constructor
    Sparse_Entry (unsigned int ind, double v = 0) : index (ind), value (v) {
    }

    //! The compare function for sorting
    inline bool operator< (const Sparse_Entry & m_r) const {
        return index < m_r.index;
    }
};

#endif //SPARSE_ENTRY_H
