#ifndef UNITTESTS_COMMON_DUMMYTRAITS
#define UNITTESTS_COMMON_DUMMYTRAITS
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Utils/color_cast.hh>
#include <array>

namespace Custom {

/// A Vector class with the absolute minimum of built-in methods to test the
/// interface expected from Vectors used in Traits
template <int DIM> class Vec {
  public:
    // Constructor with DIM components
	Vec(float x) : data({ x }) {}
	Vec(float x, float y) : data({ x, y }) {}
	Vec(float x, float y, float z) : data({{ x, y, z }}) {}
	Vec(float x, float y, float z, float w) : data({ x, y, z, w }) {}
	  
    Vec() = default;
    Vec(Vec<DIM> const &) = default;

    float &operator[](int i) { return data[i]; }
    float operator[](int i) const { return data[i]; }

  private:
    std::array<float, DIM> data;
};

template <int DIM> bool operator==(Vec<DIM> const &lhs, Vec<DIM> const &rhs) {
    for (int i = 0; i < DIM; i++)
        if (lhs[i] != rhs[i]) return false;
    return true;
}

template <int DIM>
Vec<DIM> operator+(Vec<DIM> const &lhs, Vec<DIM> const &rhs) {
    Vec<DIM> result;
    for (int i = 0; i < DIM; i++)
        result[i] = lhs[i] + rhs[i];
    return result;
}

template <int DIM>
Vec<DIM> operator-(Vec<DIM> const &lhs, Vec<DIM> const &rhs) {
    Vec<DIM> result;
    for (int i = 0; i < DIM; i++)
        result[i] = lhs[i] - rhs[i];
    return result;
}

template <int DIM> Vec<DIM> operator*(Vec<DIM> const &lhs, float rhs) {
    Vec<DIM> result;
    for (int i = 0; i < DIM; i++)
        result[i] = lhs[i] * rhs;
    return result;
}

template <int DIM> Vec<DIM> operator*(float lhs, Vec<DIM> const &rhs) {
    return rhs * lhs;
}

template <int DIM> Vec<DIM> operator/(Vec<DIM> const &lhs, float rhs) {
    Vec<DIM> result;
    for (int i = 0; i < DIM; i++)
        result[i] = lhs[i] / rhs;
    return result;
}

template <int DIM> Vec<DIM> &operator+=(Vec<DIM> &lhs, Vec<DIM> const &rhs) {
    return lhs = lhs + rhs;
}
template <int DIM> Vec<DIM> &operator-=(Vec<DIM> &lhs, Vec<DIM> const &rhs) {
    return lhs = lhs - rhs;
}
template <int DIM> Vec<DIM> &operator*=(Vec<DIM> &lhs, float rhs) {
    return lhs = lhs * rhs;
}
template <int DIM> Vec<DIM> &operator/=(Vec<DIM> &lhs, float rhs) {
    return lhs = lhs / rhs;
}

template <int DIM> float norm(Vec<DIM> const &v) {
    float sum = 0.0f;
    for (int i = 0; i < DIM; i++)
        sum += v[i] * v[i];
    return std::sqrt(sum);
}

template <int DIM> Vec<DIM> &normalize(Vec<DIM> &v) { return v /= norm(v); }
template <int DIM> Vec<DIM> &vectorize(Vec<DIM> &v, float val) {
    for (int i = 0; i < DIM; i++)
        v[i] = val;
    return v;
}

template <int DIM> Vec<DIM> &minimize(Vec<DIM> &v1, Vec<DIM> const &v2) {
    for (int i = 0; i < DIM; i++)
        v1[i] = std::min(v1[i], v2[i]);
    return v1;
}

template <int DIM> Vec<DIM> &maximize(Vec<DIM> &v1, Vec<DIM> const &v2) {
    for (int i = 0; i < DIM; i++)
        v1[i] = std::max(v1[i], v2[i]);
    return v1;
}

template <int DIM> float dot(Vec<DIM> const &v1, Vec<DIM> const &v2) {
    float sum = 0.f;
    for (int i = 0; i < DIM; i++)
        sum += v1[i] * v2[i];
    return sum;
}

inline Vec<3> cross(Vec<3> const &v1, Vec<3> const &v2) {
    return {v1[1] * v2[2] - v1[2] * v2[1], //
            v1[2] * v2[0] - v1[0] * v2[2], //
            v1[0] * v2[1] - v1[1] * v2[0]};
}
}

namespace OpenMesh {
template <int DIM> struct vector_traits<Custom::Vec<DIM>> {
    using vector_type = Custom::Vec<DIM>;
    using value_type = float;
    static const size_t size_ = DIM;
    static size_t size() { return size_; }
};
}

struct CustomTraits : public OpenMesh::DefaultTraits {
    typedef Custom::Vec<3> Point;
    typedef Custom::Vec<3> Normal;

    typedef Custom::Vec<2> TexCoord2D;
    typedef Custom::Vec<3> TexCoord3D;
};

#endif // UNITTESTS_COMMON_DUMMYTRAITS
