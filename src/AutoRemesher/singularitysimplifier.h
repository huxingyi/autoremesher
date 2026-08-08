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
#ifndef AUTO_REMESHER_SINGULARITY_SIMPLIFIER_H
#define AUTO_REMESHER_SINGULARITY_SIMPLIFIER_H
#include <cstddef>
#include <geogram/mesh/mesh.h>
#include <vector>

namespace AutoRemesher {

// Cancels pairs of opposite cross field singularities before the surface is
// parameterized. A 4-RoSy field extracted from curvature carries far more cones
// than the quad layout needs: most of them come in nearby +1/4 / -1/4 pairs that
// contribute nothing but a scattering of valence 3 and valence 5 vertices in the
// extracted mesh. Such a pair can be removed outright, so for
// each pair this walks the dual graph to find a small region containing both,
// freezes its boundary, and re-smooths the field inside. A region whose boundary
// holonomy is trivial admits a cone free interpolation, and the smoothing finds
// it; when it does not, the field is rolled back and the pair is left alone.
//
// The field is modified in place. Only faces strictly inside an accepted region
// change, so alignment everywhere else (in particular along sharp features, which
// are never enclosed in a region) is preserved exactly.
class SingularitySimplifier {
public:
    SingularitySimplifier(GEO::Mesh* mesh, GEO::Attribute<GEO::vec3>* field)
        : m_mesh(mesh)
        , m_field(field)
    {
    }

    // Longest dual graph distance, in face hops, over which two opposite
    // singularities are still considered a cancellable pair. Larger values remove
    // more cones but distort the field over wider regions.
    void setMaximumPairDistance(size_t faceHops)
    {
        m_maximumPairDistance = faceHops;
    }

    // Dihedral angle above which an edge counts as a sharp feature. Regions
    // touching a sharp edge are rejected, so feature aligned loops survive.
    void setSharpEdgeDegrees(double degrees)
    {
        m_sharpEdgeDegrees = degrees;
    }

    // Cancelling a pair changes the field, which can bring a further pair within
    // range, so the sweep is repeated until it stops finding anything.
    void setMaximumRounds(size_t rounds)
    {
        m_maximumRounds = rounds;
    }

    void simplify();

    size_t singularityCountBefore() const
    {
        return m_singularityCountBefore;
    }

    size_t singularityCountAfter() const
    {
        return m_singularityCountAfter;
    }

    size_t cancelledPairCount() const
    {
        return m_cancelledPairCount;
    }

private:
    GEO::Mesh* m_mesh = nullptr;
    GEO::Attribute<GEO::vec3>* m_field = nullptr;
    size_t m_maximumPairDistance = 6;
    size_t m_maximumRounds = 4;
    // Face hops of slack added around the corridor joining a pair. The free
    // facets are what the solve actually has to work with, so a close pair needs
    // just as much room as a distant one: the margin is absolute, not a fraction
    // of the separation.
    size_t m_regionMargin = 6;
    double m_sharpEdgeDegrees = 90.0;
    size_t m_singularityCountBefore = 0;
    size_t m_singularityCountAfter = 0;
    size_t m_cancelledPairCount = 0;
    size_t m_candidatePairCount = 0;
    size_t m_rejectedBySharpEdge = 0;
    size_t m_rejectedByRegion = 0;
    size_t m_rejectedByNoImprovement = 0;

    // Per face orthonormal tangent frame, m_frameU cross m_frameV is the normal.
    std::vector<GEO::vec3> m_frameU;
    std::vector<GEO::vec3> m_frameV;
    // Field direction of each face as an angle in that face's own frame.
    std::vector<double> m_angles;
    // Rotation carrying an angle in facet(c) into the frame of the facet across
    // corner c, and the number of quarter turns the field jumps by there.
    std::vector<double> m_connection;
    std::vector<int> m_mismatch;
    std::vector<bool> m_sharpCorner;
    // Any one corner sitting on each vertex, so a one ring walk can start in
    // constant time instead of scanning the corner array.
    std::vector<GEO::index_t> m_cornerOfVertex;
    // Scratch for the representation domain solve, kept at full facet size so a
    // region can be relaxed without reallocating per pair.
    std::vector<double> m_representationX;
    std::vector<double> m_representationY;

    void buildFrames();
    void buildConnection();
    void updateCornerOfFacets(const std::vector<GEO::index_t>& facets);
    int vertexIndex(GEO::index_t v) const;
    std::vector<GEO::index_t> facetsAroundVertex(GEO::index_t v) const;
    std::vector<GEO::index_t> collectSingularVertices(std::vector<int>* indices) const;
    bool cancelPair(GEO::index_t firstVertex, GEO::index_t secondVertex, size_t hops);
    void writeBackField(const std::vector<GEO::index_t>& facets);
};

}

#endif
