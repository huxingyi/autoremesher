/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================
 
 Copyright (C) 2003-2012 Yves Renard, Julien Pommier
 
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

/**@file gmm_inoutput.h
   @author Yves Renard <Yves.Renard@insa-lyon.fr>
   @author Julien Pommier <Julien.Pommier@insa-toulouse.fr>
   @date July 8, 2003.
   @brief Input/output on sparse matrices

   Support Harwell-Boeing and Matrix-Market formats.
*/
#ifndef GMM_INOUTPUT_H
#define GMM_INOUTPUT_H

#include <stdio.h>
#include "gmm_kernel.h"
namespace gmm {

  /*************************************************************************/
  /*                                                                       */
  /*  Functions to read and write Harwell Boeing format.                   */
  /*                                                                       */
  /*************************************************************************/

  // Fri Aug 15 16:29:47 EDT 1997
  // 
  //                      Harwell-Boeing File I/O in C
  //                               V. 1.0
  // 
  //          National Institute of Standards and Technology, MD.
  //                            K.A. Remington
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //                                NOTICE
  //
  // Permission to use, copy, modify, and distribute this software and
  // its documentation for any purpose and without fee is hereby granted
  // provided that the above copyright notice appear in all copies and
  // that both the copyright notice and this permission notice appear in
  // supporting documentation.
  //
  // Neither the Author nor the Institution (National Institute of Standards
  // and Technology) make any representations about the suitability of this 
  // software for any purpose. This software is provided "as is" without 
  // expressed or implied warranty.
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  inline void IOHBTerminate(const char *a) { GMM_ASSERT1(false, a);}

  inline bool is_complex_double__(std::complex<double>) { return true; }
  inline bool is_complex_double__(double) { return false; }

  inline int ParseIfmt(const char *fmt, int* perline, int* width) {
    if (SECURE_NONCHAR_SSCANF(fmt, " (%dI%d)", perline, width) != 2) {
      *perline = 1;
      int s = SECURE_NONCHAR_SSCANF(fmt, " (I%d)", width);
      GMM_ASSERT1(s == 1, "invalid HB I-format: " << fmt);
    }
    return *width;
  }
  
  inline int ParseRfmt(const char *fmt, int* perline, int* width,
		       int* prec, int* flag) {
    char p;
    *perline = *width = *flag = *prec = 0;
#ifdef GMM_SECURE_CRT
    if (sscanf_s(fmt, " (%d%c%d.%d)", perline, &p, sizeof(char), width, prec)
	< 3 || !strchr("PEDF", p))
#else
    if (sscanf(fmt, " (%d%c%d.%d)", perline, &p, width, prec) < 3
	|| !strchr("PEDF", p))
#endif
	{
      *perline = 1;
#ifdef GMM_SECURE_CRT
      int s = sscanf_s(fmt, " (%c%d.%d)", &p, sizeof(char), width, prec);
#else
      int s = sscanf(fmt, " (%c%d.%d)", &p, width, prec);
#endif
      GMM_ASSERT1(s>=2 && strchr("PEDF",p), "invalid HB REAL format: " << fmt);
    }
    *flag = p;
    return *width;
  }
      
  /** matrix input/output for Harwell-Boeing format */
  struct HarwellBoeing_IO {
    int nrows() const { return Nrow; }
    int ncols() const { return Ncol; }
    int nnz() const { return Nnzero; }
    int is_complex() const { return Type[0] == 'C'; }
    int is_symmetric() const { return Type[1] == 'S'; }
    int is_hermitian() const { return Type[1] == 'H'; }
    HarwellBoeing_IO() { clear(); }
    HarwellBoeing_IO(const char *filename) { clear(); open(filename); }
    ~HarwellBoeing_IO() { close(); }
    /** open filename and reads header */
    void open(const char *filename);
    /** read the opened file */
    template <typename T, int shift> void read(csc_matrix<T, shift>& A);
    template <typename MAT> void read(MAT &M) IS_DEPRECATED;
    template <typename T, int shift>
    static void write(const char *filename, const csc_matrix<T, shift>& A);
    template <typename T, int shift>
    static void write(const char *filename, const csc_matrix<T, shift>& A,
		      const std::vector<T> &rhs);
    template <typename T, typename INDI, typename INDJ, int shift> 
    static void write(const char *filename,
		      const csc_matrix_ref<T*, INDI*, INDJ*, shift>& A);
    template <typename T, typename INDI, typename INDJ, int shift> 
    static void write(const char *filename,
		      const csc_matrix_ref<T*, INDI*, INDJ*, shift>& A,
		      const std::vector<T> &rhs);

    /** static method for saving the matrix */
    template <typename MAT> static void write(const char *filename,
					      const MAT& A) IS_DEPRECATED;
  private:
    FILE *f;
    char Title[73], Key[9], Rhstype[4], Type[4];
    int Nrow, Ncol, Nnzero, Nrhs;
    char Ptrfmt[17], Indfmt[17], Valfmt[21], Rhsfmt[21];
    int Ptrcrd, Indcrd, Valcrd, Rhscrd; 
    int lcount;


    void close() { if (f) fclose(f); clear(); }
    void clear() { 
      Nrow = Ncol = Nnzero = Nrhs = 0; f = 0; lcount = 0;
      memset(Type, 0, sizeof Type); 
      memset(Key, 0, sizeof Key); 
      memset(Title, 0, sizeof Title); 
    }
    char *getline(char *buf) { 
      char *p = fgets(buf, BUFSIZ, f); ++lcount;
      int s = SECURE_NONCHAR_SSCANF(buf,"%*s");
      GMM_ASSERT1(s >= 0 && p != 0,
		  "blank line in HB file at line " << lcount);
      return buf;
    }

