/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_submenu.h
        external function/variables/defines for submenu widget
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#ifndef _GUI_SUBMENU_H_
#define _GUI_SUBMENU_H_

#include "types.h"


/* structure declaration : submenu item */
typedef struct gui_submenu_item
{
    int id;
    char  *string;
}
submenu_item_t;


/* structure declaration : submenu interface */
typedef struct gui_submenu_interface
{
    int (*setPosition) (struct gui_submenu_interface *, rect_t *);
    int (*getPosition) (struct gui_submenu_interface *, rect_t *);
    int (*addItem)     (struct gui_submenu_interface *, int, char * );
    int (*getNumItems)  (struct gui_submenu_interface *);
    submenu_item_t *(*focusNext)(struct gui_submenu_interface *);
    submenu_item_t *(*focusPrev)(struct gui_submenu_interface *);
    submenu_item_t *(*getItem)  (struct gui_submenu_interface *, int idx);
    submenu_item_t *(*getFocus) (struct gui_submenu_interface *);

#ifdef _ENABLE_SUBMENU_INTERNAL_INTERFACE_

#endif /* _ENABLE_SUBMENU_INTERNAL_INTERFACE_ */
}
submenu_t;


/* external functions for create / destroy submenu interface */
extern struct gui_submenu_interface *gui_submenu_create(void);
extern int gui_submenu_destroy(struct gui_submenu_interface *);

#endif /* _GUI_SUBMENU_H_ */
