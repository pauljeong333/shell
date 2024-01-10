#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include "check_error.h"
#include "jobs.h"
#include "common.h"


/*
* print_prompt() - prints the shell prompt only if the PROMPT macro is defined
*/
void print_prompt() {
    #ifdef PROMPT
    printprompt_err(printf("33sh> "));
    #endif
}

/*
* print_prompt_update() - prints the shell prompt only if the PROMPT macro is defined
*/
void print_prompt_update(job_list_t *j_list) {
    update_bg_jobs(j_list);
    #ifdef PROMPT
    printprompt_err(printf("33sh> "));
    #endif
}

/*
* printprompt_err() - checks for errors in printf, and prints to stderr 
*   accordiningly and subsequently flushes the output stream
*
* Input:
*   err - int, the return value of printf
*/
void printprompt_err(int err) {
    if (err < 0) { 
        fprintf(stderr, "Write to stdout failed");
    } else if (fflush(0) < 0) {
        fprintf(stderr, "Flush failed");
    } 
}

/*
* printf_err() - checks for errors in printf, and prints to stderr accordiningly
*
* Input:
*   err - int, the return value of printf
*/
void printf_err(int err) {
    if (err < 0) { 
        fprintf(stderr, "Write to stdout failed");
    }
}

/*
* syscall_err() - checks for errors for syscalls, calls perror accordingly
*
* Input:
*   err - int, the return value of a syscall
*/
void syscall_err(int err, char* msg) {
    if (err) {
        perror(msg);
    }   
}

/*
* parse_redir_err() - prints an error message given an error code from 
*   parse_redir, or returns without error if no error code is provided
*
* Input:
*   n_parse_err - int, the error number returned by parse_redir()
*   j_list - job_list_t*, the pointer to the job list
*
* Returns:
*   0, if no error, 1 if parse gave an error
*/
int parse_redir_err(int n_parse_err, job_list_t *j_list) {
    switch(n_parse_err) {
        case 2:
            printf_err(fprintf(stderr, "syntax error: multiple input files\n"));
            print_prompt(j_list);
            return 1;
        case 3:
            printf_err(fprintf(stderr, "syntax error: multiple output files\n"));
            print_prompt(j_list);
            return 1;
        case 4: 
            printf_err(fprintf(stderr, "syntax error: no input file\n"));
            print_prompt(j_list);
            return 1;
        case 5:
            printf_err(fprintf(stderr, "syntax error: no output file\n"));
            print_prompt(j_list);
            return 1;
        case 6:
            printf_err(fprintf(stderr, "error: redirects with no command\n"));
            print_prompt(j_list);
            return 1;
        case 7:
            printf_err(fprintf(stderr, "syntax error: input file is a redirection symbol\n"));
            print_prompt(j_list);
            return 1;
        case 8: 
            printf_err(fprintf(stderr, "syntax error: output file is a redirection symbol\n"));
            print_prompt(j_list);
            return 1;
        default:
            return 0;
        }
}

/*
* getpgrp_err() - error checking for getpgrp()
*
* Input:
*   err - int, error value for getpgrp()
*
* Returns:
*   1 if error is code -1 is present, 0 otherwise
*/
int getpgrp_err(int err) {
    if (err == -1) {
        return 1;
    }
    return err;
}
