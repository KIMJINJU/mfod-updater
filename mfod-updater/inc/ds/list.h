/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    list.h
        external function/variables/defines for doubly linked list data structure
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _LIST_H_
#define _LIST_H_

/* structure declaraton : list node */
typedef struct list_node
{
    int index;
    void *data;
    struct list_node *next;
    struct list_node *prev;
}
list_node_t;

/* structure declaration : list data structure */
typedef struct list
{
    int count;
    struct list_node *head;
    struct list_node *tail;
    pthread_mutex_t mutex;
}
list_t;


/* extenal functions for list data structure */

extern int list_insert_node(list_t *list, void *data);
extern int list_remove_node(list_t *list, int idx);
extern list_node_t *list_get_node(list_t *list, int idx);
extern list_t *list_create(void);
extern int list_destroy(list_t *list);

#endif /* _LIST_H_ */
