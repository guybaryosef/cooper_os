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