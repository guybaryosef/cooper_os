CC = gcc

# fifo testing
test_fifo: test_fifo.o fifo.o tas64.S
	$(CC) -o test_fifo test_fifo.o fifo.o tas64.S 

debug_test_fifo:
	$(CC) -g -o debug_test_fifo test_fifo.c fifo.c tas64.S 

# semaphore testing
test_sem: sem_test.o sem.o tas64.S
	$(CC) -o test_sem test_sem.o sem.o tas64.S 

debug_test_sem:
	$(CC) -g -o debug_test_sem test_sem.c sem.o tas64.S 

# spinlock testing
test_spinlock: test_spinlock.o tas64.S
	$(CC) -o test_spinlock test_spinlock.o tas64.S 

debug_test_spinlock:
	$(CC) -g -o debug_test_spinlock test_spinlock.c tas64.S 

# fifo
test_fifo.o: test_fifo.c
	$(CC) -c test_fifo.c

fifo.o: fifo.c fifo.h
	$(CC) -c fifo.c

# sem
sem_test.o: test_sem.c
	$(CC) -c test_sem.c

sem.o: sem.c sem.h
	$(CC) -c sem.c

# spinlock
test_spinlock.o: test_spinlock.c
	$(CC) -c test_spinlock.c

clear:
	rm test_spinlock debug_test_spinlock test_sem debug_test_sem test_fifo debug_test_fifo *.o