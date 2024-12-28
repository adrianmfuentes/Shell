#include <stdio.h>
#include <stdlib.h>
#include "command.h"
#include "shell.h"

void run_shell() {
    char *line = NULL;
    char **args = NULL;
    int status;

    do {
        printf("> ");
        line = read_line();
        args = split_line(line);
        status = execute_command(args);

        free(line);
        free(args);
    } while (status);
}
