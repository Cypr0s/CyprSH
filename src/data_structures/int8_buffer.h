#ifndef INT8_T_BUFFER_H
#define INT8_T_BUFFER_H

#include "data_structures/buffer.h"
#include "stdint.h"


typedef struct {
    size_t size;
    size_t capacity;
    int8_t* buff;
} Int8Buffer, *Int8BufferPtr;


StatusEnum int8BufferCtor(Int8BufferPtr tb, size_t capacity);

void int8BufferDtor(Int8BufferPtr tb);

StatusEnum int8BufferAppend(Int8BufferPtr tb, int8_t value);

int8_t* int8BufferTransfer(Int8BufferPtr tb);

#endif