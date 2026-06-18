#include "data_structures/stack.h"


StatusEnum stackCtor(StackPtr stack) {
    if (stack == NULL) {
        printError("stackInit", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }
    stack->top = -1;
    return SUCCESS;
}


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


int8_t stackIsEmpty(StackPtr stack) {
    if(stack == NULL) {
        printError("stackIsEmpty", "Passing NULL pointer");
        return -1;
    }
    return (stack == NULL || stack->top < 0) ? 1 : 0;
}