    int substrtoi(const char *p, size_type len) {
      char s[100]; len = std::min(len, sizeof s - 1);
      SECURE_STRNCPY(s, 100, p, len); s[len] = 0; return atoi(s);
    }
    double substrtod(const char *p, size_type len, int Valflag) {
      char s[100]; len = std::min(len, sizeof s - 1);
      SECURE_STRNCPY(s, 100, p, len); s[len] = 0;
      if ( Valflag != 'F' && !strchr(s,'E')) {
	/* insert a char prefix for exp */
	int last = int(strlen(s));
	for (int j=last+1;j>=0;j--) {
	  s[j] = s[j-1];
	  if ( s[j] == '+' || s[j] == '-' ) {
	    s[j-1] = char(Valflag);                    
	    break;
	  }
	}
      }
      return atof(s);
    }
    template <typename IND_TYPE>   
    int readHB_data(IND_TYPE colptr[], IND_TYPE rowind[], 
		    double val[]) {
      /***********************************************************************/
      /*  This function opens and reads the specified file, interpreting its */
      /*  contents as a sparse matrix stored in the Harwell/Boeing standard  */
      /*  format and creating compressed column storage scheme vectors to    */
      /*  hold the index and nonzero value information.                      */
      /*                                                                     */
      /*    ----------                                                       */
      /*    **CAVEAT**                                                       */
      /*    ----------                                                       */
      /*  Parsing real formats from Fortran is tricky, and this file reader  */
      /*  does not claim to be foolproof.   It has been tested for cases     */
      /*  when the real values are printed consistently and evenly spaced on */
      /*  each line, with Fixed (F), and Exponential (E or D) formats.       */
      /*                                                                     */
      /*  **  If the input file does not adhere to the H/B format, the  **   */
      /*  **             results will be unpredictable.                 **   */
      /*                                                                     */
      /***********************************************************************/
      int i,ind,col,offset,count;
      int Ptrperline, Ptrwidth, Indperline, Indwidth;
      int Valperline, Valwidth, Valprec, Nentries;
      int Valflag = 'D';           /* Indicates 'E','D', or 'F' float format */
      char line[BUFSIZ];
      gmm::standard_locale sl;


      /*  Parse the array input formats from Line 3 of HB file  */
      ParseIfmt(Ptrfmt,&Ptrperline,&Ptrwidth);
      ParseIfmt(Indfmt,&Indperline,&Indwidth);
      if ( Type[0] != 'P' ) {          /* Skip if pattern only  */
	ParseRfmt(Valfmt,&Valperline,&Valwidth,&Valprec,&Valflag);
      }
    
      /*  Read column pointer array:   */
      offset = 0;         /* if base 0 storage is declared (via macro def),  */
      /* then storage entries are offset by 1            */
    
      for (count = 0, i=0;i<Ptrcrd;i++) {
	getline(line);
	for (col = 0, ind = 0;ind<Ptrperline;ind++) {
	  if (count > Ncol) break;
	  colptr[count] = substrtoi(line+col,Ptrwidth)-offset;
	  count++; col += Ptrwidth;
	}
      }
    
      /*  Read row index array:  */    
      for (count = 0, i=0;i<Indcrd;i++) {
	getline(line);
	for (col = 0, ind = 0;ind<Indperline;ind++) {
	  if (count == Nnzero) break;
	  rowind[count] = substrtoi(line+col,Indwidth)-offset;
	  count++; col += Indwidth;
	}
      }
    
      /*  Read array of values:  */
      if ( Type[0] != 'P' ) {          /* Skip if pattern only  */
	if ( Type[0] == 'C' ) Nentries = 2*Nnzero;
	else Nentries = Nnzero;
      
	count = 0;
	for (i=0;i<Valcrd;i++) {
	  getline(line);
	  if (Valflag == 'D')  {
            // const_cast Due to aCC excentricity
	    char *p;
	    while( (p = const_cast<char *>(strchr(line,'D')) )) *p = 'E';
	  }
	  for (col = 0, ind = 0;ind<Valperline;ind++) {
	    if (count == Nentries) break;
	    val[count] = substrtod(line+col, Valwidth, Valflag);
	    count++; col += Valwidth;
	  }
	}
      }
      return 1;
    }
  };
  
  inline void HarwellBoeing_IO::open(const char *filename) {
    int Totcrd,Neltvl,Nrhsix;
    char line[BUFSIZ];
    close();
    SECURE_FOPEN(&f, filename, "r");
    GMM_ASSERT1(f, "could not open " << filename);
    /* First line: */
#ifdef GMM_SECURE_CRT
    sscanf_s(getline(line), "%c%s", Title, 72, Key, 8);
#else
    sscanf(getline(line), "%72c%8s", Title, Key);
#endif
    Key[8] = Title[72] = 0;
    /* Second line: */
    Totcrd = Ptrcrd = Indcrd = Valcrd = Rhscrd = 0;
    SECURE_NONCHAR_SSCANF(getline(line), "%d%d%d%d%d", &Totcrd, &Ptrcrd,
			  &Indcrd, &Valcrd, &Rhscrd);
    
    /* Third line: */
    Nrow = Ncol = Nnzero = Neltvl = 0;
#ifdef GMM_SECURE_CRT
    if (sscanf_s(getline(line), "%c%d%d%d%d", Type, 3, &Nrow, &Ncol, &Nnzero,
		 &Neltvl) < 1)
#else
    if (sscanf(getline(line), "%3c%d%d%d%d", Type, &Nrow, &Ncol, &Nnzero,
	       &Neltvl) < 1)
#endif
      IOHBTerminate("Invalid Type info, line 3 of Harwell-Boeing file.\n");
    for (size_type i = 0; i < 3; ++i) Type[i] = char(toupper(Type[i]));
    
      /*  Fourth line:  */
#ifdef GMM_SECURE_CRT
    if ( sscanf_s(getline(line), "%c%c%c%c",Ptrfmt, 16,Indfmt, 16,Valfmt,
		  20,Rhsfmt, 20) < 3)
#else
    if ( sscanf(getline(line), "%16c%16c%20c%20c",Ptrfmt,Indfmt,Valfmt,
		Rhsfmt) < 3)
#endif
      IOHBTerminate("Invalid format info, line 4 of Harwell-Boeing file.\n"); 
    Ptrfmt[16] = Indfmt[16] = Valfmt[20] = Rhsfmt[20] = 0;
    
    /*  (Optional) Fifth line: */
    if (Rhscrd != 0 ) { 
      Nrhs = Nrhsix = 0;
#ifdef GMM_SECURE_CRT
      if ( sscanf_s(getline(line), "%c%d%d", Rhstype, 3, &Nrhs, &Nrhsix) != 1)
#else
      if ( sscanf(getline(line), "%3c%d%d", Rhstype, &Nrhs, &Nrhsix) != 1)
#endif
	IOHBTerminate("Invalid RHS type information, line 5 of"
		      " Harwell-Boeing file.\n");
    }
  }

