#pragma once

#include <valarray>
#include <complex>

template<typename T>
T make_dummy_component() {
    return T();
}

template<> std::valarray<double> make_dummy_component();

template<typename Vec>
Vec make_dummy_vector() {
    return Vec(make_dummy_component<typename Vec::value_type>());
}

template<typename Scalar>
double observe_dummy_component(const Scalar& _s) {
    return _s;
}

template<typename T>
double observe_dummy_component(const std::valarray<T>& _s) {
    return _s.sum();
}

template<typename T>
double observe_dummy_component(const std::complex<T>& _s) {
    return _s.real() + _s.imag();
}

template<typename Vec>
double observe_dummy_vector(const Vec& _vec) {
    double result = 0.0;
    for (int dim = 0; dim < _vec.dim(); ++dim) {
        result += observe_dummy_component(_vec[dim]);
    }
    return result;
}
