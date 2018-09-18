/*
 * ECE 357: Computer Operating Systems 
 * Problem Set 1: Problem 3 
 * By: Guy Bar Yosef 
 *
 * minicat - concatenate and copy files
 * 
 * To run: 
 *  % gcc -w minicat.c -o minicat
 *  % ./minicat [-b ###] [-o outfile] infline1 [...infile2...]
 *  % ./minicat [-b ###] [-o outfile] 
 * 
 *  where
 *     - [-b ###] is an optional argument to specify the buffer size in bytes.
 *     - [-o outfile] is an optional argument to specify an output file.
 *     - if no input files are specified, will read from stdin. 
 *     - Similarly, one can specify stdin as the file '-' 
 */

#include <stdio.h>
#include <string.h> 
#include <stdlib.h>

#include <errno.h>
#include <fcntl.h>   
#include <unistd.h>

int main(int argc, char **argv) {
    
    int buf = 4096;           /* default buffer size in bytes */
    char *buffer = malloc(sizeof(char)*buf);

    char *output = STDOUT_FILENO;    /* output file to write to. Defaults to stdout */

    int i = 1; /* iterate through arguments to program */

    if (i < argc && strcmp(argv[i], "-b") == 0) {   /* checking for optional buffer size argument */
        char *tempPt;
        long temp;
        /* check for error in second part of buffer argument */
        if (++i == argc || (temp = strtol(argv[i], &tempPt, 10)) == 0 || errno == ERANGE)  {
            fprintf(stderr, "Invalid argument following optional '-b' flag in minicat program. "
                "The -b flag is used to specify the buffer size in bytes and requires an integer to follow it.\n");
            return -1;
        }
        else {
            buf = temp;
            buffer = realloc(buffer, buf);
        }
        ++i;
    }

    if (i < argc && strcmp(argv[i], "-o") == 0) { /* checking for optional output file argument */

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

        char *input;
        if (i==argc || ( open(argv[i], O_RDONLY) == -1 && strcmp(argv[i], "-") == 0 ) ) /* input will come from stdin */
            input = STDIN_FILENO;

        else {  /* reading from file */
            if ( (input = open(argv[i], O_RDONLY)) == -1) { 
                if (errno = ENOENT)   /* specified file does not exist */
                    fprintf(stderr, "The specified file name %s is not a valid file name.\n", argv[i]);
                else 
                    fprintf(stderr, "Unable to open %s for reading: %s\n", argv[i], strerror(errno));
                return -1;
            }
        }
        int j = 1;
        int k;
        while (j != 0) { /* while we have not reached the end-of-file */

            j = read(input, buffer, buf*sizeof(char)); /* reading from input */
            if (j < 0) { 
                fprintf(stderr, "An error occurred while trying to read from file %s: %s\n", input, strerror(errno));
                return -1;
            }

            k = 0;
            while (k != j) { /* accounts for partial writes */

                k += write(output, buffer + k, j);
                if ( k <= 0 && j != 0) {    /* error when writing to output file */
                    fprintf(stderr, "Unable to write buffer to output file %s: %s", output, strerror(errno));
                    return -1;
                }
            }
        }    
        
        if (close(input) == -1) {
            fprintf(stderr, "Unable to close output file %s in the shutdown of minicat program: %s\n", output, strerror(errno));
            return -1;
        } 
    } while (++i < argc);

    /* if output is a file (not stdout), close it */
    if (output != STDOUT_FILENO) {
        if (close(output) == -1) {
            fprintf(stderr, "Unable to close output file %s in the shutdown of minicat program: %s\n", output, strerror(errno));
            return -1;
        } 
    }

    return 0;
}