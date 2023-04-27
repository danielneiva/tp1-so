#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <time.h>
#include "dlist.h"
#include "dccthread.h"

#define STACKSIZE 32768

ucontext_t manager;
struct dlist *thread_list;
struct dlist *done_thread_list;
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
    int isDone;
};

void schedule(void) {
    dccthread_t *next;
    while(thread_list->head != NULL) {
        next = (dccthread_t *)thread_list->head->data;
        next->isRunning = 1;
        next->isDone = 1;
        swapcontext(&manager, &next->context); //
        if (!next->isDone) {
            next->isRunning = 0;
        }
        next = dlist_pop_left(thread_list);
        if (!next->isDone) {
            dlist_push_right(thread_list, next);
        }
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
    done_thread_list = dlist_create();
    dccthread_create("main", func, param);
    setcontext(&manager);
    while(1){}
}

dccthread_t *dccthread_create(const char *name, void (*func)(int), int param) {
    dccthread_t *thread = malloc(sizeof(dccthread_t));
    getcontext(&thread->context);
    thread->name = name;
    thread->isRunning = 0;
    thread->isDone = 1;
    thread->context.uc_stack.ss_sp = thread->stack;
    thread->context.uc_stack.ss_size = sizeof(thread->stack);
    thread->context.uc_link = &manager;
    makecontext(&thread->context, (void (*)(void))func, 1, param); 
    dlist_push_right(thread_list, thread);
    return thread;
}

void dccthread_yield(void) {
    struct dnode *head = thread_list->head;
    if (head == NULL) return;
    dccthread_t *current = (dccthread_t *) head->data;
    if (current != NULL) {
        current->isDone = 0;
        swapcontext(&current->context, &manager);
    }
}

dccthread_t *dccthread_self(void) {
    struct dnode *node = thread_list->head;
    dccthread_t *data;
    while(node != NULL) {
        data = (dccthread_t *) node->data;
        if (data->isRunning == 1) return data;
        node = node->next;
    }
    return NULL;
}

void dccthread_exit(void){
    //
}

void dccthread_wait(dccthread_t *tid) {
    //
}

void dccthread_sleep(struct timespec ts) {
    //
}

const char * dccthread_name(dccthread_t *tid) {
    return tid->name;
}