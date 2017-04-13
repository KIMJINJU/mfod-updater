/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    dmc.h
        external function/variables/defines for DMC interface
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _DMC_H_
#define _DMC_H_

#include <stdbool.h>

/* constant macro defines : magnetic compensation parameter index */
#define MAGPARM_FACTORY             0
#define MAGPARM_DV_PCELL            1
#define MAGPARM_IR_PCELL            2
#define MAGPARM_DV_SCELL            3
#define MAGPARM_IR_SCELL            4
#define MAGPARM_DV_EXTDC            1
#define MAGPARM_IR_EXTDC            2
#define MAGPARM_USER1               5
#define MAGPARM_USER2               6

/* constant macro defines : DMC ERROR FLAG */
#define DMC_ERROR_NONE              0x0000
#define DMC_ERROR_COMM              0x0001
#define DMC_ERROR_MCPU              0x0002
#define DMC_ERROR_MAGPARM           0x0004
#define DMC_ERROR_QCP               0x0008

/* constant macro defines : measuring mode */
#define DMC_CONTINUOUS_MEASURE      0
#define DMC_SINGLE_MEASURE          1

/* constant macro defines : DAMPING FACTOR */
#define DMC_DAMPING_MIN             0x00
#define DMC_DAMPING_MAX             0xFF
#define DMC_DAMPING_DEFAULT         0x04
#define DMC_DAMPING_NONE            0x00

/* constant macro defines : data integration time */
#define DMC_ITIME_MIN               0x00
#define DMC_ITIME_MAX               0xF0
#define DMC_ITIME_DEFAULT           0x02
#define DMC_ITIME_TAQ               0x01

#define DMC_CONFIG_NORMAL           0x00
#define DMC_CONFIG_COMPMODE         0x01

/* structure declaration : dmc interface */
struct dmc_interface
{
    int (*getError)     (struct dmc_interface *this);
    int (*standby)      (struct dmc_interface *this);
    int (*wakeup)       (struct dmc_interface *this);
    int (*reset)        (struct dmc_interface *this);
    int (*measure)      (struct dmc_interface *this, int mode);
    int (*setItime)     (struct dmc_interface *this, unsigned char itime);
    int (*setDamping)   (struct dmc_interface *this, unsigned char damping);
    int (*recallMagParm)(struct dmc_interface *this, int index);
    int (*storeMagParm) (struct dmc_interface *this, int index);
    int (*getMagParm)   (struct dmc_interface *this);
    int (*enterCompMode)(struct dmc_interface *this);
    int (*exitCompMode) (struct dmc_interface *this);
    int (*queryCompData)(struct dmc_interface *this);
    int (*getNcp)       (struct dmc_interface *this);
    int (*getFom)       (struct dmc_interface *this);
    int (*waitCompParm) (struct dmc_interface *this);
    int (*configure)    (struct dmc_interface *this, int);
    int (*getAzimuth)   (struct dmc_interface *this, int *mil, double *deg);
    int (*getElevation) (struct dmc_interface *this, int *mil, double *deg);
    int (*getBank)      (struct dmc_interface *this, int *mil, double *deg);
    int (*testModule)   (struct dmc_interface *this);
    int (*accumulate)   (struct dmc_interface *this, bool flag);
    int (*getAccIndex)  (struct dmc_interface *this);
    int (*getAccBuf)    (struct dmc_interface *this, int **azbuf, int **elbuf, int **bkbuf);

#ifdef _DMC_INTERNAL_INTERFACE_
#endif
};


/* external functions : dmc interface */
extern struct dmc_interface *dmc_create(void);
extern int dmc_destroy(struct dmc_interface *dmc);

#endif /* _DMC_H_ */
