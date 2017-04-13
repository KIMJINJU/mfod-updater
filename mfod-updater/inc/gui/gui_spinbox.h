/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_spinbox.h
        external function/variables/defines for spinbox widget
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#ifndef _GUI_SPINBOX_H_
#define _GUI_SPINBOX_H_

#include "types.h"

/* constant macro defines - spinbox font size */
#define SPINBOX_FONT_SIZE_NORMAL    0
#define SPINBOX_FONT_SIZE_LARGE     1

/* constant macro defines - spinbox font size */
#define SPINBOX_UNSIGNED            0
#define SPINBOX_SIGEND              1

/* constant macro defines - spinbox data types */
#define SPINBOX_DATATYPE_NUMERIC    0
#define SPINBOX_DATATYPE_CHAR       1


/* structure declaration : slider widget interface */
typedef struct gui_spinbox_interface
{
    int (*execCallback) (struct gui_spinbox_interface *, void *);
    int (*setCallback)  (struct gui_spinbox_interface *, int (*) (void *));
    int (*setCurrVal)   (struct gui_spinbox_interface *, int);
    int (*setFontSize)  (struct gui_spinbox_interface *, int);
    int (*setMaxValue)  (struct gui_spinbox_interface *, int);
    int (*setMinValue)  (struct gui_spinbox_interface *, int);
    int (*setDeltaValue)(struct gui_spinbox_interface *, int);
    int (*setSign)      (struct gui_spinbox_interface *, int);
    int (*setDigit)     (struct gui_spinbox_interface *, int);
    int (*setDataType)  (struct gui_spinbox_interface *, int);
    int (*setPosition)  (struct gui_spinbox_interface *, rect_t *);
    int (*getCurrVal)   (struct gui_spinbox_interface *, int *);
    int (*getFontSize)  (struct gui_spinbox_interface *);
    int (*getMaxValue)  (struct gui_spinbox_interface *, int *);
    int (*getMinValue)  (struct gui_spinbox_interface *, int *);
    int (*getDeltaValue)(struct gui_spinbox_interface *, int *);
    int (*getSign)      (struct gui_spinbox_interface *);
    int (*getDigit)     (struct gui_spinbox_interface *);
    int (*getDatatype)  (struct gui_spinbox_interface *);
    int (*getPosition)  (struct gui_spinbox_interface *, rect_t *);

#ifdef _ENABLE_SPINBOX_INTERNAL_INTERFACE_

#endif /* _ENABLE_SPINBOX_INTERNAL_INTERFACE_ */
}
spinbox_t;


/* external functions for create spinbox widget */
extern struct gui_spinbox_interface *gui_spinbox_create(void);
extern int gui_spinbox_destroy(struct gui_spinbox_interface *spinbox);

#endif /* _GUI_SPINBOX_H_ */
