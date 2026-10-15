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
#include <AutoRemesher/AutoRemesher>
#include <AutoRemesher/Remesh>
#include <AutoRemesher/RemeshPreview>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <exception>
#include <iomanip>
#include <locale>
#include <numeric>
#include <sstream>

namespace AutoRemesher {

namespace {

    const char* validate(const RemeshRequest& request)
    {
        if (request.positions.empty())
            return "input mesh has no vertices";
        if (0 != request.positions.size() % 3)
            return "positions must hold three coordinates per vertex";
        if (request.triangles.empty())
            return "input mesh has no triangles";
        if (0 != request.triangles.size() % 3)
            return "triangles must hold three vertex indices each";
        for (const double coordinate : request.positions) {
            if (!std::isfinite(coordinate))
                return "input mesh has a vertex coordinate that is not a finite number";
        }
        const size_t vertexCount = request.positions.size() / 3;
        for (const uint32_t index : request.triangles) {
            if (index >= vertexCount)
                return "a triangle refers to a vertex index past the end of positions";
        }
        const RemeshParameters& parameters = request.parameters;
        if (0 == parameters.targetQuadCount)
            return "target quad count must be greater than zero";
        if (!std::isfinite(parameters.adaptivity) || !std::isfinite(parameters.anisotropy)
            || !std::isfinite(parameters.edgeScaling) || !std::isfinite(parameters.sharpEdgeDegrees)
            || !std::isfinite(parameters.smoothNormalDegrees))
            return "parameters must be finite numbers";
        return nullptr;
    }

    void forwardProgress(void* tag, float progress, const char* status)
    {
        const auto* handler = static_cast<const RemeshProgressHandler*>(tag);
        (*handler)(progress, nullptr != status ? status : "");
    }

    size_t findRoot(std::vector<size_t>& parents, size_t vertex)
    {
        while (parents[vertex] != vertex) {
            parents[vertex] = parents[parents[vertex]];
            vertex = parents[vertex];
        }
        return vertex;
    }

    void measureMesh(const RemeshMesh& mesh, QualityReport* report)
    {
        const size_t vertexCount = mesh.positions.size() / 3;
        const size_t faceCount = mesh.faceSizes.size();
        const auto position = [&](uint32_t vertex) {
            return Vector3(mesh.positions[3 * (size_t)vertex],
                mesh.positions[3 * (size_t)vertex + 1],
                mesh.positions[3 * (size_t)vertex + 2]);
        };

        report->vertexCount = vertexCount;
        report->faceCount = faceCount;

        // One entry per face side: the undirected edge as a sorted vertex pair,
        // which face it belongs to and whether the face walks it low to high.
        struct EdgeUse {
            uint64_t key;
            uint32_t face;
            bool forward;
        };
        std::vector<EdgeUse> uses;
        uses.reserve(mesh.faceIndices.size());
        std::vector<Vector3> faceNormals(faceCount);
        std::vector<size_t> parents(vertexCount);
        std::iota(parents.begin(), parents.end(), (size_t)0);
        std::vector<uint8_t> used(vertexCount, 0);

        size_t offset = 0;
        for (size_t face = 0; face < faceCount; ++face) {
            const size_t size = mesh.faceSizes[face];
            const uint32_t* corners = mesh.faceIndices.data() + offset;
            offset += size;
            if (4 == size)
                ++report->quadCount;
            else
                ++report->nonQuadCount;
            if (0 == size) {
                ++report->degenerateFaceCount;
                continue;
            }

            bool repeated = false;
            for (size_t i = 0; i < size; ++i) {
                for (size_t j = i + 1; j < size; ++j) {
                    if (corners[i] == corners[j])
                        repeated = true;
                }
            }

            // Twice the area vector, by a fan from the first corner.
            Vector3 normal;
            double longestEdgeSquared = 0.0;
            const Vector3 origin = position(corners[0]);
            for (size_t i = 0; i < size; ++i) {
                const Vector3 from = position(corners[i]);
                const Vector3 to = position(corners[(i + 1) % size]);
                longestEdgeSquared = std::max(longestEdgeSquared, (to - from).lengthSquared());
                if (i > 0 && i + 1 < size)
                    normal += Vector3::crossProduct(from - origin, to - origin);
            }
            faceNormals[face] = normal;
            if (repeated || 0.5 * normal.length() <= 1e-12 * longestEdgeSquared)
                ++report->degenerateFaceCount;

            for (size_t i = 0; i < size; ++i) {
                const uint32_t first = corners[i];
                const uint32_t second = corners[(i + 1) % size];
                used[first] = 1;
                if (first == second)
                    continue;
                const uint32_t low = std::min(first, second);
                const uint32_t high = std::max(first, second);
                uses.push_back({ ((uint64_t)low << 32) | high, (uint32_t)face, first == low });
                const size_t firstRoot = findRoot(parents, first);
                const size_t secondRoot = findRoot(parents, second);
                if (firstRoot != secondRoot)
                    parents[std::max(firstRoot, secondRoot)] = std::min(firstRoot, secondRoot);
            }
        }

        std::sort(uses.begin(), uses.end(), [](const EdgeUse& a, const EdgeUse& b) {
            return a.key != b.key ? a.key < b.key : a.face < b.face;
        });
        std::vector<Vector3> neighborNormals(faceCount);
        for (size_t begin = 0; begin < uses.size();) {
            size_t end = begin + 1;
            while (end < uses.size() && uses[end].key == uses[begin].key)
                ++end;
            const size_t count = end - begin;
            if (1 == count) {
                ++report->boundaryEdgeCount;
            } else if (2 == count) {
                const EdgeUse& a = uses[begin];
                const EdgeUse& b = uses[begin + 1];
                if (a.forward == b.forward)
                    ++report->orientationFlipCount;
                neighborNormals[a.face] += faceNormals[b.face];
                neighborNormals[b.face] += faceNormals[a.face];
            } else {
                ++report->nonManifoldEdgeCount;
            }
            begin = end;
        }

        for (size_t face = 0; face < faceCount; ++face) {
            if (Vector3::dotProduct(faceNormals[face], neighborNormals[face]) < 0.0)
                ++report->foldedFaceCount;
        }

        for (size_t vertex = 0; vertex < vertexCount; ++vertex) {
            if (used[vertex] && findRoot(parents, vertex) == vertex)
                ++report->componentCount;
        }
    }

