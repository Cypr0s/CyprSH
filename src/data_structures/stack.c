#include "data_structures/stack.h"

StatusEnum stackInit(StackPtr stack) {
    if (stack == NULL) {
        fprintf(stderr, "stackInit: NULL pointer\n");
        return ERROR_DEFAULT;
    }
    stack->top = -1;
    return SUCCESS;
}

StatusEnum stackPush(StackPtr stack, int8_t value) {
    if (stack == NULL) {
        fprintf(stderr, "stackPush: NULL pointer\n");
        return ERROR_DEFAULT;
    }
    if (stack->top >= STACK_CAPACITY - 1) {
        fprintf(stderr, "stackPush: stack overflow (capacity %d)\n", STACK_CAPACITY);
        return ERROR_STACK_OVERFLOW;
    }
    stack->data[++stack->top] = value;
    return SUCCESS;
}

StatusEnum stackPop(StackPtr stack) {
    if (stack == NULL) {
        fprintf(stderr, "stackPop: NULL pointer\n");
        return ERROR_DEFAULT;
    }
    if (stack->top < 0) {
        fprintf(stderr, "stackPop: stack underflow\n");
        return ERROR_STACK_UNDERFLOW;
    }
    stack->top--;
    return SUCCESS;
}

StatusEnum stackTop(StackPtr stack, int8_t* out) {
    if (stack == NULL || out == NULL) {
        fprintf(stderr, "stackTop: NULL pointer\n");
        return ERROR_DEFAULT;
    }
    if (stack->top < 0) {
        fprintf(stderr, "stackTop: stack is empty\n");
        return ERROR_STACK_UNDERFLOW;
    }
    *out = stack->data[stack->top];
    return SUCCESS;
}

int8_t stackIsEmpty(StackPtr stack) {
    return (stack == NULL || stack->top < 0) ? 1 : 0;
}
