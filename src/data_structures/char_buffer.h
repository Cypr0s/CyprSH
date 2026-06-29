/**
 * @file        char_buffer.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Character buffer with dynamic resizing
 */

#ifndef CHAR_BUFFER_H
#define CHAR_BUFFER_H

#include "data_structures/buffer.h"

typedef struct {
    size_t size;
    size_t capacity;
    char* buff;
} CharBuffer, *CharBufferPtr;

/** @brief Initialize character buffer
 *  @param cb Buffer to initialize
 *  @param capacity Initial capacity (0 uses default)
 *  @return Status code
 */
StatusEnum charBufferCtor(CharBufferPtr cb, size_t capacity);

/** @brief Destroy character buffer
 *  @param cb Buffer to destroy
 */
void charBufferDtor(CharBufferPtr cb);

/** @brief Append single character to buffer
 *  @param cb Target buffer
 *  @param c Character to append
 *  @return Status code
 */
StatusEnum charBufferAppendChar(CharBufferPtr cb, char c);

/** @brief Append character string to buffer
 *  @param cb Target buffer
 *  @param str String data to append
 *  @param str_size Number of characters to append
 *  @return Status code
 */
StatusEnum charBufferAppendCharPtr(CharBufferPtr cb, char* str, size_t str_size);

/** @brief Transfer buffer ownership and reset
 *  @param cb Buffer to transfer
 *  @return Pointer to buffer data (caller must free)
 */
char* charBufferTransfer(CharBufferPtr cb);

#endif

