/**
 * @file        int8_buffer.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   8-bit integer buffer implementation
 */

#include "data-structures/int8-buffer.h"

/** @brief Initialize 8-bit buffer
 *  @param tb Buffer to initialize
 *  @param capacity Initial capacity (0 uses default)
 *  @return Status code
 */
StatusEnum int8BufferCtor(Int8BufferPtr tb, size_t capacity) {
    if(tb == NULL) {
        printError("int8BufferCtor", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }

    tb->size = 0;
    tb->capacity = capacity > 0 ? capacity : DEFAULT_BUFFER_SIZE;
    tb->buff = (int8_t*) malloc(tb->capacity * sizeof(int8_t));
    if(tb->buff == NULL) {
        printError("int8BufferCtor", "Malloc failure");
        return ERROR_MALLOC_FAILURE;
    }
    return SUCCESS;
}

/** @brief Destroy 8-bit buffer
 *  @param tb Buffer to destroy
 */
void int8BufferDtor(Int8BufferPtr tb) {
    if(tb == NULL) {
        return;
    }

    free(tb->buff);
    tb->buff = NULL;
    tb->size = 0;
    tb->capacity = 0;
}

/** @brief Append 8-bit value to buffer
 *  @param tb Target buffer
 *  @param value Value to append
 *  @return Status code
 */
StatusEnum int8BufferAppend(Int8BufferPtr tb, int8_t value) {
    if(tb == NULL) {
        printError("int8BufferAppend", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }

    size_t needed = tb->size + 1;
    if(needed > tb->capacity) {
        while(tb->capacity < needed) {
            tb->capacity *= 2;
        }
        int8_t* new_buff = (int8_t*) realloc(tb->buff, tb->capacity * sizeof(int8_t));
        if(new_buff == NULL) {
            printError("int8BufferAppend", "Realloc failure");
            return ERROR_MALLOC_FAILURE;
        }
        tb->buff = new_buff;
    }

    tb->buff[tb->size] = value;
    tb->size++;
    return SUCCESS;
}

/** @brief Transfer buffer ownership and reset
 *  @param tb Buffer to transfer
 *  @return Pointer to buffer data (caller must free)
 */
int8_t* int8BufferTransfer(Int8BufferPtr tb) {
    if(tb == NULL) {
        printError("int8BufferTransfer", "Passing NULL pointer");
        return NULL;
    }
    int8_t* out = tb->buff;
    tb->buff = NULL;
    tb->capacity = 0;
    tb->size = 0;
    return out;
}