#ifndef MORPHO_VIDEO_REFINER_INTERNAL_H
#define MORPHO_VIDEO_REFINER_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "morpho_video_refiner.h"
#include "morpho_quadr_int.h"

/**
 * Sets the NR position dependence function for luma channels.
 *
 * This method adjusts strength of noise reduction depending on the position of each pixel.
 * The default setting is following:
 * @code
 * double tmp_nr_position( const int x, const int y )
 * {
 *     return 1.;
 * }
 * @endcode
 * The return value of 1 means using the same noise reduction strength as one set by methods 
 * @ref morpho_VideoRefiner_setSpatialNRLevel() and @ref morpho_VideoRefiner_setTemporalLumaNRLevel().
 * Bigger return values lead to stronger noise reduction.
 * For example, using the following function, the strength of a peripheral region becomes
 * stronger than a center region.
 * @code
 * double tmp_nr_position( const int x, const int y )
 * {
 *     return hypot( x - WIDTH / 2., y - HEIGHT / 2. ) / ( WIDTH / 2. );
 * }
 * @endcode
 *
 * @note
 * Processing time does not depend on the specific form of this fuction.
 *
 * @param[in,out] p_vref
 * @param[in]     nr_position
 *
 * @return Error Code
 */
MORPHO_API(int)
morpho_VideoRefiner_setLumaNRPositionDependence( morpho_VideoRefiner * const p_vref, double (*nr_position)( const int x, const int y ) );

/**
 * Sets the NR position dependence function for chroma channels.
 *
 * @see morpho_VideoRefiner_setLumaNRPositionDependence()
 *
 * @param[in,out] p_vref
 * @param[in]     nr_position
 *
 * @return Error Code
 */
MORPHO_API(int)
morpho_VideoRefiner_setChromaNRPositionDependence( morpho_VideoRefiner * const p_vref, double (*nr_position)( const int x, const int y ) );

/***** debug *****/

/**
 * This is for debug use.
 */
MORPHO_API(int)
morpho_VideoRefiner_restartMovieSolid( morpho_VideoRefiner * const p_vref );

/**
 * This is for debug use.
 */
MORPHO_API(int)
morpho_VideoRefiner_restartVideoDenoiser( morpho_VideoRefiner * const p_vref );

/**
 * This is for debug use.
 */
MORPHO_API(int)
morpho_VideoRefiner_getClippingQuadr( const morpho_VideoRefiner * const p_vref, morpho_QuadrInt * const p_quadr );

#ifdef __cplusplus
}
#endif

#endif /* MORPHO_VIDEO_REFINER_INTERNAL_H */
