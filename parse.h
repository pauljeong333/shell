 /* 
 * Description: creates the token and argv arrays from the buffer character
 *   array
 */
int parse(char buffer[1024], char *tokens[512], char *argv[512], int *is_bg_proc, int *num_argv);

/*
* parse_redir() - parses tokens for redirections and their respective filepaths 
*   as well as for the bin path of the shell command
*/
int parse_redir(int num_tokens, char *tokens[512], char i_redir[1024], 
char on_redir[1024], char oa_redir[1024], char command[512]);

/*
* redir_equal_redir() - checks if the redirection filepath is another 
*   redirection symbol
*/
int redir_equal_redir(char redir[1024]);

/*
* parse_fg_bg() - parses the argv for correct syntax of fg and bg commands
*/
int parse_fg_bg(char *argv[512], int num_tokens, int *jid_num);