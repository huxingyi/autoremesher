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
#include <AutoRemesher/MixedIntegerLeastSquares>

#include <algorithm>
#include <cmath>
#include <limits>

namespace AutoRemesher {
namespace {
    const size_t noIndex = std::numeric_limits<size_t>::max();
}

MixedIntegerLeastSquares::MixedIntegerLeastSquares(size_t size)
    : m_size(size)
    , m_pass(0)
    , m_kernelSize(0)
    , m_m0(size)
    , m_period(size, 0)
    , m_initialPeriod(size, 0)
{
    for (size_t i = 0; i < size; ++i)
        m_m0[i] = i;
}

void MixedIntegerLeastSquares::normalize(std::vector<Coeff>* row)
{
    std::sort(row->begin(), row->end(), [](const Coeff& a, const Coeff& b) { return a.index < b.index; });
    std::vector<Coeff> out;
    for (size_t i = 0; i < row->size();) {
        size_t j = i + 1;
        double a = (*row)[i].a;
        while (j < row->size() && (*row)[j].index == (*row)[i].index)
            a += (*row)[j++].a;
        if (std::fabs(a) > 1e-10)
            out.push_back({ (*row)[i].index, a });
        i = j;
    }
    row->swap(out);
}

void MixedIntegerLeastSquares::add(std::vector<Coeff>* row, size_t index, double a)
{
    if (std::fabs(a) < 1e-14)
        return;
    auto it = std::lower_bound(row->begin(), row->end(), index, [](const Coeff& c, size_t i) { return c.index < i; });
    if (it != row->end() && it->index == index) {
        it->a += a;
        if (std::fabs(it->a) < 1e-10)
            row->erase(it);
    } else
        row->insert(it, { index, a });
}

std::vector<MixedIntegerLeastSquares::Coeff> MixedIntegerLeastSquares::multiply(const std::vector<Coeff>& row, const SparseMatrix& m)
{
    std::vector<Coeff> out;
    for (const Coeff& c : row)
        if (c.index < m.rows.size())
            for (const Coeff& b : m.rows[c.index])
                out.push_back({ b.index, c.a * b.a });
    normalize(&out);
    return out;
}

void MixedIntegerLeastSquares::setVariablePeriod(size_t variable, int period)
{
    if (variable < m_period.size()) {
        m_period[variable] = std::max(0, period);
        m_initialPeriod[variable] = std::max(0, period);
    }
}

void MixedIntegerLeastSquares::addConstraint(const std::vector<std::pair<size_t, double>>& row)
{
    std::vector<Coeff> converted;
    for (const auto& c : row)
        if (c.first < m_size && c.second != 0.0)
            converted.push_back({ c.first, c.second });
    normalize(&converted);
    if (!converted.empty())
        m_recordedConstraints.push_back(converted);
}

void MixedIntegerLeastSquares::finalizeConstraints()
{
    if (m_pass != 0)
        return;
    for (size_t pass = 0; pass < 4; ++pass) {
        for (const auto& row : m_recordedConstraints)
            processConstraint(row);
        endPass(pass);
    }
    m_reducedEnergy.clear();
    m_reducedEnergy.reserve(m_energy.size());
    for (const Row& energy : m_energy) {
        Row reducedEnergy;
        reducedEnergy.rhs = energy.rhs;
        reducedEnergy.weight = energy.weight;
        for (const Coeff& coefficient : energy.coefficients)
            for (const Coeff& kernelCoefficient : line(coefficient.index))
                add(&reducedEnergy.coefficients, kernelCoefficient.index, coefficient.a * kernelCoefficient.a);
        if (!reducedEnergy.coefficients.empty())
            m_reducedEnergy.push_back(std::move(reducedEnergy));
    }
}

void MixedIntegerLeastSquares::beginConstraint() { m_pendingConstraint.clear(); }

void MixedIntegerLeastSquares::addConstraintCoefficient(size_t variable, double coefficient)
{
    if (variable < m_size && coefficient != 0.0)
        m_pendingConstraint.push_back({ variable, coefficient });
}

bool MixedIntegerLeastSquares::endConstraint()
{
    normalize(&m_pendingConstraint);
    return processConstraint(m_pendingConstraint);
}

std::vector<MixedIntegerLeastSquares::Coeff> MixedIntegerLeastSquares::throughM0(const std::vector<Coeff>& r) const
{
    std::vector<Coeff> out;
    for (const Coeff& c : r)
        if (m_m0[c.index] != noIndex)
            out.push_back({ m_m0[c.index], c.a });
    normalize(&out);
    return out;
}

std::vector<MixedIntegerLeastSquares::Coeff> MixedIntegerLeastSquares::throughM1(const std::vector<Coeff>& r) const
{
    std::vector<Coeff> out;
    for (const Coeff& c : r)
        out.push_back({ m_m1[c.index].index, c.a * m_m1[c.index].a });
    normalize(&out);
    return out;
}

void MixedIntegerLeastSquares::finalizeM0()
{
    size_t n = 0;
    for (size_t i = 0; i < m_m0.size(); ++i)
        if (m_m0[i] != noIndex) {
            m_period[n] = m_period[i];
            m_m0[i] = n++;
        }
    m_period.resize(n);
    m_m1.resize(n);
    for (size_t i = 0; i < n; ++i)
        m_m1[i] = { i, 1.0 };
}

void MixedIntegerLeastSquares::finalizeM1()
{
    for (size_t i = 0; i < m_m1.size(); ++i)
        while (m_m1[i].index != m_m1[m_m1[i].index].index) {
            m_m1[i].a *= m_m1[m_m1[i].index].a;
            m_m1[i].index = m_m1[m_m1[i].index].index;
        }
    size_t n = 0;
    std::vector<size_t> group(m_m1.size(), noIndex);
    for (size_t i = 0; i < m_m1.size(); ++i)
        if (m_m1[i].index == i)
            group[i] = n++;
    std::vector<int> period(n, 0);
    for (size_t i = 0; i < m_m1.size(); ++i) {
        size_t root = m_m1[i].index;
        size_t g = group[root];
        period[g] = std::max(period[g], m_period[root]);
        m_m1[i].index = g;
    }
    m_period.swap(period);
    m_m2.rows.resize(n);
    m_m2t.rows.resize(n);
    for (size_t i = 0; i < n; ++i) {
        m_m2.rows[i].push_back({ i, 1 });
        m_m2t.rows[i].push_back({ i, 1 });
    }
}

void MixedIntegerLeastSquares::finalizeM2()
{
    m_kernelSize = m_m2.rows.size();
    m_period.resize(m_kernelSize);
}

void MixedIntegerLeastSquares::applyRemoveColumn(const std::vector<Coeff>& row, size_t remove)
{
    const Coeff pivot = row[remove];
    std::vector<Coeff> replacement;
    for (size_t i = 0; i < row.size(); ++i)
        if (i != remove)
            replacement.push_back({ row[i].index, -row[i].a / pivot.a });
    const size_t zero = pivot.index;
    std::vector<Coeff> difference = replacement;
    add(&difference, zero, -1.0);
    const std::vector<Coeff> column = m_m2t.rows[zero];
    for (const Coeff& d : difference)
        for (const Coeff& source : column) {
            add(&m_m2t.rows[d.index], source.index, d.a * source.a);
            add(&m_m2.rows[source.index], d.index, d.a * source.a);
        }
}

bool MixedIntegerLeastSquares::processConstraint(const std::vector<Coeff>& row)
{
    if (m_pass == 0) {
        if (row.size() == 1 && row[0].a != 0.0)
            m_m0[row[0].index] = noIndex;
        return true;
    }
    const std::vector<Coeff> r0 = throughM0(row);
    const std::vector<Coeff> r1 = throughM1(r0);
    if (r1.empty())
        return true;
    if (m_pass == 1) {
        if (r1.size() == 2 && std::fabs(std::fabs(r1[0].a) - 1.0) < 1e-10 && std::fabs(std::fabs(r1[1].a) - 1.0) < 1e-10) {
            const Coeff a = { r1[0].index, 1.0 };
            const Coeff b = { r1[1].index, -r1[0].a / r1[1].a };
            Coeff roots[2] = { a, b };
            for (Coeff& x : roots)
                while (x.index != m_m1[x.index].index) {
                    x.a *= m_m1[x.index].a;
                    x.index = m_m1[x.index].index;
                }
            if (m_period[roots[0].index] < m_period[roots[1].index])
                m_m1[roots[0].index] = { roots[1].index, roots[1].a * roots[0].a };
            else
                m_m1[roots[1].index] = { roots[0].index, roots[1].a * roots[0].a };
        }
        return true;
    }
    const std::vector<Coeff> r2 = multiply(r1, m_m2);
    if (r2.empty())
        return true;
    if (m_pass == 2) {
        size_t remove = 0;
        for (size_t i = 1; i < r2.size(); ++i)
            if (double(m_period[r2[i].index]) * std::fabs(r2[i].a) < double(m_period[r2[remove].index]) * std::fabs(r2[remove].a))
                remove = i;
        applyRemoveColumn(r2, remove);
        return true;
    }
    return false;
}

void MixedIntegerLeastSquares::endPass(size_t pass)
{
    if (pass != m_pass)
        return;
    if (m_pass == 0)
        finalizeM0();
    else if (m_pass == 1)
        finalizeM1();
    else if (m_pass == 2)
        finalizeM2();
    ++m_pass;
}

void MixedIntegerLeastSquares::clearEnergy() { m_energy.clear(); }

void MixedIntegerLeastSquares::addEnergy(const std::vector<std::pair<size_t, double>>& c, double rhs, double weight)
{
    Row row;
    row.rhs = rhs;
    row.weight = weight;
    for (const auto& x : c)
        if (x.first < m_size && x.second != 0)
            row.coefficients.push_back({ x.first, x.second });
    normalize(&row.coefficients);
    if (!row.coefficients.empty() && weight > 0)
        m_energy.push_back(row);
}

bool MixedIntegerLeastSquares::solveIteration()
{
    if (m_pass < 3 || m_kernelSize == 0)
        return false;
    if (m_values.empty()) {
        m_values.assign(m_kernelSize, 0.0);
        m_fixed.assign(m_kernelSize, false);
    } else {
        double threshold = 1e20;
        for (size_t i = 0; i < m_kernelSize; ++i)
            if (!m_fixed[i] && m_period[i] > 0) {
                double d = std::fabs(m_values[i] / m_period[i] - std::round(m_values[i] / m_period[i]));
                threshold = std::max(d + .001, 1.0);
                break;
            }
        for (size_t i = 0; i < m_kernelSize; ++i)
            if (!m_fixed[i] && m_period[i] > 0) {
                double d = std::fabs(m_values[i] / m_period[i] - std::round(m_values[i] / m_period[i]));
                if (d < threshold)
                    m_fixed[i] = true;
            }
    }
    ConstrainedLeastSquares system(m_kernelSize);
    for (const Row& energy : m_reducedEnergy) {
        std::vector<std::pair<size_t, double>> coefficients;
        coefficients.reserve(energy.coefficients.size());
        for (const Coeff& coefficient : energy.coefficients)
            coefficients.push_back({ coefficient.index, coefficient.a });
        system.addEnergy(coefficients, energy.rhs, energy.weight);
    }
    for (size_t i = 0; i < m_kernelSize; ++i)
        if (m_fixed[i])
            system.addConstraint({ { i, 1.0 } }, double(m_period[i]) * std::round(m_values[i] / double(m_period[i])));
    return system.solve(&m_values);
}

bool MixedIntegerLeastSquares::converged() const
{
    if (m_values.empty())
        return false;
    for (size_t i = 0; i < m_kernelSize; ++i)
        if (m_period[i] > 0 && !m_fixed[i])
            return false;
    return true;
}

std::vector<MixedIntegerLeastSquares::Coeff> MixedIntegerLeastSquares::line(size_t i) const
{
    std::vector<Coeff> r = { { i, 1.0 } };
    r = throughM0(r);
    if (m_m1.empty())
        return r;
    r = throughM1(r);
    if (m_m2.rows.empty())
        return r;
    return multiply(r, m_m2);
}

double MixedIntegerLeastSquares::value(size_t i) const
{
    double v = 0;
    if (i >= m_size)
        return v;
    for (const Coeff& x : line(i))
        if (x.index < m_values.size())
            v += x.a * m_values[x.index];
    return v;
}

size_t MixedIntegerLeastSquares::kernelSize() const { return m_kernelSize; }

size_t MixedIntegerLeastSquares::integerKernelVariableCount() const
{
    size_t count = 0;
    for (size_t i = 0; i < m_kernelSize; ++i)
        if (m_period[i] > 0)
            ++count;
    return count;
}

size_t MixedIntegerLeastSquares::originalIntegerVariableCount() const
{
    size_t count = 0;
    for (int p : m_initialPeriod)
        if (p > 0)
            ++count;
    return count;
}
}
