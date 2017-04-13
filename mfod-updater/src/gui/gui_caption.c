/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_caption.c
        external/internal function implementations of caption widget interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/logger.h"
#include "gui/gui_caption.h"

/* structure decalrations : caption widget attribute */
struct gui_caption_attribute
{
    /* external interface */
    struct gui_caption_interface extif;

    /* internal interface */
    int fontType;
    int fontSize;
    int align;
    char *string;
    rect_t postion;
    pthread_mutex_t mutex;
};


static int
gui_caption_set_position(struct gui_caption_interface *caption, rect_t *pos)
{
    int ret = 0;
    struct gui_caption_attribute *this = (struct gui_caption_attribute *) caption;

    if (this)
    {
        if (pos)
            memcpy(&this->postion, pos, sizeof(rect_t));
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid parameter\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null caption widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_caption_get_position(struct gui_caption_interface *caption, rect_t *pos)
{
    int ret = 0;
    struct gui_caption_attribute *this = (struct gui_caption_attribute *) caption;

    if (this)
        memcpy(pos, &this->postion, sizeof(rect_t));
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null caption widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_caption_set_string(struct gui_caption_interface *caption, char *str)
{
    int ret = 0;
    int length = 0;
    struct gui_caption_attribute *this = (struct gui_caption_attribute *) caption;

    if (this)
    {
        pthread_mutex_lock(&this->mutex);

        if (this->string)
            free(this->string);

        if (str)
        {
            length = strlen(str);
            this->string = malloc(sizeof(char) * (length + 1));

            if ( this->string != NULL)
            {
                memset(this->string, 0x00, length + 1);
                memcpy(this->string, str, length);
            }
            else
            {
                ret = -1;
                TLOGMSG(1, ("gui_button, %s : malloc return null. set button title to null\n", __func__));
            }
        }
        else
            this->string = NULL;

        pthread_mutex_unlock(&this->mutex);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null caption widget interface\n", DBGINFO));
    }

    return ret;
}


static char *
gui_caption_get_string(struct gui_caption_interface *caption)
{
    char *str = NULL;
    struct gui_caption_attribute *this = (struct gui_caption_attribute *) caption;

    if (this)
        str = this->string;
    else
        TLOGMSG(1, (DBGINFOFMT "null caption widget interface\n", DBGINFO));

    return str;
}


static int
gui_caption_set_align(struct gui_caption_interface *caption, int align)
{
    int ret = 0;
    struct gui_caption_attribute *this = (struct gui_caption_attribute *) caption;

    if (this)
    {
        if ((CAPTION_ALIGN_LEFT <= align) && ( align <= CAPTION_ALIGN_RIGHT))
            this->align = align;
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid align flag\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null caption widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_caption_get_align(struct gui_caption_interface *caption)
{
    int ret = 0;
    struct gui_caption_attribute *this = (struct gui_caption_attribute *) caption;

    if (this)
        ret = this->align;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null caption widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_caption_set_font_height(struct gui_caption_interface *caption, int size)
{
    int ret = 0;
    struct gui_caption_attribute *this = (struct gui_caption_attribute *) caption;

    if (this)
        this->fontSize = size;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null caption widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_caption_get_font_height(struct gui_caption_interface *caption)
{
    int ret = 0;
    struct gui_caption_attribute *this = (struct gui_caption_attribute *) caption;

    if (this)
        ret = this->fontSize;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null caption widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_caption_set_font(struct gui_caption_interface *caption, int type)
{
    int ret = 0;
    struct gui_caption_attribute *this = (struct gui_caption_attribute *) caption;

    if (this)
    {
        if ((type == CAPTION_FONT_NORMAL) || (type == CAPTION_FONT_MONOSPACED))
            this->fontType = type;
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid font type parmeter\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null caption widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_caption_get_font(struct gui_caption_interface *caption)
{
    int ret = 0;
    struct gui_caption_attribute *this = (struct gui_caption_attribute *) caption;

    if (this)
        ret = this->fontType;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null caption widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_caption_get_string_length(struct gui_caption_interface *caption)
{
    int ret = 0;
    struct gui_caption_attribute *this = (struct gui_caption_attribute *) caption;

    if (this)
    {
        if (this->string)
            ret = strlen(this->string);
        else
            ret = 0;
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null caption widget interface\n", DBGINFO));
    }

    return ret;
}


struct gui_caption_interface *
gui_caption_create(void)
{
    struct gui_caption_interface *caption = NULL;
    struct gui_caption_attribute *this = malloc(sizeof(struct gui_caption_attribute));

    if (this)
    {
        memset(this,  0x00, sizeof(struct gui_caption_attribute));
        pthread_mutex_init(&this->mutex, NULL);

        caption = &(this->extif);
        caption->setPosition     = gui_caption_set_position;
        caption->getPosition     = gui_caption_get_position;
        caption->setString       = gui_caption_set_string;
        caption->getString       = gui_caption_get_string;
        caption->setAlign        = gui_caption_set_align;
        caption->getAlign        = gui_caption_get_align;
        caption->setFontHeight   = gui_caption_set_font_height;
        caption->getFontHeight   = gui_caption_get_font_height;
        caption->setFont         = gui_caption_set_font;
        caption->getFont         = gui_caption_get_font;
        caption->getStringLength = gui_caption_get_string_length;
        TLOGMSG(0, ("create caption widget interface\n"));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));

    return caption;
}


int
gui_caption_destroy(struct gui_caption_interface *caption)
{
    int ret = 0;
    struct gui_caption_attribute *this = (struct gui_caption_attribute *) caption;

    if (this)
    {
        pthread_mutex_destroy(&this->mutex);
        free(this->string);
        free(this);
        TLOGMSG(0, ("destroy caption widget\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null caption widget interface\n", DBGINFO));
    }

    return ret;
}
