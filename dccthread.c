#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#include "dlist.h"
#include "dccthread.h"

#define STACKSIZE 32768
#define CLOCKID CLOCK_PROCESS_CPUTIME_ID

ucontext_t manager;
struct dlist *thread_list;
struct dlist *sleeping_threads;
sigset_t mask;
sigset_t sleeping_mask;
timer_t timer_id;
struct sigaction sig_action;
struct sigevent sig_event;
struct itimerspec sig_spec;

struct dccthread {
    ucontext_t context;
    ucontext_t *waiting_for;
    const char *name;
    char stack[STACKSIZE];
    void (*func)(int);
    int param;
    int is_yielded;
};

void handle_error(char *error_name) {
    printf("There has been an error executing the code: %s\n", error_name);
}

void dccthread_init(void (*func)(int), int param) {
    dccthread_t *current;
    thread_list = dlist_create();
    sleeping_threads = dlist_create();
    dccthread_create("main", func, param);
    getcontext(&manager);

    sigemptyset(&mask);
	sigaddset(&mask, SIGRTMIN);
	sigaddset(&mask, SIGRTMAX);
	sigprocmask(SIG_SETMASK, &mask, NULL);	

	sigemptyset(&sleep_mask);
	sigaddset(&sleep_mask, SIGRTMAX);

    manager.uc_sigmask = mask;

    sig_action.sa_flags = 0;
    sig_action.sa_handler = (void *)dccthread_yield;
    sigaction(SIGRTMIN, &sig_action, NULL);

    sig_event.sigev_notify = SIGEV_SIGNAL;
    sig_event.sigev_signo = SIGRTMIN;
    sig_event.sigev_value.sival_ptr = &timer_id;
    timer_create(CLOCKID, &sig_event, &timer_id);


	sig_spec.it_value.tv_sec = 0;
	sig_spec.it_value.tv_nsec = 10000000;
    sig_spec.it_interval.tv_sec = 0;
    sig_spec.it_interval.tv_nsec = 10000000;
	timer_settime(timer_id, 0, &sig_spec, NULL);

    while (!dlist_empty(thread_list) || !dlist_empty(sleeping_threads)) { 
		sigprocmask(SIG_UNBLOCK, &sleep_mask, NULL);
		sigprocmask(SIG_BLOCK, &sleep_mask, NULL);
        current = (dccthread_t *)dlist_get_index(thread_list, 0);

        if (current->waiting_for != NULL) {
            dlist_pop_left(thread_list);
            dlist_push_right(thread_list, current);
        }
        swapcontext(&manager, &current->context);
        current = dlist_pop_left(thread_list);
        if (current->is_yielded || current->waiting_for != NULL) {
            current->is_yielded = 0;
            dlist_push_right(thread_list, current);
        }
    }
    exit(0);
}

dccthread_t *dccthread_create(const char *name, void (*func)(int), int param) {
    sigprocmask(SIG_BLOCK, &mask, NULL);

    dccthread_t *thread = malloc(sizeof(dccthread_t));
    getcontext(&thread->context);
    thread->name = name;
    thread->context.uc_stack.ss_sp = thread->stack;
    thread->context.uc_stack.ss_size = sizeof(thread->stack);
    thread->context.uc_link = &manager;
    thread->waiting_for = NULL;
	sigemptyset(&thread->context.uc_sigmask);
    makecontext(&thread->context, (void (*)(void))func, 1, param); 
    dlist_push_right(thread_list, thread);

    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    return thread;
}

void dccthread_yield(void) {
    sigprocmask(SIG_BLOCK, &mask, NULL);

    dccthread_t *current = dccthread_self();
    current->is_yielded = 1;
    swapcontext(&current->context, &manager);

    sigprocmask(SIG_UNBLOCK, &mask, NULL);

}

dccthread_t *dccthread_self(void) {
    return (dccthread_t *)dlist_get_index(thread_list, 0);
}

void dccthread_exit(void){
    sigprocmask(SIG_BLOCK, &mask, NULL);

    dccthread_t *current = dccthread_self();
    for (int i = 0; i < thread_list->count; i++) {
        dccthread_t *thread = dlist_get_index(thread_list, i);
        if (thread->waiting_for == current) {
            thread->waiting_for = NULL;
        }
    }
    
    swapcontext(&current->context, &manager);

    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

void dccthread_wait(dccthread_t *tid) {
    sigprocmask(SIG_BLOCK, &mask, NULL);

    int finished = 1;
    for (int i = 0; i < thread_list->count; i++) {
        dccthread_t *thread = dlist_get_index(thread_list, 0);
        if (thread == tid) {
            finished = 0;
            break;
        }
    }

    if (finished) {
        dccthread_t *current = dlist_get_index(thread_list, 0);
        current->waiting_for = tid;
        swapcontext(&current->context, &manager);
    }

    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

void dccthread_sleep(struct timespec ts) {
    dccthread_t *current = dccthread_self();
	timer_t sleeper_id;
	struct sigevent sleep_sig_event;
	struct sigaction sleep_sig_action;
	struct itimerspec sleep_sig_spec;

	sleep_sig_action.sa_flags = SA_SIGINFO;
	sleep_sig_action.sa_sigaction = wake_thread;
	sleep_sig_action.sa_mask = mask;
	sigaction(SIGRTMAX, &t_sleep_act, NULL);

	sleep_sig_event.sigev_notify = SIGEV_SIGNAL;
	sleep_sig_event.sigev_signo = SIGRTMAX;
	sleep_sig_event.sigev_value.sival_ptr = current_thread;
	timer_create(CLOCK_REALTIME, &sig_sleep_event, &sleeper_id);

	sleep_sig_spec.it_value = ts;
	sleep_sig_spec.it_interval.tv_sec = 0;
	sleep_sig_spec.it_interval.tv_nsec = 0;
	timer_settime(t_sleep, 0, &time_out_CPU, NULL);

	dlist_push_right(sleeping_threads, current);

	swapcontext(&current_thread->context, &manager);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
}
(
void wake_thread() {
    printf("acorda fedaputa\n");
}

const char * dccthread_name(dccthread_t *tid) {
    return tid->name;
}
