#ifndef CLEANUP_H
#define CLEANUP_H

#include <stdio.h>

typedef struct Alias {
    char *name;
    char *command;
    struct Alias *next;
} Alias;

extern Alias *alias_list;

void perform_cleanup();
void free_aliases();
void close_open_files(FILE *file);

#endif // CLEANUP_H
