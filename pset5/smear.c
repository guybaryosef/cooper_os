/*
 * ECE 357: Computer Operating Systems
 * Pset 6: Smear Program
 * By: Guy Bar Yosef
 * Version: 1
 * 
 * Smear - This program searches through the user-
 * designated files and replaced each user specified
 * target with a user specified replacement.
 * 
 * NOTE: 
 *   - Each file must be small enough to fit in its
 * entierty into the virtual address space.
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
    int cur_file, cur_file_size;
    void *mapped_file;
    void *cur_target_loc, *beg_file_loc;
    struct stat cur_file_stat;
    for (int i = 3 ; i < argc ; ++i) {
        /* open file */
        if ((cur_file = open(argv[i], O_RDWR)) == -1) {
            fprintf(stderr, "Unable to open file %s for reading and " 
                                        "writing: %s\n", argv[i], strerror(errno));
            continue;
        }
        
        /* get the file's size */
        if ((fstat(cur_file, &cur_file_stat)) == -1) {
            fprintf(stderr, "Unable to find file %s's size: %s\n", argv[i], strerror(errno));
            return -1;
        }
        
        /* map the file to the program's memory */
        if ((mapped_file = mmap(NULL, cur_file_stat.st_size, PROT_READ|PROT_WRITE, 
                                                MAP_SHARED, cur_file, 0)) == MAP_FAILED) {
            fprintf(stderr, "Error mapping file %s into program: %s\n", 
                                                argv[i], strerror(errno));
            continue;
        }

        /* iterate through file, search-and-replacing 'target' with 'replacement' */ 
        beg_file_loc = mapped_file;
        cur_file_size = cur_file_stat.st_size;
        while ((cur_target_loc = memmem(beg_file_loc, cur_file_size, target, strlen(target)))) {
            memcpy(cur_target_loc, replacement, strlen(replacement));
            cur_file_size -= abs(beg_file_loc - cur_target_loc);
            beg_file_loc = cur_target_loc + 1;
        }

        /* unmap and close current file */
        if (munmap(mapped_file, cur_file_stat.st_size) == -1) {
            fprintf(stderr, "Error unmapping between file %s and "
                                        "the program: %s\n", argv[i], strerror(errno));
            return -1;
        }
        if (close(cur_file) == -1) {
            fprintf(stderr, "Error closing file %s: %s\n", argv[i], strerror(errno));
            return -1;
        }
    }

    return 0;
}