#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <time.h>
#include "dlist.h"
#include "dccthread.h"

#define STACKSIZE 32768

ucontext_t manager;
struct dlist *thread_list;
int currentIndex = 0;

// typedef struct schedule {
//     ucontext_t context;
//     char stack[STACKSIZE];
//     int running;
//     dccthread_t **threads;
// } schedule_t;
struct dccthread {
    ucontext_t context;
    const char *name;
    char stack[STACKSIZE];
    void (*func)(int);
    int param;
    int isRunning;
};

void schedule(void) {
    while(1) {
        dccthread_t *current = dccthread_self();
        dccthread_yield();
        dlist_push_right(thread_list, current);
        dccthread_t *next = dlist_pop_left(thread_list);
        swapcontext(&manager, &next->context);
    }
}

void dccthread_create_manager(void) {
    getcontext(&manager);
    manager.uc_stack.ss_sp = malloc(STACKSIZE);
    manager.uc_stack.ss_size = STACKSIZE;
    manager.uc_link = NULL;
    makecontext(&manager, schedule, 0);
}

void dccthread_init(void (*func)(int), int param) {
    dccthread_create_manager();
    thread_list = dlist_create();
    dccthread_t *main = dccthread_create("main", func, param);
    setcontext(&manager);
}

dccthread_t *dccthread_create(const char *name, void (*func)(int), int param) {
    dccthread_t *thread = malloc(sizeof(dccthread_t));
    getcontext(&thread->context);
    thread->name = name;
    thread->context.uc_stack.ss_sp = thread->stack;
    thread->context.uc_stack.ss_size = sizeof(thread->stack);
    thread->context.uc_link = &manager;
    thread->isRunning = 0;
    makecontext(&thread->context, (void (*)(void))func, 1, param); 
    dlist_push_right(thread_list, thread);
    return thread;
}

void dccthread_yield(void) {
    dccthread_t *current = dccthread_self();
    if (current != NULL) {
        current->isRunning = 0;
        swapcontext(&current->context, &manager);
    }
}

dccthread_t *dccthread_self(void) {
    struct dnode *aux = thread_list->head;
    if (aux == NULL || aux->data == NULL) return NULL;
    dccthread_t *data = (dccthread_t *) aux->data;
    if (thread_list->count == 1 && data->isRunning == 0) return NULL;
    while(data->isRunning != 1 && aux->next != NULL) {
        aux = aux->next;
        data = (dccthread_t *) aux->data;
    }
    return aux->data;
}