/**
 * @file  morpho_video_refiner.h
 * @brief Morpho video refiner engine
 *
 * @date 2014/11/04
 *
 * Copyright (C) 2014 Morpho, Inc.
 */
 
#ifndef MORPHO_VIDEO_REFINER_H
#define MORPHO_VIDEO_REFINER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "morpho_api.h"
#include "morpho_image_data.h"
#include "morpho_error.h"


/** Version string */
#define MORPHO_VIDEO_REFINER_VER "Morpho VideoRefiner 1.0.0 2014/11/04"


/**
 * Constant values for @ref morpho_VideoRefiner_getFcode().
 */
enum {
    MORPHO_VIDEO_REFINER_FAILURE_NO_ERROR       = 0,      /**< No error. */
    MORPHO_VIDEO_REFINER_FAILURE_OUT_OF_RANGE   = (1<<0), /**< Too large camera motion to be stabilized. */
    MORPHO_VIDEO_REFINER_FAILURE_NO_MOVEMENT    = (1<<1), /**< The camera is fixed. (No need for stabilization) */
    MORPHO_VIDEO_REFINER_FAILURE_MOVING_SUBJECT = (1<<2), /**< Detecting big moving subjects. */
    MORPHO_VIDEO_REFINER_FAILURE_FEATURELESS    = (1<<3), /**< The input image contains little feature points. */
    MORPHO_VIDEO_REFINER_FAILURE_GENERIC        = (1<<4), /**< Other undesirable situations. */
};

/**
 * Constant values for @ref morpho_VideoRefiner_setMode().
 */
enum {
    MORPHO_VIDEO_REFINER_MODE_VIDEO_REFINER,
    MORPHO_VIDEO_REFINER_MODE_MOVIE_SOLID,
    MORPHO_VIDEO_REFINER_MODE_MOVIE_SOLID_LITE,
    MORPHO_VIDEO_REFINER_MODE_VIDEO_DENOISER,

    MORPHO_VIDEO_REFINER_MODE_NUM,
};

/**
 * Constant values for @ref morpho_VideoRefiner_initialize().
 */
enum {
    MORPHO_VIDEO_REFINER_MOTION_6DOF_SOFT,   /**< Same as MovieSolid 3.x. */
    MORPHO_VIDEO_REFINER_MOTION_6DOF_HYBRID, /**< Same as MovieSolid 3.x. (Hybrid) */
    MORPHO_VIDEO_REFINER_MOTION_8DOF_SOFT,   /**< Same as MovieSolid 4.x. */
    MORPHO_VIDEO_REFINER_MOTION_8DOF_HYBRID, /**< Same as MovieSolid 4.x. (Hybrid) */
    
    MORPHO_VIDEO_REFINER_MOTION_8DOF_HARD,   /**< Motion detection from gyroscope data only. */

    MORPHO_VIDEO_REFINER_MOTION_4DOF_SOFT,
    MORPHO_VIDEO_REFINER_MOTION_4DOF_HYBRID,
    MORPHO_VIDEO_REFINER_MOTION_4DOF_HYBRID_MEMCPY,

    MORPHO_VIDEO_REFINER_MOTION_NUM,
};

/**
 * Constant values for @ref morpho_VideoRefiner_initialize().
 */
enum {
    MORPHO_VIDEO_REFINER_ACCURACY_HIGH,
    MORPHO_VIDEO_REFINER_ACCURACY_MIDDLE,
    MORPHO_VIDEO_REFINER_ACCURACY_LOW,

    MORPHO_VIDEO_REFINER_ACCURACY_NUM,
};

/**
 * Constant values for @ref morpho_VideoRefiner_setViewAngle().
 */
enum {
    MORPHO_VIDEO_REFINER_ANGLE_TYPE_HORIZONTAL,
    MORPHO_VIDEO_REFINER_ANGLE_TYPE_VERTICAL,
    MORPHO_VIDEO_REFINER_ANGLE_TYPE_DIAGONAL,

    MORPHO_VIDEO_REFINER_ANGLE_TYPE_NUM
};

/**
 * Constant values for @ref morpho_VideoRefiner_setScanlineOrientation().
 */
