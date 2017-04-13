/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_submenu.c
        external/internal function implementations of submenu widget interface
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
#include "gui/gui_submenu.h"


/* structure declaration : submenu attribute */
struct gui_submenu_attribute
{
    /* external interface */
    struct gui_submenu_interface extif;

    /* internal interface */
    rect_t position;
    list_t *itemList;
    list_node_t *focus;
};


static submenu_item_t *
gui_submenu_create_item(int id, char *str)
{
    int length = 0;
    submenu_item_t *item = malloc(sizeof(submenu_item_t));

    if (!item)
    {
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));
        goto error_exit;
    }

    memset(item, 0x00, sizeof(submenu_item_t));

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
    item->id = id;

    return item;

error_exit:
    if (item)
        free(item->string);

    free(item);

    return NULL;
}


static int
gui_submenu_add_item(struct gui_submenu_interface *submenu, int id, char *str)
{
    int ret = 0;
    submenu_item_t *item = NULL;
    struct gui_submenu_attribute *this = (struct gui_submenu_attribute *) submenu;

    if (this)
    {
        item = gui_submenu_create_item(id, str);

        if (item)
        {
            if (list_insert_node(this->itemList, (void *) item) == 0)
                this->focus = this->itemList->head;
            else
            {
                free(item->string);
                free(item);
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "failed to insert submenu item\n", DBGINFO));
            }
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to create submenu item\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null submenu interface\n", DBGINFO));
    }

    return ret;
}


static submenu_item_t *
gui_submenu_focus_next_item(struct gui_submenu_interface *submenu)
{
    submenu_item_t *item = NULL;
    struct gui_submenu_attribute *this = (struct gui_submenu_attribute *) submenu;

    if (this)
    {
        this->focus = this->focus->next;
        item = (submenu_item_t *) this->focus->data;
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null submenu interface\n", DBGINFO));

    return item;
}


static submenu_item_t *
gui_submenu_focus_prev_item(struct gui_submenu_interface *submenu)
{
    submenu_item_t *item = NULL;
    struct gui_submenu_attribute *this = (struct gui_submenu_attribute *) submenu;

    if (this)
    {
        this->focus = this->focus->prev;
        item = (submenu_item_t *) this->focus->data;
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null submenu interface\n", DBGINFO));

    return item;
}


static submenu_item_t *
gui_submenu_get_item(struct gui_submenu_interface *submenu, int idx)
{
    list_node_t *node = NULL;
    submenu_item_t *item = NULL;
    struct gui_submenu_attribute *this = (struct gui_submenu_attribute *) submenu;

    if (this)
    {
        node = list_get_node(this->itemList, idx);

        if (node)
        {
            item = (submenu_item_t *) node->data;

            if (!item)
                TLOGMSG(1, (DBGINFOFMT "failed to get submenu item\n", DBGINFO));
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null submenu interface\n", DBGINFO));

    return item;
}


static submenu_item_t *
gui_submenu_get_focus(struct gui_submenu_interface *submenu)
{
    submenu_item_t *item = NULL;
    struct gui_submenu_attribute *this = (struct gui_submenu_attribute *) submenu;

    if (this)
    {
        if (this->focus)
            item = (submenu_item_t *) this->focus->data;
    }
    else
        TLOGMSG(1, (DBGINFOFMT "subnull menu interface\n", DBGINFO));

    return item;
}


static int
gui_submenu_get_numitem(struct gui_submenu_interface *submenu)
{
    int ret = 0;
    struct gui_submenu_attribute *this = (struct gui_submenu_attribute *) submenu;

    if (this)
        ret = this->itemList->count;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null submenu interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_submenu_set_position(struct gui_submenu_interface *submenu, rect_t *pos)
{
    int ret = 0;
    struct gui_submenu_attribute *this = (struct gui_submenu_attribute *) submenu;

    if (this)
    {
        this->position.x = pos->x;
        this->position.y = pos->y;
        this->position.w = pos->w;
        this->position.h = pos->h;
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null submenu interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_submenu_get_position(struct gui_submenu_interface *submenu, rect_t *pos)
{
    int ret = 0;
    struct gui_submenu_attribute *this = (struct gui_submenu_attribute *) submenu;

    if (this)
    {
        pos->x = this->position.x;
        pos->y = this->position.y;
        pos->w = this->position.w;
        pos->h = this->position.h;
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null submenu interface\n", DBGINFO));
    }

    return ret;
}


struct gui_submenu_interface *
gui_submenu_create(void)
{
    struct gui_submenu_interface *submenu = NULL;
    struct gui_submenu_attribute *this = malloc(sizeof(struct gui_submenu_attribute));

    if (this)
    {
        memset(this, 0x00, sizeof(struct gui_submenu_attribute));
        this->itemList = list_create();

        if (this->itemList)
        {
            submenu = &(this->extif);
            submenu->addItem     = gui_submenu_add_item;
            submenu->focusNext   = gui_submenu_focus_next_item;
            submenu->focusPrev   = gui_submenu_focus_prev_item;
            submenu->getItem     = gui_submenu_get_item;
            submenu->getFocus    = gui_submenu_get_focus;
            submenu->getNumItems = gui_submenu_get_numitem;
            submenu->setPosition = gui_submenu_set_position;
            submenu->getPosition = gui_submenu_get_position;
            TLOGMSG(0, ("create submenu interface\n"));
        }
        else
        {
            free(this);
            TLOGMSG(1, (DBGINFOFMT "failed to create submenu item list\n", DBGINFO));
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));

    return submenu;
}


int
gui_submenu_destroy(struct gui_submenu_interface *submenu)
{
    int ret = 0;
    struct gui_submenu_attribute *this = (struct gui_submenu_attribute *) submenu;

    if (this)
    {
        list_destroy(this->itemList);
        free(this);
        TLOGMSG(0, ("destroy submenu interface\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null submenu interface\n", DBGINFO));
    }

    return ret;
}
