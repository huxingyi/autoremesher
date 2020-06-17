#include "VectorT_dummy_data.hpp"

template<> std::valarray<double> make_dummy_component() {
    return std::valarray<double>(128);
}
