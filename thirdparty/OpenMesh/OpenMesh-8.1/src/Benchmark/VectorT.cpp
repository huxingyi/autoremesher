/*
 * VectorT.cpp
 *
 *  Created on: Nov 19, 2015
 *      Author: ebke
 */

#ifndef BMPOSTFIX
#error "Do not compile directly."
#endif

#include <type_traits>

#define ASSEMBLE_(POSTFIX, PREFIX) PREFIX ## POSTFIX
#define ASSEMBLE(POSTFIX, PREFIX) ASSEMBLE_(POSTFIX, PREFIX)
#define MYBENCHMARK(NAME) BENCHMARK(NAME)
#define MYBENCHMARK_TEMPLATE(NAME, TYPE) BENCHMARK_TEMPLATE(NAME, TYPE)

template<class Vec>
static inline
typename std::enable_if<Vec::size_ == 3, Vec>::type
testVec() {
    return Vec(1.1, 1.2, 1.3);
}

template<class Vec>
static inline
typename std::enable_if<Vec::size_ == 4, Vec>::type
testVec() {
    return Vec(1.1, 1.2, 1.3, 1.4);
}

template<class Vec>
static void ASSEMBLE(BMPOSTFIX, Vec_add_compare)(benchmark::State& state) {
    Vec v1(0.0);
    Vec v2(1000.0);
    while (state.KeepRunning()) {
        v1 += testVec<Vec>();
        v2 -= testVec<Vec>();
        if (v1 == v2) {
            v1 -= v2;
            v2 += v1;
        }
    }
    // Just so nothing gets optimized away.
    static double dummy;
    dummy = v1.norm() + v2.norm();
    static_cast<void>(dummy);
}

MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_add_compare), OpenMesh::Vec3d);
MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_add_compare), OpenMesh::Vec3f);
MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_add_compare), OpenMesh::Vec4d);
MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_add_compare), OpenMesh::Vec4f);

template<class Vec>
static void ASSEMBLE(BMPOSTFIX, Vec_cross_product)(benchmark::State& state) {
    Vec v1(0.0);
    Vec v2(1000.0);
    while (state.KeepRunning()) {
        v1 += testVec<Vec>();
        v2 -= testVec<Vec>();
        v1 = (v1 % v2);
    }
    // Just so nothing gets optimized away.
    static double dummy;
    dummy = v1.norm() + v2.norm();
    static_cast<void>(dummy);
}

MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_cross_product), OpenMesh::Vec3d);
MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_cross_product), OpenMesh::Vec3f);

template<class Vec>
static void ASSEMBLE(BMPOSTFIX, Vec_scalar_product)(benchmark::State& state) {
    Vec v1(0.0);
    Vec v2(1000.0);
    double acc = 0;
    while (state.KeepRunning()) {
        v1 += testVec<Vec>();
        v2 -= testVec<Vec>();
        acc += (v1 | v2);
    }
    // Otherwise GCC will optimize everything away.
    static double dummy;
    dummy = acc;
    static_cast<void>(dummy);
}

MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_scalar_product), OpenMesh::Vec3d);
MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_scalar_product), OpenMesh::Vec3f);
MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_scalar_product), OpenMesh::Vec4d);
MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_scalar_product), OpenMesh::Vec4f);

template<class Vec>
static void ASSEMBLE(BMPOSTFIX, Vec_norm)(benchmark::State& state) {
    Vec v1(0.0);
    double acc = 0;
    while (state.KeepRunning()) {
        v1 += testVec<Vec>();
        acc += v1.norm();
    }
    // Otherwise GCC will optimize everything away.
    static double dummy;
    dummy = acc;
    static_cast<void>(dummy);
}

MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_norm), OpenMesh::Vec3d);
MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_norm), OpenMesh::Vec3f);
MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_norm), OpenMesh::Vec4d);
MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_norm), OpenMesh::Vec4f);

template<class Vec>
static void ASSEMBLE(BMPOSTFIX, Vec_times_scalar)(benchmark::State& state) {
    Vec v1(0.0);
    while (state.KeepRunning()) {
        v1 += testVec<Vec>();
        v1 *= static_cast<decltype(v1.norm())>(1.0)/v1[0];
        v1 *= v1[1];
    }
    // Otherwise GCC will optimize everything away.
    static double dummy;
    dummy = v1.norm();
    static_cast<void>(dummy);
}

MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_times_scalar), OpenMesh::Vec3d);
MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_times_scalar), OpenMesh::Vec3f);
MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_times_scalar), OpenMesh::Vec4d);
MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_times_scalar), OpenMesh::Vec4f);

#include "VectorT_dummy_data.hpp"

template<class Vec>
static void ASSEMBLE(BMPOSTFIX, Vec_add_in_place)(benchmark::State& state) {
    auto v = make_dummy_vector<Vec>();
    while (state.KeepRunning()) {
        v += v;
    }
    // Otherwise GCC will optimize everything away.
    static double dummy = observe_dummy_vector(v);
    static_cast<void>(dummy);
}

template<class Vec>
static void ASSEMBLE(BMPOSTFIX, Vec_add_temporary)(benchmark::State& state) {
    auto v = make_dummy_vector<Vec>();
    while (state.KeepRunning()) {
        v = v + v;
    }
    // Otherwise GCC will optimize everything away.
    static double dummy = observe_dummy_vector(v);
    static_cast<void>(dummy);
}

typedef OpenMesh::VectorT<std::valarray<double>, 3> Vec3VAd;
typedef OpenMesh::VectorT<std::complex<double>, 3> Vec3Cd;

MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_add_in_place), Vec3VAd);
MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_add_temporary), Vec3VAd);
MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_add_in_place), Vec3Cd);
MYBENCHMARK_TEMPLATE (ASSEMBLE(BMPOSTFIX, Vec_add_temporary), Vec3Cd);