enum {
    MORPHO_VIDEO_REFINER_SCANLINE_ORIENTATION_HORIZONTAL,
    MORPHO_VIDEO_REFINER_SCANLINE_ORIENTATION_VERTICAL,

    MORPHO_VIDEO_REFINER_SCANLINE_ORIENTATION_NUM
};

/**
 * VideoRefiner.
 */
typedef struct {
    void *p; /**< Pointer for internal objects. */

} morpho_VideoRefiner;


/**
 * @brief
 * Returns the working buffer size required by the engine with the specified parameters.
 *
 * @param[in]     in_width    Input width.
 * @param[in]     in_height   Input height.
 * @param[in]     p_format    Image format.
 *  @arg "YUV420_SEMIPLANAR"
 *  @arg "YVU420_SEMIPLANAR"
 *  @arg "YUV420_PLANAR"
 *
 * @param[in] motion Motion detection type. (d.o.f.)
 *  @arg @ref MORPHO_VIDEO_REFINER_MOTION_4DOF_SOFT
 *  @arg @ref MORPHO_VIDEO_REFINER_MOTION_4DOF_HYBRID
 *  @arg @ref MORPHO_VIDEO_REFINER_MOTION_6DOF_SOFT
 *  @arg @ref MORPHO_VIDEO_REFINER_MOTION_6DOF_HYBRID
 *  @arg @ref MORPHO_VIDEO_REFINER_MOTION_8DOF_SOFT
 *  @arg @ref MORPHO_VIDEO_REFINER_MOTION_8DOF_HYBRID
 *  @arg @ref MORPHO_VIDEO_REFINER_MOTION_8DOF_HARD
 *
 * @param[in] accuracy Motion detection accuracy.
 *  @arg @ref MORPHO_VIDEO_REFINER_ACCURACY_HIGH,
 *  @arg @ref MORPHO_VIDEO_REFINER_ACCURACY_MIDDLE,
 *  @arg @ref MORPHO_VIDEO_REFINER_ACCURACY_LOW,
 * 
 * @param[in] mode Operation mode.
 *  @arg @ref MORPHO_VIDEO_REFINER_MODE_VIDEO_REFINER
 *  @arg @ref MORPHO_VIDEO_REFINER_MODE_MOVIE_SOLID
 *  @arg @ref MORPHO_VIDEO_REFINER_MODE_MOVIE_SOLID_LITE
 *  @arg @ref MORPHO_VIDEO_REFINER_MODE_VIDEO_DENOISER
 *
 * @param[in] save_inside See also @ref morpho_VideoRefiner_process().
 *  @arg 0: Previous frames are stocked outside the engine.
 *  @arg 1: The engine itself stocks previous frames. 
 *
 * @return The buffer size.
 */
MORPHO_API(int)
morpho_VideoRefiner_getBufferSize( const int in_width, const int in_height, const char *p_format, const int motion, const int accuracy, const int mode, const int save_inside );

/**
 * @brief
 * Initializes the engine.
 *
 * @param[in,out] p_vref
 * @param[in,out] p_buffer
 * @param[in]     buffer_size
 * @param[in]     in_width    Input width.
 * @param[in]     in_height   Input height.
 * @param[in]     crop_width  Cropping width.
 * @param[in]     crop_height Cropping height.
 * @param[in]     p_format    Image format.
 *  @arg "YUV420_SEMIPLANAR"
 *  @arg "YVU420_SEMIPLANAR"
 *  @arg "YUV420_PLANAR"
 *
 * @param[in] motion Motion detection type.
 *  @arg @ref MORPHO_VIDEO_REFINER_MOTION_4DOF_SOFT
 *  @arg @ref MORPHO_VIDEO_REFINER_MOTION_4DOF_HYBRID
 *  @arg @ref MORPHO_VIDEO_REFINER_MOTION_6DOF_SOFT
 *  @arg @ref MORPHO_VIDEO_REFINER_MOTION_6DOF_HYBRID
 *  @arg @ref MORPHO_VIDEO_REFINER_MOTION_8DOF_SOFT
 *  @arg @ref MORPHO_VIDEO_REFINER_MOTION_8DOF_HYBRID
 *  @arg @ref MORPHO_VIDEO_REFINER_MOTION_8DOF_HARD
 *
 * @param[in] accuracy Motion detection accuracy.
 *  @arg @ref MORPHO_VIDEO_REFINER_ACCURACY_HIGH,
 *  @arg @ref MORPHO_VIDEO_REFINER_ACCURACY_MIDDLE,
 *  @arg @ref MORPHO_VIDEO_REFINER_ACCURACY_LOW,
 *
 * @return Error codes.
 */
