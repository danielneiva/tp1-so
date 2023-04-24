#include <stdlib.h>
#include <stdio.h>
#include "list.h"
struct list *list_create() {
    struct list *list = malloc(sizeof(struct list));
    list->tail = NULL;
    return list;
}
void list_destroy(struct list *list){
    while(!list_is_empty(list)){
        pop(list);
    }
    free (list);
}
void pop(struct list *list){
    if (list->tail != NULL) {
        if (list->tail->next == list->tail) {
            list->tail = NULL;
        } else {
            dccthread_t *node = list->tail->next;
            list->tail->next = node->next;

            free(node);
        }
    }
}

int list_is_empty(struct list *list){
    return list->tail == NULL;
}
void push_head(struct list *list, dccthread_t *node){
    if (list_is_empty(list)) {
        list->tail = node;
    } else {
        dccthread_t *second = list->tail->next->next;
        node->next = second;
        list->tail->next = node;
    }
}
void push_tail(struct list *list, dccthread_t *node){
    if (list_is_empty(list)) {
        list->tail = node;
        list->tail->next = list->tail;
    } else if (list->tail->next == list->tail) {
        node->next = list->tail;
        list->tail->next = node;
    } else {
        node->next = list->tail->next;
        list->tail->next = node;
        list->tail = list->tail->next;
    }
}
void traverse(struct list *list, void (*func)(int)){
    if (list->tail != NULL) {
        dccthread_t *node = list->tail->next;
        while (node != list->tail){
            node = node->next;
        }
    }
}