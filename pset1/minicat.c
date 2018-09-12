/*
 * ECE 357: Computer Operating Systems
 * 
 * Problem Set 1: Problem 3 
 * 
 * minicat - concatenate and copy files
 * 
 * By: Guy Bar Yosef 
 * 
 */

#include <stdio.h>
#include <string.h> 
#include <stdlib.h>

#include <errno.h>   /* Defines error numbers */
#include <fcntl.h>   

int main(int argc, char **argv) {

    int buf = 4096;           /* buffer size in bytes */
    char *buffer = malloc(sizeof(char)*buf);

    char *output = 1;    /* output file to write to. Defaults to stdout */

    // running through arguments
    for (int i = 1 ; i < argc ; ++i) {

        if (strcmp(argv[i], "-b") == 0) {   /* checking for optional buffer size argument */
            char *tempPt;
            long temp = strtol(argv[++i], &tempPt, 10);

            if ( temp==0 || errno == ERANGE ) {  /* detects error in strtol */
                fprintf(stderr, "Invalid argument following optional '-b' flag in minicat program. "
                    "The -b flag is used to specify the buffer size in bytes and requires an integer to follow it.\n");
                return -1;
            }
            else {
                buf = temp;
                buffer = realloc(buffer, buf);
            }
        }
        else if (strcmp(argv[i], "-o") == 0) { /* checking for optional output file argument */

            if ( (output = open(argv[++i], O_CREAT|O_WRONLY|O_TRUNC, 0666)) == -1 ) {
                fprintf(stderr, "Unable to open file %s for writing: %s\n", argv[i], strerror(errno) );
                return -1;
            }
        }
        else {    /* Input file to read from */
            char *input;
            if (strcmp(argv[i], "-") == 0) /* input will come from stdin */
                input = 0; 
            else {  /* reading from file */
                if ( (input = open(argv[i], O_RDONLY)) == -1) { /* Open file for reading */
                    if (errno = ENOENT)   /* specified file does not exist */
                        fprintf(stderr, "The specified file name %s is not a valid file name.\n", argv[i]);
                    else 
                        fprintf(stderr, "Unable to open %s for reading: %s\n", argv[i], strerror(errno));
                    return -1;
                }
            }
            int j = 1;
            while (j != 0) {

                j = read(input, buffer, sizeof(buffer)); /* reading from input */
                if (j < 0) { 
                    fprintf(stderr, "An error occurred while trying to read from file %s: %s\n", input, strerror(errno));
                    return -1;
                }

                int k = 0;
                while (k != j) { /* accounts for partial writes */

                    k += write(output, buffer + k, j);
                    if ( k <= 0 && j != 0) { /* error writing to output */
                        fprintf(stderr, "Unable to write buffer to output file %s: %s", output, strerror(errno));
                        return -1;
                    }
                }
            }    
            write(output, "\n", 1); /* End the output file on a newline, especially convinient for stdout */
            
            if (close(input) == -1) {
                fprintf(stderr, "Unable to close output file %s in the shutdown of minicat program: %s\n", output, strerror(errno));
                return -1;
            } 
        }
    }

    /* closing output file, if not stdout */
    if (output != 1) {
        if (close(output) == -1) {
            fprintf(stderr, "Unable to close output file %s in the shutdown of minicat program: %s\n", output, strerror(errno));
            return -1;
        } 
    }

    return 0;
}