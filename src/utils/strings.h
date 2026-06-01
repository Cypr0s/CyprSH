#ifndef STRINGS_H
#define STRINGS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

uint8_t streq(const char* str1, const char* str2);

char* strdup(const char* src);

#endif // STRINGS_H