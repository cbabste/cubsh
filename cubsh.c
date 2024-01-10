/*
 * A basic custom UNIX shell that supports upto one level of pipe redirection
 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define PROMPT              "cubsh $ "
#define GOODBYE             "Goodbye. Thanks for using cubsh."
#define MAX_COMMAND_ARGS    10

void sig_handler(int sig) {
    write(STDOUT_FILENO, "\n", 1);
    write(STDOUT_FILENO, GOODBYE, strlen(GOODBYE));
    write(STDOUT_FILENO, "\n", 1);
    _exit(EXIT_SUCCESS);
}

int init_shell() {
    // Register for SIGINT
    signal(SIGINT, sig_handler);
    printf("\nWelcome to cubsh\n");
    return 0;
}

int parse_command(char **command, char *args_list[],
                  char *piped_args_list[], int *is_piped) {
    int     index = 0;
    int     piped_args_index = 0;
    int     pipe_cnt = 0;
    char    *token = NULL;

    token = strtok(*command, " ");
    while (token != NULL) {
        if (*is_piped) {
            piped_args_list[piped_args_index++] = token;
        } else {
            args_list[index++] = token;
        }
        if (strcmp(token, "|") == 0) {
            // There is a pipe in the command.
            // Set the appropriate flag
            *is_piped = 1;
            pipe_cnt += 1;

            // Decrement previously incremented index by 1 since
            // this is the pipe character.
            index -= 1;
        }
        token = strtok(NULL, " ");
        if(pipe_cnt > 1) {
            break;
        }
    }

    if (pipe_cnt > 1) {
        printf("cubsh ~> max supported pipe count is 1\n");
        return 1;
    }

    args_list[index] = NULL;
    piped_args_list[piped_args_index] = NULL;

    return 0;
}

void exec_command(char *args_list[]) {
    pid_t           pid;
    int             pid_status = 0;

    pid = fork();
    if (pid == -1) {
        perror("cubsh (fork failure)");
        return;
    }

    if(pid) {
        // Parent process. just wait on the child.
        wait(NULL);
    } else {
        // Child process. Execute the command.
        if(execvp(args_list[0], args_list) == -1) {
            perror("cubsh (execve failed)");
        }
    }
}

void exec_piped_command(char *args_list[], char *piped_args_list[]) {
    int         fds[2];
    pid_t       command_pid, piped_command_pid;

    if(pipe(fds) < 0) {
        perror("cubsh (pipe failed)");
        return;
    }

    if((command_pid = fork()) < 0) {
        perror("cubsh (fork failed)");
        return;
    }

    if (command_pid == 0) {
        // Child process executing first command

        // fds[0] is the read end of the pipe. The first command
        // only needs the write end, so close fds[0]
        close(fds[0]);
        dup2(fds[1], STDOUT_FILENO);
        close(fds[1]);

        if(execvp(args_list[0], args_list) == -1) {
            perror("cubsh (execve failed - first command)");
        }
    } else {
        // Parent process.
        if((piped_command_pid = fork()) < 0) {
            perror("cubsh (fork failed)");
            return;
        }

        if(piped_command_pid == 0) {
            // fds[1] is the write end of the pipe. The piped command
            only needs the read end, so close fds[1] */
            close(fds[1]);
            dup2(fds[0], STDIN_FILENO);
            close(fds[0]);
            if(execvp(piped_args_list[0], piped_args_list) == -1) {
                perror("cubsh (execve failed - piped command)");
            }
        } else {
            // Close the fds in parent
            close(fds[0]);
            close(fds[1]);

            // Wait for both the children to stop
            wait(NULL);
            wait(NULL);
        }
    }
}

int check_exit_inputs(char *command) {
    if(strcmp(command, "quit") == 0 ||
       strcmp(command, "quit()") == 0 ||
       strcmp(command, "exit") == 0 ||
       strcmp(command, "exit()") == 0 ||
       strcmp(command, "q") == 0) {
        return 0;
    }

    return 1;
}

void shell_loop() {
    char            *command = NULL;
    size_t          command_len = 0;
    size_t          MAX_COMMAND_LEN = 10;
    char            *args_list[MAX_COMMAND_ARGS];
    char            *piped_args_list[MAX_COMMAND_ARGS];
    int             is_piped;

    while(1) {
        is_piped = 0;
        memset(args_list, 0, MAX_COMMAND_ARGS);
        memset(piped_args_list, 0, MAX_COMMAND_ARGS);

        // Allocate memory for command buffer
        if((command = (char *) malloc(MAX_COMMAND_LEN * sizeof(char))) == NULL) {
            perror("cubsh (malloc failed)");
            exit(EXIT_FAILURE);
        }

        //Print the prompt
        printf("%s", PROMPT);

        // Read the command from stdin
        command_len = getline(&command, &MAX_COMMAND_LEN, stdin);
        if (command_len == -1) {
            // EOF like Cltr+D 
            printf("\n%s\n", GOODBYE);
            free(command);
            exit (EXIT_SUCCESS);
        }

        if (command_len == 0) {
            // Empty command, nothing to do, flush the stdin and take input again.
            continue;
        }

        // Null terminate the command
        command[strlen(command) - 1] = '\0';
        if(check_exit_inputs(command) == 0) {
            printf("\n%s\n", GOODBYE);
            free(command);
            exit (EXIT_SUCCESS);
        }

        if(parse_command(&command, args_list, piped_args_list, &is_piped) != 0) {
            free(command);
            continue;
        }

        if (!is_piped) {
            // This is a non-pipe command.
            exec_command(args_list);
        } else {
            // This command has one pipe.
            exec_piped_command(args_list, piped_args_list);
        }
        free(command);
    }
}

int main(int argc, char *argv[]) {
    init_shell();
    shell_loop();

    return 0;
}
