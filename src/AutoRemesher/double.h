#ifndef AUTO_REMESHER_DOUBLE_H
#define AUTO_REMESHER_DOUBLE_H
#include <cmath>
#include <limits>

namespace AutoRemesher
{
    
namespace Double
{
    
inline bool isZero(double number)
{
    return std::abs(number) <= std::numeric_limits<double>::epsilon();
}

inline bool isEqual(double a, double b)
{
    return isZero(a - b);
}

} //namespace Double

}

#endif