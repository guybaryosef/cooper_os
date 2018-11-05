/*
 * ECE 357: Computer Operating Systems 
 * Problem Set 3: Problem 2 
 * By: Guy Bar Yosef 
 *
 * simpleShell - Implements a simplistic UNIX shell 
 *    which is capable of launching one program at a
 *    time, with arguments, waiting for and reporting
 *    the exit status and resource usage statistics.
 * 
 * version 1
 * 
 * To run: 
 *    % gcc simpleShell.c -o simpleShell.exe
 *    % ./simpleShell.exe
 * 
 * In shell, command format is:
 *    % command {argument {argument ...} } {redirection_op {redirection_op ...} }
 * 
 */


#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <fcntl.h>


#define MAX_PATH_LEN 1024  /* max length of file path allowed */
#define MAX_LINE_SIZE 4096 /* max length of stdin allowed */
#define MAX_ARG_SIZE 2048  /* max amount of arguments in stdin line */


enum redirection {red_Open = 0, red_CreateTrun, red_CreateApp };


int myShell(FILE *input);
int tokenizeLine(char *line, char **tokens);
int parseLine(char ** tokens);
int performIOredirect(char **tokens);
int redirectIO(char **tokens, int size, int to_redirect, enum redirection val);


int main(int argc, char **argv) {
    if (argc > 2) {
        fprintf(stderr, "Correct Usage: /shell [script-file]\n");
        return -1;
    }
    else if (argc == 2) {
        FILE *input;
        if (!(input = fopen(argv[1], "r"))) {
            fprintf(stderr, "Unable to open script file %s: %s\n", argv[1], strerror(errno));
            return -1;
        }

        return myShell(input);
    }
    else 
        return myShell(stdin);
}


/*
 * Runs as a shell
 */
int myShell(FILE *input) {
    char *buffer;
    if (!(buffer = malloc(sizeof(char)*MAX_LINE_SIZE))) {
        fprintf(stderr, "Unable to allocate buffer in order to read from input: %s\n", strerror(errno));
        return -1;
    }

    int getline_ret;
    size_t n = MAX_LINE_SIZE;
    int exit_status = 0;
    while ( (getline_ret = getline(&buffer, &n, input)) != -1) {
        
        char **tokens; /* the current line parsed to words */
        if (!(tokens = malloc(sizeof(char *)*MAX_ARG_SIZE))) {
            fprintf(stderr, "Error allocating space for stdin line: %s\n", strerror(errno));
            return -1;
        }

        if (tokenizeLine(buffer, tokens) == -1) {
            return -1;
        }

        /* check if built-in exit command, otherwise parse line */
        if (!strcmp(tokens[0], "exit")) {
            if (!tokens[1])
                exit(exit_status);
            else
                exit(atoi(tokens[1]));
        }
        else
            exit_status = parseLine(tokens);

        free(tokens);
    }

    if (errno) {
        fprintf(stderr, "Error Reading in from stdin: %s\n", strerror(errno));
        return -1;
    }
    
    free(buffer);
    return exit_status;
}


/*
 * Takes as input a line of strings.
 * Parses the string with a space delimiter and returns the 
 * tokens as the pointers in tokens.
 * Returns:
 *   0 - success.
 *  -1 - Too many arguments in the stdin line.
 */
int tokenizeLine(char *line, char **tokens) {
    while (line[0] == ' ') /* skip beginning spaces */
        ++line;
    
    int index = 0;

    const char delim[2] = " ";
    char *token = strtok(line, delim);
    while (token && index < MAX_ARG_SIZE) {
        if (token[strlen(token)-1] == '\n') /* pop newline from end of string */
            token[strlen(token)-1] = 0;
        if (strcmp(token, "\n")) /* if no newline, add to token list */
            tokens[index] = token;
        
        ++index;
        token = strtok(NULL, delim);
    }

    if (index == MAX_ARG_SIZE) {
        fprintf(stderr, "Too many arguments in stdin line.\n");
        return -1;
    }
    tokens[index] = NULL;
    return 0;
}


