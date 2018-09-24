/*
 * ECE 357: Computer Operating Systems 
 * Problem Set 2: Problem 2 
 * By: Guy Bar Yosef 
 *
 * lister - Provides a recursive directory listing
 * 
 * To run: 
 *    % gcc -w lister.c -o lister
 *    % ./lister dir
 *    % ./lister  
 *  where dir is the starting directory.
 *  If 'dir' is not specified, the program defaults to the current working directory.
 */
////////////////////////////////// TO DO: /////////////////////////////////////
///////////////// FINISH UP RELATIVE PATH IMPLEMENTATION && OPTIONAL SIMLINK STUFF ////////////


#define _GNU_SOURCE

#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int recursive_listing(DIR *cur, char *dir_name, FILE* pass, const char *path);
void parse_mode(int mode, char *output);
int parse_pass(FILE *pass, char *user, int uid, char *group, int gid);
void parse_time(long int *mtime, char *output);


int main(int argc, char ** argv) {

    char start[256];    /* default starting directory */
    strcpy(start, ".");

    /* confirm correct usage and determine starting directory */
     if (argc == 2) {
         strcpy(start, argv[1]);
    }
    else if (argc != 1) {
        fprintf(stderr, "Incorrect usage. Usage: ./lister [starting/dir]\n");
        return -1;
    }

    /* open specified starting directory */
    DIR *start_dir;
    if ( !(start_dir = opendir(start)) ) { 
        fprintf(stderr, "Can not open directory %s: %s\n", start, strerror(errno));
        return -1;
    }

    /* open passwd file needed to convert user and group numbers to IDs */
    FILE *pass;
    if ( !(pass = fopen("/etc/passwd", "r")) ) {
        fprintf(stderr, "Unable to read from linux password file: %s\n", strerror(errno) );
        return -1;
    }

    /* recursively print all the file info of each file */
    if (recursive_listing(start_dir, start, pass, start) == -1)
        return -1;
    
    /* close starting directory and password file */
    if (closedir(start_dir) == -1) {
        fprintf(stderr, "Error closing directory: %s: %s\n", start, strerror(errno) );
        return -1;
    }
    if (fclose(pass) == EOF) {
        fprintf(stderr, "Error closing password file: %s\n", strerror(errno) );
        return -1;
    }

    return 0;
}


/* 
 * Recursively iterates through every file and directory in the inputted current directory.
 * Output for each file is as follows, where (*) occurred and [*] is optional:
 *  (Inode #), (disk usage in 1K units), (inode type and mode string), (link count), (uid name), (gid name),
 *      (file size), mtime, (full path name), [string "->" and contents of the symlink].
 */
int recursive_listing(DIR *cur, char *dir_name, FILE* pass, const char *curPath) {
    struct dirent *de;
    struct stat st;

    char myPath[256]; /* path of the current file, de */
    while (de = readdir(cur)) {

        if (strcmp(de->d_name, "..") == 0) /* do not extend scope to parent directories (duh) */
            continue;

        strncpy(myPath, curPath, sizeof(myPath));

        if (strcmp(de->d_name, ".") != 0) {
            myPath[strlen(curPath)] = '/';
            myPath[strlen(curPath)+1] = 0;
            strncat(myPath, de->d_name, sizeof(myPath) - sizeof(curPath));
        }

        if (de->d_type == DT_DIR && strcmp(de->d_name, ".") != 0) {   /* If a directory, recursively call function for it */
            
            DIR *new_dir;
            if ( !(new_dir = opendir(myPath)) ) { 
                fprintf(stderr, "Can not open directory %s: %s\n", myPath, strerror(errno));
                return -1;
            }

            if (recursive_listing(new_dir, de->d_name, pass, myPath) == -1)
                return -1;

            closedir(new_dir);
        }
        else {  /* print out file info to stdout */
            if (lstat(myPath, &st) == -1) {
                fprintf(stderr, "Error getting file %s's status: %s\n", myPath, strerror(errno) );
                return -1;
            }

            char mode[11];     /* will hold the parsed file type and premissions */
            parse_mode(st.st_mode, mode);

            char user[255];    /* will hold the username */
            user[0] = 0;       /* used for error checker in parser */
            char group[255];   /* will hold the group name */
            group[0] = 0;      /* used for error checker in parser */
            if (parse_pass(pass, user, st.st_uid, group, st.st_gid) == -1)
                return -1;

            char time_val[13]; /* will hold human-readable mtime value */
            parse_time(&st.st_mtime, time_val);

            if (mode[0] == 'l') { /* if symbolic link, add its contents to printed info */
                char link[256];
                strcpy(link, "-> ");
                char linkName[256];
                int count = readlink(myPath, linkName, sizeof(linkName));
                strncat(link, linkName, count);

                printf("%9ld %6ld %s %3ld %-8s %-8s %8ld %s %s %s\n", st.st_ino, st.st_blocks/2, 
                        mode, st.st_nlink, user, group, st.st_size, time_val, myPath, link);
            }
            else
                printf("%9ld %6ld %s %3ld %-8s %-8s %8ld %s %s\n", st.st_ino, st.st_blocks/2, 
                        mode, st.st_nlink, user, group, st.st_size, time_val, myPath);
        }
    }
    if (errno) {
        fprintf(stderr, "Error reading directory: %s: %s\n", dir_name, strerror(errno) );
        return -1;
    }
}


