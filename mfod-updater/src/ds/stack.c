/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    stack.c
        external/internal function implementations of stack data structure interface
        stk file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/logger.h"
#include "ds/stack.h"


int
stack_push(stk_t *stk, void *data)
{
    int ret = 0;
    stk_node_t *node = NULL;

    if (stk)
    {
        node = malloc(sizeof(stk_node_t));

        if (node)
        {
            pthread_mutex_lock(&stk->mtx);
            node->prev = NULL;
            node->next = stk->top;
            node->data = data;
            stk->top = node;
            (stk->count)++;

            if (stk->top->next)
                stk->top->next->prev = stk->top;
            else
                stk->bottom = stk->top;

            pthread_mutex_unlock(&stk->mtx);
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to create stack node\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null stack interface\n", DBGINFO));
    }

    return ret;
}


void *
stack_pop(stk_t *stk)
{
    void *data = NULL;
    stk_node_t *node = NULL;

    if (stk)
    {
        pthread_mutex_lock(&stk->mtx);

        if (stk->count != 0)
        {
            node = stk->top;
            data = stk->top->data;
            stk->top = stk->top->next;
            (stk->count)--;
            free(node);

            if (stk->top)
                stk->top->prev = NULL;
            else
                stk->bottom = NULL;
        }
        else
            TLOGMSG(0, (DBGINFOFMT "stack is empty\n", DBGINFO));

        pthread_mutex_unlock(&stk->mtx);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null stack interface\n", DBGINFO));

    return data;
}


void *
stack_get_top(stk_t *stk)
{
    void *data = NULL;

    if (stk)
    {
        pthread_mutex_lock(&stk->mtx);

        if (stk->count != 0)
            data = stk->top->data;
        else
            TLOGMSG(0, (DBGINFOFMT "stack is empty\n", DBGINFO));

        pthread_mutex_unlock(&stk->mtx);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null stack interface\n", DBGINFO));

    return data;
}


void *
stack_get_bottom(stk_t *stk)
{
    void *data = NULL;

    if (stk)
    {
        pthread_mutex_lock(&stk->mtx);

        if (stk->count != 0)
            data = stk->bottom->data;
        else
            TLOGMSG(0, (DBGINFOFMT "stack is empty\n", DBGINFO));

        pthread_mutex_unlock(&stk->mtx);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null stack interface\n", DBGINFO));

    return data;
}


int
stack_is_empty(stk_t *stk)
{
    int ret = 0;

    if (stk)
    {
        if (stk->count == 0)
            ret = 0;
        else
            ret = 1;
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null stack interface\n", DBGINFO));
    }

    return ret;
}


stk_t *
stack_create(void)
{
    stk_t *stk = malloc(sizeof(stk_t));

    if (stk)
    {
        stk->top    = NULL;
        stk->bottom = NULL;
        stk->count  = 0;
        pthread_mutex_init(&stk->mtx, NULL);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));

    return stk;
}


int
stack_destroy(stk_t *stk)
{
    int ret = 0;
    stk_node_t *node = NULL;

    if (stk)
    {
        while (stk->count > 0)
        {
            free(stk->top->data);
            node = stk->top;
            stk->top = stk->top->next;
            free(node);
            (stk->count)--;
        }

        pthread_mutex_destroy(&stk->mtx);
        free(stk);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null stack interface\n", DBGINFO));
    }

    return ret;
}
