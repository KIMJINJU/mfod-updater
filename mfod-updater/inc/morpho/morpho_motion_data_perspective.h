/**
 * @file   morpho_motion_data_perspective.h
 * @brief  Motion data for Morpho Video Image Stabilization for Tripod Mode
 *
 * Copyright (C) 2012 Morpho, Inc.
 */
#ifndef MORPHO_MOTION_DATA_PERSPECTIVE_H
#define MORPHO_MOTION_DATA_PERSPECTIVE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int fcode;
    double v[9];
}morpho_MotionDataPerspective;

#ifdef __cplusplus
}
#endif


#endif /* MORPHO_MOTION_DATA_PERSPECTIVE_H */
