
#ifndef H_HEXDOM_ALGO_QUADMESHER_H
#define H_HEXDOM_ALGO_QUADMESHER_H

#include <exploragram/basic/common.h>
#include <exploragram/hexdom/geometry.h>
#include <exploragram/hexdom/mesh_utils.h>
#include <geogram/mesh/mesh_io.h>

#include <assert.h>
#include <cmath>

namespace GEO {

	void add_edge(Mesh* m, vec3 P0, vec3 P1, double value = 0);

	void add_triangle(Mesh* m, vec3 P0, vec3 P1, vec3 P2, double value = 0);

	void current_test(Mesh* m, Mesh* debug_mesh);
}
#endif
