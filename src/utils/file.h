#ifndef FILE_H
#define FILE_H

#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include "error.h"

StatusEnum openFile(const char* path, uint32_t flag, int32_t* file_descriptor);

StatusEnum createFile(const char* name_path);

#endif // FILE_H