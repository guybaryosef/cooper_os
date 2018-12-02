/*
 * ECE 357: Computer Operating Systems 
 * Problem Set 6: Problem 1 
 * By: Guy Bar Yosef 
 *
 * spinlockLib - Implementation of a spin lock library
 * that consists of two functions, spin_lock and 
 * spin_unlock, using an atomic test & set (TAS).
 * 
 * version: 1.0
 */


#include <sched.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>


/* from the assembly Test & Set (TAS) file */
extern int tas(volatile char *lock);


/*
 * spin_lock - Uses a test & set (TAS) to
 * lock the specified struct.
 */
int spin_lock(char *lock) {
    while (tas(lock)) {
        if (sched_yield() < 0) {
            fprintf(stderr, "Error yielding cpu to another program: %s\n", strerror(errno));
            return -1;
        }
    }
    return 1;
}


/*
 * spin_unlock - Uses a test & set (TAS)
 * to unlock the specified struct.
 */
void spin_unlock(char *lock) {
    *lock = 0;
}