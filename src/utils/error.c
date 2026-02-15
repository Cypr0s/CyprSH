#include "error.h"
#include <stdio.h>


void print_errno(const char *path) {
    fprintf(stderr, "%s: %s\n", path, strerror(errno));
}
