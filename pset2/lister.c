/*
 * ECE 357: Computer Operating Systems 
 * Problem Set 2: Problem 3 
 * By: Guy Bar Yosef 
 *
 * lister - Provides a recursive directory listing 
 *          that copies the format of "find -ls"
 * version 1
 * 
 * To run: 
 *    % gcc lister.c -o lister
 *    % ./lister dir
 *    % ./lister  
 * 
 *  where dir is the starting directory.
 *  If 'dir' is not specified, the program defaults to the current working directory.
 * 
 */

#define MAX_PATH_LEN 256 /* maximum length of path allowed */
#define MAX_USRN_LEN 32
#define MAX_GRPN_LEN 32
#define _GNU_SOURCE

#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>



int recursive_listing(DIR *cur, char *dir_name, const char *path);
void parse_mode(const int mode, char *output);
void parse_pass(char *user, int uid, char *group, const int gid);
int parse_time(const long int *mtime, char *output);


int main(int argc, char ** argv) {

    char start[MAX_PATH_LEN];    
    strcpy(start, "."); /* default starting directory */

    /* confirm correct usage and determine starting directory */
     if (argc == 2)
         strncpy(start, argv[1], MAX_PATH_LEN);
    else if (argc != 1) {
        fprintf(stderr, "Incorrect usage. Usage: ./lister [starting/dir]\n");
        return -1;
    }

    /* open specified starting directory */
    DIR *start_dir;
    if ( !(start_dir = opendir(start)) ) { 
        fprintf(stderr, "Cannot open directory %s: %s\n", start, strerror(errno));
        return -1;
    }

    /* recursively print all the file info of each file */
    if (recursive_listing(start_dir, start, start) == -1)
        return -1;
    
    /* close starting directory and password file */
    if (closedir(start_dir) == -1) {
        fprintf(stderr, "Error closing directory: %s: %s\n", start, strerror(errno) );
        return -1;
    }

    return 0;
}


/* 
 * Recursively iterates through every file and directory in the inputted current directory.
 * Output for each file is as follows, where (*) occurred and [*] is optional:
 *  (Inode #), (disk usage in 1K units), (inode type and mode string), (link count), (uid name), (gid name),
 *      (file size), mtime, (full path name), [string "->" and contents of the symlink].
 * NOTE: We will not look at path sizes greater than 256 chars. 
 */
int recursive_listing(DIR *cur, char *dir_name, const char *curPath) {
    struct dirent *de;
    struct stat st;

    char myPath[MAX_PATH_LEN]; /* path of the current file */
    while (de = readdir(cur)) {

        if (strcmp(de->d_name, "..") == 0) /* do not extend scope to parent directories (duh) */
            continue;

        strcpy(myPath, curPath);

        int pathSizeLeft;
        if ( strcmp(de->d_name, ".") ) {

            myPath[strlen(curPath)] = '/';
            myPath[strlen(curPath)+1] = 0;

            if (( pathSizeLeft = MAX_PATH_LEN - strlen(de->d_name)) < 0) {
                fprintf(stderr, "Path is too long; I decline to look at paths longer than %d length.\n", MAX_PATH_LEN);
                return -1;
            }
            strncat(myPath, de->d_name, pathSizeLeft + 1);
        }

        /* if a directory, enter and list its contents */
        if (de->d_type == DT_DIR && strcmp(de->d_name, ".")) { 
            
            DIR *new_dir;
            if ( !(new_dir = opendir(myPath)) ) { 
                fprintf(stderr, "Can not open directory %s: %s\n", myPath, strerror(errno));
                return -1;
            }

            if (recursive_listing(new_dir, de->d_name, myPath) == -1)
                return -1;

            if (closedir(new_dir) == -1) {
                fprintf(stderr, "Unable to close directory: %s: %s\n", myPath, strerror(errno) );
                return -1;
            }
        }
        else {  /* organizing and printing out file info to stdout */
            if (lstat(myPath, &st) == -1) {
                fprintf(stderr, "Error getting file %s status: %s\n", myPath, strerror(errno) );
                return -1;
            }

            char mode[11];     /* will hold the parsed file type and premissions */
            parse_mode(st.st_mode, mode);

            char user[MAX_USRN_LEN];    /* will hold the username */
            char group[MAX_GRPN_LEN];   /* will hold the group name */
            parse_pass(user, st.st_uid, group, st.st_gid);

            char time_val[13]; /* will hold human-readable mtime value */
            if (parse_time(&st.st_mtime, time_val) == -1) 
                return -1;



            if (mode[0] == 'l') { /* if symbolic link, add its contents to printed info */
                char link[MAX_PATH_LEN];
                strcpy(link, "-> ");
                char linkName[MAX_PATH_LEN - 3];

                /* readlink reads a maximum of 'M_P_L - 4', and not '-3', due to strncat behavior */
                int count = readlink(myPath, linkName, MAX_PATH_LEN - 4); 
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
void parse_mode(const int mode, char *output) {
    
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
 * If the number->name translation is not found, save into place the number.
 * Return 0 on completion or -1 on error.
 */
void parse_pass(char *user, const int uid, char *group, const int gid) {

    struct passwd *usr;
    if ( (usr = getpwuid(uid)) != NULL ) {
        strncpy(user, usr->pw_name, MAX_USRN_LEN-1);
        user[MAX_USRN_LEN-1] = 0;
    }
    else
        snprintf(user, MAX_USRN_LEN-1, "%d", uid);

    struct group *grp;
    if ( (grp = getgrgid(gid)) != NULL) {
        strncpy(group, grp->gr_name, MAX_GRPN_LEN-1);
        user[MAX_GRPN_LEN-1] = 0;
    }
    else
        snprintf(group, MAX_GRPN_LEN-1, "%d", gid);
}


/*
 * Parses mtime into human-readable format that matches the 'find' command's formatting.
 * Note that normally the hours and minutes are shown, 
 * yet if the date is more than 6 months old (or in the future),
 *  then the year is shown instead of the time.
 */
int parse_time(const long int *mtime, char *output) {
    
    char *base = ctime(mtime);
    long sixMon = 6*30*24*60*60; /* 6 months in seconds */
    time_t curTime;
    if ( (curTime = time(NULL)) == -1 ) {
        fprintf(stderr, "Could not look up current time: %s\n", strerror(errno) );
        return -1;
    }

    if (curTime - sixMon > *mtime || *mtime > curTime) { /* mtime hold date older than 6 months or in future */
        strncpy(output, base+4, 7);
        output[7] = 0;
        strncat(output, base+19, 5);
    }
    else {
        strncpy(output, base+4, 12);
        output[12] = 0;
    }
    return 0;
}