    // Always the classic locale, so a host that installs the user's locale
    // globally still gets a decimal point and no digit grouping.
    std::string jsonNumber(double value)
    {
        if (!std::isfinite(value))
            return "null";
        std::ostringstream stream;
        stream.imbue(std::locale::classic());
        stream << std::setprecision(9) << value;
        return stream.str();
    }

    std::string jsonString(const std::string& text)
    {
        std::string out = "\"";
        for (const char c : text) {
            if ('"' == c || '\\' == c) {
                out += '\\';
                out += c;
            } else if ('\n' == c) {
                out += "\\n";
            } else if ((unsigned char)c < 0x20) {
                const char* hex = "0123456789abcdef";
                out += "\\u00";
                out += hex[((unsigned char)c >> 4) & 0xf];
                out += hex[(unsigned char)c & 0xf];
            } else {
                out += c;
            }
        }
        out += '"';
        return out;
    }

    std::string jsonCount(size_t value)
    {
        return std::to_string(value);
    }

}

const char* toString(RemeshStatus status)
{
    switch (status) {
    case RemeshStatus::Ok:
        return "ok";
    case RemeshStatus::InvalidInput:
        return "invalid_input";
    case RemeshStatus::Cancelled:
        return "cancelled";
    case RemeshStatus::Failed:
        return "failed";
    }
    return "failed";
}

RemeshResult remeshWithPreview(const RemeshRequest& request,
    const RemeshCallbacks& callbacks,
    RemeshPreview* preview)
{
    const auto start = std::chrono::steady_clock::now();
    RemeshResult result;
    result.parameters = request.parameters;
    QualityReport& report = result.report;
    report.inputVertexCount = request.positions.size() / 3;
    report.inputTriangleCount = request.triangles.size() / 3;
    const auto finish = [&](RemeshStatus status, const std::string& message) {
        result.status = status;
        result.message = message;
        report.seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        return result;
    };

    if (const char* reason = validate(request))
        return finish(RemeshStatus::InvalidInput, reason);
    if (nullptr != callbacks.cancel && callbacks.cancel->cancelRequested())
        return finish(RemeshStatus::Cancelled, "cancelled before the run started");

    std::vector<Vector3> vertices;
    vertices.reserve(request.positions.size() / 3);
    for (size_t i = 0; i < request.positions.size(); i += 3)
        vertices.emplace_back(request.positions[i], request.positions[i + 1], request.positions[i + 2]);
    std::vector<std::vector<size_t>> triangles;
    triangles.reserve(request.triangles.size() / 3);
    for (size_t i = 0; i < request.triangles.size(); i += 3)
        triangles.push_back({ request.triangles[i], request.triangles[i + 1], request.triangles[i + 2] });

    const RemeshParameters& parameters = request.parameters;
    ::AutoRemesher::AutoRemesher engine(std::move(vertices), std::move(triangles));
    // The engine counts triangles, two per quad, as the app always passed it.
    engine.setTargetTriangleCount((size_t)parameters.targetQuadCount * 2);
    if (parameters.edgeScaling > 0.0)
        engine.setScaling(parameters.edgeScaling);
    engine.setGradientAdaptivity(parameters.adaptivity);
    engine.setAnisotropy(parameters.anisotropy);
    engine.setSharpEdgeDegrees(parameters.sharpEdgeDegrees);
    engine.setSmoothNormalDegrees(parameters.smoothNormalDegrees);
    if (callbacks.progress) {
        engine.setTag(const_cast<RemeshProgressHandler*>(&callbacks.progress));
        engine.setProgressHandler(forwardProgress);
    }
    if (nullptr != callbacks.cancel) {
        const CancelToken* token = callbacks.cancel;
        engine.setCancelCheck([token]() { return token->cancelRequested(); });
    }

    // Hosts get every outcome as a status: a worker process should report an
    // error, and an exception escaping into a Qt thread would end the app.
    bool completed = false;
    try {
        completed = engine.remesh();
    } catch (const std::exception& e) {
        return finish(RemeshStatus::Failed, std::string("remeshing failed: ") + e.what());
    }

    if (nullptr != preview)
        preview->phaseReport = engine.phaseReport();
    if (engine.cancelled())
        return finish(RemeshStatus::Cancelled, "cancelled");
    if (!completed)
        return finish(RemeshStatus::Failed, "the core stopped before remeshing any island");

    for (const auto& step : engine.stepTimes())
        report.stepSeconds.emplace_back(step.name, step.microseconds / 1e6);
    report.islandCount = engine.islandCount();
    report.islandsWithoutOutput = engine.islandsWithoutOutput();

    const auto& remeshedVertices = engine.remeshedVertices();
    const auto& remeshedFaces = engine.remeshedQuads();
    RemeshMesh& mesh = result.mesh;
    mesh.positions.reserve(remeshedVertices.size() * 3);
    for (const auto& vertex : remeshedVertices) {
        mesh.positions.push_back(vertex.x());
        mesh.positions.push_back(vertex.y());
        mesh.positions.push_back(vertex.z());
    }
    mesh.faceSizes.reserve(remeshedFaces.size());
    mesh.faceIndices.reserve(remeshedFaces.size() * 4);
    for (const auto& face : remeshedFaces) {
        mesh.faceSizes.push_back((uint32_t)face.size());
        for (const size_t vertex : face)
            mesh.faceIndices.push_back((uint32_t)vertex);
    }
    measureMesh(mesh, &report);

    if (nullptr != preview) {
        preview->decimated = engine.decimated();
        preview->decimatedVertices = engine.decimatedVertices();
        preview->decimatedTriangles = engine.decimatedTriangles();
        preview->isotropicVertices = engine.isotropicVertices();
        preview->isotropicTriangles = engine.isotropicTriangles();
        preview->isotropicTriangleUvs = engine.isotropicTriangleUvs();
        preview->isotropicOriginalTriangleUvs = engine.isotropicOriginalTriangleUvs();
        preview->isotropicExtractedConnectionMoved = engine.isotropicExtractedConnectionMoved();
        preview->isotropicSingularVertices = engine.isotropicSingularVertices();
        preview->isotropicExtractedConnections = engine.isotropicExtractedConnections();
    }

    if (0 == report.faceCount)
        return finish(RemeshStatus::Failed, "remeshing produced no faces");
    if (report.islandsWithoutOutput > 0) {
        report.warnings.push_back(std::to_string(report.islandsWithoutOutput) + " of "
            + std::to_string(report.islandCount) + " islands produced no faces");
    }
    return finish(RemeshStatus::Ok, std::string());
}

RemeshResult remesh(const RemeshRequest& request, const RemeshCallbacks& callbacks)
{
    return remeshWithPreview(request, callbacks, nullptr);
}

std::string toJson(const RemeshResult& result)
{
    const RemeshParameters& parameters = result.parameters;
    const QualityReport& report = result.report;
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << "{\n";
    out << "  \"schema\": \"autoremesher.remesh-result\",\n";
    out << "  \"interface_version\": \"" << InterfaceVersionMajor << "." << InterfaceVersionMinor << "\",\n";
    out << "  \"status\": " << jsonString(toString(result.status)) << ",\n";
    out << "  \"message\": " << jsonString(result.message) << ",\n";
    out << "  \"parameters\": {"
        << "\"target_quads\": " << parameters.targetQuadCount
        << ", \"adaptivity\": " << jsonNumber(parameters.adaptivity)
        << ", \"anisotropy\": " << jsonNumber(parameters.anisotropy)
        << ", \"edge_scaling\": " << jsonNumber(parameters.edgeScaling)
        << ", \"sharp_edge_degrees\": " << jsonNumber(parameters.sharpEdgeDegrees)
        << ", \"smooth_normal_degrees\": " << jsonNumber(parameters.smoothNormalDegrees)
        << "},\n";
    out << "  \"input\": {"
        << "\"vertices\": " << jsonCount(report.inputVertexCount)
        << ", \"triangles\": " << jsonCount(report.inputTriangleCount)
        << "},\n";
    out << "  \"output\": {"
        << "\"vertices\": " << jsonCount(report.vertexCount)
        << ", \"faces\": " << jsonCount(report.faceCount)
        << ", \"quads\": " << jsonCount(report.quadCount)
        << ", \"non_quads\": " << jsonCount(report.nonQuadCount)
        << ", \"quad_ratio\": " << jsonNumber(parameters.targetQuadCount > 0 ? (double)report.quadCount / parameters.targetQuadCount : 0.0)
        << ",\n             \"non_manifold_edges\": " << jsonCount(report.nonManifoldEdgeCount)
        << ", \"boundary_edges\": " << jsonCount(report.boundaryEdgeCount)
        << ", \"orientation_flips\": " << jsonCount(report.orientationFlipCount)
        << ", \"folded_faces\": " << jsonCount(report.foldedFaceCount)
        << ",\n             \"degenerate_faces\": " << jsonCount(report.degenerateFaceCount)
        << ", \"components\": " << jsonCount(report.componentCount)
        << "},\n";
    out << "  \"islands\": {"
        << "\"input\": " << jsonCount(report.islandCount)
        << ", \"without_output\": " << jsonCount(report.islandsWithoutOutput)
        << "},\n";
    out << "  \"time\": {\"total_seconds\": " << jsonNumber(report.seconds) << ", \"steps\": [";
    for (size_t i = 0; i < report.stepSeconds.size(); ++i) {
        out << (0 == i ? "\n" : ",\n")
            << "    {\"name\": " << jsonString(report.stepSeconds[i].first)
            << ", \"seconds\": " << jsonNumber(report.stepSeconds[i].second) << "}";
    }
    out << (report.stepSeconds.empty() ? "]},\n" : "\n  ]},\n");
    out << "  \"warnings\": [";
    for (size_t i = 0; i < report.warnings.size(); ++i)
        out << (0 == i ? "" : ", ") << jsonString(report.warnings[i]);
    out << "],\n";
    out << "  \"diagnostics\": {";
    for (size_t i = 0; i < report.diagnostics.size(); ++i) {
        out << (0 == i ? "" : ", ") << jsonString(report.diagnostics[i].first)
            << ": " << jsonNumber(report.diagnostics[i].second);
    }
    out << "}\n";
    out << "}\n";
    return out.str();
}

}
