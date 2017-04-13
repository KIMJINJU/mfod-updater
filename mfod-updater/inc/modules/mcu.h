/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    mcu.h
        external function/variables/defines for MCU interface
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _MCU_H_
#define _MCU_H_

#include <stdbool.h>


/* macro defines : filter position */
#define MCU_SHUTTER_CLOSE               0x00
#define MCU_SHUTTER_LXMVT               0x01
#define MCU_SHUTTER_HXMVT               0x02
#define MCU_SHUTTER_OPEN                0x03
#define MCU_SHUTTER_UNKNOWN             0x04

/* macro defines : proximity threshold mode */
#define MCU_PROXTHOLD_OPEN				0x00
#define MCU_PROXTHOLD_CLOSE				0x01

/* macro define : power source type */
#define MCU_PWRSRC_CELL                 0
#define MCU_PWRSRC_EXTDC                1

/* macro define : cell type */
#define MCU_CELL_SECONDARY              0
#define MCU_CELL_PRIMARY                1

/* macro defines : mcu error code */
#define MCU_ERROR_NONE                  0x0000
#define MCU_ERROR_COMM                  0x0001
#define MCU_ERROR_SHUTTER               0x0002


/* structure declaration : mcu interface */
struct mcu_interface
{
    int (*standby)          (struct mcu_interface *this);
    int (*wakeup)           (struct mcu_interface *this);
    int (*getError)         (struct mcu_interface *this);
    int (*testModule)       (struct mcu_interface *this);
    int (*getVersion)       (struct mcu_interface *this);
    int (*getBatteryLevel)  (struct mcu_interface *this);
    int (*getBatteryType)   (struct mcu_interface *this);
    int (*getPwrSrcType)    (struct mcu_interface *this);
    int (*getPwrSrcVolt)    (struct mcu_interface *this);
    int (*getShutterPos)    (struct mcu_interface *this);
    int (*setShutterPos)    (struct mcu_interface *this, int pos);
    int (*disablePmic)      (struct mcu_interface *this);
    int (*enterPwrSaveMode) (struct mcu_interface *this);

#ifdef _MCU_INTERNAL_INTERFACE_

#endif
};

/* external functions : create/destroy mcu interface */
extern struct mcu_interface *mcu_create(void);
extern int mcu_destroy(struct mcu_interface *mcu);

#endif /* _MCU_H_ */
