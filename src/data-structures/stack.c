/**
 * @file        stack.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Stack implementation
 */

#include "data-structures/stack.h"

/** @brief Initialize stack to empty state
 *  @param stack Stack to initialize
 *  @return Status code
 */
StatusEnum stackCtor(StackPtr stack) {
    if (stack == NULL) {
        printError("stackInit", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }
    stack->top = -1;
    return SUCCESS;
}

/** @brief Push value onto stack
 *  @param stack Target stack
 *  @param value Value to push
 *  @return Status code
 */
StatusEnum stackPush(StackPtr stack, int8_t value) {
    if (stack == NULL) {
        printError("stackPush", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }

    if (stack->top >= STACK_CAPACITY - 1) {
        printError("stackPush", "Stack overflowed maxium nestings `%d`", STACK_CAPACITY);
        return ERROR_STACK_OVERFLOW;
    }
    stack->data[++stack->top] = value;
    return SUCCESS;
}

/** @brief Pop value from stack
 *  @param stack Target stack
 *  @return Status code
 */
StatusEnum stackPop(StackPtr stack) {
    if (stack == NULL) {
        printError("stackPop", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }
    if (stack->top < 0) {
        printError("stackPop", "Stack underflow");
        return ERROR_STACK_UNDERFLOW;
    }
    stack->top--;
    return SUCCESS;
}

/** @brief Get top value without removing
 *  @param stack Source stack
 *  @param out Output pointer for top value
 *  @return Status code
 */
StatusEnum stackTop(StackPtr stack, int8_t* out) {
    if (stack == NULL || out == NULL) {
        printError("stackTop", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }
    if (stack->top < 0) {
        printError("stackTop", "Stack is empty");
        return ERROR_STACK_UNDERFLOW;
    }
    *out = stack->data[stack->top];
    return SUCCESS;
}

/** @brief Check if stack is empty
 *  @param stack Stack to check
 *  @return 1 if empty, 0 if not, -1 on error
 */
int8_t stackIsEmpty(StackPtr stack) {
    if(stack == NULL) {
        printError("stackIsEmpty", "Passing NULL pointer");
        return -1;
    }
    return (stack == NULL || stack->top < 0) ? 1 : 0;
}
