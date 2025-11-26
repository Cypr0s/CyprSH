#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include "error.h"

void open_file(const char* path, __uint32_t flag, int32_t* file_descriptor);

void create_file(const char* name_path);