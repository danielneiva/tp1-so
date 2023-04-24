#include <stdio.h>
#include <unistd.h>
#include "dccthread.h"
#include "list.h"

void prin(int x) {
    printf("int: %d\n", x);
}

void *schedule_threads(struct list *list) {
    if (list->tail != NULL) {
        dccthread_t *node = list->tail->next;
        while (node != NULL){
            pthread_t worker;
            pthread_create(&worker, NULL, node->func, node->param);
            node = node->next;
            sleep(1);
        }
    }
}

void dccthread_init(void (*func)(int), int param) {
    struct list *list = list_create();
    pthread_t manager;
    pthread_create(&manager, NULL, schedule_threads, list);
}

dccthread_t * dccthread_create(const char *name, void (*func)(int ), int param){
    dccthread_t *node;
    node->name = name;
    node->func = func;
    node->param = param;
    
    return node;
}