/**
 * @file        error.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Error handling and status codes
 */

#ifndef ERROR_H
#define ERROR_H

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

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
    ERROR_HTAB_ITEM             = 13,
    ERROR_COMM_CANNOT_EXEC      = 126,
    ERROR_COMMAND_NOT_FOUND     = 127,
} StatusEnum;

#define ERR_CHECK(status) do { \
    if ((status) != SUCCESS) { \
        return (status); \
    } \
} while(0)

/** @brief Prints system error for a given path
 *  @param path Path string for error message
 */
void printErrno(const char *path);

/** @brief Prints formatted error message
 *  @param function_name Name of function where error occurred
 *  @param format Format string with variadic arguments
 */
void printError(const char* function_name, const char* format, ...);

#endif // ERROR_H