/*
 * Given a tokenized line, this function parses it
 * according to the following specifications:
 *  - Begins with # - ignore line (it is considered a comment).
 *  - Begins with 'cd' - change directory to next token (built in).
 *  - Begins with 'pwd' - print current working directory (built in).
 *  - Begins with 'exit' - takes optional single argument and exits (built in).
 *  - Forks and creates a new process in which to run the commands.
 * 
 *  This function returns the error code of the last spawned child.
 */
int parseLine(char **tokens) {
    /* check for built-in commands and comments */
    if (tokens[0][0] == '#') /* ignore comments */
        return 0;
    else if (!strcmp(tokens[0], "cd")) { 
        if (!tokens[1] || chdir(tokens[1])) {
            fprintf(stderr, "Error changing directories: %s\n", strerror(errno));
            return -1;
        }
        return 0;
    }
    else if (!strcmp(tokens[0], "pwd")) {
        char *path;
        if (!(path = malloc(sizeof(char)*MAX_PATH_LEN))) {
            fprintf(stderr, "Error allocating memory for current path name: %s\n", strerror(errno));
            return -1;
        }

        if (!getcwd(path, MAX_PATH_LEN)) {
            fprintf(stderr, "Error finding current path name: %s\n", strerror(errno));
            return -1;
        }

        fprintf(stderr, "%s\n", path); /* no I/O redirection for built-in functions */
        free(path);
        return 0;
    }

    /* fork, run command, return time heuristics */
    int pid;
    int status;
    
    clock_t start, end;
    struct tms time_start, time_end;
    
    if ((start = times(&time_start)) == -1) {
        fprintf(stderr, "Error getting start time of command:%s\n", strerror(errno));
        return -1;
    }

    if ((pid = fork()) == -1) {
        fprintf(stderr, "Error during process forking: %s\n", strerror(errno));
        return -1;
    }
    else if (pid == 0) { /* in child */
        int command_size;
        if ((command_size = performIOredirect(tokens)) == -1)
            exit(-1);
        
        tokens[command_size] = NULL;
        
        fprintf(stderr, "Executing command: %s\n", tokens[0]);

        if (hvv) /* closing script file, if run as an interpreter */

        if (execvp(tokens[0], tokens) == -1)
            exit(-1);
    }
    else { /* in parent */
        if (wait(&status) == -1 || !WIFEXITED(status)) { /* wait for child */
            fprintf(stderr, "Child process %d exited in error: %s\n", pid, strerror(errno));
            return -1;
        }

        if ((end = times(&time_end)) == -1) {
            fprintf(stderr, "Error getting end time of command:%s\n", strerror(errno));
            return -1;
        }

        long clktck;
        if ((clktck = sysconf(_SC_CLK_TCK)) == -1) {
            fprintf(stderr, "Error acquiring clock ticks per second from sysconf:%s\n", strerror(errno));
            return -1;
        };

        /* print child statistics */
        fprintf(stderr, "Command returned with exit code: %d,\n", WEXITSTATUS(status));
        fprintf(stderr, "consuming %f real seconds, %f user, %f system.\n",
                    (end - start) / (double) clktck, 
                    (time_end.tms_cutime - time_start.tms_cutime) / (double) clktck,
                    (time_end.tms_cstime - time_start.tms_cstime) / (double) clktck);   
    }
    return status;
}


/*
 * Given a pointer to strings, interprets them as
 * a set of line commands, looking for I/O redirection
 * and redirecting the current process as necessary.
 * Returns: 
 *  - On success: positive value representing the amount of
 *      command/arguments before beginning i/o redirection.
 *  - On failur: -1
 */
