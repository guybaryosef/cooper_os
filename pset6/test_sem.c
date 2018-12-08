/*
 * ECE 357: Computer Operating Systems 
 * Problem Set 6: Problem 2 
 * By: Guy Bar Yosef 
 *
 * test_sem.c - Tests the semaphore
 * implementation.
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

#include "sem.h"

int testing();

void h(int signo) {};


int main() {

    /* make program so that it doesn't terminate with SIGUSR1 */  
    struct sigaction sa;
    sa.sa_handler = h;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGUSR1, &sa, NULL) == -1)
        perror("Error updating handling of SIGUSR1!\n");


    if (testing() < 0)
        perror("Exited with error.\n");
    return 0;
}


int testing() {
    void *shared_region;
    if ((shared_region = mmap(NULL, sizeof(Semaphore), PROT_READ|PROT_WRITE, 
                            MAP_SHARED|MAP_ANONYMOUS, 0, 0)) < 0) {

        fprintf(stderr, "Erorr mapping shared region: %s\n", strerror(errno));
        return -1;
    }

    /* our resource semaphore */
    Semaphore *sem = (Semaphore *)shared_region;

    /* out_var will be shared among all child processes of this process */
    sem_init(sem, 0);

    /* I have 8 cores on my computer, so I will fork 8 times total. */
    int p;
    int amount = 1;
    int count = 40;
    for (int i = 0 ; i < count ; ++i) {
        if ((p = fork()) == 0) {
            /* each of the 8 children adds the value 1 to the 
               shared variable, our_var, 10000000 times  */
            if (i < count/2) { /*first 3 processes are consumers */
                printf("Resource number %d is now being used.\n", sem->count);
                sem_wait(sem);
            }
            else if (i < count){
                sem_inc(sem);
                printf("Resource number %d is available.\n", sem->count);
            }
            else {
                int sem_status = sem_try(sem);
                printf("Semaphore waiting if 0==%d.\n", sem_status);
            }
            printf("child %d exited.\n", i);
            exit(0);
        }
    }
    /* the parent process waits for all its children to return and then prints the shared val. */
    while (wait(NULL) > 0) 
        ;
    printf("Parent exiting after all children have returned.\n");
}
