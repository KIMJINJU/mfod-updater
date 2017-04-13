/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_button.h
        external function/variables/defines for button widget
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#ifndef _GUI_BUTTON_H_
#define _GUI_BUTTON_H_

#include "types.h"

/* structure declaration : gui button interface */
typedef struct gui_button_interface
{
    int (*setPosition)  (struct gui_button_interface *, rect_t *);
    int (*getPosition)  (struct gui_button_interface *, rect_t *);
    int (*setCallback)  (struct gui_button_interface *, int (*) (void *));
    int (*execCallback) (struct gui_button_interface *, void *);
    int (*setTitle)     (struct gui_button_interface *, char *);
    char *(*getTitle)   (struct gui_button_interface *);

#ifdef _ENABLE_BUTTON_INTERNAL_INTERFACE_

#endif /* _ENABLE_BUTTON_INTERNAL_INTERFACE_ */
}
button_t;


/* external functions for create button widget */
extern struct gui_button_interface *gui_button_create(void);
extern int gui_button_destroy(struct gui_button_interface *btn);

#endif /* _GUI_BUTTON_H_ */
