#ifndef COMMAND_H
#define COMMAND_H

char *read_line(void);
char **split_line(char *line);
int execute_command(char **args);

#endif // COMMAND_H
