#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "command.h"
#include "shell.h"

static void print_prompt(void) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        const char *home = getenv("HOME");
        const char *display = cwd;
        char shortened[1024];
        if (home && strncmp(cwd, home, strlen(home)) == 0) {
            snprintf(shortened, sizeof(shortened), "~%s", cwd + strlen(home));
            display = shortened;
        }
        printf("\033[1;36m%s\033[0m ", display);
    }

    const char *ps1 = getenv("PS1");
    printf("%s", ps1 ? ps1 : "$ ");
    fflush(stdout);
}

void run_shell() {
    char *line = NULL;
    char **args = NULL;
    int status;

    do {
        print_prompt();
        line = read_line();
        args = split_line(line);
        status = execute_command(args);

        free(line);
        free_args(args);
    } while (status);
}
