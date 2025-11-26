#include "error.h"
#include <stdio.h>

ExitEnum error = EXIT_SUCCESS; 

void print_errno(const char *path) {
    fprintf(stderr, "%s: %s\n", path, strerror(errno));
}

void print_error(void) {
    
}