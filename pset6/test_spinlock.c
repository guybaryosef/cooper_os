/*
 * ECE 357: Computer Operating Systems 
 * Problem Set 6: Problem 1 
 * By: Guy Bar Yosef 
 *
 * test_spinlock - A sanity check to confirm
 * that the spin lock library works as intended.
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

#include "spinLockLib.c" /* gives spinlock mutex funcitonality */


void syncronized();
void unsyncronized();


int main() {
    printf("Version without a spinlock: \n");
    unsyncronized();

    printf("\nVersion with a spinlock: \n");
    syncronized();
}


/*
 * unsyncronized - Gives an example of how a multiple
 * user process program has syncronization issues.
 */
void unsyncronized() {
    void *shared_region;
    if ((shared_region = mmap(NULL, 100, PROT_READ|PROT_WRITE, 
                            MAP_SHARED|MAP_ANONYMOUS, 0, 0)) < 0) {

        fprintf(stderr, "Erorr mapping shared region: %s\n", strerror(errno));
        return;
    }

    /* our_val will be shared among all child processes of this process */
    unsigned long long *our_val = (unsigned long long *)shared_region;
    *our_val = 0;

    /* I have 8 cores on my computer, so I will fork 8 times total. */
    int p;
    int amount = 1;
    for (int i = 0 ; i < 8 ; ++i) {
        if ((p = fork()) == 0) {
            /* each of the 8 children adds the value 1 to the 
               shared variable, our_val, 10000000 times  */
            for (int j=0; j < 10000000; ++j) {
                *our_val += amount;
            }
            exit(0);
        }
    }

    /* the parent process waits for al lthe children to return and then prints our_val. */
    {
        while (wait(NULL) > 0) 
            ;

        printf("our_val should be: 10000000*8= 80000000.\nIt is: %lld\n", *our_val);
    }
}



/*
 * syncronized - Gives an example of how a multiple
 * user process program avoids syncronization issues
 * using a spinlock mutex.
 */
void syncronized() {
    void *shared_region;
    if ((shared_region = mmap(NULL, 100, PROT_READ|PROT_WRITE, 
                            MAP_SHARED|MAP_ANONYMOUS, 0, 0)) < 0) {

        fprintf(stderr, "Erorr mapping shared region: %s\n", strerror(errno));
        return;
    }

    /* local struct of our variable and its associated spinlock variable */
    typedef struct our_struct {
        char lock;
        unsigned long long val;
    } our_struct;

    /* out_var will be shared among all child processes of this process */
    our_struct *our_var = (our_struct *)shared_region;
    our_var->val = 0;
    our_var->lock = 0; /* spinlock begins unlocked */
    
    /* I have 8 cores on my computer, so I will fork 8 times total. */
    int p;
    int amount = 1;
    for (int i = 0 ; i < 8 ; ++i) {
        if ((p = fork()) == 0) {
            /* each of the 8 children adds the value 1 to the 
               shared variable, our_val, 10000000 times  */
            for (int j=0; j < 10000000; ++j) {
                spin_lock(&our_var->lock);
                our_var->val += amount; /* this line is our CRITICAL REGION */
                spin_unlock(&our_var->lock);
            }
            exit(0);
        }
    }

    /* the parent process waits for all its children to return and then prints the shared val. */
    while (wait(NULL) > 0) 
        ;

    printf("our_val should be: 10000000*8= 80000000.\nIt is: %lld\n", our_var->val);
}