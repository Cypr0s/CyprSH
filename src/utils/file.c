#include "file.h"

void open_file(const char* path, uint32_t flag, int32_t* file_descriptor) {
    *file_descriptor = open(path, flag, 0644);
    if(*file_descriptor != -1) {
        return;
    }
    print_errno(path);
    if(errno == EACCES || errno == EISDIR) {
        error = ERROR_COMM_CANNOT_EXEC;
    }
    else if(errno == ENOENT || errno == ENOTDIR) {
        error = ERROR_COMMAND_NOT_FOUND;
    }
    else {
        error = ERROR_DEFAULT;
    }
}


void create_file(const char* path) {
    int32_t file_descriptor = open(path, O_CREAT | O_EXCL, 0644);
    if(file_descriptor >= 0) {
        close(file_descriptor);
        return;
    }
    if(errno == EEXIST) {
        return;
    }
    print_errno(path);
    if(errno == EACCES || errno == EISDIR) {
        error = ERROR_COMM_CANNOT_EXEC;
    }
    else if(errno == ENOTDIR || errno == ENOENT) {
        error = ERROR_COMMAND_NOT_FOUND;
    }
    else {
        error = ERROR_DEFAULT;
    }
}