//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.
//

#ifndef _PxFourChanDeepRgba_h_
#define _PxFourChanDeepRgba_h_

#include "PxDeepUtils.h"
#include "PxBaseDeepHelper.h"

namespace PxDeep {

//-*****************************************************************************
// FOUR CHANNEL DEEP RGBA CONTINUOUS
//-*****************************************************************************
template <typename RGBA_T>
class FourChanDeepRgbaContinuous
    : public BaseDeepHelper<RGBA_T,
                            FourChanDeepRgbaContinuous<RGBA_T>,SpanRgba>
{
public:
    typedef BaseDeepHelper<RGBA_T,
                           FourChanDeepRgbaContinuous<RGBA_T>,SpanRgba>
    super_type;
    typedef FourChanDeepRgbaContinuous<RGBA_T> this_type;
    typedef typename super_type::span_type span_type;

    FourChanDeepRgbaContinuous( DtexFile* i_dtexFile,
                                int i_numDtexChans,
                                const Parameters& i_params )
      : BaseDeepHelper<RGBA_T,
                       FourChanDeepRgbaContinuous<RGBA_T>,SpanRgba>
    ( i_dtexFile,
      i_numDtexChans,
      i_params ) {}
    
    void processDeepPixel( int i_numPts );
};

//-*****************************************************************************
// FOUR CHANNEL DEEP RGBA DISCRETE
//-*****************************************************************************
template <typename RGBA_T>
class FourChanDeepRgbaDiscrete
    : public BaseDeepHelper<RGBA_T,
                            FourChanDeepRgbaDiscrete<RGBA_T>,SpanRgba>
{
public:
    typedef BaseDeepHelper<RGBA_T,
                           FourChanDeepRgbaDiscrete<RGBA_T>,SpanRgba>
    super_type;
    typedef FourChanDeepRgbaDiscrete<RGBA_T> this_type;
    typedef typename super_type::span_type span_type;

    FourChanDeepRgbaDiscrete( DtexFile* i_dtexFile,
                              int i_numDtexChans,
                              const Parameters& i_params )
      : BaseDeepHelper<RGBA_T,
                       FourChanDeepRgbaDiscrete<RGBA_T>,SpanRgba>
    ( i_dtexFile,
      i_numDtexChans,
      i_params ) {}
    
    void processDeepPixel( int i_numPts );
};

//-*****************************************************************************
template <typename RGBA_T>
void FourChanDeepRgbaContinuous<RGBA_T>::processDeepPixel( int i_numPts )
{
    assert( i_numPts > 0 );
    
    // Loop over all the dtex points and get their deepAlphas
    // and their depths. Enforce the case that deepAlpha
    // is always between 0 and 1.
    (this->m_spans).resize( ( size_t )i_numPts );

    for ( int j = 0; j < i_numPts; ++j )
    {
        float z;
        float pts[4];
        DtexPixelGetPoint( (this->m_pixel), j, &z, ( float * )pts );

        z = ClampDepth( z );

        double red = ZeroNAN( pts[0] );
        double green = ZeroNAN( pts[1] );
        double blue = ZeroNAN( pts[2] );
        double alpha = ClampAlpha( pts[3] );

        span_type& spanJ = (this->m_spans)[j];
        spanJ.clear();
        spanJ.in = z;
        spanJ.out = z;
        spanJ.viz = ClampViz( 1.0 - alpha );
        spanJ.index = j;

        // Only unpremult if the data is assumed to be premultiplied,
        // which is when the params say DON'T multiply color by alpha.
        if ( alpha > 0.0 && !(this->m_params).multiplyColorByAlpha )
        {
            spanJ.rgb[0] = red / alpha;
            spanJ.rgb[1] = green / alpha;
            spanJ.rgb[2] = blue / alpha;
        }
        else
        {
            spanJ.rgb[0] = red;
            spanJ.rgb[1] = green;
            spanJ.rgb[2] = blue;
        }
    }

    // Sort the spans.
    std::sort( (this->m_spans).begin(), (this->m_spans).end() );

    // Combine identical depths.
    double maxDensity = PXDU_MIN_NON_ZERO_DENSITY;
    {
        int activeBegin = 0;
        int activeEnd = 0;
        float interestingDepth = 0.0f;
        int numRemoved = 0;

        while ( activeBegin < i_numPts )
        {
            span_type& spanActiveBegin = (this->m_spans)[activeBegin];
            float nextInterestingDepth = spanActiveBegin.in;
            assert( nextInterestingDepth > interestingDepth );

            activeEnd = i_numPts;
            for ( int a = activeBegin + 1; a < i_numPts; ++a )
            {
                span_type& spanNext = (this->m_spans)[a];

                assert( spanNext.in > interestingDepth );
                assert( spanNext.in >= nextInterestingDepth );

                if ( spanNext.in > nextInterestingDepth )
                {
                    // This span is not active in this round,
                    // set activeEnd and get out.
                    activeEnd = a;
                    break;
                }
                else
                {
                    // This span has an identical depth to
                    // the previous one, so we must combine their
                    // alphas and eliminate the depth.
                    // We simply add their unpremultiplied color.
                    spanActiveBegin.viz *= spanNext.viz;
                    spanActiveBegin.rgb[0] += spanNext.rgb[0];
                    spanActiveBegin.rgb[1] += spanNext.rgb[1];
                    spanActiveBegin.rgb[2] += spanNext.rgb[2];
                    spanNext.in = FLT_MAX;
                    spanNext.out = FLT_MAX;
                    ++numRemoved;
                }
            }

            spanActiveBegin.viz = ClampViz( spanActiveBegin.viz );

            // Accumulate density from here to the next point.
            if ( activeEnd < i_numPts )
            {
                span_type& spanNext = (this->m_spans)[activeEnd];
                double dz = spanNext.in - spanActiveBegin.in;
                assert( spanNext.in > spanActiveBegin.in );
                assert( dz > 0.0 );

                double density = DensityFromVizDz( spanActiveBegin.viz,
                                                   dz );
                maxDensity = std::max( maxDensity, density );
            }
            
            activeBegin = activeEnd;
            interestingDepth = nextInterestingDepth;
        }

        // If any removed, re-sort the list and remove the end
        // points.
        if ( numRemoved > 0 )
        {
            assert( numRemoved < i_numPts );
            std::sort( (this->m_spans).begin(), (this->m_spans).end() );
            i_numPts -= numRemoved;
            (this->m_spans).resize( i_numPts );
        }
    }

    // Handle the single point case.
    if ( i_numPts == 1 )
    {
        span_type& span0 = (this->m_spans)[0];
        
        if ( (this->m_params).discardZeroAlphaSamples &&
             span0.viz >= 1.0 )
        {
            // Nothing!
            return;
        }

        span0.out = ClampDepth(
            IncrementPositiveFloat( span0.in ) );

        double alpha = ClampAlpha( 1.0 - span0.viz );

        // If the alpha is zero, and we're still here, it means
        // that the spans are either completely transparent and
        // the user has elected to keep them anyway, in which case
        // no multiplication by alpha is needed, or alternatively,
        // they're "glow" spans which have zero alpha but non-zero
        // color. Those glow spans were not unpremultiplied above,
        // and therefore do not need to be premultiplied here.
        // SO! If alpha is zero, we don't premultiply.
        if ( alpha > 0.0 )
        {
            span0.rgb[0] *= alpha;
            span0.rgb[1] *= alpha;
            span0.rgb[2] *= alpha;
        }

        (this->m_deepOutPixel).push_back( span0.in,
                                          span0.out,
                                          span0.rgb[0],
                                          span0.rgb[1],
                                          span0.rgb[2],
                                          alpha );

        return;
    }

    // Put the spans back out.
    // If the last point has a non-zero alpha, extrapolate the
    // maximum density to create an end point.
    for ( int j = 0; j < i_numPts; ++j )
    {
        span_type& spanJ = (this->m_spans)[j];

        if ( (this->m_params).discardZeroAlphaSamples &&
             spanJ.viz >= 1.0 )
        {
            // This span is transparent, ignore it.
            continue;
        }

        if ( j < i_numPts-1 )
        {
            spanJ.out = (this->m_spans)[j+1].in;
        }
        else
        {
            // This is the last point.
            // If it has non-zero alpha, it needs depth,
            // which we use the max density for.
            if ( spanJ.viz >= 1.0 )
            {
                // Don't need to worry about this last span!
                // It is at the end of the continuous span, and
                // is completely transparent.
                continue;
            }
            
            double dz = DzFromVizDensity( spanJ.viz, maxDensity );
            spanJ.out = ClampDepth( spanJ.in + dz );
            if ( spanJ.out <= spanJ.in )
            {
                spanJ.out =
                    ClampDepth( IncrementPositiveFloat( spanJ.in ) );
            }
        }

        double alpha = ClampAlpha( 1.0 - spanJ.viz );

        // If the alpha is zero, and we're still here, it means
        // that the spans are either completely transparent and
        // the user has elected to keep them anyway, in which case
        // no multiplication by alpha is needed, or alternatively,
        // they're "glow" spans which have zero alpha but non-zero
        // color. Those glow spans were not unpremultiplied above,
        // and therefore do not need to be premultiplied here.
        // SO! If alpha is zero, we don't premultiply.
        if ( alpha > 0.0 )
        {
            spanJ.rgb[0] *= alpha;
            spanJ.rgb[1] *= alpha;
            spanJ.rgb[2] *= alpha;
        }
        
        // Set the channels!
        (this->m_deepOutPixel).push_back( spanJ.in,
                                          spanJ.out,
                                          spanJ.rgb[0],
                                          spanJ.rgb[1],
                                          spanJ.rgb[2],
                                          alpha );
    }
}

//-*****************************************************************************
template <typename RGBA_T>
void FourChanDeepRgbaDiscrete<RGBA_T>::processDeepPixel( int i_numPts )
{
    assert( i_numPts > 0 );
    
    // Loop over all the dtex points and get their deepAlphas
    // and their depths. Enforce the case that deepAlpha
    // is always between 0 and 1.
    (this->m_spans).resize( ( size_t )i_numPts );

    for ( int j = 0; j < i_numPts; ++j )
    {
        float z;
        float pts[4];
        DtexPixelGetPoint( (this->m_pixel), j, &z, ( float * )pts );

        z = ClampDepth( z );

        double red = ZeroNAN( pts[0] );
        double green = ZeroNAN( pts[1] );
        double blue = ZeroNAN( pts[2] );
        double alpha = ClampAlpha( pts[3] );

        span_type& spanJ = (this->m_spans)[j];
        spanJ.clear();
        spanJ.in = z;
        spanJ.out = z;
        spanJ.viz = ClampViz( 1.0 - alpha );
        spanJ.index = j;

        // Only unpremult if the data is assumed to be premultiplied,
        // which is when the params say DON'T multiply color by alpha.
        if ( alpha > 0.0 && !(this->m_params).multiplyColorByAlpha )
        {
            spanJ.rgb[0] = red / alpha;
            spanJ.rgb[1] = green / alpha;
            spanJ.rgb[2] = blue / alpha;
        }
        else
        {
            spanJ.rgb[0] = red;
            spanJ.rgb[1] = green;
            spanJ.rgb[2] = blue;
        }
    }

    // Sort the spans.
    std::sort( (this->m_spans).begin(), (this->m_spans).end() );

    // Combine identical depths.
    {
        int activeBegin = 0;
        int activeEnd = 0;
        float interestingDepth = 0.0f;
        int numRemoved = 0;

        while ( activeBegin < i_numPts )
        {
            span_type& spanActiveBegin = (this->m_spans)[activeBegin];
            float nextInterestingDepth = spanActiveBegin.in;
            assert( nextInterestingDepth > interestingDepth );

            activeEnd = i_numPts;
            for ( int a = activeBegin + 1; a < i_numPts; ++a )
            {
                span_type& spanNext = (this->m_spans)[a];

                assert( spanNext.in > interestingDepth );
                assert( spanNext.in >= nextInterestingDepth );

                if ( spanNext.in > nextInterestingDepth )
                {
                    // This span is not active in this round,
                    // set activeEnd and get out.
                    activeEnd = a;
                    break;
                }
                else
                {
                    // This span has an identical depth to
                    // the previous one, so we must combine their
                    // alphas and eliminate the depth.
                    // We simply add their unpremultiplied color.
                    spanActiveBegin.viz *= spanNext.viz;
                    spanActiveBegin.rgb[0] += spanNext.rgb[0];
                    spanActiveBegin.rgb[1] += spanNext.rgb[1];
                    spanActiveBegin.rgb[2] += spanNext.rgb[2];
                    spanNext.in = FLT_MAX;
                    spanNext.out = FLT_MAX;
                    ++numRemoved;
                }
            }

            spanActiveBegin.viz = ClampViz( spanActiveBegin.viz );

            activeBegin = activeEnd;
            interestingDepth = nextInterestingDepth;
        }

        // If any removed, re-sort the list and remove the end
        // points.
        if ( numRemoved > 0 )
        {
            assert( numRemoved < i_numPts );
            std::sort( (this->m_spans).begin(), (this->m_spans).end() );
            i_numPts -= numRemoved;
            (this->m_spans).resize( i_numPts );
        }
    }

    // Put the spans back out.
    // If the last point has a non-zero alpha, extrapolate the
    // maximum density to create an end point.
    for ( int j = 0; j < i_numPts; ++j )
    {
        span_type& spanJ = (this->m_spans)[j];

        if ( (this->m_params).discardZeroAlphaSamples &&
             spanJ.viz >= 1.0 )
        {
            // This span is transparent, ignore it.
            continue;
        }

        double alpha = ClampAlpha( 1.0 - spanJ.viz );

        // If the alpha is zero, and we're still here, it means
        // that the spans are either completely transparent and
        // the user has elected to keep them anyway, in which case
        // no multiplication by alpha is needed, or alternatively,
        // they're "glow" spans which have zero alpha but non-zero
        // color. Those glow spans were not unpremultiplied above,
        // and therefore do not need to be premultiplied here.
        // SO! If alpha is zero, we don't premultiply.
        if ( alpha > 0.0 )
        {
            spanJ.rgb[0] *= alpha;
            spanJ.rgb[1] *= alpha;
            spanJ.rgb[2] *= alpha;
        }
        
        // Set the channels!
        (this->m_deepOutPixel).push_back( spanJ.in,
                                          spanJ.rgb[0],
                                          spanJ.rgb[1],
                                          spanJ.rgb[2],
                                          alpha );
    }
}

} // End namespace PxDeep

#endif
