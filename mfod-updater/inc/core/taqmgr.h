/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    taqmgr.h
        external function/variables/defines for target acquisition manager
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _TAQMGR_H_
#define _TAQMGR_H_

#include "core/taqdata_manager.h"
#include "modules/dmc.h"
#include "modules/gnss.h"
#include "modules/lrf.h"


/* macro define : coordinate system format */
#define COORDSYS_GEODETIC               0
#define COORDSYS_MGRS                   1
#define COORDSYS_UTM                    2

/* macro define : DOP level   */
#define DOPLV_POOR                      0
#define DOPLV_FAIR                      1
#define DOPLV_MODERATE                  2
#define DOPLV_GOOD                      3
#define DOPLV_EXCELLENT                 4
#define DOPLV_NONE

/* macro defines : target acquisition mode */
#define TAQMODE_POINT_TARGET            0
#define TAQMODE_MOVING_TARGET           1
#define TAQMODE_CIRCULAR_TARGET         2
#define TAQMODE_SQUARE_TARGET_LENGTH    3
#define TAQMODE_SQUARE_TARGET_WIDTH     4
#define TAQMODE_FOS_CORRECTION          5

/* constant macro defines : taqmgr status */
#define TAQMGR_STATUS_IDLE              0
#define TAQMGR_STATUS_INPROC            1
#define TAQMGR_STATUS_TARGET_ACQUIRED   2

/* macro defines : target acquistion errors */
#define TAQERROR_NONE                   0x00
#define TAQERROR_GNSS_OFFLINE           0x01
#define TAQERROR_LRF_OFFLINE            0x02
#define TAQERROR_DMC_OFFLINE            0x04
#define TAQERROR_ZERO_RANGE             0x08
#define TAQERROR_NOT_INPROC             0x10
#define TAQERROR_ANGLE_LIMIT            0x20
#define TAQERROR_START_MEASRUING_RANGE  0x40
#define TAQERROR_GRIDVAR                0x80

/* macro defines : azimuth refernce */
#define AZMODE_MAGNETIC_NORTH           0
#define AZMODE_GRID_NORTH               1
#define AZMODE_TRUE_NORTH               2

/* constant macro defines : magnetic declination */
#define MAGDECL_MAX                     90.0
#define MAGDECL_MIN                     -90.0

/* constant macro defines : grid variation */
#define GRIDVAR_MAX                     11.25
#define GRIDVAR_MIN                     -11.25

/* constant macro defines : azimuth offset index */
#define AZOFF_DV_PCELL                  0
#define AZOFF_DV_SCELL                  1
#define AZOFF_DV_EXTDC                  2
#define AZOFF_IR_PCELL                  3
#define AZOFF_IR_SCELL                  4
#define AZOFF_IR_EXTDC                  5
#define AZOFF_ZERO                      6

/* constant marcro defines : userdata for taq flag */
#define USE_DEVICE_ACQUIRED_DATA        0x00
#define USE_USER_INPUT_RANGE            0x01
#define USE_USER_INPUT_OBLOC            0x02
#define USE_USER_INPUT_GRIDVAR          0x04

/* constant macro defines */
#define GEODATA_USER_DEFAULT_LATITUDE	37.480387
#define GEODATA_USER_DEFAULT_LONGITUDE	126.678369
#define GEODATA_USER_DEFAULT_GRIDVAR    0.0
#define GEODATA_USER_DEFAULT_MAGDECL    0.0

/* constant macro defines : taq return codes */
#define TAQ_RETCODE_OK                  0
#define TAQ_RETCODE_FAIL_GEODCALC       -2
#define TAQ_RETCODE_INVALID_TAQMODE     -3
#define TAQ_RETCODE_SHIFT_RANGE_OVER    -4
#define TAQ_RETCODE_TARGET_SIZE_OVER    -5
#define TAQ_RETCODE_ZERO_RANGE          -6
#define TAQ_RETCODE_DMC_ERROR           -7
#define TAQ_RETCODE_GNSS_ERROR          -8
#define TAQ_RETCODE_ERROR               -9

/* constant macro defines : manual rangine status */
#define MANUAL_RANGING_IDLE             0
#define MANUAL_RANGING_INPROC           1


#define FIX_OBLOC_NONE                  0
#define FIX_OBLOC_GNSS                  1
#define FIX_OBLOC_USER                  2


