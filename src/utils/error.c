#include "error.h"
#include <stdio.h>

StatusEnum error = SUCCESS;

void print_errno(const char *path) {
    fprintf(stderr, "%s: %s\n", path, strerror(errno));
}

void print_error(void) {
    fprintf(stderr, "error: %d\n", error);
}
