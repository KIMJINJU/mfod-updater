/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_qsel.h
        external function/variables/defines for quick select widget
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _GUI_QSEL_H_
#define _GUI_QSEL_H_

#define NUM_QSEL_ITEMS          5

/* constant macro defines - quick select focus */
#define QSEL_NONE         -1
#define QSEL_CENTER       0
#define QSEL_LEFT         1
#define QSEL_RIGHT        2
#define QSEL_UP           3
#define QSEL_DOWN         4


/* structure declaration - gui quick select widget */
typedef struct gui_qsel_interface
{
    int   (*setFocus)    (struct gui_qsel_interface *, int);
    int   (*getFocus)    (struct gui_qsel_interface *);
    int   (*setPosition) (struct gui_qsel_interface *, int, rect_t *);
    int   (*getPosition) (struct gui_qsel_interface *, int, rect_t *);
    int   (*setCallback) (struct gui_qsel_interface *, int, int (*)(void *));
    int   (*execCallback)(struct gui_qsel_interface *, int, void *);
    int   (*setString)   (struct gui_qsel_interface *, int, char *);
    char *(*getString)   (struct gui_qsel_interface *, int);

#ifdef _ENABLE_QSEL_INTERNAL_INTERFACE_

#endif /* _ENABLE_QSEL_INTERNAL_INTERFACE_ */
}
qsel_t;


/* external functions for create/destroy qsel widget interface */
extern struct gui_qsel_interface *gui_qsel_create(void);
extern int gui_qsel_destroy(struct gui_qsel_interface *qsel);

#endif /* _GUI_QSEL_H_ */

