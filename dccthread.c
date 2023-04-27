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

struct dccthread {
    ucontext_t context;
    ucontext_t *waiting_for;
    const char *name;
    char stack[STACKSIZE];
    void (*func)(int);
    int param;
    int is_yielded;
};

void dccthread_init(void (*func)(int), int param) {
    dccthread_t *current;
    thread_list = dlist_create();
    dccthread_create("main", func, param);

    while (!dlist_empty(thread_list)) { 
        current = (dccthread_t *)dlist_get_index(thread_list, 0);

        if (current->waiting_for != NULL) {
            dlist_pop_left(thread_list);
            dlist_push_right(thread_list, current);
        }
        swapcontext(&manager, &current->context);
        current = dlist_pop_left(thread_list);
        if (current->is_yielded) {
            current->is_yielded = 0;
            dlist_push_right(thread_list, current);
        }
    }
    exit(0);
}

dccthread_t *dccthread_create(const char *name, void (*func)(int), int param) {
    dccthread_t *thread = malloc(sizeof(dccthread_t));
    getcontext(&thread->context);
    thread->name = name;
    thread->context.uc_stack.ss_sp = thread->stack;
    thread->context.uc_stack.ss_size = sizeof(thread->stack);
    thread->context.uc_link = &manager;
    thread->waiting_for = NULL;
    makecontext(&thread->context, (void (*)(void))func, 1, param); 
    dlist_push_right(thread_list, thread);
    return thread;
}

void dccthread_yield(void) {
    dccthread_t *current = dccthread_self();
    current->is_yielded = 1;
    swapcontext(&current->context, &manager);
}

dccthread_t *dccthread_self(void) {
    return dlist_get_index(thread_list, 0);
}

void dccthread_exit(void){
    dccthread_t *current = dccthread_self();
    for (int i = 0; i < thread_list->count; i++) {
        dccthread_t *thread = dlist_get_index(thread_list, i);
        if (thread->waiting_for == current) {
            thread->waiting_for = NULL;
        }
    }
    //free(current->context.uc_stack.ss_sp);
    current->is_yielded = 0;
    setcontext(&manager);
}

void dccthread_wait(dccthread_t *tid) {
    int finished = 1;
    for (int i = 0; i < thread_list->count; i++) {
        dccthread_t *thread = dlist_get_index(thread_list, 0);
        if (thread == tid) {
            finished = 0;
            break;
        }
    }

    if (!finished) {
        dccthread_t *current = dlist_get_index(thread_list, 0);
        current->waiting_for = tid;
        swapcontext(&current->context, &manager);
    }
}

void dccthread_sleep(struct timespec ts) {
    //
}

const char * dccthread_name(dccthread_t *tid) {
    return tid->name;
}