MORPHO_API(int)
morpho_VideoRefiner_initialize( morpho_VideoRefiner * const p_vref, void *p_buffer, const int buffer_size, 
                                const int in_width, const int in_height, const int crop_width, const int crop_height, const char *p_format,
                                const int motion, const int accuracy );

/**
 * @brief
 * Finalizes the engine.
 *
 * @param[in,out] p_vref
 *
 * @return Error codes.
 */
MORPHO_API(int)
morpho_VideoRefiner_finalize( morpho_VideoRefiner * const p_vref );

/**
 * @brief
 * Specifies the operation mode.
 *
 * @param[in,out] p_vref
 * @param[in] mode
 *  @arg @ref MORPHO_VIDEO_REFINER_MODE_VIDEO_REFINER
 *  @arg @ref MORPHO_VIDEO_REFINER_MODE_MOVIE_SOLID
 *  @arg @ref MORPHO_VIDEO_REFINER_MODE_MOVIE_SOLID_LITE
 *  @arg @ref MORPHO_VIDEO_REFINER_MODE_VIDEO_DENOISER
 *
 * @return Error codes.
 */
MORPHO_API(int)
morpho_VideoRefiner_setMode( morpho_VideoRefiner * const p_vref, const int mode );

/**
 * @brief
 * Sets the number of frame delay.
 *
 * If the delay is set to 0, the engine creates an output image using the input frame at that moment.
 * If set to 1, from the 1-frame-previous one.
 *
 * The recomended and defualt value is 1, for both video quality and processing time reasons.
 *
 * @param[in,out] p_vref
 * @param[in]     num    Delay frame. [0,1]
 *
 * @return Error codes.
 */
MORPHO_API(int)
morpho_VideoRefiner_setDelayFrameNum( morpho_VideoRefiner * const p_vref, const int num );

/**
 * @brief
 * Sets the size of the output image.
 *
 * The engine automatically scales the output image, corresponding to the size set here.
 * When this function is not called, the engine expects that the output is the same size as the cropping size
 * specified at the @ref morpho_VideoRefiner_initialize().
 *
 * @param[in,out] p_vref
 * @param[in]     width  Output width.
 * @param[in]     height Output height.
 *
 * @return Error codes
 */
MORPHO_API(int)
morpho_VideoRefiner_setOutputImageSize( morpho_VideoRefiner * const p_vref, const int width, const int height );

/**
 * @brief
 * Specifies the frame rate.
 *
 * @note
 * This is usable on the fly.
 *
 * @param[in,out] p_vref
 * @param[in]     fps    Frame rate. [fps]
 *
 * @return Error codes.
 */
MORPHO_API(int)
morpho_VideoRefiner_setFrameRate( morpho_VideoRefiner * const p_vref, const int fps );

/**
 * @brief
 * Specifies the view angle.
 *
 * @note
 * This is usable on the fly.
 *
 * @param[in,out] p_vref
 * @param[in]     angle_type Type of the view angle.
 *  @arg @ref MORPHO_VIDEO_REFINER_ANGLE_TYPE_HORIZONTAL
 *  @arg @ref MORPHO_VIDEO_REFINER_ANGLE_TYPE_VERTICAL
 *  @arg @ref MORPHO_VIDEO_REFINER_ANGLE_TYPE_DIAGONAL
 *
 * @param[in]     angle      View angle. [degree]
 *
 * @return Error codes.
 */
MORPHO_API(int)
morpho_VideoRefiner_setViewAngle( morpho_VideoRefiner * const p_vref, const int angle_type, const double angle );

