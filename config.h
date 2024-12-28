#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

typedef struct Alias {
    char *name;
    char *command;
    struct Alias *next;
} Alias;

extern Alias *alias_list;

void load_config(int argc, char **argv);
void process_line(char *line);
void add_alias(const char *name, const char *command);
void print_aliases();
void free_aliases();
void load_aliases_from_json(cJSON *aliases);

#endif // CONFIG_H