  /* only valid for double and complex<double> csc matrices */
  template <typename T, int shift> void
  HarwellBoeing_IO::read(csc_matrix<T, shift>& A) {

    typedef typename csc_matrix<T, shift>::IND_TYPE IND_TYPE;

    GMM_ASSERT1(f, "no file opened!");
    GMM_ASSERT1(Type[0] != 'P',
		"Bad HB matrix format (pattern matrices not supported)");
    GMM_ASSERT1(!is_complex_double__(T()) || Type[0] != 'R',
		"Bad HB matrix format (file contains a REAL matrix)");
    GMM_ASSERT1(is_complex_double__(T()) || Type[0] != 'C',
		"Bad HB matrix format (file contains a COMPLEX matrix)");
    A.nc = ncols(); A.nr = nrows();
    A.jc.resize(ncols()+1);
    A.ir.resize(nnz());
    A.pr.resize(nnz());
    readHB_data(&A.jc[0], &A.ir[0], (double*)&A.pr[0]);
    for (int i = 0; i <= ncols(); ++i) { A.jc[i] += shift; A.jc[i] -= 1; }
    for (int i = 0; i < nnz(); ++i)    { A.ir[i] += shift; A.ir[i] -= 1; }
  }

  template <typename MAT> void 
  HarwellBoeing_IO::read(MAT &M) {
    csc_matrix<typename gmm::linalg_traits<MAT>::value_type> csc;
    read(csc); 
    resize(M, mat_nrows(csc), mat_ncols(csc));
    copy(csc, M);
  }
  
