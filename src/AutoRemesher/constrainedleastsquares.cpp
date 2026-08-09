/*
 *  Copyright (c) 2026 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved.
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
#include <AutoRemesher/ConstrainedLeastSquares>

#include <Eigen/Sparse>
#include <Eigen/SparseQR>
#include <cmath>

namespace AutoRemesher {

ConstrainedLeastSquares::ConstrainedLeastSquares(size_t variableCount)
    : m_variableCount(variableCount)
{
}

void ConstrainedLeastSquares::addEnergy(const std::vector<std::pair<size_t, double>>& coefficients,
    double rightHandSide, double weight)
{
    if (!coefficients.empty() && weight > 0.0)
        m_energyEquations.push_back({ coefficients, rightHandSide, weight });
}

void ConstrainedLeastSquares::addConstraint(const std::vector<std::pair<size_t, double>>& coefficients,
    double rightHandSide)
{
    if (!coefficients.empty())
        m_constraintEquations.push_back({ coefficients, rightHandSide, 1.0 });
}

bool ConstrainedLeastSquares::solve(std::vector<double>* solution) const
{
    const Eigen::Index variableCount = static_cast<Eigen::Index>(m_variableCount);
    const Eigen::Index originalConstraintCount = static_cast<Eigen::Index>(m_constraintEquations.size());
    if (variableCount == 0)
        return false;
    Eigen::SparseMatrix<double> constraintMatrix(originalConstraintCount, variableCount);
    std::vector<Eigen::Triplet<double>> constraintEntries;
    for (Eigen::Index constraintIndex = 0; constraintIndex < originalConstraintCount; ++constraintIndex) {
        for (const auto& coefficient : m_constraintEquations[static_cast<size_t>(constraintIndex)].coefficients) {
            if (coefficient.first >= m_variableCount)
                return false;
            constraintEntries.emplace_back(constraintIndex, static_cast<Eigen::Index>(coefficient.first), coefficient.second);
        }
    }
    constraintMatrix.setFromTriplets(constraintEntries.begin(), constraintEntries.end());
    Eigen::SparseQR<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int>> constraintQR;
    constraintQR.compute(constraintMatrix.transpose());
    if (constraintQR.info() != Eigen::Success)
        return false;
    const Eigen::Index independentConstraintCount = constraintQR.rank();
    std::vector<Eigen::Index> selectedConstraints;
    selectedConstraints.reserve(static_cast<size_t>(independentConstraintCount));
    const auto permutation = constraintQR.colsPermutation().indices();
    for (Eigen::Index i = 0; i < independentConstraintCount; ++i)
        selectedConstraints.push_back(permutation[i]);

    Eigen::SparseMatrix<double> normal(variableCount + independentConstraintCount, variableCount + independentConstraintCount);
    std::vector<Eigen::Triplet<double>> entries;
    Eigen::VectorXd rightHandSide = Eigen::VectorXd::Zero(variableCount + independentConstraintCount);
    for (const LinearEquation& equation : m_energyEquations) {
        for (const auto& coefficient : equation.coefficients) {
            if (coefficient.first >= m_variableCount)
                return false;
            rightHandSide[static_cast<Eigen::Index>(coefficient.first)] += equation.weight * coefficient.second * equation.rightHandSide;
            for (const auto& otherCoefficient : equation.coefficients) {
                if (otherCoefficient.first >= m_variableCount)
                    return false;
                entries.emplace_back(static_cast<Eigen::Index>(coefficient.first), static_cast<Eigen::Index>(otherCoefficient.first),
                    equation.weight * coefficient.second * otherCoefficient.second);
            }
        }
    }
    for (Eigen::Index constraintIndex = 0; constraintIndex < independentConstraintCount; ++constraintIndex) {
        const LinearEquation& equation = m_constraintEquations[static_cast<size_t>(selectedConstraints[constraintIndex])];
        rightHandSide[variableCount + constraintIndex] = equation.rightHandSide;
        for (const auto& coefficient : equation.coefficients) {
            if (coefficient.first >= m_variableCount)
                return false;
            entries.emplace_back(static_cast<Eigen::Index>(coefficient.first), variableCount + constraintIndex, coefficient.second);
            entries.emplace_back(variableCount + constraintIndex, static_cast<Eigen::Index>(coefficient.first), coefficient.second);
        }
    }
    for (Eigen::Index i = 0; i < variableCount; ++i)
        entries.emplace_back(i, i, 1e-10);
    normal.setFromTriplets(entries.begin(), entries.end());
    Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
    solver.compute(normal);
    if (solver.info() != Eigen::Success)
        return false;
    const Eigen::VectorXd solutionVector = solver.solve(rightHandSide);
    if (solver.info() != Eigen::Success || !solutionVector.allFinite())
        return false;
    solution->resize(m_variableCount);
    for (size_t i = 0; i < m_variableCount; ++i)
        (*solution)[i] = solutionVector[static_cast<Eigen::Index>(i)];
    return true;
}

}
