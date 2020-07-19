#ifndef AUTO_REMESHER_PARAMETRIZATION_H
#define AUTO_REMESHER_PARAMETRIZATION_H
#include <AutoRemesher/HalfEdge>

namespace AutoRemesher
{
    
namespace Parametrization
{
    
struct Parameters
{
    double gradientSize = 100;
    double constraintStength = 0.5;
};

bool miq(HalfEdge::Mesh &mesh, const Parameters &parameters=Parameters());
    
}
    
}

#endif
