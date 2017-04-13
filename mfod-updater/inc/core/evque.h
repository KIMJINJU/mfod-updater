/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    evque.h
        external function/variables/defines for event queue
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _EVQUE_H_
#define _EVQUE_H_

#define EVQUE_EVPARM_BUFFER_LENGTH		64
#define EVQUE_EVPARM_KEYIN_LENGTH		20

/* macro defines : event codes */
#define EVCODE_PWRKEY_RELEASED	    	0x0100
#define EVCODE_KEYIN					0x0200
#define EVCODE_LOW_BATTERY				0x0300
#define EVCODE_DEAD_BATTERY				0x0400
#define EVCODE_PWRSRC_CHANGED			0x0500
#define EVCODE_CREATE_DIALOG			0x0600
#define EVCODE_QUERY_MAGCOMPDATA		0x0700

/* structure declaration : event */
typedef struct event
{
	unsigned int code;
	unsigned int parm;
}
event_t;

/* external functions */
extern int evque_create(void);
extern int evque_destroy(void);
extern int evque_flush(void);
extern int evque_set_event(unsigned int evcode, unsigned int evparm);
extern event_t *evque_get_event(void);

#endif /* _EVQUE_H_ */
