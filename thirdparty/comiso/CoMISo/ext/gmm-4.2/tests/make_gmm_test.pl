# Copyright (C) 2001-2012 Yves Renard
#
# This file is a part of GETFEM++
#
# Getfem++  is  free software;  you  can  redistribute  it  and/or modify it
# under  the  terms  of the  GNU  Lesser General Public License as published
# by  the  Free Software Foundation;  either version 3 of the License,  or
# (at your option) any later version along with the GCC Runtime Library
# Exception either version 3.1 or (at your option) any later version.
# This program  is  distributed  in  the  hope  that it will be useful,  but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or  FITNESS  FOR  A PARTICULAR PURPOSE.  See the GNU Lesser General Public
# License and GCC Runtime Library Exception for more details.
# You  should  have received a copy of the GNU Lesser General Public License
# along  with  this program;  if not, write to the Free Software Foundation,
# Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA.

eval 'exec perl -S $0 "$@"'
  if 0;

sub numerique { $a <=> $b; }

$nb_iter = 1;                # number of iterations on each test
$islocal = 0;
$with_qd = 0;                # test also with dd_real and qd_real
$with_lapack = 0;            # link with lapack
$srcdir = $ENV{srcdir};      # source directory
$tests_to_be_done = "";
$fix_base_type = -1;

while(@ARGV) {               # read optional parameters
  $param = $ARGV[0];
  $val = int(1 * $param);
  if ($param =~ /.cc/) {
    $tests_to_be_done = $param;
  }
  elsif ($param eq "with-qd") {
    $with_qd = 1;
  }
  elsif ($param eq "with-lapack") {
    $with_lapack = 1;
  }
  elsif ($param eq "float") {
    $fix_base_type = 0;
  }
  elsif ($param eq "double") {
    $fix_base_type = 1;
  }
  elsif ($param eq "complex_float") {
    $fix_base_type = 2;
  }
  elsif ($param eq "complex_double") {
    $fix_base_type = 3;
  }
  elsif ($param eq "dd_real") {
    $fix_base_type = 0; $with_qd = 1;
  }
  elsif ($param eq "qd_real") {
    $fix_base_type = 1; $with_qd = 1;
  }
  elsif ($param eq "complex_dd_real") {
    $fix_base_type = 2; $with_qd = 1;
  }
  elsif ($param eq "complex_qd_real") {
    $fix_base_type = 3; $with_qd = 1;
  }
  elsif ($param =~ "srcdir=") {
    ($param, $srcdir)=split('=', $param, 2);
  }
  elsif ($val != 0) {
    $nb_iter = $val;
  }
  else {
    print "Unrecognized parameter: $param\n";
    print "valid parameters are:\n";
    print ". the number of iterations on each test\n";
    print ". with-qd : test also with dd_real and qd_real\n";
    print ". with-lapack : link with lapack\n";
    print ". double, float, complex_double or complex_float";
    print " to fix the base type\n";
    print ". source name of a test procedure\n";
    print ". srcdir=name\n";
    exit(1);
  }
  shift @ARGV;
}

if ($srcdir eq "") {
  $srcdir="../../tests";
  print "WARNING : no srcdir, taking $srcdir\n";
  $islocal = 1;
}
if ($tests_to_be_done eq "") {
  $tests_to_be_done = `ls $srcdir/gmm_torture*.cc`;  # list of tests
}

if ($with_qd && $with_lapack) {
  print "Options with_qd and with_lapack are not compatible\n";
  exit(1);
}

$nb_test = 0;                # number of test procedures
$tests_list = $tests_to_be_done;
while ($tests_list)
  { ($org_name, $tests_list) = split('\s', $tests_list, 2); ++$nb_test; }

print "Gmm tests : Making $nb_iter execution";
if ($nb_iter > 1) { print "s"; }
if ($nb_test == 1) { print " of $tests_to_be_done\n"; }
else { print " of each test\n"; }

