/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    display.h
        external function/variables/defines for display interface
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <stdbool.h>

/* constant macro defines : brightness index */
#define DISP_BRIGHT_DV			0
#define DISP_BRIGHT_IR			1

/* constant macro defines : brigntess */
#define DISP_BRIGHT_DV_DEFAULT	10
#define DISP_BRIGHT_DV_MAX		0
#define DISP_BRIGHT_DV_MIN		20
#define DISP_BRIGHT_IR_DEFAULT	45
#define DISP_BRIGHT_IR_MAX		35
#define DISP_BRIGHT_IR_MIN		55

/* macro defines : display error codes */
#define DISP_ERROR_NONE 		0x00
#define DISP_ERROR_SETREG		0x01
#define DISP_ERROR_BRADJDEV		0x02

/* type define : display interface */
struct display_interface
{
	int (*setBright)   	 (struct display_interface *, int, int);
	int (*getBright)   	 (struct display_interface *, int, int *);
	int (*powerOn) 		 (struct display_interface *);
	int (*powerOff)		 (struct display_interface *);
	int (*getPowerStatus)(struct display_interface *);
	int (*getError)  	 (struct display_interface *);
	int (*testModule)    (struct display_interface *);

#ifdef _DISPLAY_INTERNAL_INTERFACE_
#endif
};


/* external functions for create/destroy display interface */
extern struct display_interface *disp_create(void);
extern int disp_destroy(struct display_interface *disp);

#endif /* _DISPLAY_H_ */
