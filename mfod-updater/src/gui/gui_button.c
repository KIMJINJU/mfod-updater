/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_button.c
        external/internal function implementations of button widget interface
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
#include "gui/gui_button.h"


/* structure declaration : button widget attribute */
struct gui_button_attribute
{
    /* external interface */
    struct gui_button_interface extif;

    /* internal interface */
    char *title;
    int (*callback) (void *);
    rect_t position;
    pthread_mutex_t mutex;
};


static int
gui_button_set_callback(struct gui_button_interface *btn, int (*callback)(void *))
{
    int ret = 0;
    struct gui_button_attribute *this = (struct gui_button_attribute *) btn;

    if (this)
        this->callback = callback;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null button interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_button_execute_callback(struct gui_button_interface *btn, void *arg)
{
    int ret = 0;
    struct gui_button_attribute *this = (struct gui_button_attribute *) btn;

    if (this)
    {
        if (this->callback)
            ret = this->callback(arg);
        else
            TLOGMSG(1, ("gui_button : null callback function\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null button interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_button_set_position(struct gui_button_interface *btn, rect_t *pos)
{
    int ret = 0;
    struct gui_button_attribute *this = (struct gui_button_attribute *) btn;

    if (this)
        memcpy(&this->position, pos, sizeof(rect_t));
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null button interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_button_get_position(struct gui_button_interface *btn, rect_t *rect)
{
    int ret = 0;
    struct gui_button_attribute *this = (struct gui_button_attribute *) btn;

    if (btn)
        memcpy(rect, &this->position, sizeof(rect_t));
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null button interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_button_set_title(struct gui_button_interface *btn, char *title)
{
    int ret = 0;
    int length = 0;
    struct gui_button_attribute *this = (struct gui_button_attribute *) btn;

    if (this)
    {
        pthread_mutex_lock(&this->mutex);

        if (this->title)
            free(this->title);

        if (title)
        {
            length = strlen(title);
            this->title = malloc(sizeof(char) * (length + 1));

            if (this->title)
            {
                memset(this->title, 0x00, length + 1);
                memcpy(this->title, title, length);
            }
            else
            {
                ret = -1;
                TLOGMSG(1, (DBGINFOFMT "malloc return null. set button title to null\n", DBGINFO));
            }
        }
        else
            this->title = NULL;

        pthread_mutex_unlock(&this->mutex);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null button interface\n", DBGINFO));
    }

    return ret;
}


static char *
gui_button_get_title(struct gui_button_interface *btn)
{
    char *title = NULL;
    struct gui_button_attribute *this = (struct gui_button_attribute *) btn;

    if (this)
        title = this->title;
    else
        TLOGMSG(1, (DBGINFOFMT "null button interface\n", DBGINFO));

    return title;
}


struct gui_button_interface *
gui_button_create(void)
{
    struct gui_button_interface *btn = NULL;
    struct gui_button_attribute *this = malloc(sizeof(struct gui_button_attribute));

    if (this)
    {
        memset(this, 0x00, sizeof(struct gui_button_attribute));
        pthread_mutex_init(&this->mutex, NULL);

        btn = &(this->extif);
        btn->setTitle     = gui_button_set_title;
        btn->getTitle     = gui_button_get_title;
        btn->setPosition  = gui_button_set_position;
        btn->getPosition  = gui_button_get_position;
        btn->setCallback  = gui_button_set_callback;
        btn->execCallback = gui_button_execute_callback;
        TLOGMSG(0, ("create button widget interface\n"));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));

    return btn;
}


int
gui_button_destroy(struct gui_button_interface *btn)
{
    int ret = 0;
    struct gui_button_attribute *this = (struct gui_button_attribute *) btn;

    if (this)
    {
        pthread_mutex_destroy(&this->mutex);
        free(this->title);
        free(this);
        TLOGMSG(0, ("destroy button widget interface\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null button interface\n", DBGINFO));
    }

    return ret;
}
