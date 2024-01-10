#include "jobs.h"


/*
* change_file_redir() - opens the right inputs and outputs according to which 
*   file redirections are used 
*/
int change_file_redir(char i_redir[1024], char on_redir[1024], char oa_redir[1024]);

/*
 * run_command() - runs any builtin commands inputted into shell, and returns
 *   error codes accordingly
 */
int run_command(char command[512], char *argv[512], char i_redir[1024], 
char on_redir[1024], char oa_redir[1024], job_list_t *j_list, int fbg_err, int jid_num, int *job_status, int *fg_pid);

/*
* update_cur_job() - updates the jobs list and prints out a message accordingly 
*   given a status of a job in the foreground that has been stopped, terminated
*   by signal, or terminated naturally. If the job is a background job, then
*   it is added to the jobs list.
*/
void update_cur_job(int pid, int job_status, int fg, job_list_t *j_list, int *j_count, char *command);

/*
* update_bg_jobs() - updates the jobs list and prints out a message accordingly 
*   given a status of a job that is run in the background (and that has 
*   been stopped, resumed, terminated normally, or terminated by signal)
*/
void update_bg_jobs(job_list_t *j_list);

/*
* run_fg() - continues a stopped job given a jid and runs it in the foreground
*/
int run_fg(int jid, job_list_t *j_list, int *job_status, int *fg_pid);

/*
* run_bg() - continues a stopped job given a jid and runs it in the background
*/
void run_bg(int jid, job_list_t *j_list);