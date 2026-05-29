#ifndef STACK_H
#define STACK_H

#include <stdint.h>
#include "../error.h"

#define STACK_CAPACITY 128

/* Generic fixed-size stack over int8_t.
   Enum values are stored by casting to int8_t; cast back on retrieval. */
typedef struct {
    int8_t data[STACK_CAPACITY];
    int8_t top;  // -1 = empty
} Stack, *StackPtr;

StatusEnum stackInit(StackPtr stack);
StatusEnum stackPush(StackPtr stack, int8_t value);
StatusEnum stackPop(StackPtr stack);
StatusEnum stackTop(StackPtr stack, int8_t* out);
int8_t stackIsEmpty(StackPtr stack);

#endif // STACK_H
