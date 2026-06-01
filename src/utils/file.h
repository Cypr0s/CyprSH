#ifndef FILE_H
#define FILE_H

#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include "error.h"

StatusEnum open_file(const char* path, uint32_t flag, int32_t* file_descriptor);

StatusEnum create_file(const char* name_path);

#endif // FILE_H