#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include "dlist.h"
#include "dccthread.h"

#define STACKSIZE 32768
#define CLOCKID CLOCK_PROCESS_CPUTIME_ID
#define INTERVAL 10000000

ucontext_t manager;
struct dlist *active_threads;
struct dlist *sleeping_threads;
sigset_t mask;
sigset_t sleeping_mask;
timer_t timer_id;
struct sigaction sig_action;
struct sigevent sig_event;
struct itimerspec sig_spec;

struct dccthread {
    ucontext_t context;
    dccthread_t *waiting_for;
    char name[128];
    char stack[STACKSIZE];
    void (*func)(int);
    int param;
    int is_yielded;
};

//dlist_find_remove removes the element when cmp is false
int compare_threads(const void *t1, const void *t2, void *userdata) {
    return (dccthread_t *) t1 != (dccthread_t *) t2;
}

void wake_thread(int sig, siginfo_t *sig_info, void *var) {
	dlist_find_remove(sleeping_threads, (dccthread_t *)sig_info->si_value.sival_ptr, compare_threads, NULL);
	dlist_push_right(active_threads, (dccthread_t *)sig_info->si_value.sival_ptr);
}

int thread_exists(dccthread_t *needle) {
    if (dlist_empty(active_threads) && dlist_empty(sleeping_threads)) return 0;

    struct dnode *current = sleeping_threads->head;

    while (current != NULL) {
        if ((dccthread_t *)current->data == needle)
            return 1;
        current = current->next;
    }

    current = active_threads->head;
    while (current != NULL) {
        if ((dccthread_t *)current->data == needle)
            return 1;
        current = current->next;
    }
    return 0;

}

void dccthread_init(void (*func)(int), int param) {
    dccthread_t *current;
    active_threads = dlist_create();
    sleeping_threads = dlist_create();
    dccthread_create("main", func, param);
    getcontext(&manager);

    sigemptyset(&mask);
	sigaddset(&mask, SIGRTMIN);
	sigaddset(&mask, SIGRTMAX);
	sigprocmask(SIG_SETMASK, &mask, NULL);	

	sigemptyset(&sleeping_mask);
	sigaddset(&sleeping_mask, SIGRTMAX);

    manager.uc_sigmask = mask;

    sig_action.sa_flags = 0;
    sig_action.sa_handler = (void *)dccthread_yield;
    sigaction(SIGRTMIN, &sig_action, NULL);

    sig_event.sigev_notify = SIGEV_SIGNAL;
    sig_event.sigev_signo = SIGRTMIN;
    sig_event.sigev_value.sival_ptr = &timer_id;
    timer_create(CLOCKID   , &sig_event, &timer_id);


	sig_spec.it_value.tv_sec = 0;
	sig_spec.it_value.tv_nsec = INTERVAL;
    sig_spec.it_interval.tv_sec = 0;
    sig_spec.it_interval.tv_nsec = INTERVAL;
	timer_settime(timer_id, 0, &sig_spec, NULL);

    while (!dlist_empty(active_threads) || !dlist_empty(sleeping_threads)) { 
		sigprocmask(SIG_UNBLOCK, &sleeping_mask, NULL);
		sigprocmask(SIG_BLOCK, &sleeping_mask, NULL);
        current = (dccthread_t *)dlist_get_index(active_threads, 0);

        if (current->waiting_for != NULL) {
            if (thread_exists(current->waiting_for)) {
                current = dlist_pop_left(active_threads);
                dlist_push_right(active_threads, current);
                continue;
            } else {
                current->waiting_for = NULL;
            }
        }

        swapcontext(&manager, &current->context);
        current = dlist_pop_left(active_threads);
        if (current->is_yielded || current->waiting_for != NULL) {
            current->is_yielded = 0;
            dlist_push_right(active_threads, current);
        }
    }
    exit(0);
}

dccthread_t *dccthread_create(const char *name, void (*func)(int), int param) {
    sigprocmask(SIG_BLOCK, &mask, NULL);

    dccthread_t *thread = malloc(sizeof(dccthread_t));
    getcontext(&thread->context);
    strcpy(thread->name, name);
    thread->context.uc_stack.ss_sp = thread->stack;
    thread->context.uc_stack.ss_size = sizeof(thread->stack);
    thread->context.uc_link = &manager;
    thread->waiting_for = NULL;
	sigemptyset(&thread->context.uc_sigmask);
    makecontext(&thread->context, (void (*)(void))func, 1, param); 
    dlist_push_right(active_threads, thread);

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
    return (dccthread_t *)dlist_get_index(active_threads, 0);
}

void dccthread_exit(void){
    sigprocmask(SIG_BLOCK, &mask, NULL);

    dccthread_t *current = dccthread_self();
    int i = 0;
    while (i < active_threads->count) {
        dccthread_t *thread = dlist_get_index(active_threads, i);
        if (thread->waiting_for == current) {
            thread->waiting_for = NULL;
        }
        i++;
    }

    free(current);
    setcontext(&manager);

    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

void dccthread_wait(dccthread_t *tid) {
    sigprocmask(SIG_BLOCK, &mask, NULL);
    dccthread_t *current = dlist_get_index(active_threads, 0);

    if(thread_exists(tid)){
        current->waiting_for = tid;
        swapcontext(&current->context, &manager);
    }

    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

void dccthread_sleep(struct timespec ts) {
    sigprocmask(SIG_BLOCK, &mask, NULL);

    dccthread_t *current = dccthread_self();
	timer_t sleeper_id;
	struct sigevent sleep_sig_event;
	struct sigaction sleep_sig_action;
	struct itimerspec sleep_sig_spec;

	sleep_sig_action.sa_flags = SA_SIGINFO | SA_NODEFER;
	sleep_sig_action.sa_sigaction = wake_thread;
	sleep_sig_action.sa_mask = mask;
	sleep_sig_event.sigev_notify = SIGEV_SIGNAL;
	sleep_sig_event.sigev_signo = SIGRTMAX;
	sleep_sig_event.sigev_value.sival_ptr = current;

	sleep_sig_spec.it_value = ts;
	sleep_sig_spec.it_interval.tv_sec = 0;
	sleep_sig_spec.it_interval.tv_nsec = 0;


	sigaction(SIGRTMAX, &sleep_sig_action, NULL);
	timer_create(CLOCK_REALTIME, &sleep_sig_event, &sleeper_id);
	timer_settime(sleeper_id, 0, &sleep_sig_spec, NULL);

	dlist_push_right(sleeping_threads, current);

	swapcontext(&current->context, &manager);

	sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

const char * dccthread_name(dccthread_t *tid) {
    return tid->name;
}