/* structure declarations : target acquisition manager */
typedef struct taqmgr_interface
{
    int (*getAzMode)        (struct taqmgr_interface *);
    int (*setAzMode)        (struct taqmgr_interface *, int );
    int (*getRange)         (struct taqmgr_interface *, int *, int *);
    int (*getAzimuth)       (struct taqmgr_interface *, int *, double *);
    int (*getBank)          (struct taqmgr_interface *, int *, double *);
    int (*getElevation)     (struct taqmgr_interface *, int *, double *);
    int (*setTaqMode)       (struct taqmgr_interface *, int);
    int (*getTaqMode)       (struct taqmgr_interface *);
    int (*setCoordSystem)   (struct taqmgr_interface *, int);
    int (*getCoordSystem)   (struct taqmgr_interface *);
    int (*getDopLevel)      (struct taqmgr_interface *);
    int (*getError)         (struct taqmgr_interface *);
    int (*isOnline)         (struct taqmgr_interface *);
    int (*getStatus)        (struct taqmgr_interface *);
    int (*setStatus)        (struct taqmgr_interface *, int);
    int (*getUserData)      (struct taqmgr_interface *);
    int (*getMrst)          (struct taqmgr_interface *);
    int (*getGridCvg)       (struct taqmgr_interface *, double *);
    int (*getMagDecl)       (struct taqmgr_interface *, double *);
    int (*setGridVar)       (struct taqmgr_interface *, double);
    int (*getGridVar)       (struct taqmgr_interface *, double *);
    int (*setUserOrigin)    (struct taqmgr_interface *, char *, int);
    int (*getUserOrigin)    (struct taqmgr_interface *, char *, int *);
    int (*setUserRange)     (struct taqmgr_interface *, int);
    int (*getUserRange)     (struct taqmgr_interface *, int *);
    int (*getOrigin)        (struct taqmgr_interface *, double *, double *, double *);
    int (*getAngleOffset)   (struct taqmgr_interface *, double *, double *);
    int (*start)            (struct taqmgr_interface *);
    int (*finalize)         (struct taqmgr_interface *);
    int (*terminate)        (struct taqmgr_interface *);
    int (*reset)            (struct taqmgr_interface *);
    int (*getResult)        (struct taqmgr_interface *, taqdata_t *);
    int (*saveResult)       (struct taqmgr_interface *);
    int (*getDistOffsetSH) 	(struct taqmgr_interface *, double *, double *);
    int (*setDistOffsetSH)	(struct taqmgr_interface *, char *, char *);
    int (*getDistOffsetMD)	(struct taqmgr_interface *, double *, double *);
    int (*setDistOffsetMD)	(struct taqmgr_interface *, char *, char *);
    int (*getDistOffsetLG)	(struct taqmgr_interface *, double *, double *);
    int (*setDistOffsetLG)	(struct taqmgr_interface *, char *, char *);
    int (*getAngleOffsetAT)	(struct taqmgr_interface *, double *);
    int (*setAngleOffsetAT) (struct taqmgr_interface *, char *);
    int (*getAngleOffsetAZ)	(struct taqmgr_interface *, double *);
    int (*setAngleOffsetAZ) (struct taqmgr_interface *, char *);
    int (*getIrArrayOffset) (struct taqmgr_interface *, unsigned char *, unsigned char *);
    int (*setIrArrayOffset) (struct taqmgr_interface *, char *, char *);
    int (*getSensorOffset)	(struct taqmgr_interface *, unsigned char *, unsigned char *);
    int (*setSensorOffset)	(struct taqmgr_interface *, char *, char *);
    int (*setIrScaleUp)     (struct taqmgr_interface *, char *);
    int (*getIrScaleUp)		(struct taqmgr_interface *, int *);
    int (*getCoordString) 	(struct taqmgr_interface *);

    taqdata_manager_t *(*getTargetList) (struct taqmgr_interface *);

#ifdef _ENABLE_TAQMGR_INTERNAL_INTERFACE_
#endif /* _ENABLE_TAQMGR_INTERNAL_INTERFACE_ */
}
taqmgr_t;


/* externanl functions for create/destroy target acquisition interface */
extern struct taqmgr_interface *taqmgr_create(struct dmc_interface *, struct gnss_interface *, struct lrf_interface *);
extern int taqmgr_destroy(struct taqmgr_interface *taqmgr);

#endif /* _TAQMGR_H_ */
