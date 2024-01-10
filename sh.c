#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

#include "common.h"
#include "parse.h"
#include "check_error.h"
#include "jobs.h"

/*
* Main REPL for the shell:
*   supports input and output redirection
*   bin commands (must use in the bin path, e.g /bin/ls)
*   6 built in commands (cd, rm, ln, exit, fg, bg)
*   background and foreground process handling
*   signal handling
*   to exit, type 'exit', CTRL-\
*
* Returns:
*   0 if exits successfully, 1 otherwise
*/
int main() {
    int ntokens = 512;
    char buffer[1024];
    char *tokens[ntokens];
    char *argv[ntokens];
    job_list_t *j_list = init_job_list();
    ssize_t line;
    int j_count = 1;
    int jid_num;

    // ignore signals
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    print_prompt();
    while ((line = read(0, buffer, 1024)) != 0) {
        char i_redir[1024];
        char on_redir[1024];
        char oa_redir[1024];
        char command[512];
        memset(i_redir, '\0', 1024);
        memset(on_redir, '\0', 1024);
        memset(oa_redir, '\0', 1024);
        memset(command, '\0', 512);
        int fbg_err;
        int is_bg_proc = 0;
        int job_status;
        pid_t child_pid;
        int num_argv;
        int fg_pid;

        if (line == -1) { perror("read"); }
        buffer[line] = 0; //add null at end of buffer
        int num_tokens = parse(buffer, tokens, argv, &is_bg_proc, &num_argv);
        int n_parse_err = parse_redir(num_tokens, tokens, i_redir, on_redir, 
        oa_redir, command); // parses redirections
        if (parse_redir_err(n_parse_err, j_list)) { continue; }
        fbg_err = parse_fg_bg(argv, num_argv, &jid_num); // parse for fg/bg

        //shell commands
        int comm_err = run_command(command, argv, i_redir, on_redir, oa_redir, 
            j_list, fbg_err, jid_num, &job_status, &fg_pid);
        // general error
        if (comm_err == 1) {
            syscall_err(tcsetpgrp(0, getpgrp_err(getpgrp())), "tcsetpgrp");
            if (!strcmp(command, "fg")) {
                update_cur_job(fg_pid, job_status, is_bg_proc, j_list, 
                &j_count, command);
            }
            print_prompt_update(j_list);
            continue;
        }
        // if "exit"
        if (comm_err == 2) {
            break;
        }
        // fg job not found error
        if (comm_err == 3) {
            print_prompt_update(j_list);
            continue;
        }
        if ((child_pid = fork()) == 0) {
            syscall_err(setpgid(0, 0), "setpgid");  // set pgid to pid 
            if (!is_bg_proc) {
                syscall_err(tcsetpgrp(0, getpgrp_err(getpgrp())), "tcsetpgrp");  // terminal control to only foreground process
            }
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            if (change_file_redir(i_redir, on_redir, oa_redir)) { 
                exit(1);
            }
            execv(command, argv);
            perror("execv");
            exit(1);
        }
        if (is_bg_proc) {
            update_cur_job(child_pid, job_status, is_bg_proc, j_list, 
                &j_count, command);
            syscall_err(tcsetpgrp(0, getpgrp_err(getpgrp())), "tcsetpgrp");  // terminal control to only foreground process
            print_prompt_update(j_list);
            continue;
        } 
        child_pid = waitpid(child_pid, &job_status, WUNTRACED);
        update_cur_job(child_pid, job_status, is_bg_proc, j_list, 
            &j_count, command);
        syscall_err(tcsetpgrp(0, getpgrp_err(getpgrp())), "tcsetpgrp");  // terminal control to only foreground process
        print_prompt_update(j_list);
    }
    cleanup_job_list(j_list);
    return 0;
}
