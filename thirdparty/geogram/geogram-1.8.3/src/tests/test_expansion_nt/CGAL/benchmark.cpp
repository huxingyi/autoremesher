
// A simple CGAL program that computes a 3D Delaunay triangulation,
// with GEOGRAM's expansion_nt class plugged into it.

// Uses expansion_nt as CGAL arbitrary precision type
//  (comment-out to use standard CGAL arbitrary precision type).
// Note: this is done by having a modified local copy of
//   include/CGAL/internal/Exact_type_selector.h
// The CMakeLists.txt declares "./" before the standard include path,
//   so that our Exact_type_selector.h is used instead of the one in
//   CGAL includes.
#define CGAL_USE_GEO_EXPANSION

#include <iostream>
#include <fstream>
#include <string>

// Must be included before the kernel
#include <CGAL_expansion_nt.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Timer.h>
#include <iostream>

typedef CGAL::Exact_predicates_inexact_constructions_kernel  K;
typedef CGAL::Delaunay_triangulation_3<K> DT;
typedef K::Point_3 Point_3;
typedef CGAL::Timer Timer;

int main(int argc, char**argv) {
    
  GEO::expansion::initialize();
  std::vector<Point_3> points;
  Point_3 p;

  //  To be able to measure timings, we compute
  // the triangulation several times.
  int niter = 5;

  //   First argument, if present, is the number
  // of times the triangulation will be computed.
  if(argc >= 2) {
      niter = atoi(argv[1]);
  }

  //   Second argument, if present, is the filename
  // where to read the points. If not present, points
  // are read from the standard input.
  if(argc >= 3) {
      std::ifstream in(argv[2]);
      while(in >> p){
          points.push_back(p);
      }
  } else {
      while(std::cin >> p){
          points.push_back(p);
      }
  }
  
  Timer timer;
  timer.start();
  size_t N = 0;
  for(int i = 0; i < niter; i++) {
      DT dt;
      dt.insert(points.begin(), points.end());
      N += dt.number_of_cells();
  }
  timer.stop();
  
  std::cerr << N << std::endl << timer.time() << " sec" << std::endl;
  return 0;
}
