#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <time.h>
#include "dlist.h"
#include "dccthread.h"

#define STACKSIZE 32768

static ucontext_t manager;

//#include "list.h"
struct dccthread {
    ucontext_t context;
    const char *name;
    char stack[STACKSIZE];
    void (*func)(int);
    int param;
};

struct dlist *thread_list;


void dccthread_init(void (*func)(int), int param) {
    // thread_list = dlist_create();
    // //dccthread_t *manager = dccthread_create("manager", schedule, 0);

    dccthread_t *main = dccthread_create("main", func, param);
    ucontext_t current;
    swapcontext(&current, &main->context);
    exit(0);
}

dccthread_t *dccthread_create(const char *name, void (*func)(int), int param) {
    dccthread_t *thread = malloc(sizeof(dccthread_t));
    getcontext(&thread->context);

    thread->context.uc_stack.ss_sp = thread->stack;
    thread->context.uc_stack.ss_size = sizeof(thread->stack);
    thread->context.uc_link = &manager;
    makecontext(&thread->context, thread->func, thread->param);  // 修改getcontext获取的ucp，如果ucp后面通过setcontext/swapcontext被激活，则函数func将被调用
    thread->name = name;
    thread->func = func;
    thread->param = param;
    dlist_push_right(thread_list, thread);
    return thread;
}

void dccthread_yield(void) {
}