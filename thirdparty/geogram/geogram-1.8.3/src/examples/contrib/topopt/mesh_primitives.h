#pragma once

#include <geogram/mesh/mesh.h>

GEO::index_t create_cylinder_3d(double base, double top, double height, int slices, GEO::Mesh &M);

GEO::index_t create_arrow_3d(double base, double top, double h1, double h2, int slices, GEO::Mesh &M);

GEO::index_t create_arrow_2d(double base, double top, double h1, double h2, GEO::Mesh &M);
