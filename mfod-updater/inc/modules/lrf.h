/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    lrf.h
        external function/variables/defines for LRF interface
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _LRF_H_
#define _LRF_H_

#include <stdbool.h>

/* macro defines : trigger state */
#define LRF_TRG_NONE				-1
#define LRF_TRG_WAIT				0
#define LRF_TRG_READY				1
#define LRF_TRG_FALSE				2
#define LRF_TRG_OVERCHARGE			3
#define LRF_TRG_FIRE				4

/* macro defines : LRF ERROR CODES */
#define LRF_ERROR_NONE				0x00
#define LRF_ERROR_XMTR				0x01
#define LRF_ERROR_RCVR				0x02
#define LRF_ERROR_COMM				0x04

/* structure declaration : LRF interface */
struct lrf_interface
{
	int (*standby)				(struct lrf_interface *);
	int (*wakeup)				(struct lrf_interface *);
	int (*fire)					(struct lrf_interface *);
	int (*getTrigger)			(struct lrf_interface *);
	int (*getRange)			    (struct lrf_interface *, int *, int *);
	int (*setMeasuringRange)	(struct lrf_interface *, int, int);
	int (*getMeasuringRange)	(struct lrf_interface *, int *, int *);
	int (*startMeasuring)		(struct lrf_interface *);
	int (*finalizeMeasuring)	(struct lrf_interface *);
	int (*testModule)			(struct lrf_interface *);
	int (*setRcc)				(struct lrf_interface *, double *, int);
	int (*getRcc)				(struct lrf_interface *, double *, double *, double *, double *);
	int (*getError)			    (struct lrf_interface *);

#ifdef _LRF_INTERNAL_INTERFACE_

#endif
};


/* external functions for create/destroy LRF interface */
extern struct lrf_interface *lrf_create(void);
extern int lrf_destroy(struct lrf_interface *lrf);


#endif /* _LRF_H_ */
