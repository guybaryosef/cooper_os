/*
 * ECE 357: Computer Operating Systems
 * Pset 5: Smear Program
 * By: Guy Bar Yosef
 * Version: 1
 * 
 * Smear - This program searches through the user-
 * designated files and replaced each user specified
 * target with a user specified replacement.
 * 
 * NOTE: 
 *   - Each file must be small enough to fit in its
 * entirety into the virtual address space.
 *   - The target and replacement must be of equal size.
 * 
 * Usage:
 *  $ gcc smear.c -o smear
 *  $ ./smear TARGET REPLACEMENT file1 {file2...}
 */


#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>


int find_and_replace(const char *file, const char *target, const char *replacement);


int main(int argc, char **argv) {

    /* confirm correct usage and get input from user */
    if (argc < 3) {
        fprintf(stderr, "Correct usage: smear TARGET REPLACEMENT file1 {file2...}\n");
        return -1;
    }
    else if (strlen(argv[1]) != strlen(argv[2])) {
        fprintf(stderr, "TARGET and REPLACEMENT must be equal in length.\n");
        return -1;
    }    
    char *target = argv[1];
    char *replacement = argv[2];

    /* iterate through each user-specified file */
    int ret;
    for (int i = 3 ; i < argc ; ++i) {
        if ((ret = find_and_replace(argv[i], target, replacement)) > -2)
            continue;
        else
            return -1;
    }
    return 0;
}


/*
 * find_and_replace - searches through 'file',
 * switching all occurrences of 'target' with 
 * 'replacement'. 
 * 
 * Returns:
 *      -  0 on success.
 *      - -1 on invalid file name.
 *      - -2 on error.
 */
int find_and_replace(const char *file, const char *target, const char *replacement) {
    int cur_file, cur_file_size;
    void *mapped_file;
    void *cur_target_loc, *beg_file_loc;
    struct stat cur_file_stat;
    
    /* open file */
    if ((cur_file = open(file, O_RDWR)) == -1) {
        fprintf(stderr, "Unable to open file %s for reading and " 
                                    "writing: %s\n", file, strerror(errno));
        return -1;
    }
    
    /* get the file's size */
    if ((fstat(cur_file, &cur_file_stat)) == -1) {
        fprintf(stderr, "Unable to find file %s's size: %s\n", file, strerror(errno));
        return -1;
    }
    
    /* map the file to the program's virtual memory */
    if ((mapped_file = mmap(NULL, cur_file_stat.st_size, PROT_READ|PROT_WRITE, 
                                            MAP_SHARED, cur_file, 0)) == MAP_FAILED) {
        fprintf(stderr, "Error mapping file %s into program: %s\n", 
                                            file, strerror(errno));
        return -1;
    }

    /* search-and-replace 'target' with 'replacement' */ 
    beg_file_loc = mapped_file;
    cur_file_size = cur_file_stat.st_size;
    while ((cur_target_loc = memmem(beg_file_loc, cur_file_size, target, strlen(target)))) {
        memcpy(cur_target_loc, replacement, strlen(replacement));
        /* absolute value accounts for decreasing or increasing memory */
        cur_file_size -= abs(beg_file_loc - cur_target_loc); 
        beg_file_loc = cur_target_loc + 1;
    }

    /* unmap and close file */
    if (munmap(mapped_file, cur_file_stat.st_size) == -1) {
        fprintf(stderr, "Error unmapping between file %s and "
                                    "the program: %s\n", file, strerror(errno));
        return -2;
    }
    if (close(cur_file) == -1) {
        fprintf(stderr, "Error closing file %s: %s\n", file, strerror(errno));
        return -2;
    }

    return 0;
}