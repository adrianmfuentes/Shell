#ifndef COMMAND_H
#define COMMAND_H

// Function Declarations for builtin shell commands
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

// List of builtin commands, followed by their corresponding functions.
extern char *builtin_str[];
extern int (*builtin_func[])(char **);

char *read_line(void);
char **split_line(char *line);
int execute_command(char **args);

#endif // COMMAND_H