int performIOredirect(char **tokens) {

    int beg_io_index = -1;
    int size = 0;
    for (char *cur = tokens[size] ; cur != NULL ; cur = tokens[++size]) {
        if (strncmp(cur, "2>", 2) == 0) { /* stderr redirection */
            if (beg_io_index  == -1)
                beg_io_index = size;

            if (fileno(stderr) != 2) {  /* check for multiple fd redirection */
                fprintf(stderr, "Cannot redirect a file more than once in a single command launch.\n");
                return -1; 
            }

            if (cur[2] == '>' && redirectIO(tokens, size, STDERR_FILENO, red_CreateApp) == -1)
                    return -1;
            else if (redirectIO(tokens, size, STDERR_FILENO, red_CreateTrun) == -1)
                    return -1;
        }

        else if (cur[0] == '>') { /* stdout redirection */
            if (beg_io_index  == -1)
                beg_io_index = size;
            
            if (fileno(stdout) != 1) { /* check for multiple fd redirection */
                fprintf(stderr, "Cannot redirect a file more than once in a single command launch.\n");
                return -1; 
            }

            if ( (cur[1] == '>' && redirectIO(tokens, size, STDOUT_FILENO, red_CreateApp) == -1)
                return -1;
            else if (redirectIO(tokens, size, STDOUT_FILENO, red_CreateTrun) == -1)
                return -1;
        }
        else if (cur[0] == '<') { /* stdin redirection */
            if (beg_io_index  == -1)
                beg_io_index = size;

            if (fileno(stdin) != 0) { /* check for multiple fd redirection */
                fprintf(stderr, "Cannot redirect a file more than once in a single command launch.\n");
                return -1;
            }

            if (redirectIO(tokens, size, STDIN_FILENO, red_Open) == -1)
                return -1;
        }
    }
    return (beg_io_index == -1) ? size : beg_io_index;
}


/*
 * Helper function to performIOredirection.
 * Given a file_name to redirect to_redirect into,
 * this function redirects it, with error checking, according
 * to the redirection enum parameter.
 */
int redirectIO(char **tokens, int size, int to_redirect, enum redirection val) {
    char *file_name = tokens[size+1];  /* filename */
    char *cur = tokens[size];
    /* get name of file we will redirect to */
    if (val == red_CreateApp && to_redirect == STDERR_FILENO) {
        if (strlen(cur) > 3)
            file_name = tokens[size]+3;
    }
    else if (val == red_CreateApp || (val == red_CreateTrun && to_redirect == STDERR_FILENO)) {
        if (strlen(cur) > 2)
            file_name = tokens[size]+2;
    }
    else if (strlen(cur) > 1)
            file_name = tokens[size]+1;

    if (!file_name || strstr(file_name, "<") || strstr(file_name, ">")) {
        fprintf(stderr, "Invalid I/O redirection command %s.\n", cur);
        return -1;
    }

    /* open the file 'to_redirect' will be redirected to */
    int new_file;
    if (val == red_Open) {
        if ((new_file = open(file_name, O_RDONLY, 0666)) == -1) {
            fprintf(stderr, "Error opening %s for I/O redirection: %s\n", file_name, strerror(errno));
            return -1;
        }
    }
    else if (val == red_CreateApp) {
        if ((new_file = open(file_name, O_WRONLY|O_CREAT|O_APPEND, 0666)) == -1) {
            fprintf(stderr, "Error opening %s for I/O redirection: %s\n", file_name, strerror(errno));
            return -1;
        }
    }
    else {
        if ((new_file = open(file_name, O_WRONLY|O_CREAT|O_TRUNC, 0666)) == -1) {
            fprintf(stderr, "Error opening %s for I/O redirection: %s\n", file_name, strerror(errno));
            return -1;
        }
    }

    if (dup2(new_file, to_redirect) < 0) { /* updating 'to_redirect' */
        fprintf(stderr, "Error redirecting I/O: %s\n", strerror(errno));
        return -1;
    }
    if (close(new_file) == -1) {
        fprintf(stderr, "Error closing extra reference to I/0 during redirection: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}