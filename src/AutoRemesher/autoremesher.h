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
#ifndef AUTO_REMESHER_AUTO_REMESHER_H
#define AUTO_REMESHER_AUTO_REMESHER_H
#include <vector>
#include <cstddef>
#include <map>
#include <AutoRemesher/Vector3>
#include <AutoRemesher/QuadRemesher>

namespace AutoRemesher
{
    
enum class ConstrainedArea
{
    ConstrainedAreaBumpy = 0,
    ConstrainedAreaFlat
};
    
class IsotropicRemesher;

class AutoRemesher
{
public:
    AutoRemesher(const std::vector<Vector3> &vertices,
            const std::vector<std::vector<size_t>> &triangles) :
        m_vertices(vertices),
        m_triangles(triangles)
    {
    }
    
    void setGradientSize(double gradientSize)
    {
        m_gradientSize = gradientSize;
    }
    
    void setConstrainedArea(ConstrainedArea constrainedArea)
    {
        m_constrainedArea = constrainedArea;
    }
    
    const std::vector<Vector3> &remeshedVertices()
    {
        return m_remeshedVertices;
    }
    
    const std::vector<std::vector<size_t>> &remeshedQuads()
    {
        return m_remeshedQuads;
    }
    
    bool remesh();
    static void calculateNormalizedFactors(const std::vector<Vector3> &vertices, 
        Vector3 *origin, 
        double *maxLength);
    static IsotropicRemesher *createIsotropicRemesh(std::vector<Vector3> sourceVertices,
        std::vector<std::vector<size_t>> sourceTriangles,
        double sharpEdgeDegrees, 
        size_t targetVertexCount,
        double *targetEdgeLength);
    
    static const double m_defaultTargetEdgeLength;
    static const std::pair<double, double> m_defaultConstraintRatio;
    static const size_t m_defaultMaxSingularityCount;
    static const size_t m_defaultMaxVertexCount;
    static const double m_defaultSharpEdgeDegrees;
    static const double m_defaultGradientSize;
private:
    std::vector<Vector3> m_vertices;
    std::vector<std::vector<size_t>> m_triangles;
    std::vector<Vector3> m_remeshedVertices;
    std::vector<std::vector<size_t>> m_remeshedQuads;
    double m_gradientSize = m_defaultGradientSize;
    ConstrainedArea m_constrainedArea = ConstrainedArea::ConstrainedAreaBumpy;
    
    void buildEdgeToFaceMap(const std::vector<std::vector<size_t>> &triangles, 
        std::map<std::pair<size_t, size_t>, size_t> &edgeToFaceMap);
    void splitToIslands(const std::vector<std::vector<size_t>> &triangles, 
        std::vector<std::vector<std::vector<size_t>>> &islands);
};
    
}

#endif
