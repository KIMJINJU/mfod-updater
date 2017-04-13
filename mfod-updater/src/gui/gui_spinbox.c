/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_spinbox.c
        external/internal function implementations of spinbox widget interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "core/logger.h"
#include "gui/gui_spinbox.h"

/* structure declaration : spinbox widget attribute */
struct gui_spinbox_attribute
{
    /* external interface */
    struct gui_spinbox_interface extif;

    /* internal interface */
    int maxValue;
    int minValue;
    int currentValue;
    int deltaValue;
    int fontSize;
    int dataType;
    int digit;
    int sign;
    rect_t position;
    int (*callback) (void *);
};


static int
gui_spinbox_set_position(struct gui_spinbox_interface *spinbox, rect_t *pos)
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (this)
    {
        memcpy(&this->position, pos, sizeof(rect_t));
        TLOGMSG(0, ("spinbox widget(%p), position = {%d %d %d %d}\n", this, pos->x, pos->y, pos->w, pos->h));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_spinbox_get_position(struct gui_spinbox_interface *spinbox, rect_t *pos)
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (this)
        memcpy(pos, &this->position, sizeof(rect_t));
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_spinbox_set_fontsize(struct gui_spinbox_interface *spinbox, int size)
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (this)
        this->fontSize = size;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_spinbox_get_fontsize(struct gui_spinbox_interface *spinbox)
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (this)
        ret = this->fontSize;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_spinbox_set_callback(struct gui_spinbox_interface *spinbox, int (*callback)(void *))
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (this)
        this->callback = callback;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_spinbox_exec_callback(struct gui_spinbox_interface *spinbox, void *arg)
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (this)
    {
        if (this->callback)
            ret = this->callback(arg);
        else
            TLOGMSG(1, (DBGINFOFMT "null callback\n", DBGINFO));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_spinbox_set_maxval(struct gui_spinbox_interface *spinbox, int value)
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (this)
    {
        if (value <= this->minValue)
        {
            this->maxValue = value;
            this->minValue = value;
            this->currentValue = value;
        }
        else
        {
            if (value < this->currentValue)
                this->currentValue = value;

            this->maxValue = value;
        }

        TLOGMSG(0, ("spinbox widget (%p), maxval = %d\n", this, this->maxValue));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_spinbox_get_maxval(struct gui_spinbox_interface *spinbox, int *value)
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (this)
        *value = this->maxValue;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_spinbox_set_minval(struct gui_spinbox_interface *spinbox, int value)
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (this)
    {
        if (value >= this->maxValue)
        {
            this->maxValue = value;
            this->minValue = value;
            this->currentValue = value;
        }
        else
        {
            if (value > this->currentValue)
                this->currentValue = value;

            this->minValue = value;
        }

        TLOGMSG(0, ("spinbox widget (%p), minval = %d\n", this, this->minValue));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_spinbox_get_minval(struct gui_spinbox_interface *spinbox, int *value)
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (this)
        *value = this->minValue;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_spinbox_set_currval(struct gui_spinbox_interface *spinbox, int value)
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (this)
    {
        if (value > this->maxValue)
            this->currentValue = this->minValue;
        else if (value < this->minValue)
            this->currentValue = this->maxValue;
        else
            this->currentValue = value;

        TLOGMSG(0, ("spinbox widget (%p), current value = %d\n", this, this->currentValue));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_spinbox_get_currval(struct gui_spinbox_interface *spinbox, int *value)
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (this)
        *value = this->currentValue;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_spinbox_set_dval(struct gui_spinbox_interface *spinbox, int value)
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (spinbox)
    {
        this->deltaValue = value;
        TLOGMSG(0, ("spinbox widget (%p), delta value = %d\n", this, this->deltaValue));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_spinbox_get_dval(struct gui_spinbox_interface *spinbox, int *value)
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (this)
        *value = this->deltaValue;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_spinbox_set_digit(struct gui_spinbox_interface *spinbox, int digit)
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (this)
    {
        if (digit > 0)
            this->digit = digit;
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid digit value\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_spinbox_set_sign(struct gui_spinbox_interface *spinbox, int sign)
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (this)
    {
        if ((sign == SPINBOX_SIGEND) || (sign == SPINBOX_UNSIGNED))
            this->sign = sign;
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid sign value\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_spinbox_set_datatype(struct gui_spinbox_interface *spinbox, int type)
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (this)
    {
        if ((type == SPINBOX_DATATYPE_CHAR) || (type == SPINBOX_DATATYPE_NUMERIC))
            this->dataType = type;
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "invalid datatype value\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_spinbox_get_digit(struct gui_spinbox_interface *spinbox)
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (spinbox)
        ret = this->digit;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_spinbox_get_sign(struct gui_spinbox_interface *spinbox)
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (this)
        ret = this->sign;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_spinbox_get_datatype(struct gui_spinbox_interface *spinbox)
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (this)
        ret = this->dataType;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}


struct gui_spinbox_interface *
gui_spinbox_create(void)
{
    struct gui_spinbox_interface *spinbox = NULL;
    struct gui_spinbox_attribute *this = malloc(sizeof(struct gui_spinbox_attribute));

    if (this)
    {
        this->callback = NULL;
        this->maxValue   = 0;
        this->minValue   = 0;
        this->currentValue  = 0;
        this->deltaValue     = 1;
        this->digit    = 0;
        this->fontSize = SPINBOX_FONT_SIZE_NORMAL;
        this->sign     = SPINBOX_UNSIGNED;
        this->dataType = SPINBOX_DATATYPE_NUMERIC;

        spinbox = &(this->extif);
        spinbox->execCallback = gui_spinbox_exec_callback;
        spinbox->setCallback  = gui_spinbox_set_callback;
        spinbox->setCurrVal   = gui_spinbox_set_currval;
        spinbox->setFontSize  = gui_spinbox_set_fontsize;
        spinbox->setMaxValue    = gui_spinbox_set_maxval;
        spinbox->setMinValue    = gui_spinbox_set_minval;
        spinbox->setDeltaValue  = gui_spinbox_set_dval;
        spinbox->setSign      = gui_spinbox_set_sign;
        spinbox->setDigit     = gui_spinbox_set_digit;
        spinbox->setDataType  = gui_spinbox_set_datatype;
        spinbox->setPosition  = gui_spinbox_set_position;
        spinbox->getCurrVal   = gui_spinbox_get_currval;
        spinbox->getFontSize  = gui_spinbox_get_fontsize;
        spinbox->getMaxValue    = gui_spinbox_get_maxval;
        spinbox->getMinValue    = gui_spinbox_get_minval;
        spinbox->getDeltaValue  = gui_spinbox_get_dval;
        spinbox->getSign      = gui_spinbox_get_sign;
        spinbox->getDigit     = gui_spinbox_get_digit;
        spinbox->getDatatype  = gui_spinbox_get_datatype;
        spinbox->getPosition  = gui_spinbox_get_position;
        TLOGMSG(0, ("create spinbox widget interface (%p)\n", spinbox));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));

    return spinbox;
}

int
gui_spinbox_destroy(struct gui_spinbox_interface *spinbox)
{
    int ret = 0;
    struct gui_spinbox_attribute *this = (struct gui_spinbox_attribute *)spinbox;

    if (this)
    {
        free(this);
        TLOGMSG(0, ("destroy spinbox widget interface\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null spinbox widget interface\n", DBGINFO));
    }

    return ret;
}
