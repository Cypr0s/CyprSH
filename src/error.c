/**
 * @file        error.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Error handling implementation
 */

#include "error.h"

/** @brief Prints system error for a given path
 *  @param path Path string for error message
 */
void printErrno(const char *path) {
    fprintf(stderr, "%s: %s\n", path, strerror(errno));
}


/** @brief Prints formatted error message
 *  @param function_name Name of function where error occurred
 *  @param format Format string with variadic arguments
 */
void printError(const char* function_name, const char* format, ...) {
    fprintf(stderr, "CyprSH: %s: ", function_name);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}