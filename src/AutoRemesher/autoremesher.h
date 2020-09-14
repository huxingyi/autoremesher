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

namespace AutoRemesher
{
    
class IsotropicRemesher;

enum class ModelType
{
    Organic,
    HardSurface
};

class AutoRemesher
{
public:
    AutoRemesher(const std::vector<Vector3> &vertices,
            const std::vector<std::vector<size_t>> &triangles) :
        m_vertices(vertices),
        m_triangles(triangles)
    {
    }
    
    void setTargetTriangleCount(size_t targetTriangleCount)
    {
        m_targetTriangleCount = targetTriangleCount;
    }
    
    void setScaling(double scaling)
    {
        m_scaling = scaling;
    }
    
    void setModelType(ModelType modelType)
    {
        m_modelType = modelType;
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
    
    static const double m_defaultSharpEdgeDegrees;
    static const double m_defaultScaling;
    static float m_stanfordBunnyArea;
private:
    std::vector<Vector3> m_vertices;
    std::vector<std::vector<size_t>> m_triangles;
    std::vector<Vector3> m_remeshedVertices;
    std::vector<std::vector<size_t>> m_remeshedQuads;
    double m_scaling = m_defaultScaling;
    size_t m_targetTriangleCount = 0;
    double m_voxelSize = 0.0;
    ModelType m_modelType = ModelType::Organic;
    
    static double calculateAverageEdgeLength(const std::vector<Vector3> &vertices,
        const std::vector<std::vector<size_t>> &faces);
    void initializeVoxelSize();
    void preprocess();
};
    
}

#endif
