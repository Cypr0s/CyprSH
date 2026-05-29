#include "strings.h"

uint8_t streq(const char* str1, const char* str2) {
    while(*str1 && *str2) {
        if(*str1 != *str2) return 0U;
        str1++;
        str2++;
    }

    return (*str1 == *str2) ? 1U : 0U;
}


char* strdup(const char* src) {
    size_t len = strlen(src);
    char* copy = (char*) malloc(len + 1);
    if(copy == NULL) {
        return NULL;
    }
    memcpy(copy, src, len + 1);
    return copy;
}

char* read_line(FILE* input) {

}