#include "error.h"


void printErrno(const char *path) {
    fprintf(stderr, "%s: %s\n", path, strerror(errno));
}


void printError(const char* function_name, const char* format, ...) {
    fprintf(stderr, "CyprSH: %s: ", function_name);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}