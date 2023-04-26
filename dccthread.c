#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <time.h>
#include "dlist.h"
#include "dccthread.h"

#define STACKSIZE 32768

static ucontext_t manager, context1, context2;
struct dlist thread_list;

//#include "list.h"
struct dccthread {
    ucontext_t context;
    const char *name;
    char stack[STACKSIZE];
    void (*func)(int);
    int param;
};


void dccthread_init(void (*func)(int), int param) {
    dccthread_t *main = dccthread_create("main", func, param);
    swapcontext(&manager, &main->context);
}

dccthread_t *dccthread_create(const char *name, void (*func)(int), int param) {
    dccthread_t *thread = malloc(sizeof(dccthread_t));
    getcontext(&thread->context);
    thread->name = name;
    thread->context.uc_stack.ss_sp = thread->stack;
    thread->context.uc_stack.ss_size = sizeof(thread->stack);
    thread->context.uc_link = &manager;
    makecontext(&thread->context, (void (*)(void))func, 1, param); 
    dlist_push_right(&thread_list, thread);
    return thread;
}

void dccthread_yield(void) {
}