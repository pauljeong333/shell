#include "jobs.h"


/*
* print_prompt() - prints the shell prompt only if the PROMPT macro is defined 
*/
void print_prompt();

/*
* print_prompt() - prints the shell prompt only if the PROMPT macro is defined 
*   and prints updates on background jobs
*/
void print_prompt_update(job_list_t *j_list);

/*
* Error checking for printf library calls
*/
void printf_err(int err);

/*
* Checks for errors in printf, and prints to stderr accordiningly
*/
void printprompt_err(int err);

/*
* syscall_err() - checks for errors for syscalls, calls perror accordingly
*/
void syscall_err(int err, char* msg);

/*
* parse_redir_err() - prints an error message given an error code from 
*   parse_redir, or returns without error if no error code is provided
*/
int parse_redir_err(int n_parse_err, job_list_t *j_list);

/*
* getpgrp_err() - error checking for getpgrp()
*/
int getpgrp_err(int err);

