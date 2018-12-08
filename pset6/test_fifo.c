/*
 * ECE 357: Computer Operating Systems 
 * Problem Set 6: Problem 4 
 * By: Guy Bar Yosef 
 *
 * test_fifo.c - Tests the fifo implementation.
 * 
 * version: 1.0
 */


#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>

#include "fifo.h"


int preliminary_test();
int acid_test(int num_writers);

void h(int signo) {}; /* signal handler for the Semaphore implementation */

int main() {

    /* semaphore implementation requires handling SIGUSR1 */  
    struct sigaction sa;
    sa.sa_handler = h;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGUSR1, &sa, NULL) == -1)
        perror("Error updating the handling of SIGUSR1!\n");

    printf("Preliminary test:\n");
    preliminary_test();
    
    printf("\nAcid test:\n");
    acid_test(N_PROC);

    return 0;
}


/*
 * preliminary_test - A preliminary test of the fifo implementation
 * with a single reader and a single writer.
 */
int preliminary_test() {

    void *shared_pointer;
    if ((shared_pointer = mmap(NULL, sizeof(Fifo), PROT_READ|PROT_WRITE,
                                    MAP_SHARED|MAP_ANONYMOUS, 0, 0)) < 0) {
        
        fprintf(stderr, "Error allocating shared memory space: %s\n", 
                                                            strerror(errno));
        return -1;
    }

    Fifo *our_fifo = (Fifo *)shared_pointer;
    fifo_init(our_fifo);

    pid_t p;
    if ((p = fork()) < 0) {
        fprintf(stderr, "Error forking preliminary test into "
                                "reader and writer: %s\n", strerror(errno));
        return -1;
    }
    else if (p == 0) { /* in writer */
        unsigned long i;
        for (i = 0 ; i < MYFIFO_BUFSIZE*8 ; ++i ) {
            fifo_wr(our_fifo, i);
        }
        exit(0);
    }
    else { /* in reader */
        
        unsigned long tmp;
        for (unsigned i = 0 ; i < MYFIFO_BUFSIZE*8 ; ++i)
            tmp = fifo_rd(our_fifo);

        if (tmp == MYFIFO_BUFSIZE*8-1) 
            fprintf(stderr, "All values were sent and received using the FIFO.\n");
        else
            fprintf(stderr, "NOT all values were sent and received using the FIFO.\n");
    }

    return 0;
}


/*
 * acit_test - A more comprehensive test of the fifo
 * implementation by having multiple writers, while 
 * still keeping a single reader. 
 */
int acid_test(int num_writers) {

    void *shared_pointer;
    if ((shared_pointer = mmap(NULL, sizeof(Fifo), PROT_READ|PROT_WRITE,
                                    MAP_SHARED|MAP_ANONYMOUS, 0, 0)) < 0) {
        
        fprintf(stderr, "Error allocating shared memory space: %s\n", 
                                                            strerror(errno));
        return -1;
    }

    Fifo *our_fifo = (Fifo *)shared_pointer;
    fifo_init(our_fifo);

    pid_t p;
    for (int i = 0 ; i < num_writers; ++i) {
        if ((p = fork()) < 0) {
            fprintf(stderr, "Error forking preliminary test into "
                                    "reader and writer: %s\n", strerror(errno));
            return -1;
        }
        else if (p == 0) {  /* in writers */

            unsigned long send_val = i << 16; /* unique writer id */
            for (int j = 0 ; j < MYFIFO_BUFSIZE*8 ; ++j, ++send_val)
                fifo_wr(our_fifo, send_val);

            exit(0);
        }
    }

    /* in reader */

    /* initialize checker array, which keeps track of incoming values */   
    unsigned long checker[num_writers];
    for (int i = 0 ; i < num_writers ; ++i)
        checker[i] = 0;

    unsigned long writer_id, cur_val, tmp;
    unsigned long WRITTEN_VALUE_MASK = pow(2,16)-1;
    for (unsigned long i = 0 ; i < num_writers*(MYFIFO_BUFSIZE*8) ; ++i) {
        
        /* read and parse next value from the fifo */
        tmp = fifo_rd(our_fifo);
        writer_id = tmp >> 16;
        cur_val = tmp & WRITTEN_VALUE_MASK;

        if (checker[writer_id]++ != cur_val)
            fprintf(stderr, "Fifo corruption occured.\n");

    }

    /* confirm all written values were received */
    int fifo_works = 1;
    for (int i = 0 ; i < num_writers ; ++i) {
        if (checker[i] != MYFIFO_BUFSIZE*8) {
            fifo_works = 0;
            break;
        }
    }

    if (fifo_works) 
        fprintf(stderr, "All values were sent and received using the FIFO.\n");
    else
        fprintf(stderr, "NOT all values were sent and received using the FIFO.\n");
        
    /* confirm that all processes exited */
    while (!wait(NULL))
        ;
    fprintf(stderr, "Parent exiting after all children exited.\n");

    return 0;
}