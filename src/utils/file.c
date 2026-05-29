#include "file.h"

StatusEnum open_file(const char* path, uint32_t flag, int32_t* file_descriptor) {
    *file_descriptor = open(path, flag, 0644);
    if(*file_descriptor != -1) {
        return SUCCESS;
    }
    printErrno(path);
    if(errno == EACCES || errno == EISDIR) {
        return ERROR_COMM_CANNOT_EXEC;
    }
    else if(errno == ENOENT || errno == ENOTDIR) {
        return ERROR_COMMAND_NOT_FOUND;
    }
    
    return ERROR_DEFAULT;
}


StatusEnum create_file(const char* path) {
    int32_t file_descriptor = open(path, O_CREAT | O_EXCL, 0644);
    if(file_descriptor >= 0) {
        close(file_descriptor);
        return SUCCESS;
    }
    if(errno == EEXIST) {
        return SUCCESS;
    }
    printErrno(path);
    if(errno == EACCES || errno == EISDIR) {
        return ERROR_COMM_CANNOT_EXEC;
    }
    else if(errno == ENOTDIR || errno == ENOENT) {
        return ERROR_COMMAND_NOT_FOUND;
    }
    
    return ERROR_DEFAULT;
}