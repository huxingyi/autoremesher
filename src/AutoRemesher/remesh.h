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
#ifndef AUTO_REMESHER_REMESH_H
#define AUTO_REMESHER_REMESH_H
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

// The stable interface between the remeshing core and its hosts (the Qt app,
// the Blender add-on's worker process, benchmarks and tests).  Everything in
// this header is versioned: adding an optional field is a minor version,
// anything that breaks a caller is a major version.  The engine class in
// <AutoRemesher/AutoRemesher> and the stage classes are not part of it.

namespace AutoRemesher {

const int InterfaceVersionMajor = 1;
const int InterfaceVersionMinor = 0;

// Defaults are those of the command line.
struct RemeshParameters {
    uint32_t targetQuadCount = 50000;
    // Curvature-adaptive density, 0 = uniform quads.
    double adaptivity = 1.0;
    // Curvature-adaptive elongation, 0 = square quads.
    double anisotropy = 1.0;
    // Multiplies the quad edge length; <= 0 leaves the core default.
    double edgeScaling = 1.0;
    // Dihedral angle above which an edge counts as sharp.
    double sharpEdgeDegrees = 90.0;
    // > 0 projects onto a PN-triangle smoothed input instead of the facets.
    double smoothNormalDegrees = 0.0;
};

struct RemeshRequest {
    // x0 y0 z0 x1 y1 z1 ..., in any units; the result is in the same space.
    std::vector<double> positions;
    // a0 b0 c0 a1 b1 c1 ..., indices into positions.  Triangles only.
    std::vector<uint32_t> triangles;
    RemeshParameters parameters;
    // Planned optional inputs, each empty by default so that leaving it out
    // keeps today's behaviour: vertexDensity, featureEdges, faceGroups,
    // guideCurves, symmetry (see INTERFACE.md in the project docs).
};

// Lets another thread stop a running remesh().  The core checks it at every
// progress point; remesh() then returns RemeshStatus::Cancelled without a mesh.
class CancelToken {
public:
    void requestCancel() noexcept
    {
        m_requested.store(true, std::memory_order_relaxed);
    }

    bool cancelRequested() const noexcept
    {
        return m_requested.load(std::memory_order_relaxed);
    }

private:
    std::atomic<bool> m_requested { false };
};

// `fraction` is how far the whole run has got, 0..1 and never decreasing;
// `step` names the step that is starting and is only valid during the call.
// Called from the calling thread or from worker threads, but never from two
// at once.  It should return quickly and must not call back into the core.
typedef std::function<void(float fraction, const char* step)> RemeshProgressHandler;

struct RemeshCallbacks {
    RemeshProgressHandler progress;
    const CancelToken* cancel = nullptr;
};

enum class RemeshStatus {
    Ok, // mesh present
    InvalidInput, // rejected before any work; message says why
    Cancelled, // stopped through the CancelToken; no mesh
    Failed // ran to the end but produced no faces
};

struct RemeshMesh {
    // x0 y0 z0 x1 y1 z1 ...
    std::vector<double> positions;
    // Corner count of each face, mostly 4.
    std::vector<uint32_t> faceSizes;
    // The corners of all faces, one face after the other.
    std::vector<uint32_t> faceIndices;
};

// Measured on the result mesh.  Field definitions are in INTERFACE.md §3.1.
struct QualityReport {
    size_t inputVertexCount = 0;
    size_t inputTriangleCount = 0;
    size_t vertexCount = 0;
    size_t faceCount = 0;
    size_t quadCount = 0;
    size_t nonQuadCount = 0;
    size_t nonManifoldEdgeCount = 0;
    size_t boundaryEdgeCount = 0;
    size_t orientationFlipCount = 0;
    size_t foldedFaceCount = 0;
    size_t degenerateFaceCount = 0;
    size_t componentCount = 0;
    size_t islandCount = 0;
    size_t islandsWithoutOutput = 0;
    // Wall time of the whole call.
    double seconds = 0.0;
    // Per-step times summed over the islands, in pipeline order.
    std::vector<std::pair<std::string, double>> stepSeconds;
    std::vector<std::string> warnings;
    // Numeric counters added by later versions (solver, rounding, cleanup);
    // readers must accept keys they don't know.
    std::vector<std::pair<std::string, double>> diagnostics;
};

struct RemeshResult {
    RemeshStatus status = RemeshStatus::Failed;
    std::string message;
    RemeshParameters parameters;
    RemeshMesh mesh;
    QualityReport report;
};

RemeshResult remesh(const RemeshRequest& request,
    const RemeshCallbacks& callbacks = RemeshCallbacks());

// Status, message, parameters and report as one JSON object (everything but
// the mesh arrays), schema "autoremesher.remesh-result".
std::string toJson(const RemeshResult& result);

const char* toString(RemeshStatus status);

}

#endif
