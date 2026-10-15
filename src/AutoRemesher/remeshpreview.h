/*
 *  Copyright (C) 2026 AutoRemesher fork contributors
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef AUTO_REMESHER_REMESH_PREVIEW_H
#define AUTO_REMESHER_REMESH_PREVIEW_H
#include <AutoRemesher/Remesh>
#include <AutoRemesher/Vector2>
#include <AutoRemesher/Vector3>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

// Not part of the stable interface: the intermediate meshes behind the Qt
// app's preview modes.  In-process only, never serialized, and free to change
// in any release without a version bump.

namespace AutoRemesher {

struct RemeshPreview {
    bool decimated = false;
    std::vector<Vector3> decimatedVertices;
    std::vector<std::vector<size_t>> decimatedTriangles;
    std::vector<Vector3> isotropicVertices;
    std::vector<std::vector<size_t>> isotropicTriangles;
    std::vector<std::vector<Vector2>> isotropicTriangleUvs;
    std::vector<std::vector<Vector2>> isotropicOriginalTriangleUvs;
    std::vector<uint8_t> isotropicExtractedConnectionMoved;
    std::vector<Vector3> isotropicSingularVertices;
    std::vector<std::pair<Vector3, Vector3>> isotropicExtractedConnections;
    // The engine's human-readable timing lines.
    std::vector<std::string> phaseReport;
};

// remesh(), and when `preview` is not null also fills it.  The meshes are only
// filled when the run completed; phaseReport always is.
RemeshResult remeshWithPreview(const RemeshRequest& request,
    const RemeshCallbacks& callbacks,
    RemeshPreview* preview);

}

#endif
