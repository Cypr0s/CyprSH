#ifndef ERROR_H
#define ERROR_H

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef enum return_values {
    SUCCESS                     = EXIT_SUCCESS,  // 0
    ERROR_DEFAULT               = EXIT_FAILURE,  // 1
    ERROR_SHELL_MISUSE          = 2,
    ERROR_MALLOC_FAILURE        = 3,
    ERROR_INT_OVERFLOW          = 4,
    ERROR_INDEX_OUT_OF_BOUNDS   = 5,
    ERROR_STACK_OVERFLOW        = 6,
    ERROR_STACK_UNDERFLOW       = 7,
    ERROR_LEXICAL_ERROR         = 8,
    ERROR_LEXER_BUFFER_OVERFLOW = 9,
    ERROR_SYNTAX_ERROR          = 10,
    ERROR_FILE_NOT_READABLE     = 11,
    ERROR_FILE_NOT_FOUND        = 12,
    ERROR_COMM_CANNOT_EXEC      = 126,
    ERROR_COMMAND_NOT_FOUND     = 127,
} StatusEnum;

#define ERR_CHECK(status) do { \
    if ((status) != SUCCESS) { \
        return (status); \
    } \
} while(0)

void printErrno(const char *path);

#endif // ERROR_H
