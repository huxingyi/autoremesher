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
#ifndef AUTO_REMESHER_PARAMETERIZER_H
#define AUTO_REMESHER_PARAMETERIZER_H
#include <AutoRemesher/HalfEdge>
#include <igl/avg_edge_length.h>
#include <igl/barycenter.h>
#include <igl/comb_cross_field.h>
#include <igl/comb_frame_field.h>
#include <igl/compute_frame_field_bisectors.h>
#include <igl/cross_field_mismatch.h>
#include <igl/cut_mesh_from_singularities.h>
#include <igl/find_cross_field_singularities.h>
#include <igl/local_basis.h>
#include <igl/rotate_vectors.h>
#include <igl/copyleft/comiso/miq.h>
#include <igl/copyleft/comiso/nrosy.h>
#include <igl/copyleft/comiso/frame_field.h>
#include <igl/readOBJ.h>
#include <igl/writeOBJ.h>
#include <igl/readDMAT.h>
#include <igl/frame_field_deformer.h>
#include <igl/frame_to_cross_field.h>
#include <igl/PI.h>
#include <igl/principal_curvature.h>

namespace AutoRemesher
{
    
class Parameterizer
{
public:
    struct Parameters
    {
        double gradientSize;
        bool constrainOnFlatArea;
    };
    
    Parameterizer(HalfEdge::Mesh *mesh, const Parameters &parameters);
    std::pair<double, double> Parameterizer::calculateLimitRelativeHeight(const std::pair<double, double> &limitRelativeHeight);
    void prepareConstraints(const std::pair<double, double> &limitRelativeHeight,
        Eigen::VectorXi **b,
        Eigen::MatrixXd **bc1,
        Eigen::MatrixXd **bc2);
    bool miq(size_t *singularityCount, 
        const Eigen::VectorXi &b,
        const Eigen::MatrixXd &bc1,
        const Eigen::MatrixXd &bc2,
        bool calculateSingularityOnly);
        
    const std::vector<size_t> &getVertexValences()
    {
        return m_vertexValences;
    }
private:
    Eigen::MatrixXd *m_V = nullptr;
    Eigen::MatrixXi *m_F = nullptr;
    Eigen::MatrixXd *m_PD1 = nullptr;
    Eigen::MatrixXd *m_PD2 = nullptr;
    HalfEdge::Mesh *m_mesh = nullptr;
    Parameters m_parameters;
    std::vector<size_t> m_vertexValences;
};
    
}

#endif
