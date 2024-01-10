#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "common.h"
#include "check_error.h"
#include "jobs.h"


/*
 * run_command() - runs any builtin commands inputted into shell, and returns
 *   error codes accordingly
 *
 * Inputs:
 *   command - char*[512], string to store the bin path for the shell command
 *   argv - char*[512], the argument array eventually used for execv()
 *   i_redir - char*[1024], string to store the filepath for < redirections
 *   on_redir - char*[1024], string to store the filepath for > redirections
 *   oa_redir - char*[1024], string to store the filepath for >> redirections
 *   j_list - job_list_t*, the pointer to the job list
 *   fbg_err - int, error flag for bg and fg parsing
 *   jid_num - int, the job id for the process to continue for bg/fg
 *   job_status - int*, job status of child process
 *   fg_pid - int*, pid of process fg is running on
 *
* Returns:
*   0 - if the command isn't a bultin and no error is raised
*   1 - if an error was raised, or a builtin command is used, shell continues
*   2 - if the exit command is used
*   3 - if fg job not found error
*/
int run_command(char command[512], char *argv[512], char i_redir[1024], 
                char on_redir[1024], char oa_redir[1024], job_list_t *j_list, 
                int fbg_err, int jid_num, int *job_status, int *fg_pid) {
    if (!argv[0]) {  //check for newline character (enter)
        return 1;
    }
    if (!strcmp(command, "exit")) {
        return 2;
    }
    if (!strcmp(command, "cd")) {
        if (argv[1] == NULL) {
            fprintf(stderr, "cd: syntax error\n");
            return 1;
        }
        syscall_err(chdir(argv[1]), "cd");
        return 1;
    }
    if (!strcmp(command, "ln")) {
        syscall_err(link(argv[1], argv[2]), "ln");
        return 1;
    }
    if (!strcmp(command, "rm")) {
        syscall_err(unlink(argv[1]), "rm");
        return 1; 
    }
    if (!strcmp(command, "jobs")) {
        jobs(j_list);
        return 1;
    }
    if (!strcmp(command, "fg")) {
        if (fbg_err == 1) {
            printf_err(printf("fg: syntax error\n"));
            return 1;
        }
        if (fbg_err == 2) {
            printf_err(printf("fg: job input does not begin with %%\n"));
            return 1;
        }
        if (run_fg(jid_num, j_list, job_status, fg_pid)) {
            return 3;
        }
        return 1;
    }
    if (!strcmp(command, "bg")) {
        if (fbg_err == 1) {
            printf_err(printf("bg: syntax error\n"));
            return 1;
        }
        if (fbg_err == 2) {
            printf_err(printf("bg: job input does not begin with %%\n"));
            return 1;
        } 
        run_bg(jid_num, j_list);
        return 1;
    }
    if (command == NULL && (i_redir != NULL || on_redir != NULL || 
    oa_redir != NULL)) {
        printf_err(printf("error: redirects with no command\n"));
        return 1;
    }
    return 0;
}

/*
* change_file_redir() - opens the right inputs and outputs according to which 
*   file redirections are used 
*
* Inputs:
*   i_redir - char*[1024], string to store the filepath for < redirections
*   on_redir - char*[1024], string to store the filepath for > redirections
*   oa_redir - char*[1024], string to store the filepath for >> redirections
*
* Returns:
*   1 if an error is detected, 0 otherwise
*/
int change_file_redir(char i_redir[1024], char on_redir[1024], char oa_redir[1024]) {
    if (i_redir[0] != '\0') {
        close(0);
        if (open(i_redir, O_RDONLY) == -1) {
            perror("open");
            return 1;
        }
    }
    if (on_redir[0] != '\0') {
        close(1);
        if (open(on_redir, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU) == -1) {
            perror("open");
            return 1;
        }
    }
    if (oa_redir[0] != '\0') {
        close(1);
        if (open(oa_redir, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU) == -1) {
            perror("open");
            return 1;
        }
    }
    return 0;
}

