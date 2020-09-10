// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file Tuple.h
/// @author Ben Kwa

#ifndef OPENVDB_MATH_TUPLE_HAS_BEEN_INCLUDED
#define OPENVDB_MATH_TUPLE_HAS_BEEN_INCLUDED

#include "Math.h"
#include <cmath>
#include <sstream>
#include <string>


namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace math {

/// @brief Dummy class for tag dispatch of conversion constructors
struct Conversion {};


/// @class Tuple "Tuple.h"
/// A base class for homogenous tuple types
template<int SIZE, typename T>
class Tuple {
public:
    using value_type = T;
    using ValueType = T;

    static const int size = SIZE;

    /// @brief Default ctor.  Does nothing.
    /// @details This is required because declaring a copy (or other) constructor
    /// prevents the compiler from synthesizing a default constructor.
    Tuple() {}

    /// Copy constructor.  Used when the class signature matches exactly.
    Tuple(Tuple const& src) {
        for (int i = 0; i < SIZE; ++i) {
            mm[i] = src.mm[i];
        }
    }

    /// @brief Assignment operator
    /// @details This is required because declaring a copy (or other) constructor
    /// prevents the compiler from synthesizing a default assignment operator.
    Tuple& operator=(Tuple const& src) {
        if (&src != this) {
            for (int i = 0; i < SIZE; ++i) {
                mm[i] = src.mm[i];
            }
        }
        return *this;
    }

    /// @brief Conversion constructor.
    /// @details Tuples with different value types and different sizes can be
    /// interconverted using this member.  Converting from a larger tuple
    /// results in truncation; converting from a smaller tuple results in
    /// the extra data members being zeroed out.  This function assumes that
    /// the integer 0 is convertible to the tuple's value type.
    template <int src_size, typename src_valtype>
    explicit Tuple(Tuple<src_size, src_valtype> const &src) {
        enum { COPY_END = (SIZE < src_size ? SIZE : src_size) };

        for (int i = 0; i < COPY_END; ++i) {
            mm[i] = src[i];
        }
        for (int i = COPY_END; i < SIZE; ++i) {
            mm[i] = 0;
        }
    }

    T operator[](int i) const {
        // we'd prefer to use size_t, but can't because gcc3.2 doesn't like
        // it - it conflicts with child class conversion operators to
        // pointer types.
//             assert(i >= 0 && i < SIZE);
        return mm[i];
    }

    T& operator[](int i) {
        // see above for size_t vs int
//             assert(i >= 0 && i < SIZE);
        return mm[i];
    }

    /// @name Compatibility
    /// These are mostly for backwards compability with functions that take
    /// old-style Vs (which are just arrays).
    //@{
    /// Copies this tuple into an array of a compatible type
    template <typename S>
    void toV(S *v) const {
        for (int i = 0; i < SIZE; ++i) {
            v[i] = mm[i];
        }
    }

    /// Exposes the internal array.  Be careful when using this function.
    value_type *asV() {
        return mm;
    }
    /// Exposes the internal array.  Be careful when using this function.
    value_type const *asV() const {
        return mm;
    }
    //@}  Compatibility

    /// @return string representation of Classname
    std::string str() const {
        std::ostringstream buffer;

        buffer << "[";

        // For each column
        for (unsigned j(0); j < SIZE; j++) {
            if (j) buffer << ", ";
            buffer << PrintCast(mm[j]);
        }

        buffer << "]";

        return buffer.str();
    }

    void write(std::ostream& os) const {
        os.write(reinterpret_cast<const char*>(&mm), sizeof(T)*SIZE);
    }
    void read(std::istream& is) {
        is.read(reinterpret_cast<char*>(&mm), sizeof(T)*SIZE);
    }

    /// True if a Nan is present in this tuple
    bool isNan() const {
        for (int i = 0; i < SIZE; ++i) {
            if (math::isNan(mm[i])) return true;
        }
        return false;
    }

    /// True if an Inf is present in this tuple
    bool isInfinite() const {
        for (int i = 0; i < SIZE; ++i) {
            if (math::isInfinite(mm[i])) return true;
        }
        return false;
    }

    /// True if no Nan or Inf values are present
    bool isFinite() const {
        for (int i = 0; i < SIZE; ++i) {
            if (!math::isFinite(mm[i])) return false;
        }
        return true;
    }

    /// True if all elements are exactly zero
    bool isZero() const {
        for (int i = 0; i < SIZE; ++i) {
            if (!math::isZero(mm[i])) return false;
        }
        return true;
    }

protected:
    T mm[SIZE];
};


////////////////////////////////////////


/// @return true if t0 < t1, comparing components in order of significance.
template<int SIZE, typename T0, typename T1>
bool
operator<(const Tuple<SIZE, T0>& t0, const Tuple<SIZE, T1>& t1)
{
    for (int i = 0; i < SIZE-1; ++i) {
        if (!isExactlyEqual(t0[i], t1[i])) return t0[i] < t1[i];
    }
    return t0[SIZE-1] < t1[SIZE-1];
}


/// @return true if t0 > t1, comparing components in order of significance.
template<int SIZE, typename T0, typename T1>
bool
operator>(const Tuple<SIZE, T0>& t0, const Tuple<SIZE, T1>& t1)
{
    for (int i = 0; i < SIZE-1; ++i) {
        if (!isExactlyEqual(t0[i], t1[i])) return t0[i] > t1[i];
    }
    return t0[SIZE-1] > t1[SIZE-1];
}


////////////////////////////////////////


/// @return the absolute value of the given Tuple.
template<int SIZE, typename T>
Tuple<SIZE, T>
Abs(const Tuple<SIZE, T>& t)
{
    Tuple<SIZE, T> result;
    for (int i = 0; i < SIZE; ++i) result[i] = math::Abs(t[i]);
    return result;
}

/// Return @c true if a Nan is present in the tuple.
template<int SIZE, typename T>
inline bool isNan(const Tuple<SIZE, T>& t) { return t.isNan(); }

/// Return @c true if an Inf is present in the tuple.
template<int SIZE, typename T>
inline bool isInfinite(const Tuple<SIZE, T>& t) { return t.isInfinite(); }

/// Return @c true if no Nan or Inf values are present.
template<int SIZE, typename T>
inline bool isFinite(const Tuple<SIZE, T>& t) { return t.isFinite(); }

/// Return @c true if all elements are exactly equal to zero.
template<int SIZE, typename T>
inline bool isZero(const Tuple<SIZE, T>& t) { return t.isZero(); }

////////////////////////////////////////


/// Write a Tuple to an output stream
template <int SIZE, typename T>
std::ostream& operator<<(std::ostream& ostr, const Tuple<SIZE, T>& classname)
{
    ostr << classname.str();
    return ostr;
}

} // namespace math
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb

#endif // OPENVDB_MATH_TUPLE_HAS_BEEN_INCLUDED
