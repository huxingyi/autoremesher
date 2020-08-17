#ifndef H_HEX_CANDIDATES_H
#define H_HEX_CANDIDATES_H

#include <exploragram/basic/common.h>
#include <geogram/mesh/mesh.h>
namespace GEO {
	// export hex candidates from a mesh m with param at corners
	void export_hexes(Mesh* m, Mesh* hex);

	// not really supposed to be there for pipeline purpose...
	void export_points(Mesh* m, Mesh* pts);
}
#endif
