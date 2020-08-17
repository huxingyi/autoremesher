#pragma once

namespace Layout {
	inline int to_index(int x, int y, int nx, int ny) { return y * nx + x; }

	template<typename vec2_t, typename vec2_u>
	int to_index(vec2_t v, vec2_u s) { return v[1] * s[0] + v[0]; }

	template<typename vec2_t>
	vec2_t to_grid(int id, int nx, int ny) { return  vec2_t(id % nx, id / nx); }

	template<typename vec2_t>
	vec2_t to_grid(int id, vec2_t dims) { return  vec2_t(id % dims[0], id / dims[0]); }

	template<typename vec2_t>
	bool is_valid(int x, int y, vec2_t dims) { return (x >= 0 && y >= 0 && x < dims[0] && y < dims[1]); }

	template<typename vec2_t>
	bool is_valid(int id, vec2_t dims) { return is_valid(id % dims[0], id / dims[0], dims); }
}
