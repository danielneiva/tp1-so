#ifndef _LIST_H_
#define _LIST_H_
#include <pthread.h>
#include "dccthread.h"

struct list {
    dccthread_t *tail;
};

struct dccthread {
    char *name;
    dccthread_t *next;
    pthread_t thread;
    void *func;
    int param;
};

struct list *list_create();
void list_destroy(struct list *list);
int list_is_empty(struct list * list);
void pop(struct list *list);
void push_head(struct list *list, dccthread_t *node);
void push_tail(struct list *list, dccthread_t *node);
void traverse(struct list *list, void (*func)(int));


#endif