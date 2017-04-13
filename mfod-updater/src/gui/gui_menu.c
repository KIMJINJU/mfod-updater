/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_menu.c
        external/internal function implementations of menu widget interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "core/logger.h"
#include "ds/list.h"
#include "gui/gui_menu.h"

/* structure declaration :  menu attribute */
struct gui_menu_attribute
{
    /* external interface */
    struct gui_menu_interface extif;

    /* internal interface */
    list_t *itemList;
    list_node_t *focus;
};


static menu_item_t *
gui_menu_create_item(int id, char *str, rect_t *pos, struct gui_submenu_interface *submenu)
{
    int length = 0;
    menu_item_t *item = malloc(sizeof(menu_item_t));

    if (!item)
    {
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));
        goto error_exit;
    }

    memset(item, 0x00, sizeof(menu_item_t));

    if (!str)
    {
        TLOGMSG(1, (DBGINFOFMT "null string parameter\n", DBGINFO));
        goto error_exit;
    }

    length = strlen(str);
    item->string = malloc(length + 1);

    if (!item->string)
    {
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));
        goto error_exit;
    }

    memset(item->string, 0x00, length + 1);
    memcpy(item->string, str, length);

    if (!pos)
    {
        TLOGMSG(1, (DBGINFOFMT "null position parameter\n", DBGINFO));
        goto error_exit;
    }

    memcpy(&item->position, pos, sizeof(rect_t));
    item->id = id;
    item->submenu = submenu;

    return item;

error_exit:
    if (item)
        free(item->string);

    free(item);

    return NULL;
}


static int
gui_menu_add_item(struct gui_menu_interface *menu, int id, char *str, rect_t *pos, struct gui_submenu_interface *submenu)
{
    int ret = 0;
    menu_item_t *item = NULL;
    struct gui_menu_attribute *this = (struct gui_menu_attribute *) menu;

    if (this)
    {
        item = gui_menu_create_item(id, str, pos, submenu);

        if (item)
        {
            if (list_insert_node(this->itemList, (void *) item) == 0)
                this->focus = this->itemList->head;
            else
            {
                free(item->string);
                free(item);
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to insert menu item\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to create menu item\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null menu interface\n", DBGINFO));
    }

    return ret;
}


static menu_item_t *
gui_menu_focus_next_item(struct gui_menu_interface *menu)
{
    menu_item_t *item = NULL;
    struct gui_menu_attribute *this = (struct gui_menu_attribute *) menu;

    if (menu)
    {
        this->focus = this->focus->next;
        item = (menu_item_t *) this->focus->data;
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null menu interface\n", DBGINFO));

    return item;
}


static menu_item_t *
gui_menu_focus_prev_item(struct gui_menu_interface *menu)
{
    menu_item_t *item = NULL;
    struct gui_menu_attribute *this = (struct gui_menu_attribute *) menu;

    if (this)
    {
        this->focus = this->focus->prev;
        item = (menu_item_t *) this->focus->data;
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null menu interface\n", DBGINFO));

    return item;
}


static menu_item_t *
gui_menu_get_item(struct gui_menu_interface *menu, int idx)
{
    list_node_t *node = NULL;
    menu_item_t *item = NULL;
    struct gui_menu_attribute *this = (struct gui_menu_attribute *) menu;

    if (this)
    {
        node = list_get_node(this->itemList, idx);

        if (node)
            item = (menu_item_t *) node->data;
        else
            TLOGMSG(1, (DBGINFOFMT "failed to get menu itme list node\n", DBGINFO));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null menu interface\n", DBGINFO));

    return item;
}


static menu_item_t *
gui_menu_get_focus(struct gui_menu_interface *menu)
{
    menu_item_t *item = NULL;
    struct gui_menu_attribute *this = (struct gui_menu_attribute *) menu;

    if (this)
        item = (menu_item_t *) this->focus->data;
    else
        TLOGMSG(1, (DBGINFOFMT "null menu interface\n", DBGINFO));

    return item;
}


static int
gui_menu_get_numitem(struct gui_menu_interface *menu)
{
    int ret = 0;
    struct gui_menu_attribute *this = (struct gui_menu_attribute *) menu;

    if (this)
        ret = this->itemList->count;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null menu interface\n", DBGINFO));
    }

    return ret;
}


struct gui_menu_interface *
gui_menu_create(void)
{
    struct gui_menu_interface *menu = NULL;
    struct gui_menu_attribute *this = malloc(sizeof(struct gui_menu_attribute));

    if (this)
    {
        memset(this, 0x00, sizeof(struct gui_menu_attribute));
        this->itemList = list_create();

        if (this->itemList)
        {
            menu = &(this->extif);
            menu->addItem    = gui_menu_add_item;
            menu->focusNext  = gui_menu_focus_next_item;
            menu->focusPrev  = gui_menu_focus_prev_item;
            menu->getItem    = gui_menu_get_item;
            menu->getFocus   = gui_menu_get_focus;
            menu->getNumItems = gui_menu_get_numitem;
            TLOGMSG(0, ("create menu interface\n"));
        }
        else
        {
            free(this);
            TLOGMSG(1, (DBGINFOFMT "failed to create menu item list\n", DBGINFO));
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));

    return menu;
}


int
gui_menu_destroy(struct gui_menu_interface *menu)
{
    int ret = 0;
    menu_item_t *item = NULL;
    struct gui_menu_attribute *this = (struct gui_menu_attribute *) menu;

    if (this)
    {
        for (int i = 0; i < this->itemList->count; i++)
        {
            item = gui_menu_get_item(menu, i);

            if (item)
            {
                gui_submenu_destroy(item->submenu);
                item->submenu = NULL;
            }
        }

        list_destroy(this->itemList);
        free(this);
        TLOGMSG(0, ("destroy menu interface\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null menu interface\n", DBGINFO));
    }

    return ret;
}
