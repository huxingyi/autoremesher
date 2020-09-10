//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.
//

#ifndef _PxDeepUtils_h_
#define _PxDeepUtils_h_

#include <ImfNamespace.h>
#include <ImfPixelType.h>
#include <ImfPartType.h>

#include <ImathFun.h>
#include <ImathVec.h>
#include <ImathBox.h>
#include <ImathMatrix.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <iostream>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>

#include <dtex.h>
#include <half.h>
#include <math.h>
#include <float.h>
#include <cmath>
#include <sys/types.h>

using std::isfinite;

namespace PxDeep {

//-*****************************************************************************
// The large block of comments below explains our working terminology and
// the justification of our limits & magic numbers. In a few places, the
// use of centimeters as a spatial unit does affect the absolute position of
// various minima and maxima, but in normal usage those should be well outside
// working ranges.
//-*****************************************************************************

//-*****************************************************************************
// DENSITY
//-*****************************************************************************
// "Density" refers to the the optical density which, when integrated
// through a line, produces an alpha. The relationship between alpha, density,
// and line segment of a given length "dz" is as follows:
//
// alpha = 1.0 - exp( -dz * density )
//
// We use a minimum non-zero density in some places in our code, which
// represents the density of dry air at atmospheric pressure. Though
// different wavelengths of light are attenuated differently, the average
// attenuation is 10^-5 per meter. To make it very minimal,
// we'll work with 1/10th that density (tiny tiny). Since our facility
// works in centimeters, this works out to (using & rearranging the
// equation above)
//
// 10^-6 = 1.0 - exp( -100.0 * MIN_NON_ZERO_DENSITY )
// exp( -100.0 * MIN_NON_ZERO_DENSITY ) = 1.0 - 10^-6
// -100.0 * MIN_NON_ZERO_DENSITY = log( 1.0 - 10^-6 )
// MIN_NON_ZERO_DENSITY = log( 1.0 - 10^-6 ) / -100.0
// MIN_NON_ZERO_DENSITY = 1.0000050000290891e-08
//
// We use double precision for density and dz calculations.
//
//-*****************************************************************************
// VISIBILITY (or 'VIZ', or 'TRANSMISSION')
//-*****************************************************************************
// Throughout the code below, we transform "alpha" into its inverse, which is
// transmissivity, or visibility, or for short, 'viz'. The relationship between
// alpha and viz is simple:
//
// alpha = 1.0 - viz,  or viz = 1.0 - alpha.
//
// Similarly, the relationship between viz and density & dz is simple:
//
// viz = exp( -dz * density )
// log( viz ) = -dz * density
//
// Viz is easier to work with than alpha, because to accumulate a total
// visibility of many adjacent samples, the relationship is just, for the
// set of sample viz's:  {viz0, viz1, viz2, ..., vizN-1}
//
// totalViz = viz0 * viz1 * viz2 * ... * vizN-1
//
// It's interesting to note that for any given set of spans, their accumulated
// visibility is the same regardless of what order they're accumulated in,
// since A*B == B*A.
//
// When using viz, we use double precision because the operation
// 1.0f - ( 1.0f - a ) loses precision, and we want as little of that as
// possible!
// 
//-*****************************************************************************
// DEPTH RANGES
//-*****************************************************************************
// Because we need to be able to arithmetically manipulate depths, we place
// a range on the valid depth values. Positive Infinity is a valid depth value
// to be stored in a DTEX file, but in order to make everything else work, we
// set the maximum depth to near (but not at) FLT_MAX, 10^30. Similarly, we
// set the minimum depth to just slightly greater than zero, 10^-4. This
// could potentially clip effects being deep composited with very small
// distances and units of meters.
//
//-*****************************************************************************
// DEEP OPACITY
//-*****************************************************************************
// "Deep Opacity" refers to a depth function in which the sample at each point
// represents the total accumulated opacity at that depth. This represents
// the way that deep shadows would have been produced by renderman with the
// Display Driver Line: "deepshad" "deepopacity", except that the files actually
// store the inverse (1.0-opacity) at each point. It is important to note
// that for any given Dtex deepopacity sample, the value represents the
// accumulation of visibility on the NEAR side of the sample - up to and
// including the sample's depth, but no further in depth. Deep Opacity
// functions are monotonically decreasing in depth, and are always
// between 0 and 1.
//
// A complication arises when the 0'th continuous deep opacity sample has a
// non-zero deep opacity, because we don't have enough information to infer
// where the continuous span that ends at the 0th sample begins in depth. We
// solve the problem by interrogating the entire deep pixel for the maximum
// density of all its spans (see above), and then solving for what dz
// would produce the given accumulated alpha for that max density. The
// near point of the initial span is then 'dz' units in front of the 0th
// sample depth.
//
// We sometimes use 'deepViz' to in the code below to refer to 1.0 - deepOpacity
// 
//-*****************************************************************************
// DEEP ALPHA
//-*****************************************************************************
// "Deep Alpha" refers to a depth function in which the sample at each point
// represents the non-accumulated alpha of that single sample. When interpreting
// the depth function as continuous instead of discrete, Deep Alpha represents
// the alpha of the FAR side of the sample - from the depth of the sample
// up to, but not including, the depth of the next sample.
// 
// A complication arises when the last continuous deep alpha sample has a
// non-zero deep alpha, because we don't have enough information to infer
// where the continuous span that begins at the last sample ends in depth. We
// solve this problem analagously to how we solve the DeepOpacity problem.
// We get the maximum density along the entire deep pixel and extrapolate to
// determine an end depth.
//
//-*****************************************************************************
// DEEP RGBA
//-*****************************************************************************
// Deep RGBA is exactly the same as Deep Alpha, for both discrete and
// continuous cases, with the additional R,G, and B channels carried along.
// The RGB can be read as premultiplied by alpha, or not. The output deep
// pixel expects RGB to be premultiplied by alpha.
// The use of premultiplied alpha makes it possible to entangle emitted and
// reflected light - basically "glows", when premultiplied R,G,B are non-zero
// but alpha is zero. However, in order for us to collapse coindicent samples,
// we need to temporarily store RGB unpremultiplied. We simply don't affect
// the samples that have zero alpha, and don't remultiply samples that have
// zero alpha. There's no way for uncombined samples that had non-zero alpha
// to produce a combined sample with zero alpha, so any sample that has
// zero alpha at the end of all the combining was entirely composed of zero
// alpha samples to begin with.  SO, if the alpha is zero, we don't
// multiply by it!
//-*****************************************************************************

//-*****************************************************************************
//-*****************************************************************************
//-*****************************************************************************
// UTILITY CONSTANTS AND FUNCTIONS
//-*****************************************************************************
//-*****************************************************************************
//-*****************************************************************************

//-*****************************************************************************
// Explained above in the "Density" section of the comments.
// We set this value to one tenth the attenuation of light in dry air
// at atmospheric pressure.
// 10^-6 = 1.0 - exp( -100.0 * MIN_NON_ZERO_DENSITY )
// exp( -100.0 * MIN_NON_ZERO_DENSITY ) = 1.0 - 10^-6
// -100.0 * MIN_NON_ZERO_DENSITY = log( 1.0 - 10^-6 )
// MIN_NON_ZERO_DENSITY = log( 1.0 - 10^-6 ) / -100.0
// MIN_NON_ZERO_DENSITY = 1.0000050000290891e-08
// static const double MIN_NON_ZERO_DENSITY = log( 1.0 - 1.0e-6 ) / -100.0;
#define PXDU_MIN_NON_ZERO_DENSITY 1.0000050000290891e-08L

//-*****************************************************************************
// The change in depth which produces maximum alpha for maximum density.
// We want this to be small without risking subnormality.
// static const double DZ_OF_ALPHA_1 = 0.001;
// static const double DZ_OF_VIZ_0 = 0.001;
#define PXDU_DZ_OF_ALPHA_1 0.001
#define PXDU_DZ_OF_VIZ_0 0.001

//-*****************************************************************************
// We set the max density of alpha 1 to the density which would produce
// an alpha of 0.99999 in a distance of 0.001 centimeters (DZ_OF_ALPHA_1)
//
// 0.99999 = 1.0 - exp( -0.001 * MAX_DENSITY )
// exp( -0.001 * MAX_DENSITY ) = 1.0 - 0.99999
// -0.001 * MAX_DENSITY = log( 1.0 - 0.99999 )
// MAX_DENSITY = log( 1.0 - 0.99999 ) / -0.001
// MAX_DENSITY = 11512.925464974779
// static const double DENSITY_OF_ALPHA_1 = log( 1.0 - 0.99999 ) / -0.001;
// static const double DENSITY_OF_VIZ_0 = DENSITY_OF_ALPHA_1;
#define PXDU_DENSITY_OF_ALPHA_1 11512.92546497478
#define PXDU_DENSITY_OF_VIZ_0 11512.92546497478

//-*****************************************************************************
// Just in case we need it. These are the constants used above.
#define PXDU_MAX_NON_OPAQUE_ALPHA 0.99999
#define PXDU_MIN_NON_OPAQUE_VIZ 0.00001

#define PXDU_MIN_NON_TRANSPARENT_ALPHA 0.00001
#define PXDU_MAX_NON_TRANSPARENT_VIZ 0.99999

//-*****************************************************************************
// Explained above in the "Depth" section of the comments.
// static const double MIN_DEEP_DEPTH = 1.0e-4;
// static const double MAX_DEEP_DEPTH = 1.0e30;
#define PXDU_MIN_DEEP_DEPTH 1.0e-4
#define PXDU_MAX_DEEP_DEPTH 1.0e30

//-*****************************************************************************
// A maximum depth change (dz)
// static const double MAX_DZ = double( MAX_DEEP_DEPTH ) -
//     double( MIN_DEEP_DEPTH );
#define PXDU_MAX_DZ 1.0e30

//-*****************************************************************************
// IEEE 754 floats can be incremented to the "next" positive float
// in this manner, for positive float inputs.
inline float IncrementPositiveFloat( float i_a, int32_t i_inc=1 )
{
    typedef union
    {
        int32_t i;
        float f;
    } intfloat;
    intfloat a;
    a.f = i_a;
    a.i += i_inc;
    return a.f;
}

//-*****************************************************************************
// IEEE 754 floats can be decremented to the "previous" positive float
// in this manner, for positive float inputs.
inline float DecrementPositiveFloat( float i_a, int32_t i_inc=1 )
{
    typedef union
    {
        int32_t i;
        float f;
    } intfloat;
    intfloat a;
    a.f = i_a;
    a.i -= i_inc;
    return a.f;
}

//-*****************************************************************************
// From: man isinf
// isinf(x)      returns 1 if x is positive infinity, and -1 if x is nega-
//                     tive infinity.
template <typename T>
inline bool IsInfinity( T i_f )
{
    return ( isinf( i_f ) == 1 );
}

//-*****************************************************************************
// A zero-nan functon, which actually zeros inf as well.
template <typename T>
inline T ZeroNAN( T i_f )
{
    if ( !isfinite( i_f ) ) { return ( T )0; }
    else { return i_f; }
}

//-*****************************************************************************
template <typename T>
inline T ClampDepth( T i_depth )
{
    if ( IsInfinity( i_depth ) )
    {
        return PXDU_MAX_DEEP_DEPTH;
    }
    else
    {
        return Imath::clamp( i_depth,
                             ( T )PXDU_MIN_DEEP_DEPTH,
                             ( T )PXDU_MAX_DEEP_DEPTH );
    }
}

//-*****************************************************************************
template <typename T>
inline T ClampDz( T i_dz )
{
    return Imath::clamp( ZeroNAN( i_dz ), ( T )0, ( T )PXDU_MAX_DZ );
}

//-*****************************************************************************
template <typename T>
inline T ClampNonZeroDz( T i_dz )
{
    return Imath::clamp( ZeroNAN( i_dz ),
                         ( T )PXDU_DZ_OF_ALPHA_1,
                         ( T )PXDU_MAX_DZ );
}

//-*****************************************************************************
template <typename T>
inline T ClampAlpha( T i_alpha )
{
    return Imath::clamp( ZeroNAN( i_alpha ), ( T )0, ( T )1 );
}

//-*****************************************************************************
// "plausible" in this case means not completely transparent, nor
// completely opaque.
template <typename T>
inline T ClampPlausibleAlpha( T i_alpha )
{
    return Imath::clamp( ZeroNAN( i_alpha ),
                         ( T )PXDU_MIN_NON_TRANSPARENT_ALPHA,
                         ( T )PXDU_MAX_NON_OPAQUE_ALPHA );
}

//-*****************************************************************************
template <typename T>
inline double ClampViz( T i_viz )
{
    return Imath::clamp( ZeroNAN( i_viz ), ( T )0, ( T )1 );
}

//-*****************************************************************************
// "plausible" in this case means not completely transparent, nor
// completely opaque.
template <typename T>
inline T ClampPlausibleViz( T i_viz )
{
    return Imath::clamp( ZeroNAN( i_viz ),
                         ( T )PXDU_MIN_NON_OPAQUE_VIZ,
                         ( T )PXDU_MAX_NON_TRANSPARENT_VIZ );
}

//-*****************************************************************************
// Plausible density is clamped between min non-zero density
// and density of alpha 1.
template <typename T>
inline T ClampPlausibleDensity( T i_density )
{
    return Imath::clamp( ZeroNAN( i_density ),
                         ( T )PXDU_MIN_NON_ZERO_DENSITY,
                         ( T )PXDU_DENSITY_OF_ALPHA_1 );
}

//-*****************************************************************************
// Density/Viz/DZ calculations are always performed in double precision.
// We try to leave them alone as much as possible, but the logarithm can get
// weird for very very small numbers. The "isfinite" call basically rules
// out NaN and Infinity results, though it doesn't bother with subnormal
// numbers, since the error case we're worried about is log being too big.
// viz = exp( -dz * density )
// log( viz ) = -dz * density
// density = -log( viz ) / dz
double DensityFromVizDz( double i_viz, double i_dz );

//-*****************************************************************************
// We can often treat "density times dz" as a single quantity without
// separating it.
// viz = exp( -densityTimesDz )
// log( viz ) = -densityTimesDz
// densityTimesDz = -log( viz )
double DensityTimesDzFromViz( double i_viz );

//-*****************************************************************************
// Plausible density defined above.
inline double PlausibleDensityFromVizDz( double i_viz, double i_dz )
{
    return ClampPlausibleDensity( DensityFromVizDz( i_viz, i_dz ) );
}

//-*****************************************************************************
// viz = exp( -dz * density )
// log( viz ) = -dz * density
// dz = -log( viz ) / density
// Note that this is basically the same as the computation above.
double DzFromVizDensity( double i_viz, double i_density );

//-*****************************************************************************
// viz = exp( -dz * density ) // valid for all finite numbers.
// negative densities or dz's will give greater than 1 viz's, which will
// get clamped!
inline double VizFromDensityDz( double i_density, double i_dz )
{
    return ClampViz( exp( -ZeroNAN( i_density * i_dz ) ) );
}

//-*****************************************************************************
// same as above.
inline double VizFromDensityTimesDz( double i_densityTimesDz )
{
    return ClampViz( exp( -ZeroNAN( i_densityTimesDz ) ) );
}

//-*****************************************************************************
//-*****************************************************************************
//-*****************************************************************************
// IMF SPECIFIC STUFF
//-*****************************************************************************
//-*****************************************************************************
//-*****************************************************************************

//-*****************************************************************************
template <typename T>
Imf::PixelType ImfPixelType();

template <>
inline Imf::PixelType ImfPixelType<half>() { return Imf::HALF; }

template <>
inline Imf::PixelType ImfPixelType<float>() { return Imf::FLOAT; }

template <>
inline Imf::PixelType ImfPixelType<uint>() { return Imf::UINT; }

//-*****************************************************************************
// Handy exception macro.
#define PXDU_THROW( TEXT )                      \
do                                              \
{                                               \
    std::stringstream sstr;                     \
    sstr << TEXT;                               \
    std::runtime_error exc( sstr.str() );       \
    throw( exc );                               \
}                                               \
while( 0 )

} // End namespace PxDeep

#endif
