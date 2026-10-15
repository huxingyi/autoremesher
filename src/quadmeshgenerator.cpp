/*
 *  Copyright (c) 2020 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
#include "quadmeshgenerator.h"
#include <AutoRemesher/RemeshPreview>
#include <QDebug>
#include <QElapsedTimer>
#include <cstdio>

void QuadMeshGenerator::process()
{
    QElapsedTimer timer;
    timer.start();

    generate();

    auto timeUsed = timer.elapsed();
    for (const auto& line : m_phaseReport)
        qDebug().noquote() << "  " << QString::fromStdString(line);
    qDebug() << "Quad mesh generation took" << timeUsed << "milliseconds";

    emit finished();
}

void QuadMeshGenerator::printProgress(float progress, const QString& status)
{
    // Reprint on a new step as well as a new percent: several steps are shorter
    // than one percent of the run and would otherwise never be named.
    const int percent = (int)(progress * 100);
    if (percent == m_lastPrintedPercent && status == m_lastPrintedStatus)
        return;
    m_lastPrintedPercent = percent;
    m_lastPrintedStatus = status;
    if (status.isEmpty())
        fprintf(stdout, "%d%% done.\n", percent);
    else
        fprintf(stdout, "%d%% done. %s\n", percent, qPrintable(status));
    fflush(stdout);
}

void QuadMeshGenerator::emitProgress(float progress)
{
    printProgress(progress, QString());
    emit reportProgress(progress);
}

void QuadMeshGenerator::emitProgress(float progress, const QString& status)
{
    printProgress(progress, status);
    emit reportProgressDetailed(progress, status);
    emit reportProgress(progress);
}

void QuadMeshGenerator::generate()
{
    AutoRemesher::RemeshRequest request;
    request.positions.reserve(m_vertices.size() * 3);
    for (const auto& vertex : m_vertices) {
        request.positions.push_back(vertex.x());
        request.positions.push_back(vertex.y());
        request.positions.push_back(vertex.z());
    }
    request.triangles.reserve(m_triangles.size() * 3);
    for (const auto& triangle : m_triangles) {
        for (size_t i = 0; i < 3; ++i)
            request.triangles.push_back((uint32_t)triangle[i]);
    }
    request.parameters = m_parameters;

    AutoRemesher::RemeshCallbacks callbacks;
    callbacks.progress = [this](float progress, const char* step) {
        emitProgress(progress, QString::fromUtf8(step));
    };
    callbacks.cancel = m_cancelToken;

    AutoRemesher::RemeshPreview preview;
    const AutoRemesher::RemeshResult result = AutoRemesher::remeshWithPreview(request, callbacks, &preview);
    m_reportJson = AutoRemesher::toJson(result);
    m_phaseReport = std::move(preview.phaseReport);
    if (AutoRemesher::RemeshStatus::Ok != result.status)
        qDebug().noquote() << "Remesh" << AutoRemesher::toString(result.status) << QString::fromStdString(result.message);
    // A run that went through but produced no faces still hands over its
    // empty mesh and previews, as before; only a rejected or cancelled run
    // leaves the result unset.
    if (AutoRemesher::RemeshStatus::InvalidInput == result.status
        || AutoRemesher::RemeshStatus::Cancelled == result.status)
        return;

    const AutoRemesher::RemeshMesh& mesh = result.mesh;
    m_remeshedVertices = std::make_unique<std::vector<AutoRemesher::Vector3>>();
    m_remeshedVertices->reserve(mesh.positions.size() / 3);
    for (size_t i = 0; i < mesh.positions.size(); i += 3)
        m_remeshedVertices->emplace_back(mesh.positions[i], mesh.positions[i + 1], mesh.positions[i + 2]);
    m_remeshedQuads = std::make_unique<std::vector<std::vector<size_t>>>();
    m_remeshedQuads->reserve(mesh.faceSizes.size());
    size_t offset = 0;
    for (const uint32_t size : mesh.faceSizes) {
        m_remeshedQuads->emplace_back(mesh.faceIndices.begin() + offset, mesh.faceIndices.begin() + offset + size);
        offset += size;
    }

    // Capture intermediate isotropic mesh data for preview overlays
    m_decimated = preview.decimated;
    m_decimatedVertices = std::move(preview.decimatedVertices);
    m_decimatedTriangles = std::move(preview.decimatedTriangles);
    m_isotropicVertices = std::move(preview.isotropicVertices);
    m_isotropicTriangles = std::move(preview.isotropicTriangles);
    m_isotropicTriangleUvs = std::move(preview.isotropicTriangleUvs);
    m_isotropicOriginalTriangleUvs = std::move(preview.isotropicOriginalTriangleUvs);
    m_isotropicSingularVertices = std::move(preview.isotropicSingularVertices);
    m_isotropicExtractedConnections = std::move(preview.isotropicExtractedConnections);
    m_isotropicExtractedConnectionMoved = std::move(preview.isotropicExtractedConnectionMoved);
}
