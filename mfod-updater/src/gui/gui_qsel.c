/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_qsel.c
        external/internal function implementations of quick select  widget interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "core/logger.h"
#include "gui/gui_qsel.h"


#define MAXLEN_ITEM_STRING      64

struct gui_qsel_attribute
{
    /* external attribute */
    struct gui_qsel_interface extif;

    /* internal attribute */
    int focus;
    char string[NUM_QSEL_ITEMS][MAXLEN_ITEM_STRING];
    rect_t position[NUM_QSEL_ITEMS];
    int (*callback[NUM_QSEL_ITEMS]) (void *);
};


static int
gui_qsel_set_item_focus(struct gui_qsel_interface *qsel, int qsel_item)
{
    int ret = 0;
    struct gui_qsel_attribute *this = (struct gui_qsel_attribute *) qsel;

    if (this)
    {
        if ((qsel_item >= QSEL_NONE) && (qsel_item <= QSEL_DOWN))
        {
            this->focus = qsel_item;
            TLOGMSG(0, ("qsel widget (0x%p) item focus = %d\n", this, this->focus));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid qsel item number\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null qsel widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_qsel_get_item_focus(struct gui_qsel_interface *qsel)
{
    int ret = 0;
    struct gui_qsel_attribute *this = (struct gui_qsel_attribute *) qsel;

    if (this)
        ret = this->focus;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null qsel widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_qsel_set_position(struct gui_qsel_interface *qsel, int qsel_item, rect_t *rect)
{
    int ret = 0;
    struct gui_qsel_attribute *this = (struct gui_qsel_attribute *) qsel;

    if (this)
    {
        if ((qsel_item >= QSEL_CENTER) && (qsel_item <= QSEL_DOWN))
        {
            memcpy(&this->position[qsel_item], rect, sizeof(rect_t));
            TLOGMSG(0, ("qsel widget (0x%p) item position = {%d, %d, %d, %d}\n", this, rect->x, rect->y, rect->w, rect->h));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid qsel item number\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null qsel widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_qsel_get_position(struct gui_qsel_interface *qsel, int qsel_item, rect_t *rect)
{
    int ret = 0;
    struct gui_qsel_attribute *this = (struct gui_qsel_attribute *) qsel;

    if (this)
    {
        if ((qsel_item >= QSEL_CENTER) && (qsel_item <= QSEL_DOWN))
            memcpy(rect, &this->position[qsel_item], sizeof(rect_t));
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid qsel item number\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null qsel widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_qsel_set_callback(struct gui_qsel_interface *qsel, int qsel_item, int (*callback)(void *))
{
    int ret = 0;
    struct gui_qsel_attribute *this = (struct gui_qsel_attribute *) qsel;

    if (this)
    {
        if ((qsel_item >= QSEL_CENTER) && (qsel_item <= QSEL_DOWN))
            this->callback[qsel_item] = callback;
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid qsel item number\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null qsel widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_qsel_exec_callback(struct gui_qsel_interface *qsel, int qsel_item, void *arg)
{
    int ret = 0;
    struct gui_qsel_attribute *this = (struct gui_qsel_attribute *) qsel;

    if (this)
    {
        if ((qsel_item >= QSEL_CENTER) && (qsel_item <= QSEL_DOWN))
        {
            if (this->callback[qsel_item] != NULL)
                ret = this->callback[qsel_item](arg);
            else
                TLOGMSG(1, (DBGINFOFMT "null callback\n", DBGINFO));
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid qsel item number\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null qsel widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_qsel_set_item_string(struct gui_qsel_interface *qsel, int qsel_item, char *string)
{
    int ret = 0;
    struct gui_qsel_attribute *this = (struct gui_qsel_attribute *) qsel;

    if (this)
    {
        if ((qsel_item >= QSEL_CENTER) && (qsel_item <= QSEL_DOWN))
        {
            memset(&this->string[qsel_item][0], 0x00, MAXLEN_ITEM_STRING);
            strncpy(&this->string[qsel_item][0], string, MAXLEN_ITEM_STRING);
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid qsel item number\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null qsel widget interface\n", DBGINFO));
    }

    return ret;
}


static char *
gui_qsel_get_item_string(struct gui_qsel_interface *qsel, int qsel_item)
{
    char *string = NULL;
    struct gui_qsel_attribute *this = (struct gui_qsel_attribute *) qsel;

    if (this)
    {
        if ((qsel_item >= QSEL_CENTER) && (qsel_item <= QSEL_DOWN))
            string = &(this->string[qsel_item][0]);
        else
            TLOGMSG(1, (DBGINFOFMT "invalid qsel item number\n", DBGINFO));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null qsel widget interface\n", DBGINFO));

    return string;
}


struct gui_qsel_interface *
gui_qsel_create(void)
{
    struct gui_qsel_interface *qsel = NULL;
    struct gui_qsel_attribute *this = malloc(sizeof(struct gui_qsel_attribute));

    if (this)
    {
        memset(this, 0x00, sizeof(struct gui_qsel_attribute));
        this->focus = QSEL_NONE;
        qsel = &(this->extif);
        qsel->setFocus     = gui_qsel_set_item_focus;
        qsel->getFocus     = gui_qsel_get_item_focus;
        qsel->setPosition  = gui_qsel_set_position;
        qsel->getPosition  = gui_qsel_get_position;
        qsel->setCallback  = gui_qsel_set_callback;
        qsel->execCallback = gui_qsel_exec_callback;
        qsel->setString    = gui_qsel_set_item_string;
        qsel->getString    = gui_qsel_get_item_string;
        TLOGMSG(0, ("create qsel widget interface (0x%p)\n", qsel));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));

    return qsel;
}


int
gui_qsel_destroy(struct gui_qsel_interface *qsel)
{
    int ret = 0;
    struct gui_qsel_attribute *this = (struct gui_qsel_attribute *) qsel;

    if (this)
    {
        free(this);
        TLOGMSG(0, ("destroy qsel widget interface\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null qsel widget interface\n", DBGINFO));
    }

    return ret;
}
