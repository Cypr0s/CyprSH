#ifndef CHAR_BUFFER_H
#define CHAR_BUFFER_H

#include "data_structures/buffer.h"

typedef struct {
    size_t size;
    size_t capacity;
    char* buff;
} CharBuffer, *CharBufferPtr;

StatusEnum charBufferCtor(CharBufferPtr cb, size_t capacity);

void charBufferDtor(CharBufferPtr cb);

StatusEnum charBufferAppendChar(CharBufferPtr cb, char c);

StatusEnum charBufferAppendCharPtr(CharBufferPtr cb, char* str, size_t str_size);

char* charBufferTransfer(CharBufferPtr cb);

#endif

