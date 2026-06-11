#include "error.h"


typedef struct {
    size_t size;
    size_t capacity;
    char* buff;
} CharBuffer, *CharBufferPtr;