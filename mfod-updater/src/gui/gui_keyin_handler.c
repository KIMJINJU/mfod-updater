/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_keyin_handler.c
        external/internal function implementations of gui input handler interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <directfb.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "core/evque.h"
#include "core/logger.h"
#include "etc/util.h"
#include "gui/gui_keyin_handler.h"

#define LONGKEY_COUNT           5
#define KEYSCAN_INTERVAL        100

/* event parameter offset for key input event */
#define KEYIN_EVPARM_OFFSET_KEYCODE   0
#define KEYIN_EVPARM_OFFSET_PUSHKEY   1
#define KEYIN_EVPARM_OFFSET_RELKEY    2
#define KEYIN_EVPARM_OFFSET_LONGKEY   3
#define KEYIN_EVPARM_OFFSET_RPTKEY    4

/* number of key input event parameters  */
#define NUM_KEYIN_EVPARM              5

/* structure declaration : keyin handler attribute */
struct keyin_handler_attribute
{
    /* external interface */
    struct keyin_handler_interface extif;

    /* internal interface */
    bool push;
    bool hold;
    bool lock;
    bool scan;
    unsigned int  keycode;
    unsigned int  repeat;

    pthread_t tid;
    pthread_cond_t cond;
    pthread_mutex_t mtx;
    IDirectFBEventBuffer *eventBuffer;
};


static unsigned int
gui_keyin_handler_convert_keycode(DFBInputDeviceKeySymbol symbol)
{
    unsigned int keycode = 0;

    switch (symbol)
    {
    case DIKS_CURSOR_LEFT:
        keycode = KEYCODE_DOWN;
        break;

    case DIKS_CURSOR_RIGHT:
        keycode = KEYCODE_RIGHT;
        break;

    case DIKS_CURSOR_UP:
        keycode = KEYCODE_UP;
        break;

    case DIKS_CURSOR_DOWN:
        keycode = KEYCODE_LEFT;
        break;

    case DIKS_ENTER:
        keycode = KEYCODE_ENTER;
        break;

    case DIKS_SPACE:
        keycode = KEYCODE_TRIGGER;
        break;

    case DIKS_HOME:
        keycode = KEYCODE_CHANNEL;
        break;

    case DIKS_END:
        keycode = KEYCODE_CAPTURE;
        break;

    default:
        keycode = KEYCODE_UNKNOWN;
        break;
    }

    return keycode;
}


static void
gui_keyin_handler_check_push(struct keyin_handler_interface *handler, DFBInputEvent *ev)
{
    unsigned int keycode = 0;
    struct keyin_handler_attribute *this = (struct keyin_handler_attribute *)handler;

    if (!this->push)
    {
        if(ev->type == DIET_KEYPRESS)
        {
            keycode = gui_keyin_handler_convert_keycode(ev->key_symbol);

            if (keycode != KEYCODE_UNKNOWN)
            {
                this->keycode = keycode;
                this->push    = true;
                this->hold    = false;
                this->repeat  = 0;
                evque_set_event(EVCODE_KEYIN, (keycode | KEYIN_PUSH));
            }
        }
    }
}

static void
gui_keyin_handler_check_release(struct keyin_handler_interface *handler, DFBInputEvent *ev)
{
    unsigned int keycode = 0;
    struct keyin_handler_attribute *this = (struct keyin_handler_attribute *)handler;

    if (this->push)
    {
        if (ev->type == DIET_KEYRELEASE)
        {
            keycode = gui_keyin_handler_convert_keycode(ev->key_symbol);

            if ((keycode != KEYCODE_UNKNOWN) && (this->keycode == keycode))
            {
                if (this->hold)
                    evque_set_event(EVCODE_KEYIN, (keycode | KEYIN_RELEASE | KEYIN_HOLD));
                else
                    evque_set_event(EVCODE_KEYIN, (keycode | KEYIN_RELEASE));

                this->repeat = 0;
                this->push   = false;
                this->hold   = false;
            }
        }
    }
}


static void
gui_keyin_handler_check_repeat(struct keyin_handler_interface *handler)
{
    struct keyin_handler_attribute *this = (struct keyin_handler_attribute *)handler;

    if (this->push)
    {
        if (this->repeat > LONGKEY_COUNT)
        {
            if (this->hold)
                evque_set_event(EVCODE_KEYIN, (this->keycode | KEYIN_REPEAT));
            else
            {
                evque_set_event(EVCODE_KEYIN, (this->keycode | KEYIN_HOLD));
                this->hold = true;
            }
        }
        else
        {
            this->repeat++;
            TLOGMSG(0, ("key repeat count = %d\n", this->repeat));
        }
    }
}


static void *
gui_keyin_handler_keyscan(void *arg)
{
    DFBInputEvent event;
    struct keyin_handler_interface *handler = (struct keyin_handler_interface *) arg;
    struct keyin_handler_attribute *this = (struct keyin_handler_attribute *)handler;
    IDirectFBEventBuffer *evbuf = this->eventBuffer;

    while (this->scan)
    {
        if (this->lock)
        {
            TLOGMSG(1, ("lock key input\n"));
            pthread_mutex_lock(&this->mtx);

            while(this->lock)
                pthread_cond_wait(&this->cond, &this->mtx);

            pthread_mutex_unlock(&this->mtx);

            this->push = false;
            this->hold = false;
            this->repeat = 0;
            evbuf->Reset(evbuf);
            TLOGMSG(1, ("unlock key input\n"));
        }
        else
        {
            if (evbuf->HasEvent(evbuf) == DFB_OK)
            {
                while (evbuf->GetEvent(evbuf, DFB_EVENT (&event)) == DFB_OK)
                {
                    gui_keyin_handler_check_push(handler, &event);
                    gui_keyin_handler_check_release(handler, &event);
                }
            }
            else
                gui_keyin_handler_check_repeat(handler);
        }

        MSLEEP(KEYSCAN_INTERVAL);
    }

    return NULL;
}


static void
gui_keyin_handler_lock(struct keyin_handler_interface *handler)
{
    struct keyin_handler_attribute *this = (struct keyin_handler_attribute *)handler;

    if (this)
    {
        pthread_mutex_lock(&this->mtx);
        this->lock = true;
        pthread_mutex_unlock(&this->mtx);
    }
    else
        TLOGMSG(1, (DBGINFOFMT, "null keyin handler interface\n", DBGINFO));
}


static void
gui_keyin_handler_unlock(struct keyin_handler_interface *handler)
{
    struct keyin_handler_attribute *this = (struct keyin_handler_attribute *)handler;

    if (this)
    {
        pthread_mutex_lock(&this->mtx);
        this->lock = false;
        pthread_mutex_unlock(&this->mtx);
        pthread_cond_signal(&this->cond);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null keyin handler interface\n", DBGINFO));
}


struct keyin_handler_interface *
gui_keyin_handler_create(IDirectFB *dfb)
{
    struct keyin_handler_interface *handler = NULL;
    struct keyin_handler_attribute *this = malloc(sizeof(struct keyin_handler_attribute));

    if (this)
    {
        if (dfb->CreateInputEventBuffer(dfb, DICAPS_KEYS, DFB_FALSE, &(this->eventBuffer)) == DFB_OK)
        {
            pthread_cond_init(&this->cond, NULL);
            pthread_mutex_init(&this->mtx, NULL);
            this->keycode = KEYCODE_NULL;
            this->push    = false;
            this->hold   = false;
            this->lock    = true;
            this->scan    = true;
            this->repeat  = 0;
            handler = &(this->extif);
            handler->lock   = gui_keyin_handler_lock;
            handler->unlock = gui_keyin_handler_unlock;

            if (pthread_create(&this->tid, NULL, gui_keyin_handler_keyscan, (void *)handler) == 0)
                TLOGMSG(1, ("create keyin handler interface\n"));
            else
            {
                pthread_cond_destroy(&this->cond);
                pthread_mutex_destroy(&this->mtx);
                free(this);
                handler = NULL;
                TLOGMSG(1, (DBGINFOFMT "failed to create key scan thread\n", DBGINFO));
            }
        }
        else
        {
            free(this);
            TLOGMSG(1, (DBGINFOFMT "failed to create directfb input event buffer\n", DBGINFO));
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));

    return handler;
}


int
gui_keyin_handler_destroy(struct keyin_handler_interface *handler)
{
    int ret = 0;
    struct keyin_handler_attribute *this = (struct keyin_handler_attribute *)handler;

    if (this)
    {
        this->scan = false;
        handler->unlock(handler);
        pthread_join(this->tid, NULL);
        pthread_mutex_destroy(&this->mtx);
        pthread_cond_destroy(&this->cond);
        this->eventBuffer->Release(this->eventBuffer);
        free(this);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null keyin handler interface\n", DBGINFO));
    }

    return ret;
}