/**
 * @brief
 * Specifies the rolling shutter coefficient.
 *
 * This cooefficient corresponds to the scan-time of the sensor.
 * For a video taken by a CCD camera, the appropriate coefficient is 0.
 * For a CMOS camera, 0 is good for very fast scan, and +100/-100 for very slow scan.
 * The sign of this coefficient reflects the direction of image scan.
 * In many cases, positive values are appropriate.
 *
 * @note
 * This is usable on the fly.
 *
 * @param[in,out] p_vref
 * @param[in]     coeff  Rolling shutter coefficient. [-100,100]
 *
 * @return Error codes
 */
MORPHO_API(int)
morpho_VideoRefiner_setRollingShutterCoeff( morpho_VideoRefiner * const p_vref, const int coeff );

/**
 * @brief
 * Specifies the orientation of the scanline.
 *
 * @param[in,out] p_vref
 * @param[in]     orien
 *  @arg @ref MORPHO_VIDEO_REFINER_SCANLINE_ORIENTATION_HORIZONTAL
 *  @arg @ref MORPHO_VIDEO_REFINER_SCANLINE_ORIENTATION_VERTICAL
 *
 * @return Error codes.
 */
MORPHO_API(int)
morpho_VideoRefiner_setScanlineOrientation( morpho_VideoRefiner * const p_vref, const int orien );

/**
 * @brief
 * Specifies the time lag between image data and the corresponding gyroscope data.
 *
 * In general, the gyroscope data taken between two image frames is not the actual data
 * between the frames. There might be a time lag.
 *
 * Positive values are for cases that the image data delays comparing with the gyroscope.
 * This value strongly affects the quality of the result.
 *
 * @note
 * This is usable on the fly.
 *
 * @param[in,out] p_vref
 * @param[in]     lag    Time lag. [usec]
 *
 * @return Error codes
 */
MORPHO_API(int)
morpho_VideoRefiner_setImageGyroTimeLag( morpho_VideoRefiner * const p_vref, const int lag );

/**
 * @brief
 * Sets the fix level.
 *
 * Larger levels result in more stabilized results.
 *
 * @note
 * This is usable on the fly.
 *
 * @param[in,out] p_vref
 * @param[in]     level  Fix level. [0,7]
 *
 * @return Error codes.
 */
MORPHO_API(int)
morpho_VideoRefiner_setFixLevel( morpho_VideoRefiner * const p_vref, const int level );

/**
 * @brief
 * Sets the moving subject level.
 *
 * When there are big moving subjects in a frame, motion detection might be deceived by these.
 * To avoid undesirable results, the engine changes its behavior for such situations.
 *
 * With larger levels, the engine tends to detect moving subjects more frequently.
 *
 * @param[in,out] p_vref
 * @param[in]     level Moving subject level. [0,7]
 *
 * @return Error codes.
 */
MORPHO_API(int)
morpho_VideoRefiner_setMovingSubjectLevel( morpho_VideoRefiner * const p_vref, const int level );

/**
 * @brief
 * Sets the no movement level.
 *
 * When calculated camera motion is sufficiently small, the engine discards this calculation and
 * processes as if there are completely no motion.
 * This leads to better results for videos taken with the tripod.
 *
 * Larger levels for more frequent no-motion judgements.
 *
 * @param[in,out] p_vref
 * @param[in]     level  No movement level. [0,7]
 *
 * @return Error codes.
 */
MORPHO_API(int)
morpho_VideoRefiner_setNoMovementLevel( morpho_VideoRefiner * const p_vref, const int level );

/**
 * @brief
 * Sets the featureless level.
 *
 * For inputs with little feature points, it is difficult to calculate accurate camera motion.
 * So, in such cases, the engine does not trust calculated motion.
 *
 * Larger levels for more frequent featureless judgements.
 *
 * @note
 * This function does nothing for some versions.
 *
 * @param[in,out] p_vref
 * @param[in]     level  Featureless level. [0,7]
 *
 * @return Error codes.
 */
MORPHO_API(int)
morpho_VideoRefiner_setFeaturelessLevel( morpho_VideoRefiner * const p_vref, const int level );

