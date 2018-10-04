/*
 * ECE 357: Computer Operating Systems 
 * Problem Set 1: Problem 3 
 * By: Guy Bar Yosef 
 *
 * minicat - concatenate and copy files
 * version: 2.0
 * 
 * To run: 
 *  % gcc minicat.c -o minicat
 *  % ./minicat [-b ###] [-o outfile] infline1 [...infile2...]
 *  % ./minicat [-b ###] [-o outfile] 
 * 
 *  where
 *     - [-b ###] is an optional argument to specify the buffer size in bytes.
 *     - [-o outfile] is an optional argument to specify an output file.
 *     - If no input files are specified, program will read from stdin.
 *     - If the filename ‘-’ is not taken then its usage will direct the program to read from stdin.
 */

#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>   
#include <unistd.h>

int main(int argc, char **argv) {
    
    int buf = 4096;           /* default buffer size in bytes */

    char *buffer;
    if (!(buffer = malloc(sizeof(char)*buf)) ) {
        fprintf(stderr, "Unable to allocate memory for default buffer size: %s\n", strerror(errno));
        return -1;
    }

    int output = STDOUT_FILENO;    /* output file to write to. Defaults to stdout */
    int i = 1; /* iterate through arguments to program */

    if (i < argc && !strcmp(argv[i], "-b")) {   /* checking for optional buffer size argument */
        char *tempPt;
        long temp;
        /* check for error in second part of buffer argument */
        if (++i == argc || !(temp = strtol(argv[i], &tempPt, 10)) || errno == ERANGE)  {
            fprintf(stderr, "Invalid argument following optional '-b' flag in minicat program. "
                "The -b flag is used to specify the buffer size in bytes and requires an integer to follow it.\n");
            return -1;
        }
        else {
            buf = temp;
            if (!(buffer = realloc(buffer, buf))) {
                fprintf(stderr, "Unable to allocate the amount of"
                                "memory requested for buffer: %s\n", strerror(errno) );
                return -1;
            }
        }
        ++i;
    }

    if (i < argc && !strcmp(argv[i], "-o")) { /* checking for optional output file argument */

        if (++i == argc ) {
            fprintf(stderr, "Invalid argument following optional '-o' flag in minicat program. "
                "The -o flag is used to specify an output file and requires the output file name to follow it.\n");
            return -1;
        }
        else if ( (output = open(argv[i], O_CREAT|O_WRONLY|O_TRUNC, 0666)) == -1 ) {
            fprintf(stderr, "Unable to open file %s for writing: %s\n", argv[i], strerror(errno) );
            return -1;
        }
        ++i;
    }
 
    do { /* iterate through inputted files */

        int input;
        if (i==argc || (open(argv[i], O_RDONLY) == -1 && !strcmp(argv[i], "-"))) /* input will come from stdin */
            input = STDIN_FILENO;

        else {  /* reading from file */
            if ( (input = open(argv[i], O_RDONLY)) == -1) { 
                fprintf(stderr, "Unable to open %s for reading: %s\n", argv[i], strerror(errno));
                return -1;
            }
        }
        int nwt, nrd, tmp; /*num of writing , num of read */
        while ( nrd = read(input, buffer, buf*sizeof(char)) ) { /* while not at end-of-file */

            if (nrd < 0) { /* confirm correct read */
                fprintf(stderr, "An error occurred while trying to"
                                "read from file descriptor %d: %s\n", input, strerror(errno));
                return -1;
            }

            nwt = 0;
            while (nwt < nrd) { /* accounts for partial writes */

                if ( (tmp = write(output, buffer + nwt, nrd - nwt)) <= 0 ) {  /* confirm correct write */
                    fprintf(stderr, "Unable to write buffer to output file %d: %s", output, strerror(errno));
                    return -1;
                }
                nwt += tmp;
            }
        }    
        
        if (input != STDERR_FILENO && close(input) == -1) {
            fprintf(stderr, "Unable to close output file descriptor %d in the"
                            "shutdown of minicat program: %s\n", output, strerror(errno));
            return -1;
        } 
    } while (++i < argc);

    /* if output is a file (not stdout), close it */
    if (output != STDOUT_FILENO && close(output) == -1) {
        fprintf(stderr, "Unable to close output file descriptor %d in the" 
                        "shutdown of minicat program: %s\n", output, strerror(errno));
        return -1;
    }

    return 0;
}