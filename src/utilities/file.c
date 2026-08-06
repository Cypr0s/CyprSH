/**
 * @file        file.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   File operations implementation
 */

#include "utilities/file.h"

/** @brief Open file with given flags
 *  @param path File path
 *  @param flag Open flags (O_RDONLY, etc.)
 *  @param file_descriptor Output file descriptor
 *  @return Status code
 */
StatusEnum openFile(const char* path, uint32_t flag, int32_t* file_descriptor) {
    *file_descriptor = open(path, flag, 0644);
    if(*file_descriptor != -1) {
        return SUCCESS;
    }
    printErrno(path);
    if(errno == ENOENT || errno == ENOTDIR) {
        return ERROR_FILE_NOT_FOUND;
    }
    if(errno == EACCES || errno == EISDIR) {
        return ERROR_FILE_NOT_READABLE;
    }
    return ERROR_DEFAULT;
}

/** @brief Create new file (ignored if exists)
 *  @param name_path File path to create
 *  @return Status code
 */
StatusEnum createFile(const char* path) {
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
        return ERROR_FILE_NOT_READABLE;
    }
    if(errno == ENOTDIR || errno == ENOENT) {
        return ERROR_FILE_NOT_FOUND;
    }
    
    return ERROR_DEFAULT;
}