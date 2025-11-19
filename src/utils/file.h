#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "error.h"

ExitEnum open_file(const char* path, __uint32_t flag, int32_t* file_descriptor);