/*
 * ECE 357: Computer Operating Systems 
 * Problem Set 6: Problem 2 
 * By: Guy Bar Yosef 
 *
 * semaphore.h - Semaphore header file.
 * 
 * Includes also the N_PROC definition, which defines 
 * maximum number of 'virtual processors'(processes).
 * 
 * version: 1.0
 */


/**** max process count ****/
#ifndef N_PROC
#define N_PROC 64

#endif


/**** semaphore ****/
#ifndef SEMAPHORE
#define SEMAPHORE

/* NOTE: The sleep/wake implementation depends on receiving 
   the USRSIG1 signal. Because this signal terminates programs
   by default, whichever program that wishes to use this 
   Semaphore implementation should handle the signal. */


/* struct for the semaphore */
typedef struct Semaphore {
    char lock;  /* used to implement mutex-locking */
    int count;  /* semaphore count value */ 
    int sleep_count; /* number of times the semaphore went to sleep */
    
    /* array of processes waiting on the semaphore */
    int waiting_processes[N_PROC]; 
    int waiting_process_index;
} Semaphore;


/*
 * sem_init - Initializes the semaphore with the 
 * specified count.
 * 
 * NOTE: Call this function prior to any usage of 
 * the semaphore.
 */
void sem_init(Semaphore *s, int count);


/*
 * sem_try - 'Tries' to perform the 'P' operation, 
 * i.e. atomically decrement the semaphore. If the
 * count is 0, sem_try does not decrement it further.
 * 
 * Returns:
 *  - 0: If the count of the semaphore is 0.
 *  - 1: If the count of the semaphore is positive. 
 */
int sem_try(Semaphore *s);


/*
 * sem_wait - Performs the 'P' operation, decrementing
 * the semaphore. 
 * 
 * If the count was positive, sem_wait returns after 
 * decrementing, otherwise sem_wait blocks until the 
 * count becomes positive and it can decrement.
 */
void sem_wait(Semaphore *s);


/*
 * sem_inc - Performs the 'V' operation- atomically 
 * incrementing the semaphore.
 * 
 * If any other tasks were sleeping on the semaphore,
 * send them a SIGUSR1 signal to wake them up.
 */
void sem_inc(Semaphore *s);

#endif