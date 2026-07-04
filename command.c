#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "command.h"
#include "config.h"

#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

char *builtin_str[] = {"cd", "help", "exit", "pwd", "echo", "clear", "alias"};
int (*builtin_func[])(char **) = {&lsh_cd, &lsh_help, &lsh_exit, &lsh_pwd, &lsh_echo, &lsh_clear, &lsh_alias};
int num_builtins = sizeof(builtin_str) / sizeof(char *);


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

    if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    int cap = LSH_RL_BUFSIZE;
    char *current = malloc(cap);
    int len = 0;
    int has_token = 0; // distinguishes "" (from quotes) from no token yet

    int in_quotes = 0;
    int escaping = 0;

    if (!current) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; line[i] != '\0'; i++) {
        char c = line[i];

        if (escaping) {
            if (len + 1 >= cap) {
                cap *= 2;
                current = realloc(current, cap);
                if (!current) {
                    fprintf(stderr, "lsh: allocation error\n");
                    exit(EXIT_FAILURE);
                }
            }
            current[len++] = c;
            has_token = 1;
            escaping = 0;
            continue;
        }

        if (c == '\\') {
            escaping = 1;
            continue;
        }

        if (c == '"') {
            in_quotes = !in_quotes;
            has_token = 1; // "" still counts as an (empty) token
            continue;
        }

        if ((c == ' ' || c == '\t') && !in_quotes) {
            if (has_token) {
                if (position >= bufsize) {
                    bufsize += LSH_TOK_BUFSIZE;
                    tokens = realloc(tokens, bufsize * sizeof(char *));
                    if (!tokens) {
                        fprintf(stderr, "lsh: allocation error\n");
                        exit(EXIT_FAILURE);
                    }
                }
                current[len] = '\0';
                tokens[position++] = strdup(current);
                len = 0;
                has_token = 0;
            }
            continue;
        }

        if (len + 1 >= cap) {
            cap *= 2;
            current = realloc(current, cap);
            if (!current) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        current[len++] = c;
        has_token = 1;
    }

    if (has_token) {
        if (position >= bufsize) {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        current[len] = '\0';
        tokens[position++] = strdup(current);
    }

    free(current);
    tokens[position] = NULL;
    return tokens;
}

void free_args(char **args) {
    if (!args) return;
    for (int i = 0; args[i] != NULL; i++) {
        free(args[i]);
    }
    free(args);
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

// Looks up args[0] as an alias and, if found, builds a new argv with the
// alias expansion spliced in front of the remaining user-supplied args.
static char **expand_alias(char **args) {
    Alias *current = alias_list;
    while (current) {
        if (strcmp(current->name, args[0]) == 0) {
            char *alias_line = strdup(current->command);
            char **alias_tokens = split_line(alias_line);
            free(alias_line);

            int alias_count = 0;
            while (alias_tokens[alias_count]) alias_count++;
            int rest_count = 0;
            while (args[rest_count + 1]) rest_count++;

            char **expanded = malloc((alias_count + rest_count + 1) * sizeof(char *));
            int pos = 0;
            for (int i = 0; i < alias_count; i++) expanded[pos++] = alias_tokens[i];
            for (int i = 0; i < rest_count; i++) expanded[pos++] = strdup(args[i + 1]);
            expanded[pos] = NULL;

            free(alias_tokens); // shallow free: ownership of strings moved into expanded
            return expanded;
        }
        current = current->next;
    }
    return NULL;
}

int execute_command(char **args) {
    if (args[0] == NULL) {
        return 1; // Empty command
    }

    char **expanded = expand_alias(args);
    char **exec_args = expanded ? expanded : args;

    int status = 1;
    if (exec_args[0] == NULL) {
        status = 1;
    } else {
        int handled = 0;
        for (int i = 0; i < num_builtins; i++) {
            if (strcmp(exec_args[0], builtin_str[i]) == 0) {
                status = builtin_func[i](exec_args);
                handled = 1;
                break;
            }
        }
        if (!handled) {
            status = launch_process(exec_args);
        }
    }

    if (expanded) free_args(expanded);
    return status;
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
    printf("  pwd        Print working directory\n");
    printf("  echo [..]  Print arguments\n");
    printf("  clear      Clear the screen\n");
    printf("  alias      List aliases loaded from the config file\n");
    return 1;
}

int lsh_exit(char **args) {
    return 0; // Exiting the shell
}

int lsh_pwd(char **args) {
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

int lsh_alias(char **args) {
    print_aliases();
    return 1;
}
