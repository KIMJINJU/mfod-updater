/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    ircam.h
        external function/variables/defines for IRCAM interface
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#ifndef _IRCAM_H_
#define _IRCAM_H_

#include <stdbool.h>


/* macro defines : nuc status */
#define IRCAM_NUC_IDLE          0
#define IRCAM_NUC_READY         1
#define IRCAM_NUC_INPROC        2

/* macro defines : thermal image polarity */
#define IRCAM_IRPOL_WHITE       0
#define IRCAM_IRPOL_BLACK       1

/* macro defines : histogram equalization method */
#define IRCAM_HISTEQ_LOCAL      0
#define IRCAM_HISTEQ_GLOBAL     1

/* macro defines : gain mode */
#define IRCAM_GAINMODE_DEFAULT  0
#define IRCAM_GAINMODE_HIGH     1

/* macro defines : shutter selection */
#define IRCAM_EXTERNAL_SHUTTER  0
#define IRCAM_INTERNAL_SHUTTER  1

/* macro define : power status */
#define IRCAM_POWER_DISABLE     0
#define IRCAM_POWER_BOOTING     1
#define IRCAM_POWER_ENABLE      2

/* macro defines : pseudo colors */
#define IRCAM_COLOR_SEPIA       4
#define IRCAM_COLOR_SPECTRUM    7
#define IRCAM_COLOR_MONO        8
#define IRCAM_COLOR_ISOTHREM    9

/* macro defines : tecless data index */
#define IRCAM_TECDATA_LOW       0
#define IRCAM_TECDATA_NORMAL    1
#define IRCAM_TECDATA_HIGH      2

/* macro defines : IRCAM error code */
#define IRCAM_ERROR_NONE        0x00
#define IRCAM_ERROR_BOOT        0x01
#define IRCAM_ERROR_DETECTOR    0x02
#define IRCAM_ERROR_COMM        0x04
#define IRCAM_ERROR_IRSHTR      0x08

/* macro defines : move */
#define IRCAM_MOVE_RIGHT		0x00
#define IRCAM_MOVE_LEFT			0x01
#define IRCAM_MOVE_UP			0x02
#define IRCAM_MOVE_DOWN			0x03

/* macro  defines : array mode */
#define IRCAM_ARRAY_START		0x01
#define IRCAM_ARRAY_SAVEEND		0x00



/* structure declaration : IRCAM interface */
struct ircam_interface
{
    int (*enablePower)     (struct ircam_interface *ircam, bool flag);
    int (*updateNuc)       (struct ircam_interface *ircam, int flag);
    int (*testModule)      (struct ircam_interface *ircam);
    int (*setPolarity)     (struct ircam_interface *ircam, int irpol);
    int (*setContrast)     (struct ircam_interface *ircam, int val);
    int (*setBright)       (struct ircam_interface *ircam, int val);
    int (*setEdge)         (struct ircam_interface *ircam, int val);
    int (*setGain)         (struct ircam_interface *ircam, int val);
    int (*setLevel)        (struct ircam_interface *ircam, int val);
    int (*setColor)        (struct ircam_interface *ircam, int val);
    int (*setGainMode)     (struct ircam_interface *ircam, int mode);
    int (*setHistEq)       (struct ircam_interface *ircam, int mode);
    int (*setTecdata)      (struct ircam_interface *ircam, int idx);
    int (*setShtrMode)     (struct ircam_interface *ircam, int flag);
    int (*setNucReady)     (struct ircam_interface *ircam);
    int (*getNucStatus)    (struct ircam_interface *ircam);
    int (*getPolarity)     (struct ircam_interface *ircam);
    int (*getContrast)     (struct ircam_interface *ircam);
    int (*getBright)       (struct ircam_interface *ircam);
    int (*getEdge)         (struct ircam_interface *ircam);
    int (*getGain)         (struct ircam_interface *ircam);
    int (*getLevel)        (struct ircam_interface *ircam);
    int (*getColor)        (struct ircam_interface *ircam);
    int (*getGainMode)     (struct ircam_interface *ircam);
    int (*getHistEq)       (struct ircam_interface *ircam);
    int (*getTecdata)      (struct ircam_interface *ircam);
    int (*getError)        (struct ircam_interface *ircam);
    int (*getShtrMode)     (struct ircam_interface *ircam);
    int (*moveIr)		   (struct ircam_interface *ircam, int direction);
    int (*arrayIr)		   (struct ircam_interface *ircam, int mode);

#ifdef _IRCAM_INTERNAL_INTERFACE_

#endif
};


/* external functions for create/destroy IRCAM interface */
extern struct ircam_interface *ircam_create(void);
extern int ircam_destroy(struct ircam_interface *ircam);

#endif /* _IRCAM_H_ */
