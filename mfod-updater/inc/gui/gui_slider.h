/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_slider.h
        external function/variables/defines for slider widget
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#ifndef _GUI_SLIDER_H_
#define _GUI_SLIDER_H_

#include "types.h"

/* macro defines */
#define SLIDER_CALLBACK_INCREASE_VALUE  0
#define SLIDER_CALLBACK_DECREASE_VALUE  1


/* structure declaration :  slider widget interface */
typedef struct gui_slider_interface
{
    int (*setTitle)       (struct gui_slider_interface *, char *);
    char *(*getTitle)     (struct gui_slider_interface *);
    int (*setMaxValue)    (struct gui_slider_interface *, int);
    int (*getMaxValue)    (struct gui_slider_interface *, int *);
    int (*setMinValue)    (struct gui_slider_interface *, int);
    int (*getMinValue)    (struct gui_slider_interface *, int *);
    int (*setCurrentValue)(struct gui_slider_interface *, int);
    int (*getCurrentValue)(struct gui_slider_interface *, int *);
    int (*setPosition)    (struct gui_slider_interface *, rect_t *);
    int (*getPosition)    (struct gui_slider_interface *, rect_t *);
    int (*setCallback)    (struct gui_slider_interface *, int, int (*) (void *));
    int (*execCallback)   (struct gui_slider_interface *, int, void *);

#ifdef _ENABLE_SLIDER_INTERNAL_INTERFACE_

#endif /* _ENABLE_SLIDER_INTERNAL_INTERFACE_ */
}
slider_t;


/* external functions for create/destroy slider widget interface */
extern struct gui_slider_interface * gui_slider_create(void);
extern int gui_slider_destroy(struct gui_slider_interface *);

#endif /* _GUI_SLIDER_H_ */
