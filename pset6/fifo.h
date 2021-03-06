/*
 * ECE 357: Computer Operating Systems 
 * Problem Set 6: Problem 3 
 * By: Guy Bar Yosef 
 *
 * fifo.h - FIFO header file.
 * 
 * version: 1.0
 */


#include "sem.h"


#ifndef FIFO
#define FIFO

#define MYFIFO_BUFSIZE 4096

/* struct definition */
typedef struct Fifo {
    /* readers and writers have different restrictions, so we need a semaphore
       for each task. Moreso, we need to maintain a spinlock functionality (we 
       will use binary semaphores) as well as the inherent sleep/wake 
       functionality of semaphores. Therefore, overall we need 4 semaphores.  */
    Semaphore sem_reader;
    Semaphore sem_writer;
    Semaphore sem_full;
    Semaphore sem_empty;
    
    unsigned long queue[MYFIFO_BUFSIZE];
    unsigned int wr_index;  /* write pointer */
    unsigned int rd_index;  /* read pointer */

} Fifo;


/*
 * fifo_init - Initializes the shared memory FIFO.
 * 
 * NOTE: Call this function before any usage of the fifo.
 */
void fifo_init(Fifo *f);


/*
 * fifo_wr - Enqueues the data d into the FIFO, blocking
 * unless and until the FIFO has room to accept it.
 * 
 * All processes that had been blocked due to the empty FIFO 
 * get awoken.
 */
void fifo_wr(Fifo *f, unsigned long d);


/*
 * fifo_rd - Dequeues the next data word from the FIFO and
 * returns it.
 * 
 * Will block unless and until there is an available word
 * queued in the FIFO. All processes that had been blocked
 * due to the FIFO being full will get awoken.
 */
unsigned long fifo_rd(Fifo *f);

#endif