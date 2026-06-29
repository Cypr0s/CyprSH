/**
 * @file        strings.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   String utility functions
 */

#ifndef STRINGS_H
#define STRINGS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "error.h"

/** @brief Compare two strings for equality
 *  @param str1 First string
 *  @param str2 Second string
 *  @return 1 if equal, 0 otherwise
 */
uint8_t streq(const char* str1, const char* str2);

/** @brief Duplicate string
 *  @param src Source string to duplicate
 *  @return Pointer to new string (caller must free) or NULL
 */
char* strdup(const char* src);

/** @brief Split assignment string into key and value
 *  @param str Assignment string (key=value format)
 *  @param key Output pointer for key string
 *  @param value Output pointer for value string
 *  @return Status code
 */
StatusEnum splitAssignment(const char* str, char** key, char** value);

#endif // STRINGS_H