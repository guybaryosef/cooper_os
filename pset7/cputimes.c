/*
 * ECE:357 Operating Systems
 * Pset 7: Problem 5
 * By: Guy Bar Yosef
 *
 * q5 - Tests the scheduling of the operating system by initializing
 * several children, the first of which sets for itself a different
 * nice value (scheduling priority) than the rest. This function then
 * appends writes CPU usage statistics to a csv file.
 *
 * Usage:
 *    $ gcc cputimes.c -o cputimes
 *    $ ./cputimes [number_of_processes] ...
 *                  [nice value change of first process]...
 *                  [sleep length of parent]
 */



#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>


int main(int argc, char **argv) {

	if (argc != 4) {
		fprintf(stderr, "Usage: ./cputimes [number_of_processes] "
			"[nice value change of first process] [sleep length of parent]\n");
		return -1;
	}

	unsigned process_num = (unsigned int)atoi(argv[1]);
	int nice_change = atoi(argv[2]);
	unsigned sleep_length = (unsigned int) atoi(argv[3]);

	/* fork and initialize the specified number of children */
	pid_t pid;
	pid_t child[process_num];
	for (unsigned int i = 0 ; i < process_num ; ++i) {
		if ((pid=fork()) < 0) {
			fprintf(stderr, "Error during fork: %s\n", strerror(errno));
			return -1;
		}
		else if (pid == 0) { /* in child */

			if (i == 0) { /* change priority of first child */
				if (nice(nice_change) < 0) {
					fprintf(stderr, "Error changing first child's "
									"nice value: %s\n", strerror(errno));
					exit(-1);
				}
			}

			/* keep the children CPU-bound */
			unsigned w = 0;
			while (1)
				++w;

			exit(-1); /* we do not want the children to reach this */
		}
		else { /* in parent, keep track of children pids */
			child[i] = pid;
		}
	}

	sleep(sleep_length); /* have parent sleep after creating children */

	/* kill all children and collect rusage from them */
	struct rusage task0_usage, c_usage; /* usage statistics of child 0 and all children */
	int child_status = 0;
	for (int i = 0 ; i < process_num ; ++i) {

		if (kill(child[i], SIGKILL) < 0)
			perror("Error sending SIGKILL to children");
		
		wait(&child_status);
		if (child_status < 0) {
			fprintf(stderr, "Error during exit of child %d: %s\n", child[i], strerror(errno));
			return -1;
		}

		if (i==0) { /* get usage statistics of first child */
			if (getrusage(RUSAGE_CHILDREN, &task0_usage) < 0)
				perror("Error getting statistics from first child.\n");
		}
	}
	if (getrusage(RUSAGE_CHILDREN, &c_usage) < 0)
		perror("Error getting statistics from first child.\n");

	/* write usage statistics into statistics file */
	int fd;
	char *file_name = "q5_statistics.csv";
	if ((fd = open(file_name, O_WRONLY|O_APPEND, 0777)) < 0) {
		if (errno ==  ENOENT) {
			errno = 0;
			if ((fd = open(file_name, O_WRONLY|O_CREAT, 0777)) < 0)
				perror("Error creating statistics file.\n");
			else {
				char *title = "Process Count, Task 0 Nice Value, "
							  "Parent waiting(us), Total CPU Time, "
							  "Task 0 CPU time, Task 0 CPU Percentage\n";
				if (write(fd, title, strlen(title)) < 0)
					perror("Error writing the title of the statistics file.\n");
			}
		}
		else
			perror("Error opening statistics file.\n");
	}

	/* parse usage info and massage it into csv format */
	unsigned long total_u_cpu = pow(10,6)*c_usage.ru_utime.tv_sec + 
												c_usage.ru_utime.tv_usec;
	unsigned long total_s_cpu = pow(10,6)*c_usage.ru_stime.tv_sec + 
												c_usage.ru_stime.tv_usec;
	unsigned long total_cpu = total_u_cpu + total_s_cpu;

	unsigned long task0_u_cpu = pow(10,6)*task0_usage.ru_utime.tv_sec + 
												task0_usage.ru_utime.tv_usec;
	unsigned long task0_s_cpu = pow(10,6)*task0_usage.ru_stime.tv_sec + 
												task0_usage.ru_stime.tv_usec;
	unsigned long task0_cpu = task0_u_cpu + task0_s_cpu;

	char *info;
	if((info = malloc(300)) == NULL)
		perror("Error allocating memory for rusage writing.\n");

	int info_size = sprintf(info, "%s, %s, %s, %lu, %lu, %f%%\n", argv[1], 
		argv[2], argv[3], total_cpu, task0_cpu, (float)task0_cpu/(float)total_cpu);

	/* write usage statistics into file and close down program */
	if (write(fd, info, info_size) < 0)
		perror("Error writing the title of the statistics file.\n");

	if (close(fd) < 0)
		perror("Error closing statistics file.\n");

	return 0;
}
