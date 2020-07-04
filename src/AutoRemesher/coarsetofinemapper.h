#ifndef AUTO_REMESHER_COARSE_TO_FINE_MAPPER_H
#define AUTO_REMESHER_COARSE_TO_FINE_MAPPER_H
#include <AutoRemesher/Vector2>
#include <TransitionFunction.hh>

namespace AutoRemesher
{
    
class UnFlipOperator
{
public:
    UnFlipOperator()
    {
        
    }
    
    QEx::TransitionFunctionT<double> m_transitionFlipI;
};
    
class CoarseToFineMapper
{
public:

private:
    
};
    
}

#endif