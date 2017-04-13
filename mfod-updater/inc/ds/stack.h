/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    stack.h
        external function/variables/defines for stack data structure interface
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#ifndef _STACK_H_
#define _STACK_H_


/* structure declaration : stack node */
typedef struct stack_node
{
	int index;
	void *data;
	struct stack_node *next;
	struct stack_node *prev;
}
stk_node_t;

/* structure declaration : stack data structure */
typedef struct stack
{
	int count;
	stk_node_t *top;
	stk_node_t *bottom;
	pthread_mutex_t mtx;
}
stk_t;


/* external functions for create/destroy stack interface */
extern int stack_push(stk_t *stk, void *data);
extern void *stack_pop(stk_t *stk);
extern void *stack_get_top(stk_t *stk);
extern void *stack_get_bottom(stk_t *stk);
extern int stack_is_empty(stk_t *stk);
extern stk_t *stack_create(void);
extern int stack_destroy(stk_t *stk);

#endif /* _STACK_H_ */
