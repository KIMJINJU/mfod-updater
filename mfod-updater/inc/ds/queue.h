/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    queue.h
        external function/variables/defines for queue interface
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _QUEUE_H_
#define _QUEUE_H_

/* structure declarations : queue node */
typedef struct queue_node
{
	int   index;
	void  *data;
	struct queue_node *next;
	struct queue_node *prev;
}
queue_node_t;

/*	structure declarations : queue data structure */
typedef struct queue
{
	int count;
    queue_node_t *head;
    queue_node_t *tail;
    pthread_mutex_t mtx;
}
queue_t;


/* external functions for create/destroy queue interface */
extern int queue_flush(queue_t *queue);
extern int queue_enque(queue_t *queue, void *data);
extern int queue_deque(queue_t *queue, void **data);
extern queue_t *queue_create(void);
extern int queue_destroy(queue_t *queue);

#endif /* _QUEUE_H_ */
