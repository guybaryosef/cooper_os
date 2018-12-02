CC = gcc

# semaphore testing
test_sem: test_sem.c sem.h sem.c spinLockLib.c tas64.S
	$(CC) test_sem.c sem.c tas64.S -o test_sem

test_sem_debug: test_sem.c sem.h sem.c spinLockLib.c tas64.S
	$(CC) -g test_sem.c sem.c tas64.S -o debug_test_sem

# spinlock testing
test_spinlock: spinLockLib.c test_spinlock.c tas64.S
	$(CC) test_spinlock.c tas64.S -o test_spinlock

test_spinlock_debug: spinLockLib.c test_spinlock.c tas64.S
	$(CC) -g test_spinlock.c tas64.S -o debug_test_spinlock


clear:
	rm test_1 debug_1 test_sem debug_test_sem