  template <typename IND_TYPE> 
  inline int writeHB_mat_double(const char* filename, int M, int N, int nz,
				const IND_TYPE colptr[],
				const IND_TYPE rowind[], 
				const double val[], int Nrhs,
				const double rhs[], const double guess[],
				const double exact[], const char* Title,
				const char* Key, const char* Type, 
				const char* Ptrfmt, const char* Indfmt,
				const char* Valfmt, const char* Rhsfmt,
				const char* Rhstype, int shift) {
    /************************************************************************/
    /*  The writeHB function opens the named file and writes the specified  */
    /*  matrix and optional right-hand-side(s) to that file in              */
    /*  Harwell-Boeing format.                                              */
    /*                                                                      */
    /*  For a description of the Harwell Boeing standard, see:              */
    /*            Duff, et al.,  ACM TOMS Vol.15, No.1, March 1989          */
    /*                                                                      */
    /************************************************************************/
    FILE *out_file;
    int i, entry, offset, j, acount, linemod;
    int totcrd, ptrcrd, indcrd, valcrd, rhscrd;
    int nvalentries, nrhsentries;
    int Ptrperline, Ptrwidth, Indperline, Indwidth;
    int Rhsperline, Rhswidth, Rhsprec, Rhsflag;
    int Valperline, Valwidth, Valprec;
    int Valflag;           /* Indicates 'E','D', or 'F' float format */
    char pformat[16],iformat[16],vformat[19],rformat[19];
    //    char *pValflag, *pRhsflag;
    gmm::standard_locale sl;
    
    if ( Type[0] == 'C' )
      { nvalentries = 2*nz; nrhsentries = 2*M; }
    else
      { nvalentries = nz; nrhsentries = M; }
    
    if ( filename != NULL ) {
      SECURE_FOPEN(&out_file, filename, "w");
      GMM_ASSERT1(out_file != NULL, "Error: Cannot open file: " << filename);
    } else out_file = stdout;
    
    if ( Ptrfmt == NULL ) Ptrfmt = "(8I10)";
    ParseIfmt(Ptrfmt, &Ptrperline, &Ptrwidth);
    SECURE_SPRINTF1(pformat,sizeof(pformat),"%%%dd",Ptrwidth);
    ptrcrd = (N+1)/Ptrperline;
    if ( (N+1)%Ptrperline != 0) ptrcrd++;
    
    if ( Indfmt == NULL ) Indfmt =  Ptrfmt;
    ParseIfmt(Indfmt, &Indperline, &Indwidth);
    SECURE_SPRINTF1(iformat,sizeof(iformat), "%%%dd",Indwidth);
    indcrd = nz/Indperline;
    if ( nz%Indperline != 0) indcrd++;
    
    if ( Type[0] != 'P' ) {          /* Skip if pattern only  */
      if ( Valfmt == NULL ) Valfmt = "(4E21.13)";
      ParseRfmt(Valfmt, &Valperline, &Valwidth, &Valprec, &Valflag);
//       if (Valflag == 'D') {
//         pValflag = (char *) strchr(Valfmt,'D');
//         *pValflag = 'E';
//       }
      if (Valflag == 'F')
	SECURE_SPRINTF2(vformat, sizeof(vformat), "%% %d.%df", Valwidth,
			Valprec);
      else
	SECURE_SPRINTF2(vformat, sizeof(vformat), "%% %d.%dE", Valwidth,
			Valprec);
      valcrd = nvalentries/Valperline;
      if ( nvalentries%Valperline != 0) valcrd++;
    } else valcrd = 0;
    
    if ( Nrhs > 0 ) {
      if ( Rhsfmt == NULL ) Rhsfmt = Valfmt;
      ParseRfmt(Rhsfmt,&Rhsperline,&Rhswidth,&Rhsprec, &Rhsflag);
      if (Rhsflag == 'F')
	SECURE_SPRINTF2(rformat,sizeof(rformat), "%% %d.%df",Rhswidth,Rhsprec);
      else
	SECURE_SPRINTF2(rformat,sizeof(rformat), "%% %d.%dE",Rhswidth,Rhsprec);
//       if (Valflag == 'D') {
//         pRhsflag = (char *) strchr(Rhsfmt,'D');
//         *pRhsflag = 'E';
//       }
      rhscrd = nrhsentries/Rhsperline; 
      if ( nrhsentries%Rhsperline != 0) rhscrd++;
      if ( Rhstype[1] == 'G' ) rhscrd+=rhscrd;
      if ( Rhstype[2] == 'X' ) rhscrd+=rhscrd;
      rhscrd*=Nrhs;
    } else rhscrd = 0;
    
    totcrd = 4+ptrcrd+indcrd+valcrd+rhscrd;
    
    
    /*  Print header information:  */
    
    fprintf(out_file,"%-72s%-8s\n%14d%14d%14d%14d%14d\n",Title, Key, totcrd,
	    ptrcrd, indcrd, valcrd, rhscrd);
    fprintf(out_file,"%3s%11s%14d%14d%14d%14d\n",Type,"          ", M, N, nz, 0);
    fprintf(out_file,"%-16s%-16s%-20s", Ptrfmt, Indfmt, Valfmt);
    if ( Nrhs != 0 ) {
      /* Print Rhsfmt on fourth line and                              */
      /*  optional fifth header line for auxillary vector information:*/
      fprintf(out_file,"%-20s\n%-14s%d\n",Rhsfmt,Rhstype,Nrhs);
    }
    else
      fprintf(out_file,"\n");
    
    offset = 1 - shift;  /* if base 0 storage is declared (via macro def), */
    /* then storage entries are offset by 1           */
    
    /*  Print column pointers:   */
    for (i = 0; i < N+1; i++) {
      entry = colptr[i]+offset;
      fprintf(out_file,pformat,entry);
      if ( (i+1)%Ptrperline == 0 ) fprintf(out_file,"\n");
    }
    
    if ( (N+1) % Ptrperline != 0 ) fprintf(out_file,"\n");
    
    /*  Print row indices:       */
    for (i=0;i<nz;i++) {
      entry = rowind[i]+offset;
      fprintf(out_file,iformat,entry);
      if ( (i+1)%Indperline == 0 ) fprintf(out_file,"\n");
    }
    
    if ( nz % Indperline != 0 ) fprintf(out_file,"\n");
    
    /*  Print values:            */
    
    if ( Type[0] != 'P' ) {          /* Skip if pattern only  */
      for (i=0;i<nvalentries;i++) {
	fprintf(out_file,vformat,val[i]);
	if ( (i+1)%Valperline == 0 ) fprintf(out_file,"\n");
      }
      
      if ( nvalentries % Valperline != 0 ) fprintf(out_file,"\n");
      
      /*  Print right hand sides:  */
      acount = 1;
      linemod=0;
      if ( Nrhs > 0 ) {
	for (j=0;j<Nrhs;j++) {
	  for (i=0;i<nrhsentries;i++) {
	    fprintf(out_file,rformat,rhs[i] /* *Rhswidth */);
	    if ( acount++%Rhsperline == linemod ) fprintf(out_file,"\n");
	  }
	  if ( acount%Rhsperline != linemod ) {
	    fprintf(out_file,"\n");
	    linemod = (acount-1)%Rhsperline;
	  }
	  if ( Rhstype[1] == 'G' ) {
	    for (i=0;i<nrhsentries;i++) {
	      fprintf(out_file,rformat,guess[i] /* *Rhswidth */);
	      if ( acount++%Rhsperline == linemod ) fprintf(out_file,"\n");
	    }
	    if ( acount%Rhsperline != linemod ) {
	      fprintf(out_file,"\n");
	      linemod = (acount-1)%Rhsperline;
	    }
	  }
	  if ( Rhstype[2] == 'X' ) {
	    for (i=0;i<nrhsentries;i++) {
	      fprintf(out_file,rformat,exact[i] /* *Rhswidth */);
	      if ( acount++%Rhsperline == linemod ) fprintf(out_file,"\n");
	    }
	    if ( acount%Rhsperline != linemod ) {
	      fprintf(out_file,"\n");
	      linemod = (acount-1)%Rhsperline;
	    }
	  }
	}
      }
    }
    int s = fclose(out_file);
    GMM_ASSERT1(s == 0, "Error closing file in writeHB_mat_double().");
    return 1;
  }
  
  template <typename T, int shift> void
  HarwellBoeing_IO::write(const char *filename,
			  const csc_matrix<T, shift>& A) {
    write(filename, csc_matrix_ref<const T*, const unsigned*,
	  const unsigned *, shift>
	  (&A.pr[0], &A.ir[0], &A.jc[0], A.nr, A.nc));
  }

  template <typename T, int shift> void
  HarwellBoeing_IO::write(const char *filename,
			  const csc_matrix<T, shift>& A,
			  const std::vector<T> &rhs) {
    write(filename, csc_matrix_ref<const T*, const unsigned*,
	  const unsigned *, shift>
	  (&A.pr[0], &A.ir[0], &A.jc[0], A.nr, A.nc), rhs);
  }

  template <typename T, typename INDI, typename INDJ, int shift> void
  HarwellBoeing_IO::write(const char *filename,
			  const csc_matrix_ref<T*, INDI*, INDJ*, shift>& A) {
    const char *t = 0;    
    if (is_complex_double__(T()))
      if (mat_nrows(A) == mat_ncols(A)) t = "CUA"; else t = "CRA";
    else
      if (mat_nrows(A) == mat_ncols(A)) t = "RUA"; else t = "RRA";
    writeHB_mat_double(filename, int(mat_nrows(A)), int(mat_ncols(A)),
		       A.jc[mat_ncols(A)], A.jc, A.ir,
		       (const double *)A.pr,
		       0, 0, 0, 0, "GETFEM++ CSC MATRIX", "CSCMAT",
		       t, 0, 0, 0, 0, "F", shift);
  }

