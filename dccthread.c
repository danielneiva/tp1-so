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
dccthread_t *current;

struct dccthread {
    ucontext_t context;
    const char *name;
    char stack[STACKSIZE];
    void (*func)(int);
    int param;
    int isRunning;
};

void schedule(void) {
    while(thread_list->head != NULL) {
        current = dlist_pop_left(thread_list);
        swapcontext(&manager, &current->context); 
        if (current->isRunning) {
            dlist_push_right(thread_list, current);
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
}

dccthread_t *dccthread_create(const char *name, void (*func)(int), int param) {
    dccthread_t *thread = malloc(sizeof(dccthread_t));
    getcontext(&thread->context);
    thread->name = name;
    thread->isRunning = 0;
    thread->context.uc_stack.ss_sp = thread->stack;
    thread->context.uc_stack.ss_size = sizeof(thread->stack);
    thread->context.uc_link = &manager;
    makecontext(&thread->context, (void (*)(void))func, 1, param); 
    dlist_push_right(thread_list, thread);
    return thread;
}

void dccthread_yield(void) {
    current->isRunning = 1;
    swapcontext(&current->context, &manager);
}

dccthread_t *dccthread_self(void) {
    return current;
}

void dccthread_exit(void){
    swapcontext(&current->context, &manager);
}

void dccthread_wait(dccthread_t *tid) {
    tid->context.uc_link = &current->context;
    swapcontext(&current->context, &manager);
}

void dccthread_sleep(struct timespec ts) {
    //
}

const char * dccthread_name(dccthread_t *tid) {
    return tid->name;
}
