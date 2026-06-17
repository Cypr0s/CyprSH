#include "data_structures/char_buffer.h"


StatusEnum charBufferCtor(CharBufferPtr cb, size_t capacity) {
    cb->size = 0;
    cb->capacity = capacity > 0 ? capacity : DEFAULT_BUFFER_SIZE;
    cb->buff = malloc(cb->capacity);
    if(cb->buff == NULL) {
        return ERROR_MALLOC_FAILURE;
    }
    cb->buff[0] = '\0';
    return SUCCESS;
}


void charBufferDtor(CharBufferPtr cb) {
    free(cb->buff);
    cb->buff = NULL;
    cb->size = 0;
    cb->capacity = 0;
}


StatusEnum charBufferAppendChar(CharBufferPtr cb, char c) {
    return charBufferAppendCharPtr(cb, &c, 1);
}


StatusEnum charBufferAppendCharPtr(CharBufferPtr cb, char* str, size_t str_size) {
    size_t needed = cb->size + str_size + 1; // +1 == '\0'
    if(needed > cb->capacity) {
        while(cb->capacity < needed) {
            cb->capacity *= 2;
        }
        char* new_buff = realloc(cb->buff, cb->capacity);
        if(new_buff == NULL) {
            return ERROR_MALLOC_FAILURE;
        }
        cb->buff = new_buff;
    }

    memcpy(cb->buff + cb->size, str, str_size);
    cb->size += str_size;
    cb->buff[cb->size] = '\0';
    return SUCCESS;
}


char* charBufferTransfer(CharBufferPtr cb) {
    char* out = cb->buff;
    cb->buff = NULL;
    cb->capacity = 0;
    cb->size = 0;
    return out;
}