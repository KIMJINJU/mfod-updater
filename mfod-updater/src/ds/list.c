/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    list.c
        external/internal function implementations of double linked list data structure
        list file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/logger.h"
#include "ds/list.h"


static int
list_update_index(list_t *list)
{
    int ret = 0;
    list_node_t *node = NULL;

    if (list)
    {
        node = list->head;

        for (int i = 0; i < list->count; i++)
        {
            node->index = i;
            node = node->next;
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null list data structure\n", DBGINFO));
    }

    return ret;
}


int
list_insert_node(list_t *list, void *data)
{
    int ret = 0;
    list_node_t *node = NULL;

    if (list)
    {
        node = malloc(sizeof(list_node_t));

        if (node)
        {
            pthread_mutex_lock(&list->mutex);
            node->data = data;

            if (list->count == 0)
            {
                node->next = node;
                node->prev = node;
                list->head = node;
            }
            else
            {
                node->next = list->head;
                node->prev = list->tail;
                list->head->prev = node;
                list->tail->next = node;
            }

            list->tail = node;
            node->index = list->count;
            (list->count)++;
            list_update_index(list);
            pthread_mutex_unlock(&list->mutex);
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
        TLOGMSG(1, (DBGINFOFMT "null list data structure\n", DBGINFO));
    }

    return ret;
}


int
list_remove_node(list_t *list, int idx)
{
    int ret = 0;
    list_node_t *node = NULL;

    if (list)
    {
        if (list->count > 0)
        {
            pthread_mutex_lock(&list->mutex);
            node = list->head;

            for(int i = 0; i < list->count; i++)
            {
                if (node->index == idx)
                {
                    free(node->data);

                    if (node == list->head)
                        list->head = node->next;
                    else if (node == list->tail)
                        list->tail = node->prev;

                    node->prev->next = node->next;
                    node->next->prev = node->prev;
                    (list->count)--;
                    free(node);
                    list_update_index(list);
                    break;
                }
                else
                    node = node->next;
            }

            pthread_mutex_unlock(&list->mutex);
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "list is empty\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null list data structure\n", DBGINFO));
    }

    return ret;
}


list_node_t *
list_get_node(list_t *list, int idx)
{
    list_node_t *node = NULL;

    if (list)
    {
        if (idx < list->count)
        {
            node = list->head;

            for (int i = 0; i < list->count; i++)
            {
                if (node->index == idx)
                    break;
                else
                    node = node->next;
            }
        }
        else
            TLOGMSG(1, (DBGINFOFMT "invalid index\n", DBGINFO));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null list data structure\n", DBGINFO));

    return node;
}


list_t *
list_create(void)
{
    list_t *list = malloc(sizeof(list_t));

    if (list)
    {
        list->head = NULL;
        list->tail = NULL;
        list->count = 0;
        pthread_mutex_init(&list->mutex, NULL);
        TLOGMSG(0, ("init list data structure\n"));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));

    return list;
}


int
list_destroy(list_t *list)
{
    int ret = 0;
    list_node_t *node = NULL;

    if (list)
    {
        while (list->count != 0)
        {
            free(list->head->data);
            node = list->head;
            list->head = list->head->next;
            free(node);
            (list->count)--;
        }

        pthread_mutex_destroy(&list->mutex);
        free(list);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null list data structure\n", DBGINFO));
    }

    return ret;
}
