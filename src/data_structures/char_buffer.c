#include "data_structures/char_buffer.h"


StatusEnum charBufferCtor(CharBufferPtr cb, size_t capacity) {
    if(cb == NULL) {
        printError("charBufferCtor", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }

    cb->size = 0;
    cb->capacity = capacity > 0 ? capacity : DEFAULT_BUFFER_SIZE;
    cb->buff = (char*) malloc(cb->capacity);
    if(cb->buff == NULL) {
        printError("charBufferCtor", "Malloc failure");
        return ERROR_MALLOC_FAILURE;
    }
    cb->buff[0] = '\0';
    return SUCCESS;
}


void charBufferDtor(CharBufferPtr cb) {
    if(cb == NULL) {
        return;
    }

    free(cb->buff);
    cb->buff = NULL;
    cb->size = 0;
    cb->capacity = 0;
}


StatusEnum charBufferAppendChar(CharBufferPtr cb, char c) {
    if(cb == NULL) {
        printError("charBufferAppend", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }

    return charBufferAppendCharPtr(cb, &c, 1);
}


StatusEnum charBufferAppendCharPtr(CharBufferPtr cb, char* str, size_t str_size) {
    if(cb == NULL || str == NULL) {
        printError("charBufferAppend", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }

    size_t needed = cb->size + str_size + 1; // +1 == '\0'
    if(needed > cb->capacity) {
        while(cb->capacity < needed) {
            cb->capacity *= 2;
        }
        char* new_buff = (char*) realloc(cb->buff, cb->capacity);
        if(new_buff == NULL) {
            printError("charBufferAppend", "Realloc failure");
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
    if(cb == NULL) {
        printError("charBufferTransfer", "Passing NULL pointer");
        return NULL;
    }
    char* out = cb->buff;
    cb->buff = NULL;
    cb->capacity = 0;
    cb->size = 0;
    return out;
}