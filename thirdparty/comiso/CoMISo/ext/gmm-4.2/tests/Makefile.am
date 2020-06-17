
check_PROGRAMS = dummy 

dummy_SOURCES = dummy.cc 

INCLUDES = -I$(top_srcdir)/include -I../include

LDADD    = -lm @SUPLDFLAGS@

TESTS = $(top_srcdir)/tests/make_gmm_test.pl

EXTRA_DIST=\
          make_gmm_test.pl\
          gmm_torture01_lusolve.cc\
          gmm_torture02_baseop.cc\
          gmm_torture05_mult.cc\
          gmm_torture06_mat_mult.cc\
          gmm_torture10_qr.cc\
          gmm_torture15_sub.cc\
          gmm_torture20_iterative_solvers.cc


CLEANFILES = toto.mat ii_files/* auto_gmm* 

TESTS_ENVIRONMENT = perl

