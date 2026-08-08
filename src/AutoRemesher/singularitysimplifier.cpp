/*
 *  Copyright (c) 2020 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
#include <AutoRemesher/SingularitySimplifier>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <geogram/mesh/mesh_geometry.h>
#include <iostream>
#include <map>
#include <set>

namespace AutoRemesher {

namespace {

    constexpr double kQuarterTurn = M_PI / 2.0;

    inline int quarterTurnsBetween(double from, double to)
    {
        const int turns = (int)std::lround((to - from) / kQuarterTurn);
        return ((turns % 4) + 4) % 4;
    }

    // The facet a corner's edge is shared with, together with the reciprocal
    // corner on that facet, or NO_CORNER when the edge is on the border or the
    // adjacency is one sided (the parameterizer breaks such edges deliberately).
    inline GEO::index_t reciprocalCorner(const GEO::Mesh& M, GEO::index_t c, GEO::index_t* neighbor)
    {
        const GEO::index_t f2 = M.facet_corners.adjacent_facet(c);
        *neighbor = f2;
        if (GEO::NO_FACET == f2)
            return GEO::NO_CORNER;
        const GEO::index_t e2 = M.facets.find_adjacent(f2, c / 3);
        if (GEO::NO_FACET == e2)
            return GEO::NO_CORNER;
        return M.facets.corners_begin(f2) + e2;
    }

}

void SingularitySimplifier::buildFrames()
{
    const GEO::Mesh& M = *m_mesh;
    m_frameU.resize(M.facets.nb());
    m_frameV.resize(M.facets.nb());
    m_angles.resize(M.facets.nb(), 0.0);
    m_cornerOfVertex.assign(M.vertices.nb(), GEO::NO_CORNER);
    m_representationX.assign(M.facets.nb(), 0.0);
    m_representationY.assign(M.facets.nb(), 0.0);
    for (GEO::index_t c = 0; c < M.facet_corners.nb(); ++c) {
        const GEO::index_t v = M.facet_corners.vertex(c);
        if (GEO::NO_CORNER == m_cornerOfVertex[v])
            m_cornerOfVertex[v] = c;
    }
    for (GEO::index_t f = 0; f < M.facets.nb(); ++f) {
        const GEO::vec3 normal = GEO::normalize(GEO::Geom::mesh_facet_normal(M, f));
        GEO::vec3 u = GEO::Geom::mesh_corner_vector(M, M.facets.corners_begin(f));
        u -= GEO::dot(u, normal) * normal;
        const double length = GEO::length(u);
        if (length <= 1e-12) {
            // Degenerate facet: any frame will do, it carries no field of its own.
            u = std::fabs(normal.x) < 0.9 ? GEO::vec3(1.0, 0.0, 0.0) : GEO::vec3(0.0, 1.0, 0.0);
            u -= GEO::dot(u, normal) * normal;
            u = GEO::normalize(u);
        } else {
            u /= length;
        }
        m_frameU[f] = u;
        m_frameV[f] = GEO::cross(normal, u);

        const GEO::vec3& b = (*m_field)[f];
        m_angles[f] = std::atan2(GEO::dot(b, m_frameV[f]), GEO::dot(b, m_frameU[f]));
    }
}

void SingularitySimplifier::buildConnection()
{
    const GEO::Mesh& M = *m_mesh;
    const double sharpRadians = m_sharpEdgeDegrees * M_PI / 180.0;
    m_connection.assign(M.facet_corners.nb(), 0.0);
    m_mismatch.assign(M.facet_corners.nb(), 0);
    m_sharpCorner.assign(M.facet_corners.nb(), false);
    for (GEO::index_t c = 0; c < M.facet_corners.nb(); ++c) {
        GEO::index_t f2 = GEO::NO_FACET;
        if (GEO::NO_CORNER == reciprocalCorner(M, c, &f2))
            continue;
        const GEO::index_t f1 = c / 3;
        // Both sides measure the same 3d edge vector, so the difference of its
        // angle in the two frames is exactly the rotation that unfolds f1 onto f2.
        const GEO::vec3 edge = GEO::Geom::mesh_corner_vector(M, c);
        const double angleInF1 = std::atan2(GEO::dot(edge, m_frameV[f1]), GEO::dot(edge, m_frameU[f1]));
        const double angleInF2 = std::atan2(GEO::dot(edge, m_frameV[f2]), GEO::dot(edge, m_frameU[f2]));
        m_connection[c] = angleInF2 - angleInF1;
        m_sharpCorner[c] = GEO::Geom::mesh_normal_angle(M, c) >= sharpRadians;
    }
    std::vector<GEO::index_t> everyFacet(M.facets.nb());
    for (GEO::index_t f = 0; f < M.facets.nb(); ++f)
        everyFacet[f] = f;
    updateCornerOfFacets(everyFacet);
}

void SingularitySimplifier::updateCornerOfFacets(const std::vector<GEO::index_t>& facets)
{
    const GEO::Mesh& M = *m_mesh;
    for (const GEO::index_t f : facets) {
        for (GEO::index_t c = M.facets.corners_begin(f); c < M.facets.corners_end(f); ++c) {
            GEO::index_t f2 = GEO::NO_FACET;
            const GEO::index_t c2 = reciprocalCorner(M, c, &f2);
            if (GEO::NO_CORNER == c2)
                continue;
            const int turns = quarterTurnsBetween(m_angles[f] + m_connection[c], m_angles[f2]);
            m_mismatch[c] = turns;
            m_mismatch[c2] = (4 - turns) % 4;
        }
    }
}

std::vector<GEO::index_t> SingularitySimplifier::facetsAroundVertex(GEO::index_t v) const
{
    const GEO::Mesh& M = *m_mesh;
    std::vector<GEO::index_t> facets;
    const GEO::index_t start = m_cornerOfVertex[v];
    if (GEO::NO_CORNER == start)
        return facets;
    GEO::index_t c = start;
    for (;;) {
        facets.push_back(c / 3);
        GEO::index_t f2 = GEO::NO_FACET;
        if (GEO::NO_CORNER == reciprocalCorner(M, c, &f2))
            return std::vector<GEO::index_t>();
        GEO::index_t next = GEO::NO_CORNER;
        for (GEO::index_t candidate = M.facets.corners_begin(f2);
            candidate < M.facets.corners_end(f2); ++candidate) {
            if (M.facet_corners.vertex(candidate) == v) {
                next = candidate;
                break;
            }
        }
        if (GEO::NO_CORNER == next)
            return std::vector<GEO::index_t>();
        c = next;
        if (c == start)
            return facets;
        if (facets.size() > M.facet_corners.nb())
            return std::vector<GEO::index_t>();
    }
}

// Index type of the cross field at v: the total number of quarter turns the field
// picks up relative to parallel transport once around the one ring, taken mod 4.
// Zero means regular; 1 and 3 are the two opposite cone types that pair up, and 2
// is a half turn cone. Boundary and non manifold vertices report regular, since a
// singularity there is not something this pass may touch.
int SingularitySimplifier::vertexIndex(GEO::index_t v) const
{
    const GEO::Mesh& M = *m_mesh;
    const GEO::index_t start = m_cornerOfVertex[v];
    if (GEO::NO_CORNER == start)
        return 0;
    int total = 0;
    size_t steps = 0;
    GEO::index_t c = start;
    do {
        if (++steps > M.facet_corners.nb())
            return 0;
        total += m_mismatch[c];
        GEO::index_t f2 = GEO::NO_FACET;
        const GEO::index_t c2 = reciprocalCorner(M, c, &f2);
        if (GEO::NO_CORNER == c2)
            return 0;
        GEO::index_t next = GEO::NO_CORNER;
        for (GEO::index_t candidate = M.facets.corners_begin(f2);
            candidate < M.facets.corners_end(f2); ++candidate) {
            if (M.facet_corners.vertex(candidate) == v) {
                next = candidate;
                break;
            }
        }
        if (GEO::NO_CORNER == next)
            return 0;
        c = next;
    } while (c != start);
    return ((total % 4) + 4) % 4;
}

std::vector<GEO::index_t> SingularitySimplifier::collectSingularVertices(std::vector<int>* indices) const
{
    const GEO::Mesh& M = *m_mesh;
    std::vector<GEO::index_t> singular;
    indices->clear();
    for (GEO::index_t v = 0; v < M.vertices.nb(); ++v) {
        const int index = vertexIndex(v);
        if (0 == index)
            continue;
        singular.push_back(v);
        indices->push_back(index);
    }
    return singular;
}

void SingularitySimplifier::writeBackField(const std::vector<GEO::index_t>& facets)
{
    for (const GEO::index_t f : facets) {
        (*m_field)[f] = std::cos(m_angles[f]) * m_frameU[f]
            + std::sin(m_angles[f]) * m_frameV[f];
    }
}

bool SingularitySimplifier::cancelPair(GEO::index_t firstVertex, GEO::index_t secondVertex, size_t hops)
{
    GEO::Mesh& M = *m_mesh;

    // A lens shaped region: everything close enough to both cones. It always
    // contains the two of them plus the corridor between, and nothing else.
    const size_t radius = hops + m_regionMargin;
    auto ball = [&](const std::vector<GEO::index_t>& seeds) {
        std::map<GEO::index_t, size_t> distance;
        std::deque<GEO::index_t> queue;
        for (const GEO::index_t f : seeds) {
            if (distance.insert({ f, 0 }).second)
                queue.push_back(f);
        }
        while (!queue.empty()) {
            const GEO::index_t f = queue.front();
            queue.pop_front();
            const size_t d = distance[f];
            if (d >= radius)
                continue;
            for (GEO::index_t c = M.facets.corners_begin(f); c < M.facets.corners_end(f); ++c) {
                GEO::index_t f2 = GEO::NO_FACET;
                if (GEO::NO_CORNER == reciprocalCorner(M, c, &f2))
                    continue;
                if (distance.insert({ f2, d + 1 }).second)
                    queue.push_back(f2);
            }
        }
        return distance;
    };

    const std::vector<GEO::index_t> firstFan = facetsAroundVertex(firstVertex);
    const std::vector<GEO::index_t> secondFan = facetsAroundVertex(secondVertex);
    if (firstFan.empty() || secondFan.empty())
        return false;
    const std::map<GEO::index_t, size_t> firstBall = ball(firstFan);
    const std::map<GEO::index_t, size_t> secondBall = ball(secondFan);

    std::set<GEO::index_t> region;
    for (const auto& it : firstBall) {
        if (secondBall.count(it.first))
            region.insert(it.first);
    }
    if (region.empty())
        return false;

    // A facet is frozen if it sits on the rim of the region, so the enclosed index
    // stays pinned by untouched data, or if it touches a sharp edge. Re-smoothing
    // over a feature would erase the very alignment the field was built to
    // capture, but that is a reason to hold those facets fixed rather than to
    // give up on the pair: the cones usually sit well away from the feature, and
    // the rest of the region still has room to relax around it.
    std::vector<GEO::index_t> freeFacets;
    for (const GEO::index_t f : region) {
        bool frozen = false;
        for (GEO::index_t c = M.facets.corners_begin(f); c < M.facets.corners_end(f); ++c) {
            GEO::index_t f2 = GEO::NO_FACET;
            if (m_sharpCorner[c]
                || GEO::NO_CORNER == reciprocalCorner(M, c, &f2) || !region.count(f2)) {
                frozen = true;
                break;
            }
        }
        if (!frozen)
            freeFacets.push_back(f);
    }
    if (freeFacets.empty()) {
        ++m_rejectedByRegion;
        return false;
    }

    // Holding a feature fixed is only worth attempting if the cones themselves are
    // still free to move; a cone pinned against a sharp edge cannot go anywhere.
    for (const GEO::index_t v : { firstVertex, secondVertex }) {
        for (const GEO::index_t f : facetsAroundVertex(v)) {
            if (std::find(freeFacets.begin(), freeFacets.end(), f) == freeFacets.end()) {
                ++m_rejectedBySharpEdge;
                return false;
            }
        }
    }

    // Every vertex whose index the smoothing could possibly disturb, so that the
    // accept test cannot be fooled by a cone pushed just outside the region.
    std::set<GEO::index_t> affectedVertices;
    for (const GEO::index_t f : region) {
        for (GEO::index_t c = M.facets.corners_begin(f); c < M.facets.corners_end(f); ++c)
            affectedVertices.insert(M.facet_corners.vertex(c));
    }
    size_t singularBefore = 0;
    for (const GEO::index_t v : affectedVertices) {
        if (0 != vertexIndex(v))
            ++singularBefore;
    }

    std::vector<double> savedAngles;
    savedAngles.reserve(freeFacets.size());
    for (const GEO::index_t f : freeFacets)
        savedAngles.push_back(m_angles[f]);

    // Smooth in the representation domain, where the 90 degree symmetry
    // disappears: a cross at angle t becomes the complex number exp(4 i t), and
    // averaging those is an ordinary linear problem.
    //
    // The magnitude has to be carried through the iteration rather than
    // renormalized each sweep. Averaging angles on the unit circle can only
    // relocate a cone, never remove one, because the winding of a unit field is
    // preserved by any local average. Letting the magnitude collapse is exactly
    // what lets a cone die: the harmonic extension of the frozen boundary sinks
    // to zero only where the boundary data genuinely forces a cone, so when the
    // enclosed index cancels out the solution is nowhere zero and comes back cone
    // free. The direction is recovered at the end as arg(z) / 4.
    for (const GEO::index_t f : region) {
        m_representationX[f] = std::cos(4.0 * m_angles[f]);
        m_representationY[f] = std::sin(4.0 * m_angles[f]);
    }

    // Update in place so information crosses the region in fewer sweeps.
    const size_t iterations = std::max<size_t>(256, 40 * radius * radius);
    for (size_t iteration = 0; iteration < iterations; ++iteration) {
        double maximumChange = 0.0;
        for (const GEO::index_t f : freeFacets) {
            double sumX = 0.0;
            double sumY = 0.0;
            double sumWeight = 0.0;
            for (GEO::index_t c = M.facets.corners_begin(f); c < M.facets.corners_end(f); ++c) {
                GEO::index_t f2 = GEO::NO_FACET;
                const GEO::index_t c2 = reciprocalCorner(M, c, &f2);
                if (GEO::NO_CORNER == c2)
                    continue;
                // m_connection[c2] carries an angle from f2 into the frame of f,
                // which in the representation domain is a rotation by 4 times it.
                const double rotation = 4.0 * m_connection[c2];
                const double cosine = std::cos(rotation);
                const double sine = std::sin(rotation);
                const double weight = GEO::length(GEO::Geom::mesh_corner_vector(M, c));
                sumX += weight * (cosine * m_representationX[f2] - sine * m_representationY[f2]);
                sumY += weight * (sine * m_representationX[f2] + cosine * m_representationY[f2]);
                sumWeight += weight;
            }
            if (sumWeight <= 0.0)
                continue;
            const double x = sumX / sumWeight;
            const double y = sumY / sumWeight;
            maximumChange = std::max(maximumChange,
                std::max(std::fabs(x - m_representationX[f]), std::fabs(y - m_representationY[f])));
            m_representationX[f] = x;
            m_representationY[f] = y;
        }
        if (maximumChange < 1e-10)
            break;
    }

    for (const GEO::index_t f : freeFacets) {
        const double x = m_representationX[f];
        const double y = m_representationY[f];
        if (0.0 == x && 0.0 == y)
            continue;
        m_angles[f] = std::atan2(y, x) / 4.0;
    }

    const std::vector<GEO::index_t> regionFacets(region.begin(), region.end());
    updateCornerOfFacets(regionFacets);

    size_t singularAfter = 0;
    for (const GEO::index_t v : affectedVertices) {
        if (0 != vertexIndex(v))
            ++singularAfter;
    }

    if (nullptr != getenv("AUTOREMESHER_DEBUG_SINGULARITY_PAIRS")) {
        std::cerr << "    pair hops=" << hops << " region=" << region.size()
                  << " free=" << freeFacets.size()
                  << " affected=" << affectedVertices.size()
                  << " cones " << singularBefore << " -> " << singularAfter << std::endl;
    }
    // Demand that the two cones aimed at are the ones that actually died. Merely
    // counting cones lets a region succeed by shuffling unrelated cones around,
    // which trades a pair the layout wanted for one it did not.
    const bool pairIsGone = 0 == vertexIndex(firstVertex) && 0 == vertexIndex(secondVertex);
    if (!pairIsGone || singularAfter + 2 > singularBefore) {
        for (size_t i = 0; i < freeFacets.size(); ++i)
            m_angles[freeFacets[i]] = savedAngles[i];
        updateCornerOfFacets(regionFacets);
        ++m_rejectedByNoImprovement;
        return false;
    }

    writeBackField(freeFacets);
    return true;
}

void SingularitySimplifier::simplify()
{
    const GEO::Mesh& M = *m_mesh;
    m_cancelledPairCount = 0;
    m_candidatePairCount = 0;
    m_rejectedBySharpEdge = 0;
    m_rejectedByRegion = 0;
    m_rejectedByNoImprovement = 0;
    if (0 == M.facets.nb()) {
        m_singularityCountBefore = 0;
        m_singularityCountAfter = 0;
        return;
    }

    buildFrames();
    buildConnection();

    const bool debug = nullptr != getenv("AUTOREMESHER_DEBUG_SINGULARITY");

    std::vector<int> indices;
    std::vector<GEO::index_t> singular = collectSingularVertices(&indices);
    m_singularityCountBefore = singular.size();
    m_singularityCountAfter = singular.size();

    if (debug) {
        // Sanity check. Indices are in quarter turns, so on a closed
        // genus g island the signed total must come to 4 * (2 - 2g): 8 for a
        // sphere, 0 for a torus. A total that is not a multiple of 4 means the
        // mismatches are being accumulated wrong.
        int signedTotal = 0;
        size_t counts[4] = { 0, 0, 0, 0 };
        for (const int index : indices) {
            ++counts[index];
            signedTotal += (1 == index) ? 1 : (3 == index ? -1 : 2);
        }
        std::cerr << "  [Singularity] cones: " << counts[1] << " of index +1/4, "
                  << counts[3] << " of index -1/4, " << counts[2]
                  << " of index +/-1/2; signed total " << signedTotal
                  << " (expected 4 * euler characteristic)" << std::endl;
    }

    for (size_t round = 0; round < m_maximumRounds; ++round) {
        if (singular.size() < 2)
            break;

        // Dual graph BFS out of every cone at once, recording which cone each
        // facet was reached from. Two cones meeting at a facet give a candidate
        // pair whose cost is the length of the corridor joining them.
        std::map<GEO::index_t, size_t> ownerOfFacet;
        std::map<GEO::index_t, size_t> distanceOfFacet;
        std::deque<GEO::index_t> queue;
        std::vector<std::pair<size_t, size_t>> touchingCones;
        for (size_t i = 0; i < singular.size(); ++i) {
            for (const GEO::index_t f : facetsAroundVertex(singular[i])) {
                auto found = ownerOfFacet.find(f);
                if (found != ownerOfFacet.end()) {
                    // Two cones sharing a facet: the loser never gets seeded, so
                    // record the pair now or the BFS would never see it.
                    touchingCones.push_back({ found->second, i });
                    continue;
                }
                ownerOfFacet[f] = i;
                distanceOfFacet[f] = 0;
                queue.push_back(f);
            }
        }
        struct Candidate {
            size_t first;
            size_t second;
            size_t hops;
        };
        std::vector<Candidate> candidates;
        std::set<std::pair<size_t, size_t>> seenPairs;
        auto consider = [&](size_t a, size_t b, size_t hops) {
            if (a == b || hops > m_maximumPairDistance)
                return;
            // Only equal and opposite cones annihilate: 1 with 3, or 2 with 2.
            if ((indices[a] + indices[b]) % 4 != 0)
                return;
            const std::pair<size_t, size_t> key(std::min(a, b), std::max(a, b));
            if (!seenPairs.insert(key).second)
                return;
            candidates.push_back({ key.first, key.second, hops });
            ++m_candidatePairCount;
        };
        for (const auto& it : touchingCones)
            consider(it.first, it.second, 0);
        while (!queue.empty()) {
            const GEO::index_t f = queue.front();
            queue.pop_front();
            const size_t d = distanceOfFacet[f];
            for (GEO::index_t c = M.facets.corners_begin(f); c < M.facets.corners_end(f); ++c) {
                GEO::index_t f2 = GEO::NO_FACET;
                if (GEO::NO_CORNER == reciprocalCorner(M, c, &f2))
                    continue;
                auto found = ownerOfFacet.find(f2);
                if (found == ownerOfFacet.end()) {
                    if (d + 1 > m_maximumPairDistance)
                        continue;
                    ownerOfFacet[f2] = ownerOfFacet[f];
                    distanceOfFacet[f2] = d + 1;
                    queue.push_back(f2);
                    continue;
                }
                consider(ownerOfFacet[f], found->second, d + distanceOfFacet[f2]);
            }
        }

        std::sort(candidates.begin(), candidates.end(),
            [](const Candidate& a, const Candidate& b) { return a.hops < b.hops; });

        std::vector<bool> used(singular.size(), false);
        size_t cancelledThisRound = 0;
        for (const Candidate& candidate : candidates) {
            if (used[candidate.first] || used[candidate.second])
                continue;
            if (!cancelPair(singular[candidate.first], singular[candidate.second], candidate.hops))
                continue;
            used[candidate.first] = true;
            used[candidate.second] = true;
            ++cancelledThisRound;
        }

        if (0 == cancelledThisRound)
            break;

        m_cancelledPairCount += cancelledThisRound;
        singular = collectSingularVertices(&indices);
        m_singularityCountAfter = singular.size();

        if (debug) {
            std::cerr << "  [Singularity] round " << (round + 1) << ": cancelled "
                      << cancelledThisRound << " pair(s), " << singular.size()
                      << " cone(s) left" << std::endl;
        }
    }

    if (debug) {
        std::cerr << "  [Singularity] " << m_candidatePairCount << " candidate pair(s); rejected "
                  << m_rejectedBySharpEdge << " on a sharp edge, "
                  << m_rejectedByRegion << " with no free facet, "
                  << m_rejectedByNoImprovement << " that did not cancel" << std::endl;
        std::cerr << "  [Singularity] " << m_singularityCountBefore << " -> "
                  << m_singularityCountAfter << " cone(s), " << m_cancelledPairCount
                  << " pair(s) cancelled" << std::endl;
    }
}

}
