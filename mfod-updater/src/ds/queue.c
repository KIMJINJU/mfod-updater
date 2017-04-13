/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    queue.c
        external/internal function implementations of queue interface
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
#include "ds/queue.h"


int
queue_flush(queue_t *queue)
{
	int ret = 0;
	queue_node_t *node = NULL;

	if (queue)
	{
		pthread_mutex_lock(&queue->mtx);

        while(queue->head != NULL)
        {
            free(queue->head->data);
            node = queue->head;
            queue->head = queue->head->next;
            (queue->count)--;
            free(node);
        }

        queue->head = NULL;
        queue->tail = NULL;
		pthread_mutex_unlock(&queue->mtx);
	}
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null queue interface\n", DBGINFO));
	}

	return ret;
}


int
queue_enque(queue_t *queue, void *data)
{
    int ret = 0;
    queue_node_t *node = NULL;

    if (queue)
    {
		node = malloc (sizeof(queue_node_t));

		if (node)
		{
			pthread_mutex_lock(&queue->mtx);
			node->data = data;
			node->next = NULL;

			if (queue->count == 0)
				queue->head = node;
			else
				queue->tail->next = node;

			(queue->count)++;
			queue->tail = node;
			pthread_mutex_unlock(&queue->mtx);
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "null data\n", DBGINFO));
		}
    }
	else
	{
		ret = -1;
		TLOGMSG(1, (DBGINFOFMT "null queue interface\n", DBGINFO));
	}

    return ret;
}


int
queue_deque(queue_t *queue, void **data)
{
    int ret = 0;
    queue_node_t *node = NULL;

    if (queue)
    {
		if (queue->count != 0)
		{
			pthread_mutex_lock(&queue->mtx);

			if (queue->head->data)
			{
				*data = queue->head->data;
				node = queue->head;

				if (queue->count == 1)
					queue->tail = queue->head = NULL;
				else
					queue->head = queue->head->next;

				(queue->count)--;
				free(node);
			}
			else
			{
				ret = -1;
				TLOGMSG(1, (DBGINFOFMT "queued data is null\n", DBGINFO));
			}

			pthread_mutex_unlock(&queue->mtx);
		}
		else
		{
			ret = -1;
			TLOGMSG(1, (DBGINFOFMT "queue is empty\n", DBGINFO));
		}
    }
    else
    {
    	ret = -1;
    	TLOGMSG(1, (DBGINFOFMT "null queue\n", DBGINFO));
    }

    return ret;
}


/* external functions for create/destroy queue interface */
queue_t *
queue_create(void)
{
	queue_t *queue = malloc(sizeof(queue_t));

	if (queue)
	{
		/* initailize data members */
		queue->count = 0;
		queue->head   = NULL;
		queue->tail   = NULL;
		pthread_mutex_init(&queue->mtx, NULL);
	}
	else
	{
		queue = NULL;
		TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));
	}

	return queue;
}


int
queue_destroy(queue_t *queue)
{
	int ret = 0;

    if (queue)
    {
        queue_flush(queue);
        pthread_mutex_destroy(&queue->mtx);
        free(queue);
    }
    else
    {
    	ret = -1;
    	TLOGMSG(1, (DBGINFOFMT "null queue\n", DBGINFO));
    }

    return ret;
}
