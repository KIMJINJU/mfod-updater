/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_dialog.h
        external function/variables/defines for dialog widget
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#ifndef _GUI_DIALOG_H_
#define _GUI_DIALOG_H_

#include "types.h"
#include "ds/list.h"
#include "gui/gui_button.h"
#include "gui/gui_caption.h"
#include "gui/gui_grid.h"
#include "gui/gui_qsel.h"
#include "gui/gui_slider.h"
#include "gui/gui_spinbox.h"
#include "gui/gui_svplot.h"



/* macro defines : dialog widget types */
#define WIDGET_BUTTON      0
#define WIDGET_CAPTION     1
#define WIDGET_GRID        2
#define WIDGET_QMENU       3
#define WIDGET_QSEL        4
#define WIDGET_SLIDER      5
#define WIDGET_SPINBOX     6
#define WIDGET_SVPLOT      7


/* structure declaration : dialog widget */
typedef struct gui_dialog_widget
{
    int id;
    int type;
    void *data;
}
dialog_widget_t;

/* structure declaration : dialog widget interface */
typedef struct gui_dialog_interface
{
    int     (*getID)                (struct gui_dialog_interface *);
    int     (*setTitle)             (struct gui_dialog_interface *, char *);
    char *  (*getTitle)             (struct gui_dialog_interface *);
    int     (*setPosition)          (struct gui_dialog_interface *, rect_t *);
    int     (*getPosition)          (struct gui_dialog_interface *, rect_t *);
    int     (*addWidget)            (struct gui_dialog_interface *, int, int, void *);
    list_t *(*getWidgetList)        (struct gui_dialog_interface *);
    dialog_widget_t *(*findWidget)  (struct gui_dialog_interface *, int);
    dialog_widget_t *(*setFocus)    (struct gui_dialog_interface *, int);
    dialog_widget_t *(*getFocus)    (struct gui_dialog_interface *);
    dialog_widget_t *(*focusNext)   (struct gui_dialog_interface *);
    dialog_widget_t *(*focusPrev)   (struct gui_dialog_interface *);

#ifdef _ENABLE_DIALOG_INTERNAL_INTERFACE_

#endif /* _ENABLE_DIALOG_INTERNAL_INTERFACE_ */
}
dialog_t ;


/* external functions for create/destroy dialog widget interface */
extern struct gui_dialog_interface *gui_dialog_create(int id);
extern int gui_dialog_destroy(struct gui_dialog_interface *dlg);

#endif /* _GUI_DIALOG_H_ */