/**
 * @brief
 * Sets the spatial noise reduction level.
 *
 * @note
 * This is usable on the fly.
 *
 * @param[in,out] p_vref
 * @param[in]     level  Spatial noise reduction level. [0,4]
 *
 * @return Error codes.
 */
MORPHO_API(int)
morpho_VideoRefiner_setSpatialNRLevel( morpho_VideoRefiner * const p_vref, const int level );

/**
 * @brief
 * Sets the temporal noise reduction level for luma channels.
 *
 * @note
 * This is usable on the fly.
 *
 * @param[in,out] p_vref
 * @param[in]     level  Temporal noise reduction level. [0,4]
 *
 * @return Error codes.
 */
MORPHO_API(int)
morpho_VideoRefiner_setTemporalLumaNRLevel( morpho_VideoRefiner * const p_vref, const int level );

/**
 * @brief
 * Sets the temporal noise reduction level for chroma channels.
 *
 * @note
 * This is usable on the fly.
 *
 * @param[in,out] p_vref
 * @param[in]     level  Temporal noise reduction level. [0,4]
 *
 * @return Error codes.
 */
MORPHO_API(int)
morpho_VideoRefiner_setTemporalChromaNRLevel( morpho_VideoRefiner * const p_vref, const int level );


/**
 * @brief
 * Sets the luma enhance level.
 *
 * @note
 * This is usable on the fly.
 *
 * @param[in,out] p_vref
 * @param[in]     level  Enhance level. [0,4]
 *
 * @return Error codes.
 */
MORPHO_API(int)
morpho_VideoRefiner_setLumaEnhanceLevel( morpho_VideoRefiner * const p_vref, const int level );

/**
 * @brief
 * Sets the chroma enhance level.
 *
 * @note
 * This is usable on the fly.
 *
 * @param[in,out] p_vref
 * @param[in]     level  Enhance level. [0,4]
 *
 * @return Error codes.
 */
MORPHO_API(int)
morpho_VideoRefiner_setChromaEnhanceLevel( morpho_VideoRefiner * const p_vref, const int level );


/**
 * Obtains the number of delay frames.
 *
 * @param[in] p_vref
 * @param[out] p_num
 * @return Error Code
 */
MORPHO_API(int)
morpho_VideoRefiner_getDelayFrameNum( morpho_VideoRefiner * const p_vref, int * const p_num );

/**
 * Obtains the size of output images.
 *
 * @param[in] p_vref
 * @param[out] p_width
 * @param[out] p_height
 * @return Error Code
 */
MORPHO_API(int)
morpho_VideoRefiner_getOutputImageSize( morpho_VideoRefiner * const p_vref, int* const p_width, int* const p_height );

/**
 * Sets the frame rate of the input video.
 *
 * @param[in] p_vref
 * @param[out] p_fps
 * @return Error Code
 */
MORPHO_API(int)
morpho_VideoRefiner_getFrameRate( morpho_VideoRefiner * const p_vref, int* const p_fps );

/**
 * Obtains the view angle of the input video images.
 *
 * @param[in] p_vref
 * @param[out] p_angle_type
 * @param[out] p_angle
 * @return Error Code
 */
MORPHO_API(int)
morpho_VideoRefiner_getViewAngle( morpho_VideoRefiner * const p_vref, int* const p_angle_type, double* const p_angle );

/**
 * Obtains the coefficient used for the correction of rolling shutter distortion.
 *
 * @param[in] p_vref
 * @param[out] p_coeff
 * @return Error Code
 */
MORPHO_API(int)
morpho_VideoRefiner_getRollingShutterCoeff( morpho_VideoRefiner * const p_vref, int* const p_coeff );

/**
 * Obtains the orientation of scanline.
 *
 * @param[in] p_vref
 * @param[out] p_orien
 * @return Error Code
 */
MORPHO_API(int)
morpho_VideoRefiner_getScanlineOrientation( morpho_VideoRefiner * const p_vref, int* const p_orien );

/**
 * Obtains the time-lag setting between image capture and gyro data.
 *
 * @param[in] p_vref
 * @param[out] p_lag
 * @return Error Code
 */
