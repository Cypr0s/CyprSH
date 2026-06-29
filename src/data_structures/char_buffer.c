/**
 * @file        char_buffer.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Character buffer implementation
 */

#include "data_structures/char_buffer.h"

/** @brief Initialize character buffer
 *  @param cb Buffer to initialize
 *  @param capacity Initial capacity (0 uses default)
 *  @return Status code
 */
StatusEnum charBufferCtor(CharBufferPtr cb, size_t capacity) {
    if(cb == NULL) {
        printError("charBufferCtor", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }

    cb->size = 0;
    cb->capacity = capacity > 0 ? capacity : DEFAULT_BUFFER_SIZE;
    cb->buff = (char*) malloc(cb->capacity);
    if(cb->buff == NULL) {
        printError("charBufferCtor", "Malloc failure");
        return ERROR_MALLOC_FAILURE;
    }
    cb->buff[0] = '\0';
    return SUCCESS;
}

/** @brief Destroy character buffer
 *  @param cb Buffer to destroy
 */
void charBufferDtor(CharBufferPtr cb) {
    if(cb == NULL) {
        return;
    }

    free(cb->buff);
    cb->buff = NULL;
    cb->size = 0;
    cb->capacity = 0;
}

/** @brief Append single character to buffer
 *  @param cb Target buffer
 *  @param c Character to append
 *  @return Status code
 */
StatusEnum charBufferAppendChar(CharBufferPtr cb, char c) {
    if(cb == NULL) {
        printError("charBufferAppend", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }

    return charBufferAppendCharPtr(cb, &c, 1);
}

/** @brief Append character string to buffer
 *  @param cb Target buffer
 *  @param str String data to append
 *  @param str_size Number of characters to append
 *  @return Status code
 */
StatusEnum charBufferAppendCharPtr(CharBufferPtr cb, char* str, size_t str_size) {
    if(cb == NULL || str == NULL) {
        printError("charBufferAppend", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }

    size_t needed = cb->size + str_size + 1; // +1 == '\0'
    if(needed > cb->capacity) {
        while(cb->capacity < needed) {
            cb->capacity *= 2;
        }
        char* new_buff = (char*) realloc(cb->buff, cb->capacity);
        if(new_buff == NULL) {
            printError("charBufferAppend", "Realloc failure");
            return ERROR_MALLOC_FAILURE;
        }
        cb->buff = new_buff;
    }

    memcpy(cb->buff + cb->size, str, str_size);
    cb->size += str_size;
    cb->buff[cb->size] = '\0';
    return SUCCESS;
}

/** @brief Transfer buffer ownership and reset
 *  @param cb Buffer to transfer
 *  @return Pointer to buffer data (caller must free)
 */
char* charBufferTransfer(CharBufferPtr cb) {
    if(cb == NULL) {
        printError("charBufferTransfer", "Passing NULL pointer");
        return NULL;
    }
    char* out = cb->buff;
    cb->buff = NULL;
    cb->capacity = 0;
    cb->size = 0;
    return out;
}