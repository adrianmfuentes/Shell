#include "cleanup.h"
#include <stdlib.h>
#include <string.h>

Alias *alias_list = NULL;
FILE *file = NULL;

void perform_cleanup() {
    printf("Cleaning before finishing...\n");

    free_aliases();
    close_open_files(file);
}

void free_aliases() {
    Alias *current = alias_list;

    while (current) {
        Alias *next = current->next;
        free(current->name);      
        free(current->command);   
        free(current);            
        current = next;
    }

    alias_list = NULL;  
}

void close_open_files(FILE *file) {
    if (file) {
        fclose(file);  
        file = NULL;   
    }
}