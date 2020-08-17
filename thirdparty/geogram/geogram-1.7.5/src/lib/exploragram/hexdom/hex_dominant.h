#ifndef H_HEX_DOMINANT_H
#define H_HEX_DOMINANT_H

#include <geogram/mesh/mesh.h>
namespace GEO {
	void  fill_cavity_with_tetgen(Mesh* input, Mesh* tri, bool with_pyramid);
	void  add_hexes_to_tetmesh(Mesh* hex, Mesh* tet_mesh);
	void Baudoin_mesher(Mesh* m);

	void hex_dominant(Mesh* cavity, Mesh* hexahedrons, Mesh* result);
}
#endif
