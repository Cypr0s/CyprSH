#ifndef STRINGS_H
#define STRINGS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "error.h"

uint8_t streq(const char* str1, const char* str2);

char* strdup(const char* src);

StatusEnum splitAssignment(const char* str, char** key, char** value);

#endif // STRINGS_H