/*
 * ECE 357: Computer Operating Systems 
 * Problem Set 6: Problem 3 
 * By: Guy Bar Yosef 
 *
 * fifo.c - Implements the fifo module defined in sem.h
 * 
 * version: 1.0
 */


#include <errno.h>
#include <sys/mman.h>


#include "sem.c"
#include "fifo.h"


/*
 * fifo_init - Initializes the shared memory FIFO.
 * 
 * NOTE: Call this function before any usage of the fifo.
 */
void fifo_init(Fifo *f) {  
    /* initialize binary semaphores (spinlock mechanism) */
    sem_init(&f->sem_reader, 1);
    sem_init(&f->sem_writer, 1);

    /* initialize the semaphore, according to an empty FIFO */
    sem_init(&f->sem_empty, 0);
    sem_init(&f->sem_full, MYFIFO_BUFSIZE);
    f->wr_index = 0;
    f->rd_index = 0;
}


/*
 * fifo_wr - Enqueues the data d into the FIFO, blocking
 * unless and until the FIFO has room to accept it.
 * 
 * All processes that had been blocked due to the empty FIFO 
 * get awoken.
 */
void fifo_wr(Fifo *f, unsigned long d) {
    
    sem_wait(&f->sem_full);
    
    sem_wait(&f->sem_writer); 
    /* begin CRITICAL REGION of semaphore spin-lock */
    f->queue[f->wr_index] = d;
    f->wr_index = (f->wr_index + 1) % MYFIFO_BUFSIZE;
    /* end of CRITICAL REGION of semaphore spin-lock */
    sem_inc(&f->sem_writer);

    sem_inc(&f->sem_empty);
}


/*
 * fifo_rd - Dequeues the next data word from the FIFO and
 * returns it.
 * 
 * Will block unless and until there is an available word
 * queued in the FIFO. All processes that had been blocked
 * due to the FIFO being full will get awoken.
 */
unsigned long fifo_rd(Fifo *f) {

    sem_wait(&f->sem_empty);
    
    sem_wait(&f->sem_reader); 
    /* begin CRITICAL REGION of semaphore spin-lock */
    unsigned long output = f->queue[f->rd_index];
    f->rd_index = (f->rd_index + 1) % MYFIFO_BUFSIZE;
    /* end of CRITICAL REGION of semaphore spin-lock */
    sem_inc(&f->sem_reader);

    sem_inc(&f->sem_full);
    
    return output;
}