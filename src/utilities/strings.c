/**
 * @file        strings.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   String utility functions implementation
 */

#include "utilities/strings.h"

/** @brief Compare two strings for equality
 *  @param str1 First string
 *  @param str2 Second string
 *  @return 1 if equal, 0 otherwise
 */
uint8_t streq(const char* str1, const char* str2) {
    if(str1 == NULL || str2 == NULL) return 0U;
    while(*str1 && *str2) {
        if(*str1 != *str2) return 0U;
        str1++;
        str2++;
    }

    return (*str1 == *str2) ? 1U : 0U;
}

/** @brief Duplicate string
 *  @param src Source string to duplicate
 *  @return Pointer to new string (caller must free) or NULL
 */
char* strdup(const char* src) {
    size_t len = strlen(src);
    char* copy = (char*) malloc(len + 1);
    if(copy == NULL) {
        return NULL;
    }
    memcpy(copy, src, len + 1);
    return copy;
}

/** @brief Split assignment string into key and value
 *  @param str Assignment string (key=value format)
 *  @param key Output pointer for key string
 *  @param value Output pointer for value string
 *  @return Status code
 */
StatusEnum splitAssignment(const char* str, char** key, char** value) {
    if(str == NULL || key == NULL || value == NULL) {
        return ERROR_DEFAULT;
    }

    const char* eq = strchr(str, '=');
    if(eq == NULL) {
        return ERROR_DEFAULT;  // no = found
    }

    size_t key_len = (size_t)(eq - str);
    *key = strndup(str, key_len);
    if(*key == NULL) {
        return ERROR_MALLOC_FAILURE;
    }

    *value = strdup(eq + 1);
    if(*value == NULL) {
        free(*key);
        *key = NULL;
        return ERROR_MALLOC_FAILURE;
    }

    return SUCCESS;
}

uint8_t isNameStart(char c) {
    return isalpha((unsigned char)c) || c == '_';
}

uint8_t isNameChar(char c) {
    return isalnum((unsigned char)c) || c == '_';
}