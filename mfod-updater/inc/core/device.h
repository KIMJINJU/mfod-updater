/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    device.h
        external function/variables/defines for device interface
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _DEVICE_H_
#define _DEVICE_H_


#include "modules/display.h"
#include "modules/dmc.h"
#include "modules/gnss.h"
#include "modules/ircam.h"
#include "modules/lrf.h"
#include "modules/mcu.h"
#include "modules/vgss.h"
#include "modules/sdcard.h"
#include "etc/ispmgr.h"

/* constant macro defines - power state flag    */
#define POWER_MODE_NORMAL               0
#define POWER_MODE_STANDBY              1
#define POWER_MODE_WAKEUP_INPROC        2

/* constant macro defines - bit flag    */
#define DEVICE_PBIT                0
#define DEVICE_IBIT                1
#define DEVICE_CBIT                2

/* constant macro defines - test result */
#define DEVICE_ERROR_NONE               0x00000000
#define DEVICE_ERROR_MEMTEST            0x00000001
#define DEVICE_ERROR_GNSS_COMM          0x00000002
#define DEVICE_ERROR_DMC_COMM           0x00000004
#define DEVICE_ERROR_DMC_MCPU           0x00000008
#define DEVICE_ERROR_LRF_XMTR           0x00000010
#define DEVICE_ERROR_LRF_COMM           0x00000020
#define DEVICE_ERROR_IRCAM_CAMIF        0x00000040
#define DEVICE_ERROR_IRCAM_COMM         0x00000080
#define DEVICE_ERROR_IRCAM_IRSHTR       0x00000100
#define DEVICE_ERROR_MCU_COMM           0x00000200
#define DEVICE_ERROR_MCU_NDFILTER       0x00000400


/* constant macro defines - bit progress flag */
#define DEVICE_BIT_PROGRESS_STANDBY     0
#define DEVICE_BIT_PROGRESS_INPROC      1
#define DEVICE_BIT_PROGRESS_DONE        2

#define DEVICE_BIT_RESULT_OK            0
#define DEVICE_BIT_RESULT_ERROR         1

#define DEVICE_MODULE_MEMORY            0
#define DEVICE_MODULE_LRF               1
#define DEVICE_MODULE_DMC               2
#define DEVICE_MODULE_GNSS              3
#define DEVICE_MODULE_DVO               4
#define DEVICE_MODULE_IRCAM             5
#define DEVICE_MODULE_BATTERY           6

#define DEVICE_NUM_BIT_ITEMS            6



/* structure declarations : device interface */
struct device_interface
{
    int (*executeBit)      (struct device_interface *);
    int (*getBitProgress)  (struct device_interface *, int);
    int (*getBitResult)    (struct device_interface *, int);
    int (*getModuleStatus) (struct device_interface *, int);
    int (*setModuleStatus) (struct device_interface *, int);
    int (*standby)         (struct device_interface *);
    int (*wakeup)          (struct device_interface *);
    int (*suspend)         (struct device_interface *);
    int (*setPowerMode)    (struct device_interface *, int);
    int (*getPowerMode)    (struct device_interface *);
    int (*getSystemTime)   (struct device_interface *, char *, char *, int, int);
    int (*updateSystemTime)(struct device_interface *);
    int (*saveUtcOffset)   (struct device_interface *, int);

    struct display_interface *disp;
    struct dmc_interface *dmc;
    struct gnss_interface *gnss;
    struct ircam_interface *ircam;
    struct lrf_interface *lrf;
    struct mcu_interface *mcu;
    struct vgss_interface *vgss;
    struct sdcard_interface *sdc;
    struct ispmgr_interface *ispmgr;

#ifdef _DEVICE_INTERNAL_INTERFACE_
#endif
};

/* external functions for create/destroy device interface */
extern struct device_interface *device_create(void);
extern int device_destroy(struct device_interface *dev);

#endif /* _DEVICE_H_ */
