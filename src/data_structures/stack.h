/**
 * @file        stack.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Fixed-size stack for 8-bit values
 */

#ifndef STACK_H
#define STACK_H

#include <stdint.h>
#include "error.h"

/** Maximum stack capacity for nested structures */
#define STACK_CAPACITY 128

/** Generic fixed-size stack over int8_t (enum values stored as int8_t) */
typedef struct {
    int8_t data[STACK_CAPACITY];
    int8_t top;  // -1 = empty
} Stack, *StackPtr;

/** @brief Initialize stack to empty state
 *  @param stack Stack to initialize
 *  @return Status code
 */
StatusEnum stackCtor(StackPtr stack);

/** @brief Push value onto stack
 *  @param stack Target stack
 *  @param value Value to push
 *  @return Status code
 */
StatusEnum stackPush(StackPtr stack, int8_t value);

/** @brief Pop value from stack
 *  @param stack Target stack
 *  @return Status code
 */
StatusEnum stackPop(StackPtr stack);

/** @brief Get top value without removing
 *  @param stack Source stack
 *  @param out Output pointer for top value
 *  @return Status code
 */
StatusEnum stackTop(StackPtr stack, int8_t* out);

/** @brief Check if stack is empty
 *  @param stack Stack to check
 *  @return 1 if empty, 0 if not, -1 on error
 */
int8_t stackIsEmpty(StackPtr stack);

#endif // STACK_H
