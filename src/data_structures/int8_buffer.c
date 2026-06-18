#include "data_structures/int8_buffer.h"


StatusEnum int8BufferCtor(Int8BufferPtr tb, size_t capacity) {
    if(tb == NULL) {
        printError("int8BufferCtor", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }

    tb->size = 0;
    tb->capacity = capacity > 0 ? capacity : DEFAULT_BUFFER_SIZE;
    tb->buff = (int8_t*) malloc(tb->capacity * sizeof(int8_t));
    if(tb->buff == NULL) {
        printError("int8BufferCtor", "Malloc failure");
        return ERROR_MALLOC_FAILURE;
    }
    return SUCCESS;
}


void int8BufferDtor(Int8BufferPtr tb) {
    if(tb == NULL) {
        return;
    }

    free(tb->buff);
    tb->buff = NULL;
    tb->size = 0;
    tb->capacity = 0;
}


StatusEnum int8BufferAppend(Int8BufferPtr tb, int8_t value) {
    if(tb == NULL) {
        printError("int8BufferAppend", "Passing NULL pointer");
        return ERROR_DEFAULT;
    }

    size_t needed = tb->size + 1;
    if(needed > tb->capacity) {
        while(tb->capacity < needed) {
            tb->capacity *= 2;
        }
        int8_t* new_buff = (int8_t*) realloc(tb->buff, tb->capacity * sizeof(int8_t));
        if(new_buff == NULL) {
            printError("int8BufferAppend", "Realloc failure");
            return ERROR_MALLOC_FAILURE;
        }
        tb->buff = new_buff;
    }

    tb->buff[tb->size] = value;
    tb->size++;
    return SUCCESS;
}

// not needed
int8_t* int8BufferTransfer(Int8BufferPtr tb) {
    if(tb == NULL) {
        printError("int8BufferTransfer", "Passing NULL pointer");
        return NULL;
    }
    int8_t* out = tb->buff;
    tb->buff = NULL;
    tb->capacity = 0;
    tb->size = 0;
    return out;
}