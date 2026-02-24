#include "file.h"

StatusEnum open_file(const char* path, uint32_t flag, int32_t* file_descriptor) {
    *file_descriptor = open(path, flag, 0644);
    if(*file_descriptor != -1) {
        return;
    }
    print_errno(path);
    if(errno == EACCES || errno == EISDIR) {
        error = EXIT_COMM_CANNOT_EXEC;
    }
    else if(errno == ENOENT || errno == ENOTDIR) {
        error = EXIT_COMMAND_NOT_FOUND;
    }
    else {
        error = EXIT_ERROR;
    }
}


void create_file(const char* path) {
    uint32_t file_descriptor = open(path, O_CREAT | O_EXCL, 0644); // owner rw any other r
    if(file_descriptor >= 0) {
        close(file_descriptor); // also close it we just want to create it
        return;
    }
    else if(file_descriptor == -1) {
        print_errno(path);
        if(errno == EACCES || errno == EISDIR) {
            error = EXIT_COMM_CANNOT_EXEC;
        }
        else if(errno == ENOTDIR || errno == ENOENT) {
            error = EXIT_COMMAND_NOT_FOUND;
        }
        else {
            error = EXIT_ERROR;
        }
    }
}