  template <typename T, typename INDI, typename INDJ, int shift> void
  HarwellBoeing_IO::write(const char *filename,
			  const csc_matrix_ref<T*, INDI*, INDJ*, shift>& A,
			  const std::vector<T> &rhs) {
    const char *t = 0;
    if (is_complex_double__(T()))
      if (mat_nrows(A) == mat_ncols(A)) t = "CUA"; else t = "CRA";
    else
      if (mat_nrows(A) == mat_ncols(A)) t = "RUA"; else t = "RRA";
    int Nrhs = gmm::vect_size(rhs) / mat_nrows(A);
    writeHB_mat_double(filename, int(mat_nrows(A)), int(mat_ncols(A)),
		       A.jc[mat_ncols(A)], A.jc, A.ir,
		       (const double *)A.pr,
		       Nrhs, (const double *)(&rhs[0]), 0, 0,
		       "GETFEM++ CSC MATRIX", "CSCMAT",
		       t, 0, 0, 0, 0, "F  ", shift);
  }

  
  template <typename MAT> void
  HarwellBoeing_IO::write(const char *filename, const MAT& A) {
    gmm::csc_matrix<typename gmm::linalg_traits<MAT>::value_type> 
      tmp(gmm::mat_nrows(A), gmm::mat_ncols(A));
    gmm::copy(A,tmp); 
    HarwellBoeing_IO::write(filename, tmp);
  }

  /** save a "double" or "std::complex<double>" csc matrix into a
      HarwellBoeing file
  */
  template <typename T, int shift> inline void
  Harwell_Boeing_save(const std::string &filename,
		      const csc_matrix<T, shift>& A)
  { HarwellBoeing_IO::write(filename.c_str(), A); }

  /** save a reference on "double" or "std::complex<double>" csc matrix
      into a HarwellBoeing file
  */
  template <typename T, typename INDI, typename INDJ, int shift> inline void
  Harwell_Boeing_save(const std::string &filename,
		      const csc_matrix_ref<T, INDI, INDJ, shift>& A)
  { HarwellBoeing_IO::write(filename.c_str(), A); }

  /** save a "double" or "std::complex<double>" generic matrix
      into a HarwellBoeing file making a copy in a csc matrix
  */
  template <typename MAT> inline void
  Harwell_Boeing_save(const std::string &filename, const MAT& A) {
    gmm::csc_matrix<typename gmm::linalg_traits<MAT>::value_type> 
      tmp(gmm::mat_nrows(A), gmm::mat_ncols(A));
    gmm::copy(A, tmp); 
    HarwellBoeing_IO::write(filename.c_str(), tmp);
  }

  template <typename MAT, typename VECT> inline void
  Harwell_Boeing_save(const std::string &filename, const MAT& A,
		      const VECT &RHS) {
    typedef typename gmm::linalg_traits<MAT>::value_type T;
    gmm::csc_matrix<T> tmp(gmm::mat_nrows(A), gmm::mat_ncols(A));
    gmm::copy(A, tmp);
    std::vector<T> tmprhs(gmm::vect_size(RHS));
    gmm::copy(RHS, tmprhs);
    HarwellBoeing_IO::write(filename.c_str(), tmp, tmprhs);
  }

  /** load a "double" or "std::complex<double>" csc matrix from a
      HarwellBoeing file
  */
  template <typename T, int shift> void
  Harwell_Boeing_load(const std::string &filename, csc_matrix<T, shift>& A) {
    HarwellBoeing_IO h(filename.c_str()); h.read(A);
  }

  /** load a "double" or "std::complex<double>" generic matrix from a
      HarwellBoeing file
  */
  template <typename MAT> void
  Harwell_Boeing_load(const std::string &filename, MAT& A) {
    csc_matrix<typename gmm::linalg_traits<MAT>::value_type> csc;
    Harwell_Boeing_load(filename, csc);
    resize(A, mat_nrows(csc), mat_ncols(csc));
    copy(csc, A);
  }

  /*************************************************************************/
  /*                                                                       */
  /*  Functions to read and write MatrixMarket format.                     */
  /*                                                                       */
  /*************************************************************************/

  /* 
   *   Matrix Market I/O library for ANSI C
   *
   *   See http://math.nist.gov/MatrixMarket for details.
   *
   *
   */

#define MM_MAX_LINE_LENGTH 1025
#define MatrixMarketBanner "%%MatrixMarket"
#define MM_MAX_TOKEN_LENGTH 64

  typedef char MM_typecode[4];

  /******************* MM_typecode query functions *************************/

#define mm_is_matrix(typecode)	        ((typecode)[0]=='M')
  
#define mm_is_sparse(typecode)	        ((typecode)[1]=='C')
#define mm_is_coordinate(typecode)      ((typecode)[1]=='C')
#define mm_is_dense(typecode)	        ((typecode)[1]=='A')
#define mm_is_array(typecode)	        ((typecode)[1]=='A')
  
#define mm_is_complex(typecode)	        ((typecode)[2]=='C')
#define mm_is_real(typecode)	        ((typecode)[2]=='R')
#define mm_is_pattern(typecode)	        ((typecode)[2]=='P')
#define mm_is_integer(typecode)         ((typecode)[2]=='I')
  
#define mm_is_symmetric(typecode)       ((typecode)[3]=='S')
#define mm_is_general(typecode)	        ((typecode)[3]=='G')
#define mm_is_skew(typecode)	        ((typecode)[3]=='K')
#define mm_is_hermitian(typecode)       ((typecode)[3]=='H')
  
  /******************* MM_typecode modify fucntions ************************/

#define mm_set_matrix(typecode)	        ((*typecode)[0]='M')
#define mm_set_coordinate(typecode)	((*typecode)[1]='C')
#define mm_set_array(typecode)	        ((*typecode)[1]='A')
#define mm_set_dense(typecode)	        mm_set_array(typecode)
#define mm_set_sparse(typecode)	        mm_set_coordinate(typecode)

#define mm_set_complex(typecode)        ((*typecode)[2]='C')
#define mm_set_real(typecode)	        ((*typecode)[2]='R')
#define mm_set_pattern(typecode)        ((*typecode)[2]='P')
#define mm_set_integer(typecode)        ((*typecode)[2]='I')


#define mm_set_symmetric(typecode)      ((*typecode)[3]='S')
#define mm_set_general(typecode)        ((*typecode)[3]='G')
#define mm_set_skew(typecode)	        ((*typecode)[3]='K')
#define mm_set_hermitian(typecode)      ((*typecode)[3]='H')

#define mm_clear_typecode(typecode)     ((*typecode)[0]=(*typecode)[1]= \
			       	        (*typecode)[2]=' ',(*typecode)[3]='G')

#define mm_initialize_typecode(typecode) mm_clear_typecode(typecode)


  /******************* Matrix Market error codes ***************************/


#define MM_COULD_NOT_READ_FILE	11
#define MM_PREMATURE_EOF		12
#define MM_NOT_MTX				13
#define MM_NO_HEADER			14
#define MM_UNSUPPORTED_TYPE		15
#define MM_LINE_TOO_LONG		16
#define MM_COULD_NOT_WRITE_FILE	17


  /******************** Matrix Market internal definitions *****************

   MM_matrix_typecode: 4-character sequence

	                object 	    sparse/   	data        storage 
	                            dense     	type        scheme

   string position:	 [0]        [1]		[2]         [3]

   Matrix typecode:     M(atrix)    C(oord)	R(eal)      G(eneral)
		                    A(array)    C(omplex)   H(ermitian)
	                                        P(attern)   S(ymmetric)
                                                I(nteger)   K(kew)

  ***********************************************************************/

