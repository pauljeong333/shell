#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include "parse.h"

/*
 * parse()
 *
 * - Description: creates the token and argv arrays from the buffer character
 *   array
 *
 * - Arguments: 
 *      buffer: a char array representing user input
 *      tokens: the tokenized input
 *      argv: the argument array eventually used for execv()
 *      is_bg_proc: *int, 1 if & is last character of input, 0 otherwise
 *      num_argv - *int, number of arguments in argv
 *
 * Returns:
 *  int, number of tokens
 */
int parse(char buffer[1024], char *tokens[512], char *argv[512], 
          int *is_bg_proc, int *num_argv) {
    char *token;
    int t_index = 0;
    int a_index = 0;
    int flag = 0;

    while ((token = strtok(buffer, " \t\n")) != NULL) {
        if (t_index == 0 && a_index == 0) {
            buffer = NULL;
        }
        if (strcmp(token, "<") == 0 || strcmp(token, ">") == 0 || 
        strcmp(token, ">>") == 0) {
            tokens[t_index] = token;
            t_index++;
            flag = 1;
        } else if (flag) {
            tokens[t_index] = token;
            t_index++;
            flag = 0;
        } else {
            char* bin_first = strchr(token, '/');
            char* bin_last = strrchr(token, '/'); 
            argv[a_index] = ((bin_first != NULL || bin_last != NULL) && 
            a_index == 0) ? (bin_last + 1) : token;
            tokens[t_index] = token;
            t_index++;
            a_index++;
        }
    }
    if (a_index > 0 && strcmp(argv[a_index - 1], "&") == 0) {
        (*is_bg_proc) = 1;
        (*num_argv) = a_index - 1;
        argv[a_index - 1] = NULL;
        return t_index;
    }
    argv[a_index] = NULL;
    (*num_argv) = a_index;
    return t_index;
}

/*
* parse_redir() - parses tokens for redirections and their respective filepaths 
*   as well as for the bin path of the shell command
*
* Input:
*   num_tokens - int, length of tokens
*   tokens - char*[512], array of the tokens
*   i_redir - char*[1024], string to store the filepath for < redirections
*   on_redir - char*[1024], string to store the filepath for > redirections
*   oa_redir - char*[1024], string to store the filepath for >> redirections
*   command - char*[512], string to store the bin path for the shell command
*
* Returns:
*   int, where
*       0 - parses with no error
*       2 - multiple input redirects
*       3 - multiple output redirects
*       4 - no input file
*       5 - no output file
*       6 - redirection but no command given
*       7 - input is redirection symbol
*       8 - output is redirection symbol
*/
int parse_redir(int num_tokens, char *tokens[512], char i_redir[1024], 
char on_redir[1024], char oa_redir[1024], char command[512]) {
    int i_filled = 0;
    int o_filled = 0;
    int com_filled = 0;

    for (int i = 0; i < num_tokens; i++) {
        if (strcmp(tokens[i], "<") == 0) {
            if (i + 1 < num_tokens && !i_filled) {
                strcpy(i_redir, tokens[i+1]);
                i_filled = 1;
                i++;
                continue;
            } else if (i_filled) {
                return 2;
            } else if (i + 1 >= num_tokens) {
                return 4;
            } 
        } else if (strcmp(tokens[i], ">") == 0) {
            if (i + 1 < num_tokens && !o_filled) {
                strcpy(on_redir, tokens[i+1]);
                o_filled = 1;
                i++;
                continue;
            } else if (o_filled) {
                return 3;
            } else if (i + 1 >= num_tokens) {
                return 5;
            } 
        } else if (strcmp(tokens[i], ">>") == 0) {
            if (i + 1 < num_tokens && !o_filled) {
                strcpy(oa_redir, tokens[i+1]);
                o_filled = 1;
                i++;
                continue;
            } else if (o_filled) {
                return 3;
            } else if (i + 1 >= num_tokens) {
                return 5;
            } 
        } else {
            if (!com_filled) {
                strcpy(command, tokens[i]);
                com_filled = 1;
            }
        }
    }
    if (!com_filled && (i_filled || o_filled)) {
        if (redir_equal_redir(i_redir)) {
            return 7;
        }
        if (redir_equal_redir(oa_redir) || redir_equal_redir(on_redir)) {
            return 8;
        }
        return 6;
    }
    return 0;
}

/*
* redir_equal_redir() - checks if the redirection filepath is another 
*   redirection symbol
*
* Input:
*   redir - char[1024], redirection file path string
*
* Returns:
*   0, if redir doesn't equal any redirection symbol, 1 if it equals at least 1
*/
int redir_equal_redir(char redir[1024]) {
    return ((strcmp(redir, "<") == 0) || (strcmp(redir, ">") == 0) || 
    (strcmp(redir, ">>") == 0));
}

/*
* parse_fg_bg() - parses the argv for correct syntax of fg and bg commands
*
* Inputs:
*   argv - char*[512], the argument array eventually used for execv()
*   num_argv - int, number of arguments in argv 
*   jid_num - *int, pointer to where the job id to continue will be stored
*
* Returns:
*   int, where
*       0 - parses succesfully
*       1 - syntax error: argv does not have 2 argv
*       2 - job input does not begin with %
*/
int parse_fg_bg(char *argv[512], int num_argv, int *jid_num) {
    if (num_argv != 2) {
        return 1;
    } 
    char *p_index = strchr(argv[1], '%');
    if (p_index == 0 || (int)(p_index - argv[1]) != 0) {
        return 2;
    }
    // if no number after % in argv[1]
    if (*(p_index + 1) == '\0') {
        *jid_num = 0;
        return 0;
    }
    *jid_num = atoi(p_index + 1);
    return 0;
}
