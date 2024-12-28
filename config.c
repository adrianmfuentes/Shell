#include "config.h"

Alias *alias_list = NULL;

void load_config(int argc, char **argv) {
    printf("Loading configuration files...\n");

    if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            perror("Error opening configuration file");
            exit(EXIT_FAILURE);
        }

        fseek(file, 0, SEEK_END);
        long length = ftell(file);
        fseek(file, 0, SEEK_SET);
        char *data = malloc(length + 1);
        fread(data, 1, length, file);
        fclose(file);

        cJSON *config = cJSON_Parse(data);
        if (!config) {
            fprintf(stderr, "Error parsing JSON configuration file\n");
            free(data);
            return;
        }

        // Load PATH, EDITOR and PROMPT from the JSON
        cJSON *path = cJSON_GetObjectItem(config, "PATH");
        if (cJSON_IsString(path)) setenv("PATH", path->valuestring, 1);
        cJSON *editor = cJSON_GetObjectItem(config, "EDITOR");
        if (cJSON_IsString(editor)) setenv("EDITOR", editor->valuestring, 1);
        cJSON *prompt = cJSON_GetObjectItem(config, "PROMPT");
        if (cJSON_IsString(prompt)) setenv("PS1", prompt->valuestring, 1);

        // Load aliases from the JSON
        cJSON *aliases = cJSON_GetObjectItem(config, "aliases");
        if (cJSON_IsObject(aliases)) load_aliases_from_json(aliases);

        // Free the cJSON object and the data buffer
        cJSON_Delete(config); free(data);

    } else { printf("No configuration file provided.\n"); }
}

void process_line(char *line) {
    if (line[0] == '\n' || line[0] == '#') return;  

    char *key = strtok(line, "=");
    char *value = strtok(NULL, "\n");

    if (key && value) {
        if (strcmp(key, "PATH") == 0) {
            setenv("PATH", value, 1);
        } else if (strcmp(key, "EDITOR") == 0) {
            setenv("EDITOR", value, 1);
        } else if (strcmp(key, "PS1") == 0) {
            setenv("PS1", value, 1);
        } else if (strncmp(key, "alias", 5) == 0) {
            add_alias(key + 6, value);  // Skip the "alias " prefix
            printf("Alias added: %s = %s\n", key + 6, value);
        } else {
            printf("Unknown configuration: %s = %s\n", key, value);
        }
    }
}

void add_alias(const char *name, const char *command) {
    Alias *new_alias = malloc(sizeof(Alias));
    if (!new_alias) {
        fprintf(stderr, "Error allocating memory for alias\n");
        return;
    }
    new_alias->name = strdup(name);
    new_alias->command = strdup(command);
    new_alias->next = alias_list;
    alias_list = new_alias;
}

void print_aliases() {
    Alias *current = alias_list;
    printf("Current aliases:\n");
    while (current) {
        printf("  %s = %s\n", current->name, current->command);
        current = current->next;
    }
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

void load_aliases_from_json(cJSON *aliases) {
    cJSON *alias = NULL;
    cJSON_ArrayForEach(alias, aliases) {
        if (alias->string && cJSON_IsString(alias)) {
            add_alias(alias->string, alias->valuestring);
            printf("Alias loaded: %s = %s\n", alias->string, alias->valuestring);
        }
    }
}