MORPHO_API(int)
morpho_VideoRefiner_getImageGyroTimeLag( morpho_VideoRefiner * const p_vref, int* const p_lag );

/**
 * Obtains fix level.
 *
 * @param[in] p_vref
 * @param[out] p_level
 * @return Error Code
 */
MORPHO_API(int)
morpho_VideoRefiner_getFixLevel( morpho_VideoRefiner * const p_vref, int* const p_level );

/**
 * Obtains the threshold for moving subject error.
 *
 * @param[in] p_vref
 * @param[in] p_level
 * @return Error Code
 */
MORPHO_API(int)
morpho_VideoRefiner_getMovingSubjectLevel( morpho_VideoRefiner * const p_vref, int* const p_level );

/**
 * Obtains the threshold for no movement condition.
 *
 * @param[in] p_vref
 * @param[out] p_level
 * @return Error Code
 */
MORPHO_API(int)
morpho_VideoRefiner_getNoMovementLevel( morpho_VideoRefiner * const p_vref, int* const p_level );

/**
 * Obtains the threshold for featureless condition.
 *
 * @param[in] p_vref
 * @param[out] p_level
 * @return Error Code
 */
MORPHO_API(int)
morpho_VideoRefiner_getFeaturelessLevel( morpho_VideoRefiner * const p_vref, int* const p_level );

/**
 * Obtains the noise reduction lecel (spacial).
 *
 * @param[in] p_vref
 * @param[out] p_level
 * @return Error Code
 */
MORPHO_API(int)
morpho_VideoRefiner_getSpatialNRLevel( morpho_VideoRefiner * const p_vref, int* const p_level );

/**
 * Obtains the luma noise reduction level (temporal).
 *
 * @param[in] p_vref
 * @param[out] p_level
 * @return Error Code
 */
MORPHO_API(int)
morpho_VideoRefiner_getTemporalLumaNRLevel( morpho_VideoRefiner * const p_vref, int* const p_level );

/**
 * Obtains the chroma noise reduction level (temporal).
 *
 * @param[in] p_vref
 * @param[out] p_level
 * @return Error Code
 */
MORPHO_API(int)
morpho_VideoRefiner_getTemporalChromaNRLevel( morpho_VideoRefiner * const p_vref, int* const p_level );

/**
 * Obtains the luma enhance level.
 *
 * @param[in] p_vref
 * @param[out] p_level
 * @return Error Code
 */
MORPHO_API(int)
morpho_VideoRefiner_getLumaEnhanceLevel( morpho_VideoRefiner * const p_vref, int* const p_level );

/**
 * Obtains the chrome enhance level.
 *
 * @param[in] p_vref
 * @param[out] p_level
 * @return Error Code
 */
MORPHO_API(int)
morpho_VideoRefiner_getChromaEnhanceLevel( morpho_VideoRefiner * const p_vref, int* const level );


/**
 * @brief
 * Starts the engine.
 *
 * @param[in,out] p_vref
 *
 * @return Error codes.
 */
MORPHO_API(int)
morpho_VideoRefiner_start( morpho_VideoRefiner * const p_vref );

/**
 * @brief
 * Inputs the gyroscope data obtained between the previous and the current frames.
 *
 * @param[in,out] p_vref
 * @param[in]     p_rot_x Angular velocity around x-axis. [rad/sec]
 * @param[in]     p_rot_y Angular velocity around y-axis. [rad/sec]
 * @param[in]     p_rot_z Angular velocity around z-axis. [rad/sec]
 * @param[in]     p_time  Time stamps. [usec]
 * @param[in]     num     Data size.
 *
 * @return Error codes.
 */
MORPHO_API(int)
morpho_VideoRefiner_updateGyroData( morpho_VideoRefiner * const p_vref, double * const p_rot_x, double * const p_rot_y, double * const p_rot_z,
                                    int * const p_time, const int num );

/**
 * @brief
 * Processes the current frame.
 *
 * Basically, this function requires the data of previous frames, in addition to the current one.
 * And the engine expect the caller to stock these previous frames, using the ring buffer technique.
 *
 * The engine might alters the image buffers designated by p_src_img and p_prv_img. And p_src_img and 
 * p_prv_img of the previous call must be coinside with p_prv_img and p_pr2_img of this call, respectively. 
 *
 * The image p_pr2_img is not required for 0-frame-delay cases.
 * @see morpho_VideoRefiner_setDelayFrameNum()
 *
 * @note
 * When this is called with p_prv_img == NULL and p_pr2_img == NULL, the engine stocks
 * requisite previous frames internally. So, in that case, the caller does not need to care about preserving
 * frames, although the processing time is slow.
 *
 * A schematic implementation example for a case of 1-frame-delay is:
 * @code
 * morpho_ImageData a_src_img[3];
 * morpho_ImageData dst_img;
 * 
 * int index = 0;
 *
 * while( 1 ) {
 *     const int index_cur = index;
 *     const int index_prv = ( index + 2 ) % 3;
 *     const int index_pr2 = ( index + 1 ) % 3;
 *
 *     if( load_current_frame( &a_src_img[index_cur] ) ) break;
 *
 *     morpho_VideoRefiner_process( p_vref, &dst_img, &a_src_img[index_cur], &a_src_img[index_prv], &a_src_img[index_prv2] );
 *
 *     index = ( index + 1 ) % 3;
 * }
 * @endcode
 *
 * For a case of 0-frame-delay:
 * @code
 * morpho_ImageData a_src_img[2];
 * morpho_ImageData dst_img;
 * 
 * int index = 0;
 *
 * while( 1 ) {
 *     const int index_cur = index;
 *     const int index_prv = ( index + 1 ) % 2;
 *
 *     if( load_current_frame( &a_src_img[index_cur] ) ) break;
 *
 *     morpho_VideoRefiner_process( p_vref, &dst_img, &a_src_img[index_cur], &a_src_img[index_prv], NULL );
 *
 *     index = ( index + 1 ) % 2;
 * }
 * @endcode
 *
 * Although it is slow, the follwing is also acceptable:
 * @code
 * morpho_ImageData src_img;
 * morpho_ImageData dst_img;
 * 
 * while( 1 ) {
 *     if( load_current_frame( &src_img ) ) break;
 *
 *     morpho_VideoRefiner_process( p_vref, &dst_img, &src_img, NULL, NULL );
 * }
 * @endcode
 *
 * @param[in,out] p_vref
 * @param[out]    p_dst_img Output image.
 * @param[in,out] p_src_img Current image.
 * @param[in,out] p_prv_img 1-frame-previous image.
 * @param[in]     p_pr2_img 2-frame-previous image.
 *
 * @return Error codes.
 */
MORPHO_API(int)
morpho_VideoRefiner_process( morpho_VideoRefiner * const p_vref, morpho_ImageData * const p_dst_img, morpho_ImageData * const p_src_img, 
                             morpho_ImageData * const p_prv_img, morpho_ImageData * const p_pr2_img );

/**
 * @brief
 * Returns the failure code of the most recent process.
 *
 * The code represents the disjunction of all applicable failure codes.
 *
 * @param[in,out] p_vref
 * @param[out]    p_fcode
 *  @arg @ref MORPHO_VIDEO_REFINER_FAILURE_NO_ERROR
 *  @arg @ref MORPHO_VIDEO_REFINER_FAILURE_OUT_OF_RANGE
 *  @arg @ref MORPHO_VIDEO_REFINER_FAILURE_NO_MOVEMENT
 *  @arg @ref MORPHO_VIDEO_REFINER_FAILURE_MOVING_SUBJECT
 *  @arg @ref MORPHO_VIDEO_REFINER_FAILURE_FEATURELESS
 *  @arg @ref MORPHO_VIDEO_REFINER_FAILURE_GENERIC
 *
 * @return Error codes.
 */
MORPHO_API(int)
morpho_VideoRefiner_getFcode( morpho_VideoRefiner * const p_vref, int * const p_fcode );

#ifdef __cplusplus
}
#endif

#endif /* MORPHO_VIDEO_REFINER_H */