for ($iter = 1; $iter <= $nb_iter; ++$iter) {
  $tests_list = $tests_to_be_done;
  while ($tests_list) {
    ($org_name, $tests_list) = split('\s', $tests_list, 2);

    if ($nb_iter == 1) { print "Testing  $org_name"; }
    else { print "Test $iter for $org_name"; }
    if ($with_lapack) { print " linked with lapack"; }
    if ($with_qd) { print " with qd types"; }
    print "\n";

    $d = $org_name;
    do { ($b, $d) = split('/', $d, 2); } while ($d);
    $dest_name = "auto_$b";
    ($root_name, $d) = split('.cc', $dest_name, 2);
    $size_max = 30.0;

    open(DATAF, $org_name) or die "Open input file impossible : $!\n";
    open(TMPF, ">$dest_name") or die "Open output file impossible : $!\n";

    print TMPF "\n\n";

    if ($with_lapack) {
      print TMPF "#include<gmm_lapack_interface.h>\n\n";
    }

    if ($with_qd) {
      print TMPF "#include <qd/dd.h>\n";
      print TMPF "#include <qd/qd.h>\n";
      print TMPF "#include <qd/fpu.h>\n\n";
    }

    $reading_param = 1;
    $nb_param = 0;

    while (($li = <DATAF> )&& ($reading_param)) {
      chomp($li);
      if ($li=~/^\/\//) {
      if ($li=~/ENDPARAM/) { $reading_param = 0; }
      elsif ($li=~/DENSE_VECTOR_PARAM/) { $param[$nb_param++] = 1; }
      elsif ($li=~/VECTOR_PARAM/) { $param[$nb_param++] = 2; }
      elsif ($li=~/RECTANGULAR_MATRIX_PARAM/) { $param[$nb_param++] = 3; }
      elsif ($li=~/SQUARED_MATRIX_PARAM/) { $param[$nb_param++] = 4; }
      elsif ($li=~/\/\//) { }
      else { die "Error in parameter list"; }
    }
    }

    $TYPES[0] = "float";
    $TYPES[1] = "std::complex<float> ";
    $TYPES[2] = "double";
    $TYPES[3] = "std::complex<double> ";
    # $TYPES[4] = "long double";
    # $TYPES[5] = "std::complex<long double> ";
    $NB_TYPES = 4.0;

    if ($with_lapack) {
      $NB_TYPES = 4.0;
    }

    if ($with_qd) {
      $TYPES[0] = "dd_real";
      $TYPES[1] = "qd_real";
      $TYPES[2] = "std::complex<dd_real> ";
      $TYPES[3] = "std::complex<qd_real> ";
      $NB_TYPES = 4.0;
    }

    if ($fix_base_type == -1) { $TYPE = $TYPES[int($NB_TYPES * rand())]; }
    else { $TYPE = $TYPES[$fix_base_type]; }

    $VECTOR_TYPES[0] = "std::vector<$TYPE> ";
    $VECTOR_TYPES[1] = "std::vector<$TYPE> ";
    $VECTOR_TYPES[2] = "gmm::rsvector<$TYPE> ";
    $VECTOR_TYPES[3] = "gmm::wsvector<$TYPE> ";
    $VECTOR_TYPES[4] = "gmm::slvector<$TYPE> ";
    $NB_VECTOR_TYPES = 5.0;

    $MATRIX_TYPES[0] = "gmm::dense_matrix<$TYPE> ";
    $MATRIX_TYPES[1] = "gmm::dense_matrix<$TYPE> ";
    $MATRIX_TYPES[2] = "gmm::row_matrix<std::vector<$TYPE> > ";
    $MATRIX_TYPES[3] = "gmm::col_matrix<std::vector<$TYPE> > ";
    $MATRIX_TYPES[4] = "gmm::row_matrix<gmm::rsvector<$TYPE> > ";
    $MATRIX_TYPES[5] = "gmm::col_matrix<gmm::rsvector<$TYPE> > ";
    $MATRIX_TYPES[6] = "gmm::row_matrix<gmm::wsvector<$TYPE> > ";
    $MATRIX_TYPES[7] = "gmm::col_matrix<gmm::wsvector<$TYPE> > ";
    $MATRIX_TYPES[8] = "gmm::row_matrix<gmm::slvector<$TYPE> > ";
    $MATRIX_TYPES[9] = "gmm::col_matrix<gmm::slvector<$TYPE> > ";
    $NB_MATRIX_TYPES = 10.0;

    while ($li = <DATAF>) { print TMPF $li; }
    $sizep = int($size_max*rand());
    $theseed = int(10000.0*rand());
#    print "Parameters for the test:\n";
    print TMPF "\n\n\n";
    print TMPF "int main(void) {\n\n";
    if ($with_qd) {
      print TMPF "  fpu_fix_start(0);\n\n";
    }
    print TMPF "  srand($theseed);\n\n";
    print TMPF "  gmm::set_warning_level(1);\n\n";
    print TMPF "  for (int iter = 0; iter < 100000; ++iter) {\n\n";
    print TMPF "    try {\n\n";
    for ($j = 0; $j < $nb_param; ++$j) {
      $a = rand(); $b = rand();
      if ($with_lapack) { $a = $b = 1.0; }
      $sizepp = $sizep + int(50.0*rand());
      $step = $sizep; if ($step == 0) { ++$step; }
      $step = int(1.0*int($sizepp/$step - 1)*rand()) + 1;

      if (($param[$j] == 1) || ($param[$j] == 2)) { # vectors
	$lt = $VECTOR_TYPES[0];
	if ($param[$j] == 2 && $with_lapack==0) {
	  $lt = $VECTOR_TYPES[int($NB_VECTOR_TYPES * rand())];
	}
	if ($a < 0.1) {
	  $li = "      $lt param$j($sizepp);";
	  $c = int(1.0*($sizepp-$sizep+1)*rand());
	  $param_name[$j]
	    = "gmm::sub_vector(param$j, gmm::sub_interval($c, $sizep))";
	}
	elsif ($a < 0.2) {
	  $li = "      $lt param$j($sizepp);";
	  $c = int(1.0*($sizepp-($sizep*$step+1))*rand());
	  $param_name[$j]
	    = "gmm::sub_vector(param$j, gmm::sub_slice($c, $sizep, $step))";
	}
	elsif ($a < 0.3) {
	  $li = "      $lt param$j($sizepp);"; @sub_index = ();
	  @sortind = 0 .. ($sizepp-1);
	  while (@sortind)
	    { push (@sub_index, splice(@sortind , rand @sortind, 1)); }
	  @sub_index = @sub_index[0..$sizep-1];
	  @sub_index = sort numerique @sub_index;
	  if ($sizep == 0)
	    { $li = "$li\n      gmm::size_type param_tab$j [1] = {0};"; }
	  else {
	    $li="$li\n      gmm::size_type param_tab$j [$sizep] ={$sub_index[0]";
	    for ($k = 1; $k < $sizep; ++$k) { $li = "$li , $sub_index[$k]"; }
	    $li = "$li};";
	  }
	  $param_name[$j] = "gmm::sub_vector(param$j,".
	    " gmm::sub_index(&param_tab$j [0], &param_tab$j [$sizep]))";
	}
	else {
	  $li = "      $lt param$j($sizep);";
	  $param_name[$j] = "param$j";
	}
	print TMPF "$li\n      gmm::fill_random(param$j);\n";
      }
      elsif ($param[$j] == 3 || $param[$j] == 4) { # matrices
	$sm = $sizep; if ($a < 0.3) { $sm = $sizep + int(50.0*rand()); }
	$s = $sizep; if ($param[$j] == 3) { $s = int($size_max*rand()); }
	$sn = $s; if ($b < 0.3) { $sn = $s + int(50.0*rand()); }
	$param_name[$j] = "param$j";
	$lt = $MATRIX_TYPES[0];
	if ($with_lapack==0) {
	  $lt = $MATRIX_TYPES[int($NB_MATRIX_TYPES * rand())];
	}
	$li = "      $lt param$j($sm, $sn);";
	
	if ($a < 0.3 || $b < 0.3) {
	  $sub1 = "gmm::sub_interval(0, $sizep)";
	  $sub2 = "gmm::sub_interval(0, $s)";
	  if ($a < 0.1) {
	    $c = int(1.0*($sm-$sizep+1)*rand());
	    $sub1 = "gmm::sub_interval($c, $sizep)";
	  }
	  elsif ($a < 0.2) {
	    $step = $sizep; if ($step == 0) { ++$step; }
	    $step = int(1.0*int($sm/$step - 1)*rand()) + 1;
	    $c = int(1.0*($sm-($sizep*$step+1))*rand());
	    $sub1 = "gmm::sub_slice($c, $sizep, $step)";
	  }
	  elsif ($a < 0.3) {
	    @sub_index = ();
	    @sortind = 0 .. ($sm-1);
	    while (@sortind)
	      { push (@sub_index, splice(@sortind , rand @sortind, 1)); }
	    @sub_index = @sub_index[0..$sizep-1];
	    @sub_index = sort numerique @sub_index;
	    if ($sizep == 0)
	      { $li = "$li\n      gmm::size_type param_t$j [1] = {0};"; }
	    else {
	      $li="$li\n      gmm::size_type param_t$j [$sizep]= {$sub_index[0]";
	      for ($k = 1; $k < $sizep; ++$k) { $li = "$li , $sub_index[$k]"; }
	      $li = "$li};";
	    }
	    $sub1 = "gmm::sub_index(&param_t$j [0], &param_t$j [$sizep])";
	  }
	  if ($b < 0.1) {
	    $c = int(1.0*($sn-$s+1)*rand());
	    $sub2 = "gmm::sub_interval($c, $s)";
	  }
	  elsif ($b < 0.2) {
	    $step = $s; if ($step == 0) { ++$step; }
	    $step = int(1.0*int($sn/$step - 1)*rand()) + 1;
	    $c = int(1.0*($sn-($s*$step+1))*rand());
	    $sub2 = "gmm::sub_slice($c, $s, $step)";
	  }
	  elsif ($b < 0.3) {
	    @sub_index = ();
	    @sortind = 0 .. ($sn-1);
	    while (@sortind)
	      { push (@sub_index, splice(@sortind , rand @sortind, 1)); }
	    @sub_index = @sub_index[0..$s-1];
	    @sub_index = sort numerique @sub_index;
	    if ($s == 0)
	      { $li = "$li\n      gmm::size_type param_u$j [1] = {0};"; }
	    else {
	      $li="$li\n      gmm::size_type param_u$j [$s] = {$sub_index[0]";
	      for ($k = 1; $k < $s; ++$k) { $li = "$li , $sub_index[$k]"; }
	      $li = "$li};";
	    }
	    $sub2 = "gmm::sub_index(&param_u$j [0], &param_u$j [$s])";
	  }
	  $param_name[$j] = "gmm::sub_matrix(param$j, $sub1, $sub2)";
	}
	if (1.0 * rand() < 0.5) {
	  print TMPF "$li\n      gmm::fill_random(param$j);\n";
	}
	else {
	  print TMPF "$li\n      gmm::fill_random(param$j, 0.2);\n";
	}
	$sizep = $s;
      }
#      print "$li ($param_name[$j])\n";
    }
    print TMPF "    \n\n      bool ret = test_procedure($param_name[0]";
    for ($j = 1; $j < $nb_param; ++$j) { print TMPF ", $param_name[$j]"; }
    print TMPF ");\n";
    print TMPF "      if (ret) return 0;\n\n";
    print TMPF "    }\n";
    print TMPF "    GMM_STANDARD_CATCH_ERROR;\n";
    print TMPF "  }\n";
    print TMPF "  return 0;\n";
    print TMPF "}\n";

    close(DATAF);
    close(TMPF);

    `rm -f $root_name`;

    $compilo=`../gmm-config --cxx` || die('cannot execute ../gmm-config --cxx'); chomp($compilo);
    $compile_options=`../gmm-config --build-flags`;
    chomp($compile_options);
    $compile_options="$compile_options -I$srcdir/../src -I$srcdir/../include -I../src -I../include";
    $compile_libs="-lm";

    if ($with_lapack) {
      $compile_libs="-llapack -lblas -lg2c $compile_libs";
      $compile_options="$compile_options -DGMM_USES_LAPACK"
    }
    if ($with_qd) { $compile_libs="-lqd $compile_libs"; }
#   print "$compilo $compile_options $dest_name -o $root_name $compile_libs\n";
    print `$compilo $compile_options $dest_name -o $root_name $compile_libs`;

    if ($? != 0) {
      print "$compilo $compile_options $dest_name -o $root_name $compile_libs\n";
      print "\n******************************************************\n";
      print "* Compilation error, please submit this bug to\n";
      print "* Yves.Renard\@insa-lyon.fr, with the file\n";
      print "* $dest_name\n";
      print "* produced in directory \"tests\".\n";
      print "******************************************************\n";
      exit(1);
    }
    print `./$root_name`;
    if ($? != 0) {
      print "$compilo $compile_options $dest_name -o $root_name $compile_libs\n";
      print "\n******************************************************\n";
      print "* Execution error, please submit this bug to\n";
      print "* Yves.Renard\@insa-lyon.fr, with the file\n";
      print "* $dest_name\n";
      print "* produced in directory \"tests\".\n";
      print "******************************************************\n";
      exit(1);
    }
#    `rm -f $dest_name`;

  }

}
