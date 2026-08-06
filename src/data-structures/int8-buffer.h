/**
 * @file        int8_buffer.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   8-bit integer buffer with dynamic resizing
 */

#ifndef INT8_T_BUFFER_H
#define INT8_T_BUFFER_H

#include "data-structures/buffer-type.h"
#include "stdint.h"

typedef struct {
    size_t size;
    size_t capacity;
    int8_t* buff;
} Int8Buffer, *Int8BufferPtr;

/** @brief Initialize 8-bit buffer
 *  @param tb Buffer to initialize
 *  @param capacity Initial capacity (0 uses default)
 *  @return Status code
 */
StatusEnum int8BufferCtor(Int8BufferPtr tb, size_t capacity);

/** @brief Destroy 8-bit buffer
 *  @param tb Buffer to destroy
 */
void int8BufferDtor(Int8BufferPtr tb);

/** @brief Append 8-bit value to buffer
 *  @param tb Target buffer
 *  @param value Value to append
 *  @return Status code
 */
StatusEnum int8BufferAppend(Int8BufferPtr tb, int8_t value);

/** @brief Transfer buffer ownership and reset
 *  @param tb Buffer to transfer
 *  @return Pointer to buffer data (caller must free)
 */
int8_t* int8BufferTransfer(Int8BufferPtr tb);

#endif