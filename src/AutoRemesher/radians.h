#ifndef AUTO_REMESHER_RADIANS_H
#define AUTO_REMESHER_RADIANS_H

namespace AutoRemesher
{
    
namespace Radians
{
    
inline double toDegrees(double radians)
{
    return radians * (180.0 / M_PI);
}

inline double fromDegrees(double degrees)
{
    return degrees * (M_PI / 180.0);
}

}

}

#endif
