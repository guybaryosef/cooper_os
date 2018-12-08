/*
 * ECE 357: Computer Operating Systems 
 * Problem Set 6: Problem 2 
 * By: Guy Bar Yosef 
 *
 * semaphore.c - Implements the semaphore module defined in sem.h
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

    /* get the mutex functionality of the semaphore set up first */
    s->lock = 0; 
    
    spin_lock(&s->lock);
    /* begin CRITICAL REGION */
    s->count = count;
    s->waiting_process_index = 0;
    s->sleep_count = 0;
    /* end CRITICAL REGION */
    spin_unlock(&s->lock);
}


/*
 * sem_try - 'Tries' to perform the 'P' operation, 
 * i.e. atomically decrement the semaphore. If the
 * count is 0, sem_try does not decrement it further.
 * 
 * Returns:
 *  - 0: If the count of the semaphore is 0.
 *  - 1: If the count of the semaphore was positive 
 * prior to decrementing. 
 */
int sem_try(Semaphore *s) {
    
    spin_lock(&s->lock);
    /* entering CRITICAL REGION */

    if (s->count > 0) {
        s->count--;  
        /* exiting CRITICAL REGION */
        spin_unlock(&s->lock);

        return 1;
    }
    else {
        spin_unlock(&s->lock);
        return 0;
    }
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

    /* block SIGUSR1 until process goes to sleep. This
       deals with the 'lost wakeup' problem. After wakeup,
       the original signal mask of the program is restored. */
    sigset_t mask, org_mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1); /* add SIGUSR1 to blocked list */
    sigprocmask(SIG_BLOCK, &mask, &org_mask);

    /* although only needed if going to sleep, getpid is called 
       now for spin-lock efficiency. */
    pid_t cur_process_pid = getpid();

    spin_lock(&s->lock);
    /* beginning of CRITICAL REGION */
    while (s->count == 0) {    /* block process */
    
        s->sleep_count++;      /* increment semaphore's sleep count */
        s->waiting_processes[s->waiting_process_index] = cur_process_pid;
        s->waiting_process_index++;

        /* end of CRITICAL REGION */
        spin_unlock(&s->lock);

        /* modify signal set to only let through SIGUSR1 */
        sigfillset(&mask);
        sigdelset(&mask, SIGUSR1); 
        sigsuspend(&mask);  /* block signal b */ 

        errno = 0; /* sigsuspend turns errno into non-zero value */
        
        spin_lock(&s->lock);
        /* re-entering CRITICAL REGION */
    }

    /* return semaphore signal mask back to original */
    sigprocmask(SIG_SETMASK, &org_mask, NULL);

    s->count--;    
    /* end of CRITICAL REGION */  
    spin_unlock(&s->lock);
}


/*
 * sem_inc - Performs the 'V' operation- atomically 
 * incrementing the semaphore.
 * 
 * If any other tasks were sleeping on the semaphore,
 * send them a SIGUSR1 signal to wake them up.
 */
void sem_inc(Semaphore *s) {
    spin_lock(&s->lock);
    /* begin CRITICAL REGION */
    s->count++;

    /* if count=0, wake up all the sleeping processes */
    if (s->count == 1) {
        /* send SIGUSR1 to awake sleeping processes */
        for (int i = 0 ; i < s->waiting_process_index; ++i)
            kill(s->waiting_processes[i], SIGUSR1); 

        s->waiting_process_index = 0;
    }
    /* end CRITICAL REGION */
    spin_unlock(&s->lock);
}