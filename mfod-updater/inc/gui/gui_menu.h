/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_menu.h
        external function/variables/defines for menu widget
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#ifndef _GUI_MENU_H_
#define _GUI_MENU_H_

#include "types.h"
#include "gui/gui_submenu.h"


/* structure declaration : menu item */
typedef struct gui_menu_item
{
    int   id;
    char  *string;
    rect_t position;
    submenu_t *submenu;
}
menu_item_t;

/* structure declaration : menu interface */
typedef struct gui_menu_interface
{
    int (*addItem)     (struct gui_menu_interface *, int, char *, rect_t *, submenu_t *);
    int (*getNumItems) (struct gui_menu_interface *);
    menu_item_t *(*focusNext)(struct gui_menu_interface *);
    menu_item_t *(*focusPrev)(struct gui_menu_interface *);
    menu_item_t *(*getItem)  (struct gui_menu_interface *, int index);
    menu_item_t *(*getFocus) (struct gui_menu_interface *);

#ifdef _ENABLE_MENU_INTERNAL_INTERFACE_

#endif /* _ENABLE_MENU_INTERNAL_INTERFACE_ */
}
menu_t;


/* external functions for create/destory menu widget interface */
extern struct gui_menu_interface *gui_menu_create(void);
extern int gui_menu_destroy(struct gui_menu_interface *menu);

#endif /* _GUI_MENU_H_ */
