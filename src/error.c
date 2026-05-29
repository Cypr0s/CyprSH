#include "error.h"
#include <stdio.h>
#include <stdlib.h>


void printErrno(const char *path) {
    fprintf(stderr, "%s: %s\n", path, strerror(errno));
}
