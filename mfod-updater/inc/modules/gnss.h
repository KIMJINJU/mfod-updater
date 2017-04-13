/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gnss.h
        external function/variables/defines for GNSS interface
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _GNSS_H_
#define _GNSS_H_

/* header files to be included for use this interface */
#include <stdbool.h>
#include <time.h>

/* constant macro defines : GNSS fix flag */
#define GNSS_FIX_NONE           1
#define GNSS_FIX_2D             2
#define GNSS_FIX_3D             3

/* constant macro defines : GNSS id flag */
#define GNSS_GPS                0
#define GNSS_GLONASS            1

/* constant macro defines : GNSS error flag */
#define GNSS_ERROR_NONE         0x0000
#define GNSS_ERROR_COMM         0x0001


/* structure declaration : GNSS interface */
struct gnss_interface
{
    int (*getLatitude) (struct gnss_interface *, int *, int *, double *, char *);
    int (*getLongitude)(struct gnss_interface *, int *, int *, double *, char *);
    int (*getAltitude) (struct gnss_interface *, double *);
    int (*getDOP)      (struct gnss_interface *, double *, double *, double *);
    int (*getNSU)      (struct gnss_interface *, int *nsu);
    int (*getNSV)      (struct gnss_interface *, int *, int *);
    int (*getUTC)      (struct gnss_interface *, time_t *);
    int (*getSatID)    (struct gnss_interface *, int, int, int *);
    int (*getSatAZ)    (struct gnss_interface *, int, int, int *);
    int (*getSatEL)    (struct gnss_interface *, int, int, int *);
    int (*getSatSNR)   (struct gnss_interface *, int, int, int *);
    int (*fix)         (struct gnss_interface *);
    int (*valid)       (struct gnss_interface *);
    int (*getError)    (struct gnss_interface *);
    int (*standby)     (struct gnss_interface *);
    int (*wakeup)      (struct gnss_interface *);
    int (*testModule)  (struct gnss_interface *);

#ifdef _GNSS_INTERNAL_INTERFACE_

#endif
};


/* external functions for create/destroy gnss interface */
extern struct gnss_interface *gnss_create(void);
extern int gnss_destroy(struct gnss_interface *gnss);


#endif /* _GNSS_H_ */