/*
 * Parses the 'stat' function's mode value, converting it into premission notation
 */
void parse_mode(int mode, char *output) {
    
    /* find the file type */
    if (S_ISREG(mode))
        output[0] = '-';
    else if (S_ISDIR(mode))
        output[0] = 'd';
    else if (S_ISCHR(mode))
        output[0] = 'c';
    else if (S_ISBLK(mode))
        output[0] = 'b';
    else if (S_ISFIFO(mode))
        output[0] = 'p';
    else if (S_ISLNK(mode))
        output[0] = 'l';
    else if (S_ISSOCK(mode))
        output[0] = 's';
    else    /* unknown mode */
        output[0] = '?';
    
    /* find premissions for user, group, and other */
    output[1] = (mode & S_IRUSR) ? 'r' : '-';
    output[2] = (mode & S_IWUSR) ? 'w' : '-';
    output[3] = (mode & S_IXUSR) ? 'x' : '-';
    output[4] = (mode & S_IRGRP) ? 'r' : '-';
    output[5] = (mode & S_IWGRP) ? 'w' : '-';
    output[6] = (mode & S_IXGRP) ? 'x' : '-';
    output[7] = (mode & S_IROTH) ? 'r' : '-';
    output[8] = (mode & S_IWOTH) ? 'w' : '-';
    output[9] = (mode & S_IXGRP) ? 'x' : '-';
    output[10] = 0;
}


/*
 * Parse the password file for the relevant user and group names based on their IDs.
 * If the number->name translation is not found, return the number.
 */
int parse_pass(FILE *pass, char *user, int uid, char *group, int gid) {
    fseek(pass, 0, SEEK_SET);
    char buf[600];
    char *delim = ":";
    int user_found = 0;
    int group_found = 0;

    while ( fgets(buf, sizeof(buf), pass) != NULL && (!user_found || !group_found) ) {
        char *name = strtok(buf, delim);     /* current username */
        strtok(NULL, delim);                 /* hash of password */
        char *userID = strtok(NULL, delim);  /* user ID */
        char *groupID = strtok(NULL, delim); /* group ID */

        if (atoi(userID) == uid) {
            strcpy(user, name);
            user_found = 1;
        }

        if (atoi(groupID) == gid) {
            strcpy(group, name);
            group_found = 1;
        }
    }
    if (errno) {
        fprintf(stderr, "Unable to parse password file: %s\n", strerror(errno) );
        return -1;
    }

    /* if no number->name translation was found, return the number */
    if (user[0] == 0)
        sprintf(user, "%d", uid);
    if (group[0] == 0)
        sprintf(group, "%d", gid);

    return 0;
}


/*
 * Parses mtime into human-readable format that matches the 'find' commands formatting
 */
void parse_time(long int *mtime, char *output) {
    char *base = ctime(mtime);
    strncpy(output, base+4, 12);
}