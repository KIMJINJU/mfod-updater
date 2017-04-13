/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    irshtr.h
        external function/variables/defines for IRSHTR interface
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _IRSHTR_H_
#define _IRSHTR_H_

#include <stdbool.h>


/* constant macro defines : ir shutter open/close state */
#define __IRSHTR_INVERSE__
#ifdef __IRSHTR_INVERSE__
#define IRSHTR_CLOSE 	0
#define IRSHTR_OPEN		1
#else
#define IRSHTR_CLOSE 	1
#define IRSHTR_OPEN		0
#endif


/* structure declarations : IRSHTR interface */
struct irshtr_interface
{
	int (*open)  		(struct irshtr_interface *);
	int (*close) 		(struct irshtr_interface *);
	int (*getPosition) 	(struct irshtr_interface *);
	int (*testModule)	(struct irshtr_interface *);

#ifdef _IRSHTR_INTERNAL_INTERFACE_

#endif
};

/* external functions for create/destroy IRSHTR interface */
extern struct irshtr_interface *irshtr_create(void);
extern int irshtr_destroy(struct irshtr_interface *irshtr);

#endif /*_IRSHTR_H_ */
