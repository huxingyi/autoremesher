//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.
//

#include "PxDeepUtils.h"

namespace PxDeep {

//-*****************************************************************************
// Density/Viz/DZ calculations are always performed in double precision.
// We try to leave them alone as much as possible, but the logarithm can get
// weird for very very small numbers. The "isfinite" call basically rules
// out NaN and Infinity results, though it doesn't bother with subnormal
// numbers, since the error case we're worried about is log being too big.
// viz = exp( -dz * density )
// log( viz ) = -dz * density
// density = -log( viz ) / dz
double DensityFromVizDz( double i_viz, double i_dz )
{
    assert( i_viz >= 0.0 );
    assert( i_viz <= 1.0 );
    assert( i_dz >= 0.0 );

    if ( i_viz >= 1.0 )
    {
        // There's no attenuation at all, so there's no density!
        return 0.0;
    }
    else if ( i_viz <= 0.0 )
    {
        // There's total attenuation, so we use our max density.
        return PXDU_DENSITY_OF_VIZ_0;
    }
    else if ( i_dz <= 0.0 )
    {
        // There's no depth, and viz is greater than zero,
        // so we assume the density is as high as possible
        return PXDU_DENSITY_OF_VIZ_0;
    }
    else
    {
        double d = -log( i_viz ) / i_dz;
        if ( !isfinite( d ) )
        {
            return PXDU_DENSITY_OF_VIZ_0;
        }
        else
        {
            return d;
        }
    }
}

//-*****************************************************************************
// We can often treat "density times dz" as a single quantity without
// separating it.
// viz = exp( -densityTimesDz )
// log( viz ) = -densityTimesDz
// densityTimesDz = -log( viz )
double DensityTimesDzFromViz( double i_viz )
{
    assert( i_viz >= 0.0 );
    assert( i_viz <= 1.0 );

    if ( i_viz >= 1.0 )
    {
        // There's no attenuation at all, so there's no density!
        return 0.0;
    }
    else if ( i_viz <= 0.0 )
    {
        // There's total attenuation, so we use our max density.
        return PXDU_DENSITY_OF_VIZ_0 * PXDU_DZ_OF_VIZ_0;
    }
    else
    {
        double d = -log( i_viz );
        if ( !isfinite( d ) )
        {
            return PXDU_DENSITY_OF_VIZ_0 * PXDU_DZ_OF_VIZ_0;
        }
        else
        {
            return d;
        }
    }
}

//-*****************************************************************************
// viz = exp( -dz * density )
// log( viz ) = -dz * density
// dz = -log( viz ) / density
// Note that this is basically the same as the computation above.
double DzFromVizDensity( double i_viz, double i_density )
{
    assert( i_viz >= 0.0 );
    assert( i_viz <= 1.0 );
    assert( i_density >= 0.0 );

    if ( i_viz >= 1.0 )
    {
        // There's no attenuation, so there's no depth.
        return 0.0;
    }
    else if ( i_viz <= 0.0 )
    {
        // There's total attenuation, so we use the smallest depth
        // for our max density.
        return PXDU_DZ_OF_VIZ_0;
    }
    else if ( i_density <= 0.0 )
    {
        // Hmmm. There's no density, but there is some attenuation,
        // which basically implies an infinite depth.
        // We'll use the minimum density.
        // This whole part is hacky at best.
        double dz = -log( i_viz ) / PXDU_MIN_NON_ZERO_DENSITY;
        if ( !isfinite( dz ) )
        {
            return PXDU_MAX_DZ;
        }
        else
        {
            return dz;
        }
    }
    else
    {
        double dz = -log( i_viz ) / i_density;
        if ( !isfinite( dz ) )
        {
            return PXDU_MAX_DZ;
        }
        else
        {
            return dz;
        }
    }
}

} // End namespace PxDeep

