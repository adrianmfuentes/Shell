#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "command.h"

#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

char *builtin_str[] = {"cd", "help", "exit", "pwd", "echo", "clear"};
int (*builtin_func[])(char **) = {&lsh_cd, &lsh_help, &lsh_exit, &lsh_pwd, &lsh_echo, &lsh_clear};


char *read_line(void) {
    int bufsize = LSH_RL_BUFSIZE, position = 0;
    char *buffer = malloc(bufsize * sizeof(char));
    int c;

    if (!buffer) {
        fprintf(stderr, "lsh: assigment error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position++] = c;
        }

        if (position >= bufsize) {
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "lsh: assigment error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char **split_line(char *line) {
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));

    char *token; 
    int i = 0, j = 0;

    int in_quotes = 0; // Flag to indicate if we are inside quotes
    int escaping = 0; // Flag to indicate if we are escaping a character

    if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; line[i] != '\0'; i++) {
        if (line[i] == '\\' && !escaping) {
            escaping = 1; continue; // Skip the next character, as it's escaped
        }

        // Handle opening/closing quotes
        if (line[i] == '"' && !escaping) {
            in_quotes = !in_quotes;  continue; // Skip the quote itself
        }

        // Handle tokenization: handle spaces only if we're not inside quotes
        if ((line[i] == ' ' || line[i] == '\t') && !in_quotes && !escaping && j > 0) {
            tokens[position] = malloc(j + 1);
            strncpy(tokens[position], &line[i - j], j);
            tokens[position][j] = '\0';
            position++;
            j = 0;
            
        } else {
            // Add current character to the current token
            if (j == 0 && position >= bufsize) {
                bufsize += LSH_TOK_BUFSIZE;
                tokens = realloc(tokens, bufsize * sizeof(char *));

                if (!tokens) {
                    fprintf(stderr, "lsh: allocation error\n");
                    exit(EXIT_FAILURE);
                }
            }
            
            line[i] == '\\' ? (escaping = 1) : (escaping = 0); // Reset escape flag
            j++;
        }
    }
    
    if (j > 0) { // Add last token if necessary
        tokens[position] = malloc(j + 1);
        strncpy(tokens[position], &line[i - j], j);
        tokens[position][j] = '\0';
        position++;
    }

    tokens[position] = NULL; // Null-terminate the array
    return tokens;
}

int launch_process(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("lsh");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

int execute_command(char **args) {
    if (args[0] == NULL) {
        return 1; // Empty command
    }

    if (strcmp(args[0], "cd") == 0) {
        return builtin_cd(args);
    } else if (strcmp(args[0], "exit") == 0) {
        return builtin_exit(args);
    }

    return launch_process(args);
}

int lsh_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "lsh: an argument was expected for \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("lsh");
        }
    }
    return 1;
}

int lsh_help(char **args) {
    printf("Shell built-in commands:\n");
    printf("  cd [dir]   Change directory\n");
    printf("  help       Display this help message\n");
    printf("  exit       Exit the shell\n");
    return 1;
}

int lsh_exit(char **args) {
    return 0; // Exiting the shell
}

nt lsh_pwd(char **args) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("lsh");
    }
    return 1;
}

int lsh_echo(char **args) {
    for (int i = 1; args[i] != NULL; i++) {
        printf("%s ", args[i]);
    }
    printf("\n");
    return 1;
}

int lsh_clear(char **args) {
    printf("\033[H\033[J"); // ANSI escape sequence to clear the screen
    return 1;
}