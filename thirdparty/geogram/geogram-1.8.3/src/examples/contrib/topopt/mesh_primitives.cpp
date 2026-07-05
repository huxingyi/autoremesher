#include "mesh_primitives.h"

using namespace GEO;

index_t create_cylinder_3d(double base, double top, double height, int slices, Mesh &M) {
	index_t v0 = M.vertices.nb();
	for (int v = 0; v < slices; ++v) {
		double alpha = v * 2.0 * M_PI / slices;
		vec3 pos = base * vec3(std::cos(alpha), std::sin(alpha), 0);
		M.vertices.create_vertex(pos.data());
	}
	for (int v = 0; v < slices; ++v) {
		double alpha = v * 2.0 * M_PI / slices;
		vec3 pos = top * vec3(std::cos(alpha), std::sin(alpha), 0);
		pos[2] = height;
		M.vertices.create_vertex(pos.data());
	}

	for (int v = 0; v < slices; ++v) {
		M.facets.create_quad(v0+v, v0+(v+1)%slices, v0+(v+1)%slices+slices, v0+v+slices);
	}
	for (int v = 0; v < slices; ++v) {
		M.facets.create_triangle(v0, v0+v, v0+(v+1)%slices);
	}
	for (int v = 0; v < slices; ++v) {
		M.facets.create_triangle(v0+slices, v0+(v+1)%slices+slices, v0+v+slices);
	}
	M.facets.connect();

	return M.vertices.nb();
}

index_t create_arrow_3d(double base, double top, double h1, double h2, int slices, GEO::Mesh &M) {
	index_t v0 = create_cylinder_3d(base, base, h1, slices, M);
	index_t v1 = create_cylinder_3d(top, 0.0, h2, slices, M);
	for (int v = v0; v < v1; ++v) {
		M.vertices.point(v) += vec3(0, 0, h1);
	}
	return v1;
}

index_t create_arrow_2d(double b, double t, double h1, double h2, GEO::Mesh &M) {
	index_t v0 = M.vertices.create_vertex(vec3(-b/2.0, 0, 0).data());
	index_t v1 = M.vertices.create_vertex(vec3(b/2.0, 0, 0).data());
	index_t v2 = M.vertices.create_vertex(vec3(b/2.0, h1, 0).data());
	index_t v3 = M.vertices.create_vertex(vec3(-b/2.0, h1, 0).data());

	index_t v4 = M.vertices.create_vertex(vec3(-t/2.0, h1, 0).data());
	index_t v5 = M.vertices.create_vertex(vec3(t/2.0, h1, 0).data());
	index_t v6 = M.vertices.create_vertex(vec3(0, h1+h2, 0).data());

	M.facets.create_quad(v0, v1, v2, v3);
	// M.facets.create_triangle(v4, v5, v6);
	M.facets.create_triangle(v4, v3, v6);
	M.facets.create_triangle(v3, v2, v6);
	M.facets.create_triangle(v2, v5, v6);
	M.facets.connect();

	return M.vertices.nb();
}