/*
* update_cur_job() - updates the jobs list and prints out a message accordingly 
*   given a status of a job in the foreground that has been stopped, terminated
*   by signal, or terminated naturally. If the job is a background job, then
*   it is added to the jobs list.
*
* Inputs:
*   pid - int, pid of job that is being updated
*   job_status - int, status of job being updated
*   is_bg_proc - int, 1 if bg process, 0 if not
*   j_list - job_list_t*, pointer to job list
*   j_count - int*, points to jid of new job to be added to job list
*   command - char*[512], string to store the bin path for the shell command
*/
void update_cur_job(int pid, int job_status, int is_bg_proc, job_list_t *j_list, 
                    int *j_count, char *command) {
    int fg_jid;
    if (is_bg_proc) {
        add_job(j_list, *j_count, pid, RUNNING, command);
        printf_err(printf("[%i] (%i)\n", *j_count, pid));
        (*j_count)++;
        return;
    }
    if (WIFSTOPPED(job_status)) {
        if ((fg_jid = get_job_jid(j_list, pid)) == -1) {
            add_job(j_list, *j_count, pid, STOPPED, command);
            printf_err(printf("[%i] (%i) suspended by signal %i\n", *j_count, pid, 
                   WSTOPSIG(job_status)));
            (*j_count)++;
            return;
        }
        // when fg is called
        update_job_jid(j_list, fg_jid, STOPPED);
        printf_err(printf("[%i] (%i) suspended by signal %i\n", fg_jid, pid, 
                   WSTOPSIG(job_status)));
        return;
    }
    if (WIFSIGNALED(job_status)) {
        if ((fg_jid = get_job_jid(j_list, pid)) == -1) {
            printf_err(printf("[%i] (%i) terminated by signal %i\n", *j_count, pid, 
                   WTERMSIG(job_status)));
            remove_job_pid(j_list, pid);
            (*j_count)++;
            return;
        }
        // when fg is called
        printf_err(printf("[%i] (%i) terminated by signal %i\n", fg_jid, pid, 
                   WTERMSIG(job_status)));
        remove_job_pid(j_list, pid);
        return;
    }
    if (WIFEXITED(job_status)) {
        remove_job_pid(j_list, pid);
        return;
    }
}   

/*
* update_bg_jobs() - updates the jobs list and prints out a message accordingly 
*   given a status of a job that is run in the background (and that has 
*   been stopped, resumed, terminated normally, or terminated by signal)
*
* Inputs:
*   j_list - job_list_t*, pointer to job list
*/
void update_bg_jobs(job_list_t *j_list) {
    pid_t wpid;
    int job_status;

    while ((wpid = waitpid(-1, &job_status, WNOHANG|WUNTRACED|WCONTINUED)) > 0) {
        int jid = get_job_jid(j_list, wpid);
        if (WIFSTOPPED(job_status)) {
            update_job_pid(j_list, wpid, STOPPED); 
            printf_err(printf("[%i] (%i) suspended by signal %i\n", jid, wpid, 
                            WSTOPSIG(job_status)));
            continue;
        }
        if (WIFSIGNALED(job_status)) {
            printf_err(printf("[%i] (%i) terminated by signal %i\n", jid, wpid, 
                            WTERMSIG(job_status)));
            remove_job_pid(j_list, wpid); 
            continue;
        }
        if (WIFEXITED(job_status)) {
            printf_err(printf("[%i] (%i) terminated with exit status %i\n", jid, 
                wpid, WEXITSTATUS(job_status)));
            remove_job_pid(j_list, wpid);
            continue;
        } 
        if (WIFCONTINUED(job_status)) {
            printf_err(printf("[%i] (%i) resumed\n", jid, wpid));
            update_job_pid(j_list, wpid, RUNNING);
            continue;
        }
    }
}

/*
* run_fg() - continues a stopped job given a jid and runs it in the foreground
*
* Inputs:
*   jid - int, jid of job being continued
*   j_list - job_list_t*, pointer to job list
*   job_status - int*, pointer to the job status that is updated after waitpid
*   fg_pid - int*, pointer to the pid of the fg process
* 
* Returns: 
*   1 for job not found err, 0 otherwise
*/
int run_fg(int jid, job_list_t *j_list, int *job_status, int *fg_pid) {
    int pid = get_job_pid(j_list, jid);
    if (pid == -1) {
        printf_err(printf("fg: job not found\n"));
        return 1;
    }
    syscall_err(kill((-pid), SIGCONT), "kill");
    syscall_err(tcsetpgrp(0, pid), "tcsetpgrp");
    waitpid(pid, job_status, WUNTRACED); 
    (*fg_pid) = pid;
    return 0;
}

/*
* run_bg() - continues a stopped job given a jid and runs it in the background
*
* Inputs:
*   jid - int, jid of job being continued
*   j_list - job_list_t*, pointer to job list
*/
void run_bg(int jid, job_list_t *j_list) {
    int pid = get_job_pid(j_list, jid);
    if (pid == -1) {
        printf_err(printf("bg: job not found\n"));
        return;
    }
    syscall_err(kill((-pid), SIGCONT), "kill");
    update_job_pid(j_list, pid, RUNNING);
    return;
}