#include "file.h"

ExitEnum open_file(const char* path, uint32_t flag, int32_t* file_descriptor) {
    *file_descriptor = open(path, flag);
    if(*file_descriptor == -1) {
        fprintf(stderr, "%s: %s\n", path, strerror(errno));
        if(errno == EACCES || errno == EISDIR) {
            return EXIT_COMM_CANNOT_EXEC;
        }
        if(errno == ENOENT || errno == ENOTDIR) {
            return EXIT_COMMAND_NOT_FOUND;
        }
        return EXIT_ERROR;
    }

    return EXIT_SUCCESS;
}