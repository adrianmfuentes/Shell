#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "shell.h"
#include "cleanup.h"

int main(int argc, char **argv) {
    load_config(argc, argv);
    run_shell();
    perform_cleanup();
    return EXIT_SUCCESS;
}
