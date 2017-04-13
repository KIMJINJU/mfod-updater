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

#include "core/evque.h"
#include "core/logger.h"
#include "ds/queue.h"

/* structure declaration : event queue */
struct event_queue
{
    queue_t *events;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
};

/* global event queue */
static struct event_queue *evque = NULL;


int
evque_create(void)
{
    int ret = 0;

    evque = malloc(sizeof(struct event_queue));

    if (evque)
    {
        evque->events = queue_create();

        if (evque->events)
        {
            pthread_mutex_init(&evque->mutex, NULL);
            pthread_cond_init(&evque->cond, NULL);
            TLOGMSG(1, ("create global event queue\n"));
        }
        else
        {
            free(evque);
            evque = NULL;
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to create global event queue\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));
    }

    return ret;
}


int
evque_destroy(void)
{
    int ret = 0;

    if (evque)
    {
        evque_flush();
        queue_destroy(evque->events);
        pthread_mutex_destroy(&evque->mutex);
        pthread_cond_destroy(&evque->cond);
        free(evque);
        evque = NULL;
        TLOGMSG(1, ("destroy global event queue\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null global event queue\n", DBGINFO));
    }

    return ret;
}


int
evque_set_event(unsigned int evcode, unsigned int evparm)
{
    int ret = 0;
    event_t *event = NULL;

    if (evque)
    {
        event = malloc(sizeof(event_t));

        if (event)
        {
            event->code = evcode;
            event->parm = evparm;
            pthread_mutex_lock(&evque->mutex);
            queue_enque(evque->events, (void *) event);
            pthread_mutex_unlock(&evque->mutex);
            pthread_cond_signal(&evque->cond);
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null global event queue\n", DBGINFO));
    }

    return ret;
}


event_t *
evque_get_event(void)
{
    event_t *ev = NULL;

    if (evque)
    {
        pthread_mutex_lock(&evque->mutex);

        while (evque->events->count == 0)
            pthread_cond_wait(&evque->cond, &evque->mutex);

        queue_deque(evque->events, (void *) &ev);

        pthread_mutex_unlock(&evque->mutex);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null global event queue\n", DBGINFO));

    return ev;
}


int
evque_flush(void)
{
    int ret = 0;
    event_t *ev = NULL;

    if (evque)
    {
        pthread_mutex_lock(&evque->mutex);

        while (evque->events->count != 0)
        {
            queue_deque(evque->events, (void *) &ev);
            free(ev);
        }

        pthread_mutex_unlock(&evque->mutex);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null global event queue\n", DBGINFO));
    }

    return ret;
}
