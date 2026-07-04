#include "cleanup.h"
#include "config.h"
#include <stdlib.h>
#include <string.h>

void perform_cleanup() {
    printf("Cleaning before finishing...\n");
    free_aliases();
}
