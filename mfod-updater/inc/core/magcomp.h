/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    magcomp.h
        magnetic compensator interface declarations and defines.
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _MAGCOMP_H_
#define _MAGCOMP_H_

#include "modules/dmc.h"

/* constant macro define : magnetic compensation geometry */
#define MAGCOMP_GEOMETRY_TRIPOD     0
#define MAGCOMP_GEOMETRY_ELO        1

/* constant macro define : move direction */
#define MAGCOMP_MOVE_DIR_STOP		0	/* stop                     */
#define MAGCOMP_MOVE_DIR_CW			1	/* clockwise                */
#define MAGCOMP_MOVE_DIR_CCW		2	/* counter clockwise        */
#define MAGCOMP_MOVE_DIR_UP			3	/* up                       */
#define MAGCOMP_MOVE_DIR_DOWN		4	/* down                     */
#define MAGCOMP_MOVE_DIR_TCW		5	/* tilt clockwise           */
#define MAGCOMP_MOVE_DIR_TCCW		6	/* tilt counter clockwise   */

/* constant macro define : QCP */
#define MAGCOMP_QCP00               0x00
#define	MAGCOMP_QCP01       	    0x01
#define	MAGCOMP_QCP02       	    0x02
#define	MAGCOMP_QCP03       	    0x03
#define	MAGCOMP_QCP04       	    0x04
#define	MAGCOMP_QCP05       	    0x05
#define	MAGCOMP_QCP06       	    0x06
#define	MAGCOMP_QCP07       	    0x07
#define	MAGCOMP_QCP08       	    0x08
#define	MAGCOMP_QCP09       	    0x09
#define	MAGCOMP_QCP10       	    0x0A
#define	MAGCOMP_QCP11       	    0x0B
#define MAGCOMP_QCP12			    0x0C

/* constant macro define : magnetic compensator task */
#define MAGCOMP_TASK_IDLE		    0x00
#define	MAGCOMP_TASK_ALIGN	        0x01
#define MAGCOMP_TASK_QCP		    0x02
#define MAGCOMP_TASK_ENDQCP		    0x03
#define	MAGCOMP_TASK_EXIT           0x04


/* structure declarations : magnetic compensator interface */
typedef struct magcomp_interface
{
    int (*setGeometry)          (struct magcomp_interface *, int);
    int (*getGeometry)          (struct magcomp_interface *);
    int (*getDirection)         (struct magcomp_interface *);
    int (*getPosition)          (struct magcomp_interface *, int *, int *, int *);
    int (*getCurrentQcp)        (struct magcomp_interface *);
    int (*getCurrentTask)       (struct magcomp_interface *);
    int (*acquireCompdata)      (struct magcomp_interface *);
    int (*startCompensation)    (struct magcomp_interface *);
    int (*terminateCompensation)(struct magcomp_interface *);

#ifdef _ENABLE_MAGCOMP_INTERNAL_INTERFACE_

#endif  /* _ENABLE_MAGCOMP_INTERNAL_INTERFACE_ */
}
magcomp_t;

/* external functions for create/destroy magcomp interface */
extern struct magcomp_interface *magcomp_create(struct dmc_interface *);
extern int magcomp_destroy(struct magcomp_interface *);

#endif /* _MAGCOMP_H_ */