#define MM_MTX_STR	   "matrix"
#define MM_ARRAY_STR	   "array"
#define MM_DENSE_STR	   "array"
#define MM_COORDINATE_STR  "coordinate" 
#define MM_SPARSE_STR	   "coordinate"
#define MM_COMPLEX_STR	   "complex"
#define MM_REAL_STR	   "real"
#define MM_INT_STR	   "integer"
#define MM_GENERAL_STR     "general"
#define MM_SYMM_STR	   "symmetric"
#define MM_HERM_STR	   "hermitian"
#define MM_SKEW_STR	   "skew-symmetric"
#define MM_PATTERN_STR     "pattern"

  inline char  *mm_typecode_to_str(MM_typecode matcode) {
    char buffer[MM_MAX_LINE_LENGTH];
    const char *types[4] = {0,0,0,0};
    /*    int error =0; */
    /*   int i; */
    
    /* check for MTX type */
    if (mm_is_matrix(matcode)) 
      types[0] = MM_MTX_STR;
    /*
      else
      error=1;
    */
    /* check for CRD or ARR matrix */
    if (mm_is_sparse(matcode))
      types[1] = MM_SPARSE_STR;
    else
      if (mm_is_dense(matcode))
        types[1] = MM_DENSE_STR;
      else
        return NULL;
    
    /* check for element data type */
    if (mm_is_real(matcode))
      types[2] = MM_REAL_STR;
    else
      if (mm_is_complex(matcode))
        types[2] = MM_COMPLEX_STR;
      else
	if (mm_is_pattern(matcode))
	  types[2] = MM_PATTERN_STR;
	else
	  if (mm_is_integer(matcode))
	    types[2] = MM_INT_STR;
	  else
	    return NULL;
    
    
    /* check for symmetry type */
    if (mm_is_general(matcode))
      types[3] = MM_GENERAL_STR;
    else if (mm_is_symmetric(matcode))
      types[3] = MM_SYMM_STR;
    else if (mm_is_hermitian(matcode))
      types[3] = MM_HERM_STR;
    else  if (mm_is_skew(matcode))
      types[3] = MM_SKEW_STR;
    else
      return NULL;
    
    SECURE_SPRINTF4(buffer, sizeof(buffer), "%s %s %s %s", types[0], types[1],
		    types[2], types[3]);
    return SECURE_STRDUP(buffer);
    
  }
  
  inline int mm_read_banner(FILE *f, MM_typecode *matcode) {
    char line[MM_MAX_LINE_LENGTH];
    char banner[MM_MAX_TOKEN_LENGTH];
    char mtx[MM_MAX_TOKEN_LENGTH]; 
    char crd[MM_MAX_TOKEN_LENGTH];
    char data_type[MM_MAX_TOKEN_LENGTH];
    char storage_scheme[MM_MAX_TOKEN_LENGTH];
    char *p;
    gmm::standard_locale sl;
    /*    int ret_code; */
    
    mm_clear_typecode(matcode);  
    
    if (fgets(line, MM_MAX_LINE_LENGTH, f) == NULL) 
      return MM_PREMATURE_EOF;

#ifdef GMM_SECURE_CRT
    if (sscanf_s(line, "%s %s %s %s %s", banner, sizeof(banner),
		 mtx, sizeof(mtx), crd, sizeof(crd), data_type,
		 sizeof(data_type), storage_scheme,
		 sizeof(storage_scheme)) != 5)
#else
	if (sscanf(line, "%s %s %s %s %s", banner, mtx, crd,
		   data_type, storage_scheme) != 5)
#endif
      return MM_PREMATURE_EOF;

    for (p=mtx; *p!='\0'; *p=char(tolower(*p)),p++) {};  /* convert to lower case */
    for (p=crd; *p!='\0'; *p=char(tolower(*p)),p++) {};  
    for (p=data_type; *p!='\0'; *p=char(tolower(*p)),p++) {};
    for (p=storage_scheme; *p!='\0'; *p=char(tolower(*p)),p++) {};

    /* check for banner */
    if (strncmp(banner, MatrixMarketBanner, strlen(MatrixMarketBanner)) != 0)
      return MM_NO_HEADER;

    /* first field should be "mtx" */
    if (strcmp(mtx, MM_MTX_STR) != 0)
      return  MM_UNSUPPORTED_TYPE;
    mm_set_matrix(matcode);


    /* second field describes whether this is a sparse matrix (in coordinate
       storgae) or a dense array */


    if (strcmp(crd, MM_SPARSE_STR) == 0)
      mm_set_sparse(matcode);
    else
      if (strcmp(crd, MM_DENSE_STR) == 0)
	mm_set_dense(matcode);
      else
        return MM_UNSUPPORTED_TYPE;
    

    /* third field */

    if (strcmp(data_type, MM_REAL_STR) == 0)
      mm_set_real(matcode);
    else
      if (strcmp(data_type, MM_COMPLEX_STR) == 0)
        mm_set_complex(matcode);
      else
	if (strcmp(data_type, MM_PATTERN_STR) == 0)
	  mm_set_pattern(matcode);
	else
	  if (strcmp(data_type, MM_INT_STR) == 0)
	    mm_set_integer(matcode);
	  else
	    return MM_UNSUPPORTED_TYPE;
    

    /* fourth field */

    if (strcmp(storage_scheme, MM_GENERAL_STR) == 0)
      mm_set_general(matcode);
    else
      if (strcmp(storage_scheme, MM_SYMM_STR) == 0)
        mm_set_symmetric(matcode);
      else
	if (strcmp(storage_scheme, MM_HERM_STR) == 0)
	  mm_set_hermitian(matcode);
	else
	  if (strcmp(storage_scheme, MM_SKEW_STR) == 0)
	    mm_set_skew(matcode);
	  else
	    return MM_UNSUPPORTED_TYPE;
        
    return 0;
  }

  inline int mm_read_mtx_crd_size(FILE *f, int *M, int *N, int *nz ) {
    char line[MM_MAX_LINE_LENGTH];
    /* int ret_code;*/
    int num_items_read;
    
    /* set return null parameter values, in case we exit with errors */
    *M = *N = *nz = 0;
    
    /* now continue scanning until you reach the end-of-comments */
    do {
      if (fgets(line,MM_MAX_LINE_LENGTH,f) == NULL) 
	return MM_PREMATURE_EOF;
    } while (line[0] == '%');
    
    /* line[] is either blank or has M,N, nz */
    if (SECURE_NONCHAR_SSCANF(line, "%d %d %d", M, N, nz) == 3) return 0;
    else
      do { 
	num_items_read = SECURE_NONCHAR_FSCANF(f, "%d %d %d", M, N, nz); 
	if (num_items_read == EOF) return MM_PREMATURE_EOF;
      }
      while (num_items_read != 3);
    
    return 0;
  }


  inline int mm_read_mtx_crd_data(FILE *f, int, int, int nz, int I[],
				  int J[], double val[], MM_typecode matcode) {
    int i;
    if (mm_is_complex(matcode)) {
      for (i=0; i<nz; i++)
	if (SECURE_NONCHAR_FSCANF(f, "%d %d %lg %lg", &I[i], &J[i],
				  &val[2*i], &val[2*i+1])
	    != 4) return MM_PREMATURE_EOF;
    }
    else if (mm_is_real(matcode)) {
      for (i=0; i<nz; i++) {
	if (SECURE_NONCHAR_FSCANF(f, "%d %d %lg\n", &I[i], &J[i], &val[i])
	    != 3) return MM_PREMATURE_EOF;
	
      }
    }
    else if (mm_is_pattern(matcode)) {
      for (i=0; i<nz; i++)
	if (SECURE_NONCHAR_FSCANF(f, "%d %d", &I[i], &J[i])
	    != 2) return MM_PREMATURE_EOF;
    }
    else return MM_UNSUPPORTED_TYPE;

    return 0;
  }

  inline int mm_write_mtx_crd(const char *fname, int M, int N, int nz,
			      int I[], int J[], const double val[],
			      MM_typecode matcode) {
    FILE *f;
    int i;
    
    if (strcmp(fname, "stdout") == 0) 
      f = stdout;
    else {
      SECURE_FOPEN(&f, fname, "w");
      if (f == NULL)
        return MM_COULD_NOT_WRITE_FILE;
    }
    
    /* print banner followed by typecode */
    fprintf(f, "%s ", MatrixMarketBanner);
    char *str = mm_typecode_to_str(matcode);
    fprintf(f, "%s\n", str);
    free(str);
    
    /* print matrix sizes and nonzeros */
    fprintf(f, "%d %d %d\n", M, N, nz);
    
    /* print values */
    if (mm_is_pattern(matcode))
      for (i=0; i<nz; i++)
	fprintf(f, "%d %d\n", I[i], J[i]);
    else
      if (mm_is_real(matcode))
        for (i=0; i<nz; i++)
	  fprintf(f, "%d %d %20.16g\n", I[i], J[i], val[i]);
      else
	if (mm_is_complex(matcode))
	  for (i=0; i<nz; i++)
            fprintf(f, "%d %d %20.16g %20.16g\n", I[i], J[i], val[2*i], 
		    val[2*i+1]);
	else {
	  if (f != stdout) fclose(f);
	  return MM_UNSUPPORTED_TYPE;
	}
    
    if (f !=stdout) fclose(f); 
    return 0;
  }
  

  /** matrix input/output for MatrixMarket storage */
  class MatrixMarket_IO {
    FILE *f;
    bool isComplex, isSymmetric, isHermitian;
    int row, col, nz;
    MM_typecode matcode;
  public:
    MatrixMarket_IO() : f(0) {}
    MatrixMarket_IO(const char *filename) : f(0) { open(filename); }
    ~MatrixMarket_IO() { if (f) fclose(f); f = 0; }

    int nrows() const { return row; }
    int ncols() const { return col; }
    int nnz() const { return nz; }
    int is_complex() const { return isComplex; }
    int is_symmetric() const { return isSymmetric; }
    int is_hermitian() const { return isHermitian; }

    /* open filename and reads header */
    void open(const char *filename);
    /* read opened file */
    template <typename Matrix> void read(Matrix &A);
    /* write a matrix */
    template <typename T, int shift> static void 
    write(const char *filename, const csc_matrix<T, shift>& A);  
    template <typename T, typename INDI, typename INDJ, int shift> static void 
    write(const char *filename,
	  const csc_matrix_ref<T*, INDI*, INDJ*, shift>& A);  
    template <typename MAT> static void 
    write(const char *filename, const MAT& A);  
  };

  /** load a matrix-market file */
  template <typename Matrix> inline void
  MatrixMarket_load(const char *filename, Matrix& A) {
    MatrixMarket_IO mm; mm.open(filename);
    mm.read(A);
  }
  /** write a matrix-market file */
  template <typename T, int shift> void
  MatrixMarket_save(const char *filename, const csc_matrix<T, shift>& A) {
    MatrixMarket_IO mm; mm.write(filename, A);
  }

  template <typename T, typename INDI, typename INDJ, int shift> inline void
  MatrixMarket_save(const char *filename,
		    const csc_matrix_ref<T, INDI, INDJ, shift>& A) {
    MatrixMarket_IO mm; mm.write(filename, A);
  }


  inline void MatrixMarket_IO::open(const char *filename) {
    gmm::standard_locale sl;
    if (f) { fclose(f); }
    SECURE_FOPEN(&f, filename, "r");
    GMM_ASSERT1(f, "Sorry, cannot open file " << filename);
    int s1 = mm_read_banner(f, &matcode);
    GMM_ASSERT1(s1 == 0, "Sorry, cannnot find the matrix market banner in "
		<< filename);
    int s2 = mm_is_coordinate(matcode), s3 = mm_is_matrix(matcode);
    GMM_ASSERT1(s2 > 0 && s3 > 0,
		"file is not coordinate storage or is not a matrix");
    int s4 = mm_is_pattern(matcode);
    GMM_ASSERT1(s4 == 0,
	       "the file does only contain the pattern of a sparse matrix");
    int s5 = mm_is_skew(matcode);
    GMM_ASSERT1(s5 == 0, "not currently supporting skew symmetric");
    isSymmetric = mm_is_symmetric(matcode) || mm_is_hermitian(matcode); 
    isHermitian = mm_is_hermitian(matcode); 
    isComplex =   mm_is_complex(matcode);
    mm_read_mtx_crd_size(f, &row, &col, &nz);
  }

  template <typename Matrix> void MatrixMarket_IO::read(Matrix &A) {
    gmm::standard_locale sl;
    typedef typename linalg_traits<Matrix>::value_type T;
    GMM_ASSERT1(f, "no file opened!");
    GMM_ASSERT1(!is_complex_double__(T()) || isComplex,
		"Bad MM matrix format (complex matrix expected)");
    GMM_ASSERT1(is_complex_double__(T()) || !isComplex,
		"Bad MM matrix format (real matrix expected)");
    A = Matrix(row, col);
    gmm::clear(A);
    
    std::vector<int> I(nz), J(nz);
    std::vector<typename Matrix::value_type> PR(nz);
    mm_read_mtx_crd_data(f, row, col, nz, &I[0], &J[0],
			 (double*)&PR[0], matcode);
    
    for (size_type i = 0; i < size_type(nz); ++i) {
        A(I[i]-1, J[i]-1) = PR[i];

        // FIXED MM Format
        if (mm_is_hermitian(matcode) && (I[i] != J[i]) ) {
            A(J[i]-1, I[i]-1) = gmm::conj(PR[i]);
        }

        if (mm_is_symmetric(matcode) && (I[i] != J[i]) ) {
            A(J[i]-1, I[i]-1) = PR[i];
        }

        if (mm_is_skew(matcode) && (I[i] != J[i]) ) {
            A(J[i]-1, I[i]-1) = -PR[i];
        }
    }
  }

  template <typename T, int shift> void 
  MatrixMarket_IO::write(const char *filename, const csc_matrix<T, shift>& A) {
    write(filename, csc_matrix_ref<const T*, const unsigned*,
	  const unsigned*,shift>
	  (&A.pr[0], &A.ir[0], &A.jc[0], A.nr, A.nc));
  }

  template <typename T, typename INDI, typename INDJ, int shift> void 
  MatrixMarket_IO::write(const char *filename, 
			 const csc_matrix_ref<T*, INDI*, INDJ*, shift>& A) {
    gmm::standard_locale sl;
    static MM_typecode t1 = {'M', 'C', 'R', 'G'};
    static MM_typecode t2 = {'M', 'C', 'C', 'G'};
    MM_typecode t;
    
    if (is_complex_double__(T())) std::copy(&(t2[0]), &(t2[0])+4, &(t[0]));
    else std::copy(&(t1[0]), &(t1[0])+4, &(t[0]));
    size_type nz = A.jc[mat_ncols(A)];
    std::vector<int> I(nz), J(nz);
    for (size_type j=0; j < mat_ncols(A); ++j) {      
      for (size_type i = A.jc[j]; i < A.jc[j+1]; ++i) {
	I[i] = A.ir[i] + 1 - shift;
	J[i] = int(j + 1);
      }
    }
    mm_write_mtx_crd(filename, int(mat_nrows(A)), int(mat_ncols(A)),
		     int(nz), &I[0], &J[0], (const double *)A.pr, t);
  }


  template <typename MAT> void
  MatrixMarket_IO::write(const char *filename, const MAT& A) {
    gmm::csc_matrix<typename gmm::linalg_traits<MAT>::value_type> 
      tmp(gmm::mat_nrows(A), gmm::mat_ncols(A));
    gmm::copy(A,tmp); 
    MatrixMarket_IO::write(filename, tmp);
  }

  template<typename VEC> static void vecsave(std::string fname, const VEC& V) {
    std::ofstream f(fname.c_str()); f.precision(16); f.imbue(std::locale("C"));
    for (size_type i=0; i < gmm::vect_size(V); ++i) f << V[i] << "\n"; 
  } 

  template<typename VEC> static void vecload(std::string fname,
					     const VEC& V_) {
    VEC &V(const_cast<VEC&>(V_));
    std::ifstream f(fname.c_str()); f.imbue(std::locale("C"));
    for (size_type i=0; i < gmm::vect_size(V); ++i) f >> V[i]; 
  }
}


#endif //  GMM_INOUTPUT_H
