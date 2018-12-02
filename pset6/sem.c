/*
 * ECE 357: Computer Operating Systems 
 * Problem Set 6: Problem 2 
 * By: Guy Bar Yosef 
 *
 * semaphore.c - Implements the 4 semaphore 
 * functions defined in sem.h
 * 
 * version: 1.0
 */


#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include "sem.h"
#include "spinLockLib.c"


/*
 * sem_init - Initializes the semaphore with the 
 * specified count.
 * 
 * NOTE: Call this function prior to any usage of 
 * the semaphore.
 */
void sem_init(Semaphore *s, int count) {
    s->count = 0;
    s->lock = 0;
    s->waiting_process_index = 0;
}


/*
 * sem_try - 'Trys' to perform the 'P' operation, 
 * i.e. atomically decrement the semaphore. If the
 * count is 0, sem_try does not decrement it further.
 * 
 * Returns:
 *  - 0: If the count of the semaphore is 0.
 *  - 1: If the count of the semaphore is positive. 
 */
int sem_try(Semaphore *s) {
    if (s->count > 0) {

        spin_lock(&s->lock);
        s->count--; /* CRITICAL REGION */
        spin_unlock(&s->lock);

        return 1;
    }
    else 
        return 0;
}


/*
 * sem_wait - Performs the 'P' operation, decrementing
 * the semaphore. 
 * 
 * If the count was positive, sem_wait returns after 
 * decrementing, otherwise sem_wait blocks until the 
 * count becomes positive and it can decrement.
 */
void sem_wait(Semaphore *s) {
    if (s->count > 0) {

        spin_lock(&s->lock);
        s->count--;          /* CRITICAL REGION */  
        spin_unlock(&s->lock);
    }
    else { /* block process */

        /* block SIGUSR1 until process goes to sleep */
        /* - solves the 'lost wakeup' problem */
        sigset_t signal_set;
        sigemptyset(&signal_set);
        sigaddset(&signal_set, 30); /* add SIGUSR1 to blocked list */
        sigprocmask(SIG_BLOCK, &signal_set, NULL); 
    
        pid_t cur_process_pid = getpid(); /* get process id */

        spin_lock(&s->lock);
        /* beginning of CRITICAL REGION */
        s->waiting_processes[s->waiting_process_index] = cur_process_pid;
        s->waiting_process_index++;
        /* end of CRITICAL REGION */
        spin_unlock(&s->lock);

        /* modify signal set to only let through SIGUSR1 (sig #30) */
        sigfillset(&signal_set);
        sigdelset(&signal_set, 30); 
        sigsuspend(&signal_set); /* block signal b */ 
        errno = 0; /* sigsuspend turns errno into non-zero */
    }
}


/*
 * sem_inc - Performs the 'V' operation- atomically 
 * incrementing the semaphor.
 * 
 * If any other tasks were sleeping on the semaphor,
 * send them a SIGUSR1 signal to wake them up.
 */
void sem_inc(Semaphore *s) {

    /* if count=0, wake up all the sleeping processes */
    if (s->count == 0) {
        spin_lock(&s->lock);
        /* begin CRITICAL REGION */
        for (int i = 0 ; i < s->waiting_process_index; ++i) {
            kill(s->waiting_processes[i], 30);
        }
        s->waiting_process_index = 0;
        /* end CRITICAL REGION */
        spin_unlock(&s->lock);
    }

    spin_lock(&s->lock);
    s->count++;         /* CRITICAL REGION */
    spin_unlock(&s->lock